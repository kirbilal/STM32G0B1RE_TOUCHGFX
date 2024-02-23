/*
 * touch_driver.c
 *
 *  Created on: Dec 2, 2022
 *      Author: osmang
 */

#include "main.h"
#include "touch_driver.h"
// COMMANDS
typedef enum {
	TOUCH_GET_VERSION = 0x00,
	TOUCH_GET_STATUS = 0x01,
	TOUCH_CONTROL_REG = 0x02,
	TOUCH_GET_COOR = 0x10,
	TOUCH_GET_X0Y0H = 0x12,
	TOUCH_GET_X0L = 0x13,
	TOUCH_GET_Y0L = 0x14
} sitronix_command_t;

bool sitronix_software_reset(sitronix_handle_t *handle) {
	uint8_t buf[1];

	buf[0] = 0x01;

	if (HAL_I2C_Mem_Write(handle->i2c_handle, handle->device_address << 1,
			TOUCH_CONTROL_REG, 1, (uint8_t*) &buf, sizeof(buf), 1000)
			!= HAL_OK) {
		return false;
	}

	return true;
}

bool sitronix_hardware_reset() {
	HAL_GPIO_WritePin(CTP_RESET_GPIO_Port, CTP_RESET_Pin, 1);
	HAL_Delay(500);
	HAL_GPIO_WritePin(CTP_RESET_GPIO_Port, CTP_RESET_Pin, 0);
	HAL_Delay(500);
	HAL_GPIO_WritePin(CTP_RESET_GPIO_Port, CTP_RESET_Pin, 1);
	HAL_Delay(500);
	return true;
}

bool sitronix_status_get(sitronix_handle_t *handle) {
	uint8_t buf[1];

	buf[0] = 0x0;

	if (HAL_I2C_Mem_Read(handle->i2c_handle, handle->device_address << 1u,
			TOUCH_GET_STATUS, 1, (uint8_t*) &buf, sizeof(buf), 1000)
			!= HAL_OK) {
		return false;
	}

	return true;
}

bool sitronix_version_get(sitronix_handle_t *handle) {
	uint8_t buf[1];

	buf[0] = 0x0;
	if (HAL_I2C_Mem_Write(handle->i2c_handle, handle->device_address << 1u,
			TOUCH_GET_VERSION, 1, (uint8_t*) &buf, sizeof(buf), 1000)
			!= HAL_OK) {
		return false;
	}

	if (HAL_I2C_Mem_Read(handle->i2c_handle, handle->device_address << 1u,
			TOUCH_CONTROL_REG, 1, (uint8_t*) &buf, sizeof(buf), 1000)
			!= HAL_OK) {
		return false;
	}

	return true;
}

bool sitronix_close_multitouch(sitronix_handle_t *handle) {
	uint8_t buf[1];
	buf[0] = 64; // 0100 0000 bit 6 is Multi-Touch disable
	if (HAL_I2C_Mem_Write(handle->i2c_handle, handle->device_address << 1u,
			TOUCH_GET_COOR, 1, (uint8_t*) &buf, sizeof(buf), 1000) != HAL_OK) {
		return false;
	}

	return true;
}

bool sitronix_get_coordinates(sitronix_handle_t *handle, uint8_t *updated,
		uint16_t *x0, uint16_t *y0) {

	uint8_t buf1[1];
	if (HAL_I2C_Mem_Read(handle->i2c_handle, handle->device_address << 1u,
			TOUCH_GET_X0Y0H, 1, (uint8_t*) &buf1, 1, 1000) != HAL_OK) {
		return false;
	}
	uint8_t valid = (buf1[0] & 0b10000000) >> 7;

	if (valid == 1) {
		uint8_t buf2[1];
		if (HAL_I2C_Mem_Read(handle->i2c_handle, handle->device_address << 1u,
				TOUCH_GET_X0L, 1, (uint8_t*) &buf2, 1, 1000) != HAL_OK) {
			return false;
		}
		uint8_t buf3[1];
		if (HAL_I2C_Mem_Read(handle->i2c_handle, handle->device_address << 1u,
				TOUCH_GET_Y0L, 1, (uint8_t*) &buf3, 1, 1000) != HAL_OK) {
			return false;
		}
		uint8_t x0_H = (buf1[0] & 0b01110000) >> 4;
		uint8_t y0_H = (buf1[0] & 0b00000111);

		*x0 = ((uint16_t) x0_H << 8u) | (uint16_t) buf2[0];
		*y0 = ((uint16_t) y0_H << 8u) | (uint16_t) buf3[0];
		*updated = 1;
		return true;
	} else {
		return false;
	}

}

