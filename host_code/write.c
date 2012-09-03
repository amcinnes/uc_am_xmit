#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>

// Total buffer: NUMTRANSFERS*PACKETS*PACKETSIZE*8
// = 200000 bits = 100ms at 2Mbps
#define NUMTRANSFERS 2
#define PACKETS 50
#define PACKETSIZE 250

double dtime() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec + 1e-6 * tv.tv_usec;
}

void transfer(struct libusb_transfer *x) {
	/*
	int r = fread(x->buffer,1,BUFSIZE,stdin);
	if (r==0) {
		if (feof(stdin)) {
			exit(0);
		} else {
			fprintf(stderr,"Error reading stdin\n");
			exit(1);
		}
	}
	x->length = r;
	*/

	x->length = PACKETS*PACKETSIZE;
	memset(x->buffer,0xaa,x->length);
	/*
	int i;
	for (i=0;i<BUFSIZE;i+=64) {
		int f=(i>=BUFSIZE/2);
		x->buffer[i] = 0x02;
		x->buffer[i+63] = 0x40;
	}
	*/

	libusb_submit_transfer(x);
}

double t;
int total;

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
	struct libusb_transfer *xfrs[NUMTRANSFERS];

	libusb_init(NULL);
	libusb_device_handle *dev = libusb_open_device_with_vid_pid(NULL,0x03eb,0x204f);
	assert(dev);

	int i;
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
