LIBS=`pkg-config --libs libusb-1.0` -lm
CFLAGS=`pkg-config --cflags libusb-1.0` -O3

write: write.o
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

clean:
	rm -f write *.o
