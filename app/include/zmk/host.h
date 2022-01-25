/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zmk/endpoints_types.h>
#include <zmk/hid.h>

struct zmk_host_report_source {
    enum zmk_endpoint endpoint;
    uint8_t profile;
};

#ifdef CONFIG_ZMK_USB

struct zmk_host_usb_report {
    size_t length;
    uint8_t *data;
};

void zmk_host_process_usb_report(struct zmk_host_usb_report report);

#endif /* CONFIG_ZMK_USB */

struct zmk_host_report_source zmk_host_current_source();

void zmk_host_process_led_report(struct zmk_hid_led_report_body led_report,
                                 struct zmk_host_report_source source);
