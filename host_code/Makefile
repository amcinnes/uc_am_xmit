LIBS=`pkg-config --libs libusb-1.0`
CFLAGS=`pkg-config --cflags libusb-1.0`

write: write.o
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

clean:
	rm -f write *.o
