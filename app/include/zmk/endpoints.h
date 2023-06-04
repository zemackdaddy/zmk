/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zmk/ble.h>
#include <zmk/endpoints_types.h>

#ifdef CONFIG_ZMK_USB
#define ZMK_ENDPOINT_USB_COUNT 1
#else
#define ZMK_ENDPOINT_USB_COUNT 0
#endif

#ifdef CONFIG_ZMK_BLE
#define ZMK_ENDPOINT_BLE_COUNT ZMK_BLE_PROFILE_COUNT
#else
#define ZMK_ENDPOINT_BLE_COUNT 0
#endif

/**
 * The total number of different (struct zmk_endpoint_instance) values that can
 * be selected.
 *
 * Note that this value may change between firmware versions, so it should not
 * be used in any persistent storage.
 */
#define ZMK_ENDPOINT_COUNT (ZMK_ENDPOINT_USB_COUNT + ZMK_ENDPOINT_BLE_COUNT)

bool zmk_endpoint_instance_equals(struct zmk_endpoint_instance a, struct zmk_endpoint_instance b);

/**
 * Writes a string identifying an endpoint instance.
 */
int zmk_endpoint_instance_print(char *str, size_t len, struct zmk_endpoint_instance endpoint);

/**
 * Gets a unique index for an endpoint instance. This can be used together with
 * ZMK_ENDPOINT_COUNT to manage separate state for each endpoint instance.
 *
 * Note that the index for a specific instance may change between firmware versions,
 * so it should not be used in any persistent storage.
 */
int zmk_endpoint_instance_to_index(struct zmk_endpoint_instance endpoint);

/**
 * Sets the preferred endpoint transport to use. (If the preferred endpoint is
 * not available, a different one may automatically be selected.)
 */
int zmk_endpoints_select_transport(enum zmk_transport transport);
int zmk_endpoints_toggle_transport(void);

/**
 * Gets the currently-selected endpoint.
 */
struct zmk_endpoint_instance zmk_endpoints_selected(void);

int zmk_endpoints_send_report(uint16_t usage_page);
int zmk_endpoints_send_mouse_report();
