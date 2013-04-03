CC	= gcc
CFLAGS	= -g 
OBJS  	= galois.o jerasure.o reed_sol.o cauchy.o liberation.o

all: ../lib/libJerasure.a headers

headers:
	mkdir -p "../include/"
	cp *.h ../include/

../lib/libJerasure.a: $(OBJS)
	ar rcs  ../lib/libJerasure.a $(OBJS)

../lib:
	mkdir -p ../lib

clean:
	rm -f ../lib/libJerasure.* $(OBJS) ../include/*

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $<

galois.o: galois.h
jerasure.o: jerasure.h galois.h
reed_sol.o: jerasure.h galois.h reed_sol.h
cauchy.o: jerasure.h galois.h cauchy.h
liberation.o: jerasure.h galois.h liberation.h

.PHONY: all headers
