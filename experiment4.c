#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "simplefs.c"

int find_elapsed_time_ms(struct timeval *end, struct timeval *start) {
    long int elapsed_ms = (end->tv_usec + 1000000 * end->tv_sec) - (start->tv_usec + 1000000 * start->tv_sec);
    return elapsed_ms;
}

int main(int argc, char **argv)
{
    int ret;
    int fd3;
    char buffer[10000];
    char vdiskname[200];
    struct timeval start, end;
    long int create_time1, create_time4, create_time16, create_time64,create_time128;
    printf ("started\n");

    if (argc != 2) {
        printf ("usage: app  <vdiskname>\n");
        exit(0);
    }
    strcpy (vdiskname, argv[1]);

    ret = sfs_mount (vdiskname);
    if (ret != 0) {
        printf ("could not mount \n");
        exit (1);
    }

    printf ("creating files\n");

    gettimeofday(&start, NULL);
    sfs_create ("file3.bin");
    gettimeofday(&end, NULL);
    create_time1 = find_elapsed_time_ms(&end, &start);

    char fileName[110];
    strcpy(fileName,"akca4");
    for(int i = 0; i<4;i++){
        gettimeofday(&start,NULL);
        sfs_create (fileName);
        gettimeofday(&end,NULL);
        create_time4 += find_elapsed_time_ms(&end,&start);
        strcat(fileName,"a");
    }

    //create 16 files
    strcpy(fileName,"akca16");
    for(int i = 0; i<16;i++){
        gettimeofday(&start,NULL);
        sfs_create (fileName);
        gettimeofday(&end,NULL);
        create_time16 += find_elapsed_time_ms(&end,&start);
        strcat(fileName,"a");
    }

    //create 64 files
    strcpy(fileName,"akca64");
    for(int i = 0; i<64;i++){
        gettimeofday(&start,NULL);
        sfs_create (fileName);
        gettimeofday(&end,NULL);
        create_time64 += find_elapsed_time_ms(&end,&start);
        strcat(fileName,"a");
    }

    //create 128 files
    /*char fileName[110];
    strcpy(fileName,"akca64");
    for(int i = 0; i<127;i++){
        gettimeofday(&start,NULL);
        sfs_create (fileName);
        gettimeofday(&end,NULL);
        create_time128 += find_elapsed_time_ms(&end,&start);
        if(i = 100){
          strcpy(fileName,"akca07");
        }
        strcat(fileName,"a");
    }*/



    ret = sfs_umount();

    printf("\nCreate Time for Experiment 4 1 file: %ld\n", create_time1);
    printf("\nCreate Time for Experiment 4 4 file: %ld\n", create_time4);
    printf("\nCreate Time for Experiment 4 16 file: %ld\n", create_time16);
    printf("\nCreate Time for Experiment 4 64 file: %ld\n", create_time64);

    printf("\nCreate Time for Experiment 4 128 file: %ld\n", create_time128);

}
