CC := gcc
CFLAGS := -Wall -Wextra -g
BINARY := mint

$(BINARY): main.o vm.o terminal.o page.o
	$(CC) $(CFLAGS) page.o terminal.o vm.o main.o -o $(BINARY)
	rm -f *.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

vm.o: vm.c
	$(CC) $(CFLAGS) -c vm.c -o vm.o

terminal.o: interfaces/terminal.c
	$(CC) $(CFLAGS) -c interfaces/terminal.c -o terminal.o

page.o: interfaces/page.c
	$(CC) $(CFLAGS) -c interfaces/page.c -o page.o

clean:
	rm -f $(BINARY) *.o
