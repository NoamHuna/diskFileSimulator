# Disk File System - Simulator

## Description
This C++ program implements a simple file system shield that operates on a simulated disk. It provides basic file management operations like creating, opening, reading, writing, and deleting files. This README gives an overview of the key components and functionalities of the program.

## Components
### fsInode Class
This class represents an Inode, which is a data structure that stores information about a file or directory. It contains details like file size, block allocation, and block size.
### FileDescriptor Class
The FileDescriptor class manages file descriptors associated with open files. It contains information about the file, including the file name, Inode, and whether it's currently in use.
### fsDisk Class
The fsDisk class simulates a disk and manages the filesystem. It includes features such as formatting the disk, creating/opening/closing files, reading/writing data to files, and listing all open files. It also maintains a directory structure using a map.

The program provides a CLI for interacting with the file system. Use the following commands:
1. List all open files.
2. Format the disk (provide block size and direct entries).
3. Create a file (provide a file name).
4. Open a file (provide a file name).
5. Close a file (provide a file descriptor).
6. Write to a file (provide a file descriptor and data).
7. Read from a file (provide a file descriptor and size to read).
8. Delete a file (provide a file name).
9. Copy a file (provide source and destination file names).
10. Rename a file (provide old and new file names).
0. Exit the program.

The program simulates a disk using a text file (DISK_SIM_FILE.txt) as storage.
The file system supports direct and indirect block allocations.
Each file has an associated Inode that stores file-specific information.
The program manages file descriptors for open files.

## Program Database
- DISK_SIM_FILE.txt: This file serves as a simulated disk, where the file system data is stored. It's used for reading and writing data to simulate disk operations.

## Program Files
- Stub_code.cpp: This is the main source code file that contains the implementation of the file system shield, including classes and main program logic.

## How to Compile?
- Compile: `g++ -o run stub_code.cpp`
- Run: `./run`

## Input
The program accepts user commands through the command-line interface (CLI). Users provide numeric commands and parameters for file system operations.

## Output
The program provides text-based output to display the results of user commands and the state of the file system. Output includes information about open files, formatting, file creation, opening, closing, reading, writing, file deletion, copying, and renaming.
