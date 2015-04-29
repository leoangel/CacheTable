#include"kvserver.h"

void redisServer::getConnection(const char *ip, const int port) {
    struct timeval timeout = {1, 500000}; // 1.5 seconds timeout
    redisConn = redisConnectWithTimeout(ip, port, timeout);
    if (redisConn->err || redisConn == NULL) {
        log->error("连接redis服务器失败! 服务器:" + string(ip) + " 端口号:" + intToString(port));
        cerr << "连接redis服务器失败. 服务器:" + string(ip) + " 端口号:" + intToString(port) << endl;
        redisFree(redisConn);
        exit(255);
    }
    memset(kvhost,'\0',sizeof(kvhost));
    memcpy(kvhost,ip,strlen(ip));
    kvport = port;
}

void redisServer::reConnection(void) {
    log->warn("探测Redis服务器是否存活.");
    while (1) {
        reply =(redisReply *) redisCommand(redisConn, "PING");
        if (reply != NULL && !strcmp(reply->str, "PONG")) {
            log->info("Redis服务器["+string(kvhost)+":"+intToString(kvport)+"] PING成功");
            break;
        }
        redisFree(redisConn);
        struct timeval timeout = {1, 500000}; // 1.5 seconds timeout
        redisConn = redisConnectWithTimeout(kvhost, kvport, timeout);
        if (!redisConn->err && redisConn != NULL) {
            log->info("Redis服务器["+string(kvhost)+":"+intToString(kvport)+"] 重连成功.");
            break;
        }
        usleep(100000);
    }
}

bool redisServer::set(const char * key, const char *val) {
    reply = (redisReply *) redisCommand(redisConn, "SET %s %s", key, val);
    if (reply == NULL) {
        log->error("set key:" + string(key)+" faied.");
        return false;
    }
    freeReplyObject(reply);
    return true;
}

string redisServer::get(const char *key) {
    reply = (redisReply *) redisCommand(redisConn, "GET %s", key);
    if (reply == NULL) {
        log->error("get key:" + string(key) + " failed.");
    }
    string tmp = string(reply->str);
    freeReplyObject(reply);
    return tmp;
}

redisServer::~redisServer() {
    redisFree(redisConn);
}

void memcachedSserver::getConnection(const char *ip, const int port) {
    memcachedConn = memcached_create(NULL);
    memcachedSvr = memcached_server_list_append(NULL, ip, port, &memcachedRet);
    memcachedRet = memcached_server_push(memcachedConn, memcachedSvr);
    memcached_behavior_set(memcachedConn, MEMCACHED_BEHAVIOR_TCP_KEEPALIVE, 1);
    memcached_behavior_set(memcachedConn, MEMCACHED_BEHAVIOR_TCP_NODELAY, 1);
    if (MEMCACHED_SUCCESS != memcachedRet) {
        log->error("连接memcached服务器失败! 服务器:" + string(ip) + " 端口号:" + intToString(port));
        cerr << "连接memcached服务器失败! 服务器:" + string(ip) + " 端口号:" + intToString(port) << endl;
        memcached_free(memcachedConn);
        exit(255);
    }
    memcached_server_list_free(memcachedSvr);

    //获取cache过期时间
    xmlParse *xp = xmlParse::getInstance();
    cacheExpires = atoi(xp->getCacheTableProp("expires").c_str());
    log->info("Memcached过期时间:" + xp->getCacheTableProp("expires"));

    memset(kvhost,'\0',sizeof(kvhost));
    memcpy(kvhost,ip,strlen(ip));
    kvport = port;
}

void memcachedSserver::reConnection(void){
    //memcached自带重连机制，故不需要做什么
    return ;
}

bool memcachedSserver::set(const char *key, const char *val) {
    if (key == NULL || val == NULL) {
        return false;
    }
    memcachedRet = memcached_set(memcachedConn, key, strlen(key), val, strlen(val), cacheExpires, 0);
    if (MEMCACHED_SUCCESS != memcachedRet) {
        log->error("set key:" + string(key)+"  failed.");
        return false;
    } else {
        return true;
    }

}

string memcachedSserver::get(const char *key) {
    if (key == NULL) {
        return "";
    }
    size_t value_length;
    char * value = memcached_get(memcachedConn, key, strlen(key), &value_length, 0, &memcachedRet);
    if (MEMCACHED_SUCCESS != memcachedRet) {
        log->error("get key:" + string(key) + " failed.");
        return "";
    }
    return value;
}

memcachedSserver::~memcachedSserver() {
    memcached_free(memcachedConn);
}

