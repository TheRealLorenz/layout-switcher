layout-switcher: main.o stringlib.o checklib.o
	gcc main.o stringlib.o checklib.o -o layout-switcher -lpthread
main.o: main.c
	gcc -c main.c
stringlib.o: stringlib.c
	gcc -c stringlib.c
checklib.o: checklib.c
	gcc -c checklib.c
clean:
	rm main.o layout-switcher
install: layout-switcher
	cp ./layout-switcher /usr/local/bin/	
uninstall:
	rm /usr/local/bin/layout-switcher
