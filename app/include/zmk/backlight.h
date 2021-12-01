/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

int zmk_backlight_toggle();
int zmk_backlight_get_state(bool *state);
int zmk_backlight_on();
int zmk_backlight_off();
uint8_t zmk_backlight_calc_brt(int direction);
int zmk_backlight_set_brt(uint8_t brightness);
int zmk_backlight_change_brt(int direction);
