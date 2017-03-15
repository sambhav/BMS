#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define USER 0
#define POLICE 1
#define ADMIN 2
#define UNAUTH_USER -1
#define RESPONSE_BYTES 512
#define REQUEST_BYTES 512
#define linesInMS 5
#define EXIT -1

void sendMsgtoClient(int clientFD, char *str) {
    int numPacketsToSend = (strlen(str)-1)/RESPONSE_BYTES + 1;
    int n = write(clientFD, &numPacketsToSend, sizeof(int));
    char *msgToSend = (char*)malloc(numPacketsToSend*RESPONSE_BYTES);
    strcpy(msgToSend, str);
    int i;
    for(i = 0; i < numPacketsToSend; ++i) {
        int n = write(clientFD, msgToSend, RESPONSE_BYTES);
        msgToSend += RESPONSE_BYTES;
    }
}

char* recieveMsgFromClient(int clientFD) {
    int numPacketsToReceive = 0;
    int n = read(clientFD, &numPacketsToReceive, sizeof(int));
    if(n <= 0) {
        shutdown(clientFD, SHUT_WR);
        return NULL;
    }
    char *str = (char*)malloc(numPacketsToReceive*REQUEST_BYTES);
    memset(str, 0, numPacketsToReceive*REQUEST_BYTES);
    char *str_p = str;
    int i;
    for(i = 0; i < numPacketsToReceive; ++i) {
        int n = read(clientFD, str, REQUEST_BYTES);
        str = str+REQUEST_BYTES;
    }
    return str_p;
}


void getupcli(char *username,char *password,int client_fd)
{
	char *ruser,*rpass;
	sendMsgtoClient(client_fd,"Enter Username: ");
	ruser=recieveMsgFromClient(client_fd);

	sendMsgtoClient(client_fd,"Enter Password: ");
	rpass=recieveMsgFromClient(client_fd);

	int i=0;
	while(ruser[i]!='\0' && ruser[i]!='\n')
	{
		username[i]=ruser[i];
		i++;
	}

	username[i]='\0';

	i=0;
	while(rpass[i]!='\0' && rpass[i]!='\n')
	{
		password[i]=rpass[i];
		i++;
	}
	password[i]='\0';

}


char *printMiniStatement(char *username,int client_fd)
{
	FILE *fp = fopen(username,"r");

	char *miniStatement = NULL;

    // Initializing miniStatement to a blank char.
	miniStatement = (char *)malloc(10000*sizeof(char));
    miniStatement[0] = '\0';

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int count=0;

	while(count<linesInMS && (read = getline(&line, &len, fp)) != -1)
	{
		strcat(miniStatement,line);
		count++;
	}

	fclose(fp);

	if(strlen(miniStatement)==0)
		strcpy(miniStatement,"None");

	return miniStatement;
}

char *printBalance(char *username)
{
	FILE *fp=fopen(username,"r");
	char * line = NULL;
    size_t len = 0;
    ssize_t read;

    if((read = getline(&line, &len, fp)) != -1)
    {
    	char *token,*prevtoken;
    	prevtoken=(char *)malloc(400*sizeof(char));
    	token=strtok(line," \n");
    	while(token!=NULL)
    	{
    		strcpy(prevtoken,token);
    		token=strtok(NULL," \n");
    	}
    	fclose(fp);
    	return prevtoken;
    }
    else
    {
    	fclose(fp);
    	char *bal=(char *)malloc(2*sizeof(char));
    	bal[0]='0';
    	bal[1]='\0';
    	return bal;
    }
}


void userRequests(char *username,char *password,int client_fd)
{
	int flag=1;
	sendMsgtoClient(client_fd,"Enter your choice\n1. Available Balance\n2. Mini Statement\nWrite exit for quitting.");
	char *buff=NULL;
	while(flag)
	{
		if(buff!=NULL)
			buff=NULL;
		buff=recieveMsgFromClient(client_fd);

		int choice;

		if(strcmp(buff,"exit")==0)
			choice=3;
		else
		    choice=atoi(buff);
		char *bal,*str;
		bal=(char *)malloc(1000*sizeof(char));
		str=(char *)malloc(10000*sizeof(char));
		strcpy(bal,"------------------\nAvailable Balance: ");
		strcpy(str,"------------------\nMini Statement: \n");
		switch(choice)
		{
			case 1:
				strcat(bal,printBalance(username));
				sendMsgtoClient(client_fd,strcat(bal,"\n------------------\n\nEnter your choice\n1. Available Balance\n2. Mini Statement\nWrite exit for quitting."));
				break;
			case 2:
			 	strcat(str,printMiniStatement(username,client_fd));
				sendMsgtoClient(client_fd,strcat(str,"\n------------------\n\nEnter your choice\n1. Available Balance\n2. Mini Statement\nWrite exit for quitting."));
				free(str);
				break;
			case 3:
				flag=0;
				break;
			default:
				sendMsgtoClient(client_fd, "Unknown Query");
				break;
		}
	}
}


int checkUser(char *user)
{
	FILE *fp=fopen("login_file","r");
	char * line = NULL;
    size_t len = 0;
    ssize_t read;

	while((read = getline(&line, &len, fp)) != -1) 
	{
		char *token=strtok(line," ");
		if(strcmp(token,user)==0)
		{
			token=strtok(NULL," ");
			token=strtok(NULL," ");
			if(token[0]=='C')
			{
				fclose(fp);
				return 1;
			}
        }
    }

    fclose(fp);
    return 0;


}

void updateTrans(char *username,int choice,double balance)
{
	FILE *fp=fopen(username,"r");
	char * line = NULL;
	char c=(choice==1)?'C':'D';
	char *line1=(char *)malloc(sizeof(char)*10000);
    size_t len = 0;
    ssize_t read;
	time_t ltime; /* calendar time */

	ltime=time(NULL); /* get current cal time */
	sprintf(line1,"%.*s %c %f\n",(int)strlen(asctime(localtime(&ltime)))-1,asctime(localtime(&ltime)),c,balance);

	while((read = getline(&line, &len, fp)) != -1)
		strcat(line1,line);

	fclose(fp);
	fp=fopen(username,"w");
	fwrite(line1, sizeof(char), strlen(line1), fp);
	fclose(fp);
}

int query(char *username, int client_fd)
{
	int flag=1;
	sendMsgtoClient(client_fd,"Choose an option\n1. Credit\n2. Debit\nWrite exit to terminate");
	while(flag)
	{
		char* buff=recieveMsgFromClient(client_fd);

		if(strcmp(buff,"exit")==0)
			return EXIT;
		else
		{
			int choice=atoi(buff);
			double balance=strtod(printBalance(username),NULL);

			if(choice!=1 && choice!=2)
				sendMsgtoClient(client_fd,"Unknown Query");
			else
			{
				sendMsgtoClient(client_fd,"Enter amount");

				while(1)
				{
					char *buff=recieveMsgFromClient(client_fd);
					double amount=strtod(buff,NULL);
					// printf("%f",amount);
					if(amount<=0)
						sendMsgtoClient(client_fd,"Enter valid amount");
					else
					{
						if(choice==2 && balance<amount)
						{
							sendMsgtoClient(client_fd,"Insufficient Balance.\n--------------------\n\nEnter username of the account holder or 'exit' to quit.");
							flag=0;
							break;
						}
						else if(choice==2)
							balance-=amount;
						else if(choice==1)
							balance +=amount;

						updateTrans(username,choice,balance);
						sendMsgtoClient(client_fd,"User updated successfully.\n--------------------\n\nEnter username of the account holder or 'exit' to quit.");
						flag=0;
						break;
					}

				}
			}

		}
	}
}
void adminRequests(int client_fd)
{
	sendMsgtoClient(client_fd,"Enter username of the account holder or 'exit' to quit");

	while(1)
	{
		char *buff=NULL;
		buff=recieveMsgFromClient(client_fd);


		if(strcmp(buff,"exit")==0)
			break;
		else if(checkUser(buff))
		{
			char *userreq=(char *)malloc(40*sizeof(char));
			strcpy(userreq,buff);

			if(query(userreq,client_fd)==EXIT)
				break;
		}
		else
			sendMsgtoClient(client_fd,"Wrong Username. Please enter a valid user");
	}



}


char * getBalanceAll()
{
	FILE *fp=fopen("login_file","r");
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char *retstr=(char *)malloc(10000*sizeof(char));
    retstr[0]='\0';

	while((read = getline(&line, &len, fp)) != -1) 
	{
		char *token=strtok(line," ");
		char *token1=strtok(NULL," ");
		char *token2=strtok(NULL," ");
		if(token2[0]=='C')
		{
			strcat(retstr,token);
			strcat(retstr," ");
			strcat(retstr,printBalance(token));
			strcat(retstr,"\n");
        }
    }

    return retstr;

}	

void policeRequests(int client_fd)
{
	sendMsgtoClient(client_fd, "Enter your choice\n1. Print Balance of all users\n2. Get mini Statement\nWrite exit to terminate");
	int flag=1;

	while(flag)
	{
		char *buff=NULL;
		buff=recieveMsgFromClient(client_fd);
		char *bal,*str;
		bal=(char *)malloc(1000*sizeof(char));
		str=(char *)malloc(10000*sizeof(char));
		strcpy(bal,"------------------\nAvailable Balance: \n");
		strcpy(str,"------------------\nMini Statement: \n");
		if(strcmp(buff,"exit")==0)
			break;
		else
		{
			int choice=atoi(buff);
			if(choice==1)
			{
				strcat(bal,getBalanceAll());
				sendMsgtoClient(client_fd,strcat(bal,"\n--------------------\n\nEnter your choice\n1. Print Balance of all users\n2. Get mini Statement\nWrite exit to terminate"));
			}
			else if(choice==2)
			{
				sendMsgtoClient(client_fd,"Enter Username or exit to terminate");

				while(1)
				{
					buff=recieveMsgFromClient(client_fd);

					if(strcmp(buff,"exit")==0)
					{
						flag=1;
						break;
					}
					else if(checkUser(buff))
					{
						char *username=(char *)malloc(sizeof(char)*40);
						strcpy(username,buff);
						strcat(str,printMiniStatement(username,client_fd));
						sendMsgtoClient(client_fd,strcat(str,"\n--------------------\n\nEnter your choice\n1. Print Balance of all users\n2. Get mini Statement\nWrite exit to terminate"));
						break;
					}
					else
						sendMsgtoClient(client_fd,"Wrong Username. Please enter a valid user");
				}
			}
		}
	}
}

int authorize(char* username,char *password)
{
	printf("Authorizing\n");
	char * line = NULL;
    size_t len = 0;
    ssize_t read;


	FILE *fp=fopen("login_file","r");
	while((read = getline(&line, &len, fp)) != -1) 
	{
		char *token=strtok(line," ");
		if(strcmp(token,username)==0)
		{
			token=strtok(NULL," ");
			if(strcmp(token,password)==0)
			{
				token=strtok(NULL," ");
                if(token[0]=='C')
                {
					fclose(fp);
                    return USER;    //return the user type
                }
                else if(token[0]=='A')
                {
                    fclose(fp);
                    return ADMIN;
                }
                else if(token[0]=='P')
                {
                    fclose(fp);
                    return POLICE;
                }
            }
        }
    }
    if(line!=NULL)
        free(line);

    fclose(fp);
	return UNAUTH_USER;
}





void closeclient(int client_fd,char *str)
{
	sendMsgtoClient(client_fd, str);
    shutdown(client_fd, SHUT_RDWR);
}



void talkToClient(int client_fd)
{
	char *username,*password;
	username=(char *)malloc(100);
	password=(char *)malloc(100);
	int utype;
	
	getupcli(username,password,client_fd);
	utype=authorize(username,password);

	char *str=(char *)malloc(sizeof(char)*60);
	strcpy(str,"Thanks ");

	switch(utype)
	{
		case USER:
			userRequests(username,password,client_fd);
			closeclient(client_fd,strcat(str,username));
			break;
		case ADMIN:
			adminRequests(client_fd);
			closeclient(client_fd,strcat(str,username));
			break;	
		case POLICE:
			policeRequests(client_fd);
			closeclient(client_fd,strcat(str,username));
			break;	
		case UNAUTH_USER:
			closeclient(client_fd,"unauthorised");
			break;
		default:
			closeclient(client_fd,"unauthorised");
			break;
	}
}



int main(int argc,char **argv)
{
	int sock_fd,client_fd,port_no;
	struct sockaddr_in serv_addr, cli_addr;

	memset((void*)&serv_addr, 0, sizeof(serv_addr));
	port_no=atoi(argv[1]);

	sock_fd=socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_port = htons(port_no);         //set the port number
	serv_addr.sin_family = AF_INET;             //setting DOMAIN
	serv_addr.sin_addr.s_addr = INADDR_ANY;     //permits any incoming IP

	if(bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
	    printf("Error on binding.\n");
	    exit(EXIT_FAILURE);
	}
	int reuse=1;
	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	listen(sock_fd, 5); 
	int clisize=sizeof(cli_addr);

	while(1) {
	    //blocking call
	    memset(&cli_addr, 0, sizeof(cli_addr));
	    if((client_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &clisize)) < 0) {
	        printf("Error on accept.\n");
	        exit(EXIT_FAILURE);
	    }

	    switch(fork()) {
	        case -1:
	            printf("Error in fork.\n");
	            break;
	        case 0: {
	            close(sock_fd);
	            talkToClient(client_fd);
	            exit(EXIT_SUCCESS);
	            break;
	        }
	        default:
	            close(client_fd);
	            break;
	    }
	}

}