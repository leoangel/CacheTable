#include"data.h"
#include "core.h"
#include<algorithm>
#include"cJSON.h"
#include"md5.h"
#include"global.h"
#include"parse.h"
#include<math.h>

uint32_t data::counter = 0;
uint32_t data::loopCount = 0;

data::data() {
    log = logger::getInstance();
    xp = xmlParse::getInstance();
    try {

        initDB();
        sql = xp->getSql();
        batchNum = xp->getSqlBatchNum();
        sleepTime = atoi(xp->getCacheTableProp("sleep").c_str());
        intervalTime = atoi(xp->getCacheTableProp("interval").c_str());
        keyPrefix = xp->getCacheTableProp("keyprefix");

        //查询模式判断
        if ((!xp->getWhereTable().empty())&&(!xp->getWhereId().empty())&&(!xp->getWhereCondition().empty())) {
            queryMode = where;
            log->info("当前查询模式: WHERE区间限定模式");
        } else {
            queryMode = limit;
            log->info("当前查询模式: LIMIT区间限定模式");
        }

        preGrabFieldInfo();
        counter = getThreadNum();
        pthread_mutex_init(&pmt, NULL);
        sem_init(&dbsem, 0, 1);
        sem_init(&cachesem, 0, 0);
        batchIndex = 0;
        initWhereSection();

        //开辟内存
        dataset = (dataSet *) malloc(sizeof (dataSet) * batchNum);
        begindataset = dataset;
        initMemPool();

    } catch (const mysqlpp::Exception& er) {
        log->error(string(er.what()));
        exit(255);
    }
}

void data::initDB() {
    mysqlpp::ConnectTimeoutOption(5);
    dbcon.set_option(new mysqlpp::SetCharsetNameOption("utf8"));
    dbcon.set_option(new mysqlpp::ReconnectOption(true));
    try {
        log->info("初始化db连接");
        if (!dbcon.connect(xp->getDbConfig("database").c_str(), xp->getDbConfig("host").c_str(), xp->getDbConfig("username").c_str(), xp->getDbConfig("password").c_str(), atoi(xp->getDbConfig("port").c_str()))) {
            log->error("连接MySQL数据库失败.");
            exit(255);
        } else {
            log->info("连接MySQL数据库成功.");
        }
    } catch (const mysqlpp::Exception& er) {
        log->error(string(er.what()));
        exit(255);
    }
}

void data::preGrabFieldInfo() {
    vector<string> res = xp->getFields();
    if (res.size() == 0) {
        cerr << "获取数据库列表名失败" << endl;
        exit(255);
    } else {
        vector<string> _tmp = split(xp->getCacheTableProp("keyfield"), "|");
        int _tmpCount = _tmp.size();
        int _tmpStartCount = 0;
        for (int i = 0; i < res.size(); i++) {
            fieldName.push_back(res[i]);
            if (find(_tmp.begin(), _tmp.end(), res[i]) != _tmp.end()) {
                _tmpStartCount++;
                keyfieldMap.insert(make_pair(res[i], i));
            }
        }
        if (_tmpCount != _tmpStartCount) {
            cerr << "请检查key值字段列名是否在SQL中存在" << endl;
            exit(255);
        }
    }
}

void data::initMemPool() {
    dataSet* item = begindataset;
    for (int i = 0; i < loopCount; i++) {
        if (item->val != NULL) {
            free(item->val);
        }
        item++;
    }
    dataset = begindataset;
    loopCount = 0;
    memset(dataset, 0, sizeof (dataSet) * batchNum);
}

void data::loadData() {
    struct timeval beforeTime, afterTime;
    MD5 md5;
    while (1) {
        gettimeofday(&beforeTime, NULL);
        try {
            mysqlpp::Query query = dbcon.query(sql + " " + getWhereSection());
            mysqlpp::StoreQueryResult res = query.store();
            gettimeofday(&afterTime, NULL);
            log->info("---- MySQL数据检索耗时: " + doubleToString(time_diff(beforeTime, afterTime)) + " s. 获取数据量: " + intToString(res.num_rows()) + " ----");
            sem_wait(&dbsem);
            if (res.num_rows() == 0) {
                if (queryMode == where) {
                    usleep(intervalTime);
                } else {
                    sleep(sleepTime);
                }
                sem_post(&dbsem);
                query.clear();
                res.clear();
                if (queryMode == limit) {
                    batchIndex = 0;
                    section.clear();
                }
                continue;
            } else {
                initMemPool();
                for (int i = 0; i < res.num_rows(); i++) {
                    cJSON *objJson = cJSON_CreateObject();
                    string cacheKey = "";
                    for (int j = 0; j < fieldName.size(); j++) {
                        cJSON_AddStringToObject(objJson, fieldName[j].c_str(), res[i][j].c_str());
                        if (keyfieldMap.find(fieldName[j]) != keyfieldMap.end()) {
                            cacheKey = cacheKey + "_" + string(res[i][j]);
                        }
                    }
                    md5.update(keyPrefix + cacheKey);
                    memcpy(dataset->key, md5.toString().substr(0, 32).c_str(), KEYLENGTH - 1);
                    dataset->val = cJSON_Print(objJson);
                    loopCount++;
                    dataset++;
                    cJSON_Delete(objJson);
                    md5.reset();
                }
                res.clear();
                query.clear();
                log->info("数据读取完毕,缓存同步线程开始.");
                gettimeofday(&cacheBefore, NULL);
                for (int i = 0; i < getThreadNum(); i++) {
                    sem_post(&cachesem);
                }
            }
            usleep(intervalTime);
        } catch (mysqlpp::Exception dbex) {
            log->error("MySQL Error: " + string(dbex.what()));
            sleep(sleepTime);
        }
    }
}

void data::cacheDone(int sid) {
    log->info("缓存服务器#" + intToString(sid) + "接收数据完毕.");
    pthread_mutex_lock(&pmt);
    counter--;
    if (counter == 0) {
        gettimeofday(&cacheAfter, NULL);
        log->info("---- 缓存同步完毕,耗时: " + doubleToString(time_diff(cacheBefore, cacheAfter)) + " s. ----");
        sem_post(&dbsem);
        counter = getThreadNum();
    }
    pthread_mutex_unlock(&pmt);
}

void data::waitData(int sid) {
    sem_wait(&cachesem);
    pthread_mutex_lock(&pmt);
    log->info("缓存服务器#" + intToString(sid) + "开始接收数据.");
    pthread_mutex_unlock(&pmt);
}

int data::getThreadNum() {
    return xp->getKvCount();
}

data::~data() {
    sem_destroy(&dbsem);
    sem_destroy(&cachesem);
}

void data::initWhereSection() {
    if (queryMode == where) {
        if (xp->getWhereCondition().empty()) {
            cerr << "设定查询区间必须配置查询方式:  WHERE或者AND" << endl;
            exit(255);
        }
        try {
            mysqlpp::Query query = dbcon.query("SELECT MAX(" + xp->getWhereId() + ") AS maxid,MIN(" + xp->getWhereId() + ") AS minid FROM " + xp->getWhereTable());
            mysqlpp::StoreQueryResult res = query.store();
            if (res.num_rows() == 1) {
                section.clear();
                batchIndex = 0;
                tableMaxId = atol(res.begin()[0][0].c_str());
                tableMinId = atol(res.begin()[0][1].c_str());
                double tmpNum = (double) tableMaxId - (double) tableMinId;
                tmpNum = ceil(tmpNum / (double) batchNum);
                int startNum = tableMinId;
                log->info("---- 计算查询区间开始 ----");
                for (int i = 0; i < tmpNum; i++) {
                    whereSection ws;
                    ws.start = startNum;
                    ws.end = startNum + batchNum;
                    section.push_back(ws);
                    log->info("生成SQL查询区间#" + intToString(i) + "：" + intToString(ws.start) + " - " + intToString(ws.end));
                    startNum += batchNum;
                }
                log->info("---- 计算查询区间结束 ----");
            } else {
                cerr << "获取表字段最大值区间失败." << endl;
                exit(255);
            }
        } catch (mysqlpp::Exception dbex) {
            log->error("获取表字段最大值区间失败.MySQL Error: " + string(dbex.what()));
            exit(255);
        }
    } else {
        batchIndex = 0;
        section.clear();
    }
    return;
}

string data::getWhereSection() {
    if (section.empty()) {
        uint32_t idx = batchIndex;
        batchIndex++;
        log->info("当前检索行:  LIMIT " + intToString(idx * batchNum) + "," + intToString(batchNum));
        return "  LIMIT " + intToString(idx * batchNum) + "," + intToString(batchNum);
    } else {
        if (batchIndex == section.size()) {
            sleep(sleepTime);
            initWhereSection();
            batchIndex = 1;
            log->info("当前查询区间：#0 " + intToString(section[0].start) + " - " + intToString(section[0].end));
            return " " + xp->getWhereCondition() + " " + xp->getWhereId() + " >= " + intToString(section[0].start) + "  AND " + xp->getWhereId() + " <= " + intToString(section[0].end) + " LIMIT 0," + intToString(batchNum);
        } else {
            uint32_t idx = batchIndex;
            log->info("当前查询区间：#" + intToString(idx) + " " + intToString(section[idx].start) + " - " + intToString(section[idx].end));
            batchIndex++;
            return " " + xp->getWhereCondition() + " " + xp->getWhereId() + " >= " + intToString(section[idx].start) + "  AND " + xp->getWhereId() + " <= " + intToString(section[idx].end) + " LIMIT 0," + intToString(batchNum);
        }
    }
}
