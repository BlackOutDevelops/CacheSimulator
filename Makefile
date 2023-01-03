all:
	gcc -g -o SIM SIM.c ${LIBS}
clean:
	rm *~ *.o SIM
