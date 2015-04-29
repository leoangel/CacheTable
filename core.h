/* 
 * File:   core.h
 * Author: leo
 *
 * Created on 2013年6月1日, 上午10:37
 */

#ifndef CORE_H
#define	CORE_H

#include<string>
#include<vector>
#include<string.h>
#include<iostream>
#include<sstream>
#include<time.h>
using namespace std;
string join(const vector<string> &str, const string &separator) ;
vector<string> split(const string &str, const string &separator) ;
bool file_exist(const char *file);
string intToString(int32_t i);
string doubleToString(double i);
double time_diff(struct timeval x, struct timeval y);

#endif	/* CORE_H */

