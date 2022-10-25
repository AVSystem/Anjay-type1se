/**
  ******************************************************************************
  * @file    plf_modem_config.h
  * @author  MCD Application Team
  * @brief   This file contains the modem configuration for TYPE1SC
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PLF_MODEM_CONFIG_H
#define PLF_MODEM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup AT_CUSTOM AT_CUSTOM
  * @{
  */

/** @addtogroup AT_CUSTOM_ALTAIR_T1SC AT_CUSTOM ALTAIR_T1SC
  * @{
  */

/** @addtogroup AT_CUSTOM_ALTAIR_T1SC_CONFIG AT_CUSTOM ALTAIR_T1SC CONFIG
  * @{
  */

/** @defgroup AT_CUSTOM_ALTAIR_T1SC_CONFIG_Exported_Defines AT_CUSTOM ALTAIR_T1SC CONFIG Exported Defines
  * @{
  */

#if defined(HWREF_MURATA_TYPE1SC_EVK)
/* already explicitly defined:
 * using HWREF_MURATA_TYPE1SC_EVK directly on STMOD+ connector
 */
#else
/* set default config */
#define HWREF_MURATA_TYPE1SC_EVK
#endif  /*  defined(HWREF_MURATA_TYPE1SC_EVK) */

/* MODEM parameters */
#define USE_MODEM_TYPE1SC
#define CONFIG_MODEM_UART_BAUDRATE (115200U)
#define CONFIG_MODEM_USE_STMOD_CONNECTOR

#define UDP_SERVICE_SUPPORTED                (1U)
#define CONFIG_MODEM_UDP_SERVICE_CONNECT_IP  ((uint8_t *)"0.0.0.0")
#define CONFIG_MODEM_MAX_SOCKET_TX_DATA_SIZE ((uint32_t)710U) /* sendto managed, remote IP/port should be send
                                                                 IPv6/IPv4: protocol around 80/50 bytes
                                                                 data send in ASCII format:(1500-80)/2 = 710 */
#define CONFIG_MODEM_MAX_SOCKET_RX_DATA_SIZE ((uint32_t)750U) /* receivefrom managed, remote IP/port should be received
                                                                 IPv6/IPv4: protocol around 80/50 bytes
                                                                 data received in ASCII format:(1600-80)/2 = 760 */
#define CONFIG_MODEM_MAX_SIM_GENERIC_ACCESS_CMD_SIZE ((uint32_t)1460U)
#define CONFIG_MODEM_MIN_SIM_GENERIC_ACCESS_RSP_SIZE ((uint32_t)4U)

/* Ping URC received before or after Reply */
#define PING_URC_RECEIVED_AFTER_REPLY        (0U)

/* UART flow control settings */
#if defined(USER_FLAG_MODEM_FORCE_NO_FLOW_CTRL)
#define CONFIG_MODEM_UART_RTS_CTS  (0)
#elif defined(USER_FLAG_MODEM_FORCE_HW_FLOW_CTRL)
#define CONFIG_MODEM_UART_RTS_CTS  (1)
#else /* default FLOW CONTROL setting for Type1SC */
#define CONFIG_MODEM_UART_RTS_CTS  (1)
#endif /* user flag for modem flow control */

/* At the end of the modem power on, this parameter defines whether we apply the theoretical delay to let the
 * modem start or if we try to establish communication immediately.
 */
#define TYPE1SC_FASTEST_POWER_ON (0U)

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /*_PLF_MODEM_CONFIG_H */
