#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>

#include "librcp.h"
#include "util.h"




#ifndef SERVER_H_
#define SERVER_H_


#define MAX_THREADS (4)
#define MAX_TRIES (5)

#define MAX_TRY_WAIT_TIME_S (1) //seconds
#define MAX_TRY_WAIT_TIME_US (0) //micro-seconds

#define MAX_RECV_WAIT_TIME_S (10) //seconds
#define MAX_RECV_WAIT_TIME_US (0) //micro-seconds

#define RECEIVED_PKT (1)

#define INIT_DATA_SIZE 1400


typedef struct {
  char filename[255]; //magic number
  uint32_t buffsize;
  server *OG_Server;
  double error_rate;
}server_info;

#define ERROR_PERCENT_ARGC 1
#define NUM_ARGS 2


void *thread_server(void *argument);

// Sends the ACK of initiate_connection() from client w/ new socke info.
rcp_pkt *establish_connection(server *Old_Server, server* New_Server, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt);


// Respond that there was a file error
void file_error(server *Server, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt, int errno_l);

rcp_pkt *transfer_file_setup(server *Server, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt);


// Keep sending data from the file until EOF
void transfer_file(server *Server, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt, FILE *fp);

// Polite way of ending the connection with the client
void end_conn(server *Server, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt);

//Tries to send a SYN pkt 5 times. Waits a 1 second on the socket
// to see if it has received from 
// fix
rcp_pkt *try_send(server *Server, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt, exp_ops ops);

void handle_args(double *error_rate, int argc, char *argv[]);


rcp_pkt *try_recv(server *Server, rcp_pkt *Recv_Pkt, exp_ops ops);

#endif 
