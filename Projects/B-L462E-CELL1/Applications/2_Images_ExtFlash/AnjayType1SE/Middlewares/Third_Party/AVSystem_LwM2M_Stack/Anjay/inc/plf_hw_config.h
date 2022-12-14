/**
  ******************************************************************************
  * @file    plf_hw_config.h
  * @author  MCD Application Team
  * @brief   This file contains the hardware configuration of the platform
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PLF_HW_CONFIG_H
#define PLF_HW_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* MISRAC messages linked to HAL include are ignored */
/*cstat -MISRAC2012-* */
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
/*cstat +MISRAC2012-* */

#include "main.h"
#include "plf_modem_config.h"
#include "usart.h"

#if defined(STM32L496xx)
/* Exported constants --------------------------------------------------------*/

/* Platform defines ----------------------------------------------------------*/

/* MODEM configuration */
#if defined(CONFIG_MODEM_USE_STMOD_CONNECTOR)
#define MODEM_UART_HANDLE                huart1
#define MODEM_UART_INSTANCE              ((USART_TypeDef *)USART1)
#define MODEM_UART_AUTOBAUD              (1)
#define MODEM_UART_IRQN                  USART1_IRQn
#define MODEM_UART_ALTERNATE             GPIO_AF7_USART1
#else
#error Modem connector not specified or invalid for this board
#endif /* defined(CONFIG_MODEM_USE_STMOD_CONNECTOR) */

/* UART interface */
#define MODEM_UART_BAUDRATE              (CONFIG_MODEM_UART_BAUDRATE)
#define MODEM_UART_WORDLENGTH            UART_WORDLENGTH_8B
#define MODEM_UART_STOPBITS              UART_STOPBITS_1
#define MODEM_UART_PARITY                UART_PARITY_NONE
#define MODEM_UART_MODE                  UART_MODE_TX_RX

#if (CONFIG_MODEM_UART_RTS_CTS == 1)
#define MODEM_UART_HWFLOWCTRL            UART_HWCONTROL_RTS_CTS
#else
#define MODEM_UART_HWFLOWCTRL            UART_HWCONTROL_NONE
#endif /* (CONFIG_MODEM_UART_RTS_CTS == 1) */

#define MODEM_TX_GPIO_PORT               ((GPIO_TypeDef *)USART1_TX_GPIO_Port)   /* for DiscoL496: GPIOB       */
#define MODEM_TX_PIN                     USART1_TX_Pin                           /* for DiscoL496: GPIO_PIN_6  */
#define MODEM_RX_GPIO_PORT               ((GPIO_TypeDef *)UART1_RX_GPIO_Port)    /* for DiscoL496: GPIOG       */
#define MODEM_RX_PIN                     UART1_RX_Pin                            /* for DiscoL496: GPIO_PIN_10 */
#define MODEM_CTS_GPIO_PORT              ((GPIO_TypeDef *)UART1_CTS_GPIO_Port)   /* for DiscoL496: GPIOG       */
#define MODEM_CTS_PIN                    UART1_CTS_Pin                           /* for DiscoL496: GPIO_PIN_11 */
#define MODEM_RTS_GPIO_PORT              ((GPIO_TypeDef *)UART1_RTS_GPIO_Port)   /* for DiscoL496: GPIOG       */
#define MODEM_RTS_PIN                    UART1_RTS_Pin                           /* for DiscoL496: GPIO_PIN_12 */

/* ---- MODEM other pins configuration ---- */
#if defined(CONFIG_MODEM_USE_STMOD_CONNECTOR)
/* output */
#define MODEM_RST_GPIO_PORT              ((GPIO_TypeDef *)STMOD_RESET_GPIO_Port)    /* for DiscoL496: GPIOB      */
#define MODEM_RST_PIN                    STMOD_RESET_Pin                            /* for DiscoL496: GPIO_PIN_2 */
#define MODEM_PWR_EN_GPIO_PORT           ((GPIO_TypeDef *)GPIOD) /* for DiscoL496: GPIOD      */
#define MODEM_PWR_EN_PIN                 GPIO_PIN_3                         /* for DiscoL496: GPIO_PIN_3 */
#define MODEM_DTR_GPIO_PORT              ((GPIO_TypeDef *)STMOD_PWM_GPIO_Port)    /* for DiscoL496: GPIOA      */
#define MODEM_DTR_PIN                    STMOD_PWM_Pin                            /* for DiscoL496: GPIO_PIN_0 */
/* input */
#define MODEM_RING_GPIO_PORT             ((GPIO_TypeDef *)STMOD_INT_GPIO_Port)  /* for DiscoL496: GPIOH      */
#define MODEM_RING_PIN                   STMOD_INT_Pin                          /* for DiscoL496: GPIO_PIN_2 */
#define MODEM_RING_IRQN                  EXTI2_IRQn
#else
#error Modem connector not specified or invalid for this board
#endif /* defined(CONFIG_MODEM_USE_STMOD_CONNECTOR) */

/* ---- MODEM SIM SELECTION pins ---- */
#define MODEM_SIM_SELECT_0_GPIO_PORT     GPIOC
#define MODEM_SIM_SELECT_0_PIN           GPIO_PIN_2
#define MODEM_SIM_SELECT_1_GPIO_PORT     GPIOI
#define MODEM_SIM_SELECT_1_PIN           GPIO_PIN_3

/* DEBUG INTERFACE CONFIGURATION */
#define TRACE_INTERFACE_UART_HANDLE      huart2
#define TRACE_INTERFACE_INSTANCE         ((USART_TypeDef *)USART2)



#elif defined(STM32L462xx)

/* Platform defines ----------------------------------------------------------*/

/* ST33 configuration */
#define ST33_SPI_HANDLE                 hspi3
#define ST33_SPI_INSTANCE               SPI3
#define ST33_SPI_BAUDRATEPRESCALER      SPI_BAUDRATEPRESCALER_8
#define ST33_SPI_MOSI_PIN               SPI3_MOSI_PIN
#define ST33_SPI_MISO_PIN               SPI3_MISO_PIN
#define ST33_SPI_MOSI_MISO_SCK_PORT     SPI3_PORT
#define ST33_SPI_CS_PIN                 ST33_CS_Pin
#define ST33_SPI_CS_PORT                ST33_CS_GPIO_Port
#define ST33_SPI_ALT_FUNCTION           SPI3_ALT_FUNCTION
#define ST33_SPI_SCK_PIN                SPI3_SCK_PIN

#define MODEM_UART_HANDLE       huart3
#define MODEM_UART_INSTANCE     USART3
#define MODEM_UART_AUTOBAUD     (0)
#define MODEM_UART_IRQN         USART3_IRQn

#define MODEM_UART_BAUDRATE     (CONFIG_MODEM_UART_BAUDRATE)
#define MODEM_UART_WORDLENGTH   UART_WORDLENGTH_8B
#define MODEM_UART_STOPBITS     UART_STOPBITS_1
#define MODEM_UART_PARITY       UART_PARITY_NONE
#define MODEM_UART_MODE         UART_MODE_TX_RX

#if (CONFIG_MODEM_UART_RTS_CTS == 1)
#define MODEM_UART_HWFLOWCTRL   UART_HWCONTROL_RTS_CTS
#else
#define MODEM_UART_HWFLOWCTRL   UART_HWCONTROL_NONE
#endif /* (CONFIG_MODEM_UART_RTS_CTS == 1) */

#define MODEM_TX_GPIO_PORT      ((GPIO_TypeDef *)MDM_UART_TX_GPIO_Port)
#define MODEM_TX_PIN            MDM_UART_TX_Pin
#define MODEM_RX_GPIO_PORT      ((GPIO_TypeDef *)MDM_UART_RX_GPIO_Port)
#define MODEM_RX_PIN            MDM_UART_RX_Pin
#define MODEM_CTS_GPIO_PORT     ((GPIO_TypeDef *)MDM_UART_CTS_GPIO_Port)
#define MODEM_CTS_PIN           MDM_UART_CTS_Pin
#define MODEM_RTS_GPIO_PORT     ((GPIO_TypeDef *)MDM_UART_RTS_GPIO_Port)
#define MODEM_RTS_PIN           MDM_UART_RTS_Pin

/* ---- MODEM other pins configuration ---- */
/* output */
#define MODEM_PWR_EN_GPIO_PORT          MDM_PWR_EN_OUT_GPIO_Port
#define MODEM_PWR_EN_PIN                MDM_PWR_EN_OUT_Pin
#define MODEM_DTR_GPIO_PORT             MDM_DTR_OUT_GPIO_Port
#define MODEM_DTR_PIN                   MDM_DTR_OUT_Pin
/* input */
#define MODEM_RING_GPIO_PORT    ((GPIO_TypeDef *)MDM_RING_GPIO_Port)
#define MODEM_RING_PIN          MDM_RING_Pin
#define MODEM_RING_IRQN         MDM_RING_EXTI_IRQn

#define PPPOS_LINK_UART_HANDLE   NULL
#define PPPOS_LINK_UART_INSTANCE NULL

/* DEBUG INTERFACE CONFIGURATION */

/* use test board trace UART - DEFAULT */
#define TRACE_INTERFACE_UART_HANDLE     huart1
#define TRACE_INTERFACE_INSTANCE        USART1

#define COM_INTERFACE_UART_HANDLE      huart2
#define COM_INTERFACE_INSTANCE        ((USART_TypeDef *)USART2)
#define COM_INTERFACE_UART_IRQ         USART2_IRQn
#define COM_INTERFACE_UART_INIT        MX_USART2_UART_Init(); \
  HAL_NVIC_EnableIRQ(COM_INTERFACE_UART_IRQ);

#define CS_DISP_PIN GPIO_PIN_2
#define CS_DISP_GPIO_PORT GPIOB

#if ((USE_DISPLAY == 1) || (USE_ST33 == 1))
#define SPI_INTERFACE                    (1)
#endif /* (USE_DISPLAY == 1) || (USE_ST33 == 1) */

#if (USE_DISPLAY == 1)
#include "stm32l462e_cell1.h"
#include "stm32l462e_cell1_lcd.h"
/*cstat -MISRAC2012-* */
#include "stm32_lcd.h"
/*cstat +MISRAC2012-* */
#define DISPLAY_WAIT_MODEM_IS_ON         (1) /* 0: no need to wait modem is on before to use display,
                                                1: need to wait modem is on before to use display   */
#define DISPLAY_DEFAULT_FONT             (Font12)
#define DISPLAY_INTERFACE                SPI_INTERFACE
#endif /* USE_DISPLAY == 1 */

#if (USE_ST33 == 1)
#define NDLC_INTERFACE                   SPI_INTERFACE
#endif /* !defined USE_DISPLAY */

#if (USE_SENSORS == 1)
#include "stm32l462e_cell1.h"
#include "stm32l462e_cell1_env_sensors.h"
#endif /* USE_SENSORS == 1 */

/* Exported types ------------------------------------------------------------*/

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#else
#error "Given board is not supported."
#endif

#ifdef __cplusplus
}
#endif

#endif /* PLF_HW_CONFIG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
