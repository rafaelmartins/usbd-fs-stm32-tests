/*
 * SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

typedef enum {
    PROTO_CMD_CONFIGURE_TEST  = 0x01,
    PROTO_CMD_GET_TEST_COUNT  = 0x02,
    PROTO_CMD_GET_TEST_RESULT = 0x03,
    PROTO_CMD_SYNC            = 0x05,
} serial_proto_cmd_t;

typedef enum {
    PROTO_REPLY_CONFIGURE_TEST_OK         = 0x00,
    PROTO_REPLY_CONFIGURE_TEST_INVALID_ID = 0x01,
    PROTO_REPLY_CONFIGURE_TEST_FAILED     = 0x02,
} serial_proto_reply_configure_test_t;

typedef enum {
    PROTO_REPLY_GET_TEST_RESULT_PASSED     = 0x00,
    PROTO_REPLY_GET_TEST_RESULT_FAILED     = 0x01,
} serial_proto_reply_get_test_result_t;

typedef enum {
    PROTO_CMD_SYNC_VAL_SYNC = 0x05,
} serial_proto_cmd_sync_t;
