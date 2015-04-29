#include"kv.h"
kv * kv::instance =NULL;

kv::kv() {
    kvServerMap.clear();
    xmlParse *xp = xmlParse::getInstance();
    cacheType = xp->getKvType();
    for (int i = 1; i <= xp->getKvCount(); i++) {
        addServer(i, xp->getKvDetailConfig(i).ip, xp->getKvDetailConfig(i).port);
    }
    log = logger::getInstance();
}

kv::~kv(){
    if(kvServerMap.size()>0){
        for(map<int, kvServer*>::iterator iter = kvServerMap.begin();iter!=kvServerMap.end();iter++){
            delete iter->second;
        }
    }
}

kv * kv::getInstance(){
    if(instance==NULL){
        instance = new kv();
    }
    return instance;
}

void kv::addServer(const int &sid, const char *ip,const int port) {
    
 
    if (cacheType == "redis") {
        kvServerMap[sid] = new redisServer();
        kvServerMap[sid]->getConnection(ip, port);
        log->info("加载redis服务器("+string(ip)+":"+intToString(port)+")配置完成.");
    } else if (cacheType == "memcached") {
        kvServerMap[sid] = new memcachedSserver();
        kvServerMap[sid]->getConnection(ip, port);
         log->info("加载memcached服务器("+string(ip)+":"+intToString(port)+")配置完成.");
    } else {
        cerr<<"缓存服务器类型配置错误."<<endl;;
        exit(255);
    }
}

void kv::pingServer(const int& sid){
    kvServerMap[sid]->reConnection();
    return ;
}

string kv::get(const int& sid, const char *key){
    return kvServerMap[sid]->get(key);
}

bool kv::set(const int& sid, const char *key, const  char *val){
    return kvServerMap[sid]->set(key,val);
}
