#include "tcp_client.h"
#include "tcp_server.h"
#include "event_base.h"


static void _dealloc(nt_tcp_client *self) {
  // free the bufferevent
  bufferevent_free(self->bev);
  
  // release socket. implies closing any connection
  nt_xrelease(self->socket);
  
  // finally free ourselves
  free(self);
}


nt_tcp_client *nt_tcp_client_new(nt_tcp_socket *socket) {
  nt_tcp_client *self;
  
  if ( !(self = (nt_tcp_client *)malloc(sizeof(nt_tcp_client))) )
    return NULL;
  
  nt_obj_init((nt_obj *)self, (nt_obj_destructor *)_dealloc);
  
  // Socket
  nt_xretain(socket);
  self->socket = socket;
  
  return self;
}


static void _bev_on_read(struct bufferevent *bev, nt_tcp_client *client) {
  evbuffer_drain(bev->input, bev->input->off);
}

static void _bev_on_write(struct bufferevent *bev, nt_tcp_client *client) {
}

static void _bev_on_error(struct bufferevent *bev, short what, nt_tcp_client *client) {
  if (!(what & EVBUFFER_EOF)) {
    warn("client socket error -- disconnecting client\n");
  }
  nt_release(client);
}


nt_tcp_client *nt_tcp_client_accept(nt_event_base_server *bs,
                                    int server_fd,
                                    nt_tcp_client_on_read *on_read,
                                    nt_tcp_client_on_write *on_write,
                                    nt_tcp_client_on_error *on_error)
{
  nt_tcp_socket *sock;
  nt_tcp_client *client;
  int af;
  
  af = (bs->server->socket6 && bs->server->socket6->fd == server_fd) ? AF_INET6 : AF_INET;
  
  // Accept connection
  if ( (sock = nt_tcp_socket_accept(server_fd, af)) == NULL )
    return NULL;
  
  // Set non-blocking
  if (nt_util_fd_setnonblock(sock->fd) == -1)
    return NULL;
  
  // Create a new client
  if ( (client = nt_tcp_client_new(sock)) == NULL ) {
    nt_release(sock);
    return NULL;
  }
  
  // As nt_tcp_client_new retains a reference to sock, we release ours here
  nt_release(sock);
  
  // Set pointer to base-server tuple
  client->bs = bs;
  
  // Make sure the callbacks are valid
  if (on_read == NULL) on_read = &_bev_on_read;
  if (on_write == NULL) on_write = &_bev_on_write;
  if (on_error == NULL) on_error = &_bev_on_error;
  
  // Create a new bufferevent
  client->bev = bufferevent_new(sock->fd,
    (void (*)(struct bufferevent *, void *))on_read,
    (void (*)(struct bufferevent *, void *))on_write,
    (void (*)(struct bufferevent *, short, void *))on_error,
    (void *)client);
  if (client->bev == NULL) {
    nt_release(client);
    return NULL;
  }
  
  // Set base if not NULL
  if (bs->base) {
    if (bufferevent_base_set(bs->base->ev_base, client->bev) == -1) {
      nt_release(client);
      return NULL;
    }
  }
  
  return client;
}