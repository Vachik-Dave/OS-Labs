#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

int mutex =1;
int emptyC;
int fullC = 0;
int Total_no_items2Pro;
int Max_sleep_sec;
int Pro_count = 0;
int Con_count = 0;
int Buffer_size;

// Global structure used for ring buffer
struct CommonArea 
{
	int * buffer;
	int head;
	int tail;
};

// Global variable for ring buffer
struct CommonArea* g_area;

//------------------------------------------------------------------------
// Semaphore Implementation Started

pthread_mutex_t	PV_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	PV_CV = PTHREAD_COND_INITIALIZER;

//Wait function P

P(int * S)
{
	pthread_mutex_lock(&PV_mutex);
	if(*S <= 0)
	{
		//printf("\nwaiting...");
		pthread_cond_wait(&PV_CV, &PV_mutex);
		//printf("\nget Signal...");
	}
	--(*S);
	pthread_mutex_unlock(&PV_mutex);
}

//Signal function S

V(int * S)
{
	pthread_mutex_lock(&PV_mutex);
	++(*S);
	pthread_cond_signal(&PV_CV);
	pthread_mutex_unlock(&PV_mutex);
}

// Semaphore Implementation Ended
//------------------------------------------------------------------------

// returns a random number in the range of 0-(n-1)
int get_rand(int n) 
{ //return range(0 to n-1) of numbers randomly 
  return  ( ((double) rand()) / RAND_MAX ) * n;
}

//------------------------------------------------------------------------
// Producer Thread function
void* Producer(void* lv) 
{

	int num = 0, count =0,t;	
	int* ID = (int *) lv;

	// print every thread starting message
	printf("\nProducer No %d : Started working\n",*ID,count);
	
	// give initial seed for random number generator
	srand(time(NULL));
	
	while(1)
	{
		// random number to add in buffer
		num = get_rand(100);
				
		// randome time sleeping
		t = get_rand(Max_sleep_sec);
		sleep(t);

		P(&emptyC);
		P(&mutex);

		// critical section Starts-----------------------------------

		// write to buffer at head posion 
		g_area->buffer[g_area->head] = num;
		
		// increament head after  
		g_area->head = (g_area->head+ 1) % Buffer_size;
		
		// Producer count increment by 1
		Pro_count = Pro_count + 1;

		if(Pro_count >= Total_no_items2Pro+1)
		// critical section Ends -------------------------------------
		{
			V(&mutex);
			V(&fullC);
			break;
		}
		else
		{
			V(&mutex);
			V(&fullC);
		}

		// thread local counter
		count++;
	}

	// print every thread local count
	printf("\nProducer No %d : Production count %d\n",*ID,count);
	
	return NULL;
}

//-------------------------------------------------------------------------
// Consumer Thread Function
void* Consumer(void* lv) 
{
	int num = 0, count =0,t;	
	int* ID = (int *) lv;	

	// print every thread starting message
	printf("\nConsumer No %d : Started working\n",*ID,count);
	
	
	// give initial seed for random number generator
	srand(time(NULL));

	while(1)
	{
		P(&fullC);
		P(&mutex);
		// critical section Starts-----------------------------------

		// read value from shared buffer at tail position
		num = g_area->buffer[g_area->tail];
		
		//increament tail
		g_area->tail = (g_area->tail+ 1) % Buffer_size;
		
		// Producer count increment by 1
		Con_count = Con_count + 1;

		if(Con_count >= Total_no_items2Pro+1)
		// critical section Ends -------------------------------------
		{
			V(&mutex);
			V(&emptyC);
			break;
		}
		else
		{
			V(&mutex);
			V(&emptyC);
		}

		// randome time sleeping
		t = get_rand(Max_sleep_sec);
		sleep(t);

		// thread local counter
		count++;		
	}
	
	// print every thread local count
	printf("\nConsumer No %d : Consumption count %d\n",*ID,count);

	return NULL;
}

//------------------------------------------------------------------------------

int main(int argc, char* argv[]) 
{
	int              i, n, no_producer, no_consumer;
	int              *tids;  
	pthread_t        *thrds;
	pthread_attr_t   *attrs;
	void             *retval;

	
	if(argc != 6) 
	{
    		fprintf(stderr, "missing arguments\n", argv[0]);
    		exit(1);
  	}
	
	no_producer = atoi(argv[1]);
	no_consumer = atoi(argv[2]);
	Max_sleep_sec = atoi(argv[3]);
	Total_no_items2Pro = atoi(argv[4]);
	Buffer_size = atoi(argv[5]);
	emptyC = Buffer_size;
	
	/*
	no_producer = 4;
	no_consumer = 4;
	Max_sleep_sec = 4;
	Total_no_items2Pro = 50;
	Buffer_size = 128;
	emptyC = Buffer_size;
	*/

	g_area = malloc(sizeof(struct CommonArea));
	g_area->buffer = calloc(Buffer_size,sizeof(int));

	n = no_producer + no_consumer;
	thrds = (pthread_t*) malloc(sizeof(pthread_t)*n);
	attrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t)*n);
	tids  = (int*) malloc(sizeof(int)*n);

  	// create threads
  	for(i = 0; i < n; i++) 
	{
		if(pthread_attr_init(attrs+i)) 
		{
	        	perror("attr_init()");
    		}

		if(i < no_producer)
		{
			tids[i] = i+1;

			if(pthread_create(thrds+i, attrs+i, Producer, tids+i) != 0) 
			{
		      		perror("pthread_create()");
		      		exit(1);
	    		}
		}
		else
		{
			tids[i] = i - no_producer +1 ;
			if(pthread_create(thrds+i, attrs+i, Consumer, tids+i) != 0) 
			{
	      			perror("pthread_create()");
	      			exit(1);
	    		}
		}
  	}

	// join threads
	for(i = 0; i < n; i++) 
	{ 
		pthread_join(thrds[i], &retval);
	}


	free(attrs);
	free(thrds);
	free(tids);

	return 0;
  	//pthread_exit(&retval);
}

