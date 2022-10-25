/*
 * Copyright ##year## AVSystem <avsystem@avsystem.com>
 * AVSystem Commons library
 * All rights reserved.
 */

#include <avsystem/commons/avs_defs.h>
#include <avsystem/commons/avs_memory.h>
#include <avsystem/commons/avs_mutex.h>

#include "avs_cmsis_os_structs.h"

int _avs_mutex_init(avs_mutex_t *mutex) {
    mutex->def.controlblock = &mutex->control_block;

    mutex->id = osMutexCreate(&mutex->def);
    if (!mutex->id) {
        return -1;
    }

    return 0;
}

int avs_mutex_create(avs_mutex_t **out_mutex) {
    AVS_ASSERT(!*out_mutex, "possible attempt to reinitialize a mutex");

    *out_mutex = (avs_mutex_t *) avs_calloc(1, sizeof(avs_mutex_t));
    if (!*out_mutex) {
        return -1;
    }

    if (_avs_mutex_init(*out_mutex)) {
        avs_free(*out_mutex);
        *out_mutex = NULL;
        return -1;
    }

    return 0;
}

int avs_mutex_lock(avs_mutex_t *mutex) {
    return osMutexWait(mutex->id, osWaitForever) == osOK ? 0 : -1;
}

int avs_mutex_try_lock(avs_mutex_t *mutex) {
    return osMutexWait(mutex->id, 0) ? 0 : 1;
}

int avs_mutex_unlock(avs_mutex_t *mutex) {
    return osMutexRelease(mutex->id) == osOK ? 0 : -1;
}

void _avs_mutex_destroy(avs_mutex_t *mutex) {
    osStatus ret = osMutexDelete(mutex->id);
    (void) ret;
    AVS_ASSERT(ret == osOK, "osMutexDelete failed");
}

void avs_mutex_cleanup(avs_mutex_t **mutex) {
    if (!*mutex) {
        return;
    }

    _avs_mutex_destroy(*mutex);
    avs_free(*mutex);
    *mutex = NULL;
}
