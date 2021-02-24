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

// Enable if you want debugging to be printed, see examble below.
// Alternative, pass
#define DEBUG
#define ERROR "ERROR TO\n"
#define PROTOCOL "TEXT TCP 1.0\n"

// Included to get the support library
#include "calcLib.h"

int main(int argc, char *argv[])
{

  /*
    Read first input, assumes <ip>:<port> syntax, convert into one string (Desthost) and one integer (port). 
     Atm, works only on dotted notation, i.e. IPv4 and DNS. IPv6 does not work if its using ':'. 
  */
  if (argc != 2)
  {
    printf("Wrong format IP:PORT\n");
    exit(0);
  }

  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);

  if (Desthost == NULL || Destport == NULL)
  {
    printf("Wrong format\n");
    exit(0);
  }
  // *Desthost now points to a sting holding whatever came before the delimiter, ':'.
  // *Dstport points to whatever string came after the delimiter.

  /* Do magic */
  int port = atoi(Destport);
  addrinfo sa, *si, *p;
  memset(&sa, 0, sizeof(sa));
  sa.ai_family = AF_INET;
  sa.ai_socktype = SOCK_STREAM;
  if (int rv = getaddrinfo(Desthost, Destport, &sa, &si) != 0)
  {
    fprintf(stderr, "%s\n", gai_strerror(rv));
    exit(0);
  }

  int sockfd;
  for (p = si; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      continue;
    }
    if ((connect(sockfd, p->ai_addr, p->ai_addrlen)) != 0)
    {
      close(sockfd);
      printf("Couldn't connect to server.\n");
      continue;
    }
    break;
  }
  if (p == NULL)
  {
    printf("Couldn't create connect.\n");
    exit(0);
  }

  freeaddrinfo(si);

  char buf[128];
  int bytes = recv(sockfd, buf, sizeof(buf), 0);

  if (bytes == -1 || strstr(buf, ERROR) != nullptr)
  {
    printf("Error, couldn't recive message. Exiting program...\n");
    exit(0);
  }
  printf("%s", buf);
  if (strstr(buf, PROTOCOL) == NULL)
  {
    printf("Wrong protocol\n");
    exit(0);
  }
  printf("Accepted protocol\n");
  char msg[] = "OK\n";
  if (send(sockfd, msg, strlen(msg), 0) == -1)
  {
    printf("Error: Failed to send message \n");
    exit(0);
  }
  memset(buf, 0, 128);
  bytes = recv(sockfd, buf, sizeof(buf), 0);

  if (bytes == -1 || strstr(buf, ERROR) != nullptr)
  {
    printf("Error, couldn't recive message. Exiting program...\n");
    exit(0);
  }
  printf("%s", buf);
  char oppr[10];
  if (buf[0] == 'f')
  {
    double val1 = 0;
    double val2 = 0;
    double total = 0;

    sscanf(buf, "%s %lf %lf", oppr, &val1, &val2);

    if (strstr(oppr, "fadd"))
    {
      total = val1 + val2;
    }
    else if (strstr(oppr, "fdiv"))
    {
      total = val1 / val2;
    }
    else if (strstr(oppr, "fmul"))
    {
      total = val1 * val2;
    }
    else if (strstr(oppr, "fsub"))
    {
      total = val1 - val2;
    }
    char answ[12];
    sprintf(answ, "%lf\n", total);
    if (send(sockfd, answ, strlen(answ), 0) == -1)
    {
      printf("Error: Failed to send message \n");
      exit(0);
    }
    printf("%lf\n", total);
  }
  else
  {
    int val1 = 0;
    int val2 = 0;
    int total = 0;
    sscanf(buf, "%s %d %d", oppr, &val1, &val2);

    if (strstr(oppr, "add"))
    {
      total = val1 + val2;
    }
    else if (strstr(oppr, "div"))
    {
      total = val1 / val2;
    }
    else if (strstr(oppr, "mul"))
    {
      total = val1 * val2;
    }
    else if (strstr(oppr, "sub"))
    {
      total = val1 - val2;
    }
    char answ[3];
    sprintf(answ, "%d\n", total);
    if (send(sockfd, answ, strlen(answ), 0) == -1)
    {
      printf("Error: Failed to send message \n");
      exit(0);
    }
    printf("%d \n", total);
  }
  memset(buf, 0, 128);
  bytes = recv(sockfd, buf, sizeof(buf), 0);
  if (bytes == -1 || strstr(buf, ERROR) != nullptr)
  {
    printf("Error, couldn't recive message. Exiting program...\n");
    exit(0);
  }
  printf("%s", buf);

#ifdef DEBUG
  printf("Host %s, and port %d.\n", Desthost, port);
#endif
  close(sockfd);
  return 0;
}
