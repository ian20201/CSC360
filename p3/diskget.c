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

int pathcounter = 0;
int end = 1; //Set End to be FALSE

int initialize_file(int fd,char* newfile,struct stat buffer, char *path[MAX_IN_COMMAND]);
char get_file_type(uint8_t file_type);
void search_sub_dir(int sub_dir_starting_block, int sub_dir_blcok_count, int blocksize, void* address,void* entirefile,struct superblock_t* superblock,char *path[MAX_IN_COMMAND],char* newfile);
int copy_file(int sub_dir_starting_block, int sub_dir_blcok_count, int blocksize, void* address,void* entirefile,struct superblock_t* superblock,char* newfile,int file_size);

int initialize_file(int fd,char* newfile,struct stat buffer, char *path[MAX_IN_COMMAND]){
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

    void* entirefile = mmap(NULL, blockcount*blocksize, PROT_READ | PROT_WRITE, MAP_SHARED, fd,0);

    int fatstartblcok = htonl(superblock->fat_start_block);
    int counter = 0;

    struct dir_entry_t* dir_entry = (void*)((char*)entirefile + starting_byte*blocksize + 0);
    struct dir_entry_timedate_t create_time_struct = dir_entry->create_time;

    search_sub_dir(htonl(superblock->root_dir_start_block),htonl(superblock->root_dir_block_count),blocksize,address,entirefile,superblock,path,newfile);


    munmap(address,buffer.st_size);
    close(fd);
}

char get_file_type(uint8_t file_type) {
  if (file_type == 5) return 'D';
  else return 'F';
}

void search_sub_dir(int sub_dir_starting_block, int sub_dir_blcok_count, int blocksize, void* address,void* entirefile,struct superblock_t* superblock,char *path[MAX_IN_COMMAND],char* newfile){
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
        int file_size = htonl(file_name->size);

        if(path[pathcounter] != NULL){
            if(!strcmp(file_name->filename,path[pathcounter]) && (path[pathcounter+1] == NULL)){
                pathcounter++;
                if(get_file_type(file_name->status) == 'F'){
                    copy_file(htonl(file_name->starting_block),htonl(file_name->block_count),blocksize,address,entirefile,superblock,newfile,file_size);
                    printed = 0;
                    printf("Copy File Success\n");
                }else{
                    printf("This Is Not The File\n");
                }     
            }else if (!strcmp(file_name->filename,path[pathcounter]) && (path[pathcounter+1] != NULL))
            {
                // printf("NULL input \n");
                pathcounter++; 
                search_sub_dir(htonl(file_name->starting_block),htonl(file_name->block_count),blocksize,address,entirefile,superblock,path,newfile);
                
            }    
        }           
        // If the path[pathcounter] is not empty the system will find the target for the user, if path[pathcounter] is empty and end is FALSE(0)
        // it will save the file in to the target file.If the File Not Exist it will print out the error message.

        if(counter%8 == 0){
            memcpy(&starting_byte,address+(fatstartblcok*blocksize)+4*starting_byte,4);
            //  4*starting_byte Relocated the starting_byte to next FAT location
            starting_byte=htonl(starting_byte);
        }
        // Change the directory to the FAT location
    }
    if(end == 1 && printed == 1){
        printf("File not found.\n");
    }
    end = 0;
    // Set end sign to be True(0)
}

int copy_file(int sub_dir_starting_block, int sub_dir_blcok_count, int blocksize, void* address,void* entirefile,struct superblock_t* superblock,char* newfile,int file_size){
    int fatstartblock = htonl(superblock->fat_start_block);

    int targetfile = open(newfile, O_TRUNC | O_CREAT | O_WRONLY | O_RDONLY | O_APPEND, 777);
    char buffer;
        
    while(sub_dir_starting_block!= 0xFFFFFFFF){
        if(file_size>blocksize){
            for(int i = 0; i < blocksize; i++){
                memcpy(&buffer,entirefile+(blocksize* sub_dir_starting_block)+i, 1);
                write(targetfile,&buffer, 1);
            }	
                file_size = file_size - blocksize;
        }else{
            for(int i = 0; i < file_size; i++){
                memcpy(&buffer,entirefile+(blocksize*sub_dir_starting_block)+i, 1);
                write(targetfile,&buffer, 1);
            }
            break;	
        }
        memcpy(&sub_dir_starting_block,entirefile+(fatstartblock*blocksize)+4*sub_dir_starting_block,4);
        sub_dir_starting_block=htonl(sub_dir_starting_block);
    }
    // while sub_dir_starting_block!= 0xFFFFFFFF(-1 if the type is int32_t), it will copt the data byte by byte into the
    // target file, and change the sub_dir_starting_block to next FAT location until it reach -1.
    end = 0;
    // Set end sign to be True(0)
}

int main(int argc, char* argv[]) 
{
    if (argc != 4) {
        printf("Enter the correct arguments: ./diskget <file system image> <file 1> <file 2>.\n");
        return(EXIT_FAILURE);
    }
    // If argc != 4 it will print error message.
    
    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        printf("Failed to open disk image.\n");
        close(fd);
        return(EXIT_FAILURE);
    }
    // If it can't open the disk image it will print error message.

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

    if((argv[2] != NULL) && (argv[2][0] == '/')){
        initialize_file(fd,argv[3],buffer,path);

    }
    // If the argv[2] is not empty the system will find the targe for the user, 
}