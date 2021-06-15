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
#define PORT 8080

char sent[1024];
char recieve[1024];
char buff[1024];
bool loggedIn=false;
int soc;

bool isSudo = false; 
char username[100];
char password[100];
int checker=0;
void Reg() {
    read(soc,recieve,1024);
    printf("%s\n",recieve);
    memset(recieve,0,sizeof(recieve));
    char uname[100];
    char pass[100];
    scanf("%s %s",uname,pass);
    sprintf(sent,"%s:%s\n",uname,pass);
    send(soc,sent,strlen(sent),0);
    memset(sent,0,sizeof(sent));
    read(soc,recieve,1024);
    printf("%s\n",recieve);
    memset(recieve,0,sizeof(recieve));
}


void Log() {
    read(soc,recieve,1024);
    printf("%s\n",recieve);
    memset(recieve,0,sizeof(recieve));
    char uname[100];
    char pass[100];
    scanf("%s %s",uname,pass);
    sprintf(sent,"%s:%s\n",uname,pass);
    send(soc,sent,strlen(sent),0);
    memset(sent,0,sizeof(sent));
    read(soc,recieve,1024);
    printf("%s\n",recieve);
    if(recieve[0]=='L'){
        loggedIn=true;
    }
    memset(recieve,0,sizeof(recieve));
}

void resR() {
    read(soc,recieve,1024);
    printf("%s\n",recieve);
    memset(recieve,0,sizeof(recieve));
}

void sends(char data[]) {
    send(soc,data,strlen(data),0);
    memset(sent,0,sizeof(sent));
}

void addFiles() {
    char temp[1024];
    for (int i=0;i<3;i++) {
        resR();
        scanf("%s",temp);
        temp[strcspn(temp,"\n")] =0;
        sends(temp);
    }
    FILE *sfd = fopen(temp,"rb");  
    char data[1024] = {0};
    
    while(1){
        memset(data,0,1024);
        size_t size = fread(data,sizeof(char),1024,sfd);
        send(soc,data,1024,0);
        break;
    }
    printf("break"); 
    fclose(sfd);
    resR();
}

void download() {
    resR();
    char temp[1024];
    scanf("%s",temp);
    temp[strcspn(temp,"\n")] =0;
    sends(temp);
    read(soc,recieve,1024);
    printf("%s\n",recieve);
    if (recieve[0]=='F') {
        char dir[300] = "/home/bayu/Documents/Prak3/";
        strcat(dir,temp);
        FILE *file = fopen(dir,"w");
        char buffer[4096]={0};
        while (1) {
            memset(buffer,0,sizeof(buffer));
            int len = read(soc,buffer,4096); 
            fprintf(file,"%s",buffer);
            break;
        }
        printf("break\n");
        fclose(file);
    }
}

void delete() {
    resR();
    char temp[1024];
    scanf("%s",temp);
    temp[strcspn(temp,"\n")]=0;
    sends(temp);
    resR();
}

void see(){
    char bigbuff[10000];
    read (soc,bigbuff,10000);
    printf("%s\n",bigbuff);
    memset(bigbuff,0,sizeof(bigbuff));
    
}

void find(){
    printf("Tulis nama file anda\n");
    char find[200] = {0};
    scanf("%s",find);
    find[strcspn(find,"\n")]=0;
    sends(find);
    resR();
}

void lower(char arr[]){
    for (int i = 0; i<strlen(arr);i++){
        tolower(arr[i]);
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

    while(1){
        char command[100];
        scanf("%s",command);
        lower(command);
        if (strcmp(command,"create")==0){
            char create[100];
            scanf("%s",create);
            lower(create);
            if (strcmp(command,"user")==0){
                char newUsername[100];
                char newPassword[100];
                scanf("%s",newUsername);
                char passwordBuffer[100];
                fgets(passwordBuffer,1024,stdin);
                char * ptr = strstr(passwordBuffer,"IDENTIFIED BY");
                ptr++;
                strcpy(newPassword,passwordBuffer);
                sprintf(sent,"%s %s",newUsername,newPassword);
                sends(sent);
            }
            if (strcmp(command,"database")==0){
                char create[100];
                scanf("%s",create);
            }
            if (strcmp(command,"table")==0){
                char create[100];
                scanf("%s",create);
            }
        }
    }
    return 0;
}
