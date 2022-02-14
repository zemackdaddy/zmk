/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_rgb_underglow

#include <device.h>
#include <drivers/behavior.h>
#include <logging/log.h>
#include <drivers/led_strip.h>

#include <settings/settings.h>
#include <dt-bindings/zmk/rgb.h>
#include <zmk/rgb_underglow.h>
#include <zmk/keymap.h>

#define STRIP_DEFAULT_NODE DT_CHOSEN(zmk_underglow)

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int
on_keymap_binding_convert_central_state_dependent_params(struct zmk_behavior_binding *binding,
                                                         struct zmk_behavior_binding_event event) {
    struct zmk_rgb_underglow_device_data * underglow_data = device_get_binding (binding->behavior_dev)->data;
    switch (binding->param1) {
    case RGB_TOG_CMD: {
        bool state;
        int err = zmk_rgb_underglow_get_state(underglow_data, &state);
        if (err) {
            LOG_ERR("Failed to get RGB underglow state (err %d)", err);
            return err;
        }

        binding->param1 = state ? RGB_OFF_CMD : RGB_ON_CMD;
        break;
    }
    case RGB_BRI_CMD: {
        struct zmk_led_hsb color = zmk_rgb_underglow_calc_brt(underglow_data, 1);

        binding->param1 = RGB_COLOR_HSB_CMD;
        binding->param2 = RGB_COLOR_HSB_VAL(color.h, color.s, color.b);
        break;
    }
    case RGB_BRD_CMD: {
        struct zmk_led_hsb color = zmk_rgb_underglow_calc_brt(underglow_data, -1);

        binding->param1 = RGB_COLOR_HSB_CMD;
        binding->param2 = RGB_COLOR_HSB_VAL(color.h, color.s, color.b);
        break;
    }
    case RGB_HUI_CMD: {
        struct zmk_led_hsb color = zmk_rgb_underglow_calc_hue(underglow_data, 1);

        binding->param1 = RGB_COLOR_HSB_CMD;
        binding->param2 = RGB_COLOR_HSB_VAL(color.h, color.s, color.b);
        break;
    }
    case RGB_HUD_CMD: {
        struct zmk_led_hsb color = zmk_rgb_underglow_calc_hue(underglow_data, -1);

        binding->param1 = RGB_COLOR_HSB_CMD;
        binding->param2 = RGB_COLOR_HSB_VAL(color.h, color.s, color.b);
        break;
    }
    case RGB_SAI_CMD: {
        struct zmk_led_hsb color = zmk_rgb_underglow_calc_sat(underglow_data, 1);

        binding->param1 = RGB_COLOR_HSB_CMD;
        binding->param2 = RGB_COLOR_HSB_VAL(color.h, color.s, color.b);
        break;
    }
    case RGB_SAD_CMD: {
        struct zmk_led_hsb color = zmk_rgb_underglow_calc_sat(underglow_data, -1);

        binding->param1 = RGB_COLOR_HSB_CMD;
        binding->param2 = RGB_COLOR_HSB_VAL(color.h, color.s, color.b);
        break;
    }
    default:
        return 0;
    }

    LOG_DBG("RGB relative convert to absolute (%d/%d)", binding->param1, binding->param2);

    return 0;
};

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    struct zmk_rgb_underglow_device_data * underglow_data = device_get_binding (binding->behavior_dev)->data;

    switch (binding->param1) {
    case RGB_TOG_CMD:
        return zmk_rgb_underglow_toggle(underglow_data);
    case RGB_ON_CMD:
        return zmk_rgb_underglow_on(underglow_data);
    case RGB_OFF_CMD:
        return zmk_rgb_underglow_off(underglow_data);
    case RGB_HUI_CMD:
        return zmk_rgb_underglow_change_hue(underglow_data, 1);
    case RGB_HUD_CMD:
        return zmk_rgb_underglow_change_hue(underglow_data, -1);
    case RGB_SAI_CMD:
        return zmk_rgb_underglow_change_sat(underglow_data, 1);
    case RGB_SAD_CMD:
        return zmk_rgb_underglow_change_sat(underglow_data, -1);
    case RGB_BRI_CMD:
        return zmk_rgb_underglow_change_brt(underglow_data, 1);
    case RGB_BRD_CMD:
        return zmk_rgb_underglow_change_brt(underglow_data, -1);
    case RGB_SPI_CMD:
        return zmk_rgb_underglow_change_spd(underglow_data, 1);
    case RGB_SPD_CMD:
        return zmk_rgb_underglow_change_spd(underglow_data, -1);
    case RGB_EFF_CMD:
        return zmk_rgb_underglow_cycle_effect(underglow_data, 1);
    case RGB_EFR_CMD:
        return zmk_rgb_underglow_cycle_effect(underglow_data, -1);
    case RGB_COLOR_HSB_CMD:
        return zmk_rgb_underglow_set_hsb(underglow_data, (struct zmk_led_hsb){.h = (binding->param2 >> 16) & 0xFFFF,
                                                              .s = (binding->param2 >> 8) & 0xFF,
                                                              .b = binding->param2 & 0xFF});
    }

    return -ENOTSUP;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_rgb_underglow_driver_api = {
    .binding_convert_central_state_dependent_params =
        on_keymap_binding_convert_central_state_dependent_params,
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define STRIP_NUM_PIXELS DT_PROP(DT_CHOSEN(zmk_underglow), chain_length)

#define RGB_UNDERGLOW_INIT(inst)                                                                             \
    static struct led_rgb subs_pixels_##inst[                                                                \
        COND_CODE_1 ( DT_NODE_HAS_PROP(DT_DRV_INST(inst), led_strip),                                        \
                      (DT_PROP_LEN(DT_INST_PHANDLE(inst, led_strip),points)),                                \
                      (STRIP_NUM_PIXELS)                                                                     \
                    )];                                                                                      \
    struct zmk_rgb_underglow_device_data rgb_underglow_##inst##_data = {                                     \
        .led_strip = COND_CODE_1 ( DT_NODE_HAS_PROP(DT_DRV_INST(inst), led_strip),                           \
                                   DEVICE_DT_GET(DT_INST_PHANDLE(inst, led_strip)),                          \
                                   DEVICE_DT_GET(STRIP_DEFAULT_NODE)                                         \
                                 ),                                                                          \
        .settings_name = "rgb/underglow_" STRINGIFY(inst),                                                   \
        .pixels = subs_pixels_##inst,                                                                        \
        .num_pixels = COND_CODE_1 ( DT_NODE_HAS_PROP(DT_DRV_INST(inst), led_strip),                          \
                                (DT_PROP_LEN(DT_INST_PHANDLE(inst, led_strip),points)),                      \
                                (STRIP_NUM_PIXELS)                                                           \
                              ),                                                                             \
    };                                                                                                       \
    DEVICE_DT_INST_DEFINE(inst, zmk_rgb_underglow_init, device_pm_control_nop,                               \
                      &rgb_underglow_##inst##_data, NULL, APPLICATION,                                       \
                      CONFIG_APPLICATION_INIT_PRIORITY, &behavior_rgb_underglow_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RGB_UNDERGLOW_INIT);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
