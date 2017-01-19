
all:
	gcc -lpthread ch5_pp2_dpp.c
	# make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

build:
	gcc -lpthread ch5_pp2_dpp.c

buildnosleep:
	gcc -lpthread ch5_pp2_dpp.c -DSLEEP_TIME_DISABLED

buildusecs:
	gcc -lpthread ch5_pp2_dpp.c -DMICRO_SECONDS_TIME_UNITS

clean:
	rm a.out
	# make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
run:
	./a.out

runpiped:
	./a.out > log_stdout.txt 2> log_stderr.txt