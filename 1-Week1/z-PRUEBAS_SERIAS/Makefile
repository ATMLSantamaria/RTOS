CFLAGS = -O0 -g

beowulfpc: dosProcesos.c
	gcc $(CFLAGS) -o dosProcesos  dosProcesos.c -lrt -pthread
primer: 1-Thread.c
	gcc $(CFLAGS) -o primerThread  1-Thread.c -lrt -pthread

segundo: 2-fifo-thread.c
	gcc $(CFLAGS) -o fifoThread  2-fifo-thread.c -lrt -pthread

resumen: 3-resumen-affinity-priority.c
	gcc $(CFLAGS) -o resumen 3-resumen-affinity-priority.c -lrt -pthread



clean:
	rm -f dosProcesos
	rm -f primerThread
	rm -f fifoThread
	rm -f resumen
