/**
  Copyright (c) 2009 Notion <http://notion.se/>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
#include "sockconn.h"
#include "sockserv.h"
#include "fd.h"
#include "sockutil.h"
#include "runloop.h"
#include "mpool.h"


static void _dealloc(nt_sockconn_t *self) {
  nt_sockconn_close(self);
  if (self->bev.input)
    nt_free(self->bev.input, sizeof(struct evbuffer));
  if (self->bev.output)
    nt_free(self->bev.output, sizeof(struct evbuffer));
  nt_free(self, sizeof(nt_sockconn_t));
}


nt_sockconn_t *nt_sockconn_new() {
  NT_OBJ_ALLOC_INIT_self(nt_sockconn_t, &_dealloc);
  NT_OBJ_CLEAR(self, nt_sockconn_t);
  
  self->fd = -1;
  self->bev.input = nt_calloc(1, sizeof(struct evbuffer));
  if ( !self->bev.input || !(self->bev.output = nt_calloc(1, sizeof(struct evbuffer))) ) {
    nt_release(self);
    return NULL;
  }
  /*
	 * Set to EV_WRITE so that using bufferevent_write is going to
	 * trigger a callback.  Reading needs to be explicitly enabled
	 * because otherwise no data will be available.
	 */
	self->bev.enabled = EV_WRITE;
	
  return self;
}


void nt_sockconn_setfd(nt_sockconn_t *self, int fd) {
  self->fd = fd;
  // might need to event_set first, with dummy values since bufferevent_setfd
  // performs event_del which might fail in future versions.
  bufferevent_setfd(&self->bev, self->fd);
}


void nt_sockconn_setrs(nt_sockconn_t *self, nt_sockserv_runloop_t *rs) {
  self->rs = rs;
  // Set base if needed
  if (self->rs->runloop)
    AZ(bufferevent_base_set(self->rs->runloop->ev_base, &self->bev));
}


NT_STATIC_INLINE
void _default_errorcb(struct bufferevent *bev, short what, nt_sockconn_t *client) {
  if (!(what & EVBUFFER_EOF))
    warn("socket error");
  nt_xrelease(client);
}


void nt_sockconn_setcb( nt_sockconn_t *self,
                          nt_sockconn_readcb_t readcb,
                          nt_sockconn_writecb_t writecb,
                          nt_sockconn_errorcb_t errorcb)
{
  if (errorcb == NULL)
    errorcb = &_default_errorcb;
	bufferevent_setcb(&self->bev,
	  (evbuffercb)readcb, (evbuffercb)writecb, (everrorcb)errorcb,
	  (void *)self);
}


bool nt_sockconn_accept(nt_sockconn_t *self,
                        nt_sockserv_runloop_t *rs,
                        int fd,
                        nt_sockconn_readcb_t readcb,
                        nt_sockconn_writecb_t writecb,
                        nt_sockconn_errorcb_t errorcb)
{
  self->fd = nt_sockutil_accept(fd, &self->addr);
  assert(self->fd != -1);
  nt_sockconn_setfd(self, self->fd);
  nt_sockconn_setrs(self, rs);
  nt_sockconn_setcb(self, readcb, writecb, errorcb);
  nt_sockutil_setblocking(self->fd, false);
  nt_runloop_addsockconn(rs->runloop, self);
  
  return true;
}


void nt_sockconn_close(nt_sockconn_t *self) {
  assert(self->rs != NULL);
  nt_runloop_rmsockconn(self->rs->runloop, self);
  nt_fd_close(&self->fd);
}


void nt_sockconn_abort(nt_sockconn_t *self) {
  struct linger lingeropt;
  lingeropt.l_onoff = 1;
  lingeropt.l_linger = 0;
  AZ(setsockopt(self->fd, SOL_SOCKET, SO_LINGER, (char *)&lingeropt, sizeof(lingeropt)));
  nt_sockutil_shutdown(self->fd);
  nt_sockconn_close(self);
}
