/* math.c */

#include <stdio.h>

// Maximum value for long type. long is 4 bytes on
// XINU therefore max value is 0xffffffff.
//#define RAND_MAX 0xffffffff
//#define RAND_MAX 0xffff
//#define RAND_MAX 77778
//#define RAND_MAX 32767
//#define RAND_MAX                10010111111010001
//#define RAND_MAX 00000000000000001111111111111111
//#define RAND_MAX 65535
//#define RAND_MAX 65536
#define RAND_MAX (077777 + 1)
//#define RAND_MAX 0100000

double pow(double x, int y) {
    double tmp = x;
    while (--y)
        tmp *= x;
    return tmp;
}

//http://www.sosmath.com/calculus/tayser/tayser01/tayser01.html
double log(double x) {

    double sum = 0;
    double tmp;
    int n;

    for (n=1; n<=20; n++) {

        tmp = pow(-1, n+1);
        tmp = tmp/n;
        tmp = tmp * pow(x-1, n);

        sum += tmp;
    }
    //kprintf("log[%d]=%d\n", (int)x, (int)sum);
    return sum;
}

double expdev(double lambda) {
    double dummy;
    do 
        dummy = (double) rand() / RAND_MAX; 
    while (dummy == 0.0); 
    if (dummy < 0) 
        dummy = -dummy;
    return -log(dummy) / lambda;
}
