#ifndef _NT_TCP_CLIENT_H_
#define _NT_TCP_CLIENT_H_

#include "obj.h"
#include "tcp_socket.h"
#include <event.h>

struct nt_event_base;
struct nt_event_base_server;
struct nt_tcp_server;


/**
  TCP client connection with buffered I/O.
  
  A client occupies 20 bytes on a 32-bit system and 36-40 bytes on a 64-bit system.
*/
typedef struct nt_tcp_client {
  NT_OBJ_HEAD
  
  /* Connection */
  nt_tcp_socket *socket;
  
  /* Buffer event */
  struct bufferevent *bev;
  
  /* Base and server tuple*/
  struct nt_event_base_server *bs;
} nt_tcp_client;


/**
  Called when there is data for the client to read.
*/
typedef void (nt_tcp_client_on_read)(struct bufferevent *bev, nt_tcp_client *client);

/**
  Called when the write buffer reaches 0.
*/
typedef void (nt_tcp_client_on_write)(struct bufferevent *bev, nt_tcp_client *client);

/**
  Called when an error occured.
  This callback is responsible for releasing the client.
*/
typedef void (nt_tcp_client_on_error)(struct bufferevent *bev, short what, nt_tcp_client *client);


/**
  Create a new nt_tcp_client object
*/
nt_tcp_client *nt_tcp_client_new(nt_tcp_socket *socket);

/**
  Accept a new client.
  Returns a new reference.
*/
nt_tcp_client *nt_tcp_client_accept(struct nt_event_base_server *bs,
                                    int server_fd,
                                    nt_tcp_client_on_read *on_read,
                                    nt_tcp_client_on_write *on_write,
                                    nt_tcp_client_on_error *on_error);

#endif
