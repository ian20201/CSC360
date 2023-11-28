==================
CSC 360 - Operating Systems
Assignment 3 - File System Operations
Author: Yu-Lun Chen (V00887293)
Acknowledgments: This assignment was completed for CSC 360 - Operating Systems, taught by Dr. Jianping Pan in Fall 2023 at the University of Victoria.
==================

File 1: diskinfo.c

This code extracts information from the superblock of the provided image file using a struct from Tutorial 9. The struct mirrors the superblock's structure, allowing direct access to its contents. The program outputs details such as block size, block count, and FAT information.

Example Output:
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

How to Run:
./diskinfo subdirs.img

==================
File 2: disklist.c

This code lists the contents of the root/subdirectory of the file system. Information is read from the image file, and output is generated using a struct. The program provides details such as file type, size, and modification date.

Example Output:
./disklist subdirs.img
F  735 mkfile.cc   2005/11/15 12:00:00
D 2048 subdir1     2005/11/15 12:00:00
F 3940 disk.img.gz 2009/08/04 21:11:13

./disklist subdirs.img /subdir1
D 2048 subdir2     2005/11/15 12:00:00

./disklist subdirs.img /subdir1/subdir2
F 2560 foo.txt     2005/11/15 12:00:00

How to Run:
./disklist subdirs.img
./disklist subdirs.img /subdir1
./disklist subdirs.img /subdir1/subdir2

==================
File 3: diskget.c

This code scans the root or sub-directory section within the image file to find a specific file. If found, it navigates the File Allocation Table (FAT) to locate the blocks containing the file's data, which is then written to a local file. If the file is not found, it prints 'FILE NOT FOUND', and if successful, it prints 'Copy File Success'.

How to Run:
./diskget subdirs.img /subdir1/subdir2/foo.txt foo.txt
./diskput test.img /subdir1/test.txt test2.txt

==================
File 4: diskput.c

This code reads a local file byte by byte and stores it in the root or sub-directory section of the file system. It checks for the file's existence, and if not found, allocates blocks from the FAT section and stores the data. Note that there is no 'overwrite' feature.

How to Run:
./diskput subdirs.img foo.txt /subdir1/subdir2/foo2.txt

Additional Information:
- To compile all codes, execute the makefile by typing 'make' in the terminal.
- The code follows the gnu99 standard for variable declarations within for loops.
- The -lm flag (for math.h) is included but remains unused in the final project, retained for troubleshooting purposes.
- The diskstructs.c and diskstructs.h files contain the necessary structures for the assignments.
