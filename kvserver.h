/* 
 * File:   kvserver.h
 * Author: leo
 *
 * Created on 2013年6月1日, 下午5:36
 */

#ifndef KVSERVER_H
#define	KVSERVER_H
#include<libmemcached/memcached.hpp>
#include<hiredis/async.h>
#include<hiredis/hiredis.h>
#include<string>
#include "logger.h"
#include "parse.h"
#include "core.h"
using namespace std;

class kvServer {
public:

    virtual void getConnection(const char *ip, const int port) {
    };

    virtual bool set(const char *key, const char *val) {
    };

    virtual string get(const char *key) {
    };

    virtual void reConnection(void) {
    };

    kvServer() {
        log = logger::getInstance();
    };
protected:
    logger *log;
    char kvhost[15];
    int kvport;

};

class redisServer : public kvServer {
public:
    void getConnection(const char *ip, const int port);
    void reConnection(void);

    bool set(const char *key, const char *val);
    string get(const char *key);
    ~redisServer();
private:
    redisContext *redisConn;
    redisReply *reply;
};

class memcachedSserver : public kvServer {
public:
    void getConnection(const char *ip, const int port);
    void reConnection(void);
    bool set(const char *key, const char *val);
    string get(const char *key);
    ~memcachedSserver();
private:
    memcached_st *memcachedConn;
    memcached_server_st *memcachedSvr;
    memcached_return memcachedRet;
    uint32_t cacheExpires;
};



#endif	/* KVSERVER_H */

