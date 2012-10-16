hello: hello.o
	gcc hello.c -o hello

hello.o: hello.c
	gcc hello.c -c -o hello.o

run:
	./hello

clean:
	-rm -f hello.o hello
