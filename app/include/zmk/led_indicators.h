/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zmk/host.h>

typedef uint8_t zmk_led_indicators_flags_t;

zmk_led_indicators_flags_t zmk_led_indicators_get_current_flags();
zmk_led_indicators_flags_t zmk_led_indicators_get_flags(struct zmk_host_report_source source);
void zmk_led_indicators_update_flags(zmk_led_indicators_flags_t leds,
                                     struct zmk_host_report_source source);
