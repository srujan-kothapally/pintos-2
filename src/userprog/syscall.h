#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
//#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
void syscall_init (void);
void close_file (int fd);
int read_file(void *buffer,int size);
struct file_pointer* get_opened_file(int fd);
struct file_pointer{
	struct file* fname;
	int fcount;
	struct list_elem elem;
};
#endif /* userprog/syscall.h */
