#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>
#define PORT 8080

char sent[1024];
char username[100];
char password[100];
bool isSudo = false;
char recieve[1024];
char buff[1024];
char user[30];
char upass[100];
char command[1000];
bool connected = false;
const char databasePath[100] = "/home/bayu/Documents/FP/Database";
int master_socket , new_socket , client_socket[30]= {0},max_clients = 30 , activity,max_sd, i , valread , sd; // GFG
fd_set readfds;
bool loggedIn = false;


void resR() {
    memset(recieve,0,sizeof(recieve));
}

void reads() { // check Disconnects + Read vals
    resR();
    int check;
    if ((check = read(sd,recieve,1024)) == 0){
        connected=false;
        loggedIn=false;
        isSudo = false;
        memset(username,0,sizeof(username));
        memset(username,0,sizeof(password));
        close(sd);
        for(i=0;i<30;i++)
        {
            client_socket[i]=client_socket[i+1];
        }
        client_socket[29]=0;
    }
}

void readsCommand() { // check Disconnects + Read vals
    memset(command,0,sizeof(command));
    int check;
    if ((check = read(sd,command,100)) == 0){
        connected=false;
        loggedIn=false;
        isSudo = false;
        memset(username,0,sizeof(username));
        memset(username,0,sizeof(password));
        close(sd);;
        for(i=0;i<30;i++)
        {
            client_socket[i]=client_socket[i+1];
        }
        client_socket[29]=0;
    }
}

void sends(char data[]) {
    send(sd,data,strlen(data),0);
    memset(sent,0,sizeof(sent));
}

void createUser(char newUsername[],char newPassword[]){
    FILE* file = fopen("/home/bayu/Documents/FP/Database/users.txt","a");
    char idpass [200];
    sprintf(idpass,"%s:%s",newUsername,newPassword);
    fputs(idpass,file);
    fputs("\n",file);
    fclose(file);
}
void createDB(char dbName[]){
    char path[200];
    sprintf(path,"%s/%s",databasePath,dbName);
    if( access( path, F_OK ) == 0 ) {
        sends("Database already exist, please use another name!\n");
    } 
    else {
        FILE* dbFile = fopen(path,"a");
        fclose(dbFile);  
        FILE* permission = fopen("/home/bayu/Documents/FP/Database/permissions.txt","a");
        char permissions [300];
        sprintf(permissions,"%s:%s:%s",username,password,dbName);
        fputs(permissions,permission);
        fputs("\n",permission);
        fclose(permission);  
    }

}

int main(int argc, char const *argv[]) {  
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &opt,sizeof(opt)) <0 ) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(master_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while(1) {
        //clear the socket set 
        FD_ZERO(&readfds);  
     
        //add master socket to set 
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  
             
        //add child sockets to set 
        for ( i = 0 ; i < max_clients ; i++)  
        {  
            //socket descriptor 
            sd = client_socket[i];  
                 
            //if valid socket descriptor then add to read list 
            if(sd > 0)  
                FD_SET( sd , &readfds);  
                 
            //highest file descriptor number, need it for the select function 
            if(sd > max_sd)  
                max_sd = sd;  
        } 
         //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
       
        if ((activity < 0) && (errno!=EINTR))  
        {  
            printf("select error");  
        }  
             
        //If something happened on the master socket , 
        //then its an incoming connection 
        if (FD_ISSET(master_socket, &readfds))  
        {  
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }
             for (i = 0; i < max_clients; i++)  
            {  
                //if position is empty 
                if( client_socket[i] == 0 )  
                {  
                    client_socket[i] = new_socket;  
                    printf("Adding to list of sockets as %d\n" , i);  
                        
                    break;  
                }  
            }  
        }
        sd = client_socket[0];
        connected = true;
        reads();
        sscanf(recieve,"%[^\t]\t%s",username,password);
        if(strlen(username) == 0 && strlen(password) == 0){
            isSudo = true;
        }
        if(!isSudo){
            printf("%s:%s has connected\n",username,password);
        }
        else {
            printf("Sudo has connected, All hail sudo!\n");
        }
        while(connected) {
                readsCommand();
                printf("Command: %s\n",command);
                char arg1[100];
                char arg2[100];
                sscanf(command,"%[^\t]\t%s",arg1,arg2);
                printf("Args: %s\n%s\n",arg1,arg2);
                if (strcmp(arg1,"create")==0 && strcmp(arg2,"user")==0){
                    unsigned int len = strlen(arg1) + strlen(arg2) + 2;
                    char * ptr = command;
                    ptr+=len;
                    printf("%s\n",ptr);
                    char newUsername[100];
                    char newPassword[100];    
                    sscanf(ptr,"%[^\t]\t%s",newUsername,newPassword);
                    printf("Username:%s\tPassword:%s\n",newUsername,newPassword);
                    createUser(newUsername,newPassword);
                }
                if (strcmp(arg1,"create")==0 && strcmp(arg2,"database")==0){
                    unsigned int len = strlen(arg1) + strlen(arg2) + 2;
                    char * ptr = command;
                    ptr+=len;
                    printf("%s\n",ptr);
                    char dbName[100];
                    sscanf(ptr,"%s",dbName);
                    createDB(dbName);
                }
            }   
        }
    return 0;
}
