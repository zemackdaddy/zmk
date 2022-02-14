/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <device.h>
#include <init.h>
#include <kernel.h>
#include <settings/settings.h>

#include <math.h>
#include <stdlib.h>

#include <logging/log.h>

#include <drivers/led_strip.h>
#include <drivers/ext_power.h>
#include <zmk/rgb_underglow.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define HUE_MAX 360
#define SAT_MAX 100
#define BRT_MAX 100

BUILD_ASSERT(CONFIG_ZMK_RGB_UNDERGLOW_BRT_MIN <= CONFIG_ZMK_RGB_UNDERGLOW_BRT_MAX,
             "ERROR: RGB underglow maximum brightness is less than minimum brightness");

enum rgb_underglow_effect {
    UNDERGLOW_EFFECT_SOLID,
    UNDERGLOW_EFFECT_BREATHE,
    UNDERGLOW_EFFECT_SPECTRUM,
    UNDERGLOW_EFFECT_SWIRL,
    UNDERGLOW_EFFECT_NUMBER // Used to track number of underglow effects
};

#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW_EXT_POWER) && ! IS_ENABLED(CONFIG_ZMK_APA_LED_STRIP)
static const struct device *ext_power;
#endif

static struct zmk_led_hsb hsb_scale_min_max(struct zmk_led_hsb hsb) {
    hsb.b = CONFIG_ZMK_RGB_UNDERGLOW_BRT_MIN +
            (CONFIG_ZMK_RGB_UNDERGLOW_BRT_MAX - CONFIG_ZMK_RGB_UNDERGLOW_BRT_MIN) * hsb.b / BRT_MAX;
    return hsb;
}

static struct zmk_led_hsb hsb_scale_zero_max(struct zmk_led_hsb hsb) {
    hsb.b = hsb.b * CONFIG_ZMK_RGB_UNDERGLOW_BRT_MAX / BRT_MAX;
    return hsb;
}

static struct led_rgb hsb_to_rgb(struct zmk_led_hsb hsb) {
    double r, g, b;

    uint8_t i = hsb.h / 60;
    double v = hsb.b / ((float)BRT_MAX);
    double s = hsb.s / ((float)SAT_MAX);
    double f = hsb.h / ((float)HUE_MAX) * 6 - i;
    double p = v * (1 - s);
    double q = v * (1 - f * s);
    double t = v * (1 - (1 - f) * s);

    switch (i % 6) {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    case 5:
        r = v;
        g = p;
        b = q;
        break;
    }

    struct led_rgb rgb = {r : r * 255, g : g * 255, b : b * 255};

    return rgb;
}

static void zmk_rgb_underglow_effect_solid(struct zmk_rgb_underglow_device_data * underglow_data) {
    for (int i = 0; i < underglow_data->num_pixels; i++) {
        underglow_data->pixels[i] = hsb_to_rgb(hsb_scale_min_max(underglow_data->state.color));
    }
}

static void zmk_rgb_underglow_effect_breathe(struct zmk_rgb_underglow_device_data * underglow_data) {
    for (int i = 0; i < underglow_data->num_pixels; i++) {
        struct zmk_led_hsb hsb = underglow_data->state.color;
        hsb.b = abs(underglow_data->state.animation_step - 1200) / 12;

        underglow_data->pixels[i] = hsb_to_rgb(hsb_scale_zero_max(hsb));
    }

    underglow_data->state.animation_step += underglow_data->state.animation_speed * 10;

    if (underglow_data->state.animation_step > 2400) {
        underglow_data->state.animation_step = 0;
    }
}

static void zmk_rgb_underglow_effect_spectrum(struct zmk_rgb_underglow_device_data * underglow_data) {
    for (int i = 0; i < underglow_data->num_pixels; i++) {
        struct zmk_led_hsb hsb = underglow_data->state.color;
        hsb.h = underglow_data->state.animation_step;

        underglow_data->pixels[i] = hsb_to_rgb(hsb_scale_min_max(hsb));
    }

    underglow_data->state.animation_step += underglow_data->state.animation_speed;
    underglow_data->state.animation_step = underglow_data->state.animation_step % HUE_MAX;
}

static void zmk_rgb_underglow_effect_swirl(struct zmk_rgb_underglow_device_data * underglow_data) {
    for (int i = 0; i < underglow_data->num_pixels; i++) {
        struct zmk_led_hsb hsb = underglow_data->state.color;
        hsb.h = (HUE_MAX / underglow_data->num_pixels * i + underglow_data->state.animation_step) % HUE_MAX;

        underglow_data->pixels[i] = hsb_to_rgb(hsb_scale_min_max(hsb));
    }

    underglow_data->state.animation_step += underglow_data->state.animation_speed * 2;
    underglow_data->state.animation_step = underglow_data->state.animation_step % HUE_MAX;
}

static void zmk_rgb_underglow_tick(struct k_work *work) {
    struct zmk_rgb_underglow_device_data * underglow_data =
        CONTAINER_OF ( work, struct zmk_rgb_underglow_device_data, underglow_work);

    switch (underglow_data->state.current_effect) {
    case UNDERGLOW_EFFECT_SOLID:
        zmk_rgb_underglow_effect_solid(underglow_data);
        break;
    case UNDERGLOW_EFFECT_BREATHE:
        zmk_rgb_underglow_effect_breathe(underglow_data);
        break;
    case UNDERGLOW_EFFECT_SPECTRUM:
        zmk_rgb_underglow_effect_spectrum(underglow_data);
        break;
    case UNDERGLOW_EFFECT_SWIRL:
        zmk_rgb_underglow_effect_swirl(underglow_data);
        break;
    }

    led_strip_update_rgb(underglow_data->led_strip, underglow_data->pixels, underglow_data->num_pixels);
}

static void zmk_rgb_underglow_tick_handler(struct k_timer *timer) {
    struct zmk_rgb_underglow_device_data * underglow_data =
        CONTAINER_OF ( timer, struct zmk_rgb_underglow_device_data, underglow_timer);

    if (!underglow_data->state.on) {
        return;
    }

    k_work_submit(&underglow_data->underglow_work);
}

#if IS_ENABLED(CONFIG_SETTINGS)
static int rgb_settings_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg) {
    struct zmk_rgb_underglow_device_data * underglow_data =
        CONTAINER_OF ( name, struct zmk_rgb_underglow_device_data, settings_name);

    const char *next;
    int rc;

    if (settings_name_steq(name, "state", &next) && !next) {
        if (len != sizeof(underglow_data->state)) {
            return -EINVAL;
        }

        rc = read_cb(cb_arg, &underglow_data->state, sizeof(underglow_data->state));
        if (rc >= 0) {
            return 0;
        }

        return rc;
    }

    return -ENOENT;
}

static void zmk_rgb_underglow_save_state_work(struct k_work *work) {
    struct k_delayed_work * p_k_delayed_work =
        CONTAINER_OF ( work, struct k_delayed_work, work);
    struct zmk_rgb_underglow_device_data * underglow_data =
        CONTAINER_OF ( p_k_delayed_work, struct zmk_rgb_underglow_device_data, underglow_save_work);
    char state_settings_name [48];
    strcpy ( state_settings_name, underglow_data->settings_name );
    strcat ( state_settings_name, "/state");

    settings_save_one(state_settings_name, &underglow_data->state, sizeof(underglow_data->state));
}

#endif

int zmk_rgb_underglow_init(const struct device *_arg) {

#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW_EXT_POWER) && ! IS_ENABLED(CONFIG_ZMK_APA_LED_STRIP)
    ext_power = device_get_binding("EXT_POWER");
    if (ext_power == NULL) {
        LOG_ERR("Unable to retrieve ext_power device: EXT_POWER");
    }
#endif

    struct zmk_rgb_underglow_device_data * underglow_data =
        (struct zmk_rgb_underglow_device_data *) _arg->data;

    underglow_data->state = (struct rgb_underglow_state){
        color : {
            h : CONFIG_ZMK_RGB_UNDERGLOW_HUE_START,
            s : CONFIG_ZMK_RGB_UNDERGLOW_SAT_START,
            b : CONFIG_ZMK_RGB_UNDERGLOW_BRT_START,
        },
        animation_speed : CONFIG_ZMK_RGB_UNDERGLOW_SPD_START,
        current_effect : CONFIG_ZMK_RGB_UNDERGLOW_EFF_START,
        animation_step : 0,
        on : IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW_ON_START)
    };

    #if IS_ENABLED(CONFIG_SETTINGS)
        underglow_data->rgb_conf.name = underglow_data->settings_name; //"_xxx";
        underglow_data->rgb_conf.h_set = rgb_settings_set;
        settings_subsys_init();

        int err = settings_register(&underglow_data->rgb_conf);
        if (err) {
            LOG_ERR("Failed to register the settings handler (err %d)", err);
            return err;
        }

        k_delayed_work_init(&underglow_data->underglow_save_work, zmk_rgb_underglow_save_state_work);

        settings_load_subtree(underglow_data->settings_name);
    #endif

    k_timer_start(&underglow_data->underglow_timer, K_NO_WAIT, K_MSEC(50));

    return 0;
}

int zmk_rgb_underglow_save_state(struct zmk_rgb_underglow_device_data * underglow_data) {
#if IS_ENABLED(CONFIG_SETTINGS)
    k_delayed_work_cancel(&underglow_data->underglow_save_work);
    return k_delayed_work_submit(&underglow_data->underglow_save_work, K_MSEC(CONFIG_ZMK_SETTINGS_SAVE_DEBOUNCE));
#else
    return 0;
#endif
}

int zmk_rgb_underglow_get_state(struct zmk_rgb_underglow_device_data * underglow_data, bool *on_off) {
    *on_off = underglow_data->state.on;
    return 0;
}

int zmk_rgb_underglow_on(struct zmk_rgb_underglow_device_data * underglow_data) {

#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW_EXT_POWER) && ! IS_ENABLED(CONFIG_ZMK_APA_LED_STRIP)
    if (ext_power != NULL) {
        int rc = ext_power_enable(ext_power);
        if (rc != 0) {
            LOG_ERR("Unable to enable EXT_POWER: %d", rc);
        }
    }
#endif

    underglow_data->state.on = true;
    underglow_data->state.animation_step = 0;
    k_timer_start(&underglow_data->underglow_timer, K_NO_WAIT, K_MSEC(50));

    return zmk_rgb_underglow_save_state(underglow_data);
}

int zmk_rgb_underglow_off(struct zmk_rgb_underglow_device_data * underglow_data) {
    #if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW_EXT_POWER) && ! IS_ENABLED(CONFIG_ZMK_APA_LED_STRIP)
        if (ext_power != NULL) {
            int rc = ext_power_disable(ext_power);
            if (rc != 0) {
                LOG_ERR("Unable to disable EXT_POWER: %d", rc);
            }
        }
    #endif

    for (int i = 0; i < underglow_data->num_pixels; i++) {
        underglow_data->pixels[i] = (struct led_rgb){r : 0, g : 0, b : 0};
    }

    led_strip_update_rgb(underglow_data->led_strip, underglow_data->pixels, underglow_data->num_pixels);

    k_timer_stop(&underglow_data->underglow_timer);
    underglow_data->state.on = false;

    return zmk_rgb_underglow_save_state(underglow_data);
}

int zmk_rgb_underglow_cycle_effect(struct zmk_rgb_underglow_device_data * underglow_data, int direction) {

    underglow_data->state.current_effect += UNDERGLOW_EFFECT_NUMBER + direction;
    underglow_data->state.current_effect %= UNDERGLOW_EFFECT_NUMBER;

    underglow_data->state.animation_step = 0;

    return zmk_rgb_underglow_save_state(underglow_data);
}

int zmk_rgb_underglow_toggle(struct zmk_rgb_underglow_device_data * underglow_data) {
    return underglow_data->state.on ? zmk_rgb_underglow_off(underglow_data) : zmk_rgb_underglow_on(underglow_data);
}

int zmk_rgb_underglow_set_hsb(struct zmk_rgb_underglow_device_data * underglow_data, struct zmk_led_hsb color) {
    if (color.h > HUE_MAX || color.s > SAT_MAX || color.b > BRT_MAX) {
        return -ENOTSUP;
    }

    underglow_data->state.color = color;

    return 0;
}

struct zmk_led_hsb zmk_rgb_underglow_calc_hue(struct zmk_rgb_underglow_device_data * underglow_data, int direction) {
    struct zmk_led_hsb color = underglow_data->state.color;

    color.h += HUE_MAX + (direction * CONFIG_ZMK_RGB_UNDERGLOW_HUE_STEP);
    color.h %= HUE_MAX;

    return color;
}

struct zmk_led_hsb zmk_rgb_underglow_calc_sat(struct zmk_rgb_underglow_device_data * underglow_data, int direction) {
    struct zmk_led_hsb color = underglow_data->state.color;

    int s = color.s + (direction * CONFIG_ZMK_RGB_UNDERGLOW_SAT_STEP);
    if (s < 0) {
        s = 0;
    } else if (s > SAT_MAX) {
        s = SAT_MAX;
    }
    color.s = s;

    return color;
}

struct zmk_led_hsb zmk_rgb_underglow_calc_brt(struct zmk_rgb_underglow_device_data * underglow_data, int direction) {
    struct zmk_led_hsb color = underglow_data->state.color;

    int b = color.b + (direction * CONFIG_ZMK_RGB_UNDERGLOW_BRT_STEP);
    color.b = CLAMP(b, 0, BRT_MAX);

    return color;
}

int zmk_rgb_underglow_change_hue(struct zmk_rgb_underglow_device_data * underglow_data, int direction) {
    underglow_data->state.color = zmk_rgb_underglow_calc_hue(underglow_data, direction);

    return zmk_rgb_underglow_save_state(underglow_data);
}

int zmk_rgb_underglow_change_sat(struct zmk_rgb_underglow_device_data * underglow_data, int direction) {
    underglow_data->state.color = zmk_rgb_underglow_calc_sat(underglow_data, direction);

    return zmk_rgb_underglow_save_state(underglow_data);
}

int zmk_rgb_underglow_change_brt(struct zmk_rgb_underglow_device_data * underglow_data, int direction) {
    underglow_data->state.color = zmk_rgb_underglow_calc_brt(underglow_data, direction);

    return zmk_rgb_underglow_save_state(underglow_data);
}

int zmk_rgb_underglow_change_spd(struct zmk_rgb_underglow_device_data * underglow_data, int direction) {
    if (underglow_data->state.animation_speed == 1 && direction < 0) {
        return 0;
    }

    underglow_data->state.animation_speed += direction;

    if (underglow_data->state.animation_speed > 5) {
        underglow_data->state.animation_speed = 5;
    }

    return zmk_rgb_underglow_save_state(underglow_data);
}

//SYS_INIT(zmk_rgb_underglow_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
