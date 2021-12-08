layout-switcher: main.o stringlib.o
	gcc main.o stringlib.o -o layout-switcher
main.o: main.c
	gcc -c main.c
stringlib.o: stringlib.c
	gcc -c stringlib.c
clean:
	rm main.o layout-switcher
install: layout-switcher
	cp ./layout-switcher /usr/local/bin/	
uninstall:
	rm /usr/local/bin/layout-switcher
