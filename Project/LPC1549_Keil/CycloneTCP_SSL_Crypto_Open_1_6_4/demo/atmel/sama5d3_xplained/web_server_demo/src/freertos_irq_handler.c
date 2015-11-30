#include "freertos.h"

/* The function called by the RTOS port layer after it has managed interrupt
entry. */
void vApplicationIRQHandler(void)
{
   typedef void (*ISRFunction_t)(void);
   ISRFunction_t pxISRFunction;
   volatile uint32_t * pulAIC_IVR = (uint32_t *) configINTERRUPT_VECTOR_ADDRESS;

   /* Obtain the address of the interrupt handler from the AIR. */
   pxISRFunction = (ISRFunction_t) *pulAIC_IVR;

   /* Write back to the SAMA5's interrupt controller's IVR register in case the
   CPU is in protect mode.  If the interrupt controller is not in protect mode
   then this write is not necessary. */
   *pulAIC_IVR = (uint32_t) pxISRFunction;

   /* Ensure the write takes before re-enabling interrupts. */
   __DSB();
   __ISB();
   __enable_irq();

   /* Call the installed ISR. */
   pxISRFunction();
}
