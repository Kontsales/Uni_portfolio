#include <xscugic.h>
#include <xttcps.h>
#include <xgpio.h>
#include <zynq_registers.h>
#include "uart_communication.h"
#include "interrupts.h"

#define BUTTONS_channel 	2
#define BUTTONS_AXI_ID 		XPAR_AXI_GPIO_SW_BTN_DEVICE_ID
#define SWITCHES_channel	1
#define SWITCHES_AXI_ID		XPAR_AXI_GPIO_SW_BTN_DEVICE_ID
#define LEDS_channel		1
#define LEDS_AXI_ID			XPAR_AXI_GPIO_LED_DEVICE_ID

#define LD0					0x1
#define LD1					0x2
#define LD2					0x4
#define LD3					0x8

XScuGic GIC; 					// Interrupt controller instance for Interrupt controller driver
XScuGic *pGIC = &GIC; 			// Ptr for above
XTtcPs TTC1;					// Timer instance
XTtcPs *pTTC1 = &TTC1;			// Timer instance pointer
XGpio BTNS_SWTS;				// Buttons and switches instance
XGpio *pBTNS_SWTS = &BTNS_SWTS;	// Buttons and switches instance pointer
XGpio LEDS; 					// LED instance
XGpio *pLEDS = &LEDS;			// LED instance pointer
u8 buttons = 0;					// Variable to save button state when using discreteread in XGpio driver

// variables for state machine
uint8_t state = 0;
uint8_t* pstate = &state;
const char statedesc [3][20] = {"Configuration mode", "Idling",  "Modulating"};

// Kp, Ki flags and pointers
uint8_t Kp_flag = TRUE;
uint8_t Ki_flag = FALSE;
uint8_t* pKp_flag = &Kp_flag;
uint8_t* pKi_flag = &Ki_flag;

#define BUFFER_SIZE 20

int main(void) {

	int Status;
	int Ki = 0;
	int Kp = 0;
	int Ref = 0;

	// Initialize GPIO, GIC and TTC1
	Status = InitGPIO(pBTNS_SWTS, pLEDS);
	if (Status != XST_SUCCESS) xil_printf("GPIO setup error\n");
	Status = InitGic(pGIC);
	if (Status == XST_FAILURE) xil_printf("Interrupt Controller Initialize error\n");
	Status = InitTTC1(pTTC1);
	if (Status != XST_SUCCESS) xil_printf("Timer setup error\n");

	// Setup and Enable all interrupts
	InitInterrupts(pGIC, pBTNS_SWTS);
	// Start TTC1
	XTtcPs_Start(pTTC1);

	initialize_uart();

	uart_send_string("\nEmSyPro9000 Started!\n");
	uart_send_string(statedesc[state]);
	uart_send_config(Ref, Ki, Kp);

	char input;
	char serial_buffer[16];
	int serial_buffer_index = 0;



	while(1){
		switch (state){
			case 0:		// Configuration mode
				AXI_LED_DATA = 0x1;
				break;

			case 1:		// Idling ...
				AXI_LED_DATA = 0x2;
				break;

			case 2:		// Modulating
				AXI_LED_DATA = 0x4;
				break;

			default:
				break;

		}

		input = uart_receive(); // polling UART receive buffer

		if (input != 0){
			// Depending on the serial terminal used, UART messages can be terminated
			// by either carriage return '\r' or line feed '\n'.
			if (input == '\r' || input == '\n'){
				serial_buffer[serial_buffer_index] = '\0';
				uart_send_string(serial_buffer);

				char buffer [8];

				// Mode change requested?
				if (strcmp(serial_buffer, "setmode 0") == 0){
					state = 0;
					uart_send_string(statedesc[state]);
					uart_send_config(Ref, Ki, Kp);
				}
				else if  (strcmp(serial_buffer, "setmode 1") == 0){
					state = 1;
					uart_send_string(statedesc[state]);
				}
				else if  (strcmp(serial_buffer, "setmode 2") == 0){
					state = 2;
					uart_send_string(statedesc[state]);
				}
				else{
					// Parameter/reference change requested?
					switch (state){
						case 0:		// Configuration mode
							// Check for Kp
							memcpy(buffer, &serial_buffer, 2);
							buffer[2] = '\0';
							if (strcmp(buffer, "kp") == 0){
								memcpy(buffer, &serial_buffer[2], 5);
								int newKp = atoi(buffer);
								if (newKp < 1000 && newKp >= 0){
									Kp = newKp;
									uart_send_string("Changes applied!\n");
								}
								else{
									uart_send_string("Invalid value for Kp!\n");
								}
								uart_send_config(Ref, Ki, Kp);
								//return
							}
							// Check for Ki
							else if (strcmp(buffer, "ki") == 0){
								memcpy(buffer, &serial_buffer[2], 5);
								int newKi = atoi(buffer);
								if (newKi < 1000 && newKi >= 0){
									Ki = newKi;
									uart_send_string("Changes applied!\n");
								}
								else{
									uart_send_string("Invalid value for Ki!\n");
								}
								uart_send_config(Ref, Ki, Kp);
							}
							break;

						case 1:		// Idling ...
							// No parameter changes when idling
							break;

						case 2:		// Modulating
							// Check for reference change
							memcpy(buffer, &serial_buffer, 3);
							buffer[3] = '\0';
							if (strcmp(buffer, "ref") == 0){
								memcpy(buffer, &serial_buffer[3], 5);
								int newRef = atoi(buffer);
								if (newRef < 1000 && newRef >= 0){
									Ref = newRef;
									uart_send_string("Changes applied!\n");
								}
								else{
									uart_send_string("Invalid value for Ref!\n");
								}
								uart_send_config(Ref, Ki, Kp);
							}
							break;

						default:
							break;

					}
				}
				serial_buffer_index = 0;
			}
			else {
				serial_buffer[serial_buffer_index] = input;
				serial_buffer_index++;
			}


		}
	}

	return 0;
}

void ButtonISR(void *data){
	XGpio_InterruptClear(&BTNS_SWTS,0xF);
	buttons = XGpio_DiscreteRead(&BTNS_SWTS, BUTTONS_channel);
	switch(buttons){
		// on btn0 press
		case LD0:
			// Cycle through 3 different modes on btn0 press
			switch(state){
				case 0:
					*pstate = 1;
					uart_send_string(statedesc[state]);
					break;
				case 1:
					*pstate = 2;
					uart_send_string(statedesc[state]);
					break;
				case 2:
					*pstate = 0;
					uart_send_string(statedesc[state]);
					break;
				default:
					break;
			}
			break;
		// On btn1 press
		case LD1:
			XGpio_DiscreteWrite(&LEDS, LEDS_channel, LD1);	// LD1.

			// if in config mode, change between Kp and Ki
			if (state == 0) {
				if (Kp_flag == TRUE) {
					*pKp_flag = FALSE;
					*pKi_flag = TRUE;
					xil_printf("\nModifying Ki value\n");
				} else if (Ki_flag == TRUE) {
					*pKi_flag = FALSE;
					*pKp_flag = TRUE;
					xil_printf("\nModifying Kp value\n");
				}
			}
			break;
		case LD2:
			XGpio_DiscreteWrite(&LEDS, LEDS_channel, LD2);	// LD2.
			break;
		case LD3:
			XGpio_DiscreteWrite(&LEDS, LEDS_channel, LD3);	// LD3.
			break;
		default:
			break;
	}
}

// This function is run on timer interrupts
void TimerISR(){
	// Read interrupt status to clear the interrupt
	XTtcPs_ClearInterruptStatus(pTTC1, XTTCPS_IXR_INTERVAL_MASK);

	// Do stuff
	xil_printf("\nTimerISR");

}
