/**
  ******************************************************************************
  * @file    stm32l462e_cell1_qspi_onboard.c
  * @author  MCD Application Team
  * @brief   This file includes a standard driver for the MT25QU512ABB QSPI
  *          flash memory mounted on STM32L462E_CELL1 board.
  * @note    Two QSPI modules are mounted on the board, they are exclusive.
  *          The MT25QU512ABB is know as "onboard QSPI" and is not active by default.
  *          It is activated when:
  *           - SB28 is ON
  *           - SB43 is OFF
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

/* Includes ------------------------------------------------------------------*/
#include "stm32l462e_cell1_qspi_onboard.h"
#include "stm32l462e_cell1_qspi.h"

#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 0)
#include "quadspi.h"
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 0) */

/** @addtogroup BSP BSP
  * @{
  */

/** @addtogroup STM32L462E_CELL1 STM32L462E_CELL1
  * @{
  */

/** @addtogroup STM32L462E_CELL1_QSPI_ONBOARD STM32L462E_CELL1 QSPI_ONBOARD
  * @{
  */

/** @defgroup STM32L462E_CELL1_QSPI_ONBOARD_Private_Private_Variables
  *              STM32L462E_CELL1 QSPI_ONBOARD Private Variables
  * @{
  */
/* default QSPI configuration for this board */
static QSPI_Ctx_t QSPICtx =
{
  .IsInitialized = BSP_QSPI_ACCESS_NONE,
  .InterfaceMode = BSP_QSPI_QPI_MODE,
  .TransferRate = BSP_QSPI_DTR_TRANSFER,
  .DualFlashMode = BSP_QSPI_DUALFLASH_DISABLE,
};
/**
  * @}
  */

/** @defgroup STM32L462E_CELL1_QSPI_ONBOARD_Private_Functions_Prototypes
  *              STM32L462E_CELL1 QSPI_ONBOARD Functions Prototypes
  * @{
  */
static int32_t QSPI_CheckID(void);
static int32_t QSPI_ResetMemory(void);
static int32_t QSPI_SetODS(void);
static int32_t QSPI_EnterDPIMode(void);
static int32_t QSPI_ExitDPIMode(void);
static int32_t QSPI_ConfigFlash(BSP_QSPI_Interface_t Mode, BSP_QSPI_Transfer_t Rate);

static MT25QL512ABB_Interface_t convert_InterfaceMode(BSP_QSPI_Interface_t inMode);
static MT25QL512ABB_DualFlash_t convert_DualFlashMode(BSP_QSPI_DualFlash_t inMode);
static MT25QL512ABB_Erase_t convert_EraseBlockSize(BSP_QSPI_Erase_t inSize);
/**
  * @}
  */

/** @defgroup STM32L462E_CELL1_QSPI_ONBOARD_Exported_Functions STM32L462E_CELL1 QSPI_ONBOARD Exported Functions
  * @{
  */

/**
  * @brief  Initializes the QSPI interface.
  * @retval BSP status
  */
int32_t BSP_QSPI_Init(void)
{
  int32_t ret;
  MT25QL512ABB_Info_t pInfo;
  uint32_t qspi_flashSize;

  /* Check if instance is already initialized */
  if (QSPICtx.IsInitialized == BSP_QSPI_ACCESS_NONE)
  {
#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
    /* Register the QSPI MSP Callbacks */
    if (BSP_QSPI_RegisterDefaultMspCallbacks() != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
#endif /* USE_HAL_QSPI_REGISTER_CALLBACKS */

      /* Get Flash information of one memory */
      (void)MT25QL512ABB_GetFlashInfo(&pInfo);

      /* set Flash Size */
      qspi_flashSize = (QSPICtx.DualFlashMode ==
                        BSP_QSPI_DUALFLASH_ENABLE) ?
                       (uint32_t)POSITION_VAL((uint32_t)pInfo.FlashSize) :
                       ((uint32_t)POSITION_VAL((uint32_t)pInfo.FlashSize) - 1U);

      /* STM32 QSPI interface initialization in dual flash mode enabled */
      if (BSP_QSPI_Configuration(&hqspi, qspi_flashSize) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      /* QSPI memory reset */
      else if (QSPI_ResetMemory() != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      /* check QSPI Id check */
      else if (QSPI_CheckID() != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        /* STM32 QSPI interface initialization in requested dual flash mode */
        if (BSP_QSPI_Configuration(&hqspi, qspi_flashSize) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else if (MT25QL512ABB_AutoPollingMemReady(&hqspi,
                                                  convert_InterfaceMode(QSPICtx.InterfaceMode),
                                                  convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }/* Force Flash enter 4 Byte address mode */
        else if (MT25QL512ABB_Enter4BytesAddressMode(&hqspi,
                                                     convert_InterfaceMode(QSPICtx.InterfaceMode)) != MT25QL512ABB_OK)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }/* Configuration of the Output driver strength on memory side */
        else if (QSPI_SetODS() != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }/* Configure Flash to desired mode */
        else if (QSPI_ConfigFlash(QSPICtx.InterfaceMode, QSPICtx.TransferRate) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
          ret = BSP_ERROR_NONE;
        }
      }
#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
    }
#endif /* USE_HAL_QSPI_REGISTER_CALLBACKS */
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  De-Initializes the QSPI interface.
  * @retval BSP status
  */
int32_t BSP_QSPI_DeInit(void)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Disable Memory mapped mode */
  if (QSPICtx.IsInitialized == BSP_QSPI_ACCESS_MMP)
  {
    if (BSP_QSPI_DisableMemoryMappedMode() != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  if (ret == BSP_ERROR_NONE)
  {
    /* Set default QSPICtx values */
    QSPICtx.IsInitialized = BSP_QSPI_ACCESS_NONE;
    QSPICtx.InterfaceMode = BSP_QSPI_SPI_MODE;
    QSPICtx.TransferRate  = BSP_QSPI_STR_TRANSFER;
    QSPICtx.DualFlashMode = BSP_QSPI_DUALFLASH_DISABLE;

    /* Msp QSPI deinitialization done by weak HAL_QSPI_MspDeInit function
    *  generated by STM32CubeMX tool.
    */

    /* Call the DeInit function to reset the driver */
    if (HAL_QSPI_DeInit(&hqspi) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Reads an amount of data from the QSPI memory.
  * @param  pData     Pointer to data to be read
  * @param  ReadAddr  Read start address
  * @param  Size      Size of data to read
  * @retval BSP status
  */
int32_t BSP_QSPI_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  int32_t ret;

  if (QSPICtx.TransferRate == BSP_QSPI_STR_TRANSFER)
  {
    if (MT25QL512ABB_ReadSTR(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                             MT25QL512ABB_4BYTES_SIZE, pData, ReadAddr, Size) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }
  else
  {
    if (MT25QL512ABB_ReadDTR(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode), MT25QL512ABB_4BYTES_SIZE, pData,
                             ReadAddr, Size) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Writes an amount of data to the QSPI memory.
  * @param  pData      Pointer to data to be written
  * @param  WriteAddr  Write start address
  * @param  Size       Size of data to write
  * @retval BSP status
  */
int32_t BSP_QSPI_Write(uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t end_addr;
  uint32_t current_size;
  uint32_t current_addr;
  uint32_t data_addr;

  /* Calculation of the size between the write address and the end of the page */
  current_size = MT25QL512ABB_PAGE_SIZE - (WriteAddr % MT25QL512ABB_PAGE_SIZE);

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > Size)
  {
    current_size = Size;
  }

  /* Initialize the address variables */
  current_addr = WriteAddr;
  end_addr = WriteAddr + Size;
  data_addr = (uint32_t)pData;

  /* Perform the write page by page */
  do
  {
    /* Check if Flash busy ? */
    if (MT25QL512ABB_AutoPollingMemReady(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                         convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Enable write operations */
    else if (MT25QL512ABB_WriteEnable(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                      convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Issue page program command */
    else if (MT25QL512ABB_PageProgram(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode), MT25QL512ABB_4BYTES_SIZE,
                                      (uint8_t *)data_addr, current_addr, current_size) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Configure automatic polling mode to wait for end of program */
    else if (MT25QL512ABB_AutoPollingMemReady(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                              convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Update the address and size variables for next page programming */
      current_addr += current_size;
      data_addr += current_size;
      current_size = ((current_addr + MT25QL512ABB_PAGE_SIZE) > end_addr) ?
                     (end_addr - current_addr) : MT25QL512ABB_PAGE_SIZE;
    }
  } while ((current_addr < end_addr) && (ret == BSP_ERROR_NONE));

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Erases the specified block of the QSPI memory.
  *         MT25QL512ABB supports 4K, 32K, 64K size block erase commands.
  * @param  BlockAddress Block address to erase
  * @param  BlockSize    Erase Block size
  * @retval BSP status
  */
int32_t BSP_QSPI_EraseBlock(uint32_t BlockAddress, BSP_QSPI_Erase_t BlockSize)
{
  int32_t ret;

  /* Check Flash busy ? */
  if (MT25QL512ABB_AutoPollingMemReady(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                       convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  /* Enable write operations */
  else if (MT25QL512ABB_WriteEnable(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                    convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  /* Issue Block Erase command */
  else if (MT25QL512ABB_BlockErase(&hqspi,
                                   convert_InterfaceMode(QSPICtx.InterfaceMode),
                                   MT25QL512ABB_4BYTES_SIZE,
                                   BlockAddress,
                                   convert_EraseBlockSize(BlockSize)) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    while (BSP_QSPI_GetStatus() == BSP_ERROR_BUSY) {}
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Erases the entire QSPI memory.
  * @retval BSP status
  */
int32_t BSP_QSPI_EraseChip(void)
{
  int32_t ret;

  /* Check Flash busy ? */
  if (MT25QL512ABB_AutoPollingMemReady(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                       convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }/* Enable write operations */
  else if (MT25QL512ABB_WriteEnable(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                    convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }/* Issue Chip erase command */
  else if (MT25QL512ABB_ChipErase(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode)) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    while (BSP_QSPI_GetStatus() == BSP_ERROR_BUSY) {}
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Reads current status of the QSPI memory.
  *         If WIP != 0 then return busy.
  * @retval QSPI memory status: whether busy or not
  */
int32_t BSP_QSPI_GetStatus(void)
{
  uint8_t reg[2];
  int32_t ret;

  if (MT25QL512ABB_ReadStatusRegister(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                      convert_DualFlashMode(QSPICtx.DualFlashMode), reg) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }/* Check the value of the register */
  else if ((reg[0] & MT25QL512ABB_SR_WIP) != 0U)
  {
    ret = BSP_ERROR_BUSY;
  }
  else if ((QSPICtx.DualFlashMode == BSP_QSPI_DUALFLASH_ENABLE) && ((reg[1] & MT25QL512ABB_SR_WIP) != 0U))
  {
    ret = BSP_ERROR_BUSY;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Return the configuration of the QSPI memory.
  * @param  pInfo  pointer on the configuration structure
  * @retval BSP status
  */
int32_t BSP_QSPI_GetInfo(BSP_QSPI_Info_t *pInfo)
{
  int32_t ret = BSP_ERROR_NONE;
  MT25QL512ABB_Info_t pDriverInfo;

  (void)MT25QL512ABB_GetFlashInfo(&pDriverInfo);
  pInfo->FlashSize          = pDriverInfo.FlashSize;
  pInfo->EraseSectorSize    = pDriverInfo.EraseSectorSize;
  pInfo->EraseSectorsNumber = pDriverInfo.EraseSectorsNumber;
  pInfo->ProgPageSize       = pDriverInfo.ProgPageSize;
  pInfo->ProgPagesNumber    = pDriverInfo.ProgPagesNumber;

  /* Return BSP status */
  return ret;
}

/**
  * @brief  In dual flash mode disabled, Get flash ID 3 Bytes:
  *         Manufacturer ID, Memory type, Memory density
  *         In dual flash mode enabled, Get 2 set of flash ID 3 bytes:
  *         Manufacturer ID 1, Manufacturer ID 2, Memory type 1, Memory type 2,
  *         Memory density 1, Memory density 2
  * @param  Id Pointer to flash ID bytes
  * @retval BSP status
  */
int32_t BSP_QSPI_ReadID(uint8_t *Id)
{
  int32_t ret;

  if (MT25QL512ABB_ReadID(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode), Id,
                          convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Configure the QSPI in memory-mapped mode
  *         Only 1 Instance can running MMP mode. And it will lock system at this mode.
  * @retval BSP status
  */
int32_t BSP_QSPI_EnableMemoryMappedMode(void)
{
  int32_t ret = BSP_ERROR_NONE;

  if (QSPICtx.TransferRate == BSP_QSPI_STR_TRANSFER)
  {
    if (MT25QL512ABB_EnableMemoryMappedModeSTR(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                               MT25QL512ABB_4BYTES_SIZE) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else /* Update QSPI context if all operations are well done */
    {
      QSPICtx.IsInitialized = BSP_QSPI_ACCESS_MMP;
    }
  }
  else
  {
    if (MT25QL512ABB_EnableMemoryMappedModeDTR(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                               MT25QL512ABB_4BYTES_SIZE) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else /* Update QSPI context if all operations are well done */
    {
      QSPICtx.IsInitialized = BSP_QSPI_ACCESS_MMP;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Exit from memory-mapped mode
  *         Only 1 Instance can running MMP mode. And it will lock system at this mode.
  * @retval BSP status
  */
int32_t BSP_QSPI_DisableMemoryMappedMode(void)
{
  int32_t ret = BSP_ERROR_NONE;

  if (QSPICtx.IsInitialized != BSP_QSPI_ACCESS_MMP)
  {
    ret = BSP_ERROR_QSPI_MMP_UNLOCK_FAILURE;
  }/* Abort MMP back to indirect mode */
  else if (HAL_QSPI_Abort(&hqspi) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else /* Update QSPI context if all operations are well done */
  {
    QSPICtx.IsInitialized = BSP_QSPI_ACCESS_INDIRECT;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @}
  */

/** @defgroup STM32L462E_CELL1_QSPI_ONBOARD_Private_Functions STM32L462E_CELL1 QSPI_ONBOARD Private Functions
  * @{
  */

/**
  * @brief  In dual flash mode disabled, Get flash ID 3 Bytes:
  *         Manufacturer ID, Memory type, Memory density
  *         In dual flash mode enabled, Get 2 set of flash ID 3 bytes:
  *         Manufacturer ID 1, Manufacturer ID 2, Memory type 1, Memory type 2,
  *         Memory density 1, Memory density 2
  * @param  Id Pointer to flash ID bytes
  * @retval BSP status
  */
static int32_t QSPI_CheckID(void)
{
  int32_t ret;
  uint8_t id[6] = { 0U };

  if (MT25QL512ABB_ReadID(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode), id,
                          convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    /* In dual flash mode disabled, Get flash ID 3 Bytes:
    * Manufacturer ID, Memory type, Memory density
    * In dual flash mode enabled, Get 2 set of flash ID 3 bytes:
    * Manufacturer ID 1, Manufacturer ID 2, Memory type 1, Memory type 2,
    * Memory density 1, Memory density 2
    * Manufacturer ID = 20h
    * Memory type =  BAh = 3V, BBh = 1.8V
    * Memory density = 22h = 2Gb, 21h = 1Gb, 20h = 512Mb, 19h = 256Mb, 18h = 128Mb, 17h = 64Mb
    */
    if ((id[0] == 0x20U) && (id[1] == 0xBBU) && (id[2] == 0x20U))
    {
      ret = BSP_ERROR_NONE;
    }
    else
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  This function reset the QSPI Flash memory.
  *         Force QPI+DPI+SPI reset to avoid system come from unknown status.
  *         Flash accept SPI commands after reset.
  * @retval BSP status
  */
static int32_t QSPI_ResetMemory(void)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Send RESET enable command in QPI mode (QUAD I/Os, 4-0-0) */
  if (MT25QL512ABB_ResetEnable(&hqspi, MT25QL512ABB_QPI_MODE) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }/* Send RESET memory command in QPI mode (QUAD I/Os, 4-0-0) */
  else if (MT25QL512ABB_ResetMemory(&hqspi, MT25QL512ABB_QPI_MODE) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }/* Send RESET enable command in DPI mode (2-0-0) */
  else if (MT25QL512ABB_ResetEnable(&hqspi, MT25QL512ABB_DPI_MODE) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }/* Send RESET memory command in DPI mode (2-0-0) */
  else if (MT25QL512ABB_ResetMemory(&hqspi, MT25QL512ABB_DPI_MODE) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }/* Send RESET enable command in SPI mode (1-0-0) */
  else if (MT25QL512ABB_ResetEnable(&hqspi, MT25QL512ABB_SPI_MODE) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }/* Send RESET memory command in SPI mode (1-0-0) */
  else if (MT25QL512ABB_ResetMemory(&hqspi, MT25QL512ABB_SPI_MODE) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    QSPICtx.IsInitialized = BSP_QSPI_ACCESS_INDIRECT;      /* After reset S/W setting to indirect access   */
    QSPICtx.InterfaceMode = BSP_QSPI_SPI_MODE;         /* After reset H/W back to SPI mode by default  */
    QSPICtx.TransferRate  = BSP_QSPI_STR_TRANSFER;     /* After reset S/W setting to STR mode          */

    /* After SWreset CMD, wait in case SWReset occurred during erase operation */
    HAL_Delay(MT25QL512ABB_RESET_MAX_TIME);
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  This function configure the Output Driver Strength on memory side.
  *         ODS bit locate in Enhanced Volatile Configuration Register[2:0]
  * @retval BSP status
  */
static int32_t QSPI_SetODS()
{
  int32_t ret;
  uint8_t reg[2];  /** reg[0] contains reg value for FLASHID_1,
                    * reg[1] contains reg value for FLASHID_2 in case of dual flash mode
                    */

  if (MT25QL512ABB_ReadEnhancedVolCfgRegister(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                              convert_DualFlashMode(QSPICtx.DualFlashMode), reg) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    /* Set Output Strength of the QSPI memory as CONF_QSPI_ODS variable ohms */
    MODIFY_REG(reg[0], MT25QL512ABB_EVCR_ODS,
               (((uint8_t)CONF_QSPI_ODS) << (POSITION_VAL(MT25QL512ABB_EVCR_ODS) & 0x7U)));
    MODIFY_REG(reg[1], MT25QL512ABB_EVCR_ODS,
               (((uint8_t)CONF_QSPI_ODS) << (POSITION_VAL(MT25QL512ABB_EVCR_ODS) & 0x7U)));

    /* Enable write operations */
    if (MT25QL512ABB_WriteEnable(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                 convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Issue Write command */
    else if (MT25QL512ABB_WriteEnhancedVolCfgRegister(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                                      convert_DualFlashMode(QSPICtx.DualFlashMode),
                                                      reg) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Configure automatic polling mode to wait for end of program */
    else if (MT25QL512ABB_AutoPollingMemReady(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                              convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  This function put QSPI memory in DPI mode (Dual I/O) from SPI mode.
  * @retval BSP status
  */
static int32_t QSPI_EnterDPIMode(void)
{
  int32_t ret;
  uint8_t reg[2];  /* reg[0] contains reg value for FLASHID_1,
                      reg[1] contains reg value for FLASHID_2 in case of dual flash mode */

  if (MT25QL512ABB_ReadEnhancedVolCfgRegister(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                              convert_DualFlashMode(QSPICtx.DualFlashMode), reg) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    /* Set Dual I/O protocol */
    CLEAR_BIT(reg[0], MT25QL512ABB_EVCR_DPI);
    CLEAR_BIT(reg[1], MT25QL512ABB_EVCR_DPI);

    /* Enable write operations */
    if (MT25QL512ABB_WriteEnable(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                 convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Issue Write command */
    else if (MT25QL512ABB_WriteEnhancedVolCfgRegister(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                                      convert_DualFlashMode(QSPICtx.DualFlashMode),
                                                      reg) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Configure automatic polling mode to wait for end of program */
    else if (MT25QL512ABB_AutoPollingMemReady(&hqspi, MT25QL512ABB_DPI_MODE,
                                              convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  This function put QSPI memory in SPI mode from DPI mode (Dual I/O).
  * @retval BSP status
  */
static int32_t QSPI_ExitDPIMode(void)
{
  int32_t ret;
  uint8_t reg[2];  /** reg[0] contains reg value for FLASHID_1,
                    * reg[1] contains reg value for FLASHID_2 in case of dual flash mode
                    */

  if (MT25QL512ABB_ReadEnhancedVolCfgRegister(&hqspi, MT25QL512ABB_DPI_MODE,
                                              convert_DualFlashMode(QSPICtx.DualFlashMode),
                                              reg) != MT25QL512ABB_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    /* Disable Dual I/O protocol */
    SET_BIT(reg[0], MT25QL512ABB_EVCR_DPI);
    SET_BIT(reg[1], MT25QL512ABB_EVCR_DPI);

    /* Enable write operations */
    if (MT25QL512ABB_WriteEnable(&hqspi, MT25QL512ABB_DPI_MODE,
                                 convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Issue Write command */
    else if (MT25QL512ABB_WriteEnhancedVolCfgRegister(&hqspi, MT25QL512ABB_DPI_MODE,
                                                      convert_DualFlashMode(QSPICtx.DualFlashMode),
                                                      reg) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Configure automatic polling mode to wait for end of program */
    else if (MT25QL512ABB_AutoPollingMemReady(&hqspi, convert_InterfaceMode(QSPICtx.InterfaceMode),
                                              convert_DualFlashMode(QSPICtx.DualFlashMode)) != MT25QL512ABB_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      ret = BSP_ERROR_NONE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set Flash to desired Interface mode. And this instance becomes current instance.
  *         If current instance running at MMP mode then this function doesn't work.
  *         Indirect -> Indirect
  * @param  Mode      QSPI mode
  * @param  Rate      QSPI transfer rate
  * @retval BSP status
  */
static int32_t QSPI_ConfigFlash(BSP_QSPI_Interface_t Mode, BSP_QSPI_Transfer_t Rate)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if MMP mode locked ************************************************/
  if (QSPICtx.IsInitialized == BSP_QSPI_ACCESS_MMP)
  {
    ret = BSP_ERROR_QSPI_MMP_LOCK_FAILURE;
  }
  else
  {
    /* Setup Flash interface ***************************************************/
    switch (QSPICtx.InterfaceMode)
    {
      case BSP_QSPI_QPI_MODE :               /* 4-4-4 commands */
        if (Mode != BSP_QSPI_QPI_MODE)
        {
          /* Exit Quad I/O protocol mode */
          if (MT25QL512ABB_ExitQPIMode(&hqspi) != MT25QL512ABB_OK)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }
          else if (Mode == BSP_QSPI_DPI_MODE)
          {
            /* Enter Dual I/O protocol mode */
            ret = QSPI_EnterDPIMode();
          }
          else
          {
            /* Nothing to do */
          }
        }
        break;

      case BSP_QSPI_DPI_MODE :               /* 2-2-2 commands */
        if (Mode != BSP_QSPI_DPI_MODE)
        {
          /* Exit Dual I/O protocol mode */
          if (QSPI_ExitDPIMode() != BSP_ERROR_NONE)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }
          else if (Mode == BSP_QSPI_QPI_MODE)
          {
            /* Enter Quad I/O protocol mode */
            if (MT25QL512ABB_EnterQPIMode(&hqspi) != MT25QL512ABB_OK)
            {
              ret = BSP_ERROR_COMPONENT_FAILURE;
            }
          }
          else
          {
            /* Nothing to do */
          }
        }
        break;

      case BSP_QSPI_SPI_MODE :               /* 1-1-1 commands, Power on H/W default setting */
      case BSP_QSPI_SPI_1I2O_MODE :          /* 1-1-2 commands */
      case BSP_QSPI_SPI_2IO_MODE :           /* 1-2-2 commands */
      case BSP_QSPI_SPI_1I4O_MODE :          /* 1-1-4 commands */
      case BSP_QSPI_SPI_4IO_MODE :           /* 1-4-4 commands */
      default :
        if (Mode == BSP_QSPI_QPI_MODE)
        {
          /* Enter Quad I/O protocol mode */
          if (MT25QL512ABB_EnterQPIMode(&hqspi) != MT25QL512ABB_OK)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }
        }
        else if (Mode == BSP_QSPI_DPI_MODE)
        {
          /* Enter Dual I/O protocol mode */
          ret = QSPI_EnterDPIMode();
        }
        else
        {
          /* Nothing to do */
        }
        break;
    }

    /* Update QSPI context if all operations are well done */
    if (ret == BSP_ERROR_NONE)
    {
      /* Update current status parameter *****************************************/
      QSPICtx.IsInitialized = BSP_QSPI_ACCESS_INDIRECT;
      QSPICtx.InterfaceMode = Mode;
      QSPICtx.TransferRate  = Rate;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Convert BSP interface mode to MT25QL512ABB driver interface mode
  * @param  inMode  BSP QSPI interface mode
  * @retval converted interface mode value for MT25QL512ABB driver
  */
static MT25QL512ABB_Interface_t convert_InterfaceMode(BSP_QSPI_Interface_t inMode)
{
  MT25QL512ABB_Interface_t outMode;

  if (inMode == BSP_QSPI_QPI_MODE)
  {
    outMode = MT25QL512ABB_QPI_MODE;
  }
  else if (inMode == BSP_QSPI_DPI_MODE)
  {
    outMode = MT25QL512ABB_DPI_MODE;
  }
  else if (inMode == BSP_QSPI_SPI_4IO_MODE)
  {
    outMode = MT25QL512ABB_SPI_4IO_MODE;
  }
  else if (inMode == BSP_QSPI_SPI_1I4O_MODE)
  {
    outMode = MT25QL512ABB_SPI_1I4O_MODE;
  }
  else if (inMode == BSP_QSPI_SPI_2IO_MODE)
  {
    outMode = MT25QL512ABB_SPI_2IO_MODE;
  }
  else if (inMode == BSP_QSPI_SPI_1I2O_MODE)
  {
    outMode = MT25QL512ABB_SPI_1I2O_MODE;
  }
  else
  {
    outMode = MT25QL512ABB_SPI_MODE;
  }

  return (outMode);
}

/**
  * @brief  Convert BSP DualFlash mode to MT25QL512ABB driver DualFlash mode
  * @param  inMode  BSP QSPI DualFlash mode
  * @retval converted DualFlash mode value for MT25QL512ABB driver
  */
static MT25QL512ABB_DualFlash_t convert_DualFlashMode(BSP_QSPI_DualFlash_t inMode)
{
  MT25QL512ABB_DualFlash_t outMode;

  if (inMode == BSP_QSPI_DUALFLASH_ENABLE)
  {
    outMode = MT25QL512ABB_DUALFLASH_ENABLE;
  }
  else
  {
    outMode = MT25QL512ABB_DUALFLASH_DISABLE;
  }

  return (outMode);
}

/**
  * @brief  Convert BSP Erase mode to MT25QL512ABB driver Erase mode
  * @param  inMode  BSP QSPI Erase mode
  * @retval converted Erase mode value for MT25QL512ABB driver
  */
static MT25QL512ABB_Erase_t convert_EraseBlockSize(BSP_QSPI_Erase_t inSize)
{
  MT25QL512ABB_Erase_t outSize;

  if (inSize == BSP_QSPI_ERASE_CHIP)
  {
    outSize = MT25QL512ABB_ERASE_BULK;
  }
  else if (inSize == BSP_QSPI_ERASE_64K)
  {
    outSize = MT25QL512ABB_ERASE_64K;
  }
  else if (inSize == BSP_QSPI_ERASE_32K)
  {
    outSize = MT25QL512ABB_ERASE_32K;
  }
  else
  {
    outSize = MT25QL512ABB_ERASE_4K;
  }

  return (outSize);
}

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

