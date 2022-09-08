.SILENT: all debug app slave view clean clean-pvs clean-all check-pvs-aux check-pvs

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

.PHONY: clean-pvs
clean-pvs:
	rm -f PVS-Studio.log report.tasks strace_out

.PHONY: clean-all
clean-all: clean clean-pvs

.PHONY: check-pvs-aux
check-pvs-aux:
	printf "\033[0;36m--- PVS OUTPUT ---\n\033[0m"
	pvs-studio-analyzer trace -- make
	pvs-studio-analyzer analyze
	plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
	printf "\033[0;36m--- END PVS OUTPUT ---\n\033[0m"

.PHONY: check-pvs
check-pvs: clean-all check-pvs-aux
