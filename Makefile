layout-switcher: main.o
	gcc main.o -o layout-switcher
main.o: main.c
	gcc -c main.c
clean:
	rm main.o layout-switcher
