#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <ftw.h>
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
char currDB[100];
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
        isSudo = false;
        memset(username,0,sizeof(username));
        memset(currDB,0,sizeof(currDB));
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
        isSudo = false;
        memset(username,0,sizeof(username));
        memset(username,0,sizeof(password));
        memset(currDB,0,sizeof(currDB));
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
    DIR* dir = opendir(path);
    if (dir) {
        sends("Database already exist, please use another name!");
    }
    else {
        mkdir(path, 0777);
        if(!isSudo){
            FILE* permission = fopen("/home/bayu/Documents/FP/Database/permissions.txt","a");
            char permissions [300];
            sprintf(permissions,"%s:%s:%s",username,password,dbName);
            fputs(permissions,permission);
            fputs("\n",permission);
            fclose(permission);
        }
        sends("Database sucessfully created!");
    }
}

void useDB(char dbName[]){
    char permissions [300];
    char sudoGranted [300];
    bool dbFlag = false;
    char path[200];
    sprintf(path,"%s/%s",databasePath,dbName);
    sprintf(permissions,"%s:%s:%s\n",username,password,dbName);
    sprintf(sudoGranted,"sudoGranted:%s:%s\n",username,dbName);
    FILE* fileR = fopen("/home/bayu/Documents/FP/Database/permissions.txt","r");
    char checker[300] = {0};
    if( access( path, F_OK ) == 0 && isSudo) {
        dbFlag = true;
        strcpy(currDB,dbName);
    } 
    else {
        while(fgets(checker,1024,fileR)!=NULL){
            if(strcmp(permissions,checker)==0){
                strcpy(currDB,dbName);
                dbFlag = true;
                break;
            }
            else if(strcmp(sudoGranted,checker)==0){
                strcpy(currDB,dbName);
                dbFlag = true;
                break;
            }
        }
    }
    if(dbFlag) {
        sends("Use command acceped!");
    }
    else {
        sends("You don't have permission for that database or that database doesn't exist!");
    }
}

void grantPermission(char dbName[],char userGranted[]){
    FILE* permission = fopen("/home/bayu/Documents/FP/Database/permissions.txt","a");
    char permissions [300];
    sprintf(permissions,"sudoGranted:%s:%s",userGranted,dbName);
    fputs(permissions,permission);
    fputs("\n",permission);
    fclose(permission);
    sends("Permission sucessfully granted!");
}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

int rmrf(char *path)
{
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

void dropDB(char dbName[]){
    char permissions [300];
    char sudoGranted [300];
    bool dbFlag = false;
    char path[200];
    sprintf(path,"%s/%s",databasePath,dbName);
    sprintf(permissions,"%s:%s:%s\n",username,password,dbName);
    sprintf(sudoGranted,"sudoGranted:%s:%s\n",username,dbName);
    FILE* fileR = fopen("/home/bayu/Documents/FP/Database/permissions.txt","r");
    char checker[300] = {0};
    if( access( path, F_OK ) == 0 && isSudo) {
        dbFlag = true;
        strcpy(currDB,dbName);
    } 
    else {
        while(fgets(checker,1024,fileR)!=NULL){
            if(strcmp(permissions,checker)==0){
                strcpy(currDB,dbName);
                dbFlag = true;
                break;
            }
            else if(strcmp(sudoGranted,checker)==0){
                strcpy(currDB,dbName);
                dbFlag = true;
                break;
            }
        }
    }
    if(dbFlag||isSudo){
        DIR* dir = opendir(path);
        if (dir) {
            closedir(dir);
            rmrf(path);
            sends("Database dropped!");
        }
        else {
            sends("Database not found :(");
        }
    }
    else {
        sends("You got no permission to be dropping other's files foo >:(");
    }
}

void deleteTable(char tableName[]){
    char path[200];
    sprintf(path,"%s/%s/%s",databasePath,currDB,tableName);
    printf("%s\n",path);
    if( access( path,F_OK ) == 0 ) {
        FILE * file = fopen(path,"w");
        fclose(file);
        sends("Table deleted!");
    } else {
        sends("Table not found :(");
    }
}

void createTable(char tableName[],char tableArgs[]){
    char path[300];
    sprintf(path,"%s/%s/%s",databasePath,currDB,tableName);
    if( access( path,F_OK ) == 0 ) {
        sends("Table already exists!");
    } else {
        char parsedArgs[200]={0};
        int counter = 0;
        printf("tableArgs = %ld\n",strlen(tableArgs));
        char * ptr = tableArgs;
        for (unsigned int i = 0; i<strlen(tableArgs);i++){
            char temp[100];
            char temp2[102];
            printf("i = %d = %c\n",i,tableArgs[i]);
            if (tableArgs[i]!=' '&& tableArgs[i]!=','){
                sscanf(ptr,"%s",temp);
                printf("temp:%s\n",temp);
                if(temp[strlen(temp)-1]==','){
                    temp[strlen(temp)-1] = '\t';
                }
                if(counter == 1){
                    sprintf(temp2," %s",temp);
                    strcat(parsedArgs,temp2);
                    counter=0;
                }
                else {
                    strcat(parsedArgs,temp);
                    counter++;
                }
                i+= strlen(temp)-1;
                ptr+= strlen(temp)-1;
                printf("i = %d = %c\n",i,tableArgs[i]);
            }
            ptr++;
        }
        printf("%s\n",parsedArgs);
        FILE* table = fopen(path,"a");
        fputs(parsedArgs,table);
        fputs("\n",table);
        fclose(table);
        sends("Table created");
    }
}

void insertInto(char tableName[], char tableArgs[]){
    char path[300];
    sprintf(path,"%s/%s/%s",databasePath,currDB,tableName);
    if( access( path,F_OK ) != 0 ) {
        sends("Table doesn't exists!");
    } else {
        char parsedArgs[200]={0};
        int counter = 0;
        printf("tableArgs = %ld\n",strlen(tableArgs));
        char * ptr = tableArgs;
        FILE* table = fopen(path,"r");
        char tableVar [200];
        fgets(tableVar,200,table);
        for (unsigned int i = 0; i<strlen(tableArgs);i++){
            char find[100];
            char value[102];
            printf("i = %d = %c\n",i,tableArgs[i]);
            if (tableArgs[i]!=' '&& tableArgs[i]!=',' && tableArgs[i]!='\''){
                sscanf(ptr,"%s",find);
                for (unsigned int j = 0; j<strlen(find);j++){
                    if(find[j]==' '|| find[j]=='\''|| find[j]==','){
                        find[j]=0;
                    }
                    i++;
                    ptr++;
                }
                printf("colName:%s\n",find);
                printf("colLength: %ld\n",strlen(find));
                printf("rest: %s\n",ptr);
                printf("idx = %d\n",i);
                while((tableArgs[i]==' ' || tableArgs[i]==',' || tableArgs[i]=='\'')){
                    ptr++;
                    i++;
                }
                sscanf(ptr,"%s",value);
                for (unsigned int j = 0; j<strlen(value);j++){
                    if(value[j]==' '|| value[j]=='\''|| value[j]==','){
                        value[j]=0;
                    }
                    ptr++;
                    i++;
                }
                printf("val = %s\n",value);
                if(strstr(tableVar,find)!=NULL){
                    printf("Gocha!\n");
                    strcat(parsedArgs,find);
                    strcat(parsedArgs," ");
                    strcat(parsedArgs,value);
                    strcat(parsedArgs,"\t");
                }
                printf("i = %d = %c\n",i,tableArgs[i]);
            }
            ptr++;
        }
        fclose(table);
        printf("%s\n",parsedArgs);
        table = fopen(path,"a");
        parsedArgs[strlen(parsedArgs)-1] = '\n';
        fputs(parsedArgs,table);
        fclose(table);
        sends("Insert successful");
    }
}

void dropTable(char tableName[]){
    char path[200];
    sprintf(path,"%s/%s/%s",databasePath,currDB,tableName);
    printf("%s\n",path);
    if( access( path,F_OK ) == 0 ) {
        remove(path);
        sends("Table deleted!");
    } else {
        sends("Table not found :(");
    }
}

void dropColumn(char colName[],char tableName[]){
    char path[200];
    char path2[200];
    sprintf(path,"%s/%s/%s",databasePath,currDB,tableName);
    FILE * file = fopen(path,"r");
    sprintf(path2,"%s/%s/temp",databasePath,currDB);
    FILE * temp = fopen(path2,"w");
    char find[1024];
    int counter = 0;
    bool found = false;
    while (fgets(find,1024,file)) {
        if(counter!=0 && found!=true){
            break;
        }
        printf("Find: %s\n",find);
        char write[1024]={0};
        char * ptr = strstr(find,colName);
        printf("Ptr: %s\n",ptr);
        if(ptr){
            if (counter == 0){
                found = true;
            }
            for (unsigned int i =0;i<strlen(find)-strlen(ptr);i++){
                write[i] = find[i];
                printf("W: %s\n",write);
            }
            printf("W: %s\n",write);
            ptr = strchr(ptr,'\t');
            if(ptr!=NULL){
                ptr++;
                printf("Ptr: %s\n",ptr);
                strcat(write,ptr);
                printf("Full W: %s\n",write);
                if(write[strlen(write)-1]!='\n'){
                    write[strlen(write)] ='\n';
                    write[strlen(write)+1] = 0;
                }
                fputs(write,temp);
            }
            else {
                fputs(write,temp);
                fputs("\n",temp);
            }
        }
        else{
            fputs(find,temp);
        }
        counter++;
    }
    fclose(file);
    fclose(temp);
    remove(path);
    rename(path2,path);
    if (found){
        sends("Dropped sucessfully");
    }
    else {
        sends("Column not found");
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
                if (strcmp(arg1,"create")==0 && strcmp(arg2,"table")==0){
                    if(currDB==NULL){
                        sends("Please use the \"Use\" Command before creating a table\n");
                        continue;
                    }
                    unsigned int len = strlen(arg1) + strlen(arg2) + 2;
                    char * ptr = command;
                    ptr+=len;
                    printf("%s\n",ptr);
                    char tableName[100];
                    char tableArg[200];    
                    sscanf(ptr,"%[^\t]",tableName);
                    ptr+=strlen(tableName)+1;
                    createTable(tableName,ptr);
                }
                if (strcmp(arg1,"use")==0){
                    useDB(arg2);
                }
                if (strcmp(arg1,"grant")==0 && strcmp(arg2,"permission")==0){
                    unsigned int len = strlen(arg1) + strlen(arg2) + 2;
                    char * ptr = command;
                    ptr+=len;
                    char dbName[100];
                    char grantUser[100];
                    sscanf(ptr,"%[^\t]\t%s",dbName,grantUser);
                    grantPermission(dbName,grantUser);
                }
                if (strcmp(arg1,"insert")==0 && strcmp(arg2,"into")==0){
                    if(currDB==NULL){
                        sends("Please use the \"Use\" Command before creating a table\n");
                        continue;
                    }
                    unsigned int len = strlen(arg1) + strlen(arg2) + 2;
                    char * ptr = command;
                    ptr+=len;
                    printf("%s\n",ptr);
                    char tableName[100];
                    char tableArg[200];    
                    sscanf(ptr,"%[^\t]",tableName);
                    ptr+=strlen(tableName)+1;
                    insertInto(tableName,ptr);
                }
                if (strcmp(arg1,"drop")==0 && strcmp(arg2,"database")==0){
                    unsigned int len = strlen(arg1) + strlen(arg2) + 2;
                    char * ptr = command;
                    ptr+=len;
                    printf("%s\n",ptr);
                    char dbName[100];
                    sscanf(ptr,"%s",dbName);
                    dropDB(dbName);
                }
                if (strcmp(arg1,"drop")==0 && strcmp(arg2,"table")==0){
                    unsigned int len = strlen(arg1) + strlen(arg2) + 2;
                    char * ptr = command;
                    ptr+=len;
                    printf("%s\n",ptr);
                    char tableName[100];
                    sscanf(ptr,"%s",tableName);
                    dropTable(tableName);
                }
                if (strcmp(arg1,"drop")==0 && strcmp(arg2,"column")==0){
                    unsigned int len = strlen(arg1) + strlen(arg2) + 2;
                    char * ptr = command;
                    ptr+=len;
                    printf("%s\n",ptr);
                    char colName[100];
                    char tableName[100];
                    sscanf(ptr,"%[^\t]\t%s",colName,tableName);
                    dropColumn(colName,tableName);
                }
                if (strcmp(arg1,"delete")==0 && strcmp(arg2,"from")==0){
                    unsigned int len = strlen(arg1) + strlen(arg2) + 2;
                    char * ptr = command;
                    ptr+=len;
                    printf("%s\n",ptr);
                    char tableName[100];
                    deleteTable(tableName);
                }
            }   
        }
    return 0;
}
