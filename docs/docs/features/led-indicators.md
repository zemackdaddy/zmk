---
title: LED indicators
sidebar_label: LED indicators
---

import Tabs from '@theme/Tabs';
import TabItem from '@theme/TabItem';

ZMK supports the following LED indicators:

- Num Lock
- Caps Lock
- Scroll Lock
- Compose
- Kana

## Enabling LED indicators

To enable LED indicators on your board or shield, simply enable the `CONFIG_ZMK_LED_INDICATORS` configuration values in the `.conf` file of your user config directory as such:

```
CONFIG_ZMK_LED_INDICATORS=y
```

You can also configure the brightness of the LEDs using:

```
CONFIG_ZMK_LED_INDICATORS_BRT=80
```

If your board or shield does not have LED indicators configured, refer to [Adding LED indicators to a Board](#adding-led-indicators-to-a-board).

## Adding LED indicators to a board

You can use any LED driver supported by ZMK or Zephyr, it is recommended to use the `ZMK_LED_PWM` driver.

<Tabs
defaultValue="pwm"
values={[
{label: 'ZMK_LED_PWM driver', value: 'pwm'},
{label: 'Other drivers', value: 'other'},
]}>
<TabItem value="pwm">

First you have to enable the driver by adding the following line to your `.conf` file:

```
CONFIG_ZMK_LED_PWM=y
```

Next, you need to enable PWM by adding the following lines to your `.overlay` file:

```
&pwm0 {
	status = "okay";
	ch0-pin = <33>;
	/* ch0-inverted; */
	ch1-pin = <35>;
	/* ch1-inverted; */
};
```

You have to declere a channel for each LED. A single PWM module has a fixed number of channels depending on your SoC, if you have many LEDs you may want to use `&pwm1` or ` &pwm2` as well.

The value `chX-pin` represents the pin that controls the LEDs. To calculate the value to use, you need a bit of math. You need the hardware port and run it through a function.
**32 \* X + Y** = `<Pin number>` where X is first part of the hardware port "PX.01" and Y is the second part of the hardware port "P1.Y".

For example, _P1.13_ would give you _32 \* 1 + 13_ = `<45>` and _P0.15_ would give you _32 \* 0 + 15_ = `<15>`.

If you need to invert the signal, you may want to enable `chX-inverted`.

Then you have to add the following lines to your `.dtsi` file inside the root devicetree node:

```
pwmleds: pwmleds {
    compatible = "pwm-leds";
    label = "LED indicators";
    pwm_led_0 {
        pwms = <&pwm0 33>;
        label = "Caps lock LED";
    };
    pwm_led_1 {
        pwms = <&pwm0 35>;
        label = "Num lock LED";
    };
};
```

The value inside `pwm_led_X` must be the same as you used before.

Then you have to add the following lines to your `.dtsi` file inside the root devicetree node:

```
led_indicators: led_indicators {
    compatible = "zmk,led-indicators";
    caps_lock_led {
        dev = <&pwmleds>;
        index = <0>;
        binding = <LED_CAPSLOCK>;
    };
    num_lock_led {
        dev = <&pwmleds>;
        index = <1>;
        binding = <LED_NUMLOCK>;
    };
};
```

You can find the definition of LEDs here: [`dt-bindings/zmk/led_indicators.h`](https://github.com/zmkfirmware/zmk/tree/main/app/include/dt-bindings/zmk/led_indicators.h).

Finally you need to add `led_indicators` to the `chosen` element of the root devicetree node:

```
chosen {
    ...
    zmk,led_indicators = &led_indicators;
};
```

</TabItem>
<TabItem value="other">

First, you need to enable your LED driver:

```
&my_led_driver: my_led_driver {
    ...
};
```

Then you have to add the following lines to your `.dtsi` file inside the root devicetree node:

```
led_indicators: led_indicators {
    compatible = "zmk,led-indicators";
    caps_lock_led {
        dev = <&my_led_driver>;
        index = <0>;
        binding = <LED_CAPSLOCK>;
    };
    num_lock_led {
        dev = <&my_led_driver>;
        index = <1>;
        binding = <LED_NUMLOCK>;
    };
};
```

You can find the definition of LEDs here: [`dt-bindings/zmk/led_indicators.h`](https://github.com/zmkfirmware/zmk/tree/main/app/include/dt-bindings/zmk/led_indicators.h).

Finally you need to add `led_indicators` to the `chosen` element of the root devicetree node:

```
chosen {
    ...
    zmk,led_indicators = &led_indicators;
};
```

</TabItem>
</Tabs>

## Custom behavior

To implement custom behavior, such as a display widget, you must first **remove** `CONFIG_ZMK_LED_INDICATORS=y` from your `.conf` file. Then you can write a listener to implement the behavior you want, for example:

```C
#include <dt-bindings/zmk/led_indicators.h>

static int led_indicators_listener(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_led_indicator_changed(eh);
    zmk_led_indicators_flags_t leds = ev->leds;

    if (leds & BIT(LED_CAPSLOCK)) {
        // do something
    }

    return 0;
}

ZMK_LISTENER(led_indicators_listener, led_indicators_listener);
ZMK_SUBSCRIPTION(led_indicators_listener, zmk_led_indicator_changed);
```
