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
/******************************************************************************
* File Name     : GNU_e2studio-IRAM.ld
* Device(s)     : RZ/A1H (R7S721001)
* Tool-Chain    : GNUARM-NONEv14.02-EABI
* H/W Platform  : RSK+RZA1H CPU Board
* Description   : Linker file for projects that require to run from 
*               : Internal device RAM 
******************************************************************************/
/******************************************************************************
* History       : DD.MM.YYYY Version Description
*               : 21.10.2014 1.00
******************************************************************************/
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(start)

MEMORY 
{
	RAM0 (rwx)    : ORIGIN = 0x20040000, LENGTH = 0x00040000
	RAM1 (rwx)    : ORIGIN = 0x20080000, LENGTH = 0x00080000
	RAM2 (rwx)    : ORIGIN = 0x20100000, LENGTH = 0x00080000
	RAM3 (rwx)    : ORIGIN = 0x20180000, LENGTH = 0x00080000
	RAM4 (rwx)    : ORIGIN = 0x20200000, LENGTH = 0x00100000

	RAM0_M (rwx)  : ORIGIN = 0x60040000, LENGTH = 0x00040000
   RAM1_M (rwx)  : ORIGIN = 0x60080000, LENGTH = 0x00080000
   RAM2_M (rwx)  : ORIGIN = 0x60100000, LENGTH = 0x00080000
   RAM3_M (rwx)  : ORIGIN = 0x60180000, LENGTH = 0x00080000
   RAM4_M (rwx)  : ORIGIN = 0x60200000, LENGTH = 0x00100000

	RAM0123 (rwx) : ORIGIN = 0x20040000, LENGTH = 0x001C0000
}

PROGRAM_STACK_SIZE = 0x8000; 	   /* Application stack                  */
IRQ_STACK_SIZE	    = 0x2000;     /* IRQ mode stack                     */
FIQ_STACK_SIZE	    = 0x2000;     /* FRQ mode stack                     */
SVC_STACK_SIZE	    = 0x2000;     /* SVC mode stack                     */
ABT_STACK_SIZE	    = 0x2000;     /* ABT mode stack                     */
TTB_SIZE           = 0x8000;     /* Level-1 Translation Table for MMU  */

EXEC_BASE          = 0x20040000; /* VECTOR_TABLE located here          */

SECTIONS
{
	.reset EXEC_BASE :
	{
		execute = .;
		*start.o (.text)
		.	= ALIGN(0x4);
		*(.text.startup)
		address_end_reset = .;
	} > RAM0123

	.text :
	{
		text_start = .;
		*(.text)
		address_end_text = .;
	} > RAM0123
	
	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
		. = ALIGN(0x8);
		address_start_data_ROM = .;
		*(.data)
		*(.data.*)
		*(.got.plt)
		*(.got)
		. = ALIGN(0x8);
		address_end_data_ROM = .;
		PROVIDE(end = .);
	} > RAM0123

	.data address_end_data_ROM :
	{
		address_start_data_RAM = .;
		. += (address_end_data_ROM - address_start_data_ROM);
		address_end_data_RAM = .;
    } > RAM0123

	.bss :
	{
         _bss = .;
         PROVIDE(__bss_start__ = .);
         *(.bss)
         *(.bss.**)
         *(COMMON)
		 . = ALIGN(0x4);
         PROVIDE(__bss_end__ = .);
         _ebss = .;
         _end = .;
    } >RAM0123

	.irq_stack : ALIGN(0x4000)
	{
		irq_stack_start = .;
		.  += IRQ_STACK_SIZE;
		.	= ALIGN(0x4);
		irq_stack_end = .;
	} >RAM0123

	.fiq_stack :
	{
		fiq_stack_start = .;
		.  += FIQ_STACK_SIZE;
		.	= ALIGN(0x4);
		fiq_stack_end = .;
	} >RAM0123

	.svc_stack :
	{
		PROVIDE(svc_stack_start = .);
		.  += SVC_STACK_SIZE;
		.	= ALIGN(0x4);
		svc_stack_end = .;
	} >RAM0123

	.abt_stack :
	{
		abt_stack_start = .;
		.  += ABT_STACK_SIZE;
		.	= ALIGN(0x4);
		abt_stack_end = .;
	} >RAM0123

	.program_stack :
	{
		program_stack_start = .;
		.  += PROGRAM_STACK_SIZE;
		.	= ALIGN(0x4);
		program_stack_end = .;
	} >RAM0123

	/* L1 translation table must be aligned to 16KB Boundary!           */
	/* Please refer to Cortex-A Series Version: 4.0 Programmer’s Guide, */
	/* section 9.4 First level address translation                      */
	.ttb_mmu1 : ALIGN(0x4000)
	{
		ttb_mmu1_base = .;
		.  += TTB_SIZE;
		.	= ALIGN(0x4);
		ttb_mmu1_end = .;
	} >RAM0123

	.BSS_DMAC_SAMPLE_INTERNAL_RAM (NOLOAD) :
    {
        *(.BSS_DMAC_SAMPLE_INTERNAL_RAM)
    } >RAM4_M
}