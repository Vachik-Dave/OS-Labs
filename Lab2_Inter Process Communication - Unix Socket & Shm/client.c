#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

//Total number of messages to be send
#define no_msges 10000000

// returns a random number in the range
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

int main()
{

	int sd,i=0;
	double num, start_time;
	double sum=0;
	struct sockaddr_in cli_addr,ser_addr;
	char str[256];
	
	// create socket
	sd = socket(AF_INET,SOCK_STREAM,0);

	//server address
	// Internet family of IPv4
	ser_addr.sin_family = AF_INET;
	// lockal host address IPv4
	ser_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	// available port assigne to server address	
	ser_addr.sin_port = htons(9602);
	
	// Client address
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	cli_addr.sin_port = htons(9500);
	
	//bind the socket to specific client address
	bind(sd,(struct sockaddr*)& cli_addr,sizeof(cli_addr));


	// try to connect the server at ser_addr throught socket
	connect(sd,(struct sockaddr*)& ser_addr,sizeof(ser_addr));
	
	//Generate numbers and send
	
	// initial seed for random number generator
	srand(time(NULL));

	// note the starting time
	start_time = get_cur_time();
	while(i < no_msges)
	{
		// get new random number
		num = get_rand(100);
		
		// send the number
		if((write(sd,&num,sizeof(double))) < 0)
			perror("writing fail");
		
		
		sum = sum+num;
		//printf("---- %d --",num);
		i++;
	}
	// send starting time as last message
	if((write(sd,&start_time,sizeof(double))) < 0)
			perror("writing fail");
	
	//print total at client(driver) side.
	printf("Driver Total = %lf \n",sum);

	close(sd);
	return 0;
}
