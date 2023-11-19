#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "diskstructs.h"

int get_fat_type_block_count(void* address, int fat_start, int block_size, int block_count, int value) {
  int block_type_count = 0;
  int fat_end = (fat_start + block_count - 1) * block_size;
  int i = 0;
  for (i = 0; i < fat_end; i += 4) {
    int block = 0;
    int offset = fat_start * block_size + i;
    memcpy(&block, (char*) address + offset, 4);

    if ((int)htonl(block) == value) block_type_count++;
  }
  return block_type_count;
}

int get_fat_alloc_block_count(int block_count, int fat_free_blocks, int fat_reserv_blocks) {
  return block_count - (fat_free_blocks + fat_reserv_blocks);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Enter the correct arguments: diskinfo <file system image>.\n");
    return(EXIT_FAILURE);
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    printf("Failed to open disk image.\n");
    return(EXIT_FAILURE);
  }

  struct stat buffer;
  void* address = mmap(NULL, buffer.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (address == MAP_FAILED) {
    printf("Failed to map disk image.\n");
    close(fd);
    return(EXIT_FAILURE);
  }

  struct superblock_t* superblock = address;
  uint16_t block_size = htons(superblock->block_size);
  uint32_t block_count = htonl(superblock->file_system_block_count);
  uint32_t fat_start = htonl(superblock->fat_start_block);
  uint32_t fat_block_count = htonl(superblock->fat_block_count);
  int fat_free_blocks = get_fat_type_block_count(address, fat_start, block_size, fat_block_count, 0);
  int fat_reserv_blocks = get_fat_type_block_count(address, fat_start, block_size, fat_block_count, 1);
  int fat_alloc_blocks = get_fat_alloc_block_count(block_count, fat_free_blocks, fat_reserv_blocks);

  printf("Super block information:\n");
  printf("Block size: %d\n", block_size);
  printf("Block count: %d\n", block_count);
  printf("FAT starts: %d\n", fat_start);
  printf("FAT blocks: %d\n", htonl(superblock->fat_block_count));
  printf("Root directory start: %d\n", htonl(superblock->root_dir_start_block));
  printf("Root directory blocks: %d\n\n", htonl(superblock->root_dir_block_count));
  printf("FAT information:\n");
  printf("Free Blocks: %i\n", fat_free_blocks);
  printf("Reserved Blocks: %i\n", fat_reserv_blocks);
  printf("Allocated Blocks: %i\n", fat_alloc_blocks);

  munmap(address, buffer.st_size);
  close(fd);
  return(EXIT_SUCCESS);
}
