build:
	gcc -g process_generator.c -o process_generator.out
	gcc -g clk.c -o clk.out
	gcc -g scheduler.c  memory.c  -o scheduler.out -lm
	gcc -g process.c -o process.out
	gcc -g test_generator.c -o test_generator.out



clean:
	rm -f *.out 

all: clean build

run:
	./process_generator.out "processes.txt" -sch 1 -q 2

test:
	./test_generator.out
 