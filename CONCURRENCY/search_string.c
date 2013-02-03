
////void main () {

////    // Search strings no longer than 80 chars
////    //


////    // Each thread handles the length of search_string more over the
////    // end of their boundary.. This accounts for matches that may
////    // occur on block boundaries. 
//

////    // Vary # of threads. blocksize is equal to size of file
////    // divided by # of threads.
//
////    // Need to use gettimeofday()
//
//      // use scanf for string match detection


////}


#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#include <sys/stat.h>
#define STDIN_FD 0
#define MAX_SEARCH 80

#define handle_error_en(en, msg) \
       do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
       do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct thread_info {    /* Used as argument to thread_start() */
   pthread_t thread_id;        /* ID returned by pthread_create() */
   int       thread_num;       /* Application-defined thread # */
   char     *argv_string;      /* From command-line argument */
};



// Function declarations
void stdin_to_memory();
void find_matches();


// This is a global pointer that will point to the location in
// memory where the contents of the file passed in on stdin will
// be replicated.
char *data;

// The size of the file passed in on STDIN in bytes
long fsize;
void find_matches(char* start, int size, char* pattern) {

    char* end = start + size + MAX_SEARCH;

    if (end > (data + fsize))
        end = data + fsize;

    char* loc = start;

    int count = 0;

    while (loc = strstr(loc, pattern)) {
        if (loc > end)
            break;

        // Move loc pass the current instance of 
        // the pattern. 
        loc += sizeof(pattern);

        // Bump our instance counter and move on
        count++;
    }

    printf("Count %d\n", count);

}


/* Thread start function: display address near top of our stack,
  and return upper-cased copy of argv_string */

static void * thread_start(void *arg) {
   struct thread_info *tinfo = (struct thread_info *) arg;
   char *uargv, *p;

   printf("Thread %d: top of stack near %p; argv_string=%s\n",
           tinfo->thread_num, &p, tinfo->argv_string);

   uargv = strdup(tinfo->argv_string);
   if (uargv == NULL)
       handle_error("strdup");

   for (p = uargv; *p != '\0'; p++)
       *p = toupper(*p);

   return uargv;
}

int main(int argc, char *argv[]) {
   int s, tnum, opt, num_threads;
   struct thread_info *tinfo;
   pthread_attr_t attr;
   int stack_size;
   void *res;

    if (argc != 2)
        fprintf(stderr, "Expected 2 arguments");

    stdin_to_memory();

   // printf("%s", data);

    //find_matches(data, fsize, "This is");
    find_matches(data, fsize, argv[1]);

    exit(EXIT_SUCCESS);


   num_threads = argc - optind;

   /* Initialize thread creation attributes */

   s = pthread_attr_init(&attr);
   if (s != 0)
       handle_error_en(s, "pthread_attr_init");

   if (stack_size > 0) {
       s = pthread_attr_setstacksize(&attr, stack_size);
       if (s != 0)
           handle_error_en(s, "pthread_attr_setstacksize");
   }

   /* Allocate memory for pthread_create() arguments */

   tinfo = calloc(num_threads, sizeof(struct thread_info));
   if (tinfo == NULL)
       handle_error("calloc");



   /* Create one thread for each command-line argument */

   for (tnum = 0; tnum < num_threads; tnum++) {
       tinfo[tnum].thread_num = tnum + 1;
       tinfo[tnum].argv_string = argv[optind + tnum];

       /* The pthread_create() call stores the thread ID into
          corresponding element of tinfo[] */

       s = pthread_create(&tinfo[tnum].thread_id, &attr,
                          &thread_start, &tinfo[tnum]);
       if (s != 0)
           handle_error_en(s, "pthread_create");
   }

   /* Destroy the thread attributes object, since it is no
      longer needed */

   s = pthread_attr_destroy(&attr);
   if (s != 0)
       handle_error_en(s, "pthread_attr_destroy");

   /* Now join with each thread, and display its returned value */

   for (tnum = 0; tnum < num_threads; tnum++) {
       s = pthread_join(tinfo[tnum].thread_id, &res);
       if (s != 0)
           handle_error_en(s, "pthread_join");

       printf("Joined with thread %d; returned value was %s\n",
               tinfo[tnum].thread_num, (char *) res);
       free(res);      /* Free memory allocated by thread */
   }

   free(tinfo);
   exit(EXIT_SUCCESS);
}


/*
 * This function will read the file that was passed in on
 * stdin and store it in the heap. This will be accessible 
 * by all threads of the program. The global variable data
 * will be updated to point to this memory. 
 */
void stdin_to_memory() {

    char *cursor;   // Pointer to the current location in memory
                    // where we are writing into.

    int nr;         // Keeps up with the # of bytes read this iter
    struct stat sb; // stat structure (stat.h)  



    // Get the stat information of stdin.
    if (fstat(STDIN_FD, &sb) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    // Set the global var fsize to be the size of the file 
    fsize = sb.st_size;

    //printf("Size is %lld\n", (long long) sb.st_size);


    // Now that we know the size lets allocate a chunk of memory
    // large enough to handle it. 
    cursor = data = calloc(fsize, sizeof(char));
    if (data == NULL)
       handle_error("data"); //XXX


    // Read in the file from STDIN 
    while(nr = read(STDIN_FD, cursor, 50))
        cursor += nr;

}

