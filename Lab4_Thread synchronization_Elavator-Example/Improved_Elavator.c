#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

// Basic structures----------------------------------------------------------------------------------------------------------------
// Structure for a person with its details
struct Person 
{
	int p_id;
	int from_floor, to_floor;
	double arrival_time;
};
// Structure for an elevator and its details
struct Elevator
{
	int e_id;
	int curr_floor;
	int targeted_floor;			// it mentions the targeted floor,if elevator already planned to serve specific person
	int direction;				//added
	int status;				// 0 idle - 1 moving
	struct Person_node * inside_p; 		//changed
};
// This structure is used to create Linked List of persons
struct Person_node
{
	struct Person p;
	struct Person_node * next;
	struct Person_node * prev;
};

//------------------------------------------------------------------------

// Global variables in single structrure to have access of all global variables togather
struct gv
{
	int num_elevators;
	int num_floors;
	int people_arrival_time;
	int elevator_speed;
	int simulation_time;
	int random_seed;
	int num_people_started;
	int num_people_finished;
	double starting_time;
	_Bool alive;
	struct Person_node * Up_Person_list;
	struct Person_node * Down_Person_list;	
	pthread_mutex_t	Up_list_mutex;
	pthread_mutex_t Down_list_mutex;
	pthread_mutex_t num_people_mutex;
	pthread_mutex_t elevator_mutex;
	pthread_cond_t elevator_cv;
	
};

//------------------------------------------------------------------------

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
void disp(struct gv * globals)
{
	struct Person_node *temp = globals->Down_Person_list;
	printf("Down List:\n");
	if(temp == NULL)
		printf("Down-empty");
	while(temp != NULL)
	{
		printf("%d--%d,%d -- %lf \n",temp->p.p_id,temp->p.from_floor,temp->p.to_floor,temp->p.arrival_time);
		temp = temp->next;
	}
	struct Person_node *temp1 = globals->Up_Person_list;
	printf("Up List:\n");
	if(temp1 == NULL)
		printf("Up-empty");	
	while(temp1 != NULL)
	{
		printf("%d--%d,%d -- %lf \n",temp1->p.p_id,temp1->p.from_floor,temp1->p.to_floor,temp1->p.arrival_time);
		temp1 = temp1->next;
	}
	return;
}
*/
//------------------------------------------------------------------------

//-----------PERSON CREATION AND ADDITION TO LIST


// add the created person to global linked list(Person_list)
		// get parameters as new person and global structure reference 
		// and returns the arrival time of the person
double add_person(struct Person person, struct gv * globals)
{
	double return_time;
	int direction;
	struct Person_node * n = NULL;
	n = malloc(sizeof(struct Person_node));
	if(n == NULL)
		printf("malloc failed!!!\n");
	struct Person_node * t ;
	n->next = NULL;
	
	//copy person's details
	n->p.p_id = person.p_id;
	n->p.from_floor = person.from_floor;
	n->p.to_floor = person.to_floor;

	direction = person.to_floor - person.from_floor;		//if +ve going up else down

	// Ceck direction the person wants to go and lock corresponding lock
	if(direction > 0)
	{
		t = globals->Up_Person_list;
		pthread_mutex_lock(&globals->Up_list_mutex);
	}
	else
	{
		t = globals->Down_Person_list;
		pthread_mutex_lock(&globals->Down_list_mutex);
	}
		//set starting time of the person here to avoid overhead of spinloack waiting time + sleeping time in person generation thread
	
		return_time = get_cur_time() - globals->starting_time;
		n->p.arrival_time = return_time;
	
	// linked list insertion at last
	
		// Empty List
		if(t == NULL)
		{
			if(direction > 0)
				//printf("U:");
				globals->Up_Person_list = n;
			else
				globals->Down_Person_list = n;
			n->prev = NULL;
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

	if(direction > 0)
		pthread_mutex_unlock(&globals->Up_list_mutex);
	else
		pthread_mutex_unlock(&globals->Down_list_mutex);
	//disp(globals);	

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
	static int id = 0;	
	double arrival_time;				//arrival time of the person
	struct gv* globals = (struct gv*)glb_var;
	struct Person p;

	while(globals->alive)
	{
		id ++;
		p = create_person(id,globals);
		sleep(globals->people_arrival_time);
		arrival_time = add_person(p,globals);
		printf("[%.04f] Person %d arrives on floor %d, waiting to go to floor %d \n",arrival_time,p.p_id,p.from_floor,p.to_floor);		

		// call signal function of conditional variable on elevator to let elevator know that new person has come to list
		pthread_mutex_lock(&globals->elevator_mutex);
		pthread_cond_signal(&globals->elevator_cv);
		pthread_mutex_unlock(&globals->elevator_mutex);

	}

	return NULL;
}
//---------------------------------------------------------------------------------------------------------------------------------------

//Elevator Initialization functioin
		// Get parameter as in value and returns elevator(structure)
struct Elevator Init_elevator(int * Id)
{
	struct Elevator e;
	e.e_id = *Id;
	e.curr_floor = 0;			// Initial floor for any elevator is ground floor(0)
	e.targeted_floor = 0;			// Initially elevtor in not targeting any person
	e.direction = 1;			// from ground floor elevator must go up(1)
	e.status = 0;				// elevator is idle
	e.inside_p = NULL;
	
	return e;
}

//##############################################
// This function returns removes the first person from global linked list of persons
		// Get parameters as address reference to the person need to be served from list and gloabl structure reference
		// and returns 1 for sucess and 0 on failure
int get_person(struct Person * * p,struct gv * globals)
{
	struct Person_node *temp;
	int dir;
	// linked list removal from front
	//printf("here");
	// Empty List
	if(globals->Up_Person_list == NULL && globals->Down_Person_list == NULL)
	{
		// return failure
		return 0;
	}
	else if(globals->Up_Person_list != NULL)	//if Up_list is not empty
	{	
		dir = 1;
//		temp = globals->Up_Person_list;
		
		// lock before accessing global shared variable	
		pthread_mutex_lock(&globals->Up_list_mutex);
		//printf("1-S\n");
		(*p) = malloc(sizeof(struct Person));
		if((*p) == NULL)
			perror("Malloc failed!!!");

		// Copy person data to pointer variable
		(*p)->p_id = globals->Up_Person_list->p.p_id;
		(*p)->from_floor = globals->Up_Person_list->p.from_floor;
		(*p)->to_floor = globals->Up_Person_list->p.to_floor;
		(*p)->arrival_time = globals->Up_Person_list->p.arrival_time;

		
		// Single element in the list
		if(globals->Up_Person_list->next == NULL)
		{	
			struct Person_node *t;			
			// remove the first person_node from linked list
			t = globals->Up_Person_list;
			free(t);
			globals->Up_Person_list = NULL;
		}
		else		
		{
			struct Person_node *t;			
			// remove the first person_node from linked list
			t = globals->Up_Person_list;
			globals->Up_Person_list = globals->Up_Person_list->next;
			globals->Up_Person_list->prev = NULL;
			free(t);
		}
		/// unlock the mutex
		//printf("1-E\n");
		pthread_mutex_unlock(&globals->Up_list_mutex);
	}
	else
	{
		dir = -1;
//		temp = globals->Down_Person_list;

		(*p) = malloc(sizeof(struct Person));
		if((*p) == NULL)
			perror("Malloc failed!!!");

		// lock before accessing global shared variable
		pthread_mutex_lock(&globals->Down_list_mutex);

		// Copy person data to pointer variable
		(*p)->p_id = globals->Down_Person_list->p.p_id;
		(*p)->from_floor = globals->Down_Person_list->p.from_floor;
		(*p)->to_floor = globals->Down_Person_list->p.to_floor;
		(*p)->arrival_time = globals->Down_Person_list->p.arrival_time;
	
		// Single element in the list
		if(globals->Down_Person_list->next == NULL)
		{	
			struct Person_node *t;			
			// remove the first person_node from linked list
			t = globals->Down_Person_list;
			free(t);
			globals->Down_Person_list = NULL;
		}
		else		
		{
			struct Person_node *t;			
			// remove the first person_node from linked list
			t = globals->Down_Person_list;
			globals->Down_Person_list = globals->Down_Person_list->next;
			globals->Down_Person_list->prev = NULL;
			free(t);
		}
	


		/// unlock the mutex
		pthread_mutex_unlock(&globals->Down_list_mutex);
	}

		//return 1 as sucess
		return 1;

}
//###################################################
//This is specific function used to find any person on the current floor and wants to go in spesific direction
		// Get parameters as address reference to the person need to be served from Up/Down list, current floor, direction of the 			elevator, if the elevator is targeted towards any specific floor(yes/no) and gloabl structure reference
		// and returns 1 for sucess(if find person) and 0 on failure
int Find_Pickup_person(struct Person * * p, int curr_floor, int dir,int targeted_floor,struct gv * globals)
{
	struct Person_node *temp = NULL;
	int flag =0;	

	if(dir > 0)		//up direction
	{
		temp = globals->Up_Person_list;
		// lock before accessing global shared variable
		pthread_mutex_lock(&globals->Up_list_mutex);
		// Empty List
		if(globals->Up_Person_list == NULL)
		{
			/// unlock the mutex
			pthread_mutex_unlock(&globals->Up_list_mutex);

			return 0;
		}
		else	//if not empty
		{
			do
			{
				flag = 0;
				while(temp != NULL && temp->p.from_floor != curr_floor)
				{	
					temp = temp->next;
				}
				// person not found
				if(temp == NULL)
				{
					/// unlock the mutex
					pthread_mutex_unlock(&globals->Up_list_mutex);
					return 0;
				}
		
				if(targeted_floor != -1)			// not specific targeted floor
				{
					if(dir > 0)
					{
						// can not pick up persons, who wants to go above targeted floor
						if(temp->p.to_floor > targeted_floor)
						{	temp = temp->next;
							flag =1;
						}
					}
					else
					{
						// can not pick up persons, who wants to go below targeted floor
						if(temp->p.to_floor < targeted_floor)
						{	temp = temp->next;
							flag =1;
						}
					}
				}
			}while(flag);		// if flag is true try to find next person

			//if temp != NULl means found matching person at specific floor
			(*p) = malloc(sizeof(struct Person));
			// Copy person data to pointer variable
			(*p)->p_id = temp->p.p_id;
			(*p)->from_floor =temp->p.from_floor;
			(*p)->to_floor = temp->p.to_floor;
			(*p)->arrival_time = temp->p.arrival_time;		
		
			// remove the person from linked list
			if(globals->Up_Person_list == temp)
				globals->Up_Person_list == NULL;
			//if temp is not last node
			if(temp->next != NULL)
				temp->next->prev = temp->prev;
			//if temp is first node				
			if(temp->prev == NULL)		
			{
				globals->Up_Person_list = temp->next;	
			}
			else
				temp->prev->next = temp->next;
		
			free(temp);
		}

		/// unlock the mutex
		pthread_mutex_unlock(&globals->Up_list_mutex);
	}
	else			//Down direction
	{
		temp = globals->Down_Person_list;
		// lock before accessing global shared variable
		pthread_mutex_lock(&globals->Down_list_mutex);	

		// Empty List
		if(globals->Down_Person_list == NULL)
		{
			/// unlock the mutex
			pthread_mutex_unlock(&globals->Down_list_mutex);
			// return failure
			return 0;
		}
		else	//if not empty
		{
			do
			{
				flag = 0;
				while(temp != NULL && temp->p.from_floor != curr_floor)
				{	
					temp = temp->next;
				}
				// person not found
				if(temp == NULL)
				{
					/// unlock the mutex
					pthread_mutex_unlock(&globals->Down_list_mutex);
					return 0;
				}
		
				if(targeted_floor != -1)			// not specific targeted floor
				{
					if(dir > 0)
					{
						// can not pick up persons, who wants to go above targeted floor
						if(temp->p.to_floor > targeted_floor)
							flag =1;
					}
					else
					{
						// can not pick up persons, who wants to go below targeted floor
						if(temp->p.to_floor < targeted_floor)
							flag =1;
					}
				}
			}while(flag);		// if flag is true try to find next person

			//if temp != NULl means found matching person at specific floor
			(*p) = malloc(sizeof(struct Person));
			// Copy person data to pointer variable
			(*p)->p_id = temp->p.p_id;
			(*p)->from_floor =temp->p.from_floor;
			(*p)->to_floor = temp->p.to_floor;
			(*p)->arrival_time = temp->p.arrival_time;		
		
			// remove the person from linked list
			if(globals->Down_Person_list == temp)
				globals->Down_Person_list == NULL;
			//if temp is not last node
			if(temp->next != NULL)
				temp->next->prev = temp->prev;
			//if temp is first node				
			if(temp->prev == NULL)		
			{
					//Down direction
					globals->Down_Person_list = temp->next;
			}
			else
				temp->prev->next = temp->next;
		
			free(temp);
		}	
		/// unlock the mutex
		pthread_mutex_unlock(&globals->Down_list_mutex);
	}

	//return sucess
	return 1;

}
//###################################################
//This is specific function used to both activity drop any person or/and pick up person who wants to in the direction of elevator
		// Get parameters as address reference of the current elevator, gloabl structure reference, and person id-if any tageted 				person will be picked up or -1(no specific person)
		// and it resturn after doing the activity without returning any thing
void Do_floor_work(struct Elevator * e, struct gv * globals, int id)
{
	int id_list[20] = {-1};				// list of person id(drop/pick up)
	int count  = -1;				// count of persons (drop/ pick up)
	int flag = 0, disp_flag =0;
	int t1,i;
	double tm0 = get_cur_time() - globals->starting_time ;
	double tm;

	/*----------------------- debugging-------- display code	
	struct Person_node *te = e->inside_p;
	printf("[%.04f] Elevator %d arrives at floor %d\n",tm0,e->e_id,e->curr_floor);
	if(te == NULL)
		printf("no person");
	while(te != NULL)
	{
		printf("%d--%d,%d -- %lf \n",te->p.p_id,te->p.from_floor,te->p.to_floor,te->p.arrival_time);
		te = te->next;
	}
	printf("---------------\n");
	//---------------------------------------------------------*/

	// remove/drop person
	struct Person_node * temp = e->inside_p;
	struct Person_node * tt = NULL;
	do
	{
		if(count >= 0)
		{
			temp = tt;
			//------------------------------debugging-------- display codd---			
			/*te = NULL;
			te = e->inside_p;
			printf("[%.04f] Elevator %d after finding one erson to drop at floor %d\n",tm0,e->e_id,e->curr_floor);
			if(te == NULL)
				printf("no person");
			while(te != NULL)
			{
				printf("%d--%d,%d -- %lf \n",te->p.p_id,te->p.from_floor,te->p.to_floor,te->p.arrival_time);
				te = te->next;
			}
			printf("---------------\n");
			//-----------------------------------------------------------*/
		}
		else
			temp = e->inside_p;
		flag = 0;
		//printf("do_floor_drop");
		// find any person to drop
		while(temp != NULL && temp->p.to_floor != e->curr_floor)
		{
			temp = temp->next;
		}
		// if find any person suitable to drop at the floor
		if(temp != NULL && temp->p.to_floor == e->curr_floor)			// person found
		{
			// look for more persons flag =1
			flag =1;
			// Linked list remove

			// Single element
			if(e->inside_p == temp && e->inside_p->next == NULL)
			{	e->inside_p = NULL;
				flag =0;
			}
			//if temp is not last node
			if(temp->next != NULL)
				temp->next->prev = temp->prev;
			else					
				flag = 0;				// if last node then no need to check further
			//if temp is first node				
			if(temp->prev == NULL)	
			{	//struct Person_node * tt = e->inside_p->next;	
				e->inside_p = temp->next;
			}
			else
				temp->prev->next = temp->next;
			//printf("--%d--%d\n",temp->p.to_floor,e->curr_floor);
			count++;
			id_list[count] = temp->p.p_id;
			tt = temp->next;
			free(temp);
			// linked list remove ends
	
			temp = NULL;
		}
	}while(flag);
	tm = get_cur_time() - globals->starting_time ;
	// if any person droped from list- display
	if (count >= 0)
	{	printf("[%.04f] Elevator %d arrives at floor %d\n",tm0,e->e_id,e->curr_floor);
		disp_flag = 1;
	}
	for(i = 0;i<= count;i++)
	{
		if(i ==0)
		{
			if(i == count)
				printf("[%.04f] Elevator %d drops Person %d\n",tm,e->e_id,id_list[i]);
			else
				printf("[%.04f] Elevator %d drops Person %d",tm,e->e_id,id_list[i]);
		}
		else if (i == count)
			printf(", and Person %d\n",id_list[i]);
		else
			printf(", Person %d",id_list[i]);
	
		id_list[i] = -1;				// to reuse in pick up person
	}
	//globals finished count
	pthread_mutex_lock(&globals->num_people_mutex);		//lock mutex before updating shared globals num_people counters
	globals->num_people_finished += (count+1);
	pthread_mutex_unlock(&globals->num_people_mutex);

	// pick up person
	count = -1;
	if( id > 0)
	{
		count++;
		id_list[count] = id; 
	}
	struct Person_node * n = NULL;
	struct Person_node * t ;
	do
	{
		//printf("do_floor_pick up");
		struct Person * p = NULL;
		// check on the current floor for suitable person
		t1 = Find_Pickup_person(&p, e->curr_floor, e->direction,e->targeted_floor,globals);
		// if person found
		if(t1 != 0)
		{
			// add person to inside_p list
			// get memory for new node			
			n = malloc(sizeof(struct Person_node));
			if(n == NULL)
				printf("malloc failed!!!\n");
			n->next = NULL;

			//copy details
			n->p.p_id = p->p_id;
			n->p.from_floor = p->from_floor;
			n->p.to_floor = p->to_floor;
			n->p.arrival_time = p->arrival_time;

			// linked list insertion at last
			t = e->inside_p;
			// Empty List
			if(t == NULL)
			{
				e->inside_p = n;
				n->prev = NULL;
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

			count++;
			id_list[count] = p->p_id;
			n = NULL;
			continue;
		}
		else
			break;
	}while(1);

	// display if never displayed this line b4 and any person is being picked up
	if (count >= 0 && disp_flag ==0)
		printf("[%.04f] Elevator %d arrives at floor %d\n",tm0,e->e_id,e->curr_floor);
	
	tm = get_cur_time() - globals->starting_time ;

	for(i = 0;i<= count;i++)
	{
		if(i ==0)
		{
			if(i == count)
				printf("[%.04f] Elevator %d picks up Person %d\n",tm,e->e_id,id_list[i]);
			else
				printf("[%.04f] Elevator %d picks up Person %d",tm,e->e_id,id_list[i]);
		}
		else if (i == count)
			printf(", and Person %d\n",id_list[i]);
		else
			printf(", Person %d",id_list[i]);
	}
	
	//globals started count
	pthread_mutex_lock(&globals->num_people_mutex);		//lock mutex before updating shared globals num_people counters
	globals->num_people_started += (count+1);
	pthread_mutex_unlock(&globals->num_people_mutex);

	//printf("do_floor_out");
	return;
}

// Elevator Thread Function
void* Elevator_work(void* glb_var) 
{
	static int count = 0;
	struct gv* globals = (struct gv*)glb_var;
	int num = 0,t,c;	
	struct Elevator e;
	struct Person * p = NULL;
	struct Person_node *p_node = NULL;
	count++;
	int ID = count;
	e = Init_elevator(&ID);	


	while(globals->alive)
	{
		// Any person is waiting or not?
		do
		{
			while(globals->Up_Person_list == NULL && globals->Down_Person_list == NULL)
			{
				pthread_mutex_lock(&globals->elevator_mutex);
				pthread_cond_wait(&globals->elevator_cv, &globals->elevator_mutex);
				pthread_mutex_unlock(&globals->elevator_mutex);
			}
			t = get_person(&p,globals);
			//if(t==1)
				//printf("hhh1");

			//t =1;
		}while(t == 0);
		e.status = 1;
		int diff = p->from_floor - e.curr_floor;
		e.targeted_floor = p->to_floor;
		double tm = get_cur_time() - globals->starting_time ;
		c =0;			// floor counter
		// set elevator direction
		if(diff > 0)
		{	c = diff;
			e.direction = 1;
			//printf("%d--",diff);
		}
		else
		{	c = 0 - diff;
			e.direction = -1;
			//printf("%d--",diff);
		}

		//-----------------------------
		//starts moving towards the person 
		if(diff != 0)
			printf("\[%.04lf] Elevator %d starts moving from %d to %d\n",tm,ID,e.curr_floor,p->from_floor);
	
		// waiting time to reach the person's starting floor		
		while(c > 1)						// untill elevator reach targeted floor - 1
		{
			sleep(globals->elevator_speed);
			//printf("##%d##",e.direction);
			if(e.direction > 0)
				e.curr_floor++;
			else
				e.curr_floor--;

			Do_floor_work(&e,globals,-1);
			c--;
		}
		// last floor to reach targeted
		if(c ==1)
		{
			sleep(globals->elevator_speed); 
			//printf("##%d##",e.direction);
			if(e.direction > 0)
				e.curr_floor++;
			else
				e.curr_floor--;
		}
		// after reaching targeted floor

		//picking up process	

		p_node = malloc(sizeof(struct Person_node));
		if(p_node == NULL)
			printf("malloc failed!!!\n");
		// copy person data
		p_node->p.p_id = p->p_id;
		p_node->p.from_floor = p->from_floor;
		p_node->p.to_floor = p->to_floor;
		p_node->p.arrival_time = p->arrival_time;

		e.inside_p = p_node;					// targeted person inside the elevator
		

		//globals started count
		pthread_mutex_lock(&globals->num_people_mutex);		//lock mutex before updating shared globals num_people counters
		globals->num_people_started++;
		pthread_mutex_unlock(&globals->num_people_mutex);

		tm = get_cur_time() - globals->starting_time ;
		//printf("[%.04f] Elevator %d arrives at floor %d\n",tm,ID,p->from_floor,p->p_id);
		//printf("[%.04f] Elevator %d picks up Person %d\n",tm,ID,p->p_id);

		// set direction
		diff = p->to_floor - p->from_floor;
		if(diff > 0)
			e.direction = 1;
		else
			e.direction = -1;			

		// now the elvator should pick up every persons who wants to go to end of the floor in the direction
		e.targeted_floor = -1;
		// pick up any one if there is.
		Do_floor_work(&e,globals,p->p_id);
		while(e.inside_p != NULL)
		{
			sleep(globals->elevator_speed);
			if(e.direction > 0)
				e.curr_floor++;
			else
				e.curr_floor--;
			// change direction at end, there wont be any person as it is the end of floors
			if(e.direction > 0 && e.curr_floor == (globals->num_floors-1) )
				e.direction = -1;
			else if (e.direction < 0 && e.curr_floor == 0 )
				e.direction = 1;
			
			Do_floor_work(&e,globals,-1);		
		}

		// no person inside
		e.status = 0;

		// remove person
		p = NULL;
		e.inside_p = NULL;
//*/
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
//------------------------------------------------------------------------------

int main(int argc, char* argv[]) 
{
	int              i,n;
	pthread_t        *thrds;
	pthread_attr_t   *attrs;
	void             *retval;
	struct gv 	 globals;
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
	//*/

	/*
	globals.num_elevators = 3;
	globals.num_floors = 10;
	globals.people_arrival_time = 1;
	globals.elevator_speed = 1;
	globals.simulation_time = 30;
	globals.random_seed = 4;
	//*/

	pthread_mutex_init(&globals.Up_list_mutex, NULL);
	pthread_mutex_init(&globals.Down_list_mutex, NULL);
	globals.Up_Person_list = NULL; 					//initialization empty list
	globals.Down_Person_list = NULL;
	pthread_mutex_init(&globals.num_people_mutex, NULL);
	globals.num_people_started = 0;
	globals.num_people_finished = 0;
	globals.alive = 1;

	srand(globals.random_seed);
	globals.starting_time = get_cur_time();
	pthread_mutex_init(&globals.elevator_mutex, NULL);
	pthread_cond_init (&globals.elevator_cv, NULL);


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
			if(pthread_create(thrds+i, attrs+i, Elevator_work, &globals) != 0) 
			{
		      		perror("pthread_create()");
		      		exit(1);
	    		}
		}
  	}

	Simulation_timmer(&globals);

	// join threads
	for(i = 0; i < n; i++) 
	{ 
		pthread_cancel(thrds[i]);
	}
	//end of the execution time
	double t = get_cur_time();
	
	printf("Simulation result: %d prople have started, %d people have finished during %d seconds\n",globals.num_people_started,globals.num_people_finished,globals.simulation_time);	


	// Freeing mutexes
	pthread_mutex_destroy(&globals.Up_list_mutex);
	pthread_mutex_destroy(&globals.Down_list_mutex);
	pthread_mutex_destroy(&globals.num_people_mutex);
	pthread_mutex_destroy(&globals.elevator_mutex);
	pthread_cond_destroy(&globals.elevator_cv);

	free(attrs);
	free(thrds);

	return 0;
}

