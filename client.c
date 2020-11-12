#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <stdlib.h>

char * buffer = 0;
int get = 0;

void write_file(char *key, char *payload){

  FILE *fp;
 char fname[255];
 

//generate file name
   snprintf(fname, 255, "recived/%s.txt", key);

  fp = fopen(fname, "w");

  fprintf(fp, "%s", payload);

  fclose(fp);

  return;
}

void read_file(char *filename){
    
    long length;
    FILE * f = fopen (filename, "rb");
       
        if (f){
            fseek (f, 0, SEEK_END);
            length = ftell (f);
            fseek (f, 0, SEEK_SET);
            buffer = malloc (length);
               
                if (buffer){
                    fread (buffer, 1, length, f);
                }

            fclose (f);
        }
}

int main(void)
{
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000], client_command[2000];

     char filename[128] = "";
    
    // Clean buffers:
    memset(server_message,'\0',sizeof(server_message));
    memset(client_command,'\0',sizeof(client_command));
    
    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return -1;
    }
    
    printf("Socket created successfully\n");
    
    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");


    while(1){

        // Get input from the user:
        printf("Enter Command: ");
        scanf("%s", client_command);

        if( strcmp(client_command, "STORE") == 0 ){

            char init_command[255] = "@STORE####";
            // strcat(init_command, client_command);
            strcpy(client_command, init_command);
            printf("[COMMAND] %s\n", client_command);
            printf("Please Input the File path: ");
            scanf("%s", filename);
            read_file(filename);
            strcat(client_command, buffer);

        }else if( strcmp(client_command, "GET") == 0 ){
            get = 1;
            char init_command[255] = "@GET######";
            // strcat(init_command, client_command);
            strcpy(client_command, init_command);
            printf("[COMMAND] %s\n", client_command);
            printf("Please Input Hash Key: ");
            scanf("%s", filename);
            strcat(client_command, filename);

        }else if( strcmp(client_command, "DELETE") == 0 ){

            char init_command[255] = "@DELETE###";
            // strcat(init_command, client_command);
            strcpy(client_command, init_command);
            printf("[COMMAND] %s\n", client_command);
            printf("Please Input Hash Key: ");
            scanf("%s", filename);
            strcat(client_command, filename);

        }else if( strcmp(client_command, "HISTORY") == 0 ){
            
            char init_command[255] = "@HISTORY##";
            // strcat(init_command, client_command);
            strcpy(client_command, init_command);
            printf("[COMMAND] %s\n", client_command);
            printf("Please Input Hash Key: ");
            scanf("%s", filename);
            strcat(client_command, filename);

        }else if( strcmp(client_command, "QUIT") == 0 ){
            close(socket_desc);
            return -1;
        }

        printf("[COMMAND SENT TO SERVER] %s\n", client_command);

        
        
        // Send the message to server:
        if(send(socket_desc, client_command, strlen(client_command), 0) < 0){
            printf("Unable to send message\n");
            return -1;
        }
        

        // Receive the server's response:
        if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
            printf("Error while receiving server's msg\n");
            return -1;
        }

        
         if(get == 1){
             write_file(filename, server_message);
             printf("File saved as recived/%s.txt on the Client\n", filename);
             get = 0;
         }else
         {
             printf("Server's response: %s\n",server_message);
         }
         

        
        strcpy(filename, "");
        strcpy(client_command, "");
        strcpy(buffer, "");
    
    }   
    
    
    // Close the socket:
    close(socket_desc);
    
    return 0;
}