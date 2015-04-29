/* 
 * File:   mysql.h
 * Author: leo
 *
 * Created on 2013年5月31日, 下午11:05
 */

#ifndef DATA_H
#define	DATA_H

#include<string>
#include<vector>
#include"logger.h"
#include"parse.h"
#include<mysql++/mysql++.h>
#include<semaphore.h>
#define KEYLENGTH 33
#define VALLENGTH 20000
using namespace std;


struct whereSection{
    uint32_t start;
    uint32_t end;
};

struct dataSet{
    char key[KEYLENGTH];
    char *val;
};


enum queryModeSet{
    limit=0x01,
    where=0x02,
};

class data {
private:
    pthread_mutex_t pmt;
    mysqlpp::Connection dbcon;
    logger *log;
    xmlParse *xp;
    uint32_t batchNum;
    uint32_t sleepTime;
    uint32_t intervalTime;
    string sql;
    string keyfield;
    map<string,int> keyfieldMap;
    string keyPrefix;
    vector<string> fieldName;
    void preGrabFieldInfo();
    sem_t dbsem,cachesem;
    uint32_t tableMaxId;
    uint32_t tableMinId;
    uint32_t batchIndex;
    vector<whereSection>  section;
    void initWhereSection();
    string getWhereSection();
    //cache耗时记录变量
    struct timeval cacheBefore,cacheAfter;
    
    //初始化内存块
    void initMemPool();
    
    //查询模式定义
    queryModeSet queryMode;

    //初始化db连接
    
    void initDB();
    
public:
    data();
    ~data();
    static uint32_t counter;
    void loadData(); 
    void waitData(int sid);
    void cacheDone(int sid);
    int getThreadNum();
    dataSet *dataset;
    dataSet *begindataset;
    static uint32_t loopCount;
};

#endif	/* DATA_H */

