#include "server.h"

int main(int argc, char *argv[])
{
  server *Server;
  rcp_pkt *Recv_Pkt;
  pthread_t threads[MAX_THREADS];
  //  int thread_args[MAX_THREADS];
  //  int active_threads = 0;
  double error_rate;
  server_info *Info = s_malloc(sizeof(server_info));

  handle_args(&error_rate, argc, argv);

  Recv_Pkt = pkt_alloc(INIT_DATA_SIZE);
  Server = server_sock(INIT_DATA_SIZE);


  printf("Port: %d\n", ntohs(Server->local.sin_port));

  while(TRUE)
    {
      recv_pkt(Recv_Pkt, Server->sock, &(Server->remote), Server->buffsize);
      if(check_pkt_state(Recv_Pkt, (OP_BEG|OP_SYN), 0)) //Server->seq
	{
	  /* printf("accepted 1st packet\n"); */
	  /* printf("befor thread - sock: %u\n", Server->sock); */
	  /* printf("before thread - remote: %p\n", &(Server->remote)); */
	  Info->error_rate = error_rate;
	  s_memcpy(&(Info->buffsize), Recv_Pkt->data + BUFF_SIZE_OFF, sizeof(uint32_t));
	  Info->buffsize = ntohl(Info->buffsize);
	  s_memcpy(&(Info->filename), Recv_Pkt->data + FILENAME_OFF, Recv_Pkt->data_len - sizeof(uint32_t));
	  Info->OG_Server = Server;
	  printf("before newthred\n");
	  pthread_create(&threads[0], NULL, thread_server, (void *)Info);
	  pthread_join(threads[0], NULL); //---remove for multithreading
	}
    }

  close(Server->sock);
  free(Server);
  free(Recv_Pkt);

  return EXIT_SUCCESS;
}

void handle_args(double *error_rate, int argc, char *argv[])
{
  if (argc == NUM_ARGS)
    {
      *error_rate = atof(argv[ERROR_PERCENT_ARGC]);
    }
  else
    {
      printf("usage: server <error_rate>\n");
      exit(EXIT_FAILURE);
    }
}


void *thread_server(void *Info)
{
  FILE *file;
  server_info *Server_Info = (server_info *)Info;
  rcp_pkt *Send_Pkt;
  rcp_pkt *Recv_Pkt;
  server *Server;
  printf("Thread\n");
  printf("buffsize: %u\n", Server_Info->buffsize);
  printf("filename: %s\n", Server_Info->filename);
  sendtoErr_setup(Server_Info->error_rate);
  Server = server_sock(Server_Info->buffsize);
  Send_Pkt = pkt_alloc(Server_Info->buffsize);
  Recv_Pkt = pkt_alloc(Server_Info->buffsize);
  
  Server->filename = Server_Info->filename;
  //  Server->seq = 1;
  if(!establish_connection(Server_Info->OG_Server, Server, Send_Pkt, Recv_Pkt))
    {
      file = fopen(Server_Info->filename,"rb+");
      if (file != NULL && !ferror(file))
	{
	  printf("opened file\n");
	  printf("transfer file\n");
	  Server->seq = 3;
	  if(transfer_file_setup(Server, Send_Pkt, Recv_Pkt) != NULL)
	    transfer_file(Server, Send_Pkt, Recv_Pkt, file);
	      
	  fclose(file);
	}
      else
	{
	  printf("file error\n");
	  file_error(Server, Send_Pkt, Recv_Pkt, errno);
	}
      end_conn(Server, Send_Pkt, Recv_Pkt);
    }
  close(Server->sock);
  free(Server);
  free(Send_Pkt);
  free(Recv_Pkt);

  pthread_exit(NULL);
}


rcp_pkt *establish_connection(server *Old_Server, server* New_Server, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt)
{
  exp_ops ops;
  Send_Pkt->data_len = sizeof(New_Server->local.sin_port);
  uint8_t *data = (uint8_t *)&(New_Server->local.sin_port);
  printf("establish creating packet\n");
  New_Server->seq = 1;
  create_pkt(Send_Pkt, (OP_BEG|OP_SYN|OP_ACK), New_Server->seq, data, Send_Pkt->data_len);
  ops.num_ops = 1;
  ops.opcode[0] = (OP_BEG|OP_ACK);
  return try_send(Old_Server, Send_Pkt, Recv_Pkt, ops);
}


void file_error(server *Server, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt, int errno_l)
{
  exp_ops ops;
  Server->seq = 3;
  create_pkt(Send_Pkt, (OP_FIL|OP_ERR|OP_SYN), Server->seq, (uint8_t *)&errno_l, sizeof(errno_l));
  ops.num_ops = 1;
  ops.opcode[0] = (OP_FIL|OP_ERR|OP_SYN);
  try_send(Server, Send_Pkt, Recv_Pkt, ops);
}


rcp_pkt *transfer_file_setup(server *Server, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt)
{
  exp_ops ops;
  create_pkt(Send_Pkt, (OP_FIL|OP_BEG|OP_SYN), Server->seq, NULL, 0); //magic number
  ops.num_ops = 1;
  ops.opcode[0] = (OP_FIL|OP_BEG|OP_ACK);
  return try_send(Server, Send_Pkt, Recv_Pkt, ops);
}


void transfer_file(server *Server, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt, FILE *fp)
{
  exp_ops ops;
  uint8_t *data;
  uint32_t data_len = 0;
  int break_flag = TRUE;
  
  ops.num_ops = 1;
  ops.opcode[0] = (OP_FIL|OP_ACK);
  data = s_malloc(Server->buffsize);

  while (!feof(fp) && break_flag)
    {
      data_len = fread(data, 1, Server->buffsize, fp);
      if (!ferror(fp))
	{
	  create_pkt(Send_Pkt, (OP_FIL|OP_SYN), Server->seq, data, data_len);
	  if(try_send(Server, Send_Pkt, Recv_Pkt, ops) == NULL)
	    break_flag = FALSE;
	}
      else
	break;
    }
  free(data);
}


void end_conn(server *Server, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt)
{
  exp_ops ops;
  create_pkt(Send_Pkt, OP_FIN|OP_SYN, Server->seq, NULL, 0); //magic number
  ops.num_ops = 1;
  ops.opcode[0] = OP_FIN|OP_ACK;
  try_send(Server, Send_Pkt, Recv_Pkt, ops);
}


rcp_pkt *try_send(server *Server, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt, exp_ops ops)
{
  int n, j;

  for (n = 0; n < MAX_TRIES; n++)
    {
      send_pkt(Send_Pkt, Server->sock, Server->remote); //may be wrong
      if (select_call(Server->sock, MAX_TRY_WAIT_TIME_S, MAX_TRY_WAIT_TIME_US))
	{
	  recv_pkt(Recv_Pkt, Server->sock, &(Server->remote), Server->buffsize); //may be wrong
	  for (j = 0; j < ops.num_ops && j < MAX_EXP_OPS; j++)
	    {
	      if (check_pkt_state(Recv_Pkt, ops.opcode[j], Server->seq+SEQ_RECV_DIFF))
		{
		  Server->seq += 2; //CHANGING SEQUENCE
		  printf("Good ACK - Seq Now: %u\n", Server->seq);
		  return Recv_Pkt;
		}
	    }
	}
    }
  printf("Connection - Time Out\n");
  return NULL;
}


/* rcp_pkt *try_recv(server *Server, rcp_pkt *Recv_Pkt, exp_ops ops) */
/* { */
/*   int break_flag = TRUE; */
/*   int j = 0; */
/*   while (break_flag) */
/*     { */
/*       if(select_call(Server->sock, MAX_RECV_WAIT_TIME_S, MAX_RECV_WAIT_TIME_US)) */
/* 	{ */
/* 	  recv_pkt(Recv_Pkt, Server->sock, &(Server->remote), Server->buffsize); */
/* 	  printf("!!!!!!!!received pkt -- seq: %u\n", Recv_Pkt->Hdr->seq); */
/* 	  //	  print_opcode(Recv_Pkt->Hdr->opcode); */
/* 	  //	  print_ */
/* 	  for (j = 0; j < ops.num_ops && MAX_EXP_OPS; j++) */
/* 	    { */
/* 	      if (check_pkt_state(Recv_Pkt, ops.opcode[j], Server->seq+SEQ_RECV_DIFF)) */
/* 		{ */
/* 		  Server->seq += 2; //CHANGING SEQUENCE */
/* 		  return Recv_Pkt; */
/* 		} */
/* 	    } */
/* 	} */
/*       else */
/* 	break_flag = FALSE; */
/*     } */
/*   return NULL; */

/* } */


