CC=gcc
CFLAGS=-fsanitize=address -Wvla -Wall -Werror -g -std=gnu11 -lasan


# fill in all your make rules

vm_x2017: 
	echo "vm_x2017"
	$(CC) -o vm_x2017 vm_x2017.c $(CFLAGS)

objdump_x2017: 
	echo "objdump_x2017"
	$(CC) -o objdump_x2017 objdump_x2017.c $(CFLAGS)

tests: 
	echo "tests"
	$(CC) -o test test.c $(CFLAGS)	

run_tests:
	echo "run_tests"
	./test

clean:
	echo "clean"
	rm vm_x2017 objdump_x2017 test

