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
    int fd1;
    char c;
    char buffer[1];
    char vdiskname[200];
    struct timeval start, end;
    long int create_time, open_time, total_write_time, close_time,
            read_time100, read_time1000, read_time10000, read_time100000;

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
    sfs_create ("file1.bin");
    gettimeofday(&end, NULL);
    create_time = find_elapsed_time_ms(&end, &start);

    gettimeofday(&start, NULL);
    fd1 = sfs_open ("file1.bin", MODE_APPEND);
    gettimeofday(&end, NULL);
    open_time = find_elapsed_time_ms(&end, &start);

    gettimeofday(&start, NULL);
    for (int i = 0; i < 111100; ++i) {
        buffer[0] = (char) 'a';
        sfs_append (fd1, (void *) buffer, 1);
    }
    gettimeofday(&end, NULL);
    total_write_time = find_elapsed_time_ms(&end, &start);

    sfs_close(fd1);
    fd1 = sfs_open ("file1.bin", MODE_READ);

    char buf[1];
    gettimeofday(&start, NULL);
    for (int i = 0; i < 100; ++i) {
      sfs_read(fd1, (void *) buf, 1);
    }
    gettimeofday(&end, NULL);
    read_time100 = find_elapsed_time_ms(&end, &start);

    gettimeofday(&start, NULL);
    for (int i = 100; i < 1100; ++i) {
      sfs_read(fd1, (void *) buf, 1);
    }
    gettimeofday(&end, NULL);
    read_time1000 = find_elapsed_time_ms(&end, &start);

    gettimeofday(&start, NULL);
    for (int i = 1100; i < 11100; ++i) {
      sfs_read(fd1, (void *) buf, 1);
    }
    gettimeofday(&end, NULL);
    read_time10000 = find_elapsed_time_ms(&end, &start);

    gettimeofday(&start, NULL);
    for (int i = 11100; i < 111100; ++i) {
      sfs_read(fd1, (void *) buf, 1);
    }
    gettimeofday(&end, NULL);
    read_time100000 = find_elapsed_time_ms(&end, &start);

    gettimeofday(&start, NULL);
    sfs_close(fd1);
    gettimeofday(&end, NULL);
    close_time = find_elapsed_time_ms(&end, &start);

    ret = sfs_umount();

    printf("\nCreate Time for Experiment 1: %ld\n", create_time);
    printf("Open Time for Experiment 1: %ld\n", open_time);
    printf("Total Write Time for Experiment 1: %ld\n", total_write_time);
    printf("Read Time for 100 Bytes for Experiment 1: %ld\n", read_time100);
    printf("Read Time for 1000 Bytes for Experiment 1: %ld\n", read_time1000);
    printf("Read Time for 10000 Bytes for Experiment 1: %ld\n", read_time10000);
    printf("Read Time for 100000 Bytes for Experiment 1: %ld\n", read_time100000);
    printf("Close Time for Experiment 1: %ld\n", close_time);
}
