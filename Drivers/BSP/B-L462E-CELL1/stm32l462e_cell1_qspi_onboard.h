/**
  ******************************************************************************
  * @file    stm32l462e_cell1_qspi_onboard.h
  * @author  MCD Application Team
  * @brief   This file contains contains specificities to "onboard" QSPI,
  *          which is based on the mt25ql512abb component.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32L462E_CELL1_QSPI_ONBOARD_H
#define STM32L462E_CELL1_QSPI_ONBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l462e_cell1_conf.h"
#include "stm32l462e_cell1_errno.h"
#include "stm32l462e_cell1_qspi.h"

#include "mt25ql512abb.h"

/** @addtogroup BSP BSP
  * @{
  */

/** @addtogroup STM32L462E_CELL1 STM32L462E_CELL1
  * @{
  */

/** @addtogroup STM32L462E_CELL1_QSPI_ONBOARD STM32L462E_CELL1 QSPI_ONBOARD
  * @{
  */
typedef struct
{
  BSP_QSPI_Access_t    IsInitialized;  /*!<  Instance access Flash method     */
  BSP_QSPI_Interface_t InterfaceMode;  /*!<  Flash Interface mode of Instance */
  BSP_QSPI_Transfer_t  TransferRate;   /*!<  Flash Transfer mode of Instance  */
  BSP_QSPI_DualFlash_t DualFlashMode;  /*!<  DualFlash mode of Instance       */
} QSPI_Ctx_t;

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

#endif /* STM32L462E_CELL1_ONBOARD_QSPI_H */
