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

struct dir_entry_t* get_file_entry(char* filename, struct dir_entry_t* dir_entry, int dir_block_count) {
  int i = 1;
  while (i <= dir_block_count) {
    if (strcmp(filename, (char*)dir_entry->filename) == 0) {
      return dir_entry;
    }
    dir_entry += i++;
  }
  return NULL;
}

void copy_file(void* address, void* new_address, int fat_start, int starting_block, int block_size, int file_size) {
  int fat_entry = starting_block;
  int bytes_remaining = file_size;
  int data_block = fat_entry * block_size;

  while (fat_entry != -1) {
    int i = 0;
    for (i = 0; i < block_size; i += 4) {
      if (bytes_remaining == 0) break;
      int offset = i + data_block;
      int data = 0;

      memcpy(&data, (char*)address + offset, 4);
      memcpy((char*)new_address + (file_size - bytes_remaining), &data, 4);
      bytes_remaining -= 4;
    }

    int fat_location = fat_start * block_size;
    fat_entry = fat_entry * 4;
    memcpy(&fat_entry, (char*)address + (fat_location + fat_entry), 4);
    fat_entry = htonl(fat_entry);
    data_block = fat_entry * block_size;
  }
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    printf("Enter the correct arguments: diskget <file system image> <file 1> <file 2>.\n");
    return(EXIT_FAILURE);
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    printf("Failed to open disk image.\n");
    close(fd);
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
  uint32_t fat_start = htonl(superblock->fat_start_block);
  uint32_t root_dir_start_block = htonl(superblock->root_dir_start_block);
  uint32_t root_dir_block_count = htonl(superblock->root_dir_block_count);
  int offset = (root_dir_start_block) * block_size;
  struct dir_entry_t* root_dir_entry = (void*)((char*)address + offset);
  struct dir_entry_t* file_entry = get_file_entry(argv[2], root_dir_entry, (int)root_dir_block_count);
  uint32_t file_size = htonl(file_entry->size);
  uint32_t starting_block = htonl(file_entry->starting_block);

  if (file_entry != NULL && file_size > 0) {
    int new_fd = open(argv[3], O_RDWR | O_CREAT, 0666);
    if (new_fd < 0) {
      printf("Failed to open disk image.\n");
      close(new_fd);
      return(EXIT_FAILURE);
    }

    if (lseek(new_fd, file_size - 1, SEEK_SET) == -1) {
      munmap(address, buffer.st_size);
      close(new_fd);
      close(fd);
      printf("Failed to stretch file.\n");
      return(EXIT_FAILURE);
    }
    
    if (write(new_fd, "", 1) == -1) {
      munmap(address, buffer.st_size);
      close(new_fd);
      close(fd);
      printf("Failed to write last byte of file.\n");
      return(EXIT_FAILURE);
    }

    void* new_address = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, new_fd, 0);
    if (new_address == MAP_FAILED) {
      printf("Failed to map disk image.\n");
      close(fd);
      return(EXIT_FAILURE);
    }

    copy_file(address, new_address, (int)fat_start, (int)starting_block, (int)block_size, (int)file_size);

    munmap(new_address, file_size);
    close(new_fd);
  } else {
    printf("File not found.\n");
  }

  munmap(address, buffer.st_size);
  close(fd);
  return(EXIT_SUCCESS);
}
