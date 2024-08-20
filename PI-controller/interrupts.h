//	EmSyPro9000
// 	Embedded system programming 2022

// Interrupt setup header file

int InitGic();			// Initialize Generic Interrupt Controller
int InitGPIO();			// Initialize GPIO, set data directions (Buttons, switches and leds)
int InitTTC1();			// Initialize TTC1, not started yet
int InitInterrupts();	// Setup and enable all interrupts
void ButtonISR();		// Button and switch interrupt service routine
void TimerISR();		// TTC1 interrupt service routine
