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

#ifndef ANJAY_CONFIG_GENERATED_H
#define ANJAY_CONFIG_GENERATED_H

/**
 * Anjay library configuration.
 */

/* Disable use of the Deregister message */
/* #undef ANJAY_WITHOUT_DEREGISTER */

/* Maximum length supported for a single Uri-Query segment */
#define ANJAY_MAX_URI_QUERY_SEGMENT_SIZE 64

/* Enable server module */
#define ANJAY_WITH_MODULE_SERVER

/* DTLS buffer size */
#define ANJAY_DTLS_SESSION_BUFFER_SIZE 1024

/* Maximum length supported for a single Uri-Path or Location-Path segment */
#define ANJAY_MAX_URI_SEGMENT_SIZE 64

/* Enable support for the anjay_resource_observation_status() API */
#define ANJAY_WITH_OBSERVATION_STATUS

/* Enable support for OSCORE-based security for LwM2M connections */
/* #undef ANJAY_WITH_COAP_OSCORE */

/* Enable TRACE-level logs */
/* #undef ANJAY_WITH_TRACE_LOGS */

/* Enable support for Non-IP Data Delivery */
/* #undef ANJAY_WITH_NIDD */

/* Enable support for Enrollment over Secure Transport */
/* #undef ANJAY_WITH_EST */

/* Support for the LwM2M Information Reporting interface */
#define ANJAY_WITH_OBSERVE

/* Enable attr_storage module */
#define ANJAY_WITH_MODULE_ATTR_STORAGE

/* Enable Time compatibility implementation */
#define ANJAY_COMPAT_TIME

/* Enable support for CBOR and SenML CBOR formats, as specified in LwM2M TS 1.1 */
/* #undef ANJAY_WITH_CBOR */

/* Enable access_control module */
#define ANJAY_WITH_MODULE_ACCESS_CONTROL

/* Enable security module */
#define ANJAY_WITH_MODULE_SECURITY

/* Disable support for Plain Text format as specified in LwM2M TS 1.0. and 1.1 */
/* #undef ANJAY_WITHOUT_PLAINTEXT */

/* Enable support for legacy CoAP Content-Format values */
/* #undef ANJAY_WITH_LEGACY_CONTENT_FORMAT_SUPPORT */

/* Support for CoAP(S) downloads */
#define ANJAY_WITH_COAP_DOWNLOAD

/* Enable bootstrapper module */
#define ANJAY_WITH_BOOTSTRAP

/* Enable bg96_nidd module */
/* #undef ANJAY_WITH_MODULE_BG96_NIDD */

/* Maximum size of the "Public Key or Identity" */
#define ANJAY_MAX_PK_OR_IDENTITY_SIZE 256

/* Enable support for custom "con" attribute that controls Confirmable notifications */
/* #undef ANJAY_WITH_CON_ATTR */

/* Enable at_sms module */
/* #undef ANJAY_WITH_MODULE_AT_SMS */

/* Support for HTTP(S) downloads */
/* #undef ANJAY_WITH_HTTP_DOWNLOAD */

/* Support for the LwM2M Discover operation */
#define ANJAY_WITH_DISCOVER

/* Disable support for TLV format as specified in LwM2M TS 1.0 */
/* #undef ANJAY_WITHOUT_TLV */

/* Enable support for the LwM2M Send operation */
/* #undef ANJAY_WITH_SEND */

/* Enable support for features new to LwM2M protocol version 1.1 */
/* #undef ANJAY_WITH_LWM2M11 */

/* Enable bootstrapper module */
/* #undef ANJAY_WITH_MODULE_BOOTSTRAPPER */

/* Enable Net compatibility implementation */
#define ANJAY_COMPAT_NET

/* Enable support for core state persistence */
#define ANJAY_WITH_CORE_PERSISTENCE

/* Maximum length supported for stringified floating-point values */
#define ANJAY_MAX_DOUBLE_STRING_SIZE 64

/* Enable fw_update module */
#define ANJAY_WITH_MODULE_FW_UPDATE

/* Enable support for sending and receiving multipart SMS messages */
#define ANJAY_WITH_SMS_MULTIPART

/* Core support for Access Control mechanisms */
#define ANJAY_WITH_ACCESS_CONTROL

/* Maximum size in bytes supported for the "Secret Key" resource in the LwM2M Security Object */
#define ANJAY_MAX_SECRET_KEY_SIZE 128

/* Downloader feature */
#define ANJAY_WITH_DOWNLOADER

/* Enable Mbedtls compatibility implementation */
#define ANJAY_COMPAT_MBEDTLS

/* Enable support for the SMS binding and the SMS trigger mechanism */
/* #undef ANJAY_WITH_SMS */

/* Enable support for SenML JSON format, as specified in LwM2M TS 1.1 */
/* #undef ANJAY_WITH_SENML_JSON */

/* Enable oscore module */
/* #undef ANJAY_WITH_MODULE_OSCORE */

/* Disable support for "IP stickiness" */
/* #undef ANJAY_WITHOUT_IP_STICKINESS */

/* Enable logging */
#define ANJAY_WITH_LOGS

/* Enable support for measuring amount of LwM2M traffic */
/* #undef ANJAY_WITH_NET_STATS */

/* Support for JSON format as specified in LwM2M TS 1.0 */
/* #undef ANJAY_WITH_LWM2M_JSON */

#endif // ANJAY_CONFIG_GENERATED_H
