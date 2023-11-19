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

// Super block
struct __attribute__((__packed__)) superblock_t{
    uint8_t fs_id[8];
    uint16_t block_size;
    uint32_t file_system_block_count;
    uint32_t fat_start_block;
    uint32_t fat_block_count;
    uint32_t root_dir_start_block;
    uint32_t root_dir_block_count;
};

// Time and date entry
struct __attribute__((__packed__)) dir_entry_timedate_t {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

// Directory entry
struct __attribute__((__packed__)) dir_entry_t {
    uint8_t status;
    uint32_t starting_block;
    uint32_t block_count;
    uint32_t size;
    struct dir_entry_timedate_t create_time;
    struct dir_entry_timedate_t modify_time;
    uint8_t filename[31];
    uint8_t unused[6];
};
    

int print_disklist(int fd,struct stat buffer){
    int status = fstat(fd, &buffer);
    void* address = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // Get address in disk image file via memory map
    struct superblock_t* superblock;
    superblock = (struct superblock_t*) address;

    int blocksize = htons(superblock->block_size);
    int blockcount = htonl(superblock->file_system_block_count);
	int rootblockcount = htonl(superblock->root_dir_block_count);
	int rootstartblock = htonl(superblock->root_dir_start_block);
    
    int startingbyte = (rootstartblock)*blocksize;

    void* entirefile = mmap(NULL, blockcount*blocksize, PROT_READ | PROT_WRITE, MAP_SHARED, fd,0);

	char stat;
    char filename[1000];
    int file_size;
    // int counter = 0;
    // int reserved = 0;
    // int available = 0;
    // int allocated = 0;
	int jump = 0;   
	int spacing1 = 0; 


    for(int i = 0; i < rootblockcount*(blocksize/64); i++){
    	memcpy(&stat, entirefile+startingbyte+jump, 1);
		memcpy(&filename, entirefile+startingbyte+jump+27, 31); 
    	if(stat & (1 << 0)){
 			if(stat & (1 << 1)){
				printf("F ");
			}
			else if(stat & (1 << 2)){
				printf("D ");
			}
			memcpy(&file_size, entirefile+startingbyte+jump+9, 4);
			file_size = ntohl(file_size);
			printf("%10d ", file_size);
			for(int name_count = 27; name_count < 58; name_count++){
				memcpy(&stat, entirefile+startingbyte+jump+name_count, 1);
				if(spacing1 == 0){
					printf("%30c", stat);
					spacing1 = 1;
				}
				else if(spacing1 == 1){
					printf("%c", stat);
				}
			}
            // printf("%s",filename);

			memcpy(&file_size, entirefile+startingbyte+jump+20, 2);
			file_size = ntohs(file_size);
			printf("\t%d/", file_size);
			memcpy(&stat, entirefile+startingbyte+jump+22, 1);
			printf("%.2d/", stat);
			memcpy(&stat, entirefile+startingbyte+jump+23, 1);
			printf("%.2d ", stat);
			memcpy(&stat, entirefile+startingbyte+jump+24, 1);
			printf("%.2d:", stat);
			memcpy(&stat, entirefile+startingbyte+jump+25, 1);
			printf("%.2d:", stat);
			memcpy(&stat, entirefile+startingbyte+jump+26, 1);
			printf("%.2d", stat);
			printf("\n");

		}
		spacing1 = 0;
		jump = jump + 64;
    	//counter = counter + 4;
    }

    munmap(address,buffer.st_size);
    close(fd);
     

}

int main(int argc, char* argv[]) 
{
    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    // Reference: https://stackoverflow.com/questions/56109844/what-is-the-different-between-struct-stat-buffer-and-buffer-in-linux-stat-fun
    // printf("%c".argv[2]);
    print_disklist(fd, buffer);  
}