#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "process.h"

static void syscall_handler (struct intr_frame *);

extern bool running;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int * p = f->esp;


  int system_call = * p;
    
	switch (system_call)
	{
		case SYS_HALT:
		shutdown_power_off();
		break;

		case SYS_EXIT:
		thread_current()->parent->ex = true;
		thread_exit();
		break;

		case SYS_WRITE:
		//printf("%s","entryto syscall");
		//if(*(p+5)==1)
		//{
                       // *p +=1;
       
			putbuf(*(p+2),*(p+3));
		//}
		break;
        case SYS_CREATE:
		//printf("%s","entryto syscall");
		//if(*(p+5)==1)
		//{
		   //      filesys_create(
		//}
		break;
                
        case SYS_OPEN:
        //char *name = (char)*(p+1);
		//printf("%shiiiiii",*(p+1));
		if(*(p+1)==NULL){
        return -1;
        }
		struct file *fx = filesys_open(*(p+1));
		if(fx==NULL){
			f->eax=-1;
		}
		
	
		break;

		case SYS_CLOSE:
		//printf("%dhiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",*(p+1));
		file_close (*(p+1)); 
		
		break;

		default:
		printf("No match\n");
	}
}
