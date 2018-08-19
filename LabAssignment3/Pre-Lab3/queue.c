#include "type.h"


int enqueue(PROC **queue, PROC *p)
{
   // enter p into queue by priority; FIFO if same prioirty
	PROC *qp = *queue; //first in queue

	if(!qp) //empty
	{
		p->next = qp;
		*queue = p; 
		return 1;
	}
	if(p->priority > qp->priority) //vip first to next one
	{
		p->next = qp;
		*queue = p;
		return 1;
	}

	while(qp->next && p->priority <= (qp->next)->priority) //traverse through based on priority
	{
		qp = qp->next;
	}
	
	p->next = qp->next;
	qp->next = p;
	return 1;
}








PROC *dequeue(PROC **queue)
{
   // remove and return first PROC from queue
	PROC *pp = *queue;
 
	if(!pp) //empty
	{
	   	return 0;
 	}

	*queue = (*queue)->next; //skip over
	return pp; 
}





int printList(char *name, PROC *p)
{
	printf("%s = ", name);
	// print list elements as [pid priority] -> ....
	
	while(p)
  	{
    		printf("[%d %d] -> ", p->pid, p->priority);
    		p = p->next;
  	}
	printf("END\n");
}






/*
void showChild()
{
	PROC *p = running->child;
	printf("%d children -> ", running->pid);

	while(p)
	{
		printf("%d, ", p->pid);
		p = p->sibling;
	}
	printf("END\n");
}
*/




