/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <sensor.h>
#include <stdio.h>
#include <misc/util.h>
#include <misc/printk.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME 	1000

static s32_t read_sensor(struct device *sensor, enum sensor_channel channel)
{
	struct sensor_value val[3];
	s32_t ret = 0;

	ret = sensor_sample_fetch(sensor);
	if (ret < 0 && ret != -EBADMSG) {
		printf("Sensor sample update error\n");
		goto end;
	}

	ret = sensor_channel_get(sensor, channel, val);
	if (ret < 0) {
		printf("Cannot read sensor channels\n");
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
	struct device *dev= device_get_binding(
						DT_INST_0_ST_LIS2DH_LABEL);

	if (dev== NULL) {
		printf("Could not get %s device\n",
				DT_INST_0_ST_LIS2DH_LABEL);
		return;
	}


	while(1){

		printf("Accelerometer data:\n");
		if (read_sensor(dev, SENSOR_CHAN_ACCEL_XYZ) < 0) {
			printf("Failed to read accelerometer data\n");
		}
		k_sleep(SLEEP_TIME);
	}
}
