/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized. This
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2014 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************/
/*******************************************************************************
* File Name     : resetprg.c
* Device(s)     : RZ/A1H (R7S721001)
* Tool-Chain    : GNUARM-NONEv14.02-EABI
* H/W Platform  : RSK+RZA1H CPU Board
* Description   : Sample Program - C library entry point
*               : Variants of this file must be created for each compiler
*******************************************************************************/
/*******************************************************************************
* History       : DD.MM.YYYY Version Description
*               : 21.10.2014 1.00
*******************************************************************************/
#include "r_typedefs.h"
#include "bsc_userdef.h"
#include "intc.h"
#include "resetprg.h"
#include "stb.h"
#include "gpio.h"
#include "bsc_userdef.h"
//#include "sio_char.h"
#include "compiler_settings.h"

#if defined(__thumb2__) || (defined(__thumb__) && defined(__ARM_ARCH_6M__))
# define THUMB_V7_V6M
#endif

/* Defined if this target supports the BLX Rm instruction.  */
#if  !defined(__ARM_ARCH_2__) && !defined(__ARM_ARCH_3__) && \
                !defined(__ARM_ARCH_3M__)    && !defined(__ARM_ARCH_4__) \
                                                   && !defined(__ARM_ARCH_4T__)
# define HAVE_CALL_INDIRECT
#endif

#ifdef HAVE_INITFINI_ARRAY
#define _init    __libc_init_array
#define _fini    __libc_fini_array
#endif

extern int L1CacheInit(void);

/*******************************************************************************
* Function Name: PowerON_Reset
* Description  :
* Arguments    : none
* Return Value : none
*******************************************************************************/
void PowerON_Reset (void)
{
    STB_Init();

    Port_Init();

    /* BSC setting */
    /* Note BSC_AREA_CS0 & BSC_AREA_CS1 initialised by Peripheral_BasicInit() */
    BSC_Init((uint8_t)(BSC_AREA_CS2 | BSC_AREA_CS3));

    /* INTC setting */
    R_INTC_Init();

    /* Initial setting of the level 1 cache */
    L1CacheInit();

    __enable_irq();
    __enable_fiq();

    main();

	/* Stops program from running off */
    while(1)
    {
		__asm__("nop");
   	}
}

