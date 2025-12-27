CC = gcc

objects = shell.o 

output: $(objects)
	$(CC) -o shell shell.o 

.PHONY: clean
clean:
	rm $(objects) shell 