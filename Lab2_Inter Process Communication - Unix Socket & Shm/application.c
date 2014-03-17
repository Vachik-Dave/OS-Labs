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


//It creates shared memory and assign it to global structure variable 
struct CommonArea* create_global_common_area(size_t size, int* shm_id) 
{
	// Key is common for both driver and application process
  	key_t               key = 2313;
  	void               *shm;
  	struct CommonArea  *g_area;
  	int                 shm_flg = 0;
	
	// application process has to create share memory segment, so it se its flag as create with both reading & writing permissions.
  	shm_flg = IPC_CREAT | 0666;
  	
	// here it will create new share memory segment and return segment ID
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


void application_process(struct CommonArea* g_area) 
{
	// Here I was creating local g_area without assigning any memory to it.
	// I really dont know why It was wrking with my machine, it shouldnt work.
	// But I hope this code will work properly.

	//struct CommonArea  *g_area;
//--------------------------------------------------------------------------------------
	int shm_id,i=0,temp;
	double num, sum =0,start_time, end_time;
	
	while(i < no_msges + 1)
	{	
		while (g_area->head == g_area->tail);
		/* Buffer is empty, so busy waiting
		*/

		// read value from shared buffer at tail position
		temp = g_area->tail;
		num = g_area->buffer[temp];
		
		//increament tail
		g_area->tail = (g_area->tail+ 1) % BUFFER_SIZE;

		// receive starting time as last number, so dont need to add to sum
		if(i == no_msges)
			start_time = num;
		else
			sum = sum + num;
		i++;
  	}

	//get current time as ending time
	end_time = get_cur_time();

	printf("Application Process Total = %lf\n",sum);

	printf("Total time for 10 million (+1) messages = %g Seconds \n",end_time - start_time);

}

int main(int argc, char* argv[]) 
{
	int status;
	struct CommonArea* g_area;	
	int shm_id;
	

	g_area = create_global_common_area(sizeof(*g_area),&shm_id);

	//initially set head and tail as 0
	g_area->head = 0;
	g_area->tail = 0;

	//application process
	application_process(g_area);

	// detach the shared memory segment from current process memory 
	if(shmdt(g_area) == -1) 
	{
		printf("application: Can't detach memory segment\n");
		exit(1);
	}

	// Remove the shared memory segment
	if (shmctl(shm_id, IPC_RMID, NULL)) 
	{
	    	printf("Driver: Can't remove memory segment\n");
	      	exit(1);
	}
  	
  return 0;
}

