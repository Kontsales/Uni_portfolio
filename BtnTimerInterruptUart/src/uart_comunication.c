//	EmSyPro9000
// 	Embedded system programming 2022

// UART communication library

#include <zynq_registers.h>
#include <xparameters.h>
#include <xuartps_hw.h>
#include "uart_communication.h"

#define BUFFER_SIZE 32

// Initialize UART (RX and TX, baudrate 115200)
void initialize_uart(void){
	uint32_t r = 0; // Temporary value variable


	//CR = control register, page 1756 of the Zynq-TRM
	// Saves the current state of UART_CTRL in the temporary value variable.
	r = UART_CTRL;
	// TX = transmit, RX = receive
	// The enable bits are cleared, and disable bits are set.
	r &= ~(XUARTPS_CR_TX_EN | XUARTPS_CR_RX_EN); // Clear Tx & Rx Enable
	r |= XUARTPS_CR_RX_DIS | XUARTPS_CR_TX_DIS; // Tx & Rx Disable
	// Writes the changes made to r, into the UART control register.
	UART_CTRL = r;

	// MR = mode register, page 1757 of the Zynq-TRM
	UART_MODE = 0;
	UART_MODE &= ~XUARTPS_MR_CLKSEL; // Clear "Input clock selection" - 0: clock source is uart_ref_clk
	UART_MODE |= XUARTPS_MR_CHARLEN_8_BIT; 	// Set "8 bits data"
	UART_MODE |= XUARTPS_MR_PARITY_NONE; 	// Set "No parity mode"
	UART_MODE |= XUARTPS_MR_STOPMODE_1_BIT; // Set "1 stop bit"
	UART_MODE |= XUARTPS_MR_CHMODE_NORM; 	// Set "Normal mode"

	// baud_rate = sel_clk / (CD * (BDIV + 1) (ref: UG585 - TRM - Ch. 19 UART)
	UART_BAUD_DIV = 6; // ("BDIV")
	UART_BAUD_GEN = 124; // ("CD")
	// Baud Rate = 100Mhz / (124 * (6 + 1)) = 115200 bps

	UART_CTRL |= (XUARTPS_CR_TXRST | XUARTPS_CR_RXRST); // TX & RX logic reset

	r = UART_CTRL;
	r |= XUARTPS_CR_RX_EN | XUARTPS_CR_TX_EN; // Set TX & RX enabled
	r &= ~(XUARTPS_CR_RX_DIS | XUARTPS_CR_TX_DIS); // Clear TX & RX disabled
	UART_CTRL = r;
}

// Send one character through UART interface
void uart_send(char c) {
	while (UART_STATUS & XUARTPS_SR_TNFUL);
	UART_FIFO = c;
	while (UART_STATUS & XUARTPS_SR_TACTIVE);
}

// Send string (character array) through UART interface
void uart_send_string(char str[BUFFER_SIZE]) {
	char *ptr = str;
	// While the string still continues.
	while (*ptr != '\0') {
		uart_send(*ptr);
		ptr++;
	}
	uart_send('\n');
}

// Send string with no newline
void uart_send_str(char str[BUFFER_SIZE]) {
	char *ptr = str;
	// While the string still continues.
	while (*ptr != '\0') {
		uart_send(*ptr);
		ptr++;
	}
}

// Check if UART receive FIFO (first in, first out) is not empty and return the new data
char uart_receive() {
	if ((UART_STATUS & XUARTPS_SR_RXEMPTY) == XUARTPS_SR_RXEMPTY) return 0;
	return UART_FIFO;
}


void uart_send_config(int pRef, int pKi, int pKp) {
	uart_send_str("Current config: Ref=");
	char buf [4];
	itoa(pRef, buf, 10);
	uart_send_str(buf);
	uart_send_str(" Ki=");
	itoa(pKi, buf, 10);
	uart_send_str(buf);
	uart_send_str(" Kp=");
	itoa(pKp, buf, 10);
	uart_send_str(buf);
	uart_send_str('\n');
}
