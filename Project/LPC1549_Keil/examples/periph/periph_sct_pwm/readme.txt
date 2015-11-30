State Configurable Timer (SCT) PWM example

Example description
This example, uses the SCT PWM driver to create 2 independent PWMs (running at
the same frequency). The PWM1 is used to output a square wave to an Output pin
and PWM2 is used to control the brightness of the LED, the brightness goes
from 0% to 100% in steps of 0.5% every 20 Milliseconds and goes down to 0% in
the same step.

Special connection requirements
LPC1549 NXP LPCXpresso Board
Connect Oscilloscope to J2 [Pin 3], the waveform on scope will have its duty
cycle increasing from 0% to 100% in steps of 10% every second.

Build procedures:
Visit the LPCOpen Quick start guide at
[http://www.lpcware.com/content/project/lpcopen-platform-nxp-lpc-microcontrollers/lpcopen-v200-quickstart-guides]
to get started building LPCOpen projects.
