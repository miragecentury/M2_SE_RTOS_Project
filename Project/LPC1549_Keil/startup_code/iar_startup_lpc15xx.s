;/*****************************************************************************
; * @file:    startup_LPC15xx.s
; * @purpose: CMSIS Cortex-M3 Core Device Startup File
; *           for the NXP LPC15xx Device Series (manually edited)
; * @version: V1.00
; * @date:    19. October 2009
; *----------------------------------------------------------------------------
; *
; * Copyright (C) 2009 ARM Limited. All rights reserved.
; *
; * ARM Limited (ARM) is supplying this software for use with Cortex-Mx
; * processor based microcontrollers.  This file can be freely distributed
; * within development tools that are supporting such ARM based processors.
; *
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; ******************************************************************************/

;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        EXTERN  SystemInit
        PUBLIC  __vector_table
        PUBLIC  __vector_table_0x1c
        PUBLIC  __Vectors
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size

        DATA

__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler

        DCD     NMI_Handler
        DCD     HardFault_Handler
        DCD     0
        DCD     0
        DCD     0
__vector_table_0x1c
        DCD     0
        DCD     0
        DCD     0
        DCD     0
        DCD     SVC_Handler
        DCD     0
        DCD     0
        DCD     PendSV_Handler
        DCD     SysTick_Handler

		DCD WDT_IRQHandler                ; WDT interrupt
		DCD BOD_IRQHandler                ; Brown out detector interrupt
		DCD FMC_IRQHandler                ; Flash Memory Controller interrupt
		DCD EEPROM_IRQHandler             ; EEPROM interrupt
		DCD DMA_IRQHandler                ; DMA Controller interrupt
		DCD GINT0_IRQHandler              ; Port interrupt group 0
		DCD GINT1_IRQHandler              ; Port interrupt group 1
		DCD PIN_INT0_IRQHandler           ; Pin interrupt 0
		DCD PIN_INT1_IRQHandler           ; Pin interrupt 1
		DCD PIN_INT2_IRQHandler           ; Pin interrupt 2
		DCD PIN_INT3_IRQHandler           ; Pin interrupt 3
		DCD PIN_INT4_IRQHandler           ; Pin interrupt 4
		DCD PIN_INT5_IRQHandler           ; Pin interrupt 5
		DCD PIN_INT6_IRQHandler           ; Pin interrupt 6
		DCD PIN_INT7_IRQHandler           ; Pin interrupt 7
		DCD RIT_IRQHandler                ; RITimer interrupt
		DCD SCT0_IRQHandler               ; SCT 0 interrupt
		DCD SCT1_IRQHandler               ; SCT 1 interrupt
		DCD SCT2_IRQHandler               ; SCT 2 interrupt
		DCD SCT3_IRQHandler               ; SCT 3 interrupt
		DCD MRT_IRQHandler                ; Multi-rate timer interrupt
		DCD UART0_IRQHandler              ; UART 0 interrupt
		DCD UART1_IRQHandler              ; UART 1 interrupt
		DCD UART2_IRQHandler              ; UART 2 interrupt
		DCD I2C0_IRQHandler               ; I2C1 interrupt
		DCD SPI0_IRQHandler               ; SPI0 interrupt
		DCD SPI1_IRQHandler               ; SPI1 interrupt
		DCD CAN_IRQHandler                ; CAN interrupt
		DCD USB_IRQHandler                ; USB IRQ interrupt
		DCD USB_FIQHandler                ; USB FIQ interrupt
		DCD USBWakeup_IRQHandler          ; USB wake-up interrupt
		DCD ADC0A_IRQHandler              ; ADC0 A sequence (A/D Converter) interrupt
		DCD ADC0B_IRQHandler              ; ADC0 B sequence (A/D Converter) interrupt
		DCD ADC0_THCMP_IRQHandler         ; ADC0 threshold compare interrupt
		DCD ADC0_OVR_IRQHandler           ; ADC0 overrun interrupt
		DCD ADC1A_IRQHandler              ; ADC1 A sequence (A/D Converter) interrupt
		DCD ADC1B_IRQHandler              ; ADC1 B sequence (A/D Converter) interrupt
		DCD ADC1_THCMP_IRQHandler         ; ADC1 threshold compare interrupt
		DCD ADC1_OVR_IRQHandler           ; ADC1 overrun interrupt
		DCD DAC_IRQHandler                ; DAC interrupt
		DCD ACMP0_IRQHandler              ; Analog comparator 0 interrupt
		DCD ACMP1_IRQHandler              ; Analog comparator 1 interrupt
		DCD ACMP2_IRQHandler              ; Analog comparator 2 interrupt
		DCD ACMP3_IRQHandler              ; Analog comparator 3 interrupt
		DCD QEI_IRQHandler                ; QEI interrupt
		DCD RTC_ALARM_IRQHandler          ; RTC alarm interrupt
		DCD RTC_WAKE_IRQHandler           ; RTC wake-up interrupt

__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size 	EQU 	__Vectors_End - __Vectors


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB

        PUBWEAK Reset_Handler
        SECTION .text:CODE:REORDER(2)
Reset_Handler
        LDR     R0, =SystemInit
        BLX     R0
        LDR     R0, =__iar_program_start
        BX      R0

        PUBWEAK NMI_Handler
        SECTION .text:CODE:REORDER(1)
NMI_Handler
        B .

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:REORDER(1)
HardFault_Handler
        B .

        PUBWEAK SVC_Handler
        SECTION .text:CODE:REORDER(1)
SVC_Handler
        B .

        PUBWEAK PendSV_Handler
        SECTION .text:CODE:REORDER(1)
PendSV_Handler
        B .

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:REORDER(1)
SysTick_Handler
        B .

	PUBWEAK Reserved_IRQHandler
        SECTION .text:CODE:REORDER(1)
Reserved_IRQHandler
        B .

		PUBWEAK WDT_IRQHandler
		PUBWEAK BOD_IRQHandler
		PUBWEAK FMC_IRQHandler
		PUBWEAK EEPROM_IRQHandler
		PUBWEAK DMA_IRQHandler
		PUBWEAK GINT0_IRQHandler
		PUBWEAK GINT1_IRQHandler
		PUBWEAK PIN_INT0_IRQHandler
		PUBWEAK PIN_INT1_IRQHandler
		PUBWEAK PIN_INT2_IRQHandler
		PUBWEAK PIN_INT3_IRQHandler
		PUBWEAK PIN_INT4_IRQHandler
		PUBWEAK PIN_INT5_IRQHandler
		PUBWEAK PIN_INT6_IRQHandler
		PUBWEAK PIN_INT7_IRQHandler
		PUBWEAK RIT_IRQHandler
		PUBWEAK SCT0_IRQHandler
		PUBWEAK SCT1_IRQHandler
		PUBWEAK SCT2_IRQHandler
		PUBWEAK SCT3_IRQHandler
		PUBWEAK MRT_IRQHandler
		PUBWEAK UART0_IRQHandler
		PUBWEAK UART1_IRQHandler
		PUBWEAK UART2_IRQHandler
		PUBWEAK I2C0_IRQHandler
		PUBWEAK SPI0_IRQHandler
		PUBWEAK SPI1_IRQHandler
		PUBWEAK CAN_IRQHandler
		PUBWEAK USB_IRQHandler
		PUBWEAK USB_FIQHandler
		PUBWEAK USBWakeup_IRQHandler
		PUBWEAK ADC0A_IRQHandler
		PUBWEAK ADC0B_IRQHandler
		PUBWEAK ADC0_THCMP_IRQHandler
		PUBWEAK ADC0_OVR_IRQHandler
		PUBWEAK ADC1A_IRQHandler
		PUBWEAK ADC1B_IRQHandler
		PUBWEAK ADC1_THCMP_IRQHandler
		PUBWEAK ADC1_OVR_IRQHandler
		PUBWEAK DAC_IRQHandler
		PUBWEAK ACMP0_IRQHandler
		PUBWEAK ACMP1_IRQHandler
		PUBWEAK ACMP2_IRQHandler
		PUBWEAK ACMP3_IRQHandler
		PUBWEAK QEI_IRQHandler
		PUBWEAK RTC_ALARM_IRQHandler
		PUBWEAK RTC_WAKE_IRQHandler

WDT_IRQHandler
BOD_IRQHandler
FMC_IRQHandler
EEPROM_IRQHandler
DMA_IRQHandler
GINT0_IRQHandler
GINT1_IRQHandler
PIN_INT0_IRQHandler
PIN_INT1_IRQHandler
PIN_INT2_IRQHandler
PIN_INT3_IRQHandler
PIN_INT4_IRQHandler
PIN_INT5_IRQHandler
PIN_INT6_IRQHandler
PIN_INT7_IRQHandler
RIT_IRQHandler
SCT0_IRQHandler
SCT1_IRQHandler
SCT2_IRQHandler
SCT3_IRQHandler
MRT_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
UART2_IRQHandler
I2C0_IRQHandler
SPI0_IRQHandler
SPI1_IRQHandler
CAN_IRQHandler
USB_IRQHandler
USB_FIQHandler
USBWakeup_IRQHandler
ADC0A_IRQHandler
ADC0B_IRQHandler
ADC0_THCMP_IRQHandler
ADC0_OVR_IRQHandler
ADC1A_IRQHandler
ADC1B_IRQHandler
ADC1_THCMP_IRQHandler
ADC1_OVR_IRQHandler
DAC_IRQHandler
ACMP0_IRQHandler
ACMP1_IRQHandler
ACMP2_IRQHandler
ACMP3_IRQHandler
QEI_IRQHandler
RTC_ALARM_IRQHandler
RTC_WAKE_IRQHandler

Default_Handler:
        B .

        END
