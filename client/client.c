#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <fcntl.h>
#include <ctype.h>
#define PORT 8080

char sent[1024];
char recieve[1024];
char buff[1024];
bool loggedIn=false;
int soc;
bool useFlag = false;
bool isSudo = false; 
char username[100];
char password[100];
int checker=0;

void reads() {
    read(soc,recieve,1024);
    printf("%s\n",recieve);
    memset(recieve,0,sizeof(recieve));
}

void sends(char data[]) {
    send(soc,data,strlen(data),0);
    memset(sent,0,sizeof(sent));
}



void lower(char arr[]){
    for (int i = 0; i<strlen(arr);i++){
        arr[i] = tolower(arr[i]);
    }
}

int main(int argc, char const *argv[]) {
    for (int i = 0; i<argc; i++) {
        if (strcmp(argv[i],"-u")==0){
            i++;
            checker++;
            strcpy(username,argv[i]);
        }
        if (strcmp(argv[i],"-p")==0){
            i++;
            checker++;
            strcpy(password,argv[i]);
        }
    }
    printf("Username: %s\nPassword: %s\n",username,password);
    if(geteuid() == 0){
        isSudo = true;
        printf("SUDO!\n");
    }
    if (isSudo!=true && checker!=2){
        printf("Supply Username and Password or use Sudo!\n");
    }
    struct sockaddr_in address;
    struct sockaddr_in serv_addr;
    if ((soc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(soc, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    
    memset(sent,0,sizeof(sent));
    sprintf(sent,"%s\t%s",username,password);
    printf("%s\n",sent);
    sends(sent);

    while(1){
        char command[100];
        scanf("%s",command);
        lower(command);
        printf("Command: %s\n",command);
        //sends(command);
        if (strcmp(command,"create")==0){
            char create[100];
            scanf("%s",create);
            lower(create);  
            //sends(create);
            printf("Create Type: %s\n",create);
            if (strcmp(create,"user")==0){
                char newUsername[100];
                scanf("%s",newUsername);
                char passwordBuffer[100];
                fgets(passwordBuffer,100,stdin);
                char * ptr = strstr(passwordBuffer,"IDENTIFIED BY");
                ptr+=14;
                printf("%s\n",ptr);
                if (ptr==NULL){
                    printf("Wrong command!\n");
                    continue;
                }
                sprintf(sent,"%s\t%s\t%s\t%s",command,create,newUsername,ptr);
                printf("Parsed Command: %s\n",sent);
                if(isSudo!=true){
                    printf("Please use sudo!\n");
                    continue;
                }
                sends(sent);
                reads();
            }
            else if (strcmp(create,"database")==0){
                char dbName[100];
                scanf("%s",dbName);
                sprintf(sent,"%s\t%s\t%s",command,create,dbName);
                sends(sent);
                reads();
            }
            else if (strcmp(create,"table")==0){
                char tableName[100];
                scanf("%s",tableName);
                char argBuffer[200];
                fgets(argBuffer,200,stdin);
                char * ptr = strchr(argBuffer,'(');
                ptr++;
                ptr[strcspn(ptr,")")]=0;
                sprintf(sent,"%s\t%s\t%s\t%s",command,create,tableName,ptr);
                sends(sent);
                reads();
            }
            else{
                printf("Wrong command!\n");
                continue;
            }
        }
        else if (strcmp(command,"use")==0){
            char dbName[100];
            scanf("%s",dbName);
            sprintf(sent,"%s\t%s",command,dbName);
            sends(sent);
            reads();
            useFlag = true;
        }
        else if (strcmp(command,"grant")==0){
            char permi[100];
            scanf("%s",permi);
            lower(permi);
            if (strcmp(permi,"permission")==0){
                char dbName[100];
                scanf("%s",dbName);
                char intoBuffer[100];
                fgets(intoBuffer,100,stdin);
                char * ptr = strstr(intoBuffer,"INTO");
                ptr+=5;
                printf("%s\n",ptr);
                if (ptr==NULL){
                    printf("Wrong command!\n");
                    continue;
                }
                sprintf(sent,"%s\t%s\t%s\t%s",command,permi,dbName,ptr);
                if(isSudo!=true){
                    printf("Please use sudo!\n");
                    continue;
                }
                sends(sent);
                reads();
            }
        }
        else if (strcmp(command,"insert")==0 && useFlag == true){
            char into[100];
            scanf("%s",into);
            lower(into);
            if (strcmp(into,"into")==0){
                char tableName[100];
                scanf("%s",tableName);
                char argBuffer[200];
                fgets(argBuffer,200,stdin);
                char * ptr = strchr(argBuffer,'(');
                ptr++;
                ptr[strcspn(ptr,")")]=0;
                printf("%s\n",ptr);
                sprintf(sent,"%s\t%s\t%s\t%s",command,into,tableName,ptr);
                sends(sent);
                reads();
            }
        }
        else if (strcmp(command,"drop")==0){
            char create[100];
            scanf("%s",create);
            lower(create);  
            if (strcmp(create,"database")==0){
                char dbName[100];
                scanf("%s",dbName);
                sprintf(sent,"%s\t%s\t%s",command,create,dbName);
                sends(sent);
                reads();
            }
            else if (strcmp(create,"table")==0 && useFlag == true){
                char tableName[100];
                scanf("%s",tableName);
                sprintf(sent,"%s\t%s\t%s",command,create,tableName);
                sends(sent);
                reads();
            }
            else if (strcmp(create,"column")==0 && useFlag == true){
                char colName[100];
                scanf("%s",colName);
                char fromBuffer[100];
                fgets(fromBuffer,100,stdin);
                char * ptr = strstr(fromBuffer,"FROM");
                ptr+=5;
                printf("%s\n",ptr);
                if (ptr==NULL){
                    printf("Wrong command!\n");
                    continue;
                }
                sprintf(sent,"%s\t%s\t%s\t%s",command,create,colName,ptr);
                sends(sent);
                reads();
            }
            else {
                printf("Wrong command\n");
                continue;
            }
        }
        else if (strcmp(command,"delete")==0){
            char create[100];
            scanf("%s",create);
            lower(create);
            printf("%s\n",create);
            if (strcmp(create,"from")==0 && useFlag == true){
                char tableName[100];
                scanf("%s",tableName);
                sprintf(sent,"%s\t%s\t%s",command,create,tableName);
                sends(sent);
                reads();
            }
            else {
                printf("Wrong command\n");
                continue;
            }
        }
    }
    return 0;
}
