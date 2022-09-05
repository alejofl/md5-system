.SILENT: all debug app slave view clean

GCC := gcc -Wall -std=c99
debug: DEBUGFLAG := -g

.PHONY: all
all: app slave view

.PHONY: debug
debug: all

app: app.c
	$(GCC) $(DEBUGFLAG) app.c -o md5

slave: slave.c
	$(GCC) $(DEBUGFLAG) slave.c -o slave

view: view.c
	$(GCC) $(DEBUGFLAG) view.c -o view

.PHONY: clean
clean:
	rm -f md5 slave view results.txt