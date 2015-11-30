//Dependencies
#include "isr_support.h"

   .global ext2IrqWrapper
   .extern extIntHandler
   .extern xISRStackTop

   .set nomips16
   .set noreorder
   .set noat

   .ent ext2IrqWrapper

//External 2 interrupt
ext2IrqWrapper:
   //Save the current task context
   portSAVE_CONTEXT
   //Call interrupt handler
   jal extIntHandler
   nop
   //Restore the context of the next task to execute
   portRESTORE_CONTEXT

   .end ext2IrqWrapper
