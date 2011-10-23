#include "rcopy.h"

int main(int argc, char *argv[]) {

  client *Client;
  char *remote_addr;
  int remote_port;
  char *remote_filename, *local_filename;
  double error_rate = 0;
  uint32_t buffsize;
  rcp_pkt *Send_Pkt;
  rcp_pkt *Recv_Pkt;
  uint8_t *data;
  FILE *file;
  exp_ops ops;

  if (argc != NUM_ARGS)
    {
      printf("usage: rcopy remote-file local-file buffer-size error-percent remote-machine remote-port\n");
      exit(EXIT_FAILURE);
    }
  remote_filename = argv[REMOTE_FILE_ARGC];
  local_filename = argv[LOCAL_FILE_ARGC];
  buffsize = atoi(argv[BUFFSIZE_ARGC]);
  error_rate = atof(argv[ERROR_PERCENT_ARGC]);
  remote_addr = argv[REMOTE_MACHINE_ARGC];
  remote_port = atoi(argv[REMOTE_PORT_ARGC]);

  Send_Pkt = pkt_alloc(buffsize);
  Recv_Pkt = pkt_alloc(buffsize);
  data = s_malloc(buffsize);

  Client = client_sock(remote_addr, (uint16_t)remote_port, buffsize);
  Client->remote_filename = (uint8_t *)remote_filename;
  sendtoErr_setup(error_rate);

  if (init_connection(Client, Send_Pkt, Recv_Pkt))
    {
      printf("initialized & establishing\n");
      establish_connection(Client, Recv_Pkt, Send_Pkt, remote_addr);
      printf("established\n");
      ops.num_ops = 2;
      ops.opcode[0] = (OP_FIL|OP_BEG|OP_SYN);
      ops.opcode[1] = (OP_FIL|OP_ERR|OP_SYN);
      printf("waiting for File transfer setup\n");
      try_recv(Client, Recv_Pkt, ops);
      if (Recv_Pkt->Hdr->opcode == (OP_FIL|OP_BEG|OP_SYN))
	{
	  file = transfer_file_setup(Client, Send_Pkt, local_filename);
	  printf("transfer file setup\n");
	  ops.num_ops = 2;
	  ops.opcode[0] = OP_FIL|OP_SYN;
	  ops.opcode[1] = OP_FIN|OP_SYN;
	  do 
	    {
	      printf("transfer file\n");
	      try_recv(Client, Recv_Pkt, ops);
	      if(Recv_Pkt->Hdr->opcode == (OP_FIL|OP_SYN))
		{
		  transfer_file(Client, Send_Pkt, Recv_Pkt, file);
		}
	    }
	  while(Recv_Pkt->Hdr->opcode == (OP_FIL|OP_SYN));
	  end_conn(Client, Send_Pkt, Recv_Pkt);
	}
      else if(Recv_Pkt->Hdr->opcode == (OP_FIL|OP_ERR|OP_SYN))
	{
	  printf("file error\n");
	  file_error(Recv_Pkt);
	  ops.num_ops = 1;
	  ops.opcode[0] = (OP_FIN|OP_SYN);
	  try_recv(Client, Recv_Pkt, ops);
	  end_conn(Client, Send_Pkt, Recv_Pkt);
	}
      else
	{
	  //--Do Nothing--//
	}
    }
  close(Client->sock);
  free(Client);
  free(data);
  
  return EXIT_SUCCESS;
}

//fix data
rcp_pkt *init_connection(client *Client, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt)
{
  exp_ops ops;
  uint32_t n_buffsize = htonl(Client->buffsize);
  uint32_t data_len = sizeof(uint32_t) + strlen((const char *)Client->remote_filename) + 1;
  uint8_t *data = s_malloc(data_len);
  s_memcpy(data + BUFF_SIZE_OFF, &n_buffsize, sizeof(uint32_t));
  s_memcpy(data + FILENAME_OFF, Client->remote_filename, strlen((const char *)Client->remote_filename)+1);
  create_pkt(Send_Pkt, (OP_BEG|OP_SYN), Client->seq, data, data_len);
  free(data);
  ops.num_ops = 1;
  ops.opcode[0] = (OP_BEG|OP_SYN|OP_ACK);
  return try_send(Client, Send_Pkt, Recv_Pkt, ops);
}


void establish_connection(client *Client, rcp_pkt *Recv_Pkt, rcp_pkt *Send_Pkt, char *remote_addr)
{
  uint32_t buffsize = Client->buffsize;
  uint16_t remote_port;
  printf("establish connecting - copying port\n");
  s_memcpy(&remote_port, Recv_Pkt->data, sizeof(uint16_t));
  remote_port = ntohs(remote_port);
  close(Client->sock);
  free(Client);
  Client = client_sock(remote_addr, remote_port, buffsize);
  Client->seq = 2;
  create_pkt(Send_Pkt, (OP_BEG|OP_ACK), Client->seq, NULL, 0);
  send_pkt(Send_Pkt, Client->sock, Client->remote);
}

void file_error(rcp_pkt *Recv_Pkt)
{  
  uint32_t errno_l;//get file error
  s_memcpy(&errno_l, Recv_Pkt->data + ERRNO_DATA_OFFSET, sizeof(uint32_t));
  errno = (int)errno_l;
  perror("File Error");
}

FILE *transfer_file_setup(client *Client, rcp_pkt *Send_Pkt, char *filename)
{
  FILE* file;
  file = fopen(filename, "wb+");
  if (!ferror(file))
    {
      create_pkt(Send_Pkt, (OP_FIL|OP_BEG|OP_ACK), Client->seq, NULL, 0); //SEQ NUM
      send_pkt(Send_Pkt, Client->sock, Client->remote);
    }
  else
    {
      perror("File Create");
      exit(EXIT_FAILURE);
    }
  return file;
}


void transfer_file(client *Client, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt, FILE *file)
{
  printf("1st byte - to be written : %c\n", (Recv_Pkt->data)[0]);
  fwrite(Recv_Pkt->data, sizeof(uint8_t),Recv_Pkt->data_len,file);
  fflush(file);
  if(ferror(file))
    {
      perror("File Write");
      exit(EXIT_FAILURE);
    }
  create_pkt(Send_Pkt, (OP_FIL|OP_ACK), Client->seq, NULL, 0);
  send_pkt(Send_Pkt, Client->sock, Client->remote);
}


void end_conn(client *Client, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt)
{
  exp_ops ops;
  ops.num_ops = 1;
  ops.opcode[1] = (OP_FIN|OP_SYN);
  if(try_recv(Client, Recv_Pkt, ops) != NULL)
    {
      create_pkt(Send_Pkt, (OP_FIN|OP_ACK), Client->seq, NULL, 0); //magic number
      send_pkt(Send_Pkt, Client->sock, Client->remote);
    }
}

rcp_pkt *try_recv(client *Client, rcp_pkt *Recv_Pkt, exp_ops ops)
{
  int break_flag = TRUE;
  int j = 0;
  while (break_flag)
    {
      printf("try - recv - befor select call\n");
      if(select_call(Client->sock, MAX_RECV_WAIT_TIME_S, MAX_RECV_WAIT_TIME_US))
	{
	  recv_pkt(Recv_Pkt, Client->sock, &(Client->remote), Client->buffsize); //may be wrong
	  printf("received pkt -- seq: %u\n", Recv_Pkt->Hdr->seq);
	  print_opcode(Recv_Pkt->Hdr->opcode);
	  for (j = 0; j < ops.num_ops && MAX_EXP_OPS; j++)
	    {
	      if (check_pkt_state(Recv_Pkt, ops.opcode[j], Client->seq+SEQ_RECV_DIFF))
		{
		  Client->seq += 2; //CHANGING SEQUENCE
		  return Recv_Pkt;
		}
	    }
	}
      else
	break_flag = FALSE;
    }
  return NULL;

}

rcp_pkt *try_send(client *Client, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt, exp_ops ops)
{
  int n, j;

  for (n = 0; n < MAX_TRIES; n++)
    {
      send_pkt(Send_Pkt, Client->sock, Client->remote); //may be wrong
      if (select_call(Client->sock, MAX_SEND_WAIT_TIME_S, MAX_SEND_WAIT_TIME_US))
	{
	  recv_pkt(Recv_Pkt, Client->sock, &(Client->remote), Client->buffsize); //may be wrong
	  for (j = 0; j < ops.num_ops && j < MAX_EXP_OPS; j++)
	    {
	      if (check_pkt_state(Recv_Pkt, ops.opcode[j], Client->seq+SEQ_RECV_DIFF))
		{
		  Client->seq += 2; //CHANGING SEQUENCE
		  return Recv_Pkt;
		}
	    }
	}
    }
  printf("No packets received\n");
  Client->seq += 1; //CHANGING SEQUENCE
  return NULL;
}
