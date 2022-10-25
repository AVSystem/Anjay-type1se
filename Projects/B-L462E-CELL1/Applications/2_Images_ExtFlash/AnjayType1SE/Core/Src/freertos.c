/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <anjay/anjay.h>
#include <anjay/attr_storage.h>
#include <anjay/security.h>
#include <anjay/server.h>

#include <avsystem/commons/avs_log.h>

#include "trace_interface.h"

#include "lwm2m.h"

#include "firmware_update.h"

#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IWDG_KEY_RELOAD 0x0000AAAAu
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId keepAliveTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void
log_handler(avs_log_level_t level, const char *module, const char *message) {
    traceIF_uartPrintForce(DBG_CHAN_APPLICATION, (uint8_t *) message,
                           strlen(message));
    traceIF_uartPrintForce(DBG_CHAN_APPLICATION, (uint8_t *) "\r\n", 2);
}

static void maybe_reboot_for_upgrade(avs_sched_t *sched, const void *data) {
    (void) data;

    if (fw_update_requested()) {
        fw_update_reboot();
        return;
    } else {
        avs_time_real_t now = avs_time_real_now();
        avs_time_real_t next_full_second = {
            .since_real_epoch = {
                .seconds = now.since_real_epoch.seconds + 1,
                .nanoseconds = 0
            }
        };
        AVS_SCHED_DELAYED(sched, NULL, avs_time_real_diff(next_full_second, now),
                          maybe_reboot_for_upgrade, NULL, NULL);
    }
}

static int setup_firmware_update_object(anjay_t *anjay) {
    if (fw_update_install(anjay)) {
        return -1;
    }

    avs_time_real_t now = avs_time_real_now();
    avs_time_real_t next_full_second = {
        .since_real_epoch = {
            .seconds = now.since_real_epoch.seconds + 1,
            .nanoseconds = 0
        }
    };

    AVS_SCHED_DELAYED(anjay_get_scheduler(anjay), NULL, avs_time_real_diff(next_full_second, now),
                      maybe_reboot_for_upgrade, NULL, NULL);

    return 0;
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  avs_log_set_handler(log_handler);
  srand(osKernelSysTick());

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of keepAliveTask */
  osThreadDef(keepAliveTask, StartDefaultTask, osPriorityNormal, 0, 256);
  keepAliveTaskHandle = osThreadCreate(osThread(keepAliveTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  cellular_init();
  anjay_t *anjay = lwm2m_init();
  setup_firmware_update_object(anjay);

  cellular_start();
  lwm2m_start();
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    osDelay(2000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
