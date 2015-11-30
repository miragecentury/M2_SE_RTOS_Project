/*****************************************************************************/
/* Startup_XMC4500.s: Startup file for XMC4500 device series for EWARM                */
/*****************************************************************************/
/**
* @file     Startup_XMC4500.s
*           XMC4000 Device Series
* @version  V1.0
* @date     Jan 2013
*
* Copyright (C) 2012 IAR Systems. All rights reserved.
* Copyright (C) 2012 Infineon Technologies AG. All rights reserved.
*
*
* @par
* Infineon Technologies AG (Infineon) is supplying this software for use with 
* Infineon's microcontrollers.  This file can be freely distributed
* within development tools that are supporting such microcontrollers.
*
* @par
* THIS SOFTWARE IS PROVIDED AS IS.  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
* ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*
******************************************************************************/
/* ********************* Version History *********************************** */
/* ***************************************************************************
V1.0 January, 30 2013:  In ths version a workoraound for the erratum PMU_CM.001
is implmented (patch for the Exception and interrupt handlers)

**************************************************************************** */

        MODULE  ?vector_table

        AAPCS INTERWORK, VFP_COMPATIBLE, RWPI_COMPATIBLE
        PRESERVE8


        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        EXTERN  SystemInit  
        PUBLIC  __vector_table

        DATA

__iar_init$$done:               ; The vector table is not needed
                                ; until after copy initialization is done

__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler

        DCD     NMI_Handler_proxy
        DCD     HardFault_Handler_proxy
        DCD     MemManage_Handler_proxy
        DCD     BusFault_Handler_proxy
        DCD     UsageFault_Handler_proxy
        DCD     0
        DCD     0
        DCD     0
        DCD     0
        DCD     SVC_Handler_proxy
        DCD     DebugMon_Handler_proxy
        DCD     0
        DCD     PendSV_Handler_proxy
        DCD     SysTick_Handler_proxy

    ; Interrupt Handlers for Service Requests (SR) from XMC4500 Peripherals
	    DCD   SCU_0_IRQHandler_proxy            ; Handler name for SR SCU_0
	    DCD   ERU0_0_IRQHandler_proxy           ; Handler name for SR ERU0_0
	    DCD   ERU0_1_IRQHandler_proxy           ; Handler name for SR ERU0_1
	    DCD   ERU0_2_IRQHandler_proxy           ; Handler name for SR ERU0_2
	    DCD   ERU0_3_IRQHandler_proxy           ; Handler name for SR ERU0_3
	    DCD   ERU1_0_IRQHandler_proxy           ; Handler name for SR ERU1_0
	    DCD   ERU1_1_IRQHandler_proxy           ; Handler name for SR ERU1_1
	    DCD   ERU1_2_IRQHandler_proxy           ; Handler name for SR ERU1_2
	    DCD   ERU1_3_IRQHandler_proxy           ; Handler name for SR ERU1_3
	    DCD   0                           ; Not Available
	    DCD   0                           ; Not Available
	    DCD   0                           ; Not Available
	    DCD   PMU0_0_IRQHandler_proxy           ; Handler name for SR PMU0_0
	    DCD   0                           ; Handler name for SR PMU0_1
	    DCD   VADC0_C0_0_IRQHandler_proxy       ; Handler name for SR VADC0_C0_0
	    DCD   VADC0_C0_1_IRQHandler_proxy       ; Handler name for SR VADC0_C0_1
	    DCD   VADC0_C0_2_IRQHandler_proxy       ; Handler name for SR VADC0_C0_1
	    DCD   VADC0_C0_3_IRQHandler_proxy       ; Handler name for SR VADC0_C0_3
	    DCD   VADC0_G0_0_IRQHandler_proxy       ; Handler name for SR VADC0_G0_0
	    DCD   VADC0_G0_1_IRQHandler_proxy       ; Handler name for SR VADC0_G0_1
	    DCD   VADC0_G0_2_IRQHandler_proxy       ; Handler name for SR VADC0_G0_2
	    DCD   VADC0_G0_3_IRQHandler_proxy       ; Handler name for SR VADC0_G0_3
	    DCD   VADC0_G1_0_IRQHandler_proxy       ; Handler name for SR VADC0_G1_0
	    DCD   VADC0_G1_1_IRQHandler_proxy       ; Handler name for SR VADC0_G1_1
	    DCD   VADC0_G1_2_IRQHandler_proxy       ; Handler name for SR VADC0_G1_2
	    DCD   VADC0_G1_3_IRQHandler_proxy       ; Handler name for SR VADC0_G1_3
	    DCD   VADC0_G2_0_IRQHandler_proxy       ; Handler name for SR VADC0_G2_0
	    DCD   VADC0_G2_1_IRQHandler_proxy       ; Handler name for SR VADC0_G2_1
	    DCD   VADC0_G2_2_IRQHandler_proxy       ; Handler name for SR VADC0_G2_2
	    DCD   VADC0_G2_3_IRQHandler_proxy       ; Handler name for SR VADC0_G2_3
	    DCD   VADC0_G3_0_IRQHandler_proxy       ; Handler name for SR VADC0_G3_0
	    DCD   VADC0_G3_1_IRQHandler_proxy       ; Handler name for SR VADC0_G3_1
	    DCD   VADC0_G3_2_IRQHandler_proxy       ; Handler name for SR VADC0_G3_2
	    DCD   VADC0_G3_3_IRQHandler_proxy       ; Handler name for SR VADC0_G3_3
	    DCD   DSD0_0_IRQHandler_proxy           ; Handler name for SR DSD0_0
	    DCD   DSD0_1_IRQHandler_proxy           ; Handler name for SR DSD0_1
	    DCD   DSD0_2_IRQHandler_proxy           ; Handler name for SR DSD0_2
	    DCD   DSD0_3_IRQHandler_proxy           ; Handler name for SR DSD0_3
	    DCD   DSD0_4_IRQHandler_proxy           ; Handler name for SR DSD0_4
	    DCD   DSD0_5_IRQHandler_proxy           ; Handler name for SR DSD0_5
	    DCD   DSD0_6_IRQHandler_proxy           ; Handler name for SR DSD0_6
	    DCD   DSD0_7_IRQHandler_proxy           ; Handler name for SR DSD0_7
	    DCD   DAC0_0_IRQHandler_proxy           ; Handler name for SR DAC0_0
	    DCD   DAC0_1_IRQHandler_proxy           ; Handler name for SR DAC0_0
	    DCD   CCU40_0_IRQHandler_proxy          ; Handler name for SR CCU40_0
	    DCD   CCU40_1_IRQHandler_proxy          ; Handler name for SR CCU40_1
	    DCD   CCU40_2_IRQHandler_proxy          ; Handler name for SR CCU40_2
	    DCD   CCU40_3_IRQHandler_proxy          ; Handler name for SR CCU40_3
	    DCD   CCU41_0_IRQHandler_proxy          ; Handler name for SR CCU41_0
	    DCD   CCU41_1_IRQHandler_proxy          ; Handler name for SR CCU41_1
	    DCD   CCU41_2_IRQHandler_proxy          ; Handler name for SR CCU41_2
	    DCD   CCU41_3_IRQHandler_proxy          ; Handler name for SR CCU41_3
	    DCD   CCU42_0_IRQHandler_proxy          ; Handler name for SR CCU42_0
	    DCD   CCU42_1_IRQHandler_proxy          ; Handler name for SR CCU42_1
	    DCD   CCU42_2_IRQHandler_proxy          ; Handler name for SR CCU42_2
	    DCD   CCU42_3_IRQHandler_proxy          ; Handler name for SR CCU42_3
	    DCD   CCU43_0_IRQHandler_proxy          ; Handler name for SR CCU43_0
	    DCD   CCU43_1_IRQHandler_proxy          ; Handler name for SR CCU43_1
	    DCD   CCU43_2_IRQHandler_proxy          ; Handler name for SR CCU43_2
	    DCD   CCU43_3_IRQHandler_proxy          ; Handler name for SR CCU43_3
	    DCD   CCU80_0_IRQHandler_proxy          ; Handler name for SR CCU80_0
	    DCD   CCU80_1_IRQHandler_proxy          ; Handler name for SR CCU80_1
	    DCD   CCU80_2_IRQHandler_proxy          ; Handler name for SR CCU80_2
	    DCD   CCU80_3_IRQHandler_proxy          ; Handler name for SR CCU80_3
	    DCD   CCU81_0_IRQHandler_proxy          ; Handler name for SR CCU81_0
	    DCD   CCU81_1_IRQHandler_proxy          ; Handler name for SR CCU81_1
	    DCD   CCU81_2_IRQHandler_proxy          ; Handler name for SR CCU81_2
	    DCD   CCU81_3_IRQHandler_proxy          ; Handler name for SR CCU81_3
	    DCD   POSIF0_0_IRQHandler_proxy         ; Handler name for SR POSIF0_0
	    DCD   POSIF0_1_IRQHandler_proxy         ; Handler name for SR POSIF0_1
	    DCD   POSIF1_0_IRQHandler_proxy         ; Handler name for SR POSIF1_0
	    DCD   POSIF1_1_IRQHandler_proxy         ; Handler name for SR POSIF1_1
	    DCD   0                           ; Not Available
	    DCD   0                           ; Not Available
	    DCD   0                           ; Not Available
	    DCD   0                           ; Not Available
	    DCD   CAN0_0_IRQHandler_proxy           ; Handler name for SR CAN0_0
	    DCD   CAN0_1_IRQHandler_proxy           ; Handler name for SR CAN0_1
	    DCD   CAN0_2_IRQHandler_proxy           ; Handler name for SR CAN0_2
	    DCD   CAN0_3_IRQHandler_proxy           ; Handler name for SR CAN0_3
	    DCD   CAN0_4_IRQHandler_proxy           ; Handler name for SR CAN0_4
	    DCD   CAN0_5_IRQHandler_proxy           ; Handler name for SR CAN0_5
	    DCD   CAN0_6_IRQHandler_proxy           ; Handler name for SR CAN0_6
	    DCD   CAN0_7_IRQHandler_proxy           ; Handler name for SR CAN0_7
	    DCD   USIC0_0_IRQHandler_proxy          ; Handler name for SR USIC0_0
	    DCD   USIC0_1_IRQHandler_proxy          ; Handler name for SR USIC0_1
	    DCD   USIC0_2_IRQHandler_proxy          ; Handler name for SR USIC0_2
	    DCD   USIC0_3_IRQHandler_proxy          ; Handler name for SR USIC0_3
	    DCD   USIC0_4_IRQHandler_proxy          ; Handler name for SR USIC0_4
	    DCD   USIC0_5_IRQHandler_proxy          ; Handler name for SR USIC0_5
	    DCD   USIC1_0_IRQHandler_proxy          ; Handler name for SR USIC1_0
	    DCD   USIC1_1_IRQHandler_proxy          ; Handler name for SR USIC1_1
	    DCD   USIC1_2_IRQHandler_proxy          ; Handler name for SR USIC1_2
	    DCD   USIC1_3_IRQHandler_proxy          ; Handler name for SR USIC1_3
	    DCD   USIC1_4_IRQHandler_proxy          ; Handler name for SR USIC1_4
	    DCD   USIC1_5_IRQHandler_proxy          ; Handler name for SR USIC1_5
	    DCD   USIC2_0_IRQHandler_proxy          ; Handler name for SR USIC2_0
	    DCD   USIC2_1_IRQHandler_proxy          ; Handler name for SR USIC2_1
	    DCD   USIC2_2_IRQHandler_proxy          ; Handler name for SR USIC2_2
	    DCD   USIC2_3_IRQHandler_proxy          ; Handler name for SR USIC2_3
	    DCD   USIC2_4_IRQHandler_proxy          ; Handler name for SR USIC2_4
	    DCD   USIC2_5_IRQHandler_proxy          ; Handler name for SR USIC2_5
	    DCD   LEDTS0_0_IRQHandler_proxy         ; Handler name for SR LEDTS0_0
	    DCD   0                           ; Not Available
	    DCD   FCE0_0_IRQHandler_proxy           ; Handler name for SR FCE0_0
	    DCD   GPDMA0_0_IRQHandler_proxy         ; Handler name for SR GPDMA0_0
	    DCD   SDMMC0_0_IRQHandler_proxy         ; Handler name for SR SDMMC0_0
	    DCD   USB0_0_IRQHandler_proxy           ; Handler name for SR USB0_0
	    DCD   ETH0_0_IRQHandler_proxy           ; Handler name for SR ETH0_0
	    DCD   0                           ; Not Available
	    DCD   GPDMA1_0_IRQHandler_proxy         ; Handler name for SR GPDMA1_0
	    DCD   0                           ; Not Available




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
        LDR     R0, =SystemInit_DAVE3
        BLX     R0  
        LDR     R0, =__iar_program_start
        BX      R0 

        PUBWEAK NMI_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
NMI_Handler
        B NMI_Handler

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
HardFault_Handler
        B HardFault_Handler
               
        PUBWEAK MemManage_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
MemManage_Handler
        B MemManage_Handler
                
        PUBWEAK BusFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
BusFault_Handler
        B BusFault_Handler
                
        PUBWEAK UsageFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
UsageFault_Handler
        B UsageFault_Handler
                
        PUBWEAK SVC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SVC_Handler
        B SVC_Handler
                
        PUBWEAK DebugMon_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
DebugMon_Handler
        B DebugMon_Handler
                
        PUBWEAK PendSV_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
PendSV_Handler
        B PendSV_Handler        
        
        PUBWEAK SysTick_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SysTick_Handler
        B SysTick_Handler
               
;; XMC4500 interrupt handlers

        PUBWEAK   SCU_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
SCU_0_IRQHandler
        B SCU_0_IRQHandler

        PUBWEAK   ERU0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
ERU0_0_IRQHandler
        B ERU0_0_IRQHandler
	    
        PUBWEAK   ERU0_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
ERU0_1_IRQHandler
        B ERU0_1_IRQHandler	    
	    
        PUBWEAK   ERU0_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
ERU0_2_IRQHandler
        B ERU0_2_IRQHandler	    
	    
        PUBWEAK   ERU0_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
ERU0_3_IRQHandler
        B ERU0_3_IRQHandler	    
	    
        PUBWEAK   ERU1_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
ERU1_0_IRQHandler
        B ERU1_0_IRQHandler	    
	    
        PUBWEAK   ERU1_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
ERU1_1_IRQHandler
        B ERU1_1_IRQHandler	    
	    
        PUBWEAK   ERU1_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
ERU1_2_IRQHandler
        B ERU1_2_IRQHandler	    
	    
        PUBWEAK   ERU1_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
ERU1_3_IRQHandler
        B ERU1_3_IRQHandler	    
	    
        PUBWEAK   PMU0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
PMU0_0_IRQHandler
        B PMU0_0_IRQHandler	    
	    
        PUBWEAK   PMU0_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
PMU0_1_IRQHandler
        B PMU0_1_IRQHandler	    
	    
        PUBWEAK   VADC0_C0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_C0_0_IRQHandler
        B VADC0_C0_0_IRQHandler	    
	    
        PUBWEAK   VADC0_C0_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_C0_1_IRQHandler
        B VADC0_C0_1_IRQHandler	    
	    
        PUBWEAK   VADC0_C0_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_C0_2_IRQHandler
        B VADC0_C0_2_IRQHandler	    
	    
        PUBWEAK   VADC0_C0_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_C0_3_IRQHandler
        B VADC0_C0_3_IRQHandler	    
	    
        PUBWEAK   VADC0_G0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G0_0_IRQHandler
        B VADC0_G0_0_IRQHandler	    
	    
        PUBWEAK   VADC0_G0_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G0_1_IRQHandler
        B VADC0_G0_1_IRQHandler	    
	    
        PUBWEAK   VADC0_G0_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G0_2_IRQHandler
        B VADC0_G0_2_IRQHandler	    
	    
        PUBWEAK   VADC0_G0_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G0_3_IRQHandler
        B VADC0_G0_3_IRQHandler	    
	    
        PUBWEAK   VADC0_G1_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G1_0_IRQHandler
        B VADC0_G1_0_IRQHandler	    
	    
        PUBWEAK   VADC0_G1_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G1_1_IRQHandler
        B VADC0_G1_1_IRQHandler	    
	    
        PUBWEAK   VADC0_G1_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G1_2_IRQHandler
        B VADC0_G1_2_IRQHandler	    
	    
        PUBWEAK   VADC0_G1_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G1_3_IRQHandler
        B VADC0_G1_3_IRQHandler	    
	    
        PUBWEAK   VADC0_G2_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G2_0_IRQHandler
        B VADC0_G2_0_IRQHandler	    
	    
        PUBWEAK   VADC0_G2_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G2_1_IRQHandler
        B VADC0_G2_1_IRQHandler	    
	    
        PUBWEAK   VADC0_G2_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G2_2_IRQHandler
        B VADC0_G2_2_IRQHandler	    
	    
        PUBWEAK   VADC0_G2_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G2_3_IRQHandler
        B VADC0_G2_3_IRQHandler	    
	    
        PUBWEAK   VADC0_G3_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G3_0_IRQHandler
        B VADC0_G3_0_IRQHandler	    
	    
        PUBWEAK   VADC0_G3_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G3_1_IRQHandler
        B VADC0_G3_1_IRQHandler	    
	    
        PUBWEAK   VADC0_G3_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G3_2_IRQHandler
        B VADC0_G3_2_IRQHandler	    
	    
        PUBWEAK   VADC0_G3_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
VADC0_G3_3_IRQHandler
        B VADC0_G3_3_IRQHandler	    
	    
        PUBWEAK   DSD0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DSD0_0_IRQHandler
        B DSD0_0_IRQHandler	    
	    
        PUBWEAK   DSD0_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DSD0_1_IRQHandler
        B DSD0_1_IRQHandler	    
	    
        PUBWEAK   DSD0_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DSD0_2_IRQHandler
        B DSD0_2_IRQHandler	    
	    
        PUBWEAK   DSD0_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DSD0_3_IRQHandler
        B DSD0_3_IRQHandler	    
	    
        PUBWEAK   DSD0_4_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DSD0_4_IRQHandler
        B DSD0_4_IRQHandler	    
	    
        PUBWEAK   DSD0_5_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DSD0_5_IRQHandler
        B DSD0_5_IRQHandler	    
	    
        PUBWEAK   DSD0_6_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DSD0_6_IRQHandler
        B DSD0_6_IRQHandler	    
	    
        PUBWEAK   DSD0_7_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DSD0_7_IRQHandler
        B DSD0_7_IRQHandler	    
	    
        PUBWEAK   DAC0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DAC0_0_IRQHandler
        B DAC0_0_IRQHandler	    
	    
        PUBWEAK   DAC0_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DAC0_1_IRQHandler
        B DAC0_1_IRQHandler	    
	    
        PUBWEAK   CCU40_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU40_0_IRQHandler
        B CCU40_0_IRQHandler	    
	    
        PUBWEAK   CCU40_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU40_1_IRQHandler
        B CCU40_1_IRQHandler	    
	    
        PUBWEAK   CCU40_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU40_2_IRQHandler
        B CCU40_2_IRQHandler	    
	    
        PUBWEAK   CCU40_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU40_3_IRQHandler
        B CCU40_3_IRQHandler	    
	    
        PUBWEAK   CCU41_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU41_0_IRQHandler
        B CCU41_0_IRQHandler	    
	    
        PUBWEAK   CCU41_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU41_1_IRQHandler
        B CCU41_1_IRQHandler	    
	    
        PUBWEAK   CCU41_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU41_2_IRQHandler
        B CCU41_2_IRQHandler	    
	    
        PUBWEAK   CCU41_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU41_3_IRQHandler
        B CCU41_3_IRQHandler	    
	    
        PUBWEAK   CCU42_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU42_0_IRQHandler
        B CCU42_0_IRQHandler	    
	    
        PUBWEAK   CCU42_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU42_1_IRQHandler
        B CCU42_1_IRQHandler	    
	    
        PUBWEAK   CCU42_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU42_2_IRQHandler
        B CCU42_2_IRQHandler	    
    
        PUBWEAK   CCU42_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU42_3_IRQHandler
        B CCU42_3_IRQHandler	    
	    
        PUBWEAK   CCU43_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU43_0_IRQHandler
        B CCU43_0_IRQHandler	    
	    
        PUBWEAK   CCU43_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU43_1_IRQHandler
        B CCU43_1_IRQHandler	    
	    
        PUBWEAK   CCU43_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU43_2_IRQHandler
        B CCU43_2_IRQHandler	    
	    
        PUBWEAK   CCU43_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU43_3_IRQHandler
        B CCU43_3_IRQHandler	    
	    
        PUBWEAK   CCU80_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU80_0_IRQHandler
        B CCU80_0_IRQHandler	    
	    
        PUBWEAK   CCU80_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU80_1_IRQHandler
        B CCU80_1_IRQHandler	    
	    
        PUBWEAK   CCU80_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU80_2_IRQHandler
        B CCU80_2_IRQHandler	    
	    
        PUBWEAK   CCU80_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU80_3_IRQHandler
        B CCU80_3_IRQHandler	    
	    
        PUBWEAK   CCU81_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU81_0_IRQHandler
        B CCU81_0_IRQHandler	    
	    
        PUBWEAK   CCU81_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU81_1_IRQHandler
        B CCU81_1_IRQHandler	    
	    
        PUBWEAK   CCU81_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU81_2_IRQHandler
        B CCU81_2_IRQHandler	    
	    
        PUBWEAK   CCU81_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CCU81_3_IRQHandler
        B CCU81_3_IRQHandler	    
	    
        PUBWEAK   POSIF0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
POSIF0_0_IRQHandler
        B POSIF0_0_IRQHandler	    
	    
        PUBWEAK   POSIF0_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
POSIF0_1_IRQHandler
        B POSIF0_1_IRQHandler	    
	    
        PUBWEAK   POSIF1_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
POSIF1_0_IRQHandler
        B POSIF1_0_IRQHandler	    
	    
        PUBWEAK   POSIF1_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
POSIF1_1_IRQHandler
        B POSIF1_1_IRQHandler	    
	    
        PUBWEAK   CAN0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CAN0_0_IRQHandler
        B CAN0_0_IRQHandler	    
	    
        PUBWEAK   CAN0_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CAN0_1_IRQHandler
        B CAN0_1_IRQHandler	    
	    
        PUBWEAK   CAN0_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CAN0_2_IRQHandler
        B CAN0_2_IRQHandler	    
	    
        PUBWEAK   CAN0_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CAN0_3_IRQHandler
        B CAN0_3_IRQHandler	    
	    
        PUBWEAK   CAN0_4_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CAN0_4_IRQHandler
        B CAN0_4_IRQHandler	    
	    
        PUBWEAK   CAN0_5_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CAN0_5_IRQHandler
        B CAN0_5_IRQHandler	    
	    
        PUBWEAK   CAN0_6_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CAN0_6_IRQHandler
        B CAN0_6_IRQHandler	    
	    
        PUBWEAK   CAN0_7_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CAN0_7_IRQHandler
        B CAN0_7_IRQHandler	    
	    
        PUBWEAK   USIC0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC0_0_IRQHandler
        B USIC0_0_IRQHandler	    
	    
        PUBWEAK   USIC0_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC0_1_IRQHandler
        B USIC0_1_IRQHandler	    
	    
        PUBWEAK   USIC0_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC0_2_IRQHandler
        B USIC0_2_IRQHandler	    
	    
        PUBWEAK   USIC0_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC0_3_IRQHandler
        B USIC0_3_IRQHandler	    
	    
        PUBWEAK   USIC0_4_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC0_4_IRQHandler
        B USIC0_4_IRQHandler	    
	    
        PUBWEAK   USIC0_5_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC0_5_IRQHandler
        B USIC0_5_IRQHandler	    
	    
        PUBWEAK   USIC1_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC1_0_IRQHandler
        B USIC1_0_IRQHandler
	    
        PUBWEAK   USIC1_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC1_1_IRQHandler
        B USIC1_1_IRQHandler	    
	    
        PUBWEAK   USIC1_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC1_2_IRQHandler
        B USIC1_2_IRQHandler	    
	    
        PUBWEAK   USIC1_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC1_3_IRQHandler
        B USIC1_3_IRQHandler	    
	    
        PUBWEAK   USIC1_4_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC1_4_IRQHandler
        B USIC1_4_IRQHandler	    
	    
        PUBWEAK   USIC1_5_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC1_5_IRQHandler
        B USIC1_5_IRQHandler	    
	    
        PUBWEAK   USIC2_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC2_0_IRQHandler
        B USIC2_0_IRQHandler	    
	    
        PUBWEAK   USIC2_1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC2_1_IRQHandler
        B USIC2_1_IRQHandler	    
	    
        PUBWEAK   USIC2_2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC2_2_IRQHandler
        B USIC2_2_IRQHandler	    
	    
        PUBWEAK   USIC2_3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC2_3_IRQHandler
        B USIC2_3_IRQHandler	    
	    
        PUBWEAK   USIC2_4_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC2_4_IRQHandler
        B USIC2_4_IRQHandler	    
	    
        PUBWEAK   USIC2_5_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USIC2_5_IRQHandler
        B USIC2_5_IRQHandler	    
	    
        PUBWEAK   LEDTS0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
LEDTS0_0_IRQHandler
        B LEDTS0_0_IRQHandler	    
	    
        PUBWEAK   FCE0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FCE0_0_IRQHandler
        B FCE0_0_IRQHandler	    
	    
        PUBWEAK   GPDMA0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
GPDMA0_0_IRQHandler
        B GPDMA0_0_IRQHandler	    
	    
        PUBWEAK   SDMMC0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
SDMMC0_0_IRQHandler
        B SDMMC0_0_IRQHandler	    
	    
        PUBWEAK   USB0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USB0_0_IRQHandler
        B USB0_0_IRQHandler	    
	    
        PUBWEAK   ETH0_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
ETH0_0_IRQHandler
        B ETH0_0_IRQHandler	    
	    
        PUBWEAK   GPDMA1_0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
GPDMA1_0_IRQHandler
        B GPDMA1_0_IRQHandler
 
 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;Proxy function to fix CPU prefetch bug
;;

ProxyHandler  macro
              PUBWEAK \1_proxy
              SECTION .text:CODE:REORDER:NOROOT(2)
\1_proxy
              LDR   R0, =\1
              PUSH	{LR}
              BLX   R0
              POP   {PC}
              endm

        ProxyHandler NMI_Handler
        ProxyHandler HardFault_Handler
        ProxyHandler MemManage_Handler
        ProxyHandler BusFault_Handler
        ProxyHandler UsageFault_Handler
        ProxyHandler SVC_Handler
        ProxyHandler DebugMon_Handler
        ProxyHandler PendSV_Handler
        ProxyHandler SysTick_Handler
          
        ProxyHandler SCU_0_IRQHandler
        ProxyHandler ERU0_0_IRQHandler
        ProxyHandler ERU0_1_IRQHandler
        ProxyHandler ERU0_2_IRQHandler
        ProxyHandler ERU0_3_IRQHandler
        ProxyHandler ERU1_0_IRQHandler
        ProxyHandler ERU1_1_IRQHandler
        ProxyHandler ERU1_2_IRQHandler
        ProxyHandler ERU1_3_IRQHandler
        ProxyHandler PMU0_0_IRQHandler
        ProxyHandler PMU0_1_IRQHandler
        ProxyHandler VADC0_C0_0_IRQHandler
        ProxyHandler VADC0_C0_1_IRQHandler
        ProxyHandler VADC0_C0_2_IRQHandler
        ProxyHandler VADC0_C0_3_IRQHandler
        ProxyHandler VADC0_G0_0_IRQHandler
        ProxyHandler VADC0_G0_1_IRQHandler
        ProxyHandler VADC0_G0_2_IRQHandler
        ProxyHandler VADC0_G0_3_IRQHandler
        ProxyHandler VADC0_G1_0_IRQHandler
        ProxyHandler VADC0_G1_1_IRQHandler
        ProxyHandler VADC0_G1_2_IRQHandler
        ProxyHandler VADC0_G1_3_IRQHandler
        ProxyHandler VADC0_G2_0_IRQHandler
        ProxyHandler VADC0_G2_1_IRQHandler
        ProxyHandler VADC0_G2_2_IRQHandler
        ProxyHandler VADC0_G2_3_IRQHandler
        ProxyHandler VADC0_G3_0_IRQHandler
        ProxyHandler VADC0_G3_1_IRQHandler
        ProxyHandler VADC0_G3_2_IRQHandler
        ProxyHandler VADC0_G3_3_IRQHandler
        ProxyHandler DSD0_0_IRQHandler
        ProxyHandler DSD0_1_IRQHandler
        ProxyHandler DSD0_2_IRQHandler
        ProxyHandler DSD0_3_IRQHandler
        ProxyHandler DSD0_4_IRQHandler
        ProxyHandler DSD0_5_IRQHandler
        ProxyHandler DSD0_6_IRQHandler
        ProxyHandler DSD0_7_IRQHandler
        ProxyHandler DAC0_0_IRQHandler
        ProxyHandler DAC0_1_IRQHandler
        ProxyHandler CCU40_0_IRQHandler
        ProxyHandler CCU40_1_IRQHandler
        ProxyHandler CCU40_2_IRQHandler
        ProxyHandler CCU40_3_IRQHandler
        ProxyHandler CCU41_0_IRQHandler
        ProxyHandler CCU41_1_IRQHandler
        ProxyHandler CCU41_2_IRQHandler
        ProxyHandler CCU41_3_IRQHandler
        ProxyHandler CCU42_0_IRQHandler
        ProxyHandler CCU42_1_IRQHandler
        ProxyHandler CCU42_2_IRQHandler
        ProxyHandler CCU42_3_IRQHandler
        ProxyHandler CCU43_0_IRQHandler
        ProxyHandler CCU43_1_IRQHandler
        ProxyHandler CCU43_2_IRQHandler
        ProxyHandler CCU43_3_IRQHandler
        ProxyHandler CCU80_0_IRQHandler
        ProxyHandler CCU80_1_IRQHandler
        ProxyHandler CCU80_2_IRQHandler
        ProxyHandler CCU80_3_IRQHandler
        ProxyHandler CCU81_0_IRQHandler
        ProxyHandler CCU81_1_IRQHandler
        ProxyHandler CCU81_2_IRQHandler
        ProxyHandler CCU81_3_IRQHandler
        ProxyHandler POSIF0_0_IRQHandler
        ProxyHandler POSIF0_1_IRQHandler
        ProxyHandler POSIF1_0_IRQHandler
        ProxyHandler POSIF1_1_IRQHandler
        ProxyHandler CAN0_0_IRQHandler
        ProxyHandler CAN0_1_IRQHandler
        ProxyHandler CAN0_2_IRQHandler
        ProxyHandler CAN0_3_IRQHandler
        ProxyHandler CAN0_4_IRQHandler
        ProxyHandler CAN0_5_IRQHandler
        ProxyHandler CAN0_6_IRQHandler
        ProxyHandler CAN0_7_IRQHandler
        ProxyHandler USIC0_0_IRQHandler
        ProxyHandler USIC0_1_IRQHandler
        ProxyHandler USIC0_2_IRQHandler
        ProxyHandler USIC0_3_IRQHandler
        ProxyHandler USIC0_4_IRQHandler
        ProxyHandler USIC0_5_IRQHandler
        ProxyHandler USIC1_0_IRQHandler
        ProxyHandler USIC1_1_IRQHandler
        ProxyHandler USIC1_2_IRQHandler
        ProxyHandler USIC1_3_IRQHandler
        ProxyHandler USIC1_4_IRQHandler
        ProxyHandler USIC1_5_IRQHandler
        ProxyHandler USIC2_0_IRQHandler
        ProxyHandler USIC2_1_IRQHandler
        ProxyHandler USIC2_2_IRQHandler
        ProxyHandler USIC2_3_IRQHandler
        ProxyHandler USIC2_4_IRQHandler
        ProxyHandler USIC2_5_IRQHandler
        ProxyHandler LEDTS0_0_IRQHandler
        ProxyHandler FCE0_0_IRQHandler
        ProxyHandler GPDMA0_0_IRQHandler
        ProxyHandler SDMMC0_0_IRQHandler
        ProxyHandler USB0_0_IRQHandler
        ProxyHandler ETH0_0_IRQHandler
        ProxyHandler GPDMA1_0_IRQHandler
          
           
; Definition of the default weak SystemInit_DAVE3 function for DAVE3 system init.
        PUBWEAK SystemInit_DAVE3
        SECTION .text:CODE:REORDER:NOROOT(2)
SystemInit_DAVE3
        NOP 
        BX LR
 
; Definition of the default weak DAVE3 function for clock App usage.
; AllowPLLInitByStartup Handler
        PUBWEAK AllowPLLInitByStartup
        SECTION .text:CODE:REORDER:NOROOT(2)
AllowPLLInitByStartup       
        MOV R0,#1
        BX LR                	    

PREF_PCON       EQU 0x58004000
SCU_GCU_PEEN    EQU 0x5000413C
SCU_GCU_PEFLAG  EQU 0x50004150

      
        END
