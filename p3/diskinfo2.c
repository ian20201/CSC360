#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>

struct __attribute__((__packed__)) superblock{
    uint8_t   fs_id[8];
    uint16_t  block_size;
    uint32_t  file_system_block_count;
    uint32_t  fat_start_block;
    uint32_t  fat_block_count;
    uint32_t  root_dir_start_block;
    uint32_t  root_dir_block_count;
};

int print_DiskInfo(int fd, struct stat buffer)
{
    int status = fstat(fd, &buffer);

	//Superblock Extraction

    int blocksize = 0;

    char* address = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	memcpy(&blocksize, address+8, 2);
	blocksize = htons(blocksize);

    struct superblock* block;
    block = (struct superblock*)address;

    //FAT Info. Extraction     

    int free = 0, reserved = 0, alloc = 0;
  
    int fatStart = ntohl(block->fat_start_block);
    int fatBlocks = ntohl(block->fat_block_count);
    
    int pos_start = blocksize*fatStart;   
    int pos_end = blocksize*(fatBlocks+fatStart);     
    
    while(pos_start < pos_end)
    {
        int curr = 0;
        memcpy(&curr, address+pos_start, 4);
        curr = ntohl(curr);
        if(curr==0)
        {
            free++;
        }else if(curr==1)
        {
            reserved++;
        }else
        {
            alloc++;
        }
        pos_start+=4; 
    }
    
    //Printing Everything 

    //Superblock Information
    printf("Super block information:\n");
    printf("Block Size: %d\n", blocksize);
    printf("Block count: %d\n",ntohl(block->file_system_block_count));
    printf("FAT starts: %d\n", ntohl(block->fat_start_block));
    printf("FAT blocks: %d\n", ntohl(block->fat_block_count));
    printf("Root directory start: %d\n", ntohl(block->root_dir_start_block));
    printf("Root directory blocks: %d\n", ntohl(block->root_dir_block_count));
    printf("\n");
    
    //FAT Information
    printf("FAT Information:\n");
    printf("Free Blocks: %d\n", free);
    printf("Reserved Blocks: %d\n", reserved);
    printf("Allocated Blocks: %d\n", alloc);
    printf("\n");

    munmap(address, buffer.st_size);
    return 0;
}


int main(int argc, char* argv[]) 
{
    int fd = open(argv[1], O_RDWR);
 	struct stat buffer;

    print_DiskInfo(fd, buffer);

    //close(fd);   
}