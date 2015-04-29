#include "parse.h"
#include"core.h"


xmlParse *xmlParse::instance=NULL;

xmlParse * xmlParse::getInstance() { 
    if (instance==NULL) {
        instance = new xmlParse(configXml);
    }
    return instance;
}

xmlParse::xmlParse(const char* xmlpath) {
 
    if (!file_exist(xmlpath)) {
        cerr<<"配置文件未找到."<<endl;
        exit(255);
    }
    xmlDoc = new TiXmlDocument(xmlpath);
    if (!xmlDoc->LoadFile()) {
        cerr<<"配置文件解析失败."<<endl;
        exit(255);
    }
    init();
}

xmlParse::~xmlParse() {
    delete log;
}

void xmlParse::init() {

    xmlRoot = xmlDoc->RootElement();
    xmlItem = xmlRoot->FirstChild("item");
    if (!xmlItem) {
        cerr<<"未找到item节点配置."<<endl;
        exit(255);
    }
    //读取db配置
    dbConfig["host"] = std::string(xmlItem->FirstChild("db")->ToElement()->Attribute("host"));
    dbConfig["port"] = std::string(xmlItem->FirstChild("db")->ToElement()->Attribute("port"));
    dbConfig["username"] = std::string(xmlItem->FirstChild("db")->ToElement()->Attribute("username"));
    dbConfig["password"] = std::string(xmlItem->FirstChild("db")->ToElement()->Attribute("password"));
    dbConfig["database"] = std::string(xmlItem->FirstChild("db")->ToElement()->Attribute("database"));

    //读取kv配置
    kvType = std::string(xmlItem->FirstChild("kv")->ToElement()->Attribute("type"));
    xmlChild = xmlItem->FirstChild("kv")->FirstChild("server");
    while (xmlChild) {
        kvServerInfo  ksi;
        memset(&ksi,'\0',sizeof(ksi));
        ksi.sid = atoi(xmlChild->ToElement()->Attribute("id"));
        memcpy(ksi.ip,xmlChild->ToElement()->Attribute("ip"),strlen(xmlChild->ToElement()->Attribute("ip")));
        ksi.port = atoi(xmlChild->ToElement()->Attribute("port"));
        kvConfig.insert(make_pair(atoi(xmlChild->ToElement()->Attribute("id")),ksi));
        xmlChild = xmlChild->NextSibling();
    }
    
    //读取sql配置
    sqlBatchNum = atoi(xmlItem->FirstChild("sql")->ToElement()->Attribute("batchnum"));
    sql = std::string(xmlItem->FirstChild("sql")->FirstChild()->Value());
    wheretable=std::string(xmlItem->FirstChild("sql")->ToElement()->Attribute("wheretable"));
    whereid=std::string(xmlItem->FirstChild("sql")->ToElement()->Attribute("whereid"));
    wherecondition = std::string(xmlItem->FirstChild("sql")->ToElement()->Attribute("wherecondition"));
    
    //读取缓存策略配置
    cacheTableProp["keyfield"] = std::string(xmlItem->FirstChild("cache")->ToElement()->Attribute("keyfield"));
    cacheTableProp["keyprefix"] = std::string(xmlItem->FirstChild("cache")->ToElement()->Attribute("keyprefix"));
    cacheTableProp["expires"] = std::string(xmlItem->FirstChild("cache")->ToElement()->Attribute("expires"));
    cacheTableProp["sleep"] = std::string(xmlItem->FirstChild("cache")->ToElement()->Attribute("sleep"));
    cacheTableProp["interval"] = std::string(xmlItem->FirstChild("cache")->ToElement()->Attribute("interval"));

    //日志输出配置
    logConfig["logpath"] = std::string(xmlItem->FirstChild("log")->ToElement()->Attribute("logpath"));
    logConfig["logname"] = std::string(xmlItem->FirstChild("log")->ToElement()->Attribute("logname"));
    logConfig["logfilesize"] = std::string(xmlItem->FirstChild("log")->ToElement()->Attribute("logfilesize"));
    logConfig["lognum"] = std::string(xmlItem->FirstChild("log")->ToElement()->Attribute("lognum"));

    //数据库列名配置
    fields = std::string(xmlItem->FirstChild("field")->ToElement()->Attribute("name"));
    
}

string xmlParse::getDbConfig(const string &k) {
    return dbConfig[k];
}

string xmlParse::getKvType() {
    return kvType;
}

int xmlParse::getKvCount() {
    return kvConfig.size();
}

kvServerInfo  xmlParse::getKvDetailConfig(const int &num) {
    return kvConfig[num];
}

int xmlParse::getSqlBatchNum() {
    return sqlBatchNum;
}

string xmlParse::getSql() {
    return sql;
}

string &xmlParse::getCacheTableProp(const string& k) {
    return cacheTableProp[k];
}

string xmlParse::getWhereTable(){
    return wheretable;
}

string xmlParse::getWhereId(){
    return whereid;
}

string xmlParse::getWhereCondition(){
    return wherecondition;
}


string xmlParse::getLogConfig(const string& k){
    if(logConfig.find(k)!=logConfig.end()){
        return logConfig[k];
    }else{
        return "";
    }
}

vector<string> xmlParse::getFields(){
    return split(fields,",");
}
