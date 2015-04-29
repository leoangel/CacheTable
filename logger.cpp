#include"logger.h"
#include "parse.h"

logger* logger::logInstance = NULL;

logger::~logger() {
    log4cpp::Category::shutdown();
}

logger *logger::getInstance() {
    {
        if (logInstance == NULL)
            logInstance = new logger();
        return logInstance;
    };
}

logger::logger(void) {

    log4cpp::RollingFileAppender * appender = new log4cpp::RollingFileAppender("run", std::string(xmlParse::getInstance()->getLogConfig("logpath") + xmlParse::getInstance()->getLogConfig("logname")).c_str(), atol(xmlParse::getInstance()->getLogConfig("logfilesize").c_str()), atoi(xmlParse::getInstance()->getLogConfig("lognum").c_str())); //第一个参数是appender的名字，第二个是log文件的名字
    log4cpp::PatternLayout *playout = new log4cpp::PatternLayout();
    playout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S} [%p]: [%c] %m%n");

    log4cpp::Category &category = log4cpp::Category::getInstance("run");
    appender->setLayout(playout);
    category.setAppender(appender);
    category.setPriority(log4cpp::Priority::INFO);
}

void logger::info(const string& msg) {
    log4cpp::Category &category = log4cpp::Category::getInstance("run");
    if (OUTPUTLOG) {
        cout << msg << endl;
    }
    category.info(msg);
}

void logger::warn(const string& msg) {
    log4cpp::Category &category = log4cpp::Category::getInstance("run");
    if (OUTPUTLOG) {
        cout << msg << endl;
    }
    category.warn(msg);
}

void logger::error(const string& msg) {
    log4cpp::Category &category = log4cpp::Category::getInstance("run");
    if (OUTPUTLOG) {
        cout << msg << endl;
    }
    category.error(msg);
}