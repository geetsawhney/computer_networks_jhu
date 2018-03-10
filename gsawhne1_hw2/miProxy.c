#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>
#include<time.h>


void executeProxyServer(char* , float , int , char* );
void modifyRequest(char *,char**);
void allBitrates(char* , int* );
int getContentLength(char*);
int getClosest(int* , double ,double , float );

int main(int argc, char **argv) {

	if (argc != 5) {
		printf("Error: missing or additional arguments");
		exit(-1);
	}


	char log[strlen(argv[1])];
	strcpy(log, argv[1]);
	float alpha = atof(argv[2]);
	int listen_port = atoi(argv[3]);
	char www_ip[strlen(argv[4])];
	strcpy(www_ip, argv[4]);
	executeProxyServer(log,alpha,listen_port,www_ip);

	return 0;
}

void executeProxyServer(char* log, float alpha, int listen_port, char* www_ip){

	struct sockaddr_in cli_addr, proxy_serv_addr, proxy_cli_addr;
	int pr_serv_sock, browser_sock,pr_cli_sock;
	static int xmlFlag=0;
	int T_cur=10,T_new;
	clock_t start, end;
	double time_elapsed;
	int contentLength;
	FILE* fileptr;
	fileptr=fopen(log,"w");
	int average=0;
	int count=0;

	if ((pr_serv_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket: \n");
		exit(-1);
	}

	bzero((char*) &proxy_serv_addr, sizeof(proxy_serv_addr));
	proxy_serv_addr.sin_family = AF_INET;
	proxy_serv_addr.sin_port = htons(listen_port);
	proxy_serv_addr.sin_addr.s_addr = INADDR_ANY;

	int len = sizeof(struct sockaddr_in);

	if ((bind(pr_serv_sock, (struct sockaddr *) &proxy_serv_addr, len)) == -1) {
		perror("bind error 1: \n");
		exit(-1);
	}

	if ((listen(pr_serv_sock, 8)) == -1) {
		perror("listen error: \n");
		exit(-1);
	}

	int cli_len = sizeof(cli_addr);

	while (1) {

		struct sockaddr_in host_addr;
		char request[16000],response[4096000],realXML[4096000];
		char* temp = NULL;
		int received=0, sent=0;

		int bitrates[4];

		if ((browser_sock = accept(pr_serv_sock, (struct sockaddr *) &cli_addr, &cli_len)) < 0) {
			perror("Problem in accepting connection");
			exit(-1);
		}

		received=recv(browser_sock, request, 16000, 0);

		start=clock(); //start time


		//printf("\n%s",request);

		if((pr_cli_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
			perror("socket: \n");
			exit(-1);
		}

		host_addr.sin_family=AF_INET;
		host_addr.sin_port=htons(80);

		if(inet_pton(AF_INET, www_ip, &host_addr.sin_addr) == -1){
			perror("Unable to translate server address for host \n");
			exit(-1);
		}
		bzero(&host_addr.sin_zero, 8);

		if(connect(pr_cli_sock, (struct sockaddr *)&host_addr, sizeof(host_addr)) <0){
			perror("Can't connect to the specified host \n");
			exit(-1);
		}

		char **compose=(char**)malloc(3*sizeof(char*));
		modifyRequest(request,compose);

		if(xmlFlag==0){
			if (strncmp (compose[1],"/vod/big_buck_bunny.f4m",23)==0){
//				printf("\n IF only\n");
				xmlFlag=1;

				char modified_request[16000];
				sprintf(modified_request,"GET /vod/big_buck_bunny_nolist.f4m %s",compose[2]);

				sent=send(pr_cli_sock,modified_request, 16000, 0);
				received = recv(pr_cli_sock,response,4096000,0);
				sent= send(browser_sock,response,4096000,0);

				//				printf("%s",modified_request);
				//printf("%s",response);


				int temp_pr_cli_sock;
				if((temp_pr_cli_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
					perror("socket: \n");
					exit(-1);
				}
				if(connect(temp_pr_cli_sock, (struct sockaddr *)&host_addr, sizeof(host_addr)) <0){
					perror("Can't connect to the specified host \n");
					exit(-1);
				}

				sent=send(temp_pr_cli_sock,request, 16000, 0);
				received = recv(temp_pr_cli_sock,realXML,4096000,0);

				allBitrates(realXML,bitrates);
//				printf("\nafter bitrates");
				end=clock();
				close(temp_pr_cli_sock);
			}
			else{
//				printf("\nIf Else\n");
				sent=send(pr_cli_sock,request, 16000, 0);
				received = recv(pr_cli_sock,response,4096000,0);
				end=clock();
				sent= send(browser_sock,response,4096000,0);
//				printf("%s",response);

			}
		}
		else{
//			printf("\nElse\n");

			char copy_request[strlen(compose[1])];
			strcpy(copy_request,compose[1]);
			char* zz=NULL;
			zz=strstr(copy_request,"Seg");

			//printf("\n%s\n",compose[1]);

			char chunk[50];
			sprintf(request,"GET /vod/%d%s %s",T_cur,zz,compose[2]);
			sprintf(chunk,"%d%s",T_cur,zz);
			sent=send(pr_cli_sock,request, 16000, 0);
			received = recv(pr_cli_sock,response,4096000,0);

			end=clock();
			time_elapsed = (double)(end - start)/CLOCKS_PER_SEC;
			contentLength=getContentLength(response);

			sent= send(browser_sock,response,4096000,0);
//			printf("%s\n",response);
//			printf("%d\n",contentLength);
//			printf("%lf\n",(contentLength/time_elapsed)/1000);
			T_new=((contentLength+strlen(response))/time_elapsed)/1000;
			average=(average*count+T_cur)/++count;

			printf("%lf %d %d %d %s %s\n",time_elapsed,T_new,average,T_cur,www_ip,chunk);
			fprintf(fileptr,"%lf %d %d %d %s %s\n",time_elapsed,T_new,average,T_cur,www_ip,chunk);
			fflush(fileptr);
			T_cur=getClosest(bitrates,T_cur,T_new,alpha);

		//	printf("\nT_cur= %d\n",T_cur);
		}
		close(pr_cli_sock);
		close(browser_sock);

	}
	close(pr_serv_sock);
	fclose(fileptr);
}


void modifyRequest(char * request,char** compose){

	char copy_request[16000],t1[100],t2[100],t3[15800];
	char* temp=NULL;


	strcpy(copy_request,request);
	strcat(copy_request,"^]");

	temp=strtok(copy_request, " ");
	strcpy(t1,temp);
	temp=strtok(NULL," ");
	strcpy(t2,temp);
	temp=strtok(NULL,"^]");
	strcpy(t3,temp);

	compose[0]=t1;
	compose[1]=t2;
	compose[2]=t3;

}

void allBitrates(char* xmlFile, int* bitrates){

	char *temp= (char*)malloc(sizeof(xmlFile));
	strcpy(temp,xmlFile);
	char * temp2 = NULL;
	int i = 0;

	while((temp = strstr(temp, "bitrate")) != NULL){
		char temp1[strlen(temp)];
		strcpy(temp1, temp);
		temp2 = strtok(temp1, "\"");
		temp2 = strtok(NULL, "\"");
		bitrates[i] = atoi(temp2);
		temp = strstr(temp, "bootstrapInfoId");
		i++;
	}

}

int getContentLength(char* response){

	char copy_response[strlen(response)];
	strcpy(copy_response,response);
	char t1[10];

	char* temp = strstr(copy_response,"Content-Length: ");
	temp=temp+strlen("Content-Length: ");
	temp=strtok(temp,"\r\n");

	//	printf("%s\n",temp);
	return atoi(temp);
}

int getClosest(int* bitrates, double T_cur,double T_new, float alpha){

	double answer= alpha * T_new + (1 - alpha) * T_cur;
	answer/=1.5;
		int i;
		for(i=3;i>=0;i--){
			if(answer>=bitrates[i])
				return bitrates[i];
		}
//	if(answer>=1000)
//		return 1000;
//	if(answer>=500)
//		return 500;
//	if(answer>=100)
//		return 100;
	return 10;
}
