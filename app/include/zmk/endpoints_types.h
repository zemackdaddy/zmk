/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/**
 * The method by which data is sent.
 */
enum zmk_transport {
    ZMK_TRANSPORT_USB,
    ZMK_TRANSPORT_BLE,
};

/**
 * A specific endpoint to which data may be sent.
 */
struct zmk_endpoint_instance {
    enum zmk_transport transport;
    union {
        // ZMK_TRANSPORT_USB: no data
        int ble_profile_index; // ZMK_TRANSPORT_BLE
    };
};
