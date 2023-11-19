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
#include "diskstructs.h"
int print_disklist(int fd,struct stat buffer, char* target);
char get_file_type(uint8_t file_type);
void print_sub_dir(int sub_dir_starting_block, int sub_dir_blcok_count, int blcoksize, void* address,void* entirefile,struct superblock_t* superblock);

int print_disklist(int fd,struct stat buffer, char* target){
    // printf("%s size: %d\n",target,(int)sizeof(target));
    int status = fstat(fd, &buffer);
    void* address = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // Get address in disk image file via memory map
    struct superblock_t* superblock;
    superblock = (struct superblock_t*) address;

    int blocksize = htons(superblock->block_size);
    int blockcount = htonl(superblock->file_system_block_count);
	int root_block_count = htonl(superblock->root_dir_block_count);
	int root_start_block = htonl(superblock->root_dir_start_block);
    
    int starting_byte = (root_start_block)*blocksize;

    // printf("FAT starts: %d\n",htonl(superblock->fat_start_block));
    // printf("FAT blocks: %d\n",htonl(superblock->fat_block_count));

    void* entirefile = mmap(NULL, blockcount*blocksize, PROT_READ | PROT_WRITE, MAP_SHARED, fd,0);

	int jump = 0;


    struct dir_entry_t* directory;

    for(int i = 0; i < root_block_count*(blocksize/64); i++){
        struct dir_entry_t* dir_entry = (void*)((char*)entirefile + starting_byte + jump);
        struct dir_entry_timedate_t create_time_struct = dir_entry->create_time;

        if(target != NULL){
            if(!strcmp(dir_entry->filename,target)){
                // printf("Find the Target: %s\n",dir_entry->filename);
                // printf("Starting Block %d \n", htonl(dir_entry->starting_block));
                // printf("Size %d \n", htonl(dir_entry->size));
                print_sub_dir(htonl(dir_entry->starting_block),htonl(dir_entry->block_count),blocksize,address,entirefile,superblock);
            }
        }else if(target == NULL){
            if(htonl(dir_entry->size) != 0){
                printf("%c %10d %30s %04d/%02d/%02d %02d:%02d:%02d\n",
                    get_file_type(dir_entry->status), htonl(dir_entry->size), dir_entry->filename,
                    htons(create_time_struct.year), create_time_struct.month, create_time_struct.day,
                    create_time_struct.hour, create_time_struct.minute, create_time_struct.second);
            }
        }
        // If the argv[2] is not empty the system will find the targe for the user, if argv[2] is empty it will print out the root
        // directory.

        jump = jump + 64;
    }
   
    munmap(address,buffer.st_size);
    close(fd);
}

char get_file_type(uint8_t file_type) {
  if (file_type == 5) return 'D';
  else return 'F';
}

void print_sub_dir(int sub_dir_starting_block, int sub_dir_blcok_count, int blocksize, void* address,void* entirefile,struct superblock_t* superblock){
    // printf("print_sub_dir\n");
    void* nowaddress = address;
    int starting_byte = (sub_dir_starting_block);
    int fatstartblcok = htonl(superblock->fat_start_block);
    int jump = 0;
    int counter = 8;
    struct dir_entry_t* file_name;
    for(int i = 0; i < sub_dir_blcok_count*(blocksize/64); i++){
        counter++;
        if(counter%8 == 0){
            memcpy(&starting_byte,address+(fatstartblcok*blocksize)+4*starting_byte,4);
            starting_byte=htonl(starting_byte);
        }
        // Change the directory to the FAT location
        struct dir_entry_t* file_name = (void*)((char*)entirefile + starting_byte*blocksize + jump);
        //Store each file info in the struct for accesss
        struct dir_entry_timedate_t create_time_struct = file_name->create_time;
        if(htonl(file_name->size) != 0){
            printf("%c %10d %30s %04d/%02d/%02d %02d:%02d:%02d\n",
                get_file_type(file_name->status), htonl(file_name->size), file_name->filename,
                htons(create_time_struct.year), create_time_struct.month, create_time_struct.day,
                create_time_struct.hour, create_time_struct.minute, create_time_struct.second);
        }
        // It print out the directy info
        jump = jump + 64;
    }
}

int main(int argc, char* argv[]) 
{
    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    // Reference: https://stackoverflow.com/questions/56109844/what-is-the-different-between-struct-stat-buffer-and-buffer-in-linux-stat-fun
    // printf("%s size: %d\n",argv[2],(int)sizeof(argv[2]));
    if(argv[2] != NULL){
        int size_of_target = sizeof(argv[2]);
        char target_dir_name[size_of_target-1];
        for(int counter = 1; counter < size_of_target;counter++){
            target_dir_name[counter-1] = argv[2][counter];
        }
        print_disklist(fd, buffer,target_dir_name);

    }else{
       print_disklist(fd, buffer,NULL);  
    } 
    // If the argv[2] is not empty the system will find the targe for the user, if argv[2] is empty it will print out the root
    // directory.
}