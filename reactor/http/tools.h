#pragma once

#include <string>
#include <set>

void loadFiles(std::string path, std::set<std::string>&files);

void error(const char *msg);

void readString(char* tmp, char endFlag, int limit, char* buf, int& idx, int bufEnd);

void skipSpace(char* buf, int& idx, int bufEnd);

/*
 * return RETURN_TYPE deduced by FILE_PATH
 * e.g. "/index.html"  ->  return "text/html"
 *      "/a.jpg"       ->  return "image/jpg"
 */
std::string getType(std::string s);
