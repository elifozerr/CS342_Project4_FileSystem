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
    int fd2;
    char buffer[1];
    char vdiskname[200];
    struct timeval start, end;
    long int create_time, open_time, total_write_time, close_time,
              read_time1, read_time10, read_time100, read_time1000,
              read_time10000;

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
    sfs_create ("file2.bin");
    gettimeofday(&end, NULL);
    create_time = find_elapsed_time_ms(&end, &start);

    gettimeofday(&start, NULL);
    fd2 = sfs_open ("file2.bin", MODE_APPEND);
    gettimeofday(&end, NULL);
    open_time = find_elapsed_time_ms(&end, &start);

    gettimeofday(&start, NULL);
    for (int i = 0; i < 50000; ++i) {
        buffer[0] = (char) 'b';
        sfs_append (fd2, (void *) buffer, 1);
    }
    gettimeofday(&end, NULL);
    total_write_time = find_elapsed_time_ms(&end, &start);

    sfs_close(fd2);

    fd2 = sfs_open ("file2.bin", MODE_READ);

    char buf[10000];
    printf("---1 byte per read---\n");
    gettimeofday(&start, NULL);
    for (int i = 0; i < 10000; ++i) {
      sfs_read(fd2, (void *) buf, 1);
    }
    gettimeofday(&end, NULL);
    printf("---done---\n");
    read_time1 = find_elapsed_time_ms(&end, &start);

    printf("---10 bytes per read---\n");
    gettimeofday(&start, NULL);
    for (int i = 0; i < 1000; ++i) {
      sfs_read(fd2, (void *) buf, 10);
    }
    gettimeofday(&end, NULL);
    printf("---done---\n");
    read_time10 = find_elapsed_time_ms(&end, &start);

    printf("---100 bytes per read---\n");
    gettimeofday(&start, NULL);
    for (int i = 0; i < 100; ++i) {
      sfs_read(fd2, (void *) buf, 100);
    }
    gettimeofday(&end, NULL);
    printf("---done---\n");
    read_time100 = find_elapsed_time_ms(&end, &start);

    printf("---1000 bytes per read---\n");
    gettimeofday(&start, NULL);
    for (int i = 0; i < 10; ++i) {
      sfs_read(fd2, (void *) buf, 1000);
    }
    gettimeofday(&end, NULL);
    printf("---done---\n");
    read_time1000 = find_elapsed_time_ms(&end, &start);

    printf("---10000 bytes per read---\n");
    gettimeofday(&start, NULL);
    sfs_read(fd2, (void *) buf, 10000);
    gettimeofday(&end, NULL);
    printf("---done---\n");
    read_time1000 = find_elapsed_time_ms(&end, &start);

    gettimeofday(&start, NULL);
    sfs_close(fd2);
    gettimeofday(&end, NULL);
    close_time = find_elapsed_time_ms(&end, &start);

    ret = sfs_umount();

    printf("\nCreate Time for Experiment 2: %ld\n", create_time);
    printf("Open Time for Experiment 2: %ld\n", open_time);
    printf("Total Write Time for Experiment 2: %ld\n", total_write_time);
    printf("Read Time for 10000 Bytes for Experiment 2 (1 byte per read): %ld\n", read_time1);
    printf("Read Time for 10000 Bytes for Experiment 2 (10 byte per read): %ld\n", read_time10);
    printf("Read Time for 10000 Bytes for Experiment 2 (100 byte per read): %ld\n", read_time100);
    printf("Read Time for 10000 Bytes for Experiment 2 (1000 byte per read): %ld\n", read_time1000);
    printf("Read Time for 10000 Bytes for Experiment 2 (10000 byte per read): %ld\n", read_time10000);
    printf("Close Time for Experiment 2: %ld\n", close_time);
}
