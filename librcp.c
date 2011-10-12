#include "librcp.h"

client *client_sock(char *remote_address, char *remote_port)
{  
  struct hostent *hp;
  client *Client = s_malloc(sizeof(client));

  Client->sock = s_socket(SOCK_DOMAIN, SOCK_TYPE, DEFAULT_PROTOCOL);

  //Remote Socket Address Set Up
  Client->remote.sin_family = SOCK_DOMAIN;
  hp = s_gethostbyname(remote_address);
  s_memcpy(&(Client->remote.sin_addr), hp->h_addr, hp->h_length);
  Client->remote.sin_port = htons(atoi(remote_port));

  return Client;
  //DOES HP NEED TO BE FREED?
}

server *server_sock(void)//char *remote_address, char *remote_port)
{  
  server *Server = s_malloc(sizeof(server));
  int len = sizeof(Server->local);


  Server->sock = s_socket(SOCK_DOMAIN, SOCK_TYPE, DEFAULT_PROTOCOL);

  //Local Socket Address Set Up
  Server->local.sin_family = SOCK_DOMAIN;
  Server->local.sin_addr.s_addr = htonl(INADDR_ANY); //???
  Server->local.sin_port = htons(DEFAULT_PORT);
  printf("port: %d\n", ntohs(Server->local.sin_port));

  //Bind The Local Address to a Port
  s_bind(Server->sock, (struct sockaddr *)&(Server->local), 
	 sizeof(Server->local)); //not sure which size of i want to go with

  //Remote Socket Address Set Up
  s_getsockname(Server->sock, (struct sockaddr *)&(Server->local), 
		(socklen_t *)&len); //not sure which size of i want to go with 
  //len may break code


  return Server;
  //DOES HP NEED TO BE FREED?
}

void send_msg(int socket, char *msg, size_t length, struct sockaddr *dst_addr)
{
  s_sendto(socket, msg, length, 0, dst_addr,sizeof(dst_addr));
}

void recv_msg(int socket, void *buffer, size_t length, struct sockaddr *src_addr)
{
  int rlen = sizeof(src_addr);
  s_recvfrom(socket, buffer, length, 0, (struct sockaddr *)src_addr, (socklen_t *)&rlen);
}
