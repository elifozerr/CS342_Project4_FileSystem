all: libsimplefs.a create_format app

libsimplefs.a: 	simplefs.c
	gcc -Wall -w -c simplefs.c -lm
	ar -cvq libsimplefs.a simplefs.o
	ranlib libsimplefs.a

create_format: create_format.c
	gcc -Wall -w -o create_format  create_format.c   -L. -lsimplefs -lm

app: 	app.c
	gcc -Wall -w -o app app.c  -L. -lsimplefs -lm

clean:
	rm -fr *.o *.a *~ a.out app  vdisk create_format
