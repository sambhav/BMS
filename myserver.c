#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define USER 0
#define POLICE 1
#define ADMIN 2
#define UNAUTH_USER -1
#define RESPONSE_BYTES 512
#define REQUEST_BYTES 512
#define linesInMS 5

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


void printMiniStatement(char *username,int client_fd)
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
        sendMsgtoClient(client_fd, "None");
    else
        sendMsgtoClient(client_fd, miniStatement);

    free(miniStatement);
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
    	prevtoken=(char *)malloc(40*sizeof(char));
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
		char *bal;
		switch(choice)
		{
			case 1:
				bal=printBalance(username);
				sendMsgtoClient(client_fd,bal);
				free(bal);
				bal = NULL;
				break;
			case 2:
				printMiniStatement(username,client_fd);
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

void adminRequests(char *username,char *password,int client_fd)
{

}

void policeRequests(char *username,char *password,int client_fd)
{

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


	switch(utype)
	{
		case USER:
			userRequests(username,password,client_fd);
			closeclient(client_fd,"Thanks User");
			break;
		case ADMIN:
			adminRequests(username,password,client_fd);
			closeclient(client_fd,"Thanks Admin");
			break;	
		case POLICE:
			policeRequests(username,password,client_fd);
			closeclient(client_fd,"Thanks Police");
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