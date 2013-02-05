/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <math.h>
#include <sched.h>

unsigned long currSP;   /* REAL sp of current process */
extern int ctxsw(int, int, int, int);
int goodness(struct pentry *p);
void newepoch();
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

    int item;    // Index of new process in qent array
    int class;   // The class of scheduling we are using
    double mark; // The random number from expdev()

        int tmp;
        int weight;
        int nitem;

    // Get pointer to current process entry
    optr = &proctab[currpid];

    // Get the scheduling class we are using
    class = getschedclass();


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
    // current process (NULL PROCESS). 
    if ((optr->pstate == PRCURR) && isempty(rdyhead))
        return OK;

    // Perform scheduling based on what scheduling algorithm we
    // are using.
    switch (class) {
     case EXPDISTSCHED: 

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
        

        mark = expdev(0.1);

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


        // If we somehow ended up on the tail item it means that the random 
        // value is larger than the largest priority in the ready queue. 
        // We must choose the value with the largest priority in this case.
        //
        // Note: This will select NULLPROC if no other processes are ready
        if (item == rdytail)
            item = q[rdytail].qprev;


        // We need to consider the currently running process. If its
        // priority is greater than mark and less than the currently
        // selected processes priority then don't context switch. 
        if ((mark < optr->pprio) && (optr->pprio < q[item].qkey))
            if (optr->pstate == PRCURR)
                return OK;

        //XXX need to do anything for null process?
        //

        break;

     case LINUXSCHED:

        //////////////
        // Linux Scheduler
        //////////////

        // Update counter for current process
        // preempt was set to counter before. Now the amount of time
        // that has passed has been (counter - preempt). Subtract
        // that amount of time from the counter.
        if (optr->counter)
            optr->counter = preempt; // Same thing as optr->counter = optr->counter - (optr->counter - preempt);
        if (optr->counter < 0)
            optr->counter = 0;



    // Until we have chosen a process
    while(1) {

        if (optr->pstate == PRCURR) {
            nitem  = -1;
            weight = goodness(optr);
        } else {
            nitem  = q[rdyhead].qnext;
            weight = goodness(&proctab[nitem]);
        }

        // Go through all items in the rdy q and 
        // Find the one with the most goodness.
        item = q[rdyhead].qnext;
        while (item != rdytail) {
            tmp = goodness(&proctab[item]);
            if (tmp >= weight) {
                weight = tmp;
                nitem = item;
            }
            item = q[item].qnext;
        }


        // A couple of possibilities here:
        //
        // weight == -1 - The null process is the only process
        // weight ==  0 - All quantum for all processes is exhausted
        // weight  >  0 - There is an eligible process


        // If no processes were eligible then the epoch is done.
        // Recalculate.
        if (weight == 0) {
            newepoch();
            continue;
        }

        // It is ok to stick with the running process if no
        // better candidate was found 
        if ((nitem == -1) && (optr->pstate == PRCURR)) {
            preempt = optr->counter;
            return OK;
        }

        item = nitem;
        break;
        }
    }


    // If we get here then we are not sticking with the current
    // process and we have a new process to context switch into. 

    /* force context switch */

    if (optr->pstate == PRCURR) {
        optr->pstate = PRREADY;
        insert(currpid,rdyhead,optr->pprio);
    }
    currpid = item;             // Update the currpid global 
    dequeue(item);              // Remove the process from the ready list  
    nptr = &proctab[currpid];   // Get a pointer to the PCB(pentry) for the proc
    nptr->pstate = PRCURR;      // mark it currently running
    if (class == LINUXSCHED)
        preempt = nptr->counter;
    
    ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
    
    /* The OLD process returns here when resumed. */
    return OK;

}


int goodness(struct pentry *p) {

    // XXXhandle null proc?
    if (p == &proctab[0])
        return -1;

    // If the process has exhausted its quantum then
    // its goodness is 0
    if (p->counter == 0)
        return 0;

    // 
    return p->counter + p->pprio;
    

    // XXX what to return for null process?

}


void newepoch() {

    int i;
    struct pentry *p;


    for (i=0; i < NPROC; i++) {

        p = &proctab[i];

        // If this process slot is free skip it.
        if (p->pstate == PRFREE)
            continue;

        // Assign a new value of quantum for the process
        // If a process has left over quantum they get to 
        // carry over roughly half of it. 
        p->counter = (p->counter >> 1) + p->pprio; 
    }
}
