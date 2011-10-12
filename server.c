#include "librcp.h"

int main(int argc, char *argv[])
{

  server *Server;
  uint8_t buffer[80];
  Server = server_sock();
  recv_msg(Server->sock, buffer, 3, (struct sockaddr *)&(Server->remote));
  buffer[79] = '\0';
  
  printf("mesg: \"$%s\"\n", buffer);
  

  return EXIT_SUCCESS;
}
