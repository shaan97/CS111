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
#include "hash.h"


#define KEEP_LOCK 0x1
#define LOCK_OWNED 0x2

int opt_yield = 0;
long long numLists = 1, iterations = 1;
pthread_mutex_t* mutex;
long long * lock_time;

long long diff(struct timespec start, struct timespec end, struct timespec delta);

void std_insert(SortedList_t * list, SortedListElement_t * element, int index){
  SortedList_insert(list + (hash((unsigned char *) element->key) % numLists), element);
}

SortedListElement_t * std_lookup(SortedList_t * list, const char* key, int lock_info, int index){
  return SortedList_lookup(list + (hash((unsigned char *) key) % numLists), key);
}

int std_length(SortedList_t * list, int index){
  int i, count = 0, temp;
  for(i = 0; i < numLists; i++){
    temp = SortedList_length(list + i);
    if(temp == -1){
      fprintf(stderr, "List corruption detected during length analysis.\n");
      exit(2);
    }
    count += temp;
  }
}

int std_delete(SortedListElement_t * element, int lock_info, int index){
  return SortedList_delete(element);
}

void mutex_insert(SortedList_t * list, SortedListElement_t * element, int i){
  long long index = hash((unsigned char *) element->key) % numLists;
  struct timespec start, end, delta;
  clock_gettime(CLOCK_REALTIME, &start);
  pthread_mutex_lock(mutex + index);
  clock_gettime(CLOCK_REALTIME, &end);
  clock_gettime(CLOCK_REALTIME, &delta);
  lock_time[i] += diff(start, end, delta);

  SortedList_insert(list + index, element);
  pthread_mutex_unlock(mutex + index);
}

SortedListElement_t* mutex_lookup(SortedList_t * list, const char *key, int lock_info, int i){
  long long index = hash((unsigned char *) key) % numLists;
  if(~lock_info & LOCK_OWNED){
    struct timespec start, end, delta;
    clock_gettime(CLOCK_REALTIME, &start);
    pthread_mutex_lock(mutex + index);
    clock_gettime(CLOCK_REALTIME, &end);
    clock_gettime(CLOCK_REALTIME, &delta);
    lock_time[i] += diff(start, end, delta);
  }
  SortedListElement_t * x = SortedList_lookup(list + index, key);
  if(~lock_info & KEEP_LOCK)
    pthread_mutex_unlock(mutex + index);
  return x;
}

int mutex_length(SortedList_t * list, int i){
  /* TODO : Pursue other locks if lock cannot be obtained */
  int count = 0, index, temp;
  for(index = 0; index < numLists; index++){
    struct timespec start, end, delta;
    clock_gettime(CLOCK_REALTIME, &start);
    pthread_mutex_lock(mutex + index);
    clock_gettime(CLOCK_REALTIME, &end);
    clock_gettime(CLOCK_REALTIME, &delta);
    lock_time[i] += diff(start, end, delta);
    
    temp = SortedList_length(list + index);
    if(temp == -1){
      fprintf(stderr, "List corruption detected during length analysis.\n");
      exit(2);
    }
    count += temp;
    pthread_mutex_unlock(mutex + index);
  }
  return count;
}

int mutex_delete(SortedListElement_t * element, int lock_info, int i){
  long long index = hash((unsigned char *)element->key) % numLists;
  if(~lock_info & LOCK_OWNED){
    struct timespec start, end, delta;
    clock_gettime(CLOCK_REALTIME, &start);
    pthread_mutex_lock(mutex + index);
    clock_gettime(CLOCK_REALTIME, &end);
    clock_gettime(CLOCK_REALTIME, &delta);
    lock_time[i] += diff(start, end, delta);
  }
  
  int val = SortedList_delete(element);
  if(~lock_info & KEEP_LOCK)
    pthread_mutex_unlock(mutex + index);
  return val;
}

int* s_flag;
void spin_insert(SortedList_t * list, SortedListElement_t * element, int i){
  long long index = hash((unsigned char *)element->key) % numLists;

  struct timespec start, end, delta;
  clock_gettime(CLOCK_REALTIME, &start);
  while(__sync_lock_test_and_set(s_flag + index, 1))
    ;
  clock_gettime(CLOCK_REALTIME, &end);
  clock_gettime(CLOCK_REALTIME, &delta);
  lock_time[i] += diff(start, end, delta);

  SortedList_insert(list + index, element);
  __sync_lock_release(s_flag + index);
}

int spin_delete(SortedListElement_t * element, int lock_info, int i){
  long long index = hash((unsigned char *)element->key) % numLists;
  if(~lock_info & LOCK_OWNED){
    struct timespec start, end, delta;
    clock_gettime(CLOCK_REALTIME, &start);
    while(__sync_lock_test_and_set(s_flag + index, 1))
      ;
    clock_gettime(CLOCK_REALTIME, &end);
    clock_gettime(CLOCK_REALTIME, &delta);
    lock_time[i] += diff(start, end, delta);
  }
  int val = SortedList_delete(element);
  if(~lock_info & KEEP_LOCK)
    __sync_lock_release(s_flag + index);
  return val;
}

SortedListElement_t* spin_lookup(SortedList_t * list, const char *key, int lock_info, int i){
  long long index = hash((unsigned char *)key) % numLists;
  if(~lock_info & LOCK_OWNED){
    struct timespec start, end, delta;
    clock_gettime(CLOCK_REALTIME, &start);
    while(__sync_lock_test_and_set(s_flag + index, 1))
      ;
    clock_gettime(CLOCK_REALTIME, &end);
    clock_gettime(CLOCK_REALTIME, &delta);
    lock_time[i] += diff(start, end, delta);
  }
  SortedListElement_t * x = SortedList_lookup(list + index, key);
  if(~lock_info & KEEP_LOCK)
    __sync_lock_release(s_flag + index);
  return x;
}

int spin_length(SortedList_t * list, int i){
  int count = 0, index, temp;
  /* TODO : Pursue other locks if lock cannot be obtained */
  for(index = 0; index < numLists; index++){
    struct timespec start, end, delta;
    clock_gettime(CLOCK_REALTIME, &start);
    while(__sync_lock_test_and_set(s_flag + index, 1))
      ;
    clock_gettime(CLOCK_REALTIME, &end);
    clock_gettime(CLOCK_REALTIME, &delta);
    lock_time[i] += diff(start, end, delta);
    
    temp = SortedList_length(list + index);
    if(temp == -1){
      fprintf(stderr, "List corruption detected during length analysis.\n");
      exit(2);
    }
    count += temp;
    __sync_lock_release(s_flag + index);
  }
  return count;
}

long long diff(struct timespec start, struct timespec end, struct timespec delta){
  long long d1 = 0, d2 = 0;
  if(end.tv_nsec < start.tv_nsec){
    d1 = 1e9;
  }

  if(delta.tv_nsec < end.tv_nsec )
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
  void (*insert)(SortedList_t*, SortedListElement_t*, int);
  int (*delete)(SortedListElement_t*, int, int);
  int (*length)(SortedList_t *, int);
  SortedListElement_t* (*lookup)(SortedList_t *, const char*, int, int);
  int start, end;
} argument;
  
int atomic_look_delete(SortedList_t* list, const char* key, SortedListElement_t* (*lookup)(SortedList_t*, const char*, int, int), int (*delete)(SortedListElement_t*, int, int), int index){
  SortedListElement_t * temp = lookup(list, key, KEEP_LOCK, index);
  if(temp == NULL)
    return -2;
  return delete(temp, LOCK_OWNED, index);

}
void* create_and_delete(void* arg){
  argument* a = (argument *) arg;
  int start = a->start;
  int end = a->end;
  int index = start / iterations;
  int i;
  for(i = start; i < end; i++){
    a->insert(a->list, &(a->nodes[i]), index);
    
  }
  if(a->length(a->list, index) == -1){
    fprintf(stderr, "List was corrupted. Length analysis showed corruption.\n");
    exit(2);
  }
  for(i = start; i < end; i++){
    int val;

    if(val = atomic_look_delete(a->list, a->nodes[i].key, a->lookup, a->delete, index)){//(temp = a->lookup(a->list, a->nodes[i].key)) == NULL  || a->delete(temp) == 1){
      if(val == -2)
	fprintf(stderr, "List was corrupted. Lookup analysis showed corruption.\n");
      else
	fprintf(stderr, "List was corrupted. Deletion analysis showed corruption. %d\n", i - start);
      exit(2);
    }

    
  }
}

int main(int argc, char** argv){
  srand(time(NULL));
  long long numThreads = 1; /* COMMAND LINE ARGUMENT */
  
  void (*insert)(SortedList_t*, SortedListElement_t*, int) = std_insert;
  int (*delete)(SortedListElement_t*, int, int) = std_delete;
  int (*length)(SortedList_t *, int) = std_length;
  SortedListElement_t*  (*lookup)(SortedList_t *, const char *, int, int) = std_lookup;


  
  char * yieldopts = "none";
  char * syncopts = "none";
  
  static struct option l_options[] = {
    {"threads", required_argument, NULL, 't'},
    {"iterations", required_argument, NULL, 'i'},
    {"yield", required_argument, NULL, 'y'},
    {"sync", required_argument, NULL, 's'},
    {"lists", required_argument, NULL, 'l'},
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
	fprintf(stderr, "usage: lab2_add [--threads=NUMBER] [--iterations=NUMBER] [--yield=[idl]] [--sync=TYPE] [--lists=NUMBER]\n");
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
	  fprintf(stderr, "usage: lab2_add [--threads=NUMBER] [--iterations=NUMBER] [--yield=[idl]] [--sync=TYPE] [--lists=NUMBER]\n");
	  exit(1);
	}
      }

      
      break;
    case 's':
      if(strlen(optarg) != 1){
	fprintf(stderr, "usage: lab2_add [--threads=NUMBER] [--iterations=NUMBER] [--yield=[idl]] [--sync=TYPE] [--lists=NUMBER]\n");
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
	fprintf(stderr, "usage: lab2_add [--threads=NUMBER] [--iterations=NUMBER] [--yield=[idl]]  [--sync=TYPE] [--lists=NUMBER]\n");
	exit(1);
	break;
	
      }
      break;
    case 'l':
      numLists = atoll(optarg);
      break;
    default:
      fprintf(stderr, "usage: lab2_add [--threads=NUMBER] [--iterations=NUMBER] [--yield=[idl]] [--sync=TYPE]  [--lists=NUMBER]\n");
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
  lock_time = (long long *) calloc(numThreads, sizeof(long long));
  mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t) * numLists);
  s_flag = (int *) malloc(sizeof(int) * numLists);
  
  
  /* INITIALIZE LIST(S) */
  SortedList_t* list = (SortedList_t*) malloc(sizeof(SortedList_t) * numLists);
  for(i = 0; i < numLists; i++){
    pthread_mutex_init(mutex + i, NULL);

    list[i].prev = NULL;
    list[i].next = NULL;
    list[i].key = NULL;

    s_flag[i] = 0;
  }
  
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
    arg1[i].list = list;
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

  int count = 0;
  for(i = 0; i < numLists; i++){
    count += SortedList_length(list + i);
  }
  
  if(count){
    fprintf(stderr, "Corrupted List. Final length nonzero.\n");
    exit(2);
  }

  long long time = diff(start, end, delta);
  long long ops = numThreads*iterations*3;
  long long lock_total = 0;
  for(i = 0; i < numThreads; i++){
    lock_total += lock_time[i];
  }
  printf("list-%s-%s,%lld,%lld,%lld,%lld,%lld,%lld,%lld\n", yieldopts, syncopts, numThreads, iterations, numLists, ops, time, time/ops, lock_total / ops);
   
  /* FREE NODES */
  for(i = 0; i < numNodes; i++){
    free((void*)(nodes[i].key));
  }
  free(nodes);

  
  return 0;
}
