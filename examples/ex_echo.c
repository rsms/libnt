#include "src/event_base.h"
#include "src/tcp_server.h"
#include "src/tcp_client.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <event.h>

static void on_client_connected(nt_tcp_client *client) {
  printf("client %p connected from %s\n", client, nt_tcp_socket_host(client->socket));
  // Send a hello message
  static char data[] = "Send something to me and I will send it back to you my friend.\n";
  bufferevent_write(client->bev, data, sizeof(data));
}

static void on_client_read(struct bufferevent *bev, nt_tcp_client *client) {
  /* 
    Simply write the bev->input to the buffer (bev->output), thus echoing
    bev->input back to the client.
  */
  if (bufferevent_write_buffer(bev, bev->input) != 0)
    warnx("bufferevent_write_buffer() failed");
  /*
    The above lines is roughly equivalent to this code:
    
    char buf[BUFLEN];
    int len = bufferevent_read(bev, buf, BUFLEN);
    if (len == 0) {
      warnx("bufferevent_read() failed");
    }
    else {
      if (bufferevent_write(bev, buf, len) != 0)
        warnx("bufferevent_write() failed");
    }
  */
}

static void on_client_error(struct bufferevent *bev, short what, nt_tcp_client *client) {
  if (what & EVBUFFER_EOF)
    printf("client %p disconnected\n", client);
  else
    warnx("client %p error\n", client);
  nt_release(client);
}

static void on_accept(int fd, short ev, nt_event_base_server *bs) {
  nt_tcp_client *client;
  if ( (client = nt_tcp_client_accept(bs, fd, &on_client_read, NULL, &on_client_error)) )
    on_client_connected(client);
}


#include <string.h>
#include <libkern/OSAtomic.h>
int testq() {
  
  
  typedef struct elem {
    long    data1;
    struct elem *link;
    int     data2;
  } elem_t;

  elem_t fred, mary, *p;

  OSQueueHead q = OS_ATOMIC_QUEUE_INIT;

  OSAtomicEnqueue( &q, &fred, offsetof(elem_t,link) );
  OSAtomicEnqueue( &q, &mary, offsetof(elem_t,link) );

  p = OSAtomicDequeue( &q, offsetof(elem_t,link) );
  
  // -------------
  
  nt_atomic_queue_head q2 = NT_ATOMIC_QUEUE_INIT;
  
  nt_atomic_queue_put(&q2, &fred, offsetof(elem_t,link));
  nt_atomic_queue_put(&q2, &mary, offsetof(elem_t,link));
  
  /*char *rp = NULL;
  rp = (char *)nt_atomic_queue_get((void **)&q, 0);
  printf("rp = %s\n", rp);
  rp = (char *)nt_atomic_queue_get((void **)&q, 0);
  printf("rp = %s\n", rp);
  rp = (char *)nt_atomic_queue_get((void **)&q, 0);
  printf("rp = %s\n", rp);*/
  
  return 1;
}


int main(int argc, char * const *argv) {
  nt_tcp_server *server;
  nt_event_base *base;
  char *addr = "";
  int port = 8080, rc;
  bool ipv6_enabled = true;
  bool ipv6_only = false;
  bool blocking = false;
  
  if(testq())
    return 0;
  
  // parse arguments
  if (argc > 1)
    addr = argv[1];
  if (argc > 2)
    port = atoi(argv[2]);

  // ------------ server: ------------
  
  // Create server
  server = nt_tcp_server_new(&on_accept);
  
  // bind
  if (!nt_tcp_server_bind(server, addr, port, ipv6_enabled, ipv6_only, blocking))
    exit(1);
  
  // print some info to stdout
  if (server->socket4) {
    printf("listening on %s:%d\n", nt_tcp_socket_host(server->socket4),
      nt_tcp_socket_port(server->socket4));
  }
  if (server->socket6) {
    printf("listening on [%s]:%d\n", nt_tcp_socket_host(server->socket6),
      nt_tcp_socket_port(server->socket6));
  }
  
  // ------------ base: ------------
  
  base = nt_event_base_new();
  
  // Add our server
  if (!nt_event_base_add_server(base, server, NULL))
    exit(1);
  
  // Enter event base runloop
  rc = nt_event_base_loop(base, 0);
  if (rc < 0)
    warnx("nt_event_base_loop: error");
  else if (rc > 0)
    warnx("nt_event_base_loop: no events");
  
  // release our reference to base
  nt_release(base);
  
  // release our reference to server
  nt_release(server);
  
  return 0;
}
