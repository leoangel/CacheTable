#include <stdlib.h>

#include"core.h"

string join(const vector<string> &str, const string &separator) {
    if (str.empty()) return "";
    string tmp = "";

    if (str.size() == 1)
        return str[0];

    for (int i = 0; i < str.size(); i++) {
        if (i == (str.size() - 1)) {
            tmp += str[i];
        } else {
            tmp += str[i] + separator;
        }
    }
    return tmp;
}

vector<string> split(const string &str, const string &separator) {
    vector<string> arr;
    string tmp = str;
    long index = 0;
    while ((index = tmp.find(separator)) != str.npos) {
        arr.push_back(tmp.substr(0, index));
        tmp = tmp.substr(index + 1);
    }
    if (tmp == "") {
        return arr;
    }
    arr.push_back(tmp);
    return arr;
}

bool file_exist(const char *file) {
    if (strlen(file) == 0 || file == NULL) return false;
    if (!access(file, R_OK)) {
        return true;
    } else {
        return false;
    }
}

string intToString(int32_t i) {
    stringstream ss;
    ss << i;
    return ss.str();
}

string doubleToString(double i) {
    stringstream ss;
    ss << i;
    return ss.str();
}

double time_diff(struct timeval x, struct timeval y) {
    double x_ms, y_ms, diff;
    x_ms = (double) x.tv_sec * 1000000 + (double) x.tv_usec;
    y_ms = (double) y.tv_sec * 1000000 + (double) y.tv_usec;
    diff = (double) y_ms - (double) x_ms;
    return diff/1000/1000;
}



