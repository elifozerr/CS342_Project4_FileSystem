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
int mounted = 0;
// ========================================================

typedef struct superBlock{
  //to fill the block elements
  char foo[BLOCKSIZE-(sizeof(int))];
  int blockCount;
}superBlock;

typedef struct openFileTableEntry {

  char name[MAX_FILE_NAME];
  struct FCB *fcb;
  int accessMode;
  int file_offset;
  int available;
  int openNum;

} openFileTableEntry;

struct openFileTableEntry openFileTable[MAX_FILE_OPEN];

typedef struct FCB {

  char foo[DIR_SIZE - 4*sizeof(int)];
  int isUsed;
  int index_table_block;
  int index;
  int fileSize;

} FCB;

struct FCB fcb_table[FCB_SIZE];

typedef struct dirEntry {

  char fileName[MAX_FILE_NAME];
  char foo[DIR_SIZE-(sizeof(char)*MAX_FILE_NAME)-(sizeof(int)*2)];
  int FCB_index;
  int available;

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
  unsigned int bitmap[BITMAP_ROW_COUNT];
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
      bitmap[count_zeros] -= pow(2, where_one_right);
      printf("new block %d\n", bitmap[count_zeros]);
      if (write_block((void *) bitmap, bitmap_block) == -1) {
        printf("bitmap write error\n");
        return -1;
      }
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
  int bitmap_block_index = block_index / (BITMAP_ROW_SIZE * BITMAP_ROW_COUNT) + 1;
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
int create_format_vdisk (char *vdiskname, unsigned int m) {
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

  //block 1-2-3-4 contain the bitmap
  for (int i = 1; i <= 4; i++) {
    unsigned int bitmap[BITMAP_ROW_COUNT];
    if (i == 1) {
      bitmap[0] = 0x0007FFFF;
      for (int i = 1; i < BITMAP_ROW_COUNT; i++) {
        bitmap[i] = UINT_MAX;
      }
    }
    else {
      for (int i = 0; i < BITMAP_ROW_COUNT; i++) {
        bitmap[i] = UINT_MAX;
      }
    }

    memcpy(block, (void *) bitmap, BITMAP_ROW_COUNT * sizeof(unsigned int));
    if (write_block(block,i) != 0)
      return -1;
  }

  printf("size of dirEntry = %ld bytes\n", sizeof(struct dirEntry));
  printf("size of openFileTableEntry = %ld bytes\n", sizeof(struct openFileTableEntry));
  printf("size of foo = %ld\n", DIR_SIZE-(sizeof(char)*MAX_FILE_NAME)-(sizeof(int)*2));

  //block 5-6-7-8 contain the directory structure
  for (int i = 5; i <= 8; i++) {
    memcpy(block,dirStructure, 32*(sizeof(struct dirEntry)));
    if (write_block(block,i) != 0)
      return -1;
  }

  for(int i = 0; i<FCB_SIZE; i++) {
    fcb_table[i].isUsed=0;
    fcb_table[i].index=i;
    fcb_table[i].index_table_block=-1;
  }

  //init fcb blocks
  int block_index = 9;
  int fcb_block_count = 4;
  for(int i = 0; i<fcb_block_count;i++) {
    memcpy(block,fcb_table,32*(sizeof(struct FCB)));
    printf("FCB: %ld\n", 32*(sizeof(struct FCB)));
    if(write_block(block,block_index) != 0 ) return -1;
    block_index++;
  }

  //initialize open file table
  for (int i = 0; i < MAX_FILE_OPEN; i++) {
    openFileTable[i].available = 1;
  }

  return (0);
}


// implemented
int sfs_mount (char *vdiskname) {
  // simply open the Linux file vdiskname and in this
  // way make it ready to be used for other operations.
  // vdisk_fd is global; hence other function can use it.
  vdisk_fd = open(vdiskname, O_RDWR);
  mounted = 1;
  return(0);
}


// implemented
int sfs_umount () {
  fsync (vdisk_fd); // copy everything in memory to disk
  close (vdisk_fd);
  mounted = 0;
  return (0);
}

//create  a  new  file with  name filename
int sfs_create(char *filename) {
    //check size of file name
    if( sizeof(filename) > MAX_FILE_NAME){
      printf("ERROR!! - Size of file name is greater than 110.\n" );
      return(-1);
    }

    if(mounted == 0){
      printf("ERROR! - Not mounted\n" );
      return(-1);
    }
    else{
      //search file in directory to check whether the file exists
      for(int i = 0; i< MAX_FILE; i++){

        if(strcmp(dirStructure[i].fileName,filename)==0){
          //return(sfs_open(filename,))
          printf("file already exists in directory, can not be created again");
          return(-1);
        }
      }

      int emptyFound = 0;
      //find available directory entry to put the file
      for(int i =0; i<DIR_SIZE;i++){
          if(dirStructure[i].available==1){
            emptyFound=1;
            strcpy(dirStructure[i].fileName,filename);
            dirStructure[i].available=0;

            //search blocks 9-10-11-12 to find not used FCB_index
            int index =0;
            for(int i = 0; i<FCB_SIZE;i++){
              if(fcb_table[i].isUsed == 0){
                index = i;
              }
            }
            dirStructure[i].FCB_index=index;

            fcb_table[i].isUsed=1;
            fcb_table[i].index = index;

            int free_block = find_free_block();
            printf("ındex table will be inserted to free block :%d\n",free_block );
            fcb_table[i].index_table_block= free_block;
            char block[BLOCKSIZE];
            int index_table[1024];
            for(int i = 0;i<1024; i++){
              index_table[i]=-1;
            }
            memcpy(block,index_table, 1024*(sizeof(int)));
            if(write_block(block,free_block) != 0 ) return -1;
            printf("index table is added to block %d\n", free_block);
            printf("%s file is created\n" ,filename);
            return(0);
          }
      }
    }
    return (0);
}


int sfs_open(char *file, int mode)
{

    int file_index=-1;
    int empty_index=-1;
    int dir_index =-1;
    if(mounted){

        int check_found = 0;
        int check_found_dir=0;
        int found_in_table = 0;

        //search open file table to check whether file is open
        for(int i = 0;i< MAX_FILE_OPEN;i++){
            if(check_found==0){
                if( (openFileTable[i].available==0) && strcmp(openFileTable[i].name,file)==0){
                    check_found = 1;
                    openFileTable[i].openNum += 1;
                    openFileTable[i].file_offset=0;
                    openFileTable[i].accessMode = mode; //??
                    file_index=1;
                    return file_index;

                }
            }
        }
        //find file by searching directory structure
        for(int i=0;i<DIR_SIZE;i++){

              if(check_found_dir!=1 && dirStructure[i].available==0
                && strcmp(dirStructure[i].fileName,file)==0 ){
                  check_found_dir = 1;
                  //search open file to find empty read_position
                  for(int j =0; j<MAX_FILE_OPEN; j++){

                      if(openFileTable[j].available != 0 && found_in_table!=1){
                          found_in_table=1;
                          empty_index = j;
                          dir_index = i;
                          printf("empty location found in open file table in %d \n",empty_index);
                          break;
                      }
                  }
                  break;
              }
        }
    }

    //dirStructure[dir_index].FCB_index
    openFileTable[empty_index].fcb = &(fcb_table[dir_index]);
    openFileTable[empty_index].openNum = 1;
    openFileTable[empty_index].accessMode= mode;
    strcpy(openFileTable[empty_index].name,file);
    openFileTable[empty_index].file_offset=0;
    openFileTable[empty_index].available=0;
    printf("file is added to open file table\n" );
    return empty_index;
}

int sfs_close(int fd){

    if(mounted){
        //check whether the the file in open file fcb_table
        if(openFileTable[fd].available == 1){
          printf("the index of fd %d is empty in the open file table\n", fd);
          return (-1);
        }
        else{

          openFileTable[fd].openNum -=1;
          if(openFileTable[fd].openNum ==0){
            openFileTable[fd].available = 1;
            char name [MAX_FILE_NAME];
            strcpy(name,openFileTable[fd].name);
            printf("the file %s is closed\n", name);
            return(0);
          }
        }
    }
    else{
        printf("ERROR - Not mounted\n" );
        return(-1);
    }

}

int sfs_getsize (int  fd)
{
    return (0);
}

int sfs_read(int fd, void *buf, int n) {
    if (mounted) { // check if mount operation is done
      if(openFileTable[fd].accessMode == MODE_READ) { // if access mode is read
        if (!openFileTable[fd].available) { // if file is opened
          int index_table_block = openFileTable[fd].fcb->index_table_block; // get the index table block of the file
          int *index_table;
          if (read_block((void *) index_table, index_table_block) == -1) { // get the index table
            printf("index_table read error\n");
            return -1;
          }

          if (index_table[openFileTable[fd].file_offset / BLOCKSIZE] != -1) { // check if the file has content in it
            int read_position = openFileTable[fd].file_offset / BLOCKSIZE; // get the index table's index to start reading from
            int block_to_read = index_table[read_position]; // get the block index which contains the content of the file from the index table
            int block_pos = openFileTable[fd].file_offset % BLOCKSIZE; // get the position where to start in the content block

            char content_block[BLOCKSIZE];
            if (read_block(content_block, block_to_read) == -1) {
              printf("content read error\n");
              return -1;
            }
            int bytesRead = 0; // read bytes
            int no_more_blocks = 0; // 1 if no blocks left to read
            char buffer[n]; // temporary buffer
            int buffer_pos = 0; // temporary buffer position to write

            /* while there are blocks to read, read less than n bytes,
              and read less than the file size */
            while ((no_more_blocks == 0) && (bytesRead < n)
                    && bytesRead < openFileTable[fd].fcb->fileSize) {
              memcpy(&buffer[buffer_pos], &content_block[block_pos], 1); //copy byte by byte to the temporary buffer
              buffer_pos++;
              block_pos++;
              bytesRead++;
              openFileTable[fd].file_offset++;

              if (block_pos >= BLOCKSIZE) { // if block is finished
                read_position = read_position + 1;
                block_to_read = index_table[read_position]; // get the next content block from the index table
                if (block_to_read != -1) {
                  if (read_block(content_block, block_to_read) == -1) {
                    printf("content read error\n");
                    return -1;
                  }
                  block_pos = 0;
                }
                else {
                  no_more_blocks = 0; // there are no blocks left
                }
              }
            }
            memcpy(buf, buffer, n); // copy the temporary buffer to the actual buffer
            return bytesRead;
          }
        }
      }
    }
    return -1;
}


int sfs_append(int fd, void *buf, int n) {
  if (mounted) { // check if mount operation is done
   if(openFileTable[fd].accessMode == MODE_APPEND) { // if access mode is read
     if (!openFileTable[fd].available) { // if file is opened

       //get the appended content
       char buffer[n];
       memcpy(buffer, buf, n);

       int *index_table;
       int index_table_block = openFileTable[fd].fcb->index_table_block; // get the index table block of the file
       if (read_block((void *) index_table, index_table_block) == -1) { // get the index table
         printf("index_table read error\n");
         return -1;
       }

       int bytes_written = 0;
       int index_table_index = 0;

       if (index_table[index_table_index] == -1) { // no content, first write
         char block[BLOCKSIZE];
         int block_count = (int) (n / BLOCKSIZE);
         int last_block_write = n % BLOCKSIZE;

         int free_block;
         int buf_index = 0;

         for (int i = 0; i < block_count; i++) {

           if ((free_block = find_free_block()) == -1) {
             printf("no free blocks left\n");
             return bytes_written;
           }

           if (i != block_count) {
             for (int j = 0; j < BLOCKSIZE; j++) {
               memcpy(&block[j], &buffer[buf_index], 1);
               buf_index++;
               bytes_written++;
               openFileTable[fd].fcb->fileSize++;
             }
           }
           else {
             for (int j = 0; j < last_block_write; j++) {
               memcpy(&block[j], &buffer[buf_index], 1);
               buf_index++;
               bytes_written++;
               openFileTable[fd].fcb->fileSize++;
             }
           }

           write_block(block, free_block);
           index_table[index_table_index] = free_block;
           index_table_index++;
           openFileTable[fd].file_offset = bytes_written;
         }
       }
       else {
         int write_position = openFileTable[fd].file_offset / BLOCKSIZE; // get the index table's index to start writing from
         int block_to_write = index_table[write_position]; // get the block index which contains the content of the file from the index table
         int block_pos = openFileTable[fd].file_offset % BLOCKSIZE; // get the position where to start in the content block

         char block[BLOCKSIZE];
         if (read_block(block, block_to_write) == -1) { // get the block to write
           printf("block to write read error\n");
           return -1;
         }

         int buf_index = 0;

         while (bytes_written < n) {
           memcpy(&block[block_pos], &buffer[buf_index], 1);
           bytes_written++;
           openFileTable[fd].fcb->fileSize++;
           block_pos++;
           openFileTable[fd].file_offset++;
           buf_index++;

           if (block_pos >= BLOCKSIZE) {
             if (write_block(block, block_to_write) == -1) {
               printf("cannot write to block");
               return -1;
             }
             if ((block_to_write = find_free_block()) == -1) {
               printf("no free blocks left\n");
               return bytes_written;
             }

             if (read_block(block, block_to_write) == -1) { // get the block to write
               printf("block to write read error\n");
               return -1;
             }

             block_pos = 0;
           }
         }
         write_block(block, block_to_write);
         return bytes_written;
       }
     }
   }
 }
 return -1;
}

int sfs_delete(char *filename)
{
    return (0);
}

int main(int argc, char const *argv[]) {
  /* code */
  create_format_vdisk("disk",20);

  sfs_mount("disk");
  sfs_create("elif");
  sfs_open("elif", 1);
  sfs_create("elif-dosya-oldu");
  sfs_open("elif-dosya-oldu",0);
  sfs_close(0);
  sfs_close(1);
  /*char block[BLOCKSIZE];
=======
  char block[BLOCKSIZE];

  for (int i = 0; i < 2 * BITMAP_ROW_COUNT; i++) {
    printf("FREE = %d\n", find_free_block());
  }

>>>>>>> 3b0cdd326df874b91e91304c44c0b36200494361
  read_block(block, 1);
  int c, k;
  for (int i = 0; i < BITMAP_ROW_COUNT; i++) {
    for (c = 31; c >= 0; c--) {
      k = ((unsigned int *)block)[i] >> c;
      if (k & 1)
        printf("1");
      else
        printf("0");
    }
    printf("\n");
  }*/
  return 0;
}
