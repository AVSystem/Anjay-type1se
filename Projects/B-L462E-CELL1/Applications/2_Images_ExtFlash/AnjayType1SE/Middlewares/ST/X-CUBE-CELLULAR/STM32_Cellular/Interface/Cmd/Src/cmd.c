/**
  ******************************************************************************
  * @file    cmd.c
  * @author  MCD Application Team
  * @brief   console cmd management
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


/* Includes ------------------------------------------------------------------*/
#include "cmd.h"

#if (USE_CMD_CONSOLE == 1)
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "cellular_runtime_standard.h"
#include "cellular_runtime_custom.h"
#include "usart.h"
#include "rtosal.h"
#include "error_handler.h"

/* Private defines -----------------------------------------------------------*/

#if !defined APPLICATION_CMD_NB
#define APPLICATION_CMD_NB          (0U) /* No application command usage */
#endif /* !defined APPLICATION_CMD_NB */

/** In Cellular, number max of components that add a Cmd
  * CellularService: 3U or 4U according to LowPower definition,
  * TraceInterface : 1U,
  * Cmd            : 1U,
  * ComLib         : 1U */
#if (USE_LOW_POWER == 1)
#define CMD_MAX_CMD                 (7U + (APPLICATION_CMD_NB)) /* number max of recorded components */
#else /* USE_LOW_POWER == 0 */
#define CMD_MAX_CMD                 (6U + (APPLICATION_CMD_NB)) /* number max of recorded components */
#endif  /* (USE_LOW_POWER == 1) */
#define CMD_MAX_LINE_SIZE           (100U)                      /* maximum size of command           */
#define CMD_READMEM_LINE_SIZE_MAX   (256U)                      /* maximum size of memory read       */
#define CMD_COMMAND_ALIGN_COLUMN    (16U)                       /* alignment size to display component description */


/* Private macros ------------------------------------------------------------*/
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_FORCE(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_UTILITIES, DBL_LVL_P0, "" format "", ## args)
#else
#include <stdio.h>
#define PRINT_FORCE(format, args...)   (void)printf("" format "", ## args);
#endif  /* (USE_PRINTF == 0U) */

/* Private typedef -----------------------------------------------------------*/
/* structure to record registered components */
typedef struct
{
  uint8_t         *CmdName;             /* header command of component */
  uint8_t         *CmdLabel;            /* component description       */
  CMD_HandlerCmd  CmdHandler;
} CMD_Struct_t;

/* Private variables ---------------------------------------------------------*/

static uint8_t             CMD_ReceivedChar;                       /* char to receive from UART */

static CMD_Struct_t        CMD_a_cmd_list[CMD_MAX_CMD];            /* list of recorded components */
static uint8_t             CMD_LastCommandLine[CMD_MAX_LINE_SIZE]; /* last command received       */
static uint8_t             CMD_CommandLine[2][CMD_MAX_LINE_SIZE];  /* current command receiving    */
static osSemaphoreId       CMD_rcvSemaphore  = 0U;
static uint32_t            CMD_NbCmd         = 0U;       /* Number of recorded components */

static uint8_t  *CMD_current_cmd;                        /* pointer on current received command  */
static uint8_t  *CMD_current_rcv_line;                   /* pointer on current receiving command */
static uint8_t  *CMD_completed_line;
static uint32_t  CMD_CurrentPos    = 0U;

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  Thread core of the command management
  * @note   This function find the component associated to the command (among the recorded components)
  * @param  p_argument (not used)
  * @retval -
  */
static void CMD_thread(const void *p_argument);

/**
  * @brief  Get CMD line from UART buffer
  * @param  p_Cmd_p command (not used because not parameter for this command)
  * @retval -
  */
static void CMD_GetLine(uint8_t *p_cmd, uint32_t max_size);

/**
  * @brief  Board reset command management
  * @param  -
  * @retval -
  */
static void CMD_BoardReset(void);

/**
  * @brief help command management
  * @note  display all recorded component (component command header and description)
  * @param  p_cmd (unused)
  * @retval -
  */
static void CMD_Help(uint8_t *p_cmd);

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief Get CMD line from UART buffer
  * @param  p_Cmd_p command (not used because not parameter for this command)
  * @retval -
  */
static void CMD_GetLine(uint8_t *command_line, uint32_t max_size)
{
  uint32_t size;
  /* Get Semaphore to avoid re-enter */
  /* Semaphore release has to be done at each end of line in UART IT callback  */
  (void)rtosalSemaphoreAcquire(CMD_rcvSemaphore, RTOSAL_WAIT_FOREVER);

  size = crs_strlen(CMD_current_cmd) + 1U;
  if (max_size < size)
  {
    /* If Uart buffer is too long, truncate it to the max_size */
    size = max_size;
  }

  /* Copy UART buffer to working buffer */
  (void)memcpy((CRC_CHAR_t *)command_line, (CRC_CHAR_t *)CMD_current_cmd, size);
}

void CMD_SetLine(uint8_t *command_line)
{
  (void)memcpy((CRC_CHAR_t *)CMD_current_cmd, (CRC_CHAR_t *)command_line, crs_strlen(command_line) + 1U);
  (void)rtosalSemaphoreRelease(CMD_rcvSemaphore);
}

/**
  * @brief  Thread core of the command management
  * @note   This function find the component associated to the command (among the recorded components)
  * @param  p_argument (not used)
  * @retval -
  */
static void CMD_thread(const void *p_argument)
{
  uint8_t cmd_prompt[3] = "$>"; /* command prompt to display $>\0 */
  uint8_t command_line[CMD_MAX_LINE_SIZE];
  uint32_t i;
  uint32_t cmd_size;
  uint32_t cmd_line_len;

  UNUSED(p_argument);

  for (;;)
  {
    /* get command line */
    CMD_GetLine(command_line, CMD_MAX_LINE_SIZE);
    if (command_line[0] != (uint8_t)'#')
    {
      /* not a comment line    */
      if (command_line[0] == 0U)
      {
        if (CMD_LastCommandLine[0] == 0U)
        {
          /* no last command: display help  */
          (void)memcpy((CRC_CHAR_t *)command_line, (CRC_CHAR_t *)"help", crs_strlen((const uint8_t *)"help") + 1U);
        }
        else
        {
          /* execute again last command  */
          /* No memory overflow: sizeof(command_line) == sizeof(CMD_LastCommandLine) */
          (void)memcpy((CRC_CHAR_t *)command_line, (CRC_CHAR_t *)CMD_LastCommandLine,
                       crs_strlen(CMD_LastCommandLine) + 1U);
        }
      }
      else
      {
        cmd_line_len = crs_strlen(command_line);
        if (cmd_line_len > 1U)
        {
          /* store last command             */
          /* No memory overflow: sizeof(command_line) == sizeof(CMD_LastCommandLine) */
          (void)memcpy((CRC_CHAR_t *)CMD_LastCommandLine, (CRC_CHAR_t *)command_line, crs_strlen(command_line) + 1U);
        }
      }

      /* command analysis                     */
      for (i = 0U; i < CMD_MAX_LINE_SIZE ;  i++)
      {
        /* Search for a space or the end of the string */
        if ((command_line[i] == (uint8_t)' ') || (command_line[i] == (uint8_t)0))
        {
          /* Exit the loop if character at position i is a space or the end of string */
          break;
        }
      }

      /* Length of the found word is i */
      cmd_size = i;

      if (memcmp((CRC_CHAR_t *)"reset", (CRC_CHAR_t *)command_line, cmd_size) == 0)
      {
        /* Command "reset" found, request a board reset --------------------------------------------------------------*/
        CMD_BoardReset();
      }
      else if (i != CMD_MAX_LINE_SIZE)
      {
        /* not an empty line / the end of the line */
        for (i = 0U; i < CMD_NbCmd ; i++)
        {
          /* Search for a registered cmd command */
          if (memcmp((CRC_CHAR_t *)CMD_a_cmd_list[i].CmdName, (CRC_CHAR_t *)command_line, cmd_size) == 0)
          {
            /* Command found => call processing  */
            PRINT_FORCE("\r\n")
            (void)CMD_a_cmd_list[i].CmdHandler((uint8_t *)command_line);
            break;
          }
        }
        if (i >= CMD_NbCmd)
        {
          /* unknown command   */
          PRINT_FORCE("\r\nCMD : unknown command : %s\r\n", command_line)
          CMD_Help((uint8_t *)command_line);
        }
      }
      else
      {
        __NOP(); /* Nothing to do */
      }
    }
    else
    {
      PRINT_FORCE("\r\n")
    }
    PRINT_FORCE("%s", (CRC_CHAR_t *)cmd_prompt)
  }
}

/**
  * @brief  Board reset command management
  * @param  -
  * @retval -
  */
static void CMD_BoardReset(void)
{
  PRINT_FORCE("Board reset requested !\r\n");
  (void)rtosalDelay(1000); /* Let time to display the message */

  NVIC_SystemReset();
  /* NVIC_SystemReset never return  */
}

/**
  * @brief help command management
  * @note  display all recorded component (component command header and description)
  * @param  p_cmd (unused)
  * @retval -
  */
static void CMD_Help(uint8_t *p_cmd)
{
  /* Local variables */
  uint32_t i;
  uint32_t align_offset;
  uint32_t cmd_size;

  UNUSED(p_cmd);

  PRINT_FORCE("***** help *****\r\n");

  PRINT_FORCE("\r\nList of commands\r\n")
  PRINT_FORCE("----------------\r\n")
  uint8_t   CMD_CmdAlignOffsetString[CMD_COMMAND_ALIGN_COLUMN];

  /* display registered commands  */
  for (i = 0U; i < CMD_NbCmd ; i++)
  {
    cmd_size = (uint32_t)crs_strlen(CMD_a_cmd_list[i].CmdName);
    align_offset = CMD_COMMAND_ALIGN_COLUMN - cmd_size;
    if ((align_offset < CMD_COMMAND_ALIGN_COLUMN))
    {
      /* alignment of the component descriptions */
      (void)memset(CMD_CmdAlignOffsetString, (int32_t)' ', align_offset);
      CMD_CmdAlignOffsetString[align_offset] = 0U;
    }
    PRINT_FORCE("%s%s %s\r\n", CMD_a_cmd_list[i].CmdName, CMD_CmdAlignOffsetString, CMD_a_cmd_list[i].CmdLabel);
  }

  /* display general syntax of the commands */
  PRINT_FORCE("\r\nHelp syntax\r\n");
  PRINT_FORCE("-----------\r\n");
  PRINT_FORCE("warning: case sensitive commands\r\n");
  PRINT_FORCE("[optional parameter]\r\n");
  PRINT_FORCE("<parameter value>\r\n");
  PRINT_FORCE("<val_1>|<val_2>|...|<val_n>: parameter value list\r\n");
  PRINT_FORCE("(command description)\r\n");
  PRINT_FORCE("return key: last command re-execution\r\n");
  PRINT_FORCE("#: comment line\r\n");
  PRINT_FORCE("\r\nAdvice\r\n");
  PRINT_FORCE("-----------\r\n");
  PRINT_FORCE("to use commands it is advised to use one of the following command to disable traces\r\n");
  PRINT_FORCE("trace off (allows disable all traces)\r\n");
  PRINT_FORCE("cst polling off  (allows to disable modem polling and avoid to display uncomfortable modem traces\r\n");
  PRINT_FORCE("\r\n");
}


/* -------------------------*/
/* External functions       */
/* -------------------------*/
/**
  * @brief  get an integer value from the argument
  * @param  string_p   (IN) acscii value to convert
  * @param  value_p    (OUT) converted uint32_t value
  * @retval return value
  */
uint32_t CMD_GetValue(uint8_t *string_p, uint32_t *value_p)
{
  uint32_t ret;
  uint8_t digit8;
  uint32_t digit;
  ret = 0U;

  if (string_p == NULL)
  {
    /* If Input parameter is null return error and output value set to zero */
    ret = 1U;
    *value_p = 0U;
  }
  else
  {
    if (memcmp((CRC_CHAR_t *)string_p, "0x", 2U) == 0)
    {
      /* Hexa value found, convert it to decimal value */
      *value_p = (uint32_t)crs_atoi_hex(&string_p[2]);
    }
    else
    {
      /* Convert string to integer */
      digit8 = (*string_p - (uint8_t)'0');
      digit  = (uint32_t)digit8;
      if (digit <= 9U)
      {
        /* Input is in the range ['0'-'9'], conversion get a valid value */
        *value_p = (uint32_t)crs_atoi(string_p);
      }
      else
      {
        /* Conversion get an invalid value, Input is not a numeric value between "0" and "9". Return error */
        ret = 1U;
        *value_p = 0U;
      }
    }
  }
  return ret;
}

/**
  * @brief register a component
  * @param  cmd_name_p     command header of the component
  * @param  cmd_handler    callback of the component to manage the command
  * @param  cmd_label_p    description of the component to display at the help  command
  * @retval -
  */
void CMD_Declare(uint8_t *cmd_name_p, CMD_HandlerCmd cmd_handler, uint8_t *cmd_label_p)
{
  if (CMD_NbCmd < CMD_MAX_CMD)
  {
    /* Max number of components not reach. Register the new component*/
    CMD_a_cmd_list[CMD_NbCmd].CmdName    = cmd_name_p;
    CMD_a_cmd_list[CMD_NbCmd].CmdLabel   = cmd_label_p;
    CMD_a_cmd_list[CMD_NbCmd].CmdHandler = cmd_handler;
    /* Increment the number of components actually registered */
    CMD_NbCmd++;
  }
  else
  {
    /* too many recorded components */
    ERROR_Handler(DBG_CHAN_UTILITIES, 10, ERROR_WARNING);
  }
}

/**
  * @brief console UART receive IT Callback
  * @param  uart_handle_p       console UART handle
  * @retval -
  */
void CMD_RxCpltCallback(UART_HandleTypeDef *uart_handle_p)
{
  static UART_HandleTypeDef *CMD_CurrentUart;

  CMD_CurrentUart = uart_handle_p;
  uint8_t rec_char;
  uint8_t *temp;

  /* store the received char */
  rec_char = CMD_ReceivedChar;

  /* rearm the IT receive for the next char */
  if (HAL_UART_Receive_IT(CMD_CurrentUart, (uint8_t *)&CMD_ReceivedChar, 1U) != HAL_OK)
  {
    __NOP(); /* Nothing to do */
  }

  /* ignore '\n' char */
  if (rec_char != (uint8_t)'\n')
  {
    if ((rec_char == (uint8_t)'\r') || (CMD_CurrentPos >= (CMD_MAX_LINE_SIZE - 1U)))
    {
      /* end of line reached or end of buffer : switch between received buffer and receiving buffer */
      CMD_current_rcv_line[CMD_CurrentPos] = 0;
      temp = CMD_completed_line;
      CMD_completed_line = CMD_current_rcv_line;
      CMD_current_cmd    = CMD_completed_line;
      CMD_current_rcv_line = temp;
      CMD_CurrentPos = 0;
      (void)rtosalSemaphoreRelease(CMD_rcvSemaphore);
    }
    else
    {
      /* not the end of line */
      if (rec_char == (uint8_t)'\b')
      {
        /* back space */
        if (CMD_CurrentPos > 0U)
        {
          /* remove the last char received only if the receiving buffer is not empty */
          CMD_CurrentPos--;
        }
      }
      else
      {
        /* normal char  */
        CMD_current_rcv_line[CMD_CurrentPos] = rec_char;
        CMD_CurrentPos++;
      }
    }
  }
}


/**
  * @brief display component help
  * @param  label   component description
  * @retval -
  */
void CMD_print_help(uint8_t *label)
{
  PRINT_FORCE("***** %s help *****\r\n", label);
}


/**
  * @brief  module initialization
  * @param  -
  * @retval -
  */
void CMD_init(void)
{
  CMD_NbCmd           = 0U;

  CMD_CommandLine[0][0] = 0;
  CMD_CommandLine[1][0] = 0;
  CMD_current_rcv_line  = CMD_CommandLine[0];
  CMD_current_cmd       = CMD_CommandLine[1];
  CMD_completed_line    = CMD_CommandLine[1];
  CMD_CurrentPos        = 0;

  CMD_Declare((uint8_t *)"help", CMD_Help, (uint8_t *)"help command");

  CMD_LastCommandLine[0] = 0;

  CMD_rcvSemaphore = rtosalSemaphoreNew((const rtosal_char_t *)"CMD_SEM_CMD_COMPLETED", 1);
}

/**
  * @brief  module start
  * @param  -
  * @retval -
  */
void CMD_start(void)
{
  static osThreadId CMD_ThreadId;
  HAL_StatusTypeDef ret;

  CMD_CommandLine[0][0] = 0;
  CMD_CommandLine[1][0] = 0;

  /* Get the semaphore to avoid function CMD_GetLine to start analyzing an input command that won't exist */
  (void)rtosalSemaphoreAcquire(CMD_rcvSemaphore, RTOSAL_WAIT_FOREVER);

  CMD_ThreadId = rtosalThreadNew((const rtosal_char_t *)"Cmd", CMD_thread, CMD_THREAD_PRIO, CMD_THREAD_STACK_SIZE,
                                 NULL);
  if (CMD_ThreadId == NULL)
  {
    ERROR_Handler(DBG_CHAN_UTILITIES, 2, ERROR_FATAL);
  }

  for (;;)
  {
    ret = HAL_UART_Receive_IT(&TRACE_INTERFACE_UART_HANDLE, &CMD_ReceivedChar, 1U);
    if (ret == HAL_OK)
    {
      break;
    }
    (void)rtosalDelay(10);
  }
}

#endif  /* USE_CMD_CONSOLE */
