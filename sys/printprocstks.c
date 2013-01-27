#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <q.h>


static unsigned long *esp;

/*------------------------------------------------------------------------
 * void printprocstks()
 *     o For each existing process with smaller priority than the 
 *       parameter, print the stack base, stack size, stacklimit, 
 *       and stack pointer. Also, for each process, include the process 
 *       name, the process id and the process priority.
 *------------------------------------------------------------------------
 */
void printprocstks(int priority) {

    int i;
    struct pentry *pptr;

    printf("# Filter: %u\n", priority);

    for (i=0; i<NPROC; i++) {

        pptr = &proctab[i];

        // If this process slot is free skip it.
        if (pptr->pstate == PRFREE)
            continue;
        // If this process has higher priority then skip it
        if (pptr->pprio >= priority)
            continue;

        // If the process is the current process then we need
        // to get the stack pointer from the %esp register.
        if (i == currpid)
            asm("movl %esp,esp"); //Uses AT&T Syntax
        else
            esp = (unsigned long *)pptr->pesp;

        printf("Process [%s]\n", pptr->pname);
        printf("\tpid: %u\n", i);
        printf("\tpriority: %u\n", pptr->pprio);
        printf("\tbase: 0x%08x\n", pptr->pbase);
        printf("\tlimit: 0x%08x\n", pptr->plimit);
        printf("\tlen: %u\n", pptr->pstklen);
        printf("\tpointer: 0x%08x\n", esp);


    }

}
