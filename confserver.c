// CSCE515
// Project 3
//
// confserver.c
// Yixing Cheng

#ifndef FD_SETSIZE
#define FD_SETSIZE   256
#endif

#ifndef MAXCLIENTS
#define MAXCLIENTS   20     //20 clients in conference at maximum
#endif

#include "unp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){
/**************** Declarations ***************************************/
    int                      n, sockfd, maxfp;
    int                      clinum;
    struct sockaddr_in       servaddr, cliaddrs[MAXCLIENTS], swaptemp;
    // a temporary socket address for testing
    struct sockaddr_in       *tempaddr;
    socklen_t                servlen, clilen;
    //let server to read between stdin and upd socket with select
    fd_set                   rset;
    char                     stdinBuf[MAXLINE];
    char                     recvLine[MAXLINE];
    char                     sendLine[MAXLINE];              
    char                     *addrstr;     //a string to store address
    char                     *nickname;    //a string to store nickname
    char                     **nicknames;   //an array of string to store all nicknames
    char                     *tempstr;
    char                     ip[30];

/**************** Socket setup ***************************************/
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ip, "127.0.0.1");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &servaddr.sin_addr);
    servaddr.sin_port = htons(0);   
    servlen = sizeof(servlen);

    Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));

    getsockname(sockfd, (SA *) &servaddr, &servlen);  
    printf("SYSTEM: Server is at %s.\n", Sock_ntop((SA *) &servaddr, servlen));

    clilen = sizeof(servaddr);    
    tempaddr = (struct sockaddr_in *)Malloc(clilen);
    addrstr  = Malloc(MAXLINE * sizeof(char));
    nickname = Malloc(MAXLINE * sizeof(char));
    tempstr = Malloc(MAXLINE * sizeof(char));
    nicknames = Malloc(MAXCLIENTS * MAXLINE * sizeof(char));
    int y;
    for( y = 0; y < MAXCLIENTS; y++){
       nicknames[y] = Malloc(MAXLINE * sizeof(char));
    }
    
/*************** I/O multiplexing setup ******************************/
    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    //will test for EOF from stdin
    FD_SET(fileno(stdin), &rset);
    maxfp = max(fileno(stdin), sockfd) + 1;
 

/*************** main loop for processing *****************************/
/**** There are just three things for server:
      1. Listen for JOIN then update active client list
      2. Listen for LEAVE then update active client list
      3. Listen for EOF from stdin then exit 
****/
  
    clinum = 0;    

    for( ; ; ){
again:
 
        FD_SET(sockfd, &rset);
        //will test for EOF from stdin
        FD_SET(fileno(stdin), &rset);
        maxfp = max(fileno(stdin), sockfd) + 1;
        Select(maxfp, &rset, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &rset)){
             ////process client
             socklen_t    len;
             len = clilen;
             n = Recvfrom(sockfd, recvLine, MAXLINE, 0, (SA *) tempaddr, &clilen);
             if (len == clilen){
                  int i;
                  //test if incoming client is already in list, if not, add it, if yes delete it
                  for(i = 0; i < clinum; i++ ){
                      SA *cliptr = (SA *) &cliaddrs[i];
                      if(memcmp(cliptr, tempaddr, len) == 0){
                          memcpy(nickname, nicknames[i], strlen(nicknames[i]));
                          printf("%s will be deleted %s.\n", nickname, Sock_ntop((SA *) tempaddr, len));
                          int m;
                          for (m = i; m < clinum - 1; m++){
                              swaptemp = cliaddrs[m];
                              memcpy(tempstr, nicknames[m], strlen(nicknames[m]));
                              cliaddrs[m] = cliaddrs[m+1];
                              memcpy(nicknames[m], nicknames[m+1], strlen(nicknames[m+1]));
                              cliaddrs[m+1] = swaptemp;
                              memcpy(nicknames[m+1], tempstr, strlen(tempstr));
                          } 
                          //swapped and the last one is the going to be deleted and inform other clients with this one
                          clinum--;
                          int z;
                          for (z = 0; z< clinum + 1; z++){
                              printf("nicknames[%d] is: %s\n", z, nicknames[z]);
                          }

                          //inform all remaining clients there's one left
                          int x;
                          for(x = 0; x < clinum; x++){
                              printf("what's in last nickname: %s\n", nicknames[m]);
                              memcpy(nickname, nicknames[m], strlen(nicknames[m]));
                              printf("what's in nickname: %s\n", nickname);
                              addrstr = Sock_ntop((SA *) tempaddr, len);
                              strcat(nickname, " ");
                              strcat(nickname, addrstr);

                    //          printf("what's in addrstr %s.\n", addrstr);
                    //          printf("server still have %s.\n", Sock_ntop((SA *) &cliaddrs[x], len));
                              snprintf(sendLine, sizeof(sendLine), "%s\r\n", nickname);
                              printf("what's in sendline when deleting %s", sendLine);
                              Sendto(sockfd, sendLine, strlen(sendLine), 0, (SA *) &cliaddrs[x], len);
                              memset(sendLine, 0, strlen(sendLine));
                              memset(addrstr, 0, strlen(addrstr));
                              memset(nickname, 0, strlen(nickname));
                          }

                          goto again; 
                      }   
                  }
      
                 

                 //no matching client then add client to the list
                 cliaddrs[clinum] = *tempaddr;   //assignment

                 memcpy(nickname, recvLine, n);
                 memcpy(nicknames[clinum], recvLine, n);
                 printf("%s from %s joined.\n", nickname, Sock_ntop((SA *) tempaddr, len));
                 clinum++;         //increment client number
                 
                 //update the new client to all clients 
                 for(i = 0; i<clinum ; i++){
                     memcpy(nickname, recvLine, n);
                     addrstr = Sock_ntop((SA *) tempaddr, len);
                     strcat(nickname, " ");
                     strcat(nickname, addrstr);
       //              printf("what's in nickname %s.\n", nickname);
                     snprintf(sendLine, sizeof(sendLine), "%s\r\n", nickname);
       //              printf("sendline is %s", sendLine);
       //              printf("will send new address to %s.\n", Sock_ntop((SA *) &cliaddrs[i], len));
                     Sendto(sockfd, sendLine, strlen(sendLine), 0, (SA *) &cliaddrs[i], len);
                     memset(sendLine, 0, strlen(sendLine));
                     memset(addrstr, 0, strlen(addrstr));
                     memset(nickname, 0, strlen(nickname));
          
                 }

                 //send the client list to new client
                 for(i = 0; i < clinum - 1 ; i++){
                     memcpy(nickname, nicknames[i], n);
                     addrstr = Sock_ntop((SA *) &cliaddrs[i], len);
                     strcat(nickname, " ");
                     strcat(nickname, addrstr);
                     snprintf(sendLine, sizeof(sendLine), "%s\r\n", nickname);
        //             printf("this will be sent to new client %s", sendLine);
                     Sendto(sockfd, sendLine, strlen(sendLine), 0, (SA *) tempaddr, len);
                     memset(sendLine, 0, strlen(sendLine));
                     memset(addrstr, 0, strlen(addrstr));
                     memset(nickname, 0, strlen(nickname));
                 }
 
            }
        }

        if (FD_ISSET(fileno(stdin), &rset)){
            if( (n = Read(fileno(stdin), stdinBuf, MAXLINE)) == 0){
                 //********sent close information to all clients
                 //EOF in stdin, shut down server
                 break;
            }
        }        

    }

    exit(0);
}
