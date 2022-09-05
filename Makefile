.SILENT: all debug app slave view clean

GCC := gcc -Wall -std=c99 -pthread -lrt
debug: DEBUGFLAG := -g

.PHONY: all
all: clean app slave view

.PHONY: debug
debug: all

app: app.c shm.c
	$(GCC) $(DEBUGFLAG) shm.c app.c -o md5

slave: slave.c
	$(GCC) $(DEBUGFLAG) slave.c -o slave

view: view.c shm.c
	$(GCC) $(DEBUGFLAG) shm.c view.c -o view

.PHONY: clean
clean:
	rm -f md5 slave view results.txt