Yu-Lun Chen
V00887293
CSC 360 Assignment 3

The project is divided into four separate code pieces. To compile all of them, simply execute the makefile by typing 'make' in the terminal. The code follows the gnu99 standard to enable variable declarations within for loops. Although the -lm flag (for math.h) is included in each file, it remains unused in the final project. It has been retained for troubleshooting purposes. Also, the diskstructs.c and diskstructs.h contains the needed structure for the assignments.

Acknowledgments: This assignment was completed for CSC 360 - Operating Systems, taught by Dr. Jianping Pan in Fall 2023 from the University of Victoria.

==================
File 1: diskinfo.c
==================
This code utilizes the provided struct from Tutorial 9 to extract information from the superblock. The struct is organized to mirror the superblock's structure, enabling direct access to its contents. 

--------------
Example Output
--------------
Super block information
Block size: 512
Block count: 6400
FAT starts: 2
FAT blocks: 50
Root directory starts: 53
Root directory blocks: 8

FAT information
Free blocks: 6184
Reserved blocks: 50
Allocated blocks: 166

--------------
How to run
--------------
./diskinfo subdirs.img

==================
File 2: disklist.c
==================

This code outputs the contents in the root/sub directory of the file system. The contents were read from the img file similar to part 1, but within the section containing root/sub directory files/folders info. Most of the output was printed out from struct. 

--------------
Example Output
--------------
./disklist subdirs.img
F        735                      mkfile.cc 2005/11/15 12:00:00
D       2048                        subdir1 2005/11/15 12:00:00
F       3940                    disk.img.gz 2009/08/04 21:11:13

./disklist subdirs.img /subdir1
D       2048                        subdir2 2005/11/15 12:00:00

./disklist subdirs.img /subdir1/subdir2
F       2560                        foo.txt 2005/11/15 12:00:00
--------------
How to run
--------------
./disklist subdirs.img
./disklist subdirs.img /subdir1
./disklist subdirs.img /subdir1/subdir2

==================
File 3: diskget.c
==================

The given code scans the root or sub-directory section within the image file to determine the existence of a specific file. If the file is found, the code proceeds to navigate the File Allocation Table (FAT) section to locate the blocks containing the file's data. Subsequently, the data is written to a file in the current working directory on the local machine. In case the file is not found, the console prints 'FILE NOT FOUND'. If it success, console prints 'Copy File Success'.

--------------
How to run
--------------
./diskget subdirs.img /subdir1/subdir2/foo.txt foo.txt
./diskput test.img /subdir1/test.txt test2.txt 

==================
File 4: diskput.c
==================


If the file is not already present in the file system, the code reads the file byte by byte and stores it within the file system. Initially, it checks for the file's existence by examining the root/sub-directory section. If the file is not found, the code allocates blocks from the FAT section and subsequently stores the data in these blocks. 
**It is important to note that there is no 'overwrite' feature. If the diskput command is used for a file that already exists in the file system with different content, no action will be taken, and the console will display 'file {%s} exists'.**

--------------
How to run
--------------
./diskput subdirs.img foo.txt /subdir1/subdir2/foo2.txt