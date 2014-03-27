#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

double ** Matrix;
double ** Result_thrd;
int row,col,N;
pthread_mutex_t	mutex = PTHREAD_MUTEX_INITIALIZER;

//--------------------------------------------------------------------------
// returns a random number in the range of 0-(n-1)
int get_rand(int n) 
{ //return range(0 to n-1) of numbers randomly 
  return  ( ((double) rand()) / RAND_MAX ) * n;
}

//--------------------------------------------------------------------------
// Initialize the Matrix with sequence of numbers
void InitializeMatrix()
{
	int i,j;
	Matrix = (double **) malloc(N * sizeof(double*));
	for(i=0;i<N;i++)
	{
		Matrix[i] = (double*) malloc(N * sizeof(double));
		for(j=0;j<N;j++)
			//Matrix[i][j] = (N*i) + (j+1);		//Sequencial values
			Matrix[i][j] = get_rand(10);		//Random Values
	}
	// printed to check
	/*for(i=0;i<N;i++)
	{
		for(j=0;j<N;j++)
			printf("%d  ",Matrix[i][j]);
		printf("\n");
	}*/
}
//--------------------------------------------------------------------------
// This function compare result of both methods element by element and returns 0(false) or 1(true).
int Check_result(double ** Result_seq)
{
	int i,j,flag = 1;
	for(i=0;i<N;i++)
	{
		for(j=0;j<N;j++)
		{
			if(Result_thrd[i][j] != Result_seq[i][j])
			{	flag =0;
				break;
			}
		}
	}
	return flag;	
}
//--------------------------------------------------------------------------
// Matrix multiplication using sequencial version(3 for loops), which returns resultant matrix.
double ** MatMul()
{
	int i,j,k,sum;
	double ** Result = (double **) malloc(N * sizeof(double*));
	
	
	for(i=0;i<N;i++)
	{
		Result[i] = (double *) malloc(N * sizeof(double));
		for(j=0;j<N;j++)
		{	sum =0;
			for(k=0;k<N;k++)
			{
				sum = sum + Matrix[i][k] * Matrix[k][j];	
			}
			Result[i][j] = sum;
		}
	}
	
	// printed to check
	/*for(i=0;i<N;i++)
	{
		for(j=0;j<N;j++)
			printf("%d   ",Result[i][j]);
		printf("\n");
	}*/
	return Result;
}
//-----------------------------------------------------------------------
// Calculate value for r-row & c-column (used by threads)
void calculate(int r,int c)
{
	int k,sum =0;
	for(k=0;k<N;k++)
	{
		// Matrix is global, but only read by threads
		sum = sum + Matrix[r][k] * Matrix[k][c];
	}

	// Though Result_thrd is global and shared among the threads
	//-------no threads try to overwrite other thread's work as 
	//-------r & c are unique for each thread managed by critical section.
	Result_thrd[r][c] = sum;
	
	return;
}
//-----------------------------------------------------------------------
// Thread function used by every threads, which calculates specific elements.

//------ for managing race condition for row & col variables which contains next elemnet to be calculated
//------------------------those two variables are used in critical section insind while loop.

void* Thread_work(void* lv) 
{
	int* ID = (int *) lv;
	int count=0;
	int r,c;	
	// print every thread starting message
	printf("\nThread No %d : Started working\n",*ID,count);
	while(1)
	{
		// Manage critical section
		pthread_mutex_lock(&mutex);
		// read row and column value for next elemnet		
		r = row;
		c = col;
		// increament the row & column value

		// if 0-(N-1) rows are calculated then stop working
		if(row == N)
		{
			//printf("here %d",*ID);
			pthread_mutex_unlock(&mutex);
			break;
		}

		// if column reach at the end of matrix column size
		if(col == N-1)
		{	
			row = row+1;
			col =0;
		}
		// else increament only column value
		else
			col = col +1;

		pthread_mutex_unlock(&mutex);
		// End of critical section

		// r, c and count are local variable to threads, no need to put in critical section.
		calculate(r,c);
		count++;
	}
	
	// print every thread local count
	printf("\nThread No %d : Calculated %d elements\n",*ID,count);

	return NULL;
}

//----------------------------------------------------------------------------------
// Matric multiplication using threading, have number of threads as parameter.

void MatMul_thrd(int no_thrd)
{
	int              *tids;  
	pthread_t        *thrds;
	pthread_attr_t   *attrs;
	void             *retval;
	int i;
	
	thrds = (pthread_t*) malloc(sizeof(pthread_t)*no_thrd);
	attrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t)*no_thrd);
	tids  = (int*) malloc(sizeof(int)*no_thrd);
	
	// assigning memory to resultant array
	Result_thrd = (double **) malloc(N * sizeof(double*));
	for(i=0;i<N;i++)	
		Result_thrd[i] = (double *) malloc(N * sizeof(double));

	// create threads
  	for(i = 0; i < no_thrd; i++) 
	{
		if(pthread_attr_init(attrs+i)) 
		{
	        	perror("attr_init()");
    		}
		
		tids[i] = i+1;
		if(pthread_create(thrds+i, attrs+i, Thread_work, tids+i) != 0) 
		{
	      		perror("pthread_create()");
	      		exit(1);
    		}
  	}

	// join threads
	for(i = 0; i < no_thrd; i++) 
	{ 
		pthread_join(thrds[i], &retval);
	}


	free(attrs);
	free(thrds);
	free(tids);

	return;
	
}
//----------------------------------------------------------------------------
// Main function
int main(int argc, char* argv[]) 
{
	int i,j;
	double **Result_Seq;
	int no_thrd;
	
	if(argc != 3) 
	{
    		fprintf(stderr, "missing arguments\n", argv[0]);
    		exit(1);
  	}
	//N =3;
	//no_thrd = 2;

	N = atoi(argv[1]);
	no_thrd = atoi(argv[2]);
	
	InitializeMatrix();
	
	Result_Seq = MatMul();
	
	// row and column initialization
	row =0;
	col=0;
	MatMul_thrd(no_thrd);
	
	if(Check_result(Result_Seq))
		printf("\n----------------Results are same for both versions-------------------\n\n");
	else
		printf("\n-----#####----Error: Both result are not same-------####---------\n\n");
	
	return 0;
}
