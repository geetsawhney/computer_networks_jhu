#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<errno.h>
#include<string.h>
#include<arpa/inet.h>
#include<time.h>
#include<sys/types.h>
#include<sys/socket.h>


int main(int argc, char **argv){

	if(argc != 4 && argc != 8){
		printf("Error: missing or additional arguments");
		return 0;
	}

	char mode[strlen(argv[1])];
	strcpy(mode, argv[1]);
	int port = atoi(argv[3]);

	if(strcmp(mode, "-s") == 0){
		serverMode(argc, argv);
	}
	else if(strcmp(mode, "-c") == 0){
		clientMode(argc, argv);
	}
	else{
		printf("Illegal mode");
	}

}

int serverMode(int argc, char **argv){

    printf("Running in server mode\n");

	if(argc != 4){
		printf("Error: missing or additional arguments\n");
		return 0;
	}
	
	
//	printf("check 1\n");



	int port = atoi(argv[3]);
	if(!(port >= 1024 && port <= 65535)){
		printf("Error: port number must be in the range 1024 to 65535\n");
		return 0;
	}
	
	
//	printf("check 2\n");


	int sock, len, cli, sent;
	
	struct sockaddr_in server, client;
	char message[] = "Connected\n";

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket: \n");
		exit(-1);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&server.sin_zero, 8);

	
//	printf("check 3\n");



	len = sizeof(struct sockaddr_in);

	if((bind(sock, (struct sockaddr *)&server, len)) == -1){
		perror("bind: \n");
		exit(-1);
	}



//	printf("check 4\n");


	if((listen(sock, 5)) == -1){
		perror("listen: \n");
		exit(-1);
	}


	
//	printf("check 5\n");


	long bytesReceived = 0;
	int valRead = 0, n;
    char messageReceived[1001] = {0};
    clock_t start, end;
	while(1){
		if((cli = accept(sock, (struct sockaddr *)&client, &len)) == -1){
			perror("accept: \n");
			exit(-1);
		}


//		printf("check 6\n");


		if((n = recv(cli, messageReceived, 1000, 0)) == -1){
		    perror("Receive failed\n");
		    exit(-1);
		}
		
		
//		printf("check 7\n");


		start = clock();
		while(n > 0 && strchr(messageReceived,'e')==NULL){
		    bytesReceived = bytesReceived + n;
		    sent = send(cli, "Ok", strlen("Ok"), 0);
		    if((n = recv(cli, messageReceived, 1000, 0)) == -1){
		        perror("Receive failed\n");
		        exit(-1);
		    }
		}
		
//		printf("check 8\n");

      	  sent = send(cli, "Bye", strlen("Bye"), 0);

      	  end = clock();
      	  long total = bytesReceived/1000;
      	  double secs = (double)(end - start)/CLOCKS_PER_SEC;
      	  double rate = total*8/(secs * 1000);
      	  printf("received=%ldKB, rate=%lfMbps\n", total, rate);
        
      	  close(sock);
      	  return 0;
	}
}

int clientMode(int argc, char **argv){

	printf("Running in client mode\n");

	if(argc != 8){
		printf("Error: missing or additional arguments\n");
		return 0;
	}


//	printf("check 1\n");


	int server_port = atoi(argv[5]);
	if(!(server_port >= 1024 && server_port <= 65535)){
		printf("Error: port number must be in the range 1024 to 65535\n");
		return 0;
	}

//	printf("check 2\n");


	int time = atoi(argv[7]);
	struct sockaddr_in client, server;
	int sock = 0, valRead, len;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
	    perror("socket: \n");
	    exit(-1);
	}


//	printf("check 3\n");



	server.sin_family = AF_INET;
	server.sin_port = htons(server_port);

	if(inet_pton(AF_INET, argv[3], &server.sin_addr) == -1){
	    perror("Unable to translate server address\n");
	    exit(-1);
	}

	len = sizeof(server);
	
	
//	printf("check 4\n");

	
	
	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) == -1){
	    perror("Can't connect to the specified server\n");
	    exit(-1);
	}



//	printf("check 5\n");


	clock_t start, end;
	double time_elapsed;

	char messageSent[1000];
	char messageReceived[1000] = {0};
	
	memset(messageSent,'0',1000);

	long bytesSent = 0;
	start = clock();
	
	while(time_elapsed <= (double)time){
	
	    if(send(sock, messageSent, strlen(messageSent), 0) == -1){
	        perror("Error sending data to the server");
	        exit(-1);
	    }
	
	    bytesSent += strlen(messageSent);
	    valRead = recv(sock, messageReceived, 1000, 0);
	    end = clock();
	    time_elapsed = (double)(end - start)/CLOCKS_PER_SEC;

		//printf("time elaPSED= %lf \n",time_elapsed);

	}
	
	
	
//	printf("check 6\n");


	send(sock, "Tatae", strlen("Tatae"), 0);
	char finalMessage[1000];
	valRead = recv(sock, finalMessage, 1000, 0);
	end=clock();
	time_elapsed = (double)(end - start)/CLOCKS_PER_SEC;

	long total = bytesSent/1000;
	double rate = total*8/(time_elapsed*1000);

	printf("sent=%ldKB, rate=%lfMbps\n", total, rate);
	close(sock);

	return 0;

}
