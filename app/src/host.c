/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <kernel.h>
#include <logging/log.h>

#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/hid.h>
#include <zmk/host.h>
#include <zmk/led_indicators.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#ifdef CONFIG_ZMK_USB

#define LED_REPORT_ID 1

void zmk_host_process_usb_report(struct zmk_host_usb_report report) {
    uint8_t report_id = report.data[0];

    struct zmk_host_report_source source = {.endpoint = ZMK_ENDPOINT_USB};

    switch (report_id) {
    case LED_REPORT_ID:
        if (report.length != sizeof(struct zmk_hid_led_report)) {
            LOG_ERR("LED report is malformed: length=%d", report.length);
        } else {
            struct zmk_hid_led_report led_report;
            memcpy(&led_report, report.data, report.length);
            zmk_host_process_led_report(led_report.body, source);
        }
        break;
    default:
        LOG_WRN("Unsupported host report: %d", report_id);
        break;
    }
}

#endif /* CONFIG_ZMK_USB */

struct zmk_host_report_source zmk_host_current_source() {
    enum zmk_endpoint endpoint = zmk_endpoints_selected();
    uint8_t profile = 0;

#if IS_ENABLED(CONFIG_ZMK_BLE)
    if (endpoint == ZMK_ENDPOINT_BLE) {
        profile = zmk_ble_active_profile_index();
    }
#endif

    return (struct zmk_host_report_source){
        .endpoint = endpoint,
        .profile = profile,
    };
}

void zmk_host_process_led_report(struct zmk_hid_led_report_body report,
                                 struct zmk_host_report_source source) {
    zmk_led_indicators_update_flags(report.leds, source);
}
