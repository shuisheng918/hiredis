#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hiredis.h"
#include "async.h"
#include "adapters/libswevent.h"

#define TEST_COUNT 100000

void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    if (reply == NULL) return;
    long i = (long)privdata;
    if (i % 5000 == 0)
    {
        printf("i=%ld\n", i);
    }
    if (i == TEST_COUNT)
    {
        redisAsyncDisconnect(c);
    }
}

void setCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    if (reply == NULL) return;
    long i = (long)privdata;
    if (i % 5000 == 0)
    {
        printf("i=%ld\n", i);
    }
    if (i == TEST_COUNT)
    {
        redisAsyncDisconnect(c);
    }
}

void delCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    if (reply == NULL) return;
    long i = (long)privdata;
    if (i % 5000 == 0)
    {
        printf("i=%ld\n", i);
    }
    if (i == TEST_COUNT)
    {
        redisAsyncDisconnect(c);
    }
}

void subscirbeCallback(redisAsyncContext *c, void *r, void *privdata) {
    (void)c;
    (void)privdata;
    redisReply *reply = r;
    if (reply == NULL) return;
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        if (0 == strcmp(reply->element[0]->str, "subscribe"))
        {
            printf("subscribe channel %s ok\n", reply->element[1]->str);
        }
        else
        {
            printf("recv publish msg: %s, from channel %s\n", reply->element[2]->str, reply->element[1]->str);
        }
    }
}

void connectCallback(const redisAsyncContext *c) {
    ((void)c);
    printf("connected...\n");
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
    }
    printf("disconnected...\n");
    exit(0);
}



int main (int argc, char **argv) {
    (void)argc;
    (void)argv;
    signal(SIGPIPE, SIG_IGN);
    struct sw_ev_context *ctx = sw_ev_context_new();
    int i;
    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    if (c->err) {
        /* Let *c leak for now... */
        printf("Error: %s\n", c->errstr);
        return 1;
    }

    redisLibsweventAttach(c, ctx);
    redisAsyncSetConnectCallback(c,connectCallback);
    redisAsyncSetDisconnectCallback(c,disconnectCallback);
    
    for (i = 0; i <= TEST_COUNT; ++i)
    {
        redisAsyncCommand(c, setCallback, (char*)i, "SET key_%d %d", i, i);
        //redisAsyncCommand(c, getCallback, (char*)i, "GET key_%d", i);
        //redisAsyncCommand(c, delCallback, (char*)i, "DEL key_%d", i);
    }
    
    //redisAsyncCommand(c, subscirbeCallback, (void*)"sub-23434", "subscribe news");
    sw_ev_loop(ctx);
    return 0;
}
