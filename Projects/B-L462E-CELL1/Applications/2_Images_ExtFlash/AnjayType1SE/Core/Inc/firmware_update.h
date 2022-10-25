/*
 * Copyright ##year## AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay LwM2M SDK
 * ALL RIGHTS RESERVED
 */

#pragma once

#include <anjay/anjay.h>
#include <anjay/fw_update.h>

#include <stdbool.h>

int fw_update_install(anjay_t *anjay);
void fw_update_apply(void);
bool fw_update_requested(void);
void fw_update_reboot(void);
