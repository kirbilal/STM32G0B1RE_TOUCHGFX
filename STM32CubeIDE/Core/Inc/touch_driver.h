/*
 * touch_driver.h
 *
 *  Created on: Dec 2, 2022
 *      Author: osmang
 */

#ifndef INC_TOUCH_DRIVER_H_
#define INC_TOUCH_DRIVER_H_
#include <stdbool.h>
#include "stm32g0xx_hal.h"


#define SITRONIX_TOUCH_ADDRESS 0x55


typedef struct {

	I2C_HandleTypeDef *i2c_handle;

	uint16_t device_address;

} sitronix_handle_t;



bool sitronix_hardware_reset();
bool sitronix_status_get(sitronix_handle_t *handle);
bool sitronix_software_reset(sitronix_handle_t *handle);
bool sitronix_version_get(sitronix_handle_t *handle);
bool sitronix_get_coordinates(sitronix_handle_t *handle, uint8_t *updated, uint16_t *x0, uint16_t *y0 );
bool sitronix_close_multitouch(sitronix_handle_t *handle);



#endif /* INC_TOUCH_DRIVER_H_ */
