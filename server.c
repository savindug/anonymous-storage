#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/md5.h>

unsigned char key[MD5_DIGEST_LENGTH];
char f_Name[255];
char * buffer = 0;

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

void write_history(char *filename, char *key){
  FILE *file_ptr;

  file_ptr = fopen("hash_map.txt", "a");

  if (file_ptr==NULL)
  {
    perror("Error opening file");
    fflush(stdout);
  }

  fprintf(file_ptr, "%s %s\n", key, filename);
  
  fclose(file_ptr);

}

int calculate_md5sum(char *filename)
{
  //open file for calculating md5sum
  FILE *file_ptr;

  file_ptr = fopen(filename, "r");

  if (file_ptr==NULL)
  {
    perror("Error opening file");
    fflush(stdout);
    return 1;
  }

  int n;
  MD5_CTX c;
  char buf[512];
  ssize_t bytes;
  unsigned char out[MD5_DIGEST_LENGTH];
  MD5_Init(&c);
  do
  {
    bytes=fread(buf, 1, 512, file_ptr);
    MD5_Update(&c, buf, bytes);
  }while(bytes > 0);

  MD5_Final(out, &c);

  printf("%s ", filename);

  for(n=0; n<MD5_DIGEST_LENGTH; n++){

    sprintf(key + n*2, "%02x", out[n]);
    
    printf("%02x", out[n]);
  }

  printf("\n");

//printf("key: %s\n", key);

fclose(file_ptr);

write_history(filename, key);

  return 0;
}


void write_file(char *payload){

  FILE *fp;
  char fname[255];
 

//generate file name
  snprintf(fname, 255, "storage/file_%d.txt", (int)time(NULL));

  fp = fopen(fname, "w");

  fprintf(fp, "%s", payload);

  fclose(fp);
  
  
  calculate_md5sum(fname);
  

  return;
}

void retrive_filename(char *key){

    char *token;

    char *search = "\0";

    
    char point[128], value[128];    

    FILE *file = fopen ("hash_map.txt", "r" );

    if ( file != NULL )
    {
      char line[128];

      while ( fgets ( line, sizeof(line), file ) != NULL ) {
        fscanf(file, "%s %s",point, value);
        
        if( strcmp(point, key) == 0 ){ 
          strcpy(f_Name, value);
        }

      }

     // printf("%s => %s\n",key, f_Name);

      }
      fclose (file);

      return;
}

void delete_file(char *f_name){
    if (remove(f_name) == 0) 
      printf("Deleted successfully"); 
   else
      printf("Unable to delete the file"); 
}

int main(void)
{
    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    char server_message[2000], client_message[2000];
    
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));
    
    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    
    char command[10];
    char payload[255];
    
    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");
    
    

    // Listen for clients:
    if(listen(socket_desc, 1) < 0){
        printf("Error while listening\n");
        return -1;
    }
    printf("\nListening for incoming connections.....\n");
    
    // Accept an incoming connection:
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);
    
    if (client_sock < 0){
        printf("Can't accept\n");
        return -1;
    }
    printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    while(1){

         strcpy(client_message, "");
        strcpy(command, "");
        strcpy(payload, "");
        strcpy(server_message, "");
        strcpy(f_Name, "");
        strcpy(key, "");
        //strcpy(buffer, "");
    
    // Receive client's message:
    if (recv(client_sock, client_message, sizeof(client_message), 0) < 0){
        printf("Couldn't receive\n");
        return -1;
    }
    printf("Msg from client: \n",);

    
    int count = 0;
    int fp_count = 0;

    for(int i=1; i<10; i++){

        if(client_message[i] != '#'){
            
            command[count++] = client_message[i];
           
        }

    }

    for(int i=10; i < strlen(client_message); i++){
        payload[fp_count++] = client_message[i];
    }

    //printf("[COMMAND RECIEVED] %s\n", command);

    if( strcmp(command, "STORE") == 0 ){
        
     printf("[ACTIVE COMMAND: %s] %s\n", command, payload);

        write_file(payload);
        printf("[+]Data written in the file successfully %s.\n", key);
         strcpy(server_message, key);

    }else if( strcmp(command, "GET") == 0 ){
        
        printf("[ACTIVE COMMAND: %s] %s\n", command, payload);
        retrive_filename(payload);


        if(strlen(f_Name) <= 0){
           strcpy(server_message, "GET: Error! Hash key is not valid"); 
        }else{
            printf("[+]File Retrived successfully :: %s.\n", f_Name);
            read_file(f_Name);
            strcpy(server_message, buffer);
        }


    }else if( strcmp(command, "DELETE") == 0 ){
        printf("[ACTIVE COMMAND: %s] %s\n", command, payload);
        retrive_filename(payload);
        
        if(strlen(f_Name) <= 0){
           strcpy(server_message, "DELETE: Error! Hash key is not valid"); 
        }else{
            delete_file(f_Name);
            printf("[+]File Deleted successfully :: %s.\n", f_Name);
            strcpy(server_message, "DELETE: File with hash key has been deleted");
        }

    }

    // Respond to client:
    if (send(client_sock, server_message, strlen(server_message), 0) < 0){
        printf("Can't send\n");
        return -1;
    }

   

    }
    
    // Closing the socket:
    close(client_sock);
    close(socket_desc);
    
    return 0;
}