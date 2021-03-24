#pragma once
#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<string>
#include<unordered_map>
#include<queue>
#include<utility>
#include<array>
#include"windows.h"


//测试环境中，逻辑核为8，设置线程个数
const int threadNumber = 8;  
//每个线程处理文件个数
const int hanleFileNumber = 16;
//分为多少组哈希文件
const int hashFileSize = threadNumber * hanleFileNumber; 
//堆大小
const int heapLength = 100; 
//一次性读取文件的大小
const int bufferFileLength = 20 * 1024 * 1024;
//每个哈希文件大小最大值
const long hashFileMaxLength = (1024 * 1024 * 1024) / threadNumber;
//一次性刷盘300M
const long flushFileSize = 300 * 1024 * 1024;
const int radix = 33;
using pairUrlCount = std::pair<std::string, int>;


//
class HandleUrlFile
{
public:
	HandleUrlFile();
	~HandleUrlFile()=default;
	void handleFile();
private:
	bool initHashFile();
	void readUrlFile();
	void analyseUrl(std::string& url,long urlLength);
	int calculateHashValue( std::string &url);
	bool checkHashBuffer();
	void flushHashFile();
	bool checkExcerptHashFile();
	void closeHashFileHandle();
	bool createExcerptFileHandle();
	void handleExcerptFile();
	void analyseExcertFileUrl(std::string&,long );
	void flushExcerptFile(int hashPos,int excerptPos);
private:
	//读取大文件，统计部分url个数
	using partUrlCount = std::unordered_map<std::string, int>;
	//一次读取文件的大小
	char urlBuffer[bufferFileLength];
	//分组文件的句柄
	std::array<HANDLE, hashFileSize> groupHindle;
	//多个文件写磁盘缓存
	std::array<partUrlCount, hashFileSize> groupFileBuffer;
	//上一次读取未完整的url
	std::string lastUrl;
	//文件句柄是否初始化失败
	bool isInitFileError;
	//超标文件的文件编号
	std::vector<int> excerptFile;
	//超标文件句柄
	std::vector<HANDLE> excerptFileHandle;
	//超标文件上次的不完整数据
	std::string lastExcerptUrl;
	//超标文件统计的哈希表
	partUrlCount excerptFileBuffer;
};