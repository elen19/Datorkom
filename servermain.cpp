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
  int sockfd, len, connfd;
  struct sockaddr_in client;

  addrinfo sa, *si, *p;
  memset(&sa, 0, sizeof(sa));
  sa.ai_family = AF_UNSPEC;
  sa.ai_socktype = SOCK_STREAM;
  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;
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
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
  freeaddrinfo(si);

  if (listen(sockfd, 5) != 0)
  {
    printf("Failed to listen.\n");
    exit(0);
  }
  printf("Listening...\n");
  len = sizeof(client);

  char recvBuf[128];
  char sendBuf[128];
  bool clientIsActive = false;
  initCalcLib();
  double fl1, fl2;
  double flAnsw = -1;
  double clientAnswFl = -1;
  int in1, in2;
  int inAnsw = -1;
  int clientAnswInt = -1;
  char *arith;
  while (true)
  {
    if (clientIsActive == false)
    {
      if ((connfd = accept(sockfd, (struct sockaddr *)&client, (socklen_t *)&len)) == -1)
      {
        printf("Nothing to accpet. Still waiting for client...\n");
        continue;
      }
      else
      {
        clientIsActive = true;
        char buf[sizeof(PROTOCOL)] = PROTOCOL;
        send(connfd, buf, strlen(buf), 0);
        arith = randomType();
      }
    }
    memset(recvBuf, 0, sizeof(recvBuf));
    memset(sendBuf, 0, sizeof(sendBuf));
    if (recv(connfd, recvBuf, sizeof(recvBuf), 0) == -1)
    {
      if (errno == EAGAIN)
      {
        printf("Recive timed out. Sending error to client.\n");
        send(connfd, "ERROR TO\n", strlen("ERROR TO\n"), 0);
      }
      else
      {
        printf("Something went wrong with recive.\n");
      }
      close(connfd);
      clientIsActive = false;
      continue;
    }
    if (strcmp(recvBuf, "OK\n") == 0)
    {
      if (arith[0] == 'f')
      {
        fl1 = randomFloat();
        fl2 = randomFloat();
        if (strcmp(arith, "fadd") == 0)
        {
          flAnsw = fl1 + fl2;
        }
        else if (strcmp(arith, "fsub") == 0)
        {
          flAnsw = fl1 - fl2;
        }
        else if (strcmp(arith, "fdiv") == 0)
        {
          flAnsw = fl1 / fl2;
        }
        else if (strcmp(arith, "fmul") == 0)
        {
          flAnsw = fl1 * fl2;
        }
        //printf("%lf %lf %lf\n", fl1, fl2, flAnsw);
        sprintf(sendBuf, "%s %lf %lf\n", arith, fl1, fl2);
      }
      else
      {
        in1 = randomInt();
        in2 = randomInt();
        if (strcmp(arith, "add") == 0)
        {
          inAnsw = in1 + in2;
        }
        else if (strcmp(arith, "sub") == 0)
        {
          inAnsw = in1 - in2;
        }
        else if (strcmp(arith, "div") == 0)
        {
          inAnsw = in1 / in2;
        }
        else if (strcmp(arith, "mul") == 0)
        {
          inAnsw = in1 * in2;
        }
        //printf("%d %d %d\n", in1, in2, inAnsw);
        sprintf(sendBuf, "%s %d %d\n", arith, in1, in2);
      }
    }
    else if (arith[0] == 'f')
    {
      sscanf(recvBuf, "%lf", &clientAnswFl);
      if (abs(clientAnswFl - flAnsw) < 0.0001f)
      {
        printf("Client handled, answears matched.\n");
        sprintf(sendBuf, "%s", "OK\n");
        clientIsActive = false;
      }
      else
      {
        printf("Client handled, answears did not matched.\n");
        sprintf(sendBuf, "%s", "ERROR\n");
        clientIsActive = false;
      }
    }
    else if (arith[0] != 'f')
    {
      sscanf(recvBuf, "%d", &clientAnswInt);
      if (clientAnswInt == inAnsw)
      {
        printf("Client handled, answears matched.\n");
        sprintf(sendBuf, "%s", "OK\n");
        clientIsActive = false;
      }
      else
      {
        printf("Client handled, answears did not matched.\n");
        sprintf(sendBuf, "%s", "ERROR\n");
        clientIsActive = false;
      }
    }
    else
    {
      sprintf(sendBuf, "%s", "ERROR TO\n");
      clientIsActive = false;
    }
    if (send(connfd, sendBuf, strlen(sendBuf), 0) == -1)
    {
      printf("Couldn't send message. \n");
      exit(0);
    }
  }
  close(sockfd);
  return 0;

#ifdef DEBUG
  printf("Host %s, and port %d.\n", Desthost, port);
#endif
}
