#CFLAGS = -DDEBUG
CFLAGS =

beowulfpc: dosProcesos.c
	gcc $(CFLAGS) -o dosProcesos  dosProcesos.c -lrt -pthread
primer: 1-Thread.c
	gcc $(CFLAGS) -o primerThread  1-Thread.c -lrt -pthread
clean:
	rm -f dosProcesos
	rm -f primerThread
