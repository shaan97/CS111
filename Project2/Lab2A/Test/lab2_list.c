/* NAME: SHAAN MATHUR
   EMAIL: SHAANKARANMATHUR@GMAIL.COM
   UID: 904606576
*/



#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h>
#include <sched.h>
#include <string.h>
#include <signal.h>
#include "SortedList.h"
#include <assert.h>
#include <errno.h>

int opt_yield = 0;
pthread_mutex_t mutex;

void mutex_insert(SortedList_t * list, SortedListElement_t * element){
  pthread_mutex_lock(&mutex);
  SortedList_insert(list, element);
  pthread_mutex_unlock(&mutex);
}

SortedListElement_t* mutex_lookup(SortedList_t * list, const char *key){
  pthread_mutex_lock(&mutex);
  SortedListElement_t * x = SortedList_lookup(list, key);
  pthread_mutex_unlock(&mutex);
  return x;
}

int mutex_length(SortedList_t * list){
  pthread_mutex_lock(&mutex);
  int x = SortedList_length(list);
  pthread_mutex_unlock(&mutex);
  return x;
}

int mutex_delete(SortedListElement_t * element){
  pthread_mutex_lock(&mutex);
  int val = SortedList_delete(element);
  pthread_mutex_unlock(&mutex);
  return val;
}

int s_flag = 0;
void spin_insert(SortedList_t * list, SortedListElement_t * element){
  while(__sync_lock_test_and_set(&s_flag, 1))
    ;
  SortedList_insert(list, element);
  __sync_lock_release(&s_flag);
}

int spin_delete(SortedListElement_t * element){
  while(__sync_lock_test_and_set(&s_flag, 1))
    ;
  int val = SortedList_delete(element);
  __sync_lock_release(&s_flag);
  return val;
}

SortedListElement_t* spin_lookup(SortedList_t * list, const char *key){
  while(__sync_lock_test_and_set(&s_flag, 1))
    ;
  
  SortedListElement_t * x = SortedList_lookup(list, key);
  __sync_lock_release(&s_flag);
  return x;
}

int spin_length(SortedList_t * list){
  while(__sync_lock_test_and_set(&s_flag, 1))
    ;
  int x = SortedList_length(list);
  __sync_lock_release(&s_flag);
  return x;
}

long long diff(struct timespec start, struct timespec end, struct timespec delta){
  long long d1 = 0, d2 = 0;
  if(end.tv_nsec - start.tv_nsec < 0){
    d1 = 1e9;
  }

  if(delta.tv_nsec - end.tv_nsec < 0)
    d2 = 1e9;

  long long x = 1e9*(2*end.tv_sec - start.tv_sec - delta.tv_sec);
  long long y = ((d1 + end.tv_nsec - start.tv_nsec) - (d2 + delta.tv_nsec - end.tv_nsec));
  return (x + y);

}


char * generate_string(){
  int num_bytes = (rand() % 256) + 2;
  char * arr = (char *) malloc(sizeof(int) * num_bytes);
  if(arr == NULL){
    fprintf(stderr, "Error Number: %d. Error Message: %s", errno, strerror(errno));
    exit(1);
  }
  
  int i;
  for(i = 0; i < num_bytes - 1; i++){
    arr[i] = (char) ((rand() % 26) + 65);
  }
  arr[num_bytes - 1] = 0;

  return arr;
}

void seg_fault(int signum){
  fprintf(stderr, "Segementation Fault Detected. List was possibly corrupted.\n");
  exit(2);
}

typedef struct arg{
  SortedList_t* list;
  SortedListElement_t * nodes;
  void (*insert)(SortedList_t*, SortedListElement_t*);
  int (*delete)(SortedListElement_t*);
  int (*length)(SortedList_t *);
  SortedListElement_t* (*lookup)(SortedList_t *, const char*);
  int start, end;
} argument;
  

void* create_and_delete(void* arg){
  argument* a = (argument *) arg;
  int start = a->start;
  int end = a->end;

  int i;
  for(i = start; i < end; i++){
    a->insert(a->list, &(a->nodes[i]));
    
  }
  if(a->length(a->list) == -1){
    fprintf(stderr, "List was corrupted. Length analysis showed corruption.\n");
    exit(2);
  }
  for(i = start; i < end; i++){
    SortedListElement_t * temp;
    if((temp = a->lookup(a->list, a->nodes[i].key)) == NULL  || a->delete(temp) == 1){
      if(temp == NULL)
	fprintf(stderr, "List was corrupted. Lookup analysis showed corruption.\n");
      else
	fprintf(stderr, "List was corrupted. Deletion analysis showed corruption.\n");
      exit(2);
    }
    
  }
}

int main(int argc, char** argv){
  srand(time(NULL));
  long long numThreads = 1; /* COMMAND LINE ARGUMENT */
  long long iterations = 1; /* COMMAND LINE ARGUMENT */
  void (*insert)(SortedList_t*, SortedListElement_t*) = SortedList_insert;
  int (*delete)(SortedListElement_t*) = SortedList_delete;
  int (*length)(SortedList_t *) = SortedList_length;
  SortedListElement_t*  (*lookup)(SortedList_t *, const char *) = SortedList_lookup;
  pthread_mutex_init(&mutex, NULL);
  
  char * yieldopts = "none";
  char * syncopts = "none";
  
  static struct option l_options[] = {
    {"threads", required_argument, NULL, 't'},
    {"iterations", required_argument, NULL, 'i'},
    {"yield", required_argument, NULL, 'y'},
    {"sync", required_argument, NULL, 's'},
    {0, 0, 0, 0}
  };

  char y;
  char c;
  int len, i;
  while( (c = getopt_long(argc, argv, "t:i:", l_options, NULL)) != -1 ){
    switch(c){
    case 't':
      numThreads = atoll(optarg);
      break;
    case 'i':
      iterations = atoll(optarg);
      break;
    case 'y':
      len = strlen(optarg);
      if(len < 1 || len > 3){
	fprintf(stderr, "usage: lab2_add [--threads=NUMBER] [--iterations=NUMBER] [--yield=[idl]] [--sync=TYPE]\n");
	exit(1);
      }

      for(i = 0; i < len; i++){
	y = optarg[i];
	switch(y){
	case 'i':
	  opt_yield |= INSERT_YIELD;
	  break;
	case 'd':
	  opt_yield |= DELETE_YIELD;
	  break;
	case 'l':
	  opt_yield |= LOOKUP_YIELD;
	  break;
	default:
	  fprintf(stderr, "usage: lab2_add [--threads=NUMBER] [--iterations=NUMBER] [--yield=[idl]] [--sync=TYPE]\n");
	  exit(1);
	}
      }

      
      break;
    case 's':
      if(strlen(optarg) != 1){
	fprintf(stderr, "usage: lab2_add [--threads=NUMBER] [--iterations=NUMBER] [--yield=[idl]] [--sync=TYPE]\n");
	exit(1);
      }

      y = optarg[0];
      switch(y){
      case 'm':
	insert = mutex_insert;
	delete = mutex_delete;
	lookup = mutex_lookup;
	length = mutex_length;
	syncopts = "m";
	break;
      case 's':
	insert = spin_insert;
	delete = spin_delete;
	lookup = spin_lookup;
	length = spin_length;
	syncopts = "s";
	break;
      default:
	fprintf(stderr, "usage: lab2_add [--threads=NUMBER] [--iterations=NUMBER] [--yield=[idl]]  [--sync=TYPE]\n");
	exit(1);
	break;
	
      }
      break;
    default:
      fprintf(stderr, "usage: lab2_add [--threads=NUMBER] [--iterations=NUMBER] [--yield=[idl]] [--sync=TYPE]\n");
      exit(1);
    }
  }
  switch(opt_yield){
  case INSERT_YIELD:
    yieldopts = "i";
    break;
  case DELETE_YIELD:
    yieldopts = "d";
    break;
  case LOOKUP_YIELD:
    yieldopts = "l";
    break;
  case (INSERT_YIELD | DELETE_YIELD):
    yieldopts = "id";
    break;
  case (INSERT_YIELD | LOOKUP_YIELD):
    yieldopts = "il";
    break;
  case (DELETE_YIELD | LOOKUP_YIELD):
    yieldopts = "dl";
    break;
  case (INSERT_YIELD | DELETE_YIELD | LOOKUP_YIELD):
    yieldopts = "idl";
    break;
  

  }
  
  /* INITIALIZE LIST */
  SortedList_t list;
  list.prev = NULL;
  list.next = NULL;
  list.key = NULL;

  /* INITIALIZE NUMTHREAD * ITERATIONS NODES */
  int numNodes = numThreads * iterations;
  SortedListElement_t* nodes = (SortedListElement_t *) malloc(sizeof(SortedListElement_t) * numNodes);

  for(i = 0; i < numNodes; i++){
    nodes[i].prev = nodes[i].next = NULL;
    nodes[i].key = generate_string();
  }

  pthread_t* threads = (pthread_t *) malloc(sizeof(pthread_t) * numThreads);
  int* status = (int *) malloc(sizeof(int) * numThreads);
  signal(SIGSEGV, seg_fault);
  
  /* SET UP ARGUMENTS */
  
  argument* arg1 =  (argument *) malloc(sizeof(argument)*numThreads);
  for(i = 0; i < numThreads; i++){
    arg1[i].insert = insert;
    arg1[i].delete = delete;
    arg1[i].list = &list;
    arg1[i].length = length;
    arg1[i].lookup = lookup;
    arg1[i].nodes = nodes;
    arg1[i].start = iterations * i;
    arg1[i].end = iterations * (i + 1);
  }

  struct timespec start, end, delta;
  clock_gettime(CLOCK_REALTIME, &start);
  for(i = 0; i < numThreads; i++){
    if(pthread_create(threads + i, NULL, create_and_delete, (void*) (arg1 + i)))
      status[i] = 0;
    else
      status[i] = 1;
  }
  
  for(i = 0; i < numThreads; i++){
    if(status[i]){
      pthread_join(threads[i], NULL);
    }
  }


  clock_gettime(CLOCK_REALTIME, &end);
  clock_gettime(CLOCK_REALTIME, &delta); // Capture overhead timing for function call

    if(SortedList_length(&list)){
    fprintf(stderr, "Corrupted List. Final length nonzero.\n");
    exit(2);
  }

  long long time = diff(start, end, delta);
  long long ops = numThreads*iterations*3;
  printf("list-%s-%s,%lld,%lld,1,%lld,%lld,%lld\n", yieldopts, syncopts, numThreads, iterations, ops, time, time/ops);
   
  /* FREE NODES */
  for(i = 0; i < numNodes; i++){
    free((void*)(nodes[i].key));
  }
  free(nodes);

  
  return 0;
}
