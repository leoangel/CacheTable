/* 
 * File:   kv.h
 * Author: leo
 *
 * Created on 2013年5月31日, 下午11:04
 */

#ifndef KV_H
#define	KV_H


#include<libmemcached/memcached.hpp>
#include<hiredis/async.h>
#include<hiredis/hiredis.h>
#include<string>
#include "logger.h"
#include"kvserver.h"
using namespace std;


class kv {
private:
    static kv * instance;
    map<int, kvServer*> kvServerMap;
    kv();
    string cacheType;
    logger *log;
public:
    ~kv();
    static kv * getInstance();
    void addServer(const int &sid, const char *ip,const int port);
    void pingServer(const int &sid);
    bool set(const int &sid, const char *key, const char *val);
    string get(const int &sid, const char *key);
};

#endif	/* KV_H */

