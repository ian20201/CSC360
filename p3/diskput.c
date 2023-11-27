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

int write_file(char* entirefile, char* p, int file_size, int* FATaddr, int* FATval, struct superblock_t* superblock);
void write_dir_info(char* entirefile, int where, char status, int start_block, int number_block, int file_size, char* file_name, struct dir_entry_timedate_t *timeb);
void write_dir(char* entirefile, int now_block, int block_count, char *path[MAX_IN_COMMAND], int cur, int next_fat_pos, int file_start_block, struct stat *filedata, struct superblock_t* superblock);

int calc(char* entirefile, int where, int len) {
    int res;
    memcpy(&res, entirefile + where, len);
    return htonl(res);
}

int write_file(char* entirefile, char* p, int file_size, int* FATaddr, int* FATval, struct superblock_t* superblock) {
    int i;
    int fatstartblock = htonl(superblock->fat_start_block);
    int blocksize = htons(superblock->block_size);
    int j = 0;
    for (i = 0; i < file_size / blocksize; i++) {
        while (calc(entirefile, fatstartblock * blocksize + 4 * j, 4) != 0) {
            j += 1;
        }
        FATaddr[i] = fatstartblock * blocksize + 4 * j;
        FATval[i] = htonl(j);
        memcpy(entirefile + j * blocksize, p + i * blocksize, blocksize);
        j += 1;
    }
    if (file_size % blocksize != 0) {
        while (calc(entirefile, fatstartblock * blocksize + 4 * j, 4) != 0) {
            j += 1;
        }
        memcpy(entirefile + j * blocksize, p + i * blocksize, file_size % blocksize);
        FATaddr[i] = fatstartblock * blocksize + 4 * j;
        FATval[i++] = htonl(j);
        j += 1;
    }
    int k;
    for (k = 0; k < i; k++) {
        FATval[k] = FATval[k + 1];
    }
    FATval[i - 1] = htonl(-1);
    return j;
}

void write_dir_info(char* entirefile, int where, char status, int start_block, int number_block, int file_size, char* file_name, struct dir_entry_timedate_t *timeb) {
    getCurrentTime(timeb);
    short year = htons(timeb->year);
    memcpy(entirefile + where , &status, 1);
    memcpy(entirefile + where + 1, &start_block, 4);
    memcpy(entirefile + where + 5, &number_block, 4);
    memcpy(entirefile + where + 9, &file_size, 4);
    memcpy(entirefile + where + 13, &year, 2);
    memcpy(entirefile + where + 15, &(timeb->month), 1);
    memcpy(entirefile + where + 16, &(timeb->day), 1);
    memcpy(entirefile + where + 17, &(timeb->hour), 1);
    memcpy(entirefile + where + 18, &(timeb->minute), 1);
    memcpy(entirefile + where + 19, &(timeb->second), 1);
    memcpy(entirefile + where + 20, &year, 2);
    memcpy(entirefile + where + 22, &(timeb->month), 1);
    memcpy(entirefile + where + 23, &(timeb->day), 1);
    memcpy(entirefile + where + 24, &(timeb->hour), 1);
    memcpy(entirefile + where + 25, &(timeb->minute), 1);
    memcpy(entirefile + where + 26, &(timeb->second), 1);
    memcpy(entirefile + where + 27, file_name, strlen(file_name)+1);
}

void write_dir(char* entirefile, int now_block, int block_count, char *path[MAX_IN_COMMAND], int cur, int next_fat_pos, int file_start_block, struct stat *filedata, struct superblock_t* superblock) {
    if (path[cur] == NULL) {
        return;
    }
    // printf("%d %d %d\n", now_block, block_count, cur);
    int pre_block = now_block;
    int i;
    int fatstartblock = htonl(superblock->fat_start_block);
    int blocksize = htons(superblock->block_size);
    for (i = 0; i < block_count * (blocksize / 64); i++) {
        if (i != 0 && i % 8 == 0) {
            now_block = calc(entirefile, fatstartblock * blocksize + 4 * now_block, 4);
        }
        struct dir_entry_t* file_info = (void*)(entirefile + now_block*blocksize + (i%8)*64);
        char file_status = file_info->status;
        int sub_dir_starting_block = htonl(file_info->starting_block);
        int sub_dir_block_count = htonl(file_info->block_count);
        if (file_status == 5 && strcmp(path[cur], file_info->filename) == 0) {
            write_dir(entirefile, sub_dir_starting_block, sub_dir_block_count, path, cur + 1, next_fat_pos, file_start_block, filedata, superblock);
            return;
        }
        if (path[cur + 1] == NULL && file_status == 3 && strcmp(path[cur], file_info->filename) == 0) {
            printf("file {%s} exsited\n", path[cur]);
            exit(1);
        }
    }

    now_block = pre_block;
    while (calc(entirefile, fatstartblock * blocksize + 4 * next_fat_pos, 4) != 0) {
        next_fat_pos += 1;
    }
    int next_block = next_fat_pos;
    next_fat_pos += 1;
    int temp = 0xFFFFFFFF;
    memcpy(entirefile + fatstartblock * blocksize + 4 * next_block, &temp, 4);
    char status = (path[cur + 1] == NULL ? 3 : 5);
    int start_block = htonl(path[cur + 1] == NULL ? (file_start_block - fatstartblock * blocksize) / 4 : next_block);
    int number_block = htonl(path[cur + 1] == NULL ? ((int)filedata->st_size + blocksize - 1) / blocksize : 1);
    int file_size = htonl((path[cur + 1] == NULL ? (int)filedata->st_size : blocksize));
    struct dir_entry_timedate_t *timeb = try_malloc(sizeof(struct dir_entry_timedate_t));

    for (i = 0; i < block_count * (blocksize / 64); i++) {
        if (i != 0 && i % 8 == 0) {
            now_block = calc(entirefile, fatstartblock * blocksize + 4 * now_block, 4);
        }
        struct dir_entry_t* file_info = (void*)(entirefile + now_block * blocksize + (i % 8) * 64);
        char file_status = file_info->status;
        if (file_status == 0) {
            write_dir_info(entirefile, now_block * blocksize + (i % 8) * 64, status, start_block, number_block, file_size, path[cur], timeb);
            write_dir(entirefile, htonl(start_block), htonl(number_block), path, cur + 1, next_fat_pos, file_start_block, filedata, superblock);
            return;
        }
    }    

    while (calc(entirefile, fatstartblock * blocksize + 4 * next_fat_pos, 4) != 0) {
        next_fat_pos += 1;
    }
    next_block = next_fat_pos;
    next_fat_pos += 1;
    temp = htonl(next_block);
    memcpy(entirefile + fatstartblock * blocksize + 4 * now_block, &temp, 4);
    temp = 0xFFFFFFFF;
    memcpy(entirefile + fatstartblock * blocksize + 4 * next_block, &temp, 4);
    temp = htonl(block_count + 1);
    memcpy(entirefile + pre_block * blocksize + 5, &temp, 4);
    memcpy(&temp, entirefile + pre_block * blocksize + 9, 4);
    temp = htonl(htonl(temp) + blocksize);
    memcpy(entirefile + pre_block * blocksize + 9, &temp, 4);
    now_block = next_block;

    write_dir_info(entirefile, now_block * blocksize, status, start_block, number_block, file_size, path[cur], timeb);
    write_dir(entirefile, htonl(start_block), htonl(number_block), path, cur + 1, next_fat_pos, file_start_block, filedata, superblock);
    
}

void put_file(char* file, char* entirefile, char *path[MAX_IN_COMMAND]) {
    struct superblock_t* superblock;
    superblock = (struct superblock_t*) entirefile;

    int ifd = open(file, O_RDWR);
    struct stat buffer;
    char* p = NULL;
    if (ifd >= 0) {
        fstat(ifd, &buffer);
        p = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, ifd, 0);
    } else {
        printf("File not found.\n");
        close(ifd);
        exit(1);
    }
    int blocksize = htons(superblock->block_size);
    int file_size = buffer.st_size;
    int file_blocks = file_size / blocksize + (file_size % blocksize == 0 ? 0 : 1);
    int FATaddr[file_blocks], FATval[file_blocks];
    int next_fat_pos = write_file(entirefile, p, file_size, FATaddr, FATval, superblock);
    int i;
    for(i = 0; i < file_blocks; i++){
        memcpy(entirefile + FATaddr[i], FATval + i, 4);
    }
    int root_block_count = htonl(superblock->root_dir_block_count);
	int root_start_block = htonl(superblock->root_dir_start_block);
    write_dir(entirefile, root_start_block, root_block_count, path, 0, next_fat_pos, FATaddr[0], &buffer, superblock);
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
    fstat(fd, &buffer);
    char* address = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // Get address in disk image file via memory map

    char *path[MAX_IN_COMMAND];
    for(int counter = 0;counter < MAX_IN_COMMAND;counter++){
        path[counter] = NULL;
    }
    // Initialze the array
    if((argv[3] != NULL) && (argv[3][0] == '/')){
        char input[MAX_IN_COMMAND];
        strcpy(input, argv[3]);
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
    // printf("argv[3]: %s\n",argv[3]);
    if((argv[3] != NULL) && (argv[3][0] == '/')){
        put_file(argv[2], address, path);
    }
    // If the argv[3] is not empty the system will find the targe for the user, 
    // and replace or create the file in the target directory
}
