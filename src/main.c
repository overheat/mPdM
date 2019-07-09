/* main.c - Application main entry point */

/*
 * Copyright (c) 2019 Aaron Tsui aaron.tsui@outlook.com
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <misc/printk.h>
#include <misc/byteorder.h>
#include <zephyr.h>

#include <settings/settings.h>

#include <device.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>

#define LED_PORT DT_ALIAS_LED0_GPIOS_CONTROLLER
#define LED	DT_ALIAS_LED0_GPIOS_PIN


static s32_t read_sensor(struct device *sensor, enum sensor_channel channel)
{
	struct sensor_value val[3];
	s32_t ret = 0;

	ret = sensor_sample_fetch(sensor);
	if (ret < 0 && ret != -EBADMSG) {
		printk("Sensor sample update error\n");
		goto end;
	}

	ret = sensor_channel_get(sensor, channel, val);
	if (ret < 0) {
		printk("Cannot read sensor channels\n");
		goto end;
	}

	printf("( x y z ) = ( %f  %f  %f )\n", sensor_value_to_double(&val[0]),
					       sensor_value_to_double(&val[1]),
					       sensor_value_to_double(&val[2]));

end:
	return ret;
}

void main(void)
{
	struct device *dev = device_get_binding(LED_PORT);
	gpio_pin_configure(dev, LED, GPIO_DIR_OUT);

	struct device *accelerometer = device_get_binding(
						DT_INST_0_ST_LIS2DH_LABEL);

	if (accelerometer == NULL) {
		printk("Could not get %s device\n",
				DT_INST_0_ST_LIS2DH_LABEL);
		return;
	}

	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(MSEC_PER_SEC);

		static int cnt = 0;
		/* Set pin to HIGH/LOW every 1 second */
		gpio_pin_write(dev, LED, cnt % 2);
		cnt++;

		printk("Accelerometer data:\n");
		if (read_sensor(accelerometer, SENSOR_CHAN_ACCEL_XYZ) < 0) {
			printk("Failed to read accelerometer data\n");
		}

	}
}
