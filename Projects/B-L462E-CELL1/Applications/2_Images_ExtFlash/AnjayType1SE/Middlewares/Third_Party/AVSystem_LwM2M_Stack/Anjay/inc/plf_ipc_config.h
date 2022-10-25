/**
  ******************************************************************************
  * @file    plf_ipc_config.h
  * @author  MCD Application Team
  * @brief   This file defines IPC Configuration
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PLF_IPC_CONFIG_H
#define PLF_IPC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"

#define IPC_BUFFER_EXT    ((uint16_t) 400U) /* size added to RX buffer because of RX queue implementation (using
                                            * headers for messages)
                                            */
#define IPC_RXBUF_MAXSIZE ((uint16_t) 1600U + IPC_BUFFER_EXT) /* maximum size of character queue
                                                              * size has to match ATCMD_MAX_CMD_SIZE
                                                              */

/* IPC tuning parameters */
#if (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM)
/* SOCKET MODE (IP stack in the modem) */
#define IPC_USE_STREAM_MODE (0U)
#else
/*  STREAM MODE (IP stack in MCU) */
#define IPC_USE_STREAM_MODE (1U)
#define IPC_RXBUF_STREAM_MAXSIZE  ((uint16_t) IPC_RXBUF_MAXSIZE) /* maximum size of stream queue (if used) */
#endif  /* (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM) */

/* IPC_RXBUF_MAXSIZE and IPC_RXBUF_STREAM_MAXSIZE are defined above */
#define IPC_RXBUF_THRESHOLD  ((uint16_t) 20U)

/* IPC interface */
#define IPC_USE_UART (1U) /* UART activated by default */
#define IPC_USE_SPI  (0U) /* SPI NOT SUPPORTED YET */
#define IPC_USE_I2C  (0U) /* I2C NOT SUPPORTED YET */

/* Debug flags */
#define DBG_IPC_RX_FIFO  (0U)             /* additional debug infos */
#define DBG_QUEUE_SIZE ((uint16_t) 1000U) /* debug message history depth */

#ifdef __cplusplus
}
#endif

#endif /* PLF_IPC_CONFIG_H */
