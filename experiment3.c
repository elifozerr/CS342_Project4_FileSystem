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
    long int create_time, open_time, close_time,
              write_time1, write_time10, write_time100, write_time1000,
              write_time10000;

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
    create_time = find_elapsed_time_ms(&end, &start);

    gettimeofday(&start, NULL);
    fd3 = sfs_open ("file3.bin", MODE_APPEND);
    gettimeofday(&end, NULL);
    open_time = find_elapsed_time_ms(&end, &start);

    printf("---1 byte per write---\n");
    gettimeofday(&start, NULL);
    for (int i = 0; i < 10000; ++i) {
        buffer[0] = (char) 'c';
        sfs_append (fd3, (void *) buffer, 1);
    }
    gettimeofday(&end, NULL);
    printf("---done---\n");
    write_time1 = find_elapsed_time_ms(&end, &start);

    printf("---10 byte per write---\n");
    gettimeofday(&start, NULL);
    for (int i = 0; i < 1000; ++i) {
        for (int j = 0; j < 10; j++) {
          buffer[j] = 'c';
        }
        sfs_append (fd3, (void *) buffer, 10);
    }
    gettimeofday(&end, NULL);
    printf("---done---\n");
    write_time10 = find_elapsed_time_ms(&end, &start);

    printf("---100 byte per write---\n");
    gettimeofday(&start, NULL);
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 100; j++) {
          buffer[j] = 'c';
        }
        sfs_append (fd3, (void *) buffer, 100);
    }
    gettimeofday(&end, NULL);
    printf("---done---\n");
    write_time100 = find_elapsed_time_ms(&end, &start);

    printf("---1000 byte per write---\n");
    gettimeofday(&start, NULL);
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 1000; j++) {
          buffer[j] = 'c';
        }
        sfs_append (fd3, (void *) buffer, 1000);
    }
    gettimeofday(&end, NULL);
    printf("---done---\n");
    write_time1000 = find_elapsed_time_ms(&end, &start);

    printf("---10000 byte per write---\n");
    gettimeofday(&start, NULL);
    for (int j = 0; j < 10000; j++) {
      buffer[j] = 'c';
    }
    sfs_append (fd3, (void *) buffer, 10000);
    gettimeofday(&end, NULL);
    printf("---done---\n");
    write_time10000 = find_elapsed_time_ms(&end, &start);

    sfs_close(fd3);

    gettimeofday(&start, NULL);
    sfs_close(fd3);
    gettimeofday(&end, NULL);
    close_time = find_elapsed_time_ms(&end, &start);

    ret = sfs_umount();

    printf("\nCreate Time for Experiment 3: %ld\n", create_time);
    printf("Open Time for Experiment 3: %ld\n", open_time);
    printf("Write Time for 10000 Bytes for Experiment 3 (1 byte per write): %ld\n", write_time1);
    printf("Write Time for 10000 Bytes for Experiment 3 (10 byte per write): %ld\n", write_time10);
    printf("Write Time for 10000 Bytes for Experiment 3 (100 byte per write): %ld\n", write_time100);
    printf("Write Time for 10000 Bytes for Experiment 3 (1000 byte per write): %ld\n", write_time1000);
    printf("Write Time for 10000 Bytes for Experiment 3 (10000 byte per write): %ld\n", write_time10000);
    printf("Close Time for Experiment 3: %ld\n", close_time);
}
