#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <sched.h>
#include <stdio.h>
#include <math.h>
#include <printprocstks.h>
#include <printrdyq.h>

#define LOOP	50

int prA, prB, prC;
int proc_a(), proc_b(), proc_c();
int proc(char c);
int proc_basic(char c);
volatile int a_cnt = 0;
volatile int b_cnt = 0;
volatile int c_cnt = 0;
volatile int s = 0;

int main() {
	int i;
	int count = 0;
	char buf[8];

	srand(1234);
	int total;

	kprintf("Please Input:\n");
	while ((i = read(CONSOLE, buf, sizeof(buf))) < 1)
		;
	buf[i] = 0;
	s = atoi(buf);
	kprintf("Get %d\n", s);

	// EXPDISTSCHED
	if (s < 2) {
		setschedclass(EXPDISTSCHED);
		switch (s) {
		case 1:
			prA = create(proc_a, 2000, 10, "proc A", 1, 'A');
			prB = create(proc_b, 2000, 20, "proc B", 1, 'B');
			prC = create(proc_c, 2000, 30, "proc C", 1, 'C');
			resume(prA);
			resume(prB);
			resume(prC);
			break;
		}

		sleep(10);
		kill(prA);
		kill(prB);
		kill(prC);

		switch (s) {
		case 1:
			kprintf("\nTest2 RESULT: A = %d, B = %d, C = %d\n", a_cnt, b_cnt,
					c_cnt);
			total = a_cnt + b_cnt + c_cnt;
			kprintf("\n%d\t%d\t%d\n", 
				100*a_cnt/total,
				100*b_cnt/total,
				100*c_cnt/total
			);
			break;
		}
	}
	// LINUXSCHED
	else {
	        kprintf("HERE1\n");
	        printrdyq();
	        sleep(2);
	        kprintf("HERE2\n");
	        printrdyq();
	        setschedclass(LINUXSCHED);
	        kprintf("HERE3\n");
	        printrdyq();
	        sleep(2);
	        kprintf("HERE4\n");
		switch (s) {
		case 2:
			resume(prA = create(proc, 2000, 5, "proc A", 1, 'A'));
			resume(prB = create(proc, 2000, 50, "proc B", 1, 'B'));
			resume(prC = create(proc, 2000, 90, "proc C", 1, 'C'));
			break;
		}

		//printprocstks(100);
	//	sleep(2);

		while (count++ < LOOP) {
			kprintf("M");

			if (s == 7 && count == 5) {
				kprintf("CHPRIO\n");
				chprio(prA, 90);
				chprio(prB, 60);
				chprio(prC, 30);
			}
			if (s == 2 || s == 5 || s == 7) {
				for (i = 0; i < 10000000; i++)
					;
			} else if (s == 6)
				sleep(1);
		}
	        printrdyq();

	}

}

proc_basic(char c) {
	int count = 0;

	//kprintf("\nStart %c...\n", c);
	while (count++ < 1000) {
		kprintf("%c", c);
	}
}


proc_a(c)
	char c; {
	int i;
	kprintf("Start... %c\n", c);
	if (s < 4) {
		b_cnt = 0;
		c = 0;
	}

	while (1) {
		for (i = 0; i < 10000; i++)
			;
		a_cnt++;
	}
}

proc_b(c)
	char c; {
	int i;
	if (s < 4) {
		a_cnt = 0;
		c_cnt = 0;
	}

	kprintf("Start... %c\n", c);
	while (1) {
		for (i = 0; i < 10000; i++)
			;
		b_cnt++;
	}
}

proc_c(c)
	char c; {
	int i;
	if (s < 4) {
		a_cnt = 0;
		b_cnt = 0;
	}

	kprintf("Start... %c\n", c);
	while (1) {
		for (i = 0; i < 10000; i++)
			;
		c_cnt++;
	}
}

proc(char c) {
	int i;
	int count = 0;

	while (count++ < LOOP) {
		kprintf("%c", c);
		if (s == 2 || s == 5 || s == 7) {
			for (i = 0; i < 10000000; i++)
				;
		} else if (s == 6) {
			sleep(1);
		}
	}

}
