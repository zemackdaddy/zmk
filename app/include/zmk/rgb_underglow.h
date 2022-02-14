/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

struct zmk_led_hsb {
    uint16_t h;
    uint8_t s;
    uint8_t b;
};

struct rgb_underglow_state {
    struct zmk_led_hsb color;
    uint8_t animation_speed;
    uint8_t current_effect;
    uint16_t animation_step;
    bool on;
};

struct zmk_rgb_underglow_device_data {
    const struct device * led_strip;
    char settings_name[32];
    struct led_rgb * pixels;
    uint8_t num_pixels;
    struct rgb_underglow_state state;
    struct k_timer underglow_timer;
    struct k_work underglow_work;
    #if IS_ENABLED(CONFIG_SETTINGS)
        struct settings_handler rgb_conf;
        struct k_delayed_work underglow_save_work;
    #endif
};
int zmk_rgb_underglow_init(const struct device *_arg);

int zmk_rgb_underglow_toggle(struct zmk_rgb_underglow_device_data * underglow_data);
int zmk_rgb_underglow_get_state(struct zmk_rgb_underglow_device_data * underglow_data, bool *state);
int zmk_rgb_underglow_on(struct zmk_rgb_underglow_device_data * underglow_data);
int zmk_rgb_underglow_off(struct zmk_rgb_underglow_device_data * underglow_data);
int zmk_rgb_underglow_cycle_effect(struct zmk_rgb_underglow_device_data * underglow_data, int direction);
struct zmk_led_hsb zmk_rgb_underglow_calc_hue(struct zmk_rgb_underglow_device_data * underglow_data, int direction);
struct zmk_led_hsb zmk_rgb_underglow_calc_sat(struct zmk_rgb_underglow_device_data * underglow_data, int direction);
struct zmk_led_hsb zmk_rgb_underglow_calc_brt(struct zmk_rgb_underglow_device_data * underglow_data, int direction);
int zmk_rgb_underglow_change_hue(struct zmk_rgb_underglow_device_data * underglow_data, int direction);
int zmk_rgb_underglow_change_sat(struct zmk_rgb_underglow_device_data * underglow_data, int direction);
int zmk_rgb_underglow_change_brt(struct zmk_rgb_underglow_device_data * underglow_data, int direction);
int zmk_rgb_underglow_change_spd(struct zmk_rgb_underglow_device_data * underglow_data, int direction);
int zmk_rgb_underglow_set_hsb(struct zmk_rgb_underglow_device_data * underglow_data, struct zmk_led_hsb color);
int zmk_rgb_underglow_save_state(struct zmk_rgb_underglow_device_data * underglow_data);