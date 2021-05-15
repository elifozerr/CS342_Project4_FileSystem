#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "simplefs.c"

int main(int argc, char **argv)
{
    int ret;
    char vdiskname[200];
    int m;

    if (argc != 3) {
	printf ("usage: create_format <vdiskname> <m>\n");
	exit(1);
    }

    strcpy (vdiskname, argv[1]);
    m = atoi(argv[2]);

    printf ("started\n");

    ret  = create_format_vdisk (vdiskname, m);
    if (ret != 0) {
        printf ("there was an error in creating the disk\n");
        exit(1);
    }
    /*int c, k;
    for (int i = 0; i < 1024; i++) {
      for (c = 31; c >= 0; c--) {
        k = ((unsigned int *)block)[i] >> c;
        if (k & 1)
          printf("1");
        else
          printf("0");
      }
      printf("\n");
    }*/

    printf ("disk created and formatted. %s %d\n", vdiskname, m);
}
