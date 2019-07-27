#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void close_file (int fd);
int read_file(void *buffer,int size);
struct file_pointer* get_opened_file(int fd);
#endif /* userprog/syscall.h */
