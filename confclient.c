// CSCE515
// Project 3
//
// confclient.c
// Yixing Cheng

#include "unp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAXCLIENTS
#define MAXCLIENTS  20   //20 clients in conference at maximum
#endif

int main(int argc, char **argv){
/************************** Declarations *****************/
   int                  n, sockfd, maxfp;
   int                  clinum;
   struct sockaddr_in   servaddr, cliaddrs[MAXCLIENTS], swaptemp;
   struct sockaddr_in   *tempaddr;
   socklen_t            servlen;
   fd_set               rset;
   char                 sendLine[MAXLINE];
   char                 recvLine[MAXLINE];
   char                 preBuf[MAXLINE];
   char                 *addrstr;
//   void                 intHandler(int);

/************************** Initiation *******************/
   if ( argc !=4 )
      err_quit("usage: a.out <IPaddress> <Port> <nickname>");

/************************** Socket setup *****************/
   if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        err_sys("socket error");
   
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port   = htons(atoi(argv[2]));

   if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
            err_quit("inet_pton eror for %s", argv[1]);
   
   servlen = sizeof(servaddr);
   tempaddr = (struct sockaddr_in *)Malloc(servlen);
   clinum = 0;
   addrstr = Malloc(128 * sizeof(char));
/********************* Ctrl+C signal handler *****************/
//   Signal(SIGINT, intHandler);              

/********************** JOIN *****************************/
   //send a JOIN message with nickname
   strcpy(preBuf, "SYSTEM: ");
   strcat(preBuf, argv[3]);                              //send the nickname to server
   Sendto(sockfd, preBuf, strlen(preBuf), 0, (SA *) &servaddr, servlen);
   memset(preBuf, 0, strlen(preBuf));

/*********************** I/O multiplexing ****************/
   FD_ZERO(&rset);
//   FD_SET(sockfd, &rset);
//   FD_SET(fileno(stdin), &rset);
//   maxfp = max(fileno(stdin), sockfd) + 1;

/*********************** main loop **********************/
/***** Main loop for client do the following things:
       1. Listen for termination of server then return
       2. Listen for EOF of stdin then return
*****/
   int i;
   for( ; ; ){
again:
       FD_SET(sockfd, &rset);
       FD_SET(fileno(stdin), &rset);
       maxfp = max(fileno(stdin), sockfd) + 1;
       Select(maxfp, &rset, NULL, NULL, NULL);

       if (FD_ISSET(sockfd, &rset)){
           socklen_t len;
           len = servlen;
           n = Recvfrom(sockfd, recvLine, MAXLINE, 0, (SA *) tempaddr, &servlen);
           
           //if receive data from server which is client update
           if ( (len == servlen) && (memcmp(&servaddr, tempaddr, len) == 0)){
 //               printf("what's in recvline %s", recvLine);
                //if the address is not in the list, add it. if the address is already in the list and the server send it again, it mean to delete it from list 
                for(i = 0; i <= clinum; i++ ){
                    addrstr = Sock_ntop((SA *) &cliaddrs[i], len);
           //         printf("what's addrstr %s.\n", addrstr);
                    char *beforews, *afterws, *nosys, *nickname;
                    int indexws;
                    nosys = (char *) Malloc((n-10) * sizeof(char));
                    memcpy(nosys, &recvLine[8], n-10);
 //                   printf("what's in nosys: %s\n", nosys);
                    beforews = strchr(nosys, ' ');
                    indexws = (int) (beforews - nosys);
                    nickname = (char *) Malloc(indexws * sizeof(char));
                    memcpy(nickname, nosys, indexws);
                    afterws = (char *) Malloc((strlen(beforews) - 1) * sizeof(char));
                    memcpy(afterws, &beforews[1], strlen(beforews) - 1);
         //           printf("what's in afterws: %s\n", afterws);
                    if ( strncmp(addrstr, afterws, strlen(addrstr)) == 0){
                        printf("SYSTEM: %s from %s left!\n", nickname, addrstr);  //if find address in list then delete it from list reorganize list
                        int m;
                        for ( m=i; m < clinum - 1; m++ ){
                             swaptemp = cliaddrs[m];
                             cliaddrs[m] = cliaddrs[m+1];
                             cliaddrs[m+1] = swaptemp;
                        }

                        clinum--;

    //                    int x;
    //                    for(x = 0; x< clinum; x++){
    //                        printf("client's list still have %s.\n", Sock_ntop((SA *) &cliaddrs[x], len));
    //                    }
                        
                        goto again;
                    }
                    memset(addrstr, 0, strlen(addrstr));
                    
                }

                //--------------string manipulation here------------------------
                char  *delim, *delimws, *ipAddress, *portNum, *nickname, *noprefix, *protaddr;
                int   delimindex, wsindex;
     //           printf("n is %d.\n", n);
                noprefix = (char *) Malloc((n-10) * sizeof(char));
                memcpy(noprefix, &recvLine[8], n-10);
     //           printf("what's in noprefix: %s\n", noprefix);
                delimws = strchr(noprefix, ' ');
     //           printf("delimws is %s.\n", delimws);
		wsindex = (int) (delimws - noprefix);
                nickname =(char *) Malloc(wsindex * sizeof(char));
                memcpy(nickname, noprefix, wsindex);
   //             printf("nickname is: %s.\n", nickname);
                protaddr = (char *) Malloc((strlen(delimws) - 1) * sizeof(char) );
                memcpy(protaddr, &delimws[1], strlen(delimws) - 1);                
    //            printf("protaddr is: %s.\n", protaddr);
                delim = strchr(protaddr, ':');
   //             printf("delim is %s.\n", delim);
                delimindex = (int) (delim - protaddr);
    //            printf("index of : is %d.\n", delimindex);
    //            printf("length of protaddr is %d.\n", strlen(protaddr));
                cliaddrs[clinum].sin_family = AF_INET;
                ipAddress = (char *) Malloc(delimindex * sizeof(char));
                portNum = (char *) Malloc((strlen(protaddr) - 1 - delimindex) * sizeof(char));
                memcpy( ipAddress, protaddr, delimindex);
                memcpy( portNum, &protaddr[delimindex+1], strlen(protaddr)-1-delimindex);
    //            printf("ipAddress is %s.\n", ipAddress);
    //            printf("portNum is %s.\n", portNum);
                inet_pton(AF_INET, ipAddress, &cliaddrs[clinum].sin_addr);
                cliaddrs[clinum].sin_port = htons(atoi(portNum));
                printf("SYSTEM: %s from %s joined!\n", nickname, Sock_ntop((SA *) &cliaddrs[clinum], len));
                clinum++;
   //             printf("%d clients in list.\n", clinum);
                continue;
           //     free(delim);
           //     free(ipAddress);
           //     free(portNum);
 
           }

           //if receive data from other clients for communication
           if ( len == servlen ){
               for(i = 0; i < clinum; i++){
                   if(memcmp(&cliaddrs[i], tempaddr, len) == 0){
                       Write(fileno(stdout), recvLine, n);
                       break;
                   }
               }
               //if didn't find the incoming address from client list, send alert message
               printf("SYSTEM: not from a legitimate address!\n");
           }

            printf("SYSTEM: not from a legitimate address!\n");

       }
       
 //-----------------read from stdin and broadcast to all other clients-------------
       if (FD_ISSET(fileno(stdin), &rset)){
            //----------------handle the situation where ctrl+D--------------------
            if ((n = Read(fileno(stdin), sendLine, MAXLINE)) == 0 ){
                //-----------------send LEAVE message-----------------------
                 
                Sendto(sockfd, sendLine, strlen(sendLine), 0, (SA *) &servaddr, servlen);
                break;
            }

            strcpy(preBuf, argv[3]);
            strcat(preBuf, ": ");
            strcat(preBuf, sendLine);
            // i starts from 1 because 1 store the address of client itself and we don't send it back to client itself
            for(i = 1; i < clinum; i++ ){
                Sendto(sockfd, preBuf, strlen(preBuf), 0, (SA *) &cliaddrs[i], servlen);
            }
            memset(preBuf, 0, strlen(preBuf));
            memset(sendLine, 0, strlen(sendLine));
       }
       
   }
   exit(0);
  
}

void intHandler(int signo){
     
}
