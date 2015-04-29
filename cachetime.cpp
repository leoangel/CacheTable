/* 
 * File:   main.cpp
 * Author: leo
 *
 * Created on 2013年5月31日, 下午10:59
 */

#include"cachetable.h"
#include"logger.h"
#include "parse.h"
#include "kv.h"
#include<unistd.h>
#include <string.h>

void getoption(int argc, char ** argv) {
    int ret;
    while ((ret=getopt(argc, argv, "c:d")) != -1) {
        switch (ret) {
            case 'c':
                memcpy(xmlFile,optarg,sizeof(xmlFile));
                break;
            case 'd':
             daemonFlag = true;
                break;
            default:
                perror("参数错误!");
                exit(255);
        }
    }
}





int main(int argc, char** argv) {
    bzero(xmlFile,sizeof(xmlFile));
    getoption(argc,argv);
   
    kv *k = kv::getInstance("memcached");
    k->addServer(1,"127.0.0.1","3305");
    
    
    
    
    

    return 0;
}

