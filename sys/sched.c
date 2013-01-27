/* sched.c */

#include <sched.h>

int schedclass = EXPDISTSCHED;

void setschedclass(int sc) {
    schedclass = sc;
}
int getschedclass() {
    return schedclass;
}
