
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>



#define STDIN_FD 0
#define MAX_SEARCH 80 // The maximum length of the search string


// Note: The code for creating and joining threads was heavily
//       mirrored after the code in the pthread_create man page.


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


// Thread start function
static void * thread_start(void *arg) {
    struct thread_info *tinfo = (struct thread_info *) arg;
    return (void *) find_matches(tinfo->start, tinfo->bsize, tinfo->pattern);
}

int main(int argc, char *argv[]) {
    char* pattern;
    char* tmp;
    int i, rc, nthreads, blocksize;
    struct thread_info *tinfo;
    pthread_attr_t attr;
    unsigned int count;

    struct timeval tv1,tv2;
    unsigned long secs, usecs;

    void* x;

    if (argc != 2) {
        fprintf(stderr, "Expected 2 arguments\n");
        exit(EXIT_FAILURE);
    }

    // The pattern we are searching for was provided
    // on the command line;
    pattern = argv[1];

    // Read the file into memory
    stdin_to_memory();


    for (nthreads = 1; nthreads <= 100; nthreads++) {
        count = 0;
        
        // Set the size that each thread will work on
        blocksize = fsize/nthreads + 1;



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

        // Prepare the thread info data structures for each thread
        for (i = 0; i < nthreads; i++) {
            tinfo[i].tnum    = i+1;
            tinfo[i].pattern = pattern; 

            // Make a copy of the data this thread will operate on.
            // Make it MAX_SEARCH longer than it needs to be so that
            // we can handle matches across boundaries
            tinfo[i].start = strndup(data + blocksize*i, blocksize + MAX_SEARCH);

            // If we are in the very last block then adjust the blocksize 
            if ((data + blocksize*i) > (data + fsize))
                tinfo[i].bsize = (data + fsize) - (data + blocksize*i); 
            else 
                tinfo[i].bsize = blocksize; 
        }

        gettimeofday(&tv1, NULL);

        // Spawn off nthreads
        for (i = 0; i < nthreads; i++) {

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

          //printf("Joined with thread %d; returned value was %d\n",
          //       tinfo[i].tnum, (int) x);


            free(tinfo[i].start);

            count += (unsigned int) x;

        }

        // Determine amount of time that was spent here.
        gettimeofday(&tv2, NULL);
        secs  = tv2.tv_sec - tv1.tv_sec;
        usecs = secs*1000000 + tv2.tv_usec - tv1.tv_usec;

        printf("nthreads: %d, Total Count: %d, usecs: %lu\n", nthreads, count, usecs);
        free(tinfo);
    }

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

    // Now that we know the size lets allocate a chunk of memory
    // large enough to handle it. 
    cursor = data = calloc(fsize, sizeof(char));
    if (data == NULL) {
        perror("data");
        exit(EXIT_FAILURE);
    }

    // Read in the file from STDIN 
    while(nr = read(STDIN_FD, cursor, 128))
        cursor += nr;

}


/*
 * To be called by thread_start(). This function make a copy of the
 * block defined by start and size and will then find all matching
 * strings within that block.
 */
int find_matches(char* start, int size, char* pattern) {

    char* end;
    char* loc;
    int count = 0;
    int searchlen = strlen(pattern);

    // Determine where the end of our block is. We won't 
    // accept any matches past the end of the block.
    end = start + size;

    // Update temporary variable to point to start of block
    loc = start;

    // Iterate through the string finding matches along the way
    while (loc = strstr(loc, pattern)) {
        if (loc >= end)
            break;

        // Move loc past the current instance of 
        // the pattern. 
        loc += searchlen;

        // Bump our instance counter and move on
        count++;
    }

    // Return the value to the caller.
    return count;
}

