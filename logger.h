/* 
 * File:   log.h
 * Author: leo
 *
 * Created on 2013年6月1日, 下午12:06
 */

#ifndef LOG_H
#define	LOG_H
#include<string>
#include<log4cpp/Category.hh>
#include<log4cpp/FileAppender.hh>
#include<log4cpp/SimpleLayout.hh>
#include<log4cpp/SimpleConfigurator.hh>
#include<log4cpp/RollingFileAppender.hh>
#include<log4cpp/PatternLayout.hh>
#include<log4cpp/Appender.hh>
#include<log4cpp/NDC.hh>
#define OUTPUTLOG 1
using namespace std;

class logger {
private:
    logger(void);
    static logger *logInstance;
public:
    static logger *getInstance() ;
    ~logger();
    void error(const string &msg);
    void info(const string &msg);
    void warn(const string &msg);
};

#endif	/* LOG_H */

