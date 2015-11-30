/*
 * @brief I2C bus master example using the ROM API in polling mode
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include <string.h>
#include "board.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* I2C master handle and memory for ROM API */
static I2C_HANDLE_T *i2cHandleMaster;

/* Use a buffer size larger than the expected return value of
   i2c_get_mem_size() for the static I2C handle type */
static uint32_t i2cMasterHandleMEM[0x20];

#define SPEED_100KHZ                (100000)
#define I2C_RD_CMD_BIT      (0x01)
/* Standard I2C mode */
#define I2C_MODE    (0)

/* 7-bit I2C addresses */
#if defined(BOARD_NXP_LPCXPRESSO_1549)
#define I2C_ADDR_7BIT_OLCD        0x78		/* This is the 7-bit address shifted up 1-bit (orig 0x48) */
#define I2C_ADDR_7BIT_TEMP_SENSOR 0x90
#endif

#define TICKRATE_HZ         1000

static int i2c_repeat_tout;
static int volatile ticks = 0;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/
/* Turn on LED to indicate an error */
static void I2C_DieError(const char *msg, ErrorCode_t erno)
{
	DEBUGOUT("ERROR:%s: Error number: 0x%X\r\n", msg, erno);
	while(1) {
		__WFI();
	}
}

#if defined(BOARD_NXP_LPCXPRESSO_1549)
static uint8_t init_buffer[] = {I2C_ADDR_7BIT_OLCD, 0x00, 0xA1, 0x00, 0xA5, 0x00, 0xA6, 0x00, 0xAF};
static I2C_PARAM_T *I2C_Get_XferData(void)
{
	static I2C_PARAM_T param;
	static int state;
	static uint8_t buf[5];
	memset(&param, 0, sizeof(param));

	switch (state) {
		/* Initialize the OLCD */
		case 0:
			param.num_bytes_send = sizeof(init_buffer);
			param.buffer_ptr_send = init_buffer;
			state = 1;
			i2c_repeat_tout = 150;
			break;

		case 1:
			init_buffer[1] = 0x40;
			init_buffer[2] = 0x00;
			param.num_bytes_send = 3;
			param.buffer_ptr_send = init_buffer;
			i2c_repeat_tout = 100;
			state = 3;
			break;

		case 3:
			buf[0] = I2C_ADDR_7BIT_TEMP_SENSOR;
			buf[1] = 0;
			param.num_bytes_rec = 3;
			param.num_bytes_send = 2;
			buf[2] = buf[0] | 1;
			param.buffer_ptr_rec = &buf[2];
			param.buffer_ptr_send = &buf[0];
			i2c_repeat_tout = 100;
			state = 4;
			break;

		case 4:
			DEBUGOUT("TEMP VAL: 0x%x\r\n", (buf[3] << 3) | (buf[4] >> 5));
			state = 1;
			return NULL;
	}
	return &param;
}
#endif

/* Initializes pin muxing for I2C interface - note that SystemInit() may
   already setup your pin muxing at system startup */
static void Init_I2C_PinMux(void)
{
#if defined(BOARD_NXP_LPCXPRESSO_1549)
	Chip_SYSCTL_PeriphReset(RESET_I2C0);
	Chip_SWM_FixedPinEnable(SWM_FIXED_I2C0_SDA, 1);
	Chip_SWM_FixedPinEnable(SWM_FIXED_I2C0_SCL, 1);

	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 22, IOCON_STDI2C_EN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 23, IOCON_STDI2C_EN);
	/* Enable the OLCD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 2, IOCON_MODE_INACT);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 2);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, 1, 2);
#else
	#warning "No I2C Pin Muxing defined for board"
#endif
}

/* Setup I2C handle and parameters */
static void I2C_SetupMaster(void)
{
	/* Enable I2C clock and reset I2C peripheral - the boot ROM does not
	   do this */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_I2C0);

	/* Perform a sanity check on the storage allocation */
	if (LPC_I2CD_API->i2c_get_mem_size() > sizeof(i2cMasterHandleMEM)) {
		/* Example only: this should never happen and probably isn't needed for
		   most I2C code. */
		I2C_DieError("Not enough memory allocated", ERR_I2C_GENERAL_FAILURE);
	}

	/* Setup the I2C handle */
	i2cHandleMaster = LPC_I2CD_API->i2c_setup(LPC_I2C_BASE, i2cMasterHandleMEM);
	if (i2cHandleMaster == NULL) {
		I2C_DieError("i2c_setup", ERR_I2C_GENERAL_FAILURE);
	}

	/* Set I2C bitrate */
	if (LPC_I2CD_API->i2c_set_bitrate(i2cHandleMaster, Chip_Clock_GetSystemClockRate(),
									  SPEED_100KHZ) != LPC_OK) {
		I2C_DieError("i2c_set_bitrate", ERR_I2C_GENERAL_FAILURE);
	}
}

static void I2C_MasterXfer(I2C_PARAM_T *param)
{
	ErrorCode_t er = LPC_OK;
	static I2C_RESULT_T res;

	if (!param) return;

	param->stop_flag = 1;

	/* Set timeout (much) greater than the transfer length */
	LPC_I2CD_API->i2c_set_timeout(i2cHandleMaster, 100000);

	if (param->num_bytes_send && !param->num_bytes_rec) { /* Transmit only */
		/* Function is non-blocking, returned error should be LPC_OK, but isn't checked here */
		er = LPC_I2CD_API->i2c_master_transmit_poll(i2cHandleMaster, param, &res);
	} else if (!param->num_bytes_send && param->num_bytes_rec) { /* Receive only */
		er = LPC_I2CD_API->i2c_master_receive_poll(i2cHandleMaster, param, &res);
	} else if (param->num_bytes_send && param->num_bytes_rec) {
		er = LPC_I2CD_API->i2c_master_tx_rx_poll(i2cHandleMaster, param, &res);
	} else { /* Empty data(?) */
		DEBUGSTR("IGNORING EMPTY XFER DATA\r\n");
	}
	if (er != LPC_OK)
		I2C_DieError("I2C_MasterXfer", er);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void)
{
	ticks++;
}

/**
 * @brief	Main routine for I2C example
 * @return	Function should not exit
 */
int main(void)
{
	uint32_t ctick;

	/* Generic Initialization */
	SystemCoreClockUpdate();
	Board_Init();

	/* Clear activity LED */
	Board_LED_Set(0, false);

	/* Setup I2C pin muxing */
	Init_I2C_PinMux();

	/* Allocate I2C handle, setup I2C rate, and initialize I2C
	   clocking */
	I2C_SetupMaster();

	/* Enable SysTick Timer */
	SysTick_Config(SystemCoreClock / TICKRATE_HZ);

	/* Loop forever, toggle LED on other board via I2C */
	while (1) {
		/* Read current temperature from LM75A part over I2C */
		I2C_MasterXfer(I2C_Get_XferData());

		/* Wait for timeout */
		ctick = ticks;
		while (ctick + i2c_repeat_tout > ticks);

		/* Toggle LED to show activity. */
		Board_LED_Toggle(0);
	}

	/* Code never reaches here. Only used to satisfy standard main() */
	return 0;
}
