CC := gcc
CFLAGS := -Wall -Wextra
BINARY := mint

$(BINARY): main.o vm.o
	$(CC) $(CFLAGS) vm.o main.o -o $(BINARY)
	rm -f *.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

vm.o: vm.c vm.h op_table.h
	$(CC) $(CFLAGS) -c vm.c -o vm.o

clean:
	rm -f $(BINARY) *.o