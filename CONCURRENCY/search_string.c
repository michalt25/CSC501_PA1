
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


struct thread_info {    /* Used as argument to thread_start() */
   pthread_t thread_id;        // ID returned by pthread_create()
   int       tnum;             // Application-defined thread #
   int       bsize;            // Size (bytes) of assigned block of the string
   char     *start;            // Location of the start of the block
   char     *pattern;          // Location of a string representing the search pattern
};



// Function declarations
void stdin_to_memory();
int find_matches(char* start, int size, char* pattern);


// This is a global pointer that will point to the location in
// memory where the contents of the file passed in on stdin will
// be replicated.
char *data;

// The size of the file passed in on STDIN in bytes
long fsize;
int find_matches(char* start, int size, char* pattern) {

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

  //printf("Count %d\n", count);

    return count;

}


// Thread start function
static void * thread_start(void *arg) {
    struct thread_info *tinfo = (struct thread_info *) arg;
    return (void *) find_matches(tinfo->start, tinfo->bsize, tinfo->pattern);
}

int main(int argc, char *argv[]) {
    char* pattern;
    int count, i, rc, nthreads, blocksize;
    struct thread_info *tinfo;
    pthread_attr_t attr;

    void* x;

    if (argc != 2) {
        fprintf(stderr, "Expected 2 arguments\n");
        exit(EXIT_FAILURE);
    }

    // The pattern we are searching for was provided
    // on the command line;
    pattern = argv[1];
    nthreads = 10;

    stdin_to_memory();

    blocksize = fsize/nthreads + 1;

   // printf("%s", data);

    //find_matches(data, fsize, "This is");
//  find_matches(data, fsize, argv[1]);

// exit(EXIT_SUCCESS);




    // Initialize thread creation attributes
    rc = pthread_attr_init(&attr);
    if (rc != 0) {
       errno = rc;
       perror("pthread_attr_init");
       exit(EXIT_FAILURE);
    }

    // Allocate memory for pthread_create() arguments
    tinfo = calloc(nthreads, sizeof(struct thread_info));
    if (tinfo == NULL) {
       perror("calloc");
       exit(EXIT_FAILURE);
    }


    // Spawn off nthreads
    for (i = 0; i < nthreads; i++) {
        tinfo[i].tnum    = i+1;
        tinfo[i].bsize   = blocksize; 
        tinfo[i].start   = data + blocksize*i; 
        tinfo[i].pattern = pattern; 
        

        // The pthread_create() call stores the thread ID into
        // corresponding element of tinfo[]
        rc = pthread_create(
            &tinfo[i].thread_id, 
            &attr,
            &thread_start,
            &tinfo[i]
        );
        if (rc != 0) {
            errno = rc;
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Destroy the thread attributes object, since it is no longer needed 
    rc = pthread_attr_destroy(&attr);
    if (rc != 0) {
        errno = rc;
        perror("pthread_attr_destroy");
        exit(EXIT_FAILURE);
    }



    // Now join with each thread, and display its returned value
    for (i = 0; i < nthreads; i++) {
        rc = pthread_join(tinfo[i].thread_id, &x);
        if (rc != 0) {
            errno = rc;
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }

        printf("Joined with thread %d; returned value was %d\n",
               tinfo[i].tnum, (int) x);

        count += x;

    }

    printf("Total Count: %d\n", count);

   free(tinfo);
   free(data);
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
    if (data == NULL) {
        perror("data");
        exit(EXIT_FAILURE);
    }

    // Read in the file from STDIN 
    while(nr = read(STDIN_FD, cursor, 50))
        cursor += nr;

}

