/*
 * Copyright ##year## AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay LwM2M SDK
 * ALL RIGHTS RESERVED
 */
#include <anjay/fw_update.h>
#include <avsystem/commons/avs_log.h>

#define SFU_APP_NEW_IMAGE_C
#define SFU_FWIMG_COMMON_C

#include "flash_if.h"
#include "se_def.h"
#include "se_interface_application.h"
#include "sfu_fwimg_regions.h"
#include "stm32l462e_cell1.h"
#include "stm32l4xx_hal.h"

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#include "mapping_fwimg.h"
#include "mapping_sbsfu.h"
#elif defined(__ICCARM__) || defined(__GNUC__)
#include "mapping_export.h"
#endif /* __CC_ARM || __ARMCC_VERSION */

#include "firmware_update.h"
#include "utils.h"

static bool just_updated;
static bool update_requested;
static bool update_initialized;

static uint8_t fw_header_dwl_slot[SE_FW_HEADER_TOT_LEN];
static struct {
  uint32_t  max_size_in_bytes;     /*!< The maximum allowed size for the FwImage in User Flash (in Bytes) */
  uint32_t  download_addr;         /*!< The download address for the FwImage in UserFlash */
  uint32_t  image_offset_in_bytes; /*!< Image write starts at this offset */
  uint32_t  execution_addr;        /*!< The execution address for the FwImage in UserFlash */
} fw_image_dwl_area;
static uint32_t flash_offset;

static int fw_stream_open(void *user_ptr, const char *package_uri,
                          const struct anjay_etag *package_etag) {
  (void)user_ptr;
  (void)package_uri;
  (void)package_etag;

  /* Get Info about the download area */
  fw_image_dwl_area.download_addr = SlotStartAdd[SLOT_DWL_1];
  fw_image_dwl_area.max_size_in_bytes = (uint32_t) SLOT_SIZE(SLOT_DWL_1);
  fw_image_dwl_area.image_offset_in_bytes = SFU_IMG_IMAGE_OFFSET;

  /* Cleanup the memory for the firmware download */
  if (FLASH_If_Erase_Size((void *)(fw_image_dwl_area.download_addr),
                          fw_image_dwl_area.max_size_in_bytes) != HAL_OK) {
    avs_log(fw_update, INFO, "Init not successfull :(");

    return -1;
  }

  avs_log(fw_update, INFO, "Init successfull");

  flash_offset = 0U;
  update_initialized = true;

  return 0;
}

static int fw_stream_write(void *user_ptr, const void *data, size_t length) {
  (void) user_ptr;

  assert(update_initialized);

  if (FLASH_If_Write((void *) (fw_image_dwl_area.download_addr + flash_offset),
                     data, length) != HAL_OK) {
    return -1;
  }
  avs_log(fw_update, INFO, "Max size %lu bytes, downloaded %lu bytes.", fw_image_dwl_area.max_size_in_bytes, flash_offset);

  flash_offset += length;

  return 0;
}

static int fw_stream_finish(void *user_ptr) {
  (void)user_ptr;

  assert(update_initialized);

  /* Read header in download slot */
  (void) FLASH_If_Read(fw_header_dwl_slot,
                       (void *) fw_image_dwl_area.download_addr,
                       SE_FW_HEADER_TOT_LEN);

  update_initialized = false;

  return 0;
}

static void fw_reset(void *user_ptr) {
  (void)user_ptr;

  update_initialized = false;
}

static int fw_perform_upgrade(void *anjay) {
  (void) anjay;

/*#ifndef SFU_NO_SWAP
  ret = FLASH_If_Erase_Size((void *) SlotStartAdd[SLOT_SWAP], SFU_IMG_IMAGE_OFFSET);
  if (ret == HAL_OK) {
    ret = FLASH_If_Write((void *)SlotStartAdd[SLOT_SWAP], pfw_header, SE_FW_HEADER_TOT_LEN);
  }
#endif*/ /* !SFU_NO_SWAP */

  update_requested = true;

  return 0;
}

static const anjay_fw_update_handlers_t handlers = {
    .stream_open = fw_stream_open,
    .stream_write = fw_stream_write,
    .stream_finish = fw_stream_finish,
    .reset = fw_reset,
    .perform_upgrade = fw_perform_upgrade};

int fw_update_install(anjay_t *anjay) {
  anjay_fw_update_initial_state_t state = {0};

  if (just_updated) {
    state.result = ANJAY_FW_UPDATE_INITIAL_SUCCESS;
  }

  return anjay_fw_update_install(anjay, &handlers, anjay, &state);
}

void fw_update_apply(void) {}

bool fw_update_requested(void) { return update_requested; }

void fw_update_reboot(void) {
  avs_log(fota, INFO, "Rebooting to perform a firmware upgrade...");
  HAL_Delay(1000U);
  NVIC_SystemReset();
}
