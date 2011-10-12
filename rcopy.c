#include "librcp.h"

int main(int argc, char *argv[])
{

  client *Client;
  char *remote_addr = argv[1];
  char *remote_port = argv[2];
  Client = client_sock(remote_addr, remote_port);
  send_msg(Client->sock, "adfa\0", 3, (struct sockaddr *)&(Client->remote));
  

  return EXIT_SUCCESS;
}
