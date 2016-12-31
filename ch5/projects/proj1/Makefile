
all:
	gcc -lpthread posix-unnamed-sem-TA.c
  # make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

build:
	gcc -lpthread posix-unnamed-sem-TA.c

buildnosleep:
	gcc -lpthread posix-unnamed-sem-TA.c -DSLEEP_TIME_DISABLED

buildusecs:
	gcc -lpthread posix-unnamed-sem-TA.c -DMICRO_SECONDS_TIME_UNITS

clean:
	rm a.out
  # make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
run:
	./a.out

runpiped:
	./a.out > log_stdout.txt 2> log_stderr.txt