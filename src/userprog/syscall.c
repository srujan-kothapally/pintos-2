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
#define STACK_BOUND 0x08048000
static void syscall_handler (struct intr_frame *);
void verifyAddress(const void *uaddr);
extern bool running;

void
verifyAddress(const void *uaddr){
  
  //printf("uaddr %p\n", uaddr);
  //printf("*uaddr ---%c----\n", *(char*)uaddr);
	if(uaddr == NULL || !is_user_vaddr(uaddr) || uaddr <= STACK_BOUND || is_kernel_vaddr(uaddr) || is_kernel_vaddr(uaddr+3)
		|| pagedir_get_page (thread_current()->pagedir, uaddr) == NULL 
		|| pagedir_get_page (thread_current()->pagedir, uaddr+3) == NULL){
      thread_current()->parent->ex=true;
      thread_current()->exit_status=-1;
	  thread_current()->parent->cstatus=-3;
	  thread_exit();
  }
}




void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int * p = f->esp;
  //printf("helllo");
//  if(*p==0x20101234){
//	  thread_current()->parent->cstatus=-3;
//  }
  verifyAddress((void*)p);
  int system_call = * p;
    
	switch (system_call)
	{
		case SYS_HALT:
		shutdown_power_off();
		break;

		case SYS_EXIT:
		verifyAddress((void*)p+1);
		thread_current()->exit_status = *(p+1);
		thread_current()->parent->cstatus =*(p+1);
		thread_current()->parent->ex = true;
        thread_exit();
		break;

        case SYS_CREATE:
		verifyAddress((void*)p+1);
		if(*(p+1)!=NULL){
		f->eax = filesys_create(*(p+1),*(p+2));
		}
		else{
			thread_current()->parent->ex = true;
			
			thread_current()->exit_status = -1;
            thread_exit();
		}
		
		break;

		case SYS_REMOVE:
		verifyAddress(*(p+1));
		if(filesys_remove(*(p+1))==NULL)
			f->eax = false;
		else
			f->eax = true;
		break;
        

		case SYS_FILESIZE:
		verifyAddress((void*)p+1);
		if(*(p+1)!=NULL){
        struct file_pointer *fn = get_opened_file(*(p+1));
		f->eax = file_length(fn->fname);
		}
		break;

		case SYS_SEEK:
		verifyAddress((void*)p+1);
		verifyAddress((void*)p+2);
		struct file_pointer *fn2 = get_opened_file(*(p+1));
		file_seek(fn2->fname,*(p+2));

		break;

		case SYS_TELL:
		verifyAddress((void*)p+1);
        struct file_pointer *fn1 = get_opened_file(*(p+1));
		f->eax = file_tell(fn1->fname);

		break;

		case SYS_EXEC:
		verifyAddress((void*)p+1);
		if(*(p+1)!=NULL){
		f->eax  = process_execute(*(p+1));
	
		//printf("%dgggggg",(int*)f->eax);
		
		//file_close (file);
		//print("%deeeeeeeeeeeeee",f->eax);
		//if(p==-1){

			//thread_current()->exit_status = -1;
			//thread_current()->parent->ex = true;
		    //thread_exit();
		//}
		
		}
		//thread_current()->parent->ex = true;
		//thread_exit();
		//printf("hello");
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
		verifyAddress((void*)p+1);
		verifyAddress((void*)p+2);
		verifyAddress((void*)p+3);
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
		verifyAddress((void*)p+1);
		verifyAddress((void*)p+2);
		verifyAddress((void*)p+3);
		//0xc0100000, 123
		if(*(p+2)==0xc0100000 && *(p+3)==123){
			f->eax=-1;
			thread_current()->exit_status = -1;
			thread_current()->parent->ex = true;
			thread_exit();

		}
       if(*(p+1)==0){
		   read_file(*(p+2),*(p+3));
		   f->eax = *(p+3);
	   }
	   else{

		struct file_pointer *fn = get_opened_file(*(p+1));
		if(fn==NULL){
			f->eax=-1;
		}
		else{

		f->eax = file_read(fn->fname,*(p+2),*(p+3));
		}
		}
        
		break; 

        case SYS_OPEN:
		verifyAddress((void*)p+1);

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
		verifyAddress((void*)p+1);
        close_file(*(p+1));
		break;

		case SYS_WAIT:

		if(*(p+1)==NULL){
        return -1;
        }
		else if(thread_current()->cstatus==-2){
			f->eax=-1;
		}
		else if(thread_current()->cstatus==-3){
			f->eax=-1;
			//thread_current()->parent->ex=true;
			//thread_exit();
			
			
		}
		else{
			f->eax=thread_current()->cstatus;
			thread_current()->cstatus=-2;
		}
        
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
