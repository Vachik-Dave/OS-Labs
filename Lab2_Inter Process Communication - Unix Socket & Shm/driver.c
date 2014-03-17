#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

// Size of ring buffer
#define BUFFER_SIZE  (1)

//Total number of messages to be send
#define no_msges 10000000

// structure required to implement ring buffer
// Named as Commond area as it is being shared by both processes
struct CommonArea 
{
	int buffer[BUFFER_SIZE];
	int head;
	int tail;
};

//returning current time in double type
double get_cur_time() 
{
  	struct timeval   tv;
  	struct timezone  tz;
  	double cur_time;
	
	//getting current time
  	gettimeofday(&tv, &tz);

	//calculate curret time in seconds
  	cur_time = tv.tv_sec + tv.tv_usec / 1000000.0;

  	return cur_time;
}

// returns a random number in the range
int get_rand(int n) 
{ //return range(0 to n-1) of numbers randomly 
  return  ( ((double) rand()) / RAND_MAX ) * n;
}

//It creates shared memory and assign it to global structure variable 
struct CommonArea* create_global_common_area(size_t size, int* shm_id) 
{
	// Key is common for both driver and application process
  	key_t               key = 2313;
  	void               *shm;
  	struct CommonArea  *g_area;
	//here application process just need to get shared memory segment created by application process
  	int                 shm_flg = 0;

	// Here, it'll return segment ID, which referes shared memory segment created and shared by application process
  	if ((*shm_id = shmget(key, sizeof(*g_area), shm_flg)) < 0) 
	{
    		printf("Error: shmget() failed\n");
    		exit(1);
  	}

	// It'll attache the shared memory segment to driver process memory
  	if ((shm = shmat(*shm_id, NULL, 0)) == (void *) -1) 
	{
    		printf("Error: shmat() failed\n");
    		exit(1);
  	}
	
	//assigning the shared memory to the structure
  	g_area = (struct CommonArea*) shm; 

  	return g_area;
}

void driver_process(struct CommonArea* g_area) 
{
	double num, sum = 0,start_time;
	int i=0;

	// give initial seed for random number generator
	srand(time(NULL));

	// note the starting time	
	start_time = get_cur_time();

	// Repeate the sending message for no_msges times + send starting time as last message
	while(i < no_msges + 1)
	{
		// send starting time as last number
		if(i == no_msges)
			num = start_time;
		else
			num = get_rand(100);
		while(((g_area->head + 1) % BUFFER_SIZE) == g_area->tail); 
		/*
		Buffer is full, so busy waiting
		*/
		// write to buffer at head posion 
		g_area->buffer[g_area->head] = num;

		// increament head after  
		g_area->head = (g_area->head+ 1) % BUFFER_SIZE;

		// last message should not be added to sum
		if(i == no_msges)
			start_time = num;
		else
			sum = sum + num;
		i++;
	}
	
	printf("Driver Process Total = %lf \n",sum);
}

int main(int argc, char* argv[]) 
{
	int status;
	struct CommonArea* g_area;
	int shm_id;

	g_area = create_global_common_area(sizeof(*g_area), &shm_id);

	//initially set head and tail as 0
	g_area->head = 0;
	g_area->tail = 0;

	//driver process
	driver_process(g_area);
    	
	// detache the shared memory segment from this process memory after work done
	if (shmdt(g_area) == -1) 
	{
	    	printf("Driver: Can't detach memory segment\n");
	      	exit(1);
    	}

  return 0;
}

