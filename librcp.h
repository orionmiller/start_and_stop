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

#include "safe.h"
#include "checksum.h"

extern int h_errno;

//Is the buffer size absolute for the entire packet or is it the
//size of the data/payload coming.

// RCP - Application Header
//
//
// 0----------------------------15----------------------------31
// |           Opcode            |          Checksum           |
// |-----------------------------------------------------------|
// |                       Header Info                         |
// |-----------------------------------------------------------|
// |                        Sequence                           |
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
// Header Info
//   Buff Size
//     When a SYN packet is sent a buffer size should be put inside the info 
//     field so the server knows what size of the data buffer is.
//
//   File Offset
//     Contains the file offset based off of the buff size.
//     (e.g. Buff Size = 500 bytes & offset = 3
//

typedef struct {
  uint16_t opcode;
  uint16_t checksum;
  uint32_t info;
}rcp_hdr;

// RCP HDR 
#define RCP_HDR_LEN (sizeof(rcp_hdr))
#define RCP_OPCODE_OFFSET (sizeof(uint16_t))
#define RCP_CHECKSUM_OFFSET (sizeof(uint16_t))
#define RCP_INFO_OFFSET (sizeof(uint32_t))

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
#define DEFAULT_PROTOCOL 0
#define DEFAULT_PORT 0

typedef struct{
  int sock;
  struct sockaddr_in remote;
}client;

typedef struct{
  int sock;
  struct sockaddr_in remote;
  struct sockaddr_in local;
  //  uint32_t l_len;
  //  uint32_T r_len;
  
}server;


// Client Sock
//   Generates and sets up a client side UDP socket.
client *client_sock(char *remote_addr, char *remote_port);

// Server Sock
//   Generates and sets up a server side UDP socket.
server *server_sock(void);//char *remote_addr, char *remote_port);

//--RCP Packets--//

// Send SYN
//   Sends a SYN packet through the given socket.
int send_big(int socket);

// Send ACK
//   Sends an ACK packet through the given socket.
int send_ack(int socket);

// Send FIN
//   Sends a FIN packet through the given socket.
int send_fin(int socket);

// Send ERR
//   Sends an ERR packet through the given socket.
int send_err(int socket, rcp_hdr *Rcp_Hdr);

// Send FIL
//   Sends a 
int send_fil(int socket); //add a file descriptor or file pointer

void send_msg(int socket, char *msg, size_t length, struct sockaddr *dst_addr);

void recv_msg(int socket, void *buffer, size_t length, struct sockaddr *src_addr);
//--RCP HDR--//

// 
rcp_hdr *get_rcp_hdr(uint8_t *datagram);

// 
int check_rcp_hdr(uint8_t *datagram);

