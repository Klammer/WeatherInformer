CC=g++
CFLAGS_GMP=-lcurl -ltidy
CFLAGS_WND=-lX11
CFLAGS_ALL=$(CFLAGS_WND) $(CFLAGS_GMP) -lpthread

all: full

full: main.o src_CMN.o src_GMP.o src_WND.o
	$(CC) $(CFLAGS_ALL) main.o src_CMN.o src_GMP.o src_WND.o -o WeatherInformer

main.o: main.cpp
	$(CC) -c $(CFLAGS_ALL) main.cpp

src_CMN.o: src_CMN.cpp
	$(CC) -c src_CMN.cpp

src_GMP.o: src_GMP.cpp
	$(CC) -c $(CFLAGS_GMP) src_GMP.cpp

src_WND.o: src_WND.cpp
	$(CC) -c $(CFLAGS_WND) src_WND.cpp

clean:
	rm -rf *.o full
