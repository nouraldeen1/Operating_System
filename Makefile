build:
	gcc -g process_generator.c -o outs/process_generator.out
	gcc -g clk.c -o outs/clk.out
	
	gcc -g scheduler.c  memory.c queue.c -o outs/scheduler.out -lm
	gcc -g process.c -o outs/process.out
	gcc -g test_generator.c -o outs/test_generator.out



clean:
	rm -f outs/*.out 

all: clean build

run:
	./outs/process_generator.out "inputs_logs/processes.txt" -sch 3 -q 3

test:
	./outs/test_generator.out
 