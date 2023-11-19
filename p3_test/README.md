<p align="center">
  <img alt="oracle" src="https://user-images.githubusercontent.com/16131737/38450102-528f8cbe-39cd-11e8-8352-bb89403adec6.png" height="60%" width="60%" />
</p>
<p align="center">
  <img alt="mario" src="https://user-images.githubusercontent.com/16131737/37385328-c42681cc-2710-11e8-8213-13ce1aa3aafd.gif" />
</p>

## Overview
This project was an assignment for the [Operating Systems](https://github.com/williamgrosset/oracle/blob/master/csc360_p3.pdf) class at the University of Victoria. The programs `diskinfo` and `disklist` will return information regarding the file system superblock, FAT, and directory content, with `diskget` allowing users to copy a file from the file system root directory to the current directory.

## Usage
### Compile
```
make
```

### Clean build
```
make clean
```

### Run `diskinfo`
Display information about the file system super block and FAT.
```bash
./diskinfo.o test.img
```
#### Sample output
```bash
Super block information:
Block size: 512
Block count: 6400
FAT starts: 2
FAT blocks: 50
Root directory start: 53
Root directory blocks: 8

FAT information:
Free Blocks: 6192
Reserved Blocks: 50
Allocated Blocks: 158
```

### Run `disklist`
Display content for the file system root directory.
```bash
./disklist.o test.img /
```
#### Sample output
```bash
F        735                      mkfile.cc 2005/11/15 12:00:00
F       2560                        foo.txt 2005/11/15 12:00:00
F       3940                    disk.img.gz 2009/08/04 21:11:13
```

### Run `diskget`
Copy a file from the file system root directory to the current directory. If the specified file is not found in the file system, an error message will be displayed.
```bash
./diskget.o test.img mkfile.cc foo2.txt
```
