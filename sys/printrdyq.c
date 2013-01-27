#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <q.h>
/*------------------------------------------------------------------------
 * void printrdyq()
 *     o Print out each process in the ready q
 *------------------------------------------------------------------------
 */
void printrdyq() {

    struct pentry *pptr;

	int item;

        item = rdyhead;
        item = q[rdyhead].qnext;
        while (item != rdytail) {
            pptr = &proctab[item];
	    kprintf("Process %d [%s] is ready with quantum %d\n", 
                       item, pptr->pname, pptr->counter);
            item = q[item].qnext;
        }
}
