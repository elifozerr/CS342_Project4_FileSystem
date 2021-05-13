#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "simplefs.h"
#include <limits.h>
#include <math.h>

#define BITMAP_ROW_SIZE 32
#define BITMAP_ROW_COUNT 1024

#define MAX_FILE_NAME 110
#define DIR_SIZE 128
#define FCB_SIZE 128
#define MAX_FILE_OPEN 16
#define MAX_FILE 128



// Global Variables =======================================
int vdisk_fd; // Global virtual disk file descriptor. Global within the library.
              // Will be assigned with the vsfs_mount call.
              // Any function in this file can use this.
              // Applications will not use  this directly.

int sizeOfDisk;
int blockNum;
char name_disk[256];
// ========================================================

typedef struct superBlock{
  //to fill the block elements
  char foo[BLOCKSIZE-(sizeof(int))];
  int blockCount;
}superBlock;

typedef struct openFileTableEntry {

  char name[MAX_FILE_NAME];
  int accessMode;
  int file_offset;
  int available;
  int openNum;

} openFileTableEntry;

struct openFileTableEntry openFileTable[MAX_FILE_OPEN];

typedef struct FCB {

  int isUsed;
  int index_table_block;
  int index;

} FCB;

struct FCB fcb_table[FCB_SIZE];

typedef struct dirEntry {

  char fileName[MAX_FILE_NAME];
  int FCB_index;
  int available;
  char foo[DIR_SIZE-(sizeof(char)*MAX_FILE_NAME)-(sizeof(int)*2)];

} dirEntry;

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

int find_free_block() {
  int count_zeros = 0;
  int found = 0;
  int bitmap_block = 1;
  unsigned int *bitmap;
  if (read_block((void *) bitmap, bitmap_block) == -1) {
    printf("bitmap read error\n");
    return -1;
  }

  while (bitmap_block <= 4) {
    unsigned int row = bitmap[count_zeros];
    printf("row = %d\n", row);
    if (row == 0) {
      count_zeros++;
      printf("count zeros incremented: %d\n", count_zeros);
    }
    else {
      int where_one_right = (int) log2(row);
      printf("from right %d\n", where_one_right);
      int where_one_left = BITMAP_ROW_SIZE - where_one_right;
      printf("from left %d\n", where_one_left);
      block[count_zeros] -= pow(2, where_one_right);
      printf("new block %d\n", block[count_zeros]);
      printf("block = %d\n", count_zeros * BITMAP_ROW_SIZE + where_one_left - 1);
      return count_zeros * BITMAP_ROW_SIZE + where_one_left - 1 + (bitmap_block - 1) * BITMAP_ROW_SIZE * BITMAP_ROW_COUNT;
    }

    if (count_zeros == BITMAP_ROW_COUNT) {
      count_zeros = 0;
      bitmap_block++;
      if (read_block((void *) bitmap, bitmap_block) == -1) {
        printf("bitmap read error\n");
        return -1;
      }
    }
  }

  return -1;
}

int free_block(int block_index) {
  int row = (int) (block_index / BITMAP_ROW_SIZE);
  int row_pos = block_index % BITMAP_ROW_SIZE;
  unsigned int *block;
  bitmap_block_index = block_index / (BITMAP_ROW_SIZE * BITMAP_ROW_COUNT) + 1
  read_block((void *) block, bitmap_block_index);
  block[row] += pow(2, BITMAP_ROW_SIZE - (row_pos + 1));
  write_block((void *) block, bitmap_block_index);
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

    //create disk
    int disk = open(vdiskname, O_CREAT|O_RDWR ,0666);
    if(disk == -1){
      printf("Disk is not created\n" );
      exit(-1);
    }

    char buffer[BLOCKSIZE];
    int n;
    //erase data in BLOCKSIZE bytes from mem starting from buffer
    bzero((void*)buffer, BLOCKSIZE);
    vdisk_fd = open(vdiskname, O_RDWR);
    int numBlocks = size/BLOCKSIZE;
    //write to the blocks
    for(int i = 0; i< numBlocks; i++){
      n = write(vdisk_fd,buffer,BLOCKSIZE);

    }
    close(vdisk_fd);

    printf("Virtual disk is created\n" );

    //format operations of the disk starting here
    vdisk_fd=open(vdiskname,O_RDWR);

    //init directory structure
    for(int i = 0; i<DIR_SIZE; i++){
      strcpy(dirStructure[i].fileName,"");
      //dirStructure[i].fileName="";
      dirStructure[i].available = 1;
      dirStructure[i].FCB_index=-1;

    }

    //init openFileTable
    for(int i = 0; i<MAX_FILE_OPEN;i++){
      strcpy(openFileTable[i].name,"");
      openFileTable[i].accessMode=-1;
      openFileTable[i].file_offset=0;
      openFileTable[i].available = 1;
      openFileTable[i].openNum=0;
      //openFileTable[i].name="";
    }

    //define block
    char block[BLOCKSIZE];

    //block 5-6-7-8 contain the directory structure
    //copy 1/4 of the directory into block 6
    memcpy(block,dirStructure, 32*(sizeof(struct dirEntry)));
    //insert block into disk
    if (write_block(block,5) != 0) return -1;
    //copy 1/4 of the directory into block 7
    memcpy(block,dirStructure, 32*(sizeof(struct dirEntry)));
    //insert block into disk
    if (write_block(block,6) != 0) return -1;
    //copy 1/4 of the directory into block 8
    memcpy(block,dirStructure, 32*(sizeof(struct dirEntry)));
    //insert block into disk
    if (write_block(block,7) != 0) return -1;
    //copy 1/4 of the directory into block 9
    memcpy(block,dirStructure, 32*(sizeof(struct dirEntry)));
    //insert block into disk
    if (write_block(block,8) != 0) return -1;

    for(int i = 0; i<FCB_SIZE; i++){
      fcb_table[i].isUsed=0;
      fcb_table[i].index=i;
      fcb_table[i].index_table_block=-1;

    }

    //init fcb index_table_block
    int block_index = 9;
    int fcb_block_count = 4;
    for(int i = 0; i<fcb_block_count;i++){
      memcpy(block,fcb_table,32*(sizeof(struct FCB)));
      if(write_block(block,block_index) != 0 ) return -1;
      block_index++;
    }

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
