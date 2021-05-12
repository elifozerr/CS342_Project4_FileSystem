#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "simplefs.h"

#define MAX_FILE_NAME 110
#define DIR_SIZE 128
#define FCB_SIZE 128
#define MAX_FILE 16



// Global Variables =======================================
int vdisk_fd; // Global virtual disk file descriptor. Global within the library.
              // Will be assigned with the vsfs_mount call.
              // Any function in this file can use this.
              // Applications will not use  this directly.
// ========================================================
int sizeOfDisk;
int blockNum;
char name_disk[256];


typedef struct openFileTableEntry{

  char name[MAX_FILE_NAME];
  int accessMode;
  int offset;
  int available;
  int openNum;

}

struct openFileTableEntry openFileTable[MAX_FILE];
typedef struct FCB{

  int isUsed;
  int size;
  int blockNo;

}FCB;


typedef struct dirEntry{

  char fileName[MAX_FILE_NAME];
  struct FCB fcb;

}dirEntry;

struct dirEntry dirStructure[DIR_SIZE];
// read block k from disk (virtual disk) into buffer block.
// size of the block is BLOCKSIZE.
// space for block must be allocated outside of this function.
// block numbers start from 0 in the virtual disk.
int read_block (void *block, int k)
{
    int n;
    int offset;

    offset = k * BLOCKSIZE;
    lseek(vdisk_fd, (off_t) offset, SEEK_SET);
    n = read (vdisk_fd, block, BLOCKSIZE);
    if (n != BLOCKSIZE) {
	printf ("read error\n");
	return -1;
    }
    return (0);
}

// write block k into the virtual disk.
int write_block (void *block, int k)
{
    int n;
    int offset;

    offset = k * BLOCKSIZE;
    lseek(vdisk_fd, (off_t) offset, SEEK_SET);
    n = write (vdisk_fd, block, BLOCKSIZE);
    if (n != BLOCKSIZE) {
	printf ("write error\n");
	return (-1);
    }
    return 0;
}


/**********************************************************************
   The following functions are to be called by applications directly.
***********************************************************************/

// this function is partially implemented.
/*
*create and  format a  virtual  disk. Size will be 2^m bytes.
*initialize/create an sfs  file  system  on  the  virtual  disk
*On-disk  file  system  structures  initialized  on  the virtual disk.
*/
int create_format_vdisk (char *vdiskname, unsigned int m)
{
    char command[1000];
    int size;
    int num = 1;
    int count;
    size  = num << m;
    count = size / BLOCKSIZE; //block count
    //    printf ("%d %d", m, size);
    sprintf (command, "dd if=/dev/zero of=%s bs=%d count=%d",
             vdiskname, BLOCKSIZE, count);
    //printf ("executing command = %s\n", command);
    system (command);

    // now write the code to format the disk below.
    // .. your code...

    size = pow(2,m);
    //create disk
    int disk = open(vdiskname, O_CREAT|O_RDWR ,0666);
    if(disk == -1){
      printf("Disk is not created\n" );
      exit(-1);
    }

    char buffer[BLOCKSIZE];

    //erase data in BLOCKSIZE bytes from mem starting from buffer
    bzero((void*)buffer, BLOCKSIZE);
    vdisk_fd = open(vdiskname, O_RDWR);
    int numBlocks = size/BLOCKSIZE;
    //write to the blocks
    for(int i = 0; i< numBlocks; i++){
      n = write(fd,buffer,BLOCKSIZE);

    }
    close(fd_vdisk);

    printf("Virtual disk is created\n" );

    //format operations of the disk starting here


    return (0);
}


// implemented
int sfs_mount (char *vdiskname)
{
    // simply open the Linux file vdiskname and in this
    // way make it ready to be used for other operations.
    // vdisk_fd is global; hence other function can use it.
    vdisk_fd = open(vdiskname, O_RDWR);
    return(0);
}


// implemented
int sfs_umount ()
{
    fsync (vdisk_fd); // copy everything in memory to disk
    close (vdisk_fd);
    return (0);
}

//create  a  new  file with  name filename
int sfs_create(char *filename)
{

    return (0);
}


int sfs_open(char *file, int mode)
{
    return (0);
}

int sfs_close(int fd){
    return (0);
}

int sfs_getsize (int  fd)
{
    return (0);
}

int sfs_read(int fd, void *buf, int n){
    return (0);
}


int sfs_append(int fd, void *buf, int n)
{
    return (0);
}

int sfs_delete(char *filename)
{
    return (0);
}
