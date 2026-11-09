all: central

central: central.c central.h
	gcc central.c -o central

clean:
	rm -f central *.o 