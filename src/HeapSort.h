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
#include<thread>
#include<atomic>
#include"windows.h"
#include"HandleUrlFile.h"

//小顶堆
struct greaterCmp {
	bool operator()(pairUrlCount a, pairUrlCount b) {
		return a.second > b.second;
	}

};

//大顶堆
struct lessCmp {
	bool operator()(pairUrlCount a, pairUrlCount b) {
		return a.second < b.second;
	}

};

class HeapSort
{
public:
	HeapSort()=default;
	~HeapSort()=default;
	void resultTopUrl();
private:
	void threadHandleFile();
	void waitThreadFinish();
	void handleGroupFile(int threadId,int beginFilePos, int endFilePos);
	void analyseUrl(int threadId,std::string& url, int urlLength,std::unordered_map<std::string, int>& urlHash);
	void constructBigHeap(int pos,std::unordered_map<std::string, int>& urlHash);
	void findTopUrl();
	void outputTopUrl();
private:
	//分组统计的大顶堆
	std::array<std::priority_queue<pairUrlCount, std::vector<pairUrlCount>, lessCmp>, hashFileSize> groupTopHeap;
	//线程计数
	std::atomic<int>  threadCount;
	//读取文件缓存大小10M
	char urlBuffer[bufferFileLength];
	//上一次读取未完整的url
	std::array<std::string, threadNumber> lastUrl;
	//构建top 100url小顶堆
	std::priority_queue<pairUrlCount, std::vector<pairUrlCount>, greaterCmp> topHeapUrl;
};