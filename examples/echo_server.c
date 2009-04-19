/**
 This code is released in the Public Domain (no restrictions, no support
 100% free) by Notion.
*/
#include "../src/runloop.h"
#include "../src/sockserv.h"
#include "../src/sockconn.h"
#include "../src/mpool.h"
#include "../src/sockaddr.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <signal.h>
#include <event.h>

/**
  Helper to reduce noise in the code below
**/
#define HOSTPORT(addrptr) nt_sockaddr_host(addrptr), nt_sockaddr_port(addrptr)

/**
  We call this function when a new client have connected.
*/
static void on_connected(nt_sockconn_t *conn) {
  printf("connection %p (%s on port %d)\n", conn, HOSTPORT(&conn->addr));
  
  // Send a warm welcome
  static char data[] = 
    "Send something to me and I will send it back to you my friend.\n";
  nt_sockconn_write(conn, data, sizeof(data));
}

/**
  We call this function when a connection have been invalidated.
*/
static void on_disconnected(nt_sockconn_t *conn) {
  printf("connection %p closed (%s on port %d)\n", conn, HOSTPORT(&conn->addr));
  
  // Simply release our reference to conn
  nt_release(conn);
}

/**
  Called when it's time to read something a client is sending to us.
  
  In this program we simply send back whetever we received.
*/
static void on_conn_read(struct bufferevent *bev, nt_sockconn_t *conn) {
  /* 
    Simply write the bev->input to the buffer (bev->output), thus echoing
    bev->input back to the client.
  */
  //nt_sockconn_writebuf(conn, bev->input);
  /*
    The above line is roughly equivalent to this code:
    
    char buf[BUFLEN];
    int len = bufferevent_read(&client->bev, buf, BUFLEN);
    bufferevent_write(&client->bev, buf, len);
  */
  nt_sockutil_shutdown(conn->fd);
}

/**
  Handles connection "errors".
  
  Basically, this function is called when a client is no longer available.
  The cause may be a channel error or a nice disconnect.
*/
static void on_conn_error(struct bufferevent *bev, short what, nt_sockconn_t *client) {
  if (what & EVBUFFER_EOF)
    printf("connection %p EOF\n", client);
  if (what & EVBUFFER_ERROR)
    warn("connection %p error", client);
  else {
    printf("connection %p broken:", client);
    if (what & EVBUFFER_READ) printf(" EVBUFFER_READ");
    if (what & EVBUFFER_WRITE) printf(" EVBUFFER_WRITE");
    if (what & EVBUFFER_ERROR) printf(" EVBUFFER_ERROR");
    if (what & EVBUFFER_TIMEOUT) printf(" EVBUFFER_TIMEOUT");
    printf("\n");
  }
  nt_sockconn_close(client);
  on_disconnected(client);
}

/**
  Accepts new connections.
  
  This function is called when a new client is knocking on the door, waiting
  to be accept() ed. You have to accept (or remove) the event, or else we will
  end up in an never-ending loop (because of nt_event_base).
*/
static void on_accept(int fd, short ev, nt_sockserv_runloop_t *rs) {
  nt_sockconn_t *conn;
  conn = nt_sockconn_new();
  
  // Set read and write timeouts, in seconds
  nt_sockconn_settimeout(conn, 5, 5);
  
  if (nt_sockconn_accept(conn, rs, fd, &on_conn_read, NULL, &on_conn_error))
    on_connected(conn);
  else
    nt_release(conn);
}

/**
  Called when a signal which we have subscribed to was raised
**/
static void on_signal(int signum, short event, struct event *ev) {
	printf("%s: got signal %d %s\n", __func__, signum, sys_signame[signum]);
  event_base_loopbreak(ev->ev_base);
}


int main(int argc, char * const *argv) {
  // Our server and runloop
  nt_sockserv_t *server;
  nt_runloop_t *runloop;
  
  // Command line arguments, with default values
  char *addr = "";  // Address or hostname to bind to
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
  server = nt_sockserv_new(&on_accept);
  
  // bind
  if (!nt_sockserv_bind(server, addr, port, SOCK_STREAM, AF_UNSPEC))
    exit(1);
  
  // listen
  nt_sockserv_listen(server);
  
  // print bound addresses to stdout
  if (server->fd4 != -1)
    printf("listening on %s:%d\n", HOSTPORT(&server->addr4));
  if (server->fd6 != -1)
    printf("listening on [%s]:%d\n", HOSTPORT(&server->addr6));
  
  /** -----------------------------------------------------------------------
  
    Setup a runloop and add the server to it.
    
    A runloop handles asynchronous I/O in the most efficient manner
    possible (epoll, kqueue, etc).
    
    In a multi-threaded environment you have one runloop per thread, and
    add the server to each runloop.
  */
  
  runloop = nt_runloop_new();
  
  // Add our server
  if (!nt_runloop_addsockserv(runloop, server))
    exit(1);
  
  // Register signal handler
  struct event sigev;
	event_set(&sigev, SIGPIPE, EV_SIGNAL|EV_PERSIST, (void (*)(int, short, void *))on_signal, (void *)&sigev);
  nt_runloop_addev(runloop, &sigev, NULL);
  
  // Run runloop
  int rc = nt_runloop_run(runloop, 0);
  if (rc < 0)
    warnx("nt_event_base_loop: error");
  else if (rc > 0)
    warnx("nt_event_base_loop: no events");
  
  
  /** ----------------------------------------------------------------------- */
  
  // release our reference to base
  nt_release(runloop);
  
  // release our reference to server
  nt_release(server);
  
  return 0;
}
