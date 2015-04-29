/* 
 * File:   parse.h
 * Author: leo
 *
 * Created on 2013年5月31日, 下午11:04
 */

#ifndef PARSE_H
#define	PARSE_H
#include<map>
#include<string>
#include"tinyxml/tinystr.h"
#include"tinyxml/tinyxml.h"
#include"logger.h"
#include "kvserver.h"
#include"global.h"
#define configXmlDefaultPath "../conf/config.xml"
extern char configXml[200];

using namespace std;


struct kvServerInfo{
    int sid;
    char ip[15];
    int port;
};

class xmlParse {
public:
    static xmlParse * getInstance();
    ~xmlParse();
    string getDbConfig(const string &k);
    string getKvType();
    int getKvCount();
    kvServerInfo getKvDetailConfig(const int &num);
    int getSqlBatchNum();
    string getSql();
    string &getCacheTableProp(const string &k);
    string getWhereTable();
    string getWhereId();
    string getWhereCondition();
    string getLogConfig(const string &k);
    vector<string> getFields();
    map<string, string> cacheTableProp;
    map<int, kvServerInfo> kvConfig;
    map<string, string> dbConfig;
private:
    map<string,string> logConfig;
    xmlParse(const char *xmlpath);
    static  xmlParse * instance;
    string kvType;
    int sqlBatchNum;
    string sql;
    string wheretable;
    string whereid;
    string wherecondition;
    string fields;
    void init();
    TiXmlDocument *xmlDoc;
    TiXmlElement *xmlRoot;
    TiXmlNode *xmlItem;
    TiXmlNode *xmlChild;
    logger *log;
};

#endif	/* PARSE_H */

