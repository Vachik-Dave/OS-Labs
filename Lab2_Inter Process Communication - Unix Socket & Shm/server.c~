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
#include <sys/time.h>

//Total number of messages to be send
#define no_msges 10000000

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

	int cd,ld,len,i=0;
	double num,start_time,end_time;
	double sum=0;
	struct sockaddr_in cli_addr,ser_addr;


	//create socket
	ld = socket(AF_INET,SOCK_STREAM,0);

	//server address
	// Internet family of IPv4
	ser_addr.sin_family = AF_INET;
	// lockal host address IPv4
	ser_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	// available port assigne to server address	
	ser_addr.sin_port = htons(9602);
	

	//bind the server address to the socket
	bind(ld,(struct sockaddr*)& ser_addr,sizeof(ser_addr));

	//wait for client
	listen(ld,1);

	len = sizeof(cli_addr);

	// accept connect request from client
	cd = accept(ld,(struct sockaddr*)& cli_addr,&len);

	//wait for receiving numbers
	while(i < no_msges)
	{
		//read message from client
		if(read(cd,&num,sizeof(double)) <0)
			perror("reading fail");
		//printf("%d -- ",num);	
		sum = sum +num;
		i++;
		
	}
	//get starting time as last message
	if(read(cd,&start_time,sizeof(double)) <0)
			perror("reading fail");
	
	//get current time as ending time
	end_time = get_cur_time();

	// print total at server(application) side.
	printf("Application Total = %lf \n",sum);

	// print time taken to send these messages
	printf("Total time for 10 million (+1) messages = %g Seconds\n",end_time - start_time);

	close(ld);
	close(cd);	
	return 0;
}
