//-----------------------------------Simple Shall program in c.
//the only thing is , My > is not working with pipe. > its working for 1 commnad & '.' (current directory) can not work with c program.


#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<stdlib.h>  

//|   
char *** cmd_parse(char *, int *, int * *);
int cmd_executor(int, char * * *, int * );
void do_exe (int ,char * * *, int * , int);

int main()
{
	int err = 0,i,j,k;
	char s[50];
	char * *cmd_list;
	int no_cmd;
	char * * * argv_list;
	int * argc_list;
	printf("\n v_shell-> ");
		gets(s);

	do
	{		
		no_cmd = 0;
		argv_list = cmd_parse(s,&no_cmd,&argc_list);
		
		//printf("%d",no_cmd);
		/*if(err)
		{	printf("Parsing error...");
			exit(1);
		}
		/*for(i =0; i <= no_cmd;i++)
		{
			printf("%d: ",argc_list[i]);
			for(j=0;j<=argc_list[i];j++)
			{
				printf(" %s \n",argv_list[i][j]);
			}

		}*/

		err = cmd_executor(no_cmd,argv_list,argc_list);
//		if (err)
//		{	printf("Execution Error...");
//			exit(1);
//		}
		
		//clear memory of argv & argc		
		for(i =0; i < no_cmd;i++)
		{
			for(j=0;j<=argc_list[i];j++)
			{
				free(argv_list[i][j]);
			}
			free(argv_list[i]);
			//free(argc_list[i]);
		}
		free(argv_list);
		free(argc_list);

		printf("\n v_shell-> ");
		gets(s);

	}while(strcmp(s,"exit")!=0 && strcmp(s,"Exit") != 0 && strcmp(s,"EXIT") != 0);
	return 0;
}

char *** cmd_parse(char * s, int * no_cmd,int * * argc_list)
{
	
	char * token;
	int t =0,i,total=512,c=0;
	*no_cmd =-1;
	short int flag = 0;
	char * *cmd_list;
	char * * * argv_list;

	//temporary array of tokens(list of cmmands) with maximum limit of total(300) commands 
	char ** list_tokens = (char**)calloc(total,sizeof(char*));

	// commands are separated by pipe-line symbol
	token = strtok(s,"|");

	// store tokens in temporary array
	while(token != NULL)
	{		
		if(token == NULL)			//break if token is empty
			break;		
		int tttt = strlen(token);
		//printf("%d   ",tttt);
		list_tokens[t] = (char *) calloc(tttt+1,sizeof(char));		// assign memory of string length + 1(null)
		//printf("%s \n",token);
		strcpy(list_tokens[t],token);
		//printf("%s \n",list_tokens[t]);
		t++;c++;
		// reallocate memory if number of tokens are above 512
		if(c == 512)
		{
			c=0;
			list_tokens = (char**)realloc(list_tokens,total*sizeof(char*));
		}
		token = strtok(NULL,"|");
	}
	
	// number of commands are t-1
	*no_cmd = t-1;

	// allocate only required memory(for total no. of commands only) to command list
	cmd_list = (char **)calloc (*no_cmd,sizeof(char*));

	// copy list of commands	
	for(i=0;i<=*no_cmd;i++)
	{	
		cmd_list[i] = (char *)calloc(strlen(list_tokens[i])+1,sizeof(char));		// assign memory of string length + 1(null)
		strcpy(cmd_list[i],list_tokens[i]);		
		//printf("%s \n",cmd_list[i]);
		//printf("%d",t);
		free(list_tokens[i]);					// free temporary variable memory
	}	
	free(list_tokens);				// freeing all remaining memory (300-t) 

	
	// assigning memory to list of arguments for total no. of commands
	argv_list = (char***)calloc(*no_cmd,sizeof(char**));
	*argc_list = (int *) calloc(*no_cmd,sizeof(int));
	
	
	// For each command (list of arguments) arguments are separated by " " or tab
	for(i =0;i<=*no_cmd;i++)
	{
		int l=0,temp=0;
		token = NULL;
		//list_tokens = NULL;
		//temporary array of tokens(list of argument) with maximum limit of total(300) arguments / command
		list_tokens = (char**)calloc(total,sizeof(char*));		
		c=0;
		token  = strtok(cmd_list[i]," \t");		
		while(token != NULL)
		{			
			if(token == NULL)			//break if token is empty
				break;		
			int tttt = strlen(token);

			list_tokens[l] = (char *) calloc(tttt+1,sizeof(char));		// assign memory of string length + 1(null)

			strcpy(list_tokens[l],token);
			l++;c++;
			// reallocate memory if number of tokens are above 512
			if(c == 512)
			{
				c=0;
				list_tokens = (char**)realloc(list_tokens,total*sizeof(char*));
			}
			token = strtok(NULL," \t");
		}

		// copy count of arguments in argument count list
		(*argc_list)[i] = l-1;				

		// allocate only required memory(for l arguments only) +1(for NULL srting required for execvp) to argument list for ith command
		argv_list[i] = (char **)calloc ((l-1),sizeof(char*));

		// copy list of arguments	
		for(temp=0;temp<l;temp++)
		{	
			// assign memory of string length+1(for null End of string symbol)
			argv_list[i][temp] = (char *)calloc(strlen(list_tokens[temp])+1,sizeof(char));		
			
			strcpy(argv_list[i][temp],list_tokens[temp]);		

			//printf("%s \n",(*argv_list)[i][temp]);
			//printf("%d",t);
			
			// free temporary variable memory
			free(list_tokens[temp]);					
		}	
		// last string of each command argument need to be NULL for execvp arguments
		//(*argv_list)[i][l] = 0;
		free(list_tokens);				// freeing all remaining memory (300-l)

	}

	int flg_less =0,flg_grt = 0;

	// For each command arguments- check the syntext	
	for(i=0;i<=*no_cmd;i++)
	{
		//printf("%d -> %d \n", i, argc_list[i]);
		for(t=0;t<=(*argc_list)[i];t++)
		{		
			//printf("%s",(*argv_list)[i][t]);			
			char * tt=NULL;
		
			// check syntext errors related to file redirection symbols
			tt = strchr((argv_list)[i][t],'<');
			//if '<' symbol found  
			if(tt != NULL )
			{	// first time set flag
				if(!flg_less)
					flg_less =1;
				// second time give error
				else
				{	printf("1Syntext Error... in command - %d @ word - %s \n",i +1,(argv_list)[i][t]);
					(*argc_list)[i] = -1;
					flag = 1;
					break;
				}
				// symbol is not in first command 
				if(i != 0)
				{	printf("1Syntext Error... in command - %d @ word - %s \n",i +1,(argv_list)[i][t]);
					(*argc_list)[i] = -1;
					flag = 1;
					break;
				}
				// if symbol within other string then it must start with '<' else file name is missing 
				if(strlen((argv_list)[i][t]) != 1)
				{	if((argv_list)[i][t][0] != '<')
					{	printf("1Syntext Error... in command - %d @ word - %s \n",i +1,(argv_list)[i][t]);
						(*argc_list)[i] = -1;
						flag = 1;
						break;
					}
				}
				// filename not provided with '<' give error
				else if(t == (*argc_list)[i])
				{	printf("1Syntext Error(file name missing)... in command - %d @ word - %s \n",i +1,(argv_list)[i][t]);
					(*argc_list)[i] = -1;
					flag = 1;
					break;
				}
				// command is not provided with '<' give error
				if( t == 0)
				{	printf("1Syntext Error(command missing)... in command - %d @ word - %s \n",i +1,(argv_list)[i][t]);
					(*argc_list)[i] = -1;
					flag = 1;
					break;
				}
			}


			tt = NULL;
			tt = strchr((argv_list)[i][t],'>');
			//if '>' symbol found 
			if(tt != NULL)
			{	// first time set flag
				if(!flg_grt)
					flg_grt =1;
				// second time give error
				else
				{	printf("2Syntext Error... in command - %d @ word - %s \n",i +1,(argv_list)[i][t]);
					(*argc_list)[i] = -1;
					flag = 1;
					break;
				}				
				// symbol is not in last command 
				if(i != *no_cmd)
				{	printf("2Syntext Error... in command - %d @ word - %s \n",i +1,(argv_list)[i][t]);
					(*argc_list)[i] = -1;
					flag = 1;
					break;
				}
				// symbol within other strings then it must starts with '>'else not in last command 
				if(strlen((argv_list)[i][t]) != 1)
				{	if((argv_list)[i][t][0] != '>')
					{
						printf("2Syntext Error... in command - %d @ word - %s \n",i +1,(argv_list)[i][t]);
						(*argc_list)[i] = -1;
						flag = 1;
						break;
					}
				}
				// filename not provided with '>' give error
				else if(t == (*argc_list)[i])
				{	printf("2Syntext Error(file name missing)... in command - %d @ word - %s \n",i +1,(argv_list)[i][t]);
					(*argc_list)[i] = -1;
					flag = 1;
					break;
				}
				// command is not provided with '>' give error
				if( t == 0)
				{	printf("2Syntext Error(command missing)... in command - %d @ word - %s \n",i +1,(argv_list)[i][t]);
					(*argc_list)[i] = -1;
					flag = 1;
					break;
				}
			}


			tt = NULL;
			tt = strchr((argv_list)[i][t],'&');
			//if '&' symbol found other than last position or with other characters give error
			if(tt != NULL)
			{
				if(strlen((argv_list)[i][t]) != 1) 
				{	printf("3Syntext Error... in command - %d @ word - %s \n",i +1,(argv_list)[i][t]);
					(*argc_list)[i] = -1;
					flag = 1;
					break;
				}	
			}
			

			tt = NULL;
			tt = strchr((argv_list)[i][t],'/');
			//if '/' symbol found as a file name or any sigular string give error
			if((tt != NULL && strlen((argv_list)[i][t]) == 1) )
			{	printf("4Syntext Error... in command - %d @ word - %s \n",i +1,(argv_list)[i][t]);
				(*argc_list)[i] = -1;
				flag = 1;
				break;
			}
			
			tt = NULL;
			tt = strchr((argv_list)[i][t],'\0');
			//if NULL character found as a file name or any sigular string give error
			if((tt != NULL && strlen((argv_list)[i][t]) == 0) )
			{	printf("5Syntext Error... in command - %d @ word - %s \n",i +1,(argv_list)[i][t]);
				(*argc_list)[i] = -1;
				flag = 1;
				break;
			}

		}
		
	}	
/*	int j;
		for(i =0; i <= (*no_cmd);i++)
		{
			printf("%d: ",(*argc_list)[i]);
			for(j=0;j<=(*argc_list)[i];j++)
			{
				printf(" %s \n",(*argv_list)[i][j]);
			}

		}
/ *
	int pid,pid2;
	if((pid=fork()) != -1)
	{	if(pid == 0)
			execvp((*argv_list)[0][0], (*argv_list)[0]);
		else {
			//sleep(1);
	}		wait(); }
	if ((pid2 =fork()) != -1)
	{	if(pid2==0)
			execvp((*argv_list)[1][0],(*argv_list)[1]);
		else{
			//sleep(1);
			wait();  }
	}
*/

//	argv_list_1 = (char ****)malloc(sizeof(argv_list));
//	argv_list_1 = &argv_list;
//	argc_list_1 = (int**)malloc(sizeof(argc_list));
	//argc_list_1 = &argc_list;
//	*argc_list_1 = (int *) calloc(no_cmd,sizeof(int));
//	*argc_list_1[0] = argc_list[0];
//	*argc_list_1[1] = argc_list[1];
//	printf("val: %d - %d ",(*argc_list)[0],(*argc_list)[1]);
	if(flag)
	{	perror("Parsing error...");
		exit(1);
	}
	else
		return argv_list;
}

int cmd_executor(int no_cmd, char * * * argv_list, int * argc_list)
{	
	int i,t,fd,fd1,pid,status,k,fl=0;
	int background_flag =0;
	char * * cmd;
	// for last command check for &
	if(argv_list[no_cmd][argc_list[no_cmd]] == "&")
	{
		background_flag =1;
	}
	/////////////////////////////////////////////////////////////		
	pid = fork();
	if(pid < 0 )
	{	perror("fork failed...");
		exit(1);
	}

	else if(pid == 0)
	{
		//perror("hi hi hi");
		if(no_cmd == 0)
		{
			//perror("hi hi");
			/*const int  argc= argc_list[0]+1;
			char * cmd[argc];
			for(t =0; t<=argc ; t++)
			{
				strcpy(cmd[t],argv_list[0][t]);
			} 
			cmd[t] = 0;
			*/
			
			// for last command check for >
			i=no_cmd;
			for(t=0;t<=argc_list[i];t++)
			{	char * tt=NULL;
				tt = strchr(argv_list[i][t],'>');
				//if '<' symbol found  
				if(tt != NULL)
				{	char * file_name;
					//perror("hhhh");
					//strcpy(argv_list[i][t],NULL);
					if(strlen(argv_list[i][t]) != 1)
					{	file_name = (char*)malloc(sizeof(char)*strlen(tt));
						strcpy(file_name,&tt[1]);	
					}
					else
					{	file_name = (char*)malloc(sizeof(char)*(strlen(argv_list[i][t+1])+1));	
						strcpy(file_name,argv_list[i][t+1]);
						//strcpy(argv_list[i][t+1],NULL);
					}
					fd1 = open(file_name,O_CREAT|O_WRONLY, 0644);
					//dup2(1,temp_stdout);
					if(fd1 < 0 )					
					{	perror("fd1 : hi hi");}
					if(dup2(fd1,1) != 1)
					{
						perror("dup2- file rediraction :");
						exit(1);
					}
					cmd = (char**)calloc(t+1,sizeof(char*));
					for(k =0; k<t ; k++)
					{
						cmd[k] = (char*) calloc(strlen(argv_list[i][k])+1,sizeof(char));
						strcpy(cmd[k],argv_list[i][k]);
					} 
					cmd[t] = 0;
					fl =1;
					break;
				}
			}

			// for first command check for <
			i=0;
			for(t=0;t<=argc_list[i];t++)
			{	char * tt=NULL;
				tt = strchr(argv_list[i][t],'<');
				//if '<' symbol found  
				if(tt != NULL)
				{	//perror("< found");
					char * file_name;
					if(strlen(argv_list[i][t]) != 1)
					{	file_name = (char*)malloc(sizeof(char)*strlen(tt));
						strcpy(file_name,&tt[1]);	
					}
					else
					{	file_name = (char*)malloc(sizeof(char)*(strlen(argv_list[i][t+1])+1));
						strcpy(file_name,argv_list[i][t+1]);
					}
					printf("%s", file_name);
					fflush(stdout);
					fd = open(file_name,O_RDONLY);

					if(fd <0 )
					{	perror("fd fails");}
						//dup2(0,temp_stdin);					
					if(dup2(fd,0) != 0)
					{
						perror("dup2- file rediraction :");
						exit(1);
					}
					
					cmd = (char**)calloc(t+1,sizeof(char*));
					for(k =0; k<t ; k++)
					{
						cmd[k] = (char*) calloc(strlen(argv_list[i][k])+1,sizeof(char));
						strcpy(cmd[k],argv_list[i][k]);
					} 
					cmd[t] = 0;
					fl=1;
					break;
				}
			}
			if(fl)
			{
				if(execvp(argv_list[0][0], cmd) <0)
				{
					perror("execvp error:");
					exit(1);
				}

			}
			else
			{
				if(execvp(argv_list[0][0], argv_list[0]) <0)
				{
					perror("execvp error:");
					exit(1);
				}
			}			
			
		}
		else
		{
			//perror("hi hi hi");
			// First time executor call with 0 index
			do_exe(no_cmd,argv_list,argc_list,0);
		}
	}
	else
	{
		if(!background_flag)
			wait(&status);
	}
	
	return 0;
}

void do_exe (int no_cmd,char * * * argv_list, int * argc_list, int index)
{

	int pid,status,new_index = index+1;
	int fds[2];
	int i,t;

	if(index == no_cmd)
		return;


   	if(pipe(fds) <0)
	{
		perror("pipe error");
		exit(1);
	}

    	pid = fork();

	if (pid == -1)
    	{
		perror ("fork error\n");
		exit(1);
	}
	else  if (pid == 0)
	{ 
        //child process...

		if(dup2(fds[1],1) <0)
		{
			perror("dup2 error\n");
			exit(1);
		}
		close(fds[0]);
     
		if(index ==0)
		{
			int fd,k,fl=0;
			char ** cmd;
			// for first command check for <
			i=0;
			for(t=0;t<=argc_list[i];t++)
			{	char * tt=NULL;
				tt = strchr(argv_list[i][t],'<');
				//if '<' symbol found  
				if(tt != NULL)
				{	//perror("< found");
					char * file_name;
					if(strlen(argv_list[i][t]) != 1)
					{	file_name = (char*)malloc(sizeof(char)*strlen(tt));
						strcpy(file_name,&tt[1]);	
					}
					else
					{	file_name = (char*)malloc(sizeof(char)*(strlen(argv_list[i][t+1])+1));
						strcpy(file_name,argv_list[i][t+1]);
					}
					printf("%s", file_name);
					fflush(stdout);
					fd = open(file_name,O_RDONLY);

					if(fd <0 )
					{	perror("fd fails");}
						//dup2(0,temp_stdin);					
					if(dup2(fd,0) != 0)
					{
						perror("dup2- file rediraction :");
						exit(1);
					}
					
					cmd = (char**)calloc(t+1,sizeof(char*));
					for(k =0; k<t ; k++)
					{
						cmd[k] = (char*) calloc(strlen(argv_list[i][k])+1,sizeof(char));
						strcpy(cmd[k],argv_list[i][k]);
					} 
					cmd[t] = 0;
					fl=1;
					break;
				}
			}
			if(fl)
			{
				if(execvp(argv_list[0][0], cmd) <0)
				{
					perror("execvp error:");
					exit(1);
				}

			}
			else
			{
				if(execvp(argv_list[0][0], argv_list[0]) <0)
				{
					perror("execvp error:");
					exit(1);
				}
			}
		}
		if(execvp(argv_list[index][0], argv_list[index]) < 0)
		{
			perror("execvp error:");
			exit(1);
		}    
	}
	else
	{
        //parent process...
     		int fd1;
		if(dup2(fds[0],0)<0)
		{
			perror("dup2 error\n");
			exit(1);
		}
		close(fds[1]);

		// Recurrsive executor call...
		do_exe(no_cmd,argv_list,argc_list,new_index);

/*------------------------ This code is working for 2-3 command with > , but giving segmentation fault for larg string for inputs

		// for last command check for >
		if(new_index=no_cmd)
		{
			// for last command check for >
			i=no_cmd;
			for(t=0;t<=argc_list[i];t++)
			{	char * tt=NULL;
				tt = strchr(argv_list[i][t],'>');
				//if '<' symbol found  
				if(tt != NULL)
				{	char * file_name;
					//perror("hhhh");
					//strcpy(argv_list[i][t],NULL);
					if(strlen(argv_list[i][t]) != 1)
					{	file_name = (char*)malloc(sizeof(char)*strlen(tt));
						strcpy(file_name,&tt[1]);	
					}
					else
					{	file_name = (char*)malloc(sizeof(char)*(strlen(argv_list[i][t+1])+1));	
						strcpy(file_name,argv_list[i][t+1]);
						//strcpy(argv_list[i][t+1],NULL);
					}
					fd1 = open(file_name,O_CREAT|O_WRONLY, 0644);
					//dup2(1,temp_stdout);
					if(fd1 < 0 )					
					{	perror("fd1 : hi hi");}
					if(dup2(fd1,1) != 1)
					{
						perror("dup2- file rediraction :");
						exit(1);
					}
					cmd = (char**)calloc(t+1,sizeof(char*));
					for(k =0; k<t ; k++)
					{
						cmd[k] = (char*) calloc(strlen(argv_list[i][k])+1,sizeof(char));
						strcpy(cmd[k],argv_list[i][k]);
					} 
					cmd[t] = 0;
					fl =1;
					break;
				}
			}

			if (fl)
			{
					if(execvp(argv_list[new_index][0],cmd) <0)
					{
						perror("execvp error:");
						exit(1);
					}
			}
			else
			{
					if(execvp(argv_list[new_index][0], argv_list[new_index]) <0)
					{
						perror("execvp error:");
						exit(1);
					}

			}
		}*/
//-----------------------------------end of > logic
		// for last command check for >
		if(new_index=no_cmd)
		{
		}
		if(execvp(argv_list[new_index][0], argv_list[new_index]) <0)
		{
			perror("execvp error:");
			exit(1);
		}
	}
}

 

