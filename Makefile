CC=g++
CFLAGS=-std=c++17 -Wall -Wextra

all: main

main:
	$(CC) $(CFLAGS) -c main.cpp -o main.o
	$(CC) $(CFLAGS) -c traceroute.cpp -o traceroute.o
	$(CC) $(CFLAGS) -c icmp.cpp -o icmp.o
	$(CC) $(CFLAGS) -c func.cpp -o func.o
	$(CC) $(CFLAGS) -o traceroute main.o traceroute.o icmp.o func.o

clean:
	rm -vf *.o

distclean:
	rm -vf *.o
	rm -vf traceroute