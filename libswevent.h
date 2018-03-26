#include <sw_event.h>
#include "../hiredis.h"
#include "../async.h"
#include <fcntl.h>

typedef struct redisLibsweventEvents {
    redisAsyncContext *context;
    struct sw_ev_context *sw_ctx;
    int fd;
} redisLibsweventEvents;

void redisLibsweventReadEvent(int fd, int event, void *arg) {
    ((void)fd); ((void)event);
    redisLibsweventEvents *e = (redisLibsweventEvents*)arg;
    redisAsyncHandleRead(e->context);
}

void redisLibsweventWriteEvent(int fd, int event, void *arg) {
    ((void)fd); ((void)event);
    redisLibsweventEvents *e = (redisLibsweventEvents*)arg;
    redisAsyncHandleWrite(e->context);
}

void redisLibsweventIoReadyEvent(int fd, int event, void *arg) {
    if (event & SW_EV_READ)
    {
        redisLibsweventReadEvent(fd, event, arg);
    }
    if (event & SW_EV_WRITE)
    {
        redisLibsweventWriteEvent(fd, event, arg);
    }
};

void redisLibsweventAddRead(void *privdata) {
    redisLibsweventEvents *e = (redisLibsweventEvents*)privdata;
    sw_ev_io_add(e->sw_ctx, e->fd, SW_EV_READ, redisLibsweventIoReadyEvent, e);
}

void redisLibsweventDelRead(void *privdata) {
    redisLibsweventEvents *e = (redisLibsweventEvents*)privdata;
    sw_ev_io_del(e->sw_ctx, e->fd, SW_EV_READ);
}

void redisLibsweventAddWrite(void *privdata) {
    redisLibsweventEvents *e = (redisLibsweventEvents*)privdata;
    sw_ev_io_add(e->sw_ctx, e->fd, SW_EV_WRITE, redisLibsweventIoReadyEvent, e);
}

void redisLibsweventDelWrite(void *privdata) {
    redisLibsweventEvents *e = (redisLibsweventEvents*)privdata;
    sw_ev_io_del(e->sw_ctx, e->fd, SW_EV_WRITE);
}

void redisLibsweventCleanup(void *privdata) {
    redisLibsweventEvents *e = (redisLibsweventEvents*)privdata;
    redisLibsweventDelRead(privdata);
    redisLibsweventDelWrite(privdata);
    free(e);
}

int redisLibsweventAttach(redisAsyncContext *ac, struct sw_ev_context *ctx) {
    redisContext *c = &(ac->c);
    redisLibsweventEvents *e;

    /* Nothing should be attached when something is already attached */
    if (ac->ev.data != NULL)
        return REDIS_ERR;

    /* Create container for context and r/w events */
    e = (redisLibsweventEvents*)malloc(sizeof(*e));
    e->context = ac;
    e->sw_ctx = ctx;
    e->fd = c->fd;

    /* Register functions to start/stop listening for events */
    ac->ev.addRead = redisLibsweventAddRead;
    ac->ev.delRead = redisLibsweventDelRead;
    ac->ev.addWrite = redisLibsweventAddWrite;
    ac->ev.delWrite = redisLibsweventDelWrite;
    ac->ev.cleanup = redisLibsweventCleanup;
    ac->ev.data = e;

    return REDIS_OK;
}

