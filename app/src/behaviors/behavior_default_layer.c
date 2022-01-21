/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_default_layer

#include <device.h>
#include <drivers/behavior.h>
#include <logging/log.h>

#include <dt-bindings/zmk/default_layer.h>
#include <zmk/keymap.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int behavior_default_layer_init(const struct device *dev) { return 0; };

static int default_layer_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                                struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d direction %d", event.position, binding->param1);
    switch (binding->param1) {
    case LYRUP:
        return zmk_keymap_layer_change_default(1);
    case LYRDN:
        return zmk_keymap_layer_change_default(-1);
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static int default_layer_keymap_binding_released(struct zmk_behavior_binding *binding,
                                                 struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d direction %d", event.position, binding->param1);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_default_layer_driver_api = {
    .binding_pressed = default_layer_keymap_binding_pressed,
    .binding_released = default_layer_keymap_binding_released,
};

DEVICE_DT_INST_DEFINE(0, behavior_default_layer_init, device_pm_control_nop, NULL, NULL,
                      APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_default_layer_driver_api);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
