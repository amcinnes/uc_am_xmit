#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>

// Total buffer: NUMTRANSFERS*PACKETS*PACKETSIZE*8
// = 200000 bits = 100ms at 2Mbps
#define NUMTRANSFERS 2
#define PACKETS 50
#define PACKETSIZE 250

int sample = 0;
double sinbuffer[2000];
FILE *file;
double data;

double t;
int total;
double last_quant_error;

double dtime() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec + 1e-6 * tv.tv_usec;
}

#define TAPS 10

double fir_coeff[TAPS] = {
	//1.0000,
   -0.1576,
   -0.6547,
    0.2788,
    0.2174,
   -0.1423,
   -0.0201,
   -0.0049,
    0.0081,
    0.0596,
   -0.0143,
};

double olderrors[TAPS];

char bit(int sample) {
	// time is sample count, at 2Mbps
	// we want a 567khz signal
	double filter_output = 0.0;
	int i;
	for (i=0;i<TAPS;i++) {
		filter_output += fir_coeff[i] * olderrors[(sample+TAPS-i-1)%TAPS];
	}
	double s = (sinbuffer[sample%2000] * (data+0.5)) - filter_output;
	// signal between 0 and 1
	// rectangular dither, width 2
	// such that if signal is -1, the probability of result being positive is 0
	// and if signal is +1, the probability of result being negative is 0
	double dithered = s + drand48()*2-1;
	if (dithered>0) {
		olderrors[sample%TAPS] = 1-s;
	} else {
		olderrors[sample%TAPS] = -1-s;
	}
	return dithered>0;
}

void transfer(struct libusb_transfer *x) {
	int i,j;
	for (i=0;i<x->length;i++) {
		x->buffer[i] = 0;
		// LSB comes first
		for (j=0;j<8;j++) {
			if (sample%100==0) {
				int16_t data16;
				fread(&data16,2,1,file);
				data = ((double)data16)/37268+0.5;
			}
			x->buffer[i] |= bit(sample)<<j;
			sample++;
		}

	}
	//memset(x->buffer,0xaa,x->length);

	libusb_submit_transfer(x);
}

void callback(struct libusb_transfer *xfr) {
	if (xfr->status!=LIBUSB_TRANSFER_COMPLETED) {
		fprintf(stderr,"Transfer failed: %d\n",xfr->status);
		exit(1);
	}
	transfer(xfr);

	total += xfr->length;
	double rate = total/(dtime()-t);
	fprintf(stderr,"%10d %lf\n",total,rate);
}

int main() {
	int i;
	for (i=0;i<2000;i++) {
		sinbuffer[i] = sin(i*567*2*M_PI/2000.);
	}

	file = fopen("20k.bin","r");
	assert(file);

	struct libusb_transfer *xfrs[NUMTRANSFERS];

	libusb_init(NULL);
	libusb_device_handle *dev = libusb_open_device_with_vid_pid(NULL,0x03eb,0x204f);
	assert(dev);

	for (i=0;i<NUMTRANSFERS;i++) {
		xfrs[i] = libusb_alloc_transfer(50);
		char *buf = malloc(PACKETS*PACKETSIZE);
		libusb_fill_iso_transfer(xfrs[i],dev,0x01,buf,PACKETS*PACKETSIZE,PACKETS,callback,NULL,0);
		libusb_set_iso_packet_lengths(xfrs[i],PACKETSIZE);
		transfer(xfrs[i]);
	}
	t = dtime();
	while (1) libusb_handle_events(NULL);
}
