/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <math.h>

double e_total = 0.0;
int e_num = 0;

unsigned long currSP;   /* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:   Upon entry, currpid gives current process id.
 *      Proctab[currpid].pstate gives correct NEXT state for
 *          current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
    register struct pentry  *optr;  /* pointer to old process entry */
    register struct pentry  *nptr;  /* pointer to new process entry */
    int item;
    double mark;

    // This scheduler chooses a next process based on a random value
    // that follows an exponential distribution. When a rescheduling 
    // occurs, the scheduler generates a random number with exponential
    // distribution and a process with the smallest priority that is 
    // larger than the random number is scheduled. If the random value 
    // is less than the smallest priority in the ready queue, then
    // the process with the smallest priority is scheduled. If the
    // random value is no less than the largest priority in the ready
    // queue, the process with the largest priority is chosen. When 
    // processes have the same priority, the processes should be scheduled
    // in a roundrobin way.
    

    optr = &proctab[currpid];
    mark = expdev(0.1);

    e_total += mark;
    e_num   += 1;


    // NOTE: There is an array in memory that make up several 
    //       queues including the ready queue structure. This
    //       array is set up in such a way that index 0 to
    //       NPROC-1 correspond to the processes with PID equal
    //       to the index number. Each entry in this array is a
    //       a qent structure with three ints in it (qnext, qprev, 
    //       qkey). The ready queue head item is simply later in
    //       array and its qnext points to a valid process as is
    //       shown below. The ready tail works similarly.
    //       __________________________________   
    //      /                                  |
    //+---+---+---+---+---+---+---------+---+-------+-------+
    //| 0 | 1 | 2 | 3 | 4 |...|NPROC - 1|...|rdyhead|rdytail|
    //+---+---+---+---+---+---+---------+---+-------+-------+
    //      |      ^ | ^^|                            ^ |
    //      |______| |_|||____________________________| |
    //                  |_______________________________|
    //

    // If the ready queue is empty then just stick with the 
    // current process. 
    if ((optr->pstate == PRCURR) && isempty(rdyhead))
        return(OK);

    //
    // Find the first entry in the ready queue with priority greater
    // than "mark". The queue has smallest priority tasks at head
    // and larger (higher) priority tasks at the tail. If the random value 
    // ("mark") is less than the smallest priority in the ready 
    // queue, then the process with the smallest priority (the head 
    // of the ready queue) is scheduled.
    //
    // NOTE: The tail has MAXINT as its qkey value.
    item = q[rdyhead].qnext;
    while (q[item].qkey < mark)
        item = q[item].qnext;

    // If multiple processes have this priority then they should be
    // scheduled in a round robin fashion.
    //
    // Looking at insert.c it seems that if a process gets added
    // to the queue and there are other processes with the same
    // priority then it will get added closer to the head of the
    // queue. i.e:
    //
    // head - 10 - 11 - 20 - 20 - 45 - tail
    //
    // head - 10 - 11 - 20 - 20 - 20 - 45 - tail
    //                   ^
    //                   |
    //
    // This means in order to do round robin for processes with the
    // same priority, we need to use the process closer to the tail.
    //
    while (q[item].qkey == q[q[item].qnext].qkey)
        item = q[item].qnext;


    // If we somehow ended up on the item it means that the random 
    // value is larger than the largest priority in the ready queue. 
    // We must choose the value with the largest priority in this case.
    if (item == rdytail)
        item = q[rdytail].qprev;


    // We need to consider the currently running process. If its
    // priority is greater than mark and less than the currently
    // selected processes priority then don't context switch. 
    if ((mark < optr->pprio) && (optr->pprio < q[item].qkey))
        if (optr->pstate == PRCURR)
            return(OK);


    /* force context switch */

    if (optr->pstate == PRCURR) {
        optr->pstate = PRREADY;
        insert(currpid,rdyhead,optr->pprio);
    }


    /* remove highest priority process at end of ready list */
    currpid = item;             // Update the currpid global 
    dequeue(item);              // Remove the process from the ready list  
    nptr = &proctab[currpid];   // Get a pointer to the PCB(pentry) for the proc
    nptr->pstate = PRCURR;      // mark it currently running
    
    ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
    
    /* The OLD process returns here when resumed. */
    return OK;

//ORIG


////    /* no switch needed if current process priority higher than next*/

////    if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
////       (lastkey(rdytail)<optr->pprio)) {
////        return(OK);
////    }
////    
////    /* force context switch */

////    if (optr->pstate == PRCURR) {
////        optr->pstate = PRREADY;
////        insert(currpid,rdyhead,optr->pprio);
////    }

////    /* remove highest priority process at end of ready list */

////    nptr = &proctab[ (currpid = getlast(rdytail)) ];
////    nptr->pstate = PRCURR;      /* mark it currently running    */
////#ifdef  RTCLOCK
////    preempt = QUANTUM;      /* reset preemption counter */
////#endif
////    
////    ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
////    
////    /* The OLD process returns here when resumed. */
////    return OK;
//ORIG
}
