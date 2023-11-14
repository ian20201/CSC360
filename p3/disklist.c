#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>

struct __attribute__((__packed__)) superblock_t{
    uint8_t fs_id[8];
    uint16_t block_size;
    uint32_t file_system_block_count;
    uint32_t fat_start_block;
    uint32_t fat_block_count;
    uint32_t root_dir_start_block;
    uint32_t root_dir_block_count;
};

int print_disklist(int fd,struct stat buffer){

}

int main(int argc, char* argv[]) 
{
    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    // Reference: https://stackoverflow.com/questions/56109844/what-is-the-different-between-struct-stat-buffer-and-buffer-in-linux-stat-fun

    print_disklist(fd, buffer);  
}