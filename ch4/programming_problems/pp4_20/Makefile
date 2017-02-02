CC = gcc
PROB_NAME = pp4.20
SOURCE_FILES = $(PROB_NAME).c pid_manager.c
OUT_EXE = $(PROB_NAME).bin

all:
	$(CC) -o $(OUT_EXE) -lpthread $(SOURCE_FILES)
  # make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

build:
	$(CC) -o $(OUT_EXE) -lpthread $(SOURCE_FILES)

buildnosleep:
	$(CC) -o $(OUT_EXE) -lpthread $(SOURCE_FILES) -DSLEEP_TIME_DISABLED

buildusecs:
	$(CC) -o $(OUT_EXE) -lpthread $(SOURCE_FILES) -DMICRO_SECONDS_TIME_UNITS

clean:
	rm $(OUT_EXE)
  # make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
run:
	./$(OUT_EXE)

runpiped:
	./$(OUT_EXE) > log_stdout.txt 2> log_stderr.txt