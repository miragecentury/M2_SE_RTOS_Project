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
* File Name     : irqfiq_handler.s
* Device(s)     : RZ/A1H (R7S721001)
* Tool-Chain    : GNUARM-NONEv14.02-EABI
* H/W Platform  : RSK+RZA1H CPU Board
* Description   : Sample Program - IRQ, FIQ handler
*******************************************************************************/
/*******************************************************************************
* History       : DD.MM.YYYY Version Description
*               : 21.10.2014 1.00
/*******************************************************************************/

/* Standard definitions of mode bits and interrupt (I & F) flags in PSRs */
    .equ    SYS_MODE          , 0x1F
    .equ    INTC_ICCIAR_ADDR  , 0xE820200C
    .equ    INTC_ICCEOIR_ADDR , 0xE8202010
    .equ    INTC_ICCHPIR_ADDR , 0xE8202018
    .equ    INTC_ICDIPR0_ADDR , 0xE8201400


/* ================================================================== */
/* Entry point for the FIQ handler */
/* ================================================================== */
    .global  irq_handler
    .global  fiq_handler

fiq_handler:

    LDR     r8, =INTC_ICDIPR0_ADDR // force recalculation of highest priority interrupt
    LDR     r9, [r8]               // read interrupt priority register 0 into r9 - errata 733075 for GIC
    STR     r9, [r8]               // write back the same value
    DSB                            // ensure write completes before other ICCHIPR or ICCIAR register reads

    SUB     lr, lr, #4             // adjust the return address for exception return
    SRSDB   sp!, #SYS_MODE         // save LR_fiq and PSR_fiq on the SYSTEM mode stack

    CPS     #SYS_MODE              // change to system mode
    PUSH    {r0-r3, r12}           // save AACPS stack on system mode stack
                                   // since the handler is a C function

    AND     r0, sp, #4             // align stack pointer to 8 bytes (could be skipped since FIQ is not nested?)
    SUB     sp, sp, r0             // adjust as necessary
    PUSH    {r0, lr}               // store adjustment and lr to SYSTEM stack

    BL      fiq_handler_interrupt  // execute handler

    POP     {r0, lr}               // restore LR_sys and r0
    ADD     sp, sp, r0             // un-adjust stack

    POP     {r0-r3, r12}           // restore AACPS registers

fiq_handler_end:

    LDR     r8, =INTC_ICCHPIR_ADDR // load highest priority pending interrupt register address
    LDR     r9, [r8]               // read into r9 - errata 801120 for GIC
                                   // can discard the result
                                   // do this here in case FIQ preempts IRQ just before ICCHPIR read

    RFEFD   sp!

/* ================================================================== */
/* Entry point for the IRQ handler */
/* ================================================================== */
    .func irq_handler
irq_handler:

    SUB     lr, lr, #4             // adjust the return address for exception return
    SRSDB   sp!, #SYS_MODE         // save LR_irq and PSR_irq on the SYSTEM mode stack

    CPS     #SYS_MODE              // change to system mode
    PUSH    {r0-r3, r12}           // save AACPS stack on system mode stack

    LDR     r2, =INTC_ICDIPR0_ADDR // force recalculation of highest priority interrupt
    LDR     r0, [r2]               // read interrupt priority register 0 into r0 - errata 733075 for GIC
    STR     r0, [r2]               // write back the same value
    DSB                            // ensure write completes before other ICCHIPR or ICCIAR register reads

    LDR     r2, =INTC_ICCHPIR_ADDR // load highest priority pending interrupt register address
    LDR     r0, [r2]               // read into r0 - errata 801120 for GIC
                                   // can discard the result

    LDR     r2, =INTC_ICCIAR_ADDR  // load interrupt acknowledge register address
    LDR     r0, [r2]               // read into r0
    PUSH    {r0}                   // store r0 on the system mode stack (interrupt id, parameter for the handler)

    AND     r1, sp, #4             // align stack pointer to 8 bytes
    SUB     sp, sp, r1             // adjust as necessary
    PUSH    {r1, lr}               // store adjustment and lr to SYSTEM stack

    BL      intc_handler_interrupt // branch to IRQ handler

    POP     {r1, lr}               // restore LR_sys
    ADD     sp, sp, r1             // un-adjust stack

    POP     {r0}                   // get interrupt id back from stack

    LDR     r2, =INTC_ICCEOIR_ADDR // load end of interrupt register address
    STR     r0, [r2]               // quit interrupt by writing id

    POP     {r0-r3, r12}           // restore AACPS registers

irq_handler_end:

    RFEFD   sp!                    // return from the SYSTEM mode stack


Literals3:

    .endfunc
    .end
