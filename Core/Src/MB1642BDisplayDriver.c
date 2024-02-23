#include "stm32g0xx_hal.h"
#include "DCS.h"
#include "main.h"
#include "MB1642BDisplayDriver.h"
#include "touch_driver.h"
#include <assert.h>

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern TIM_HandleTypeDef htim2;

volatile uint16_t TE = 0;

//Signal TE interrupt to TouchGFX
void touchgfxSignalVSync(void);

static void Display_DCS_Send(uint8_t command) {
	// Reset the nCS pin
	DISPLAY_CSX_GPIO_Port->BRR = DISPLAY_CSX_Pin;
	// Set the DCX pin
	DISPLAY_DCX_GPIO_Port->BRR = DISPLAY_DCX_Pin;

	// Send the command
	*((__IO uint8_t*) &hspi1.Instance->DR) = command;

	// Wait until the bus is not busy before changing configuration
	while (((hspi1.Instance->SR) & SPI_FLAG_BSY) != RESET)
		;

	// Reset the DCX pin
	DISPLAY_DCX_GPIO_Port->BSRR = DISPLAY_DCX_Pin;

	// Set the nCS
	DISPLAY_CSX_GPIO_Port->BSRR = DISPLAY_CSX_Pin;
}

static void Display_DCS_Send_With_Data(uint8_t command, uint8_t *data,
		uint8_t size) {
	// Reset the nCS pin
	DISPLAY_CSX_GPIO_Port->BRR = DISPLAY_CSX_Pin;
	// Set the DCX pin
	DISPLAY_DCX_GPIO_Port->BRR = DISPLAY_DCX_Pin;

	*((__IO uint8_t*) &hspi1.Instance->DR) = command;

	// Wait until the bus is not busy before changing configuration
	while (((hspi1.Instance->SR) & SPI_FLAG_BSY) != RESET)
		;
	DISPLAY_DCX_GPIO_Port->BSRR = DISPLAY_DCX_Pin;

	while (size > 0U) {
		*((__IO uint8_t*) &hspi1.Instance->DR) = *data;
		data++;
		size--;
		/* Wait until TXE flag is set to send data */
		while (((hspi1.Instance->SR) & SPI_FLAG_TXE) != SPI_FLAG_TXE)
			;
	}

	// Wait until the bus is not busy before changing configuration
	while (((hspi1.Instance->SR) & SPI_FLAG_BSY) != RESET)
		;

	// Set the nCS
	DISPLAY_CSX_GPIO_Port->BSRR = DISPLAY_CSX_Pin;
}

void MB1642BDisplayDriver_DisplayOn(void) {
	// Display ON
	Display_DCS_Send(DCS_SET_DISPLAY_ON);
	HAL_Delay(100);
}

void Display_OFF(void) {
	// Display OFF
	Display_DCS_Send(DCS_SET_DISPLAY_OFF);
	HAL_Delay(100);
}

static uint16_t old_x0 = 0xFFFF, old_x1 = 0xFFFF, old_y0 = 0xFFFF, old_y1 =
		0xFFFF;

void Display_Set_Area(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	uint8_t arguments[4];

	// Set columns, if changed
	if (x0 != old_x0 || x1 != old_x1) {
		arguments[0] = x0 >> 8;
		arguments[1] = x0 & 0xFF;
		arguments[2] = x1 >> 8;
		arguments[3] = x1 & 0xFF;
		Display_DCS_Send_With_Data(0x2A, arguments, 4);

		old_x0 = x0;
		old_x1 = x1;
	}

	// Set rows, if changed
	if (y0 != old_y0 || y1 != old_y1) {
		arguments[0] = y0 >> 8;
		arguments[1] = y0 & 0xFF;
		arguments[2] = y1 >> 8;
		arguments[3] = y1 & 0xFF;
		Display_DCS_Send_With_Data(0x2B, arguments, 4);

		old_y0 = y0;
		old_y1 = y1;
	}
}

volatile uint8_t IsTransmittingBlock_;
void Display_Bitmap(const uint16_t *bitmap, uint16_t posx, uint16_t posy,
		uint16_t sizex, uint16_t sizey) {
	IsTransmittingBlock_ = 1;
	__HAL_SPI_ENABLE(&hspi1); // Enables SPI peripheral
	uint8_t command = DCS_WRITE_MEMORY_START;

	// Define the display area
	Display_Set_Area(posx, posy, posx + sizex - 1, posy + sizey - 1);

	// Reset the nCS pin
	DISPLAY_CSX_GPIO_Port->BRR = DISPLAY_CSX_Pin;
	// Set the DCX pin
	DISPLAY_DCX_GPIO_Port->BRR = DISPLAY_DCX_Pin;

	*((__IO uint8_t*) &hspi1.Instance->DR) = command;

	// Wait until the bus is not busy before changing configuration
	while (((hspi1.Instance->SR) & SPI_FLAG_BSY) != RESET)
		;
	DISPLAY_DCX_GPIO_Port->BSRR = DISPLAY_DCX_Pin;

	// Set the SPI in 16-bit mode to match endianess
	hspi1.Instance->CR2 = SPI_DATASIZE_16BIT;

	// Disable spi peripherals
	__HAL_SPI_DISABLE(&hspi1);
	__HAL_DMA_DISABLE(&hdma_spi1_tx);

	CLEAR_BIT(hspi1.Instance->CR2, SPI_CR2_LDMATX);

	/* Clear all flags */
	__HAL_DMA_CLEAR_FLAG(&hdma_spi1_tx,
			(DMA_FLAG_GI1 << (hdma_spi1_tx.ChannelIndex & 0x1cU)));

	/* Configure DMA Channel data length */
	hdma_spi1_tx.Instance->CNDTR = sizex * sizey;
	/* Configure DMA Channel destination address */
	hdma_spi1_tx.Instance->CPAR = (uint32_t) &hspi1.Instance->DR;

	/* Configure DMA Channel source address */
	hdma_spi1_tx.Instance->CMAR = (uint32_t) bitmap;

	/* Disable the transfer half complete interrupt */
	__HAL_DMA_DISABLE_IT(&hdma_spi1_tx, DMA_IT_HT);
	/* Enable the transfer complete interrupt */
	__HAL_DMA_ENABLE_IT(&hdma_spi1_tx, (DMA_IT_TC | DMA_IT_TE));

	/* Enable the Peripherals */
	__HAL_DMA_ENABLE(&hdma_spi1_tx);
	__HAL_SPI_ENABLE(&hspi1);

	/* Enable Tx DMA Request */
	SET_BIT(hspi1.Instance->CR2, SPI_CR2_TXDMAEN);
}
int button_pressed = 0;
extern sitronix_handle_t handle;
uint8_t updated;
uint16_t x0;
uint16_t y0;

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin) {
	UNUSED(GPIO_Pin);

	if (GPIO_Pin == BUTTON_USER_Pin) {
		button_pressed++;
	}
	else if (GPIO_Pin == CPT_INT_Pin) {
		HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
		sitronix_get_coordinates(&handle, &updated, &x0, &y0);
		HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
		//touchgfxSignalVSync();

	}else{
		TE++;
		(&htim2)->Instance->CR1 &= ~(TIM_CR1_CEN);
		(&htim2)->Instance->CNT = 0;
		HAL_IncTick();
		touchgfxSignalVSync();
	}

}

extern uint8_t IntCount;

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin) {

	UNUSED(GPIO_Pin);
	(&htim2)->Instance->CR1 = (TIM_CR1_CEN);
	HAL_IncTick();
	touchgfxSignalVSync();
}

void MB1642BDisplayDriver_DisplayInit(void) {
	uint8_t arguments[4];
	__HAL_SPI_ENABLE(&hspi1);
//  // Sleep out
//  Display_DCS_Send(DCS_EXIT_SLEEP_MODE);
//  HAL_Delay(100);
//
//  // Display Normal mode
//  Display_DCS_Send(DCS_ENTER_NORMAL_MODE);
//  HAL_Delay(100);
//
//  // Display Normal mode   //E2O
	Display_DCS_Send(DCS_ENTER_INVERT_MODE);
	HAL_Delay(100);
//
//  // MADCTL: Exchange RGB / BGR + Mirror X
//  arguments[0] = 0x48; // 0x48
//  Display_DCS_Send_With_Data(DCS_SET_ADDRESS_MODE, arguments, 1);
//  HAL_Delay(100);
//
//  // Pixel Format
////  arguments[0] = 0x05; // RGB565
//  arguments[0] = 0x55; // RGB565  65K //E2O
//  Display_DCS_Send_With_Data(DCS_SET_PIXEL_FORMAT, arguments, 1);
//  HAL_Delay(100);

	Display_DCS_Send(DCS_SOFT_RESET); // software reset comand
	HAL_Delay(100);
	Display_DCS_Send(DCS_SET_DISPLAY_OFF); // display off
	//------------power control------------------------------
//  ILI9341_SendCommand (ILI9341_POWER1); // power control
////   ILI9341_SendData   (0x26); // GVDD = 4.75v
//  ILI9341_SendData   (0x16); // GVDD = 4.75v  // 0x26 to 0x16 (E2O)

	arguments[0] = 0x16; // GVDD = 4.75v
	Display_DCS_Send_With_Data(DCS_POWER1, arguments, 1);

//  ILI9341_SendCommand (ILI9341_POWER2); // power control
//  ILI9341_SendData   (0x11); // AVDD=VCIx2, VGH=VCIx7, VGL=-VCIx3

	arguments[0] = 0x11; // AVDD=VCIx2, VGH=VCIx7, VGL=-VCIx3
	Display_DCS_Send_With_Data(DCS_POWER2, arguments, 1);

	//--------------VCOM-------------------------------------
//  ILI9341_SendCommand (ILI9341_VCOM1); // vcom control
//  ILI9341_SendData   (0x35); // Set the VCOMH voltage (0x35 = 4.025v)
//  ILI9341_SendData   (0x3e); // Set the VCOML voltage (0x3E = -0.950v)
//  ILI9341_SendCommand (ILI9341_VCOM2); // vcom control
//  ILI9341_SendData   (0xbe);

	arguments[0] = 0x35; // Set the VCOMH voltage (0x35 = 4.025v)
	arguments[1] = 0x3E; // Set the VCOML voltage (0x3E = -0.950v)
	Display_DCS_Send_With_Data(DCS_VCOM1, arguments, 2);

	arguments[0] = 0xBE;
	Display_DCS_Send_With_Data(DCS_VCOM2, arguments, 1);

	//------------memory access control------------------------
//  ILI9341_SendCommand (ILI9341_MAC); // memory access control
//  ILI9341_SendData(0x48);

//	arguments[0] = 0x48;
	arguments[0] = 0xE8;	// Aşağıya Bakan
//	arguments[0] = 0x28;	// Yukarıya Bakan
	Display_DCS_Send_With_Data(DCS_SET_ADDRESS_MODE, arguments, 1);

//  ILI9341_SendCommand (ILI9341_PIXEL_FORMAT); // pixel format set
//  ILI9341_SendData   (0x55); // 16bit /pixel

	arguments[0] = 0x55;  // 16bit /pixel
	Display_DCS_Send_With_Data(DCS_SET_PIXEL_FORMAT, arguments, 1);

//	 ILI9341_SendCommand(ILI9341_FRC);
//  ILI9341_SendData(0);
//  ILI9341_SendData(0x1F);

	arguments[0] = 0x00;
	arguments[1] = 0x1F;
	Display_DCS_Send_With_Data(DCS_FRC, arguments, 2);

	//-------------ddram ----------------------------  //????
//  ILI9341_SendCommand (ILI9341_COLUMN_ADDR); // column set
//  ILI9341_SendData   (0x00); // x0_HIGH---0
//  ILI9341_SendData   (0x00); // x0_LOW----0
//  ILI9341_SendData   (0x00); // x1_HIGH---240
//  ILI9341_SendData   (0xEF); // x1_LOW----240

	arguments[0] = 0x00;  // x0_HIGH---0
	arguments[1] = 0x00;  // x0_LOW----0
	arguments[2] = 0x00;  // x1_HIGH---240
	arguments[3] = 0xEF;  // x1_LOW----240
	Display_DCS_Send_With_Data(DCS_SET_COLUMN_ADDRESS, arguments, 4);

//  ILI9341_SendCommand (ILI9341_PAGE_ADDR); // page address set
//  ILI9341_SendData   (0x00); // y0_HIGH---0
//  ILI9341_SendData   (0x00); // y0_LOW----0
//  ILI9341_SendData   (0x01); // y1_HIGH---320
//  ILI9341_SendData   (0x3F); // y1_LOW----320

	arguments[0] = 0x00;  // y0_HIGH---0
	arguments[1] = 0x00;  // y0_LOW----0
	arguments[2] = 0x01;  // y1_HIGH---320
	arguments[3] = 0x3F;  // y1_LOW----320
	Display_DCS_Send_With_Data(DCS_SET_PAGE_ADDRESS, arguments, 4);

//  ILI9341_SendCommand (ILI9341_TEARING_OFF); // tearing effect off
	//LCD_write_cmd(ILI9341_TEARING_ON); // tearing effect on
	//LCD_write_cmd(ILI9341_DISPLAY_INVERSION); // display inversion
//  ILI9341_SendCommand (ILI9341_Entry_Mode_Set); // entry mode set
//  // Deep Standby Mode: OFF
//  // Set the output level of gate driver G1-G320: Normal display
//  // Low voltage detection: Disable
//  ILI9341_SendData   (0x07);

	arguments[0] = 0x07;
	Display_DCS_Send_With_Data(DCS_Entry_Mode_Set, arguments, 1);

	//-----------------display------------------------
//  ILI9341_SendCommand (ILI9341_DFC); // display function control
//  //Set the scan mode in non-display area
//  //Determine source/VCOM output in a non-display area in the partial display mode
//  ILI9341_SendData   (0x0a);
//  //Select whether the liquid crystal type is normally white type or normally black type
//  //Sets the direction of scan by the gate driver in the range determined by SCN and NL
//  //Select the shift direction of outputs from the source driver
//  //Sets the gate driver pin arrangement in combination with the GS bit to select the optimal scan mode for the module
//  //Specify the scan cycle interval of gate driver in non-display area when PTG to select interval scan
//  ILI9341_SendData   (0x82);
//  // Sets the number of lines to drive the LCD at an interval of 8 lines
//  ILI9341_SendData   (0x27);
//  ILI9341_SendData   (0x00); // clock divisor

//  arguments[0] = 0x0A;
//  arguments[1] = 0x82;
//  arguments[2] = 0x27;
//  arguments[3] = 0x00;
//  Display_DCS_Send_With_Data(DCS_DFC, arguments, 4);

//  // Ajusted Scan Direction (E2O)
//  ILI9341_SendCommand (ST7789V_GATECTL);
//  ILI9341_SendData   (0x27);
//  ILI9341_SendData   (0x00);
//  ILI9341_SendData   (0x11);

	arguments[0] = 0x27;
	arguments[1] = 0x00;
	arguments[2] = 0x11;
	Display_DCS_Send_With_Data(DCS_GATECTL, arguments, 3);

//  ILI9341_SendCommand (ILI9341_SLEEP_OUT); // sleep out
	Display_DCS_Send(DCS_EXIT_SLEEP_MODE); // sleep out
	HAL_Delay(100);
//  ILI9341_SendCommand (ILI9341_DISPLAY_ON); // display on
	Display_DCS_Send(DCS_SET_DISPLAY_ON); // display on
	HAL_Delay(100);
//  ILI9341_SendCommand (ILI9341_GRAM); // memory write
	Display_DCS_Send(DCS_WRITE_MEMORY_START); // memory write
	HAL_Delay(5);

	// Tearing effect line on
	arguments[0] = 0; //0x00;
	Display_DCS_Send_With_Data(DCS_SET_TEAR_ON, arguments, 1);
	HAL_Delay(100);

	// Tearing effect scan line
	arguments[0] = 0;
	arguments[1] = 0;
	Display_DCS_Send_With_Data(DCS_SET_TEAR_SCANLINE, arguments, 2);
	HAL_Delay(100);

}

void MB1642BDisplayDriver_DisplayReset(void) {
	HAL_GPIO_WritePin(DISPLAY_RESET_GPIO_Port, DISPLAY_RESET_Pin,
			GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(DISPLAY_RESET_GPIO_Port, DISPLAY_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
}

void MB1642BDisplayDriver_Init(void) {
	/* DMA interrupt init */
	/* DMA1_Channel1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

int touchgfxDisplayDriverTransmitActive(void) {
	return IsTransmittingBlock_;
}

void touchgfxDisplayDriverTransmitBlock(const uint8_t *pixels, uint16_t x,
		uint16_t y, uint16_t w, uint16_t h) {
	Display_Bitmap((uint16_t*) pixels, x, y, w, h);
}

void MB1642BDisplayDriver_DMACallback(void) {
	/* Transfer Complete Interrupt management ***********************************/
	if ((0U != (DMA1->ISR & (DMA_FLAG_TC1)))
			&& (0U != (hdma_spi1_tx.Instance->CCR & DMA_IT_TC))) {
		/* Disable the transfer complete and error interrupt */
		__HAL_DMA_DISABLE_IT(&hdma_spi1_tx, DMA_IT_TE | DMA_IT_TC);

		/* Clear the transfer complete flag */
		__HAL_DMA_CLEAR_FLAG(&hdma_spi1_tx, DMA_FLAG_TC1);

		IsTransmittingBlock_ = 0;

		// Wait until the bus is not busy before changing configuration
		// SPI is busy in communication or Tx buffer is not empty
		while (((hspi1.Instance->SR) & SPI_FLAG_BSY) != RESET) {
		}

		// Set the nCS
		DISPLAY_CSX_GPIO_Port->BSRR = DISPLAY_CSX_Pin;

		// Go back to 8-bit mode
		hspi1.Instance->CR2 = SPI_DATASIZE_8BIT;

		// Signal Transfer Complete to TouchGFX
		DisplayDriver_TransferCompleteCallback();
	}
	/* Transfer Error Interrupt management **************************************/
	else if ((0U != (DMA1->ISR & (DMA_FLAG_TC1)))
			&& (0U != (hdma_spi1_tx.Instance->CCR & DMA_IT_TE))) {
		/* When a DMA transfer error occurs */
		/* A hardware clear of its EN bits is performed */
		/* Disable ALL DMA IT */
		__HAL_DMA_DISABLE_IT(&hdma_spi1_tx,
				(DMA_IT_TC | DMA_IT_HT | DMA_IT_TE));

		/* Clear all flags */
		__HAL_DMA_CLEAR_FLAG(&hdma_spi1_tx, DMA_FLAG_GI1);

		assert(0);  // Halting program - Transfer Error Interrupt received.
	}
}
int touchgfxDisplayDriverShouldTransferBlock(uint16_t bottom) {
	//return (bottom < getCurrentLine());
	return (bottom < (TE > 0 ? 0xFFFF : ((__IO uint16_t) htim2.Instance->CNT)));
}

