;/*****************************************************************************
; * @file:    startup_LPC15xx.s
; * @purpose: CMSIS Cortex-M3 Core Device Startup File 
; *           for the NXP LPC15xx Device Series (manually edited)
; * @version: V1.0
; * @date:    25. Nov. 2008
; *------- <<< Use Configuration Wizard in Context Menu >>> ------------------
; *
; * Copyright (C) 2008 ARM Limited. All rights reserved.
; * ARM Limited (ARM) is supplying this software for use with Cortex-M0 
; * processor based microcontrollers.  This file can be freely distributed 
; * within development tools that are supporting such ARM based processors. 
; *
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; *****************************************************************************/

; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000200

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000100

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler

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

                IF      :LNOT::DEF:NO_CRP
                AREA    |.ARM.__at_0x02FC|, CODE, READONLY
CRP_Key         DCD     0xFFFFFFFF
                ENDIF

                AREA    |.text|, CODE, READONLY

; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP

; Dummy Exception Handlers (infinite loops which can be modified)                

; now, under COMMON NMI.c and NMI.h, a real NMI handler is created if NMI is enabled 
; for particular peripheral.
NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP
Reserved_IRQHandler PROC
                EXPORT  Reserved_IRQHandler       [WEAK]
                B       .
                ENDP

Default_Handler PROC
				EXPORT  WDT_IRQHandler            [WEAK]
				EXPORT  BOD_IRQHandler            [WEAK]
				EXPORT  FMC_IRQHandler            [WEAK]
				EXPORT  EEPROM_IRQHandler         [WEAK]
				EXPORT  DMA_IRQHandler            [WEAK]
				EXPORT  GINT0_IRQHandler          [WEAK]
				EXPORT  GINT1_IRQHandler          [WEAK]
				EXPORT  PIN_INT0_IRQHandler       [WEAK]
				EXPORT  PIN_INT1_IRQHandler       [WEAK]
				EXPORT  PIN_INT2_IRQHandler       [WEAK]
				EXPORT  PIN_INT3_IRQHandler       [WEAK]
				EXPORT  PIN_INT4_IRQHandler       [WEAK]
				EXPORT  PIN_INT5_IRQHandler       [WEAK]
				EXPORT  PIN_INT6_IRQHandler       [WEAK]
				EXPORT  PIN_INT7_IRQHandler       [WEAK]
				EXPORT  RIT_IRQHandler            [WEAK]
				EXPORT  SCT0_IRQHandler           [WEAK]
				EXPORT  SCT1_IRQHandler           [WEAK]
				EXPORT  SCT2_IRQHandler           [WEAK]
				EXPORT  SCT3_IRQHandler           [WEAK]
				EXPORT  MRT_IRQHandler            [WEAK]
				EXPORT  UART0_IRQHandler          [WEAK]
				EXPORT  UART1_IRQHandler          [WEAK]
				EXPORT  UART2_IRQHandler          [WEAK]
				EXPORT  I2C0_IRQHandler           [WEAK]
				EXPORT  SPI0_IRQHandler           [WEAK]
				EXPORT  SPI1_IRQHandler           [WEAK]
				EXPORT  CAN_IRQHandler            [WEAK]
				EXPORT  USB_IRQHandler            [WEAK]
				EXPORT  USB_FIQHandler            [WEAK]
				EXPORT  USBWakeup_IRQHandler      [WEAK]
				EXPORT  ADC0A_IRQHandler          [WEAK]
				EXPORT  ADC0B_IRQHandler          [WEAK]
				EXPORT  ADC0_THCMP_IRQHandler     [WEAK]
				EXPORT  ADC0_OVR_IRQHandler       [WEAK]
				EXPORT  ADC1A_IRQHandler          [WEAK]
				EXPORT  ADC1B_IRQHandler          [WEAK]
				EXPORT  ADC1_THCMP_IRQHandler     [WEAK]
				EXPORT  ADC1_OVR_IRQHandler       [WEAK]
				EXPORT  DAC_IRQHandler            [WEAK]
				EXPORT  ACMP0_IRQHandler          [WEAK]
				EXPORT  ACMP1_IRQHandler          [WEAK]
				EXPORT  ACMP2_IRQHandler          [WEAK]
				EXPORT  ACMP3_IRQHandler          [WEAK]
				EXPORT  QEI_IRQHandler            [WEAK]
				EXPORT  RTC_ALARM_IRQHandler      [WEAK]
				EXPORT  RTC_WAKE_IRQHandler       [WEAK]

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

                B       .

                ENDP

                ALIGN

; User Initial Stack & Heap

                IF      :DEF:__MICROLIB
                
                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit
                
                ELSE
                
                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap
__user_initial_stackheap

                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

                ENDIF

                END
