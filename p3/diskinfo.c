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

int print_DiskInfo(int fd,struct stat buffer){
    int status = fstat(fd, &buffer);
    void* address = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // Get the address for the fd
    // mmap() creates a new mapping in the virtual address space of the
    // calling process.  The starting address for the new mapping is
    // specified in addr.  The length argument specifies the length of
    // the mapping (which must be greater than 0).
    // Reference: https://man7.org/linux/man-pages/man2/mmap.2.html

    struct superblock_t* superblock;
    superblock = (struct superblock_t*) address;
    printf("Super block information\n");
    printf("Block size: %d\n", htons(superblock->block_size));
    printf("Block count: %d\n",htonl(superblock->file_system_block_count));
    printf("FAT starts: %d\n",htonl(superblock->fat_start_block));
    printf("FAT blocks: %d\n",htonl(superblock->fat_block_count));
    printf("Root directory starts: %d\n",htonl(superblock->root_dir_start_block));
    printf("Root directory blocks: %d\n",htonl(superblock->root_dir_block_count));

    int fat_starting_blcok = htonl(superblock->fat_start_block);
    int fat_block_count = htonl(superblock->fat_block_count);
    int blocksize = htons(superblock->block_size);
    int blockcount = htonl(superblock->file_system_block_count);

    int starting_byte = (fat_starting_blcok)*blocksize;

    void* entirefile = mmap(NULL, blockcount*blocksize, PROT_READ | PROT_WRITE, MAP_SHARED, fd,0);
    //It times the number of the superblock and blocksize, and read through all the block

    int files_system_size;
    int counter = 0;
    int reserved = 0;
    int available = 0;
    int allocated = 0;


    for(int tracker = 0; tracker < fat_block_count*(blocksize/4); tracker++){
        memcpy(&files_system_size, entirefile+starting_byte+counter, 4);
        files_system_size = ntohl(files_system_size);
        if(files_system_size == 1){
    		reserved++;
    	}else if(files_system_size == 0){
    		available++;
    	}else{
    		allocated++;
    	}
        counter = counter + 4;
    }

    printf("\nFAT information\n");
    printf("Free blocks: %d\n",available);
    printf("Reserved blocks: %d\n",reserved);
    printf("Allocated blocks: %d\n",allocated);

    munmap(address,buffer.st_size);
    // The munmap() function removes the mappings for pages in the range [addr, addr + len) rounding 
    // the len argument up to the next multiple of the page size as returned by sysconf(). 
    // If addr is not the address of a mapping established by a prior call to mmap(), 
    // the behavior is undefined. After a successful call to munmap() and before any 
    // subsequent mapping of the unmapped pages, further references to these pages will result 
    // in the delivery of a SIGBUS or SIGSEGV signal to the process.
    // Reference: https://www.ibm.com/docs/en/zos/2.2.0?topic=functions-munmap-unmap-pages-memory

    close(fd);
}

int main(int argc, char* argv[]) 
{
    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    // Reference: https://stackoverflow.com/questions/56109844/what-is-the-different-between-struct-stat-buffer-and-buffer-in-linux-stat-fun

    print_DiskInfo(fd, buffer);  
}