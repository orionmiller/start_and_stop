#include "util.h"

int s_socket(int domain, int type, int protocol)
{
  int sock_d;
  if ((sock_d= socket(domain, type, protocol)) < 0) //magic num
    {
      perror("s_socket");
      exit(EXIT_FAILURE);
    }
  return sock_d;
}

ssize_t s_sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len)
{
  ssize_t ret_val;
  ret_val = sendto(socket, message, length, flags, dest_addr, dest_len);
  if (ret_val == -1 || ret_val != length)
    {
      perror("s_sendto");
      exit(EXIT_FAILURE);
    }
  return ret_val;
}

ssize_t s_recvfrom(int socket, void *buffer, size_t length, int flags, struct sockaddr *address, socklen_t *address_len)
{
  ssize_t ret_val;
  ret_val = recvfrom(socket, buffer, length, flags, address, address_len);
  if (ret_val < 0)
    {
      perror("s_recvfrom");
      exit(EXIT_FAILURE);
    }
  return ret_val;
}


void s_memcpy(void *dest, const void *src, size_t n)
{
  if (memcpy(dest,src,n) != dest)
    {
      fprintf(stderr, "%s\n","s_memcpy");
      exit(EXIT_FAILURE);
    }
}

void *s_malloc(size_t size)
{
  void *data;
  if((data=malloc(size)) == NULL)
    {
      perror("s_malloc");
      exit(EXIT_FAILURE);
    }
  return data;
}


void s_bind(int socket, const struct sockaddr *address, socklen_t address_len)
{
  if(bindMod(socket, address, address_len) < 0)
    {
      perror("s_bind");
      exit(EXIT_FAILURE);
    }
}


void s_getsockname(int socket, struct sockaddr *address, socklen_t *address_len)
{
  if (getsockname(socket, address, address_len) < 0)
    {
      perror("s_getsockbyname");
      exit(EXIT_FAILURE);
    }
}


struct hostent* s_gethostbyname(const char *name)
{
  struct hostent *hp;
  if((hp = gethostbyname(name))==NULL)
    {
      perror("s_gethostbyname");
      exit(EXIT_FAILURE);
    }
  return hp;
}

