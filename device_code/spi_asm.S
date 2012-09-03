#include <avr/io.h>

; If we are running at 16MHz and transmitting at 8Mbps, we have 16 cycles for
; each byte. But there's a little bit of flexibility. The UART's 2-byte buffer
; allows us to get a little bit ahead during the packet so we have time left at
; the end.

; It turns out USB bulk transfers aren't real-time-y enough. I need a packet
; every 64us consistently, and the host can't seem to manage that while
; simultaneously handling other interrupts.

; I could do isochronous, 256 byte packets. Then I can only do one packet per ms.
; So then I'm restricted to outputting at 2Mbps rather than 8Mbps.

; The other alternative, if I could find the cycles, is to have a buffer. I
; could fit a buffer of about 1ms in RAM. This would still require the host to
; be quite real-time-y.

.globl main_loop
main_loop:
	rjmp .start
.packetloop:

	.rept 64

	; Wait for port to be ready
	; (4 cycles if ready. 9 or 14 cycles otherwise.)
1:
	lds		r24, UCSR1A
	sbrs	r24, UDRE1
	rjmp	1b

	; Copy byte (4 cycles)
	lds		r24, UEDATX
	sts		UDR1, r24

	.endr

	; ClearOUT (5 cycles)
	lds		r24, UEINTX
	andi	r24, ~((1<<FIFOCON)+(1<<RXOUTI))
	sts		UEINTX, r24

.start:
	; If RWAL, loop (5 cycles)
	lds		r24, UEINTX
	sbrc	r24, RWAL
	rjmp	.packetloop

	ret
