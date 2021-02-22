#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>

/* You will to add includes here */

// Included to get the support library
#include "calcLib.h"

// Enable if you want debugging to be printed, see examble below.
// Alternative, pass argument during compilation '-DDEBUG'
#define PROTOCOL "TEXT TCP 1.0\n\n"
#define DEBUG

using namespace std;

int main(int argc, char *argv[])
{

  /*
    Read first input, assumes <ip>:<port> syntax, convert into one string (Desthost) and one integer (port). 
     Atm, works only on dotted notation, i.e. IPv4 and DNS. IPv6 does not work if its using ':'. 
  */
  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);
  // *Desthost now points to a sting holding whatever came before the delimiter, ':'.
  // *Dstport points to whatever string came after the delimiter.

  /* Do magic */
  int port = atoi(Destport);
  int sockfd, len, connfd;
  struct sockaddr_in serveraddr, client;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    printf("Failed to create socket.\n");
    close(sockfd);
    exit(0);
  }
  bzero(&serveraddr, sizeof(serveraddr));

  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(port);
  printf("LOL\n");
  if ((bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) != 0)
  {
    printf("Failed to bind.\n");
    close(sockfd);
    exit(0);
  }
  printf("POG\n");

  if (listen(sockfd, 5) != 0)
  {
    printf("Failed to listen.\n");
    close(sockfd);
    exit(0);
  }
printf("lyssnar :eyes: \n");
  len = sizeof(client);

  if ((connfd = accept(sockfd, (struct sockaddr *)&client, (socklen_t *)len)) == -1)
  {
    printf("Server failed to accpet.\n");
    close(sockfd);
    close(connfd);
    exit(0);
  }

  char buf[128] = PROTOCOL;

  close(connfd);
  close(sockfd);
  return 0;

#ifdef DEBUG
  printf("Host %s, and port %d.\n", Desthost, port);
#endif
}
