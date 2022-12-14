/*
 * Copyright 2017-2021 AVSystem <avsystem@avsystem.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdbool.h>

#include <anjay/anjay.h>
#include <anjay/attr_storage.h>
#include <anjay/security.h>
#include <anjay/server.h>

#include <avsystem/commons/avs_list.h>
#include <avsystem/commons/avs_log.h>
#include <avsystem/commons/avs_prng.h>

#include "cellular_service_datacache.h"
#include "cmsis_os_misrac2012.h"
#include "dc_common.h"
#include "error_handler.h"

#include "lwm2m.h"
#include "anjay_client_config.h"

#include "device_object.h"

#include "lwip/sockets.h"

#define LOG(level, ...) avs_log(app, level, __VA_ARGS__)

static anjay_t *g_anjay;
static avs_crypto_prng_ctx_t *g_prng_ctx;

static osThreadId g_lwm2m_task_handle;
static osThreadId g_lwm2m_notify_task_handle;

osMutexDef(anjay_mtx);
static osMutexId g_anjay_mtx;

#define LOCKED(mtx)                                                 \
    for (osStatus s = osMutexWait((mtx), osWaitForever); s == osOK; \
         s = osErrorOS, osMutexRelease((mtx)))

extern RNG_HandleTypeDef hrng;

// Used to communicate between datacache callback and lwm2m thread
static osMessageQId status_msg_queue;

static volatile bool g_network_up;

static void dc_cellular_callback(dc_com_event_id_t dc_event_id,
                                 const void *user_arg) {
    (void) user_arg;

    if (dc_event_id == DC_CELLULAR_NIFMAN_INFO) {
        dc_nifman_info_t dc_nifman_info;
        (void) dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO,
                           (void *) &dc_nifman_info, sizeof(dc_nifman_info));
        if (dc_nifman_info.rt_state == DC_SERVICE_ON) {
            g_network_up = true;
            LOG(INFO, "network is up");
            (void) osMessagePut(status_msg_queue, (uint32_t) dc_event_id, 0);
        } else {
            g_network_up = false;
            LOG(INFO, "network is down");
        }
    } else if (dc_event_id == DC_CELLULAR_CONFIG) {
        dc_cellular_params_t dc_cellular_params;
        (void) dc_com_read(&dc_com_db, DC_CELLULAR_CONFIG,
                           (void *) &dc_cellular_params,
                           sizeof(dc_cellular_params));
        if (dc_cellular_params.rt_state == DC_SERVICE_ON) {
            LOG(INFO, "cellular configuration is updated");
        }
    }
}

void main_loop(void) {
    while (g_network_up) {
        AVS_LIST(avs_net_socket_t *const) sockets = NULL;
        LOCKED(g_anjay_mtx) {
            sockets = anjay_get_sockets(g_anjay);
        }

        size_t numsocks = AVS_LIST_SIZE(sockets);
        struct pollfd pollfds[numsocks];
        size_t i = 0;
        AVS_LIST(avs_net_socket_t *const) sock;
        AVS_LIST_FOREACH(sock, sockets) {
            pollfds[i].fd = *(const int *) avs_net_socket_get_system(*sock);
            pollfds[i].events = POLLIN;
            pollfds[i].revents = 0;
            ++i;
        }

        const int max_wait_time_ms = 1000;
        int wait_ms = max_wait_time_ms;
        LOCKED(g_anjay_mtx) {
            wait_ms = anjay_sched_calculate_wait_time_ms(g_anjay,
                                                         max_wait_time_ms);
        }

        if (poll(pollfds, numsocks, wait_ms) > 0) {
            int socket_id = 0;
            AVS_LIST(avs_net_socket_t *const) socket = NULL;
            AVS_LIST_FOREACH(socket, sockets) {
                if (pollfds[socket_id].revents) {
                    LOCKED(g_anjay_mtx) {
                        if (anjay_serve(g_anjay, *socket)) {
                            LOG(ERROR, "anjay_serve() failed");
                        }
                    }
                }
                ++socket_id;
            }
        }

        LOCKED(g_anjay_mtx) {
            anjay_sched_run(g_anjay);
        }
    }
}

static void lwm2m_thread(void const *user_arg) {
    (void) user_arg;

    (void) osMessageGet(status_msg_queue, RTOS_WAIT_FOREVER);

    // TODO handle connection lost
    main_loop();
}


static void lwm2m_notify_thread(void const *process_fcn) {
    while (true) {
        LOCKED(g_anjay_mtx) {
            device_object_update(g_anjay);
            ((void (*)())process_fcn)();
        }
        osDelay(1000);
    }
}

static int
entropy_callback(unsigned char *out_buf, size_t out_buf_len, void *user_ptr) {
    uint32_t random_number;
    for (size_t i = 0; i < out_buf_len / sizeof(random_number); i++) {
        if (HAL_RNG_GenerateRandomNumber(&hrng, &random_number) != HAL_OK) {
            return -1;
        }
        memcpy(out_buf, &random_number, sizeof(random_number));
        out_buf += sizeof(random_number);
    }

    size_t last_chunk_size = out_buf_len % sizeof(random_number);
    if (last_chunk_size) {
        if (HAL_RNG_GenerateRandomNumber(&hrng, &random_number) != HAL_OK) {
            return -1;
        }
        memcpy(out_buf, &random_number, last_chunk_size);
    }

    return 0;
}

static int setup_security_object() {
    if (anjay_security_object_install(g_anjay)) {
        return -1;
    }

    const char *endpoint_name = ANJAY_CLIENT_CONFIG_PSK_IDENTITY;
    const char *psk = ANJAY_CLIENT_CONFIG_PSK;

    const anjay_security_instance_t security_instance = {
        .ssid = 1,
        .server_uri = ANJAY_CLIENT_CONFIG_SERVER_URI,
        .security_mode = ANJAY_SECURITY_PSK,
        .public_cert_or_psk_identity = (uint8_t *) endpoint_name,
        .public_cert_or_psk_identity_size = strlen(endpoint_name),
        .private_cert_or_psk_key = (uint8_t *) psk,
        .private_cert_or_psk_key_size = strlen(psk)
    };

    anjay_iid_t security_instance_id = ANJAY_ID_INVALID;
    return anjay_security_object_add_instance(g_anjay, &security_instance,
                                              &security_instance_id);
}

static int setup_server_object() {
    if (anjay_server_object_install(g_anjay)) {
        return -1;
    }

    const anjay_server_instance_t server_instance = {
        .ssid = 1,
        .lifetime = 60,
        .default_min_period = -1,
        .default_max_period = -1,
        .disable_timeout = -1,
        .binding = "U"
    };

    anjay_iid_t server_instance_id = ANJAY_ID_INVALID;
    return anjay_server_object_add_instance(g_anjay, &server_instance,
                                            &server_instance_id);
}

static void configure_modem(void) {
    dc_cellular_params_t cellular_params;
    memset(&cellular_params, 0, sizeof(cellular_params));
    dc_com_read(&dc_com_db, DC_CELLULAR_CONFIG, &cellular_params,
                sizeof(cellular_params));

    const uint8_t sim_slot_index = 0;
    const dc_sim_slot_t *sim_slot = &cellular_params.sim_slot[sim_slot_index];

    strcpy((char *) sim_slot->apn, ANJAY_CLIENT_CONFIG_APN);
    strcpy((char *) sim_slot->username, ANJAY_CLIENT_CONFIG_APN_USERNAME);
    strcpy((char *) sim_slot->password, ANJAY_CLIENT_CONFIG_APN_PASSWORD);

    dc_com_write(&dc_com_db, DC_CELLULAR_CONFIG, &cellular_params,
                 sizeof(cellular_params));
    LOG(INFO, "Modem config written");
}

anjay_t * lwm2m_init(void) {
    osMessageQDef(status_msg_queue, 1, uint32_t);
    status_msg_queue = osMessageCreate(osMessageQ(status_msg_queue), NULL);
    if (!status_msg_queue) {
        LOG(ERROR, "failed to create message queue");
        ERROR_Handler(DBG_CHAN_APPLICATION, 0, ERROR_FATAL);
    }

    // Registration to datacache
    dc_com_reg_id_t reg =
            dc_com_register_gen_event_cb(&dc_com_db, dc_cellular_callback,
                                         NULL);
    if (reg == DC_COM_INVALID_ENTRY) {
        LOG(ERROR, "failed to subscribe to datacache events");
        ERROR_Handler(DBG_CHAN_APPLICATION, 0, ERROR_FATAL);
    }

    g_prng_ctx = avs_crypto_prng_new(entropy_callback, NULL);
    if (!g_prng_ctx) {
        LOG(ERROR, "failed to create PRNG ctx");
        ERROR_Handler(DBG_CHAN_APPLICATION, 0, ERROR_FATAL);
    }

    anjay_configuration_t config = {
        .endpoint_name = ANJAY_CLIENT_CONFIG_ENDPOINT_NAME,
        .in_buffer_size = 2048,
        .out_buffer_size = 2048,
        .msg_cache_size = 2048,
        .prng_ctx = g_prng_ctx
    };

    if (!(g_anjay = anjay_new(&config))) {
        LOG(ERROR, "failed to create Anjay object");
        ERROR_Handler(DBG_CHAN_APPLICATION, 0, ERROR_FATAL);
    }

    if (setup_security_object() || setup_server_object()
            || anjay_attr_storage_install(g_anjay)
            || device_object_install(g_anjay)) {
        LOG(ERROR, "failed to setup required objects");
        ERROR_Handler(DBG_CHAN_APPLICATION, 0, ERROR_FATAL);
    }

    if (!(g_anjay_mtx = osMutexCreate(osMutex(anjay_mtx)))) {
        LOG(ERROR, "failed to create Anjay mutex");
        ERROR_Handler(DBG_CHAN_APPLICATION, 0, ERROR_FATAL);
    }
    LOG(INFO, "Initialized LwM2M");
    configure_modem();
    return g_anjay;
}

static uint32_t lwm2m_thread_stack_buffer[LWM2M_THREAD_STACK_SIZE];
static osStaticThreadDef_t lwm2m_thread_controlblock;
void lwm2m_start(void) {
    osThreadStaticDef(lwm2m_task, lwm2m_thread, LWM2M_THREAD_PRIO, 0,
                      LWM2M_THREAD_STACK_SIZE, lwm2m_thread_stack_buffer,
                      &lwm2m_thread_controlblock);
    g_lwm2m_task_handle = osThreadCreate(osThread(lwm2m_task), NULL);

    if (!g_lwm2m_task_handle) {
        LOG(ERROR, "failed to create thread");
        ERROR_Handler(DBG_CHAN_APPLICATION, 0, ERROR_FATAL);
    }
    LOG(INFO, "Created Anjay LwM2M thread.");
}

static uint32_t
        lwm2m_notify_thread_stack_buffer[LWM2M_NOTIFY_THREAD_STACK_SIZE];
static osStaticThreadDef_t lwm2m_notify_controlblock;
void lwm2m_notify_start(void (* process_fcn)()) {
    osThreadStaticDef(lwm2m_notify_task, lwm2m_notify_thread,
                      LWM2M_NOTIFY_THREAD_PRIO, 0,
                      LWM2M_NOTIFY_THREAD_STACK_SIZE,
                      lwm2m_notify_thread_stack_buffer,
                      &lwm2m_notify_controlblock);
    g_lwm2m_notify_task_handle =
            osThreadCreate(osThread(lwm2m_notify_task), (void *)process_fcn);

    if (!g_lwm2m_notify_task_handle) {
        LOG(ERROR, "failed to create thread");
        ERROR_Handler(DBG_CHAN_APPLICATION, 0, ERROR_FATAL);
    }
}

