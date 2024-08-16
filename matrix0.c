/********************************************************
 * An example source module to accompany...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 *     by Brad nichols, Dick Buttlar, Jackie Farrell
 *     O'Reilly & Associates, Inc.
 * to compile, use -lpthread  
 *     gcc -lpthread 
 ********************************************************
 * matrix0.c --
 *   matrix multiplication: MA * MB = MC  
 *   where each matrix is 10x10 (ARRAY_SIZE x ARRAY_SIZE) 
 *         
 * A master thread spawns separate child threads to compute each
 * element in the resulting array. That is, 10x10=100 threads
 *
 * Each of the child threads is passed a pointer to a structure 
 * that contains the element indices and pointers to starting 
 * and resulting arrays.
 *
 * The master thread joins to each thread, prints the result and 
 * exits.
 *********************************************************
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <pthread.h>

#define MIN_REQ_SSIZE 81920
#define ARRAY_SIZE 1000

pthread_mutex_t mutexsum;

typedef int matrix_t[ARRAY_SIZE][ARRAY_SIZE];

typedef struct {
  int       id;
  int       size;
  int       BcolMax;
  int       BcolMin;
  matrix_t  *MA, *MB, *MC;
} package_t; 

matrix_t MA,MB,MC;

int dSum;

/*
* Routine to multiply a row by a column and place element in 
* resulting matrix.
*/
void multiply(int size,
	  int row,
	  int column,
	  matrix_t MA, 
	  matrix_t MB,
	  matrix_t MC,
	  int id	)
{
  int position;
  int sum = 0;

  MC[row][column] = 0;
  for(position = 0; position < size; position++) {
    sum = sum + ( MA[row][position] * MB[position][column] ) ;
  }


  /* your code for mutex to lock here */
    pthread_mutex_lock(&mutexsum);
    MC[row][column] =     MC[row][column] + sum;
	if(row == column) {
		dSum = MC[row][column] + dSum;
	}
	printf("Thread %d updates MC(%d,%d) set to the sum %d\n", id, row, column, sum);
	pthread_mutex_unlock(&mutexsum);
  /* your code for mutex to unlock here */

}


/*
 * Routine to start off a worker thread. 
 */
void *mult_worker(void *arg)
{
  package_t *p=(package_t *)arg;\
	int row;
	int col;
	for(row = 0; row < p->size; row++) {
		for(col = p->BcolMin; col < p->BcolMax; col++) {
 		 	multiply(p->size, row, col, 
				*(p->MA), *(p->MB), *(p->MC), p->id);

 
		}
  	}
	printf("MATRIX THREAD %d: complete\n", p->id);

  	free(p);
  return(NULL); 
}

/*
* Main(): allocates matrix, assigns values, then 
* creates threads to process rows and columns.
*/

extern int
main(int argc, char **argv)
{
  int       size, row, column, num_threads, i;
  pthread_t *threads;  /* threads holds the thread ids of all 
			         threads created, so that the
			         main thread can join with them. */
  package_t *p;      /* argument list to pass to each thread. */

	dSum = 0;
  
  unsigned long thread_stack_size;
  pthread_attr_t *pthread_attr_p, pthread_custom_attr;

  /* Currently size hardwired to ARRAY_SIZE size */
  size = atoi(argv[1]);

  /* one thread will be created for each element of the matrix */
  threads = (pthread_t *)malloc(size*sizeof(pthread_t));
  
  /* Fill in matrix values, currently values are hardwired */
  for (row = 0; row < size; row++) {
    for (column = 0; column < size; column++) {
      MA[row][column] = 1;
    }
  }

  for (row = 0; row < size; row++) {
    for (column = 0; column < size; column++) {
      MB[row][column] = row + column + 1;
    }
  }
 
 /* This is not required for the program as the peers stack 
    will not be too big. It is just here to show how to check
    and set pthread attributes.
 */

  pthread_attr_init(&pthread_custom_attr);

#ifdef _POSIX_THREAD_ATTR_STACKSIZE 
  pthread_attr_getstacksize(&pthread_custom_attr, 
                            &thread_stack_size);

  if (thread_stack_size < MIN_REQ_SSIZE) {
    pthread_attr_setstacksize(&pthread_custom_attr, 
                              (long)MIN_REQ_SSIZE);
  }

#endif 

 

  num_threads = 0;
  
  int split = size/atoi(argv[2]);
  for(i = 0; i < atoi(argv[2]); i++){
	int startPos = split * num_threads;
	int endPos = split * (num_threads + 1);
      p = (package_t *)malloc(sizeof(package_t));
      p->id = num_threads;
      p->size = size;
      p->BcolMax = split*(num_threads + 1);
      p->BcolMin = split*num_threads;
      (p->MA) = &MA;
      (p->MB) = &MB;
      (p->MC) = &MC;
      pthread_create(&threads[num_threads], 
		     &pthread_custom_attr,
		     mult_worker, 
		     (void *) p);

      printf("MATRIX MAIN THREAD: thread %d created\n", 
               num_threads);

      num_threads++;
     } 

  

/* Synchronize on the completion of the element in each thread */
  for (i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
    printf("MATRIX MAIN THREAD: child %d has joined\n", i);
  }
  

/* Print results */
  printf("MATRIX MAIN THREAD: The resulting matrix C is;\n");
  for(row = 0; row < size; row ++) {
    for (column = 0; column < size; column++) {
      printf("%5d ",MC[row][column]);
    }
    printf("\n");
  }
	printf("Diagonal Sum: %d\n",dSum);

	system("date; hostname; who | grep 230002; ps -eaf; ls -l  ");
	system(" who > week10who.txt");
	
  return 0;
} 	
