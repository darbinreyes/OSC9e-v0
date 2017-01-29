
all:
	gcc -lpthread pp5.41.c barrier.c
	# make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

build:
	gcc -lpthread pp5.41.c barrier.c

buildnosleep:
	gcc -lpthread pp5.41.c barrier.c -DSLEEP_TIME_DISABLED

buildusecs:
	gcc -lpthread pp5.41.c barrier.c -DMICRO_SECONDS_TIME_UNITS

clean:
	rm a.out
	# make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
run:
	./a.out

runpiped:
	./a.out > log_stdout.txt 2> log_stderr.txt