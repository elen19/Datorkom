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
  struct sockaddr_in client;

  addrinfo sa, *si, *p;
  memset(&sa, 0, sizeof(sa));
  sa.ai_family = AF_UNSPEC;
  sa.ai_socktype = SOCK_STREAM;
  if (int rv = getaddrinfo(Desthost, Destport, &sa, &si) != 0)
  {
    fprintf(stderr, "%s\n", gai_strerror(rv));
    exit(0);
  }

  for (p = si; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      continue;
    }
    if ((bind(sockfd, p->ai_addr, p->ai_addrlen)) != 0)
    {
      printf("Failed to bind.\n");
      close(sockfd);
      continue;
    }
    break;
  }
  if (p == NULL)
  {
    printf("Couldn't create/bind socket.\n");
    exit(0);
  }

  freeaddrinfo(si);

  if (listen(sockfd, 5) != 0)
  {
    printf("Failed to listen.\n");
    exit(0);
  }
  print("Listening...\n");
  len = sizeof(client);

  char buffer[128];
  bool clientIsActive = false;
  while (true)
  {
    if (clientIsActive == false)
    {
      if ((connfd = accept(sockfd, (struct sockaddr *)&client, (socklen_t *)&len)) == -1)
      {
        printf("Server failed to accpet.\n");
        exit(0);
      }
      else
      {
        clientIsActive = true;
        char buf[strlen(PROTOCOL)] = PROTOCOL;
        send(connfd, buf, strlen(buf), 0);
      }
    }
    memset(buffer, 0, sizeof(buffer));
    if (recv(connfd, buffer, sizeof(buffer), 0) == -1)
    {
      printf("Recive timed out. Sending error to client.\n");
      send(connfd, "ERROR TO\n", strlen("ERROR TO\n"), 0);
      close(connfd);
    }
    send(connfd, buf, strlen(buf), 0);
  }
  close(sockfd);
  return 0;

#ifdef DEBUG
  printf("Host %s, and port %d.\n", Desthost, port);
#endif
}
