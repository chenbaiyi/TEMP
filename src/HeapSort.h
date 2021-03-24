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

//С����
struct greaterCmp {
	bool operator()(pairUrlCount a, pairUrlCount b) {
		return a.second > b.second;
	}

};

//�󶥶�
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
	//����ͳ�ƵĴ󶥶�
	std::array<std::priority_queue<pairUrlCount, std::vector<pairUrlCount>, lessCmp>, hashFileSize> groupTopHeap;
	//�̼߳���
	std::atomic<int>  threadCount;
	//��ȡ�ļ������С10M
	char urlBuffer[bufferFileLength];
	//��һ�ζ�ȡδ������url
	std::array<std::string, threadNumber> lastUrl;
	//����top 100urlС����
	std::priority_queue<pairUrlCount, std::vector<pairUrlCount>, greaterCmp> topHeapUrl;
};