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


struct device *accelerometer;

static s32_t read_sensor(struct device *sensor, enum sensor_channel channel)
{
	struct sensor_value val[3];
	s32_t ret = 0;

	ret = sensor_sample_fetch(sensor);
	if (ret < 0 && ret != -EBADMSG) {
		printk("Sensor sample update error\n");
	}

	ret = sensor_channel_get(sensor, channel, val);
	if (ret < 0) {
		printk("Cannot read sensor channels\n");
	}

	printf("( x y z ) = ( %f  %f  %f )\n", sensor_value_to_double(&val[0]),
					       sensor_value_to_double(&val[1]),
					       sensor_value_to_double(&val[2]));

	return ret;
}

static void lis2dh_handler(struct device *dev,
			   struct sensor_trigger *trig)
{
	read_sensor(accelerometer, SENSOR_CHAN_ACCEL_XYZ);
}

void main(void)
{
	struct device *dev = device_get_binding(LED_PORT);
	gpio_pin_configure(dev, LED, GPIO_DIR_OUT);

	accelerometer = device_get_binding(
						DT_INST_0_ST_LIS2DH_LABEL);

	if (accelerometer == NULL) {
		printk("Could not get %s device\n",
				DT_INST_0_ST_LIS2DH_LABEL);
		return;
	}

	if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER)) {
		struct sensor_trigger trig = {
			.type = SENSOR_TRIG_DATA_READY,
			.chan = SENSOR_CHAN_ALL,
		};
		if (sensor_trigger_set(dev, &trig, lis2dh_handler) < 0) {
			printf("Cannot configure trigger\n");
			return;
		};
	}

	while (!IS_ENABLED(CONFIG_LIS2DH_TRIGGER)) {
		printk("Accelerometer data:\n");
		if (read_sensor(accelerometer, SENSOR_CHAN_ACCEL_XYZ) < 0) {
			printk("Failed to read accelerometer data\n");
		}

		static int cnt = 0;
		/* Set pin to HIGH/LOW every 1 second */
		gpio_pin_write(dev, LED, cnt % 2);
		cnt++;
		k_sleep(2000);
	}
	k_sleep(K_FOREVER);
}
