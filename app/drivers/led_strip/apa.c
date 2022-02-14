/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @brief APA LED Strip Interface
 * @defgroup apa_led_strip_interface APA LED Strip Interface
 * @ingroup io_interfaces
 * @{
 */


#include <device.h>
#include <logging/log.h>

#include <drivers/led_strip.h>
#include <drivers/apa_led_strip.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define DT_DRV_COMPAT zmk_apa_led_strip

#if IS_ENABLED(CONFIG_ZMK_APA_LED_STRIP)
#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct apa_led_strip_data {
    struct led_rgb * pixels_buffer;
    uint8_t num_pixels;
    const struct device *led_strip;
};

static int apa_led_strip_update_rgb (  const struct device * apa_device,
				                struct led_rgb *pixels,
				                size_t num_pixels,
                                uint8_t * points,
                                uint8_t num_points ) {
    struct apa_led_strip_data * apa_data = (struct apa_led_strip_data * ) apa_device->data;
    const struct led_strip_driver_api *api =
		(const struct led_strip_driver_api *)apa_data->led_strip->api;
    int pixel_index ;

    for ( pixel_index = 0;
        pixel_index < num_pixels && pixel_index < num_points;
        pixel_index ++ ) {
        if ( points[pixel_index] < apa_data->num_pixels ) {
            apa_data->pixels_buffer[points[pixel_index]]=
                pixels[pixel_index];
        }
    }
    return api->update_rgb(apa_data->led_strip, apa_data->pixels_buffer, apa_data->num_pixels);
}

static int apa_led_strip_init(const struct device *_arg) {
    return 0;
}
static const struct apa_led_strip_driver_api apa_driver_api = {
	.update_rgb = apa_led_strip_update_rgb,
};

#define RGB_APA_LED_STRIP_INIT(inst)                                                                             \
    static struct led_rgb pixels_buffer_##inst[DT_INST_PROP_BY_PHANDLE(inst, led_strip, chain_length)            \
    static struct apa_led_strip_data apa_led_strip_device_##inst##_data = {                                      \
        .pixels_buffer = led_rgb pixels_buffer_##inst,                                                           \
        .num_pixels = DT_INST_PROP_BY_PHANDLE(inst, led_strip, chain_length),                                    \
        .led_strip = DEVICE_DT_GET(DT_INST_PHANDLE(inst, led_strip))                                             \
    };                                                                                                           \
    DEVICE_DT_INST_DEFINE(inst, apa_led_strip_init, device_pm_control_nop,                                       \
                      &apa_led_strip_device_##inst##_data, NULL, APPLICATION,                                    \
                      CONFIG_APPLICATION_INIT_PRIORITY, &apa_driver_api);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */

#endif /* IS_ENABLED(CONFIG_ZMK_APA_LED_STRIP) */
/**
 * @}
 */
