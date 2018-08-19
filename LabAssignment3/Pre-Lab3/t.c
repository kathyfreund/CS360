/*********** A Multitasking System ************/
#include "type.h"


PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;

int ksleep(int event)
{
  printf("proc %d sleep on %d: ", running->pid, event);
  running->event = event;         // sleep event value
  running->status = SLEEP;        // status = SLEEP
  enqueue(&sleepList, running);   // enter into sleepList
  //printsleepList();
  tswitch();                      // switch process to give up CPU 
}






//****************************TO DO - KWAKEUP
int kwakeup(int event)
{
	PROC *p, *q = 0;
	
	while(p = dequeue(&sleepList)) //go through sleepList/ take it out
	{
		if(p->event == event) //if in this event, WAKE UP
		{
			p->status = READY;
			enqueue(&readyQueue, p);
			printf("Waking up %d\n", p->pid);
			continue;
		}
		enqueue(&q, p); //not in event
	}
	sleepList = q; //put back all PROCS that were not woken up in this event
}


int do_switch()
{
	printf("proc %d switching task\n", running->pid);
 	tswitch();
   	printf("proc %d resuming\n", running->pid);
}

int do_kfork()
{
   	int child = kfork();
   	if (child < 0)
	{
      		printf("kfork failed\n");
	}
   	else
	{
      		printf("proc %d kforked a child = %d\n", running->pid, child); 
      		printList("readyQueue", readyQueue);
   	}
   	return child;
}




//*********************************TO DO - KEXIT
int kexit()
{
	int i;
  	PROC *p;
  	printf("proc %d in kexit()\n", running->pid);
  	if (running->pid==1)
	{
	      printf("P1 never dies\n");
	      return 0;
	}


	running->exitCode = running->pid; //record pid as exitStatus
	running->status = ZOMBIE; //become a ZOMBIE

	for(i = 1; i < NPROC; i++) //send children
	{
		p = &proc[i];
		if (p->status != FREE && p->ppid == running->pid)
		{
			p->ppid = 1;
			p->parent = &proc[1];
		}
	}

	kwakeup(running->parent); //wakeup!
	kwakeup(&proc[1]);
   	tswitch();
}

int do_exit()
{
	kexit();    // journey of no return 
}





//*********************************TO DO - KWAIT

int kwait(int *status)
{
	PROC *p;
	int i, n, f = 0;
	n = NPROC;
 	while(1)
	{
		for (i = 0; i < n; i++)
		{
			p = &proc[i];
			if (p->ppid == running->pid && p->status != FREE)
			{
				f = 1;
				if (p->status == ZOMBIE) //if we found a zombie child
				{
					//remove child from list
        				*status = p->exitCode;
					enqueue(freeList, p); //add to freeList
					p->status = FREE; 
					printList("freeList", freeList);
        				return (p->pid);
     				}
			}
		}
		if(!f)
		{		
			return(-1);
		}	
     		ksleep(running->pid); 
  	}
}

int do_wait()
{
  int pid, status;
  pid = kwait(&status);
  printf("proc %d waited for a ZOMBIE child %d status=%d\n", 
         running->pid, pid, status);
}


int body()
{
  int c, CR;
  printf("proc %d resume to body()\n", running->pid);
  while(1){
    printf("***************************************\n");
    printf("proc %d running: Parent=%d  ", running->pid, running->ppid);
    //showChild(); // show child list of running PROC
    printf("enter a key [f|s|q|w] : "); // ADD 'w' command
    c = getchar(); CR=getchar();
     
    switch(c){
      case 'f': do_kfork();     break;
      case 's': do_switch();    break;
      case 'q': do_exit();      break;
      case 'w': do_wait();      break;     // implement wait() operation
      default :                 break;  
    }
  }
}
    



/*******************************************************
  kfork() creates a child porc; returns child pid.
  When scheduled to run, child PROC resumes to body();
********************************************************/
int kfork()
{
  PROC *p;
  int  i;
  /*** get a proc from freeList for child proc: ***/
  p = dequeue(&freeList);
  if (!p){
     printf("no more proc\n");
     return(-1);
  }

  /* initialize the new proc and its stack */
  p->status = READY;
  p->priority = 1;         // for ALL PROCs except P0
  p->ppid = running->pid;

  //                    -1   -2  -3  -4  -5  -6  -7  -8   -9
  // kstack contains: |retPC|eax|ebx|ecx|edx|ebp|esi|edi|eflag|
  for (i=1; i<10; i++)
    p->kstack[SSIZE - i] = 0;

  p->kstack[SSIZE-1] = (int)body;
  p->saved_sp = &(p->kstack[SSIZE - 9]); 

  enqueue(&readyQueue, p);
  return p->pid;
}



int init()
{
  int i; 
  PROC *p;
  for (i = 0; i < NPROC; i++){
    p = &proc[i];
    p->pid = i; 
    p->status = FREE;
    p->priority = 0;
    p->next = p + 1;
  }
  proc[NPROC-1].next = 0;
 
  freeList = &proc[0];        
  readyQueue = 0;
  sleepList = 0;

  // create P0 as the initial running process
  p = running = dequeue(&freeList);
  p->status = READY;
  p->priority = 0;
  p->child = 0;
  p->sibling = 0;
  p->parent = p;
  printList("freeList", freeList);
  printf("init complete: P0 running\n"); 
}

/*************** main() ***************/

/*
main()
{
   printf("\nWelcome to 360 Multitasking System\n");
   init();
   kfork();  
   printf("P0: switch task\n");
     tswitch();
   printf("All dead. Happy ending\n");
}
*/


/*********** scheduler *************/
int scheduler()
{ 
  printf("proc %d in scheduler()\n", running->pid);
  if (running->status == READY)
      enqueue(&readyQueue, running);
  printList("readyQueue", readyQueue);
  running = dequeue(&readyQueue);
  printf("next running = %d\n", running->pid);  
}



