/*********** A Multitasking System ************/
#include "type.h"


PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;

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
      printf("kfork failed\n");
   else{
      printf("proc %d kforked a child = %d\n", running->pid, child); 
      printList("readyQueue", readyQueue);
   }
   return child;
}

char *gasp[NPROC]={
  "Oh! I'm dying ....", 
  "Oh! You're killing me ....",
  "Oh! I'm a goner ....", 
  "Oh! I'm mortally wounded ....",
};

int kexit(){
 
   printf("*************************************\n"); 
   printf("proc %d: %s\n", running->pid, gasp[running->pid % 4]);
   printf("*************************************\n");
   running->status = FREE;
   running->priority = 0;

// ASSIGNMENT 3: add YOUR CODE to delete running PROC from parent's child list

   enqueue(&freeList, running);     // enter running into freeList
   printList("freeList", freeList); // show freeList

   tswitch();
}

int do_exit()
{
  if (running->pid==1){
    printf("P1 never dies\n");
    return -1;
  }
  kexit();    // journey of no return 
}

int body()
{
  int c, CR;
  printf("proc %d starts from body()\n", running->pid);
  while(1){
    printf("***************************************\n");
    printf("proc %d running: Parent = %d\n", running->pid, running->ppid);

 // ASSIGNMENT 3: add YOUR CODE to show child list
/*
	printList("freelist ", freeList);
	printList("readyQueue", readyQueue);
	printList("sleepList ", sleepList);
*/

    printf("input a char [ f-fork | s-switch | q-quit | x-exit ] : ");
    c = getchar(); CR=getchar(); 
    switch(c){
      case 'f': do_kfork();     break;
      case 's': do_switch();    break;
      case 'q': do_exit();      break;
	case 'x': 		return;
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
  
/**************** ASSIGNMENT 3  ********************
  add YOUR code to implement the PROC tree as a BINARY tree
  enter_child(running, p);
****************************************************/


  enqueue(&readyQueue, p);
  return p->pid;
}

int init()
{
  int i;
  for (i = 0; i < NPROC; i++){
    proc[i].pid = i; 
    proc[i].status = FREE;
    proc[i].priority = 0;
    proc[i].next = (PROC *)&proc[(i+1)];
  }
  proc[NPROC-1].next = 0;
 
  freeList = &proc[0];        
  readyQueue = 0;
  sleepList = 0;

  // create P0 as the initial running process
  running = dequeue(&freeList);
  running->status = READY;
  running->priority = 0;  

  running->child = 0;  
  running->sibling = 0;  
  running->parent = running;

  printf("init complete: P0 running\n"); 
  printList("freeList", freeList);
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

int main()
{
  int i;
  PROC *p;
  readyQueue = 0;

  for (i=0; i < NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->priority = rand() % 10;
    printf("pid=%d priority=%d\n", p->pid, p->priority);
    enqueue(&readyQueue, p);
    printList("readyQ", readyQueue);
  }
}

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



