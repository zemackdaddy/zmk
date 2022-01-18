/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int apa_led_strip_update_rgb (  const struct device * zmk_rgb_underglow_device,
				                struct led_rgb *pixels,
				                size_t num_pixels ); 



#ifdef __cplusplus
}
#endif