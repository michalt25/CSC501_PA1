/* sched.h */

#define EXPDISTSCHED 1
#define LINUXSCHED   2

int schedclass = EXPDISTSCHED;

void setschedclass(int sc) {
    schedclass = sc;
}
int getschedclass() {
    return schedclass;
}
