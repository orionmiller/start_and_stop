#include "librcp.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef RCOPY_H_
#define RCOPY_H_


#define MAX_RECV_WAIT_TIME_S (10) //seconds
#define MAX_RECV_WAIT_TIME_US (0) //micro-seconds

#define MAX_SEND_WAIT_TIME_S (1) //seconds
#define MAX_SEND_WAIT_TIME_US (0) //micro-seconds

#define MAX_TRIES 5

#define ERRNO_DATA_OFF (0) 

#define REMOTE_FILE_ARGC 1
#define LOCAL_FILE_ARGC 2
#define BUFFSIZE_ARGC 3
#define ERROR_PERCENT_ARGC 4
#define REMOTE_MACHINE_ARGC 5
#define REMOTE_PORT_ARGC 6
#define NUM_ARGS 7



rcp_pkt *init_connection(client *Client, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt);

void establish_connection(client *Client, rcp_pkt *Recv_Pkt, rcp_pkt *Send_Pkt, char *remote_addr);
void file_error(rcp_pkt *Recv_Pkt);
FILE *transfer_file_setup(client *Client, rcp_pkt *Send_Pkt, char *filename);
void transfer_file(client *Client, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt, FILE *file);
void end_conn(client *Client, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt);
rcp_pkt *try_recv(client *Client, rcp_pkt *Recv_Pkt, exp_ops ops);
rcp_pkt *try_send(client *Client, rcp_pkt *Send_Pkt, rcp_pkt *Recv_Pkt, exp_ops ops);


#endif
