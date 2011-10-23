// Author: Orion Miller (aomiller)
// 
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>

#include "util.h"
#include "cpe464.h"

#ifndef LIBRCP_H_
#define LIBRCP_H_

//extern int h_errno;

//Is the buffer size absolute for the entire packet or is it the
//size of the data/payload coming.

// RCP - Application Header
//
//
// 0----------------------------15----------------------------31
// |                        Sequence                           |
// |-----------------------------------------------------------|
// |          Checksum           |           Opcode            |
// |-----------------------------------------------------------|
// .                           Data::                          .
// .                                                           .
// .............................................................
//
// Opcode
//   Contains the action you that is supposed to happen.
//
// Checksum
//   The checksum of the Header and the Data.
//

typedef struct {
  uint32_t seq;
  uint16_t checksum;
  uint16_t opcode;
} rcp_hdr;


// RCP HDR 
#define RCP_HDR_LEN (sizeof(rcp_hdr))
#define RCP_HDR_SEQ_OFFSET (0)
#define RCP_HDR_CHECKSUM_OFFSET (4)
#define RCP_HDR_OPCODE_OFFSET (6)


// Op-Codes
//
// SYN
//  The client sends this to the server to initiate a file transfer. The client
//  will wait (X seconds) for the server to receive an ACK reply. If the
//  client does not receive an ACK within that time it will exit with an error
//  stating that the client was not able to reach the server.
//  Note: If the server receives a SYN packet with an incorrect checksum the
///  no ACK back will be sent.
//  
// ACK - Acknowledgement
//  The server is supposed to send an ACK back to the client after receiving
//  a SYN packet to let the client know it can start transfering the file
//  to the server.
//
// BEG - Begin
//  Packet sent to initiate communications between the server and the client.
//   Client sends a BEG|SYN to initiate the communications.
//   Server responds with BEG|ACK to finish the start up process.
//
// FIN - Finished
//  The client sends this to the server to notify that the transfer and
//  connection is completed. The client and server should properly
//  tear down their connection and wrap everything up properly.
//
// ERR - Error
//  The server will send this back to the client if it receives a bad checksum
//  for a FIL packet and its corresponding Sequene number from the FIL packet
//  header.
//
// FIL - File
//  The client will send this packet after initiating communications with the
//  server (i.e. sending a SYN packet and receiving an ACK packet back from
//  the server).
//
// RST - Reset
//  This resets the sequence number.
//   Server - Sends RST|SYN to notify the client is reseting its sequence
//            number.
//   Client - Responds with RST|ACK to notify the server acknowledges
//    the reset of the sequence number.

// Op-Codes & Corresponding Value.
#define OP_SYN (0x01)
#define OP_ACK (0x02)
#define OP_BEG (0x04)
#define OP_FIN (0x08)
#define OP_ERR (0x10)
#define OP_FIL (0x20)
#define OP_RST (0x40)

//--Socket Set Up--//
#define SOCK_DOMAIN AF_INET
#define SOCK_TYPE SOCK_DGRAM
#define DEFAULT_PROTOCOL (0)
#define DEFAULT_PORT (0)

#define TRUE (1)
#define FALSE (0)

#define MAX_EXP_OPS (5) //max expected opcodes
#define SEQ_RECV_DIFF (1) //difference between current seq number and expected incoming seq


typedef struct {
  rcp_hdr *Hdr;
  uint8_t *datagram;
  uint32_t datagram_len;
  uint8_t *data;
  uint32_t data_len;
} rcp_pkt;


typedef struct {
  uint16_t opcode[MAX_EXP_OPS];
  uint32_t num_ops;
} exp_ops;

typedef struct {
  int sock;
  struct sockaddr_in remote;
  uint32_t seq;
  uint32_t buffsize;
  uint8_t *remote_filename;
  uint8_t *local_filename;
} client;

typedef struct {
  int sock;
  struct sockaddr_in remote;
  struct sockaddr_in local;
  uint32_t seq;
  uint32_t buffsize;
  char *filename;
} server;

//--sendErr--//
#define ERROR_RATE_ZERO (0)

//--OPCODE Specific Data Offsets--//
#define FILENAME_OFF (4)
#define BUFF_SIZE_OFF (0)
#define REMOTE_PORT_OFF (0)
#define ERRNO_DATA_OFFSET (0)


//-----------------------Prototypes---------------------------//


//--Set Up--//

// Client Sock
//   Generates and sets up a client side UDP socket.
client *client_sock(char *remote_address, uint16_t remote_port, uint32_t buffsize);

// Server Sock
//   Generates and sets up a server side UDP socket.
server *server_sock(uint32_t buffsize);


// Sets up the Send to Err
//  Pass in the error rate you want and it will call sendtoErr_init
//  with its according correct parameters.
void sendtoErr_setup(double error_rate);

//--Packets--//

// Create Packet
//   Pass in 
void create_pkt(rcp_pkt *Pkt, uint16_t opcode, uint32_t seq, uint8_t *data, uint32_t data_len);


// Send Packet
//  Sends any packet you wish.
void send_pkt(rcp_pkt *Pkt, int socket, struct sockaddr_in dst_addr);

// Receive Packet
// 
// Pkt needs to be properly malloced before this fu
void recv_pkt(rcp_pkt *Pkt, int socket, struct sockaddr_in *src_addr, uint32_t buffsize);

//--HDR--//
void create_hdr(rcp_pkt *Pkt, uint32_t seq, uint16_t opcode);


// Get RCP Header
// 
void get_hdr(rcp_pkt *Pkt);

void get_pkt(rcp_pkt *Pkt);

// Check RCP header

uint16_t pkt_checksum(rcp_pkt *Pkt);

//--Sate Check--//
//   Check to see if the pkt state is what your expecting.
//  Pkt is the packet which have been filled with data by calling recv_pkt(...)
//  opcode - is the opcoe which your expecting
//  seq - is the sequence number your expecting

int check_pkt_state(rcp_pkt *Pkt, uint16_t opcode, uint32_t seq);

int select_call(int socket, int seconds, int useconds);

rcp_pkt *pkt_alloc(uint32_t buffsize);

void print_opcode(uint16_t opcode);

//Opcode Data
//  What data an opcode is expecting in it's data field.
//
// SYN|BEG
//  Expects sending a buffer size.
//
//             DATA
//  0------------------------15
//  |       Buffer Size       |   
//  +-------------------------+
//
//
// SYN|FIL
//   Expects the file name which it is requesting to download.
//   The character array must end in '\0'
//  
//   uint8_t filename[256]
//
// ACK|FIL|ERR
//   Right after the SYN|FIL packet was sent. Server stating there
//   is an error with the file. Atomic stating types of errors.
//
//   uint32_t error;
//     
//   FIL_NAME_DNE - filename doesn't exist
//  

#endif
