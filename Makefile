all:
	gcc -shared -fPIC -o dlta.so dlta.c

install:
	install -m 0755 *.so /usr/lib/deadbeef/

clean:
	rm dlta.so

uninstall:
	rm /usr/lib/deadbeef/dlta.so
