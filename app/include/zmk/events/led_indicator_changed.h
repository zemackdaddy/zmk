/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zmk/led_indicators.h>
#include <zmk/event_manager.h>

struct zmk_led_indicator_changed {
    zmk_led_indicators_flags_t leds;
};

ZMK_EVENT_DECLARE(zmk_led_indicator_changed);
