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

#define DT_DRV_COMPAT zmk_behavior_rgb_underglow

#include <device.h>
#include <logging/log.h>

#include <zmk/hid.h>
#include <zmk/keys.h>
#include <dt-bindings/zmk/modifiers.h>
#include <zmk/behavior.h>
#include <drivers/led_strip.h>
#include <zmk/apa_led_strip.h>
#include <zmk/rgb_underglow.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
#if IS_ENABLED(CONFIG_ZMK_APA_LED_STRIP)

static struct led_rgb pixels_buffer[STRIP_NUM_PIXELS] = {{0,},};

struct underglow_subsets_child_config {
    uint8_t points[STRIP_NUM_PIXELS];
    uint8_t num_of_points;
};

int apa_led_strip_update_rgb (  const struct device * zmk_rgb_underglow_device,
				                struct led_rgb *pixels,
				                size_t num_pixels ) {

    uint8_t num_points = ((struct rgb_underglow_config *)zmk_rgb_underglow_device->config)->num_points;
    if ( num_points){
        // underglow behavior has been called with a led strip subset, so only a pixels subset is affected
        int point_index ;
        uint8_t * points = ((struct rgb_underglow_config *)zmk_rgb_underglow_device->config)->points;

        for ( point_index = 0;
            point_index < num_points && point_index < STRIP_NUM_PIXELS;
            point_index ++ ) {
            if ( points[point_index] < STRIP_NUM_PIXELS ) {
                pixels_buffer[points[point_index]]=
                    pixels[points[point_index]];
            }
        }
    }
    else {
        // underglow behavior has been called without a led strip subset, so all pixels are affected
        int pixels_index;
        for ( pixels_index = 0;
              pixels_index < STRIP_NUM_PIXELS && pixels_index < num_pixels;
              pixels_index ++
            ) {
            pixels_buffer[pixels_index] = pixels [pixels_index];
        }
    }
    return led_strip_update_rgb( ((struct rgb_underglow_config *)zmk_rgb_underglow_device->config)->led_strip, 
                                 pixels_buffer,
                                 num_pixels
                               );
}

#endif /* IS_ENABLED(CONFIG_ZMK_APA_LED_STRIP) */
#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
/**
 * @}
 */
