#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>

// I want about 100ms total buffer, at 1MB/s
// so each of the two buffers should be about 50KB
#define BUFSIZE 51200
#define NUMBUFS 2

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

	x->length = BUFSIZE;
	memset(x->buffer,0x55,x->length);

	libusb_submit_transfer(x);
}

double dtime() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec + 1e-6 * tv.tv_usec;
}

double t;
int total;

void callback(struct libusb_transfer *xfr) {
	if (xfr->status!=LIBUSB_TRANSFER_COMPLETED) {
		fprintf(stderr,"Transfer failed: %d\n",xfr->status);
		exit(1);
	}
	assert(xfr->actual_length==xfr->length);
	transfer(xfr);

	total += xfr->actual_length;
	double rate = total/(dtime()-t);
	fprintf(stderr,"%10d %lf\n",total,rate);
}

int main() {
	struct libusb_transfer *xfrs[NUMBUFS];

	libusb_init(NULL);
	libusb_device_handle *dev = libusb_open_device_with_vid_pid(NULL,0x03eb,0x204f);
	assert(dev);

	t = dtime();

	int i;
	for (i=0;i<NUMBUFS;i++) {
		xfrs[i] = libusb_alloc_transfer(0);
		char *buf = malloc(BUFSIZE);
		libusb_fill_bulk_transfer(xfrs[i],dev,0x01,buf,0,callback,NULL,0);
		transfer(xfrs[i]);
	}
	while (1) libusb_handle_events(NULL);
}
