# Copyright (c) 2021 The ZMK Contributors
# SPDX-License-Identifier: MIT

zephyr_library_sources_ifdef(CONFIG_ZMK_LED_PWM led_pwm.c)
