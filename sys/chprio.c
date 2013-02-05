/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <sched.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
    STATWORD ps;    
    struct  pentry  *pptr;

    disable(ps);
    if (isbadpid(pid) || newprio<=0 ||
        (pptr = &proctab[pid])->pstate == PRFREE) {
        restore(ps);
        return(SYSERR);
    }
    pptr->pprio = newprio;

    // If we are doing exponential scheduling then
    // we need to change the processes order in the 
    // ready queue. 
    if ((getschedclass() == EXPDISTSCHED) && (pid != currpid)) {
        dequeue(pid);
        insert(pid, rdyhead, newprio);
    }
    restore(ps);
    return(newprio);
}
