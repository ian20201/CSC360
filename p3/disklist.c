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

#define MAX_IN_COMMAND 100
#define MAX_IN_CHARS 200

int pathcounter = 0;
int end = 1; //Set End to be FALSE

int print_disklist(int fd,struct stat buffer, char *path[MAX_IN_COMMAND]);
char get_file_type(uint8_t file_type);
void search_sub_dir(int sub_dir_starting_block, int sub_dir_blcok_count, int blcoksize, void* address,void* entirefile,struct superblock_t* superblock,char *path[MAX_IN_COMMAND]);

int print_disklist(int fd,struct stat buffer, char *path[MAX_IN_COMMAND]){
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
    
    int starting_byte = (root_start_block);

    // printf("FAT starts: %d\n",htonl(superblock->fat_start_block));
    // printf("FAT blocks: %d\n",htonl(superblock->fat_block_count));

    void* entirefile = mmap(NULL, blockcount*blocksize, PROT_READ | PROT_WRITE, MAP_SHARED, fd,0);

    int fatstartblcok = htonl(superblock->fat_start_block);
    int counter = 0;

    struct dir_entry_t* dir_entry = (void*)((char*)entirefile + starting_byte*blocksize + 0);
    struct dir_entry_timedate_t create_time_struct = dir_entry->create_time;
    // printf("directory starting_block %d\n",htonl(superblock->root_dir_start_block));
    search_sub_dir(htonl(superblock->root_dir_start_block),htonl(superblock->root_dir_block_count),blocksize,address,entirefile,superblock,path);

    munmap(address,buffer.st_size);
    close(fd);
}

char get_file_type(uint8_t file_type) {
  if (file_type == 5) return 'D';
  else return 'F';
}

void search_sub_dir(int sub_dir_starting_block, int sub_dir_blcok_count, int blocksize, void* address,void* entirefile,struct superblock_t* superblock,char *path[MAX_IN_COMMAND]){
    // printf("sub_dir_starting_block %d\n",sub_dir_starting_block);
    int starting_byte = (sub_dir_starting_block);
    int fatstartblcok = htonl(superblock->fat_start_block);
    int printed = 1; //Set printed to be FALSE
    int counter = 0;
    struct dir_entry_t* file_name;
    for(int i = 0; i < sub_dir_blcok_count*(blocksize/64); i++){
        counter++;

        struct dir_entry_t* file_name = (void*)((char*)entirefile + starting_byte*blocksize + (i%8)*64);
        //Store each file info in the struct for accesss
        //(i%8)*64 it use to Zero the Jump after each block
        struct dir_entry_timedate_t create_time_struct = file_name->create_time;

        if(path[pathcounter] != NULL){
            if(!strcmp(file_name->filename,path[pathcounter]) && (path[pathcounter+1] == NULL)){
                // printf("Find the Target: %s\n",dir_entry->filename);
                // printf("Starting Block %d \n", htonl(dir_entry->starting_block));
                // printf("Size %d \n", htonl(dir_entry->size));
                // printf("NULL input 11\n");
                pathcounter++;
                search_sub_dir(htonl(file_name->starting_block),htonl(file_name->block_count),blocksize,address,entirefile,superblock,path);
            }else if (!strcmp(file_name->filename,path[pathcounter]) && (path[pathcounter+1] != NULL))
            {
                // printf("NULL input 22\n");
                pathcounter++; 
                search_sub_dir(htonl(file_name->starting_block),htonl(file_name->block_count),blocksize,address,entirefile,superblock,path);
                
            }    
        }else if((path[pathcounter] == NULL) && (end == 1)){
            // printf("NULL input \n");
            if(htonl(file_name->size) != 0){
                printf("%c %10d %30s %04d/%02d/%02d %02d:%02d:%02d\n",
                    get_file_type(file_name->status), htonl(file_name->size), file_name->filename,
                    htons(create_time_struct.year), create_time_struct.month, create_time_struct.day,
                    create_time_struct.hour, create_time_struct.minute, create_time_struct.second);
                printed = 0;
            }
        }              
        // If the path[pathcounter] is not empty the system will find the target for the user, if path[pathcounter] is empty and end is FALSE(0)
        // it will print out the root directory. If the Directory Not Exist it will print out the error message.

        if(counter%8 == 0){
            memcpy(&starting_byte,address+(fatstartblcok*blocksize)+4*starting_byte,4);
            //  4*starting_byte Relocated the starting_byte to next FAT location
            starting_byte=htonl(starting_byte);
            // printf("starting_byte%d\n",(int)starting_byte);
        }
        // Change the directory to the FAT location
    }
    if(end == 1 && printed == 1){
        printf("Directory Not Found\n");
    }
    end = 0;
    // Set end sign to be True(0)
}

int main(int argc, char* argv[]) 
{
    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    // Reference: https://stackoverflow.com/questions/56109844/what-is-the-different-between-struct-stat-buffer-and-buffer-in-linux-stat-fun

    char *path[MAX_IN_COMMAND];
    for(int counter = 0;counter < MAX_IN_COMMAND;counter++){
        path[counter] = NULL;
    }
    // Initialze the array
    if((argv[2] != NULL) && (argv[2][0] == '/')){
        char input[MAX_IN_COMMAND];
        strcpy(input,argv[2]);
        // Copy the src into input for strtok
        char *token;
        token = strtok(input,"/");
        path[0] = token;
        // get the first token and store into the array 
        int counter = 1;
        while( token != NULL ) {
            token = strtok(NULL,"/");
            if(token == NULL)
                break;
            path[counter] = token;
            counter++;
        }
        // walk through other tokens and store into array
    }
    // printf("%s,%s\n",path[0],path[1]);

    if((argv[2] != NULL) && (argv[2][0] == '/')){
        print_disklist(fd, buffer,path);

    }else{
        if(argv[2] == NULL){
            print_disklist(fd, buffer,path);  
        }else{
            printf("Wrong Input\n");
        }
        
    } 
    // If the argv[2] is not empty the system will find the targe for the user, 
    // if argv[2] is empty it will print out the root directory.
}