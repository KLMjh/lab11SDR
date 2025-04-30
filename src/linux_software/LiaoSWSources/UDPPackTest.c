/*
 * Send a UDP packet to server.c.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#define DATA "The sea is calm tonight, the tide is full . . ."

main(int argc, char *argv[])
{
  int sock;
  struct sockaddr_in name;
  struct hostent *hp, *gethostbyname();

  if(argc != 3){
    fprintf(stderr, "Usage: targetIP numPackets\n");
    exit(1);
  }

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock < 0){
    perror("socket");
    exit(1);
  }

  hp = gethostbyname(argv[1]);
  if(hp == 0){
    fprintf(stderr, "%s: unknown host\n", argv[1]);
    exit(1);
  }
  int numPack = argv[2];
  bcopy(hp->h_addr, &name.sin_addr, hp->h_length);
  name.sin_family = AF_INET;
  name.sin_port = htons(25344);

  int DUMMY[257];

  for (int i = 0; i< atoi(argv[2]); i++) {
  for (int j = 0; j<256; j++) {
	DUMMY[j+1] = 65537*(j+1)*(i+1);
  }
  
  //for (int i = 0; i < atoi(argv[2]); i++) {
  	DUMMY[0] = i;
  	if(sendto(sock, DUMMY, sizeof(DUMMY), 0, &name, sizeof(name)) < 0)
    	  perror("sendto");
  }
  exit(0);
}
