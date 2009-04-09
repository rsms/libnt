/**
 This code is released in the Public Domain (no restrictions, no support
 100% free) by Notion.
*/
#include "../src/runloop.h"
#include "../src/tcp_server.h"
#include "../src/tcp_client.h"
#include "../src/mpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <event.h>

/**
  We call this function when a new client have connected.
*/
static void on_client_connected(nt_tcp_client_t *client) {
  printf("client %p connected from %s on port %d\n", client, 
    nt_tcp_socket_host(client->socket), nt_tcp_socket_port(client->socket));
  // Send a warm welcome
  static char data[] = 
    "Send something to me and I will send it back to you my friend.\n";
  bufferevent_write(client->bev, data, sizeof(data));
}

/**
  Called when it's time to read something a client is sending to us.
  
  In this program we simply send back whetever we received.
*/
static void on_client_read(struct bufferevent *bev, nt_tcp_client_t *client) {
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

/**
  Handles connection "errors" (might EOF or a true error).
  
  Basically, this function is called when a client is no longer available.
  The cause may be a channel error or a normal disconnect.
*/
static void on_client_error(struct bufferevent *bev, short what, nt_tcp_client_t *client) {
  if (what & EVBUFFER_EOF)
    printf("client %p disconnected\n", client);
  else
    warnx("client %p error\n", client);
  nt_release(client); // Probably freeling the client, implying closing the
                      // underlying socket.
}

/**
  Accepts new connections.
  
  This function is called when a new client is knocking on the door, waiting
  to be accept() ed. You have to accept (or remove) the event, or else we will
  end up in an never-ending loop (because of nt_event_base).
*/
static void on_accept(int fd, short ev, nt_event_base_server *bs) {
  nt_tcp_client_t *client;
  if ( (client = nt_tcp_server_accept_client(bs, fd, &on_client_read, NULL, &on_client_error)) ) {
    /* We disable the Nagle algorithm, sending messages directly */
    //nt_tcp_socket_setiopt(client->socket, IPPROTO_TCP, TCP_NODELAY, 1);
    /* And call our on_client_connected function */
    on_client_connected(client);
  }
  // In a real program, you would proabaly handle a NULL return from
  // nt_tcp_client_accept here.
}


int main(int argc, char * const *argv) {
  // Our server and base objects
  nt_tcp_server *server;
  nt_event_base *base;
  
  // Parameters written out for educational purposes
  bool ipv6_enabled = true; // Enable binding to IP v6 addresses
  bool ipv6_only = false;   // Only allow binding to IP v6 addresses
  
  // Command line arguments, with default values
  char *addr = "";  // Address to bind to, in human notion
  int port = 7000;  // Port to bind to
  
  // Parse command line arguments
  if (argc > 1)
    addr = argv[1];
  if (argc > 2)
    port = atoi(argv[2]);
  
  /** -----------------------------------------------------------------------
    
    Setup the global memory pool (optional).
    
    You can comment out this whole block in order to disable the library
    global memory pool, which will cause the library to use standard malloc()
    and friends.
  */
  
  int error_p = 0;
  nt_mpool_shared = nt_mpool_open(
    0/* standard mpool */,
    0/* system-default pagesize */,
    NULL/* let mmap decide start address */,
    &error_p/* mpool error pointer */);
  if (!nt_mpool_shared) {
    warnx("failed to open memory pool: %s -- disabling shared mpool",
      nt_mpool_strerror(error_p));
  }
  
  /** -----------------------------------------------------------------------
    
    Create and bind a TCP server.
    
  */
  
  // Create server
  server = nt_tcp_server_new(&on_accept);
  
  // bind
  if (!nt_tcp_server_bind(server, addr, port, ipv6_enabled, ipv6_only))
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
  
  /** -----------------------------------------------------------------------
  
    Create an event base, add the server and enter the runloop.
    
    A nt_event_base handles asynchronous I/O in the most efficient manner
    possible (epoll, kqueue, etc).
    
    In a multi-threaded environment you have one event base per thread and
    add the server to each event base.
  
  */
  
  base = nt_event_base_new();
  
  // Add our server
  if (!nt_event_base_add_server(base, server, NULL))
    exit(1);
  
  // Enter event base runloop
  int rc = nt_event_base_loop(base, 0);
  if (rc < 0)
    warnx("nt_event_base_loop: error");
  else if (rc > 0)
    warnx("nt_event_base_loop: no events");
  
  
  /** ----------------------------------------------------------------------- */
  
  // release our reference to base
  nt_release(base);
  
  // release our reference to server
  nt_release(server);
  
  return 0;
}
