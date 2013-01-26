/* math.c */

#include <stdio.h>

// Looking at rand(), a value is computed and then gets anded (&)
// with octal 077777. This is then returned as the random number. 
// This means that the maximum possible value ever returned will be
// octal 077777. Thus RAND_MAX is (077777 + 1).
#define RAND_MAX (077777 + 1)

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
    //
    // XXX Is this needed?
    if (dummy < 0) 
        dummy = -dummy;
    return -log(dummy) / lambda;
}
