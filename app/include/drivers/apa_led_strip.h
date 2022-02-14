/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*apa_led_api_update_rgb)(const struct device *dev,
				  struct led_rgb *pixels,
				  size_t num_pixels,
				  uint8_t * points,
				  uint8_t num_points
		          );

struct apa_led_strip_driver_api {
	apa_led_api_update_rgb update_rgb;
};

#ifdef __cplusplus
}
#endif