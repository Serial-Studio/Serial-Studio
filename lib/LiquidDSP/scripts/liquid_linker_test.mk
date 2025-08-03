# simple makefile to test linking to liquid after library has been installed

all: check

check: liquid_linker_test
	./$<

liquid_linker_test: % : scripts/%.c
	$(CC) -Wall -O2 -o $@ $< -lm -lc -lliquid

clean:
	rm -f liquid_linker_test

