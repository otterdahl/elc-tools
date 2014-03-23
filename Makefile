parcopy: parcopy.o serial_posix.o
	gcc -o $@ parcopy.o serial_posix.o

elc3sim: elc3sim.o serial_posix.o
	gcc -o $@ elc3sim.o serial_posix.o

elc3sim.o: elc3sim.c
	gcc -c elc3sim.c

parcopy.o: parcopy.c serial_posix.h
	gcc -c parcopy.c 

serial_posix.o: serial_posix.c serial_posix.h
	gcc -c serial_posix.c

.PHONY: clean
clean:
	-rm -f parcopy elc3sim *.o
