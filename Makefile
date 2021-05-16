all: libsimplefs.a create_format app experiment1 experiment2 experiment3 experiment4

libsimplefs.a: 	simplefs.c
	gcc -Wall -w -c simplefs.c -lm
	ar -cvq libsimplefs.a simplefs.o
	ranlib libsimplefs.a

create_format: create_format.c
	gcc -Wall -w -o create_format  create_format.c   -L. -lsimplefs -lm

app: 	app.c
	gcc -Wall -w -o app app.c  -L. -lsimplefs -lm

experiment1: 	experiment1.c
	gcc -Wall -w -o experiment1 experiment1.c  -L. -lsimplefs -lm

experiment2: 	experiment2.c
	gcc -Wall -w -o experiment2 experiment2.c  -L. -lsimplefs -lm

experiment3: 	experiment3.c
	gcc -Wall -w -o experiment3 experiment3.c  -L. -lsimplefs -lm

experiment4: 	experiment4.c
	gcc -Wall -w -o experiment4 experiment4.c  -L. -lsimplefs -lm

clean:
	rm -fr *.o *.a *~ a.out app experiment1 experiment2 experiment3 experiment4 vdisk create_format
