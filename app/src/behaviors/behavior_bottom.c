/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_bottom

#include <device.h>
#include <drivers/behavior.h>
#include <kernel.h>
#include <logging/log.h>
#include <zmk/behavior.h>
#include <random/rand32.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_bottom_config {
    int delay_ms;
};

static void zmk_bottom_tick(struct k_work *work) {
    int key = (sys_rand32_get() % 26) + 4;
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(key, true, k_uptime_get()));
    k_msleep(5);
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(key, false, k_uptime_get()));
}

K_WORK_DEFINE(bottom_work, zmk_bottom_tick);

static void zmk_bottom_tick_handler(struct k_timer *timer) { k_work_submit(&bottom_work); }

K_TIMER_DEFINE(bottom_tick, zmk_bottom_tick_handler, NULL);

static int on_bottom_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = device_get_binding(binding->behavior_dev);
    const struct behavior_bottom_config *cfg = dev->config;

    k_timer_start(&bottom_tick, K_NO_WAIT, K_MSEC(cfg->delay_ms));

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_bottom_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    k_timer_stop(&bottom_tick);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_bottom_driver_api = {
    .binding_pressed = on_bottom_binding_pressed, .binding_released = on_bottom_binding_released,
};

static int behavior_bottom_init(const struct device *dev) { return 0; }

static struct behavior_bottom_config behavior_bottom_config = {
    .delay_ms = DT_INST_PROP(0, delay_ms),
};

DEVICE_DT_INST_DEFINE(0, behavior_bottom_init, device_pm_control_nop, NULL, &behavior_bottom_config,
                      APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_bottom_driver_api);

#endif
