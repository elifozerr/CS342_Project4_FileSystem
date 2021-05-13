#include <stdio.h>
#include <limits.h>
#include <math.h>

#define BITMAP_ROW_SIZE 32
#define BITMAP_ROW_COUNT 1024

unsigned int block[BITMAP_ROW_COUNT];

int find_free_block() {
  int count_zeros = 0;
  int found = 0;

  while (!found) {
    unsigned int row = block[count_zeros];
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
      found = 1;
      return count_zeros * BITMAP_ROW_SIZE + where_one_left - 1;
    }

    if (count_zeros == BITMAP_ROW_COUNT) {
      break;
    }
  }

  return -1;
}

int free_block(int block_index) {
  int row = (int) (block_index / BITMAP_ROW_SIZE);
  int row_pos = block_index % BITMAP_ROW_SIZE;
  block[row] += pow(2, BITMAP_ROW_SIZE - (row_pos + 1));
}

int main() {
  block[0] = 0x0007FFFF;
  for (int i = 1; i < BITMAP_ROW_COUNT; i++) {
    block[i] = UINT_MAX;
  }

  printf("log2(int max) = %f", log2(UINT_MAX));

  for (int i = 0; i < 100; i++)
    printf("Block %d is free\n", find_free_block());

  free_block(15);
  free_block(5);
  free_block(63);
  free_block(64);

  int c, k;
  for (int i = 0; i < BITMAP_ROW_COUNT; i++) {
    for (c = 31; c >= 0; c--) {
      k = block[i] >> c;
      if (k & 1)
        printf("1");
      else
        printf("0");
    }
    printf("\n");
  }
}
