/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <kernel.h>
#include <device.h>
#include <init.h>

#include <logging/log.h>
#include <drivers/led.h>

#include <zmk/hid.h>
#include <zmk/led_indicators.h>
#include <zmk/events/led_indicator_changed.h>
#include <dt-bindings/zmk/led_indicators.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#ifdef CONFIG_ZMK_LED_INDICATORS

BUILD_ASSERT(DT_HAS_CHOSEN(zmk_led_indicators),
             "CONFIG_ZMK_LED_INDICATORS is enabled but no zmk,led-indicators chosen node found");

struct led_indicator_cfg {
    const struct device *dev;
    int index;
    int binding;
};

#define LED_CFG(node_id)                                                                           \
    (struct led_indicator_cfg){                                                                    \
        .dev = DEVICE_DT_GET(DT_PHANDLE(node_id, dev)),                                            \
        .index = DT_PROP(node_id, index),                                                          \
        .binding = DT_PROP(node_id, binding),                                                      \
    },

static const struct led_indicator_cfg led_indicators[] = {
    DT_FOREACH_CHILD(DT_CHOSEN(zmk_led_indicators), LED_CFG)};

static int zmk_led_indicators_update() {
    zmk_led_indicators_flags_t flags = zmk_led_indicators_get_current_flags();
    for (int i = 0; i < ARRAY_SIZE(led_indicators); i++) {
        uint8_t value =
            (flags & BIT(led_indicators[i].binding)) ? CONFIG_ZMK_LED_INDICATORS_BRT : 0;
        int rc = led_set_brightness(led_indicators[i].dev, led_indicators[i].index, value);
        if (rc != 0) {
            return rc;
        }
    }
    return 0;
}

static int zmk_led_indicators_init(const struct device *_arg) {
    for (int i = 0; i < ARRAY_SIZE(led_indicators); i++) {
        if (!device_is_ready(led_indicators[i].dev)) {
            LOG_ERR("LED device \"%s\" is not ready", led_indicators[i].dev->name);
            return -ENODEV;
        }
    }

    return 0;
}

SYS_INIT(zmk_led_indicators_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

#endif // CONFIG_ZMK_LED_INDICATORS

#define NUM_USB_PROFILES (COND_CODE_1(IS_ENABLED(CONFIG_ZMK_USB), (1), (0)))
#define NUM_BLE_PROFILES (COND_CODE_1(IS_ENABLED(CONFIG_ZMK_BLE), (CONFIG_BT_MAX_CONN), (0)))
#define NUM_PROFILES (NUM_USB_PROFILES + NUM_BLE_PROFILES)

static zmk_led_indicators_flags_t led_flags[NUM_PROFILES];

zmk_led_indicators_flags_t zmk_led_indicators_get_current_flags() {
    struct zmk_host_report_source source = zmk_host_current_source();
    return zmk_led_indicators_get_flags(source);
}

zmk_led_indicators_flags_t zmk_led_indicators_get_flags(struct zmk_host_report_source source) {
    size_t index = source.profile + source.endpoint == ZMK_ENDPOINT_BLE;
    return led_flags[index];
}

static void raise_led_changed_event(struct k_work *_work) {
#ifdef CONFIG_ZMK_LED_INDICATORS
    zmk_led_indicators_update();
#endif

    ZMK_EVENT_RAISE(new_zmk_led_indicator_changed(
        (struct zmk_led_indicator_changed){.leds = zmk_led_indicators_get_current_flags()}));
}

static K_WORK_DEFINE(led_changed_work, raise_led_changed_event);

void zmk_led_indicators_update_flags(zmk_led_indicators_flags_t leds,
                                     struct zmk_host_report_source source) {
    size_t index = source.profile + source.endpoint == ZMK_ENDPOINT_BLE;
    led_flags[index] = leds;

    k_work_submit(&led_changed_work);
}
