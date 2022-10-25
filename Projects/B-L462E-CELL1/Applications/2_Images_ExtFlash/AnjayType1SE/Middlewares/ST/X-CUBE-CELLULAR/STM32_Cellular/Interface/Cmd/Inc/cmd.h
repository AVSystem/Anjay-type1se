/**
  ******************************************************************************
  * @file    cmd.h
  * @author  MCD Application Team
  * @brief   Header for cmd.c module
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
#ifndef CMD_H
#define CMD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"

#if (USE_CMD_CONSOLE == 1)

#include <stdint.h>

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef void (*CMD_HandlerCmd)(uint8_t *);

/* Exported functions ------------------------------------------------------- */
/**
  * @brief register a component
  * @param  cmd_name_p     command header of the component
  * @param  cmd_handler    callback of the component to manage the command
  * @param  cmd_label_p    description of the component to display at the help  command
  * @retval -
  */
void CMD_Declare(uint8_t *cmd_name_p, CMD_HandlerCmd cmd_handler, uint8_t *cmd_label_p);

/**
  * @brief  module initialization
  * @param  -
  * @retval -
  */
void CMD_init(void);

/**
  * @brief  module start
  * @param  -
  * @retval -
  */
void CMD_start(void);

/**
  * @brief console UART receive IT Callback
  * @param  uart_handle_p       console UART handle
  * @retval -
  */
void CMD_RxCpltCallback(UART_HandleTypeDef *uart_handle_p);

void CMD_SetLine(uint8_t *command_line);

/**
  * @brief display component help
  * @param  label   component description
  * @retval -
  */
void CMD_print_help(uint8_t *label);

/**
  * @brief  get an integer value from the argument
  * @param  string_p   (IN) acscii value to convert
  * @param  value_p    (OUT) converted uint32_t value
  * @retval return value
  */
uint32_t CMD_GetValue(uint8_t *string_p, uint32_t *value_p);

#endif /* USE_CMD_CONSOLE == 1 */

#ifdef __cplusplus
}
#endif

#endif /* CMD_H_ */
