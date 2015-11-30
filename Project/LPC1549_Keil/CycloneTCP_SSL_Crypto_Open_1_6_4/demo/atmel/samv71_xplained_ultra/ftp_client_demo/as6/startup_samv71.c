//Dependencies
#include "samv71.h"

//Linker file constants
extern uint32_t _sfixed;
extern uint32_t _efixed;
extern uint32_t _etext;
extern uint32_t _srelocate;
extern uint32_t _erelocate;
extern uint32_t _szero;
extern uint32_t _ezero;
extern uint32_t _sstack;
extern uint32_t _estack;

//Function declaration
void SystemInit(void);
void __libc_init_array(void);
int main(void);

//Default empty handler
void Default_Handler(void);

//Cortex-M3 core handlers
void Reset_Handler         (void);
void NMI_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler     (void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler     (void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler      (void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler    (void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler      (void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler        (void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler       (void) __attribute__((weak, alias("Default_Handler")));

//Peripheral handlers
void SUPC_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void RSTC_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void RTC_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void RTT_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void WDT_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void PMC_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void EFC_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void UART0_Handler         (void) __attribute__((weak, alias("Default_Handler")));
void UART1_Handler         (void) __attribute__((weak, alias("Default_Handler")));
void PIOA_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void PIOB_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void PIOC_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void USART0_Handler        (void) __attribute__((weak, alias("Default_Handler")));
void USART1_Handler        (void) __attribute__((weak, alias("Default_Handler")));
void USART2_Handler        (void) __attribute__((weak, alias("Default_Handler")));
void PIOD_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void PIOE_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void HSMCI_Handler         (void) __attribute__((weak, alias("Default_Handler")));
void TWIHS0_Handler        (void) __attribute__((weak, alias("Default_Handler")));
void TWIHS1_Handler        (void) __attribute__((weak, alias("Default_Handler")));
void SPI0_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void SSC_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void TC0_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void TC1_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void TC2_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void TC3_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void TC4_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void TC5_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void AFEC0_Handler         (void) __attribute__((weak, alias("Default_Handler")));
void DACC_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void PWM0_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void ICM_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void ACC_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void USBHS_Handler         (void) __attribute__((weak, alias("Default_Handler")));
void MCAN0_Handler         (void) __attribute__((weak, alias("Default_Handler")));
void MCAN0_Line1_Handler   (void) __attribute__((weak, alias("Default_Handler")));
void MCAN1_Handler         (void) __attribute__((weak, alias("Default_Handler")));
void MCAN1_Line1_Handler   (void) __attribute__((weak, alias("Default_Handler")));
void GMAC_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void AFEC1_Handler         (void) __attribute__((weak, alias("Default_Handler")));
void TWIHS2_Handler        (void) __attribute__((weak, alias("Default_Handler")));
void SPI1_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void QSPI_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void UART2_Handler         (void) __attribute__((weak, alias("Default_Handler")));
void UART3_Handler         (void) __attribute__((weak, alias("Default_Handler")));
void UART4_Handler         (void) __attribute__((weak, alias("Default_Handler")));
void TC6_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void TC7_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void TC8_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void TC9_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void TC10_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void TC11_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void MLB_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void AES_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void TRNG_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void XDMAC_Handler         (void) __attribute__((weak, alias("Default_Handler")));
void ISI_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void PWM1_Handler          (void) __attribute__((weak, alias("Default_Handler")));
void SDRAMC_Handler        (void) __attribute__((weak, alias("Default_Handler")));
void RSWDT_Handler         (void) __attribute__((weak, alias("Default_Handler")));

//Vector table
__attribute__((section(".vectors")))
const uint32_t vectorTable[80] =
{
	//Initial stack pointer
   (uint32_t) (&_estack),

   //Core handlers
   (uint32_t) Reset_Handler,
   (uint32_t) NMI_Handler,
   (uint32_t) HardFault_Handler,
   (uint32_t) MemManage_Handler,
   (uint32_t) BusFault_Handler,
   (uint32_t) UsageFault_Handler,
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) SVC_Handler,
   (uint32_t) DebugMon_Handler,
   (uint32_t) 0,
   (uint32_t) PendSV_Handler,
   (uint32_t) SysTick_Handler,

   //Peripheral handlers
   (uint32_t) SUPC_Handler,         //0  Supply Controller
   (uint32_t) RSTC_Handler,         //1  Reset Controller
   (uint32_t) RTC_Handler,          //2  Real Time Clock
   (uint32_t) RTT_Handler,          //3  Real Time Timer
   (uint32_t) WDT_Handler,          //4  Watchdog Timer 0
   (uint32_t) PMC_Handler,          //5  Power Management Controller
   (uint32_t) EFC_Handler,          //6  Enhanced Embedded Flash Controller
   (uint32_t) UART0_Handler,        //7  UART 0
   (uint32_t) UART1_Handler,        //8  UART 1
   (uint32_t) 0,                    //9  Reserved
   (uint32_t) PIOA_Handler,         //10 Parallel I/O Controller A
   (uint32_t) PIOB_Handler,         //11 Parallel I/O Controller B
#ifdef _SAMV71_PIOC_INSTANCE_
   (uint32_t) PIOC_Handler,         //12 Parallel I/O Controller C
#else
   (uint32_t) 0,                    //12 Reserved
#endif
   (uint32_t) USART0_Handler,       //13 USART 0
   (uint32_t) USART1_Handler,       //14 USART 1
   (uint32_t) USART2_Handler,       //15 USART 2
   (uint32_t) PIOD_Handler,         //16 Parallel I/O Controller D
#ifdef _SAMV71_PIOE_INSTANCE_
   (uint32_t) PIOE_Handler,         //17 Parallel I/O Controller E
#else
   (uint32_t) 0,                    //17 Reserved
#endif
#ifdef _SAMV71_HSMCI_INSTANCE_
   (uint32_t) HSMCI_Handler,        //18 Multimedia Card Interface
#else
   (uint32_t) 0,                    //18 Reserved
#endif
   (uint32_t) TWIHS0_Handler,       //19 Two Wire Interface 0 HS
   (uint32_t) TWIHS1_Handler,       //20 Two Wire Interface 1 HS
   (uint32_t) SPI0_Handler,         //21 Serial Peripheral Interface 0
   (uint32_t) SSC_Handler,          //22 Synchronous Serial Controller
   (uint32_t) TC0_Handler,          //23 Timer/Counter 0
   (uint32_t) TC1_Handler,          //24 Timer/Counter 1
   (uint32_t) TC2_Handler,          //25 Timer/Counter 2
#ifdef _SAMV71_TC1_INSTANCE_
   (uint32_t) TC3_Handler,          //26 Timer/Counter 3
#else
   (uint32_t) 0,                    //26 Reserved
#endif
#ifdef _SAMV71_TC1_INSTANCE_
   (uint32_t) TC4_Handler,          //27 Timer/Counter 4
#else
   (uint32_t) 0,                    //27 Reserved
#endif
#ifdef _SAMV71_TC1_INSTANCE_
   (uint32_t) TC5_Handler,          //28 Timer/Counter 5
#else
   (uint32_t) 0,                    //28 Reserved
#endif
   (uint32_t) AFEC0_Handler,        //29 Analog Front End 0
#ifdef _SAMV71_DACC_INSTANCE_
   (uint32_t) DACC_Handler,         //30 Digital To Analog Converter
#else
   (uint32_t) 0,                    //30 Reserved
#endif
   (uint32_t) PWM0_Handler,         //31 Pulse Width Modulation 0
   (uint32_t) ICM_Handler,          //32 Integrity Check Monitor
   (uint32_t) ACC_Handler,          //33 Analog Comparator
   (uint32_t) USBHS_Handler,        //34 USB Host / Device Controller
   (uint32_t) MCAN0_Handler,        //35 CAN Controller 0
   (uint32_t) 0,                    //36 CAN Controller 0 - Line 1 (MCAN0_Line1_Handler)
   (uint32_t) MCAN1_Handler,        //37 CAN Controller 1
   (uint32_t) 0,                    //38 CAN Controller 1 - Line 1 (MCAN1_Line1_Handler)
   (uint32_t) GMAC_Handler,         //39 Ethernet MAC
   (uint32_t) AFEC1_Handler,        //40 Analog Front End 1
#ifdef _SAMV71_TWIHS2_INSTANCE_
   (uint32_t) TWIHS2_Handler,       //41 Two Wire Interface 2 HS
#else
   (uint32_t) 0,                    //41 Reserved
#endif
   (uint32_t) SPI1_Handler,         //42 Serial Peripheral Interface 1
   (uint32_t) QSPI_Handler,         //43 Quad I/O Serial Peripheral Interface
   (uint32_t) UART2_Handler,        //44 UART 2
   (uint32_t) UART3_Handler,        //45 UART 3
   (uint32_t) UART4_Handler,        //46 UART 4
#ifdef _SAMV71_TC2_INSTANCE_
   (uint32_t) TC6_Handler,          //47 Timer/Counter 6
#else
   (uint32_t) 0,                    //47 Reserved
#endif
#ifdef _SAMV71_TC2_INSTANCE_
   (uint32_t) TC7_Handler,          //48 Timer/Counter 7
#else
   (uint32_t) 0,                    //48 Reserved
#endif
#ifdef _SAMV71_TC2_INSTANCE_
   (uint32_t) TC8_Handler,          //49 Timer/Counter 8
#else
   (uint32_t) 0,                    //49 Reserved
#endif
   (uint32_t) TC9_Handler,          //50 Timer/Counter 9
   (uint32_t) TC10_Handler,         //51 Timer/Counter 10
   (uint32_t) TC11_Handler,         //52 Timer/Counter 11
   (uint32_t) MLB_Handler,          //53 MediaLB
   (uint32_t) 0,                    //54 Reserved
   (uint32_t) 0,                    //55 Reserved
   (uint32_t) AES_Handler,          //56 AES
   (uint32_t) TRNG_Handler,         //57 True Random Generator
   (uint32_t) XDMAC_Handler,        //58 DMA
   (uint32_t) ISI_Handler,          //59 Camera Interface
   (uint32_t) PWM1_Handler,         //60 Pulse Width Modulation 1
   (uint32_t) 0,                    //61 Reserved
#ifdef _SAMV71_SDRAMC_INSTANCE_
   (uint32_t) SDRAMC_Handler,       //62 SDRAM Controller
#else
   (uint32_t) 0,                    //62 Reserved
#endif
   (uint32_t) RSWDT_Handler         //63 Watchdog Timer 1
};


/**
 * @brief Enable TCM memory
 **/

__STATIC_INLINE void TCM_Enable(void) 
{
   __DSB();
   __ISB();
   SCB->ITCMCR = (SCB_ITCMCR_EN_Msk | SCB_ITCMCR_RMW_Msk | SCB_ITCMCR_RETEN_Msk);
   SCB->DTCMCR = (SCB_DTCMCR_EN_Msk | SCB_DTCMCR_RMW_Msk | SCB_DTCMCR_RETEN_Msk);
   __DSB();
   __ISB();
}


/**
 * @brief Disable TCM memory
 **/

__STATIC_INLINE void TCM_Disable(void) 
{
   __DSB();
   __ISB();
   SCB->ITCMCR &= ~((uint32_t) SCB_ITCMCR_EN_Msk);
   SCB->DTCMCR &= ~((uint32_t) SCB_ITCMCR_EN_Msk);
   __DSB();
   __ISB();
}


/**
 * @brief Reset handler
 **/

void Reset_Handler(void)
{
   uint32_t *src;
   uint32_t *dest;

   //System initialization
   SystemInit();

   //Initialize the relocate segment
   src = &_etext;
   dest = &_srelocate;

   if(src != dest)
   {
      while(dest < &_erelocate)
      {
         *dest++ = *src++;
      }
   }

   //Clear the zero segment
   for(dest = &_szero; dest < &_ezero;)
   {
      *dest++ = 0;
   }

   //Set the vector table base address
   src = (uint32_t *) & _sfixed;
   SCB->VTOR = ((uint32_t) src & SCB_VTOR_TBLOFF_Msk);

#ifdef ENABLE_TCM 
   //32KB
   EFC->EEFC_FCR = (EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FCMD_CGPB | EEFC_FCR_FARG(8));
   EFC->EEFC_FCR = (EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FCMD_SGPB | EEFC_FCR_FARG(7));

   TCM_Enable();
#else
   EFC->EEFC_FCR = (EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FCMD_CGPB | EEFC_FCR_FARG(8));
   EFC->EEFC_FCR = (EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FCMD_CGPB | EEFC_FCR_FARG(7));

   TCM_Disable();
#endif

   //C library initialization
   __libc_init_array();

   //Branch to main function
   main();

   //Endless loop
   while(1);
}


/**
 * @brief Default interrupt handler
 **/

void Default_Handler(void)
{
   while(1)
   {
   }
}
