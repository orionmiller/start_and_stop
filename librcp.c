#include "librcp.h"

client *client_sock(char *remote_address, uint16_t remote_port, uint32_t buffsize)
{  
  struct hostent *hp;
  client *Client = s_malloc(sizeof(client));

  Client->sock = s_socket(SOCK_DOMAIN, SOCK_TYPE, DEFAULT_PROTOCOL);
  Client->buffsize = buffsize;

  //Remote Socket Address Set Up
  Client->remote.sin_family = SOCK_DOMAIN;
  hp = s_gethostbyname(remote_address);
  s_memcpy(&(Client->remote.sin_addr), hp->h_addr, hp->h_length);
  Client->remote.sin_port = htons(remote_port);

  Client->seq = 0;

  return Client;
  //DOES HP NEED TO BE FREED?
}


server *server_sock(uint32_t buffsize)
{  
  server *Server = s_malloc(sizeof(server));
  int len = sizeof(Server->local);
  Server->buffsize = buffsize;
  
  Server->sock = s_socket(SOCK_DOMAIN, SOCK_TYPE, DEFAULT_PROTOCOL);

  Server->local.sin_family = SOCK_DOMAIN;
  Server->local.sin_addr.s_addr = htonl(INADDR_ANY); //???
  Server->local.sin_port = htons(DEFAULT_PORT);

  s_bind(Server->sock, (struct sockaddr *)&(Server->local), 
  	 sizeof(Server->local));

  s_getsockname(Server->sock, (struct sockaddr *)&(Server->local), 
		(socklen_t *)&len);

  Server->seq = 0;
  return Server;
  //DOES HP NEED TO BE FREED?
}

void send_pkt(rcp_pkt *Pkt, int socket, struct sockaddr_in dst_addr)
{
  int flags = 0;
  int dst_addr_len = (int)sizeof(dst_addr);
  if (Pkt != NULL && Pkt->datagram != NULL)
    {
      print_opcode(Pkt->Hdr->opcode);
      printf(" Seq: %u ;;\n", Pkt->Hdr->seq);
      sendtoErr(socket, Pkt->datagram, Pkt->datagram_len, flags, 
		(struct sockaddr *)&dst_addr, dst_addr_len);

    }

  else 
    fprintf(stderr, "send_pkt argument error\n");
}


void recv_pkt(rcp_pkt *Pkt, int socket, struct sockaddr_in *src_addr, uint32_t buffsize)
{
  int flags = 0;
  int src_addr_len = sizeof(*src_addr);

  if (Pkt != NULL && Pkt->datagram != NULL)
    {
      Pkt->datagram_len = s_recvfrom(socket, Pkt->datagram, buffsize+RCP_HDR_LEN, flags, 
       (struct sockaddr *)src_addr, (socklen_t *)&(src_addr_len));
      Pkt->data_len = Pkt->datagram_len - RCP_HDR_LEN;
      get_hdr(Pkt);
      //      print_opcode(Pkt->Hdr->opcode);
      /* printf("src_addr %p\n", &(src_addr)); */
    }
  else
    {
      fprintf(stderr, "recv_pkt argument error\n");
    }
}

void print_opcode(uint16_t opcode)
{
  printf("Opcode: ");
  if (opcode & OP_SYN)
    printf("SYN ");
  if (opcode & OP_ACK)
    printf("ACK ");
  if (opcode & OP_BEG)
    printf("BEG ");
  if (opcode & OP_FIN)
    printf("FIN ");
  if (opcode & OP_ERR)
    printf("ERR ");
  if (opcode & OP_FIL)
    printf("FIL ");
  if (opcode & OP_RST)
    printf("RST ");
  fflush(stdout);
}


void create_pkt(rcp_pkt *Pkt, uint16_t opcode, uint32_t seq, uint8_t *data, uint32_t data_len)
{
  //  int c;
  if (Pkt != NULL && ((data == NULL && data_len == 0)||(data != NULL && data_len > 0)))//MAGICNUM
    {
      Pkt->data_len = data_len;
      Pkt->datagram_len = RCP_HDR_LEN + data_len;
      bzero(Pkt->datagram, Pkt->datagram_len);
      if (data != NULL)
	s_memcpy(Pkt->data, data, Pkt->data_len);
      create_hdr(Pkt, seq, opcode);
      get_hdr(Pkt);
    }
  else 
    {
      fprintf(stderr, "creat_pkt invalid arguments\n");
    }
}


void create_hdr(rcp_pkt *Pkt, uint32_t seq, uint16_t opcode)
{
  uint16_t n_checksum = 0;
  uint16_t n_opcode = htons(opcode);
  uint32_t n_seq = htonl(seq);
  if (Pkt != NULL)
    {
      bzero(Pkt->datagram, RCP_HDR_LEN);
      s_memcpy(Pkt->datagram + RCP_HDR_SEQ_OFFSET, &n_seq, sizeof(uint32_t));
      s_memcpy(Pkt->datagram + RCP_HDR_OPCODE_OFFSET, &n_opcode, sizeof(uint16_t));
      n_checksum = in_cksum((unsigned short *)Pkt->datagram, Pkt->datagram_len);
      s_memcpy(Pkt->datagram + RCP_HDR_CHECKSUM_OFFSET, &n_checksum, sizeof(uint16_t));
    }
  else 
    {
      fprintf(stderr, "create_hdr Pkt is NULL\n");
    }
}

//broken somehow
void get_hdr(rcp_pkt *Pkt)
{
  if (Pkt != NULL && Pkt->Hdr != NULL && Pkt->datagram != NULL)
    {
      s_memcpy(&(Pkt->Hdr->checksum), Pkt->datagram + RCP_HDR_CHECKSUM_OFFSET, sizeof(uint16_t));
      s_memcpy(&(Pkt->Hdr->opcode), Pkt->datagram + RCP_HDR_OPCODE_OFFSET,  sizeof(uint16_t));
      Pkt->Hdr->opcode =  ntohs(Pkt->Hdr->opcode);
      s_memcpy(&(Pkt->Hdr->seq), Pkt->datagram + RCP_HDR_SEQ_OFFSET, sizeof(uint32_t));
      Pkt->Hdr->seq = ntohl(Pkt->Hdr->seq);
    }
}


uint16_t pkt_checksum(rcp_pkt *Pkt)
{
  uint16_t checksum = 0;
  if (Pkt != NULL && Pkt->datagram != NULL)
    {
      checksum = in_cksum((unsigned short *)Pkt->datagram, Pkt->datagram_len);
    }
  else
    {
      fprintf(stderr, "pkt_checksum - bad Pkt\n");
      exit(EXIT_FAILURE);
    }
  return checksum;
}


int check_pkt_state(rcp_pkt *Pkt, uint16_t opcode, uint32_t seq)
{
  if (Pkt == NULL)
    {
      fprintf(stderr, "check_pkt_state Pkt is NULL\n");
    }
  else if (pkt_checksum(Pkt) == 0 && Pkt->Hdr->opcode == opcode && Pkt->Hdr->seq == seq)
    {
      return TRUE; 
    }
  else
    {
      fprintf(stderr, "::INCORRECT PKT STATE::\n");
      printf("EXPECTED OPCODE::");
      print_opcode(opcode);
      printf("\n");
      printf("RECEIVED OPCODE::");
      print_opcode(Pkt->Hdr->opcode);
      printf("\n");
      printf("EXPECTED SEQ:: %u\n", seq);
      printf("RECEIVED SEQ:: %u\n", Pkt->Hdr->seq);
      printf("Checksum: %X\n", pkt_checksum(Pkt));
    }
  return FALSE;
}


void sendtoErr_setup(double error_rate)
{
  //CHANGE BEFORE HANDIN
  sendtoErr_init(error_rate, DROP_OFF, FLIP_OFF, DEBUG_OFF, RSEED_OFF);
}

rcp_pkt *pkt_alloc(uint32_t buffsize)
{
  rcp_pkt *Pkt = s_malloc(sizeof(rcp_pkt));
  Pkt->Hdr = s_malloc(sizeof(rcp_hdr));
  Pkt->datagram = s_malloc(RCP_HDR_LEN + buffsize);
  Pkt->data = Pkt->datagram + RCP_HDR_LEN;
  return Pkt;
}

int select_call(int socket, int seconds, int useconds)
{
  static struct timeval timeout;
  fd_set fdvar;
  int select_out;
  timeout.tv_sec = seconds;
  timeout.tv_usec = useconds;
  FD_ZERO(&fdvar);
  FD_SET(socket, &fdvar);
  select_out = selectMod(socket+1, (fd_set *)&fdvar, (fd_set *)0, (fd_set *)0, &timeout);
  if (select_out == -1) //magic num
    {
      perror("selectMod");
      exit(EXIT_FAILURE);
    }
  return select_out;//(FD_ISSET(socket, &fdvar));
}
