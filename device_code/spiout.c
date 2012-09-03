#include <LUFA/Drivers/USB/USB.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay_basic.h>
#include "spiout.h"

int main(void) {
	// Select 16MHz clock
	clock_prescale_set(clock_div_1);

	// Init USART for SPI mode
	UCSR1C = 0xc6; // Set SPI mode, leave other settings at default
	UCSR1B = 0x08; // Set TXEN
	// UBRR1 is already at 0 (=2Mbps)
	UBRR1L = 1; // 500000 bytes per second
	DDRD = 0x20; // Clock pin must be output for USART to operate in master mode

	sei();
	USB_Init();
	while (1) {
		// Handle control requests (Like USBTask)
		Endpoint_SelectEndpoint(ENDPOINT_CONTROLEP);
		if (Endpoint_IsSETUPReceived()) {
		    USB_Device_ProcessControlRequest();
		}
		// Handle data input
		Endpoint_SelectEndpoint(DATA_EPNUM);
		while (Endpoint_IsReadWriteAllowed()) {
			do {
				while (!(UCSR1A&0x20)) {
				}
				UDR1 = Endpoint_Read_8();
			} while (Endpoint_IsReadWriteAllowed());
			Endpoint_ClearOUT();
		}
	}
}

void EVENT_USB_Device_ConfigurationChanged() {
	Endpoint_ConfigureEndpoint(DATA_EPNUM, EP_TYPE_BULK, ENDPOINT_DIR_OUT, DATA_EPSIZE, ENDPOINT_BANK_DOUBLE);
}
