/* math.h */
////////double log(double x);
////////double pow(double x, int y);
////////double expdev(double lambda);


#define EXPDISTSCHED 1
#define LINUXSCHED   2

void setschedclass(int sched_class);
int getschedclass();
