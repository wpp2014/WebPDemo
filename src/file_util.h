#ifndef FILE_UTIL_H_
#define FILE_UTIL_H_

#include <stdio.h>

#include <string>

// 打开文件
FILE* OpenFile(const std::string& filename, const char* mode);

// 关闭文件
bool CloseFile(FILE* file);

// 读取文件内容
bool ReadFileToString(const std::string& path, std::string* contents);

// 读取指定字节的文件内容
bool ReadFileToStringWithMaxSize(const std::string& path,
                                 std::string* contents,
                                 size_t max_size);

#endif  // FILE_UTIL_H_
