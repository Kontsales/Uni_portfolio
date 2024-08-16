//	EmSyPro9000
// 	Embedded system programming 2022

#include <xscugic.h>
#include <xgpio.h>
#include <xttcps.h>
#include <zynq_registers.h>
#include "Interrupts.h"

//*********************** USEFUL STUFF *************************//
// XTtcPs_ commands to control TTC1, for example:
// XTtcPs_Start(pTTC1); Starts timer
// XTtcPs_Stop(pTTC1); Stop timer
// XTtcPs_ResetCounterValue(pTTC1); reset counter value
// More in "xttcps.h"

// XGpio_ commands to control buttons, switches and leds, for example:
// XGpio_DiscreteWrite(pLEDS, LEDS_channel, 0xF); Write all LEDS High
// XGpio_DiscreteRead(pBTNS_SWTS, BUTTONS_channel); Read status of buttons
// More in "xgpio.h"

//////// Following commands control interrupts:

// To disable button interrupts:
//		XGpio_InterruptDisable(pBTNS_SWTS, 0x3);
//		XGpio_InterruptGlobalDisable(pBTNS_SWTS);

// To enable button interrupts:
//		XGpio_InterruptEnable(pBTNS_SWTS, 0x3);
//		XGpio_InterruptGlobalEnable(pBTNS_SWTS);

// To Disable TTC1 interrupts:
//		TTC1_IER &= ~XTTCPS_IXR_INTERVAL_MASK;
//		XTtcPs_ResetCounterValue(pTTC1);

// To Enable TTC1 interrupts:
//		TTC1_IER |= XTTCPS_IXR_INTERVAL_MASK;
//

// CHANGE THIS TO SET FREQUENCY FOR TTC1 Interrupts
#define OUTPUTHZ 1			// must be integer value [Hz]

// Interrupt controller device ID
#define INTC_DEVICE_ID			XPAR_PS7_SCUGIC_0_DEVICE_ID
// Interrupt ID for TTC1_timer0 interrupts
#define INT_TTC1_TIMER0_ID		69
// Define device ID:s for buttons, switches and leds from "xparameters.h"
// Used with XGPIO (Device driver for Xilinx GPIO)
#define BUTTONS_channel 	2
#define BUTTONS_AXI_ID 		XPAR_AXI_GPIO_SW_BTN_DEVICE_ID
#define SWITCHES_channel	1
#define SWITCHES_AXI_ID		XPAR_AXI_GPIO_SW_BTN_DEVICE_ID
#define LEDS_channel		1
#define LEDS_AXI_ID			XPAR_AXI_GPIO_LED_DEVICE_ID

// for holding timer settings
typedef struct {
	u32 OutputHz;	// Output frequency
	XInterval Interval;	// Interval value
	u8 Prescaler;	// Prescaler value
	u16 Options;	// Option settings
} TimerSetup;

// Initialize Generic Interrupt Controller
int InitGic(XScuGic *pGIC) {
	int Status;

	XScuGic_Config *pGICcnf;

	// Get configuration of Interrupt Controller based on device ID
	pGICcnf = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == pGICcnf) return XST_FAILURE;

	// Initialize the Interrupt Controller instance based on configuration
	Status = XScuGic_CfgInitialize(pGIC, pGICcnf, pGICcnf->CpuBaseAddress);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	// Connect FIQ interrupts on processor side to ButtonISR() interrupt service routine
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FIQ_INT,
								(Xil_ExceptionHandler) ButtonISR,
								(void*) 0);

	// Connect IRQ interrupts on processor side to TimerISR() interrupt service routine
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
								(Xil_ExceptionHandler) TimerISR,
								(void*) 0);

	return XST_SUCCESS;
}
// Initialize GPIO, set data directions (Buttons, switches and leds)
int InitGPIO(XGpio *pBTNS_SWTS, XGpio *pLEDS) {
	int Status;

	// Initialize buttons and switches
	Status = XGpio_Initialize(pBTNS_SWTS, BUTTONS_AXI_ID);	// (Pointer to instance, On-board device ID)
	if (Status != XST_SUCCESS){
		xil_printf("Buttons and switches error\n");
		return XST_FAILURE;
	}

	// Initialize LEDs
	Status = XGpio_Initialize(pLEDS, LEDS_AXI_ID);	// (Pointer to instance, On-board device ID)
	if (Status != XST_SUCCESS){
		xil_printf("LEDs error\n");
		return XST_FAILURE;
	}

	// Set data directions (buttons and switches as inputs, leds as outputs)
	XGpio_SetDataDirection(pBTNS_SWTS,2,0xF);
	XGpio_SetDataDirection(pBTNS_SWTS,1,0xF);
	XGpio_SetDataDirection(pLEDS, 1, 0x0);

	return XST_SUCCESS;
}
// Initialize TTC1, not started yet
int InitTTC1(XTtcPs *pTTC1) {

	int Status;
	static TimerSetup *TTC1Setup;

	XTtcPs_Config *pTTC1cnf;
	// Lookup timer configuration based on device ID
	pTTC1cnf = XTtcPs_LookupConfig(XPAR_PS7_TTC_3_DEVICE_ID);
	if (NULL == pTTC1cnf) return XST_FAILURE;

	// Force timer to stop (caused some problems when not here XTtcPs_Stop() didnt work)
	Xil_Out32(pTTC1cnf->BaseAddress + (u32)(XTTCPS_CNT_CNTRL_OFFSET), (u32)(XTTCPS_CNT_CNTRL_DIS_MASK|XTTCPS_CNT_CNTRL_RST_MASK));

	// Initialize a timer instance based on config lookupped above
	Status = XTtcPs_CfgInitialize(pTTC1, pTTC1cnf, pTTC1cnf->BaseAddress);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	// Reset counter value
	XTtcPs_ResetCounterValue(pTTC1);

	// Set interval mode and disable PWM output
	TTC1Setup->Options = (XTTCPS_OPTION_INTERVAL_MODE |XTTCPS_OPTION_WAVE_DISABLE);

	// Set the options
	XTtcPs_SetOptions(pTTC1,TTC1Setup->Options);

	// Calculate and set Interval and prescaler based on output frequency
	TTC1Setup->OutputHz = OUTPUTHZ;
	XTtcPs_CalcIntervalFromFreq(pTTC1, TTC1Setup->OutputHz, &(TTC1Setup->Interval), &(TTC1Setup->Prescaler));
	XTtcPs_SetInterval(pTTC1, TTC1Setup->Interval);
	XTtcPs_SetPrescaler(pTTC1, TTC1Setup->Prescaler);

	// Timer not yet started
	return XST_SUCCESS;
}


// Setup and enable all interrupts
int InitInterrupts(XScuGic *pGIC, XGpio *pBTNS_SWTS) {
	// enable button interrupts on GPIO side
	XGpio_InterruptEnable(pBTNS_SWTS, 0x3);
	XGpio_InterruptGlobalEnable(pBTNS_SWTS);
	// enable FIQ interrupts on processor side
	Xil_ExceptionEnableMask(XIL_EXCEPTION_FIQ);
	// enable IRQ interrupts on processor side
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	// enable TTC interrupts in Interrupt Controller instance
	XScuGic_Enable(pGIC, INT_TTC1_TIMER0_ID);
	// enable timer interrupts on interval
	TTC1_IER |= XTTCPS_IXR_INTERVAL_MASK;
	return XST_SUCCESS;
}



