/*
 * Copyright 2017-2021 AVSystem <avsystem@avsystem.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANJAY_CLIENT_CONFIG_GENERATED_H
#define ANJAY_CLIENT_CONFIG_GENERATED_H

/**
 * Anjay client configuration.
 *
 * This file is not intended to be edited manually. Configuration should be
 * changed using STM32CubeMX.
 */

/* Client config PSK Identity */
#define ANJAY_CLIENT_CONFIG_PSK_IDENTITY "anjay-freertos-demo2"

/* Client config PSK */
#define ANJAY_CLIENT_CONFIG_PSK "psk"

/* Client config Server URI */
#define ANJAY_CLIENT_CONFIG_SERVER_URI "coaps://lwm2m-test.avsystem.io:5684"

/* Client config APN Username */
#define ANJAY_CLIENT_CONFIG_APN_USERNAME "internet"

/* Client config APN Password */
#define ANJAY_CLIENT_CONFIG_APN_PASSWORD "internet"

/* Client config APN */
#define ANJAY_CLIENT_CONFIG_APN "internet"

/* Client config Endpoint Name */
#define ANJAY_CLIENT_CONFIG_ENDPOINT_NAME "anjay-freertos-demo2"

#endif // ANJAY_CLIENT_CONFIG_GENERATED_H
