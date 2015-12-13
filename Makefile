

all: libbuddy.a app


libbuddy.a: 	buddy.c
	gcc -g -Wall -c buddy.c
	ar -cvq libbuddy.a buddy.o
	ranlib libbuddy.a

app: 	app.c
	gcc -Wall -g -o app app.c  -L. -lbuddy


clean: 
	rm -fr *.o *.a *~ a.out app
