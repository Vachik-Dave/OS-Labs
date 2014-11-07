#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <fcntl.h>

// Basic structures----------------------------------------------------------------------------------------------------------------
// Structure for a person with its details
struct Person 
{
	int p_id;			//person ID
	int from_floor, to_floor;	//floor details
	double arrival_time;
};
// Structure for an elevator and its details
struct Elevator
{
	int e_id;			//elevator ID	
	int curr_floor;
	struct Person * inside_p;	//Person in the elevator
};
// This structure is used to create Linked List of persons
struct Person_node
{
	struct Person p;
	struct Person_node * next;
	struct Person_node * prev;
};

//-----------------------------------------------------------------------------------------------------------------------------------

// Global variables in single structrure to have access of all global variables togather
struct gv
{
	int num_elevators;			//Number of elevators
	int num_floors;				//Number of floors
	int people_arrival_time;		//time gap between arrival of two consecutive person
	int elevator_speed;			//speed of an elevator in unit of (time)per floor
	int simulation_time;			//total time the simulation need to be run
	int random_seed;
	int num_people_started;			//A global counter for people started using elevator
	int num_people_finished;		//A global counter for people ended using elevator
	double starting_time;			//Starting time to be deducted from every time stamp in the simulation to get reference from starting time
	_Bool alive;				//boolean variable to specify whether simulation is alive or need to be ended
	struct Person_node * Person_list;	//Llinked list of person arrived and waiting to get service from any elevator
	pthread_mutex_t	list_mutex;		//mutex to protect global linked list of persons
	pthread_mutex_t num_people_mutex;	//mutex to protect globals counters both num_people_started & num_people_finished
	pthread_mutex_t elevator_mutex;		//mutext to protect conditional variable use for elevator
	pthread_cond_t elevator_cv;		//Conditional variable for elevator waitinng for new person to come
	
};

//-----------------------------------------------------------------------------------------------------------------------------------------

// General Functions (Random & time) used by different functions

// returns a random number in the range of 0-(n-1)
int get_rand(int n) 
{ //return range(0 to n-1) of numbers randomly 
  return  ( ((double) rand()) / RAND_MAX ) * n;
}

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

// Used for only debugging
/*
void disp()
{
	struct Person_node *temp = globals->Person_list;
	if(temp == NULL)
		printf("empty");	
	while(temp != NULL)
	{
		printf("%d--%d,%d -- %lf \n",temp->p.p_id,temp->p.from_floor,temp->p.to_floor,temp->p.arrival_time);
		temp = temp->next;
	}
	return;
}
*/

//-----------------------------------------------------------------------------------------------------------------------------------------

//-----------PERSON CREATION AND ADDITION TO LIST

// add the created person to global linked list(Person_list)
		// get parameters as new person and global structure reference 
		// and returns the arrival time of the person
double add_person(struct Person person,struct gv* globals)
{
	double return_time;
	struct Person_node * n = NULL;
	n = malloc(sizeof(struct Person_node));
	if(n == NULL)
		perror("Malloc failed!!!");
	struct Person_node * t = globals->Person_list;
	n->next = NULL;

	//copy person's details
	n->p.p_id = person.p_id;
	n->p.from_floor = person.from_floor;
	n->p.to_floor = person.to_floor;

	// lock linked list mutex befor add a new person to list
	pthread_mutex_lock(&globals->list_mutex);

	//set starting time of the person here to avoid overhead of spinloack waiting time + sleeping time in person generation thread
	return_time = get_cur_time() - globals->starting_time;
	n->p.arrival_time = return_time;
	
	// linked list insertion at last
	
	// Empty List
	if(globals->Person_list == NULL)
	{
		globals->Person_list = n;
		globals->Person_list->prev = NULL;
	}
	else	//if not empty
	{
		// travel to last node
	
		while(t->next != NULL)
		{
			t = t->next;
		}
		// last node will point to new node
		t->next = n;
		// new node previous pointer should point to previous last node
		n->prev = t;
	}


	// linked list insertion ends

	pthread_mutex_unlock(&globals->list_mutex);
	//unloack mutex

	return return_time;
}

//##############################################
// Create new person with specific ID but roandom other values of Person structure
		// Get parameters as id number and globals variable reference and 
		// returns created person(structure)
struct Person create_person(int id,struct gv * globals)
{

	struct Person p;
	//set details
	p.p_id = id;
	// select random floor as a starting floor of the person
	p.from_floor = get_rand(globals->num_floors);
	
	// repeat untill valid to(ending) floor is set
	while(1)	
	{
		int t = get_rand(globals->num_floors);

		// if starting and ending floor are not same then valid 
		if(p.from_floor != t)
		{	p.to_floor = t;
			break;
		}
	}

	return p;
}
//##############################################
// People generation Thread function
		// get parameter as reference to global variables
void* People_generation(void* glb_var) 
{
	struct gv * globals = (struct gv*)glb_var;	//pointer to global variable
	static int id = 0;				//Id for persons
	double arrival_time;				//arrival time of the person
	struct Person p;

	// If simulation is still running
	while(globals->alive)
	{
		id ++;
		p = create_person(id,globals);			//Create new person with next id
		sleep(globals->people_arrival_time);		//sleep for assuming time for arrival of next person
		arrival_time = add_person(p,globals);		//Add the generated person to global Linked list of persons
		printf("[%.04f] Person %d arrives on floor %d, waiting to go to floor %d \n",arrival_time,p.p_id,p.from_floor,p.to_floor);		

		// call signal function of conditional variable on elevator to let elevator know that new person has come to list
		pthread_mutex_lock(&globals->elevator_mutex);
		pthread_cond_signal(&globals->elevator_cv);
		pthread_mutex_unlock(&globals->elevator_mutex);

	}

	//disp();
	return NULL;
}

//---------------------------------------------------------------------------------------------------------------------------------------

//Elevator Initialization functioin
		// Get parameter as in value and returns elevator(structure)
struct Elevator Init_elevator(int  Id)
{
	struct Elevator e;
	e.e_id = Id;
	e.curr_floor = 0;			// Initial floor for any elevator is ground floor(0)
	e.inside_p = NULL;			// Initially no person inside the elevator
	
	return e;
}
//##############################################
// This function returns removes the first person from global linked list of persons
		// Get parameters as address reference to the person need to be served from list and gloabl structure reference
		// and returns 1 for sucess and 0 on failure
int get_person(struct Person * * p, struct gv* globals)
{
	// linked list removal from front

	// Empty List
	if(globals->Person_list == NULL)
	{
		// return failure
		return 0;
	}
	else	//if not empty
	{
		// get memory for new person
		(*p) = malloc(sizeof(struct Person));
		if((*p) == NULL)
			perror("Malloc failed!!!");

		// Copy person data to the person variable
		(*p)->p_id = globals->Person_list->p.p_id;
		(*p)->from_floor = globals->Person_list->p.from_floor;
		(*p)->to_floor = globals->Person_list->p.to_floor;
		(*p)->arrival_time = globals->Person_list->p.arrival_time;

		// lock global person linked list before accessing
		pthread_mutex_lock(&globals->list_mutex);
		
		// only single person in the list
		if(globals->Person_list->next == NULL)
		{	
			struct Person_node *temp;			
			
			// remove the first person_node from linked list
			temp = globals->Person_list;
			free(temp);
			globals->Person_list = NULL;
		}
		else
		{
			struct Person_node *temp;			
			// remove the first person_node from linked list
			temp = globals->Person_list;
			globals->Person_list = globals->Person_list->next;
			globals->Person_list->prev = NULL;
			free(temp);
		}
		/// unlock the mutex
		pthread_mutex_unlock(&globals->list_mutex);

		//printf("after del: ");
		//disp();
		
		//return 1 as sucess
		return 1;
	}
}
//###################################################

//This is specific function used to wait for elevator to move specific number of floors
	//Get parameters number of floor difference and refernce to globals parameters
void ele_sleep(int diff,struct gv* globals)
{
	if(diff > 0)
		sleep(diff * globals->elevator_speed);
	else
	{
		int i = 0-diff;
		sleep(i * globals->elevator_speed);
	}
	return;
}
//##################################################

// Elevator Thread Function
	// get parameter as reference to global variables
void* Elevator_work(void* glb_var) 
{
	static int count = 0;				//static varibles counts the number of elevators to give them sequencial ID's
	int t;
	struct gv* globals = (struct gv *) glb_var;	// pointer to global variable
	struct Elevator e;
	struct Person * p = NULL;
	count++;
	int ID = count;					// assign id based on counter value
	e = Init_elevator(ID);				// Initialize value to the elevator

	// If simulation is still running
	while(globals->alive)
	{
		do
		{
			// Any person is waiting or not? If person list is empty?
			while(globals->Person_list == NULL)
			{
				// Wait for a new person to come
				pthread_mutex_lock(&globals->elevator_mutex);
				pthread_cond_wait(&globals->elevator_cv, &globals->elevator_mutex);
				pthread_mutex_unlock(&globals->elevator_mutex);
			}
			// if got signal get a person
			t = get_person(&p,globals);
			//t =1;
		// if not sucessful wait again
		}while(t == 0);

		// find floor difference		
		int diff = e.curr_floor - p->from_floor;
		// current time reference to starting time of simulation
		double tm = get_cur_time() - globals->starting_time ;

		//-----------------------------
		//starts moving towards the person if it is not on the same floor
		if(diff != 0)
			printf("[%.04f] Elevator %d starts moving from %d to %d\n",tm,ID,e.curr_floor,p->from_floor);

//////////////////////////////////////////
//			printf("\n[%lf] elevator %d starts moving from floor %d to floor %d to pick up Person:%d\n",tm,ID,e.curr_floor,p->from_floor,p->p_id);
//		else
//			printf("\nelevator-%d is at floor %d itself,so picks up Person:%d\n",ID,e.curr_floor,p->p_id);
//////////////////////////////////////////

		// waiting time to reach the person's starting floor
		ele_sleep(diff,globals);

		// Reach at person's starting floor and picks up the person
		// Reaching time with reference to starting time
		tm = get_cur_time() - globals->starting_time ;

		printf("[%.04f] Elevator %d arrives at floor %d\n",tm,ID,p->from_floor,p->p_id);
		printf("[%.04f] Elevator %d picks up Person %d\n",tm,ID,p->p_id);

		//----------------------------
		//picking up process

		//lock mutex before updating shared globals num_people counters
		pthread_mutex_lock(&globals->num_people_mutex);		
		globals->num_people_started++;
		pthread_mutex_unlock(&globals->num_people_mutex);	

		// update elevator current floor
		e.curr_floor = p->from_floor;
	
		// set person inside
		e.inside_p = p;

		// find difference of current and destination floor
		diff = e.curr_floor - p->to_floor;
		// Current time with reference to starting time
		tm = get_cur_time() - globals->starting_time ;

		//----------------------------

		//starts moving towards the person's destination to_floor
		printf("\[%.04f] Elevator %d starts moving from %d to %d\n",tm,ID,e.curr_floor,p->to_floor);

//		printf("\nAt %lf elevator-%d starts moving from floor %d to floor %d to drop Person:%d\n",tm,ID,e.curr_floor,p->to_floor,p->p_id);
	
		// waiting time to reach the person's destination floor
		ele_sleep(diff,globals);

		// Reach at person's destination floor
		// get time
		tm = get_cur_time() - globals->starting_time ;
		printf("[%.04f] Elevator %d arrives at floor %d\n",tm,ID,p->to_floor);
		printf("[%.04f] Elevator %d drops Person %d\n",tm,ID,p->p_id);	
		//--------------------------

		//Dropping process
		pthread_mutex_lock(&globals->num_people_mutex);		//lock mutex before updating shared globals num_people counters
		globals->num_people_finished++;
		pthread_mutex_unlock(&globals->num_people_mutex);
		
		//Change the status of the elevator current floor		
		e.curr_floor = p->to_floor;
		
		// remove person
		p = NULL;
		e.inside_p = NULL;

	}

	return NULL;
}
//---------------------------------------------------------------------------------------------------------------------------------------

// Function to handle simulation timings
	// get global variable reference as parameter
void Simulation_timmer(struct gv* globals)
{
	sleep(globals->simulation_time);
	globals->alive = 0;
	
	return;
}
//---------------------------------------------------------------------------------------------------------------------------------------

//Main funciton
	// gets command line argumnets as mention in definition
int main(int argc, char* argv[]) 
{
	int              i,n;
	int              *tids;  
	pthread_t        *thrds;
	pthread_attr_t   *attrs;
	void             *retval;
	struct gv 	 globals;
	int 		 fd;
	//----------------------------------------
	//global initialization
	
	// Read arguments
	///*
	if(argc != 7) 
	{
    		fprintf(stderr, "missing arguments\n", argv[0]);
    		exit(1);
  	}

	
	globals.num_elevators = atoi(argv[1]);
	globals.num_floors = atoi(argv[2]);
	globals.people_arrival_time = atoi(argv[3]);
	globals.elevator_speed = atoi(argv[4]);
	globals.simulation_time = atoi(argv[5]);
	globals.random_seed = atoi(argv[6]);
	//* /

	// to copy answer to a file comment out next line
/*
	fd = open("Ele_Out.txt",O_WRONLY | O_CREAT); // open for writing 
	if(fd < 0)
	{
		printf("file can not be opened");
		return 1;
	}
	close(1);
	if(dup(fd) != 1)
	{
		perror("dup2- file rediraction :");
		return 1;
	}
//* /

/*
	globals.num_elevators = 3;
	globals.num_floors = 10;
	globals.people_arrival_time = 2;
	globals.elevator_speed = 1;
	globals.simulation_time = 30;
	globals.random_seed = 4;
//*/
	pthread_mutex_init(&globals.list_mutex, NULL);
	globals.Person_list = NULL; 				//initialization empty list
	pthread_mutex_init(&globals.num_people_mutex, NULL);
	globals.num_people_started = 0;
	globals.num_people_finished = 0;
	globals.alive = 1;

	srand(globals.random_seed);

	pthread_mutex_init(&globals.elevator_mutex, NULL);
	pthread_cond_init (&globals.elevator_cv, NULL);

	globals.starting_time = get_cur_time();

	//globals initializations are ended
	
	//globals error cheking of inputs
	if(globals.num_elevators <= 0)	
	{
		printf("number of elevators must be more than 0\n");
		return 1;
	}
	if(globals.num_floors <= 0)	
	{
		printf("number of floors must be more than 0\n");
		return 1;
	}
	if(globals.people_arrival_time <= 0)	
	{
		printf("people arrival time must be more than 0\n");
		return 1;
	}
	if(globals.elevator_speed <= 0)	
	{
		printf("elevator speed must be more than 0/floor\n");
		return 1;
	}
	if(globals.simulation_time <= 0)	
	{
		printf("total simulation time must be more than 0\n");
		return 1;
	}
	//--------------------------------------

	// threads for each elevator + one for person generation
	n = globals.num_elevators + 1 ;
	thrds = (pthread_t*) malloc(sizeof(pthread_t)*n);
	if(thrds == NULL)
		perror("Malloc failed!!!");
	attrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t)*n);
	if(attrs == NULL)
		perror("Malloc failed!!!");
	
	/*tids  = (int*) malloc(sizeof(int)*n);
	if(tids == NULL)
		perror("Malloc failed!!!");
*/
  	// create elevator threads
  	for(i = 0; i < n; i++) 
	{
		// Initialize pthread attributes
		if(pthread_attr_init(attrs+i)) 
		{
	        	perror("attr_init()");
    		}
		if(i == 0)
		{
			// First Thread for people generation 
			if(pthread_create(thrds+i, attrs+i, People_generation, &globals) != 0) 
			{
				perror("pthread_create()");
				exit(1);
			}			
		}
		else
		{
			// Threads for elevator generation and handling
			//tids[i] = i;
			if(pthread_create(thrds+i, attrs+i, Elevator_work, &globals) != 0) 
			{
		      		perror("pthread_create()");
		      		exit(1);
	    		}
		}
  	}

	// Call simulation timmer to let the main thread to sleep for simulation timing
	Simulation_timmer(&globals);

	// End the threads, intterupt pthread_cond_wait and exit all threads, 
	// and clear(free) all variables using pthread_cleanup_push()
	for(i = 0; i < n; i++) 
	{ 
		pthread_cancel(thrds[i]);
	}
	//end of the execution time
	double t = get_cur_time();
	
	printf("Simulation result: %d prople have started, %d people have finished during %d seconds\n",globals.num_people_started,globals.num_people_finished,globals.simulation_time);	

	// Freeing mutexes
	pthread_mutex_destroy(&globals.list_mutex);
	pthread_mutex_destroy(&globals.num_people_mutex);
	pthread_mutex_destroy(&globals.elevator_mutex);
	pthread_cond_destroy(&globals.elevator_cv);

	// freeing other variables	
	free(attrs);
	free(thrds);
//	free(tids);

//	close(fd);

	return 0;
}

