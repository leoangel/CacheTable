/* 
 * File:   main.cpp
 * Author: leo
 *
 * Created on 2013年5月31日, 下午10:59
 */


#include<unistd.h>
#include <string.h>
#include<sys/param.h>
#include<signal.h>
#include"cJSON.h"
#include"cachetable.h"
#include"logger.h"
#include "parse.h"
#include "kv.h"
char configXml[200];

void init_daemon(void) {

    // Create child process
    pid_t pid = fork();
 
    if (pid < 0) {
        cerr << "开启后台运行模式失败";
        exit(1);
    }
    // Child process created, exit parent process
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    // Set file permission for files created by our child process
    umask(0);
 
    // Create session for our new process
    pid_t sid = setsid();
    if (sid < 0) {
         cerr << "开启后台运行模式失败";
         exit(1);
    }
 
    // close all standard file descriptors.
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
 
    // Ignore signal  when terminal session is closed.  This keeps our
    // daemon alive even when user closed terminal session
    signal(SIGHUP, SIG_IGN);
    return;
}

void getoption(int argc, char ** argv) {
    int ret;
    bzero(configXml,sizeof(configXml));
    memcpy(configXml,configXmlDefaultPath,strlen(configXmlDefaultPath));
    while ((ret = getopt(argc, argv, "c:d")) != -1) {
        switch (ret) {
            case 'c':
                bzero(configXml, sizeof (configXml));
                memcpy(configXml, optarg, strlen(optarg));
                break;
            case 'd':
                init_daemon();
                break;
            default:
                cerr << "参数错误!" << endl;
                exit(255);
        }
    }
}

kv* kvs;
data *dataObj;

void *pushCache(void *arg) {
    logger *log = logger::getInstance();
    dataSet *begin;
    int sid = (long) arg;
    while (1) {
        dataObj->waitData(sid);
        begin=dataObj->begindataset;
        for (int i=0;i<dataObj->loopCount;i++) {
            if (!kvs->set(sid, begin->key, begin->val)) {
                log->error("写入数据到缓存失败.");
                kvs->pingServer(sid);
            }
            begin++;
        }
        dataObj->cacheDone(sid);
    }
    delete log;
    pthread_exit((void*) 0);
}

int main(int argc, char** argv) {
    getoption(argc, argv);
    dataObj = new data();
    logger *log = logger::getInstance();
    kvs = kv::getInstance();
    pthread_t pid;
    
    for (int i = 1; i <= dataObj->getThreadNum(); i++) {
        if (pthread_create(&pid, NULL, pushCache, (void *) i) != 0) {
            log->error("创建缓存同步线程#"+intToString(i)+"失败.");
        }else{
            log->info("创建缓存同步线程#"+intToString(i)+"成功.");
          }
    }
    dataObj->loadData();
    delete dataObj;
    delete kvs;
    return 0;
}

