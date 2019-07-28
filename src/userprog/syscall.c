#include "userprog/syscall.h"
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
//void close_file(int fd);
static void syscall_handler (struct intr_frame *);

extern bool running;




struct file_pointer{
	struct file* fname;
	int fcount;
	struct list_elem elem;
};

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
		thread_current()->exit_status = *(p+1);
        thread_exit();
		break;

        case SYS_CREATE:
		if(*(p+1)!=NULL){
		f->eax = filesys_create(*(p+1),*(p+2));
		}
		else{
			thread_current()->exit_status = -1;
            thread_exit();
		}
		
		break;
        

		case SYS_FILESIZE:
		if(*(p+1)!=NULL){
        struct file_pointer *fn = get_opened_file(*(p+1));
		f->eax = file_length(fn->fname);
		}
		break;

		case SYS_EXEC:
		f->eax = process_execute(*(p+1));
		/*if(*(p+1)!=NULL){
        pid_t *p = process_execute(*(p+1));
		if(p==TID_ERROR){
			f->eax=-1;
		}
		else{
			f->eax=p;
		}
		}*/
		break;

		case SYS_WRITE:
		   if(*(p+1)==1){
			putbuf(*(p+2),*(p+3));
		   }
		   else if(*(p+1)==0){
			   f->eax=-1;
		   }
		   else{
			   struct file_pointer *fn = get_opened_file(*(p+1));
		       f->eax = file_write(fn->fname,*(p+2),*(p+3));
		   }

		break;


		case SYS_READ:
       if(*(p+1)==0){
		   read_file(*(p+2),*(p+3));
		   f->eax = *(p+3);
	   }
	   else{
		//printf("%ddddddddddddddddddd",*(p+1));
		//printf("%ddddddddddddddddddd",*(p+1));
		struct file_pointer *fn = get_opened_file(*(p+1));
		if(fn==NULL){
			f->eax=-1;
		}
		else{
		//printf("%ddddddddddddddddddd",*(p+1));
		f->eax = file_read(fn->fname,*(p+2),*(p+3));
		}
		}
        
		break; 

        case SYS_OPEN:
		if(*(p+1)==NULL){
        return -1;
        }
		struct file *fx = filesys_open(*(p+1));
		if(fx==NULL){
			f->eax=-1;
		}
		else{
		struct file_pointer *fpr = malloc(sizeof(*fpr));
	    fpr->fname = fx;
        fpr->fcount = thread_current()->file_count;
        thread_current()->file_count++;
        list_push_back(&thread_current()->opened_files, &fpr->elem);
        f->eax = fpr->fcount;
		}
		break;

		case SYS_CLOSE:
        close_file(*(p+1));
		break;

		default:
		printf("none");
		
	}
   
	
	}
	void close_file(int fd){
	    struct list_elem *e ;
        struct thread *t = thread_current();
		
        for (e = list_begin (&t->opened_files); e != list_end (&t->opened_files); e = list_next (e)){
        struct file_pointer *fp= list_entry(e, struct file_pointer, elem);
        if(fp->fcount == fd){
		
        file_close (fp->fname);
		//printf("%dfiiii",fd);
        list_remove(e);
        free(fp);
        break;
        }
        }
	}

	int read_file(void *buffer,int size){
        uint8_t * sbuffer = (uint8_t*)buffer;
		for(unsigned i=0;i<size;i++){
			sbuffer[i] = input_getc();	
		}
		return size;
	}

	struct file_pointer* get_opened_file(int fd){
	    struct list_elem *e ;
        struct thread *t = thread_current();
		
        for (e = list_begin (&t->opened_files); e != list_end (&t->opened_files); e = list_next (e)){
        struct file_pointer *fp= list_entry(e, struct file_pointer, elem);
        if(fp->fcount == fd){
		
		return fp;
        
        //free(fp);
        //break;
        }
        }
		return NULL;
	}
