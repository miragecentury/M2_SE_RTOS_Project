   ;Dependencies
   include portmacro.inc

   export emacIrqWrapper
   import sam9263EthIrqHandler

   arm
   area  |.text|, code, readonly

;EMAC interrupt
emacIrqWrapper
   preserve8
   ;Save the current task context
   portSAVE_CONTEXT
   ;Call interrupt handler
   ldr   r0, =sam9263EthIrqHandler
   mov   lr, pc
   bx    r0
   ;Restore the context of the next task to execute
   portRESTORE_CONTEXT

   end
