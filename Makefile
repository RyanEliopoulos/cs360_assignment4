all: connect.c
	gcc -o connect connect.c
clean:
	rm connect.c README.txt Makefile connect
run:
	./connect ${invoke}
