MY_SEATEST_GIT_PROJECT_PATH = ~/dev/private_dev/my_libs/seatest/src
CC = gcc
FILES = example_project.c test_suite_one.c test_suite_two.c $(MY_SEATEST_GIT_PROJECT_PATH)/seatest.c
OUT_EXE = example

build: $(FILES)
	$(CC) -o $(OUT_EXE) $(FILES) -I $(MY_SEATEST_GIT_PROJECT_PATH)

abort_if_failed:
	$(CC) -D ABORT_TEST_IF_ASSERT_FAIL -o $(OUT_EXE) $(FILES) -I $(MY_SEATEST_GIT_PROJECT_PATH)

clean:
	rm -f *.o core

rebuild: clean build
