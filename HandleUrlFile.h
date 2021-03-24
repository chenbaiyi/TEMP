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


//���Ի����У��߼���Ϊ8�������̸߳���
const int threadNumber = 8;  
//ÿ���̴߳����ļ�����
const int hanleFileNumber = 16;
//��Ϊ�������ϣ�ļ�
const int hashFileSize = threadNumber * hanleFileNumber; 
//�Ѵ�С
const int heapLength = 100; 
//һ���Զ�ȡ�ļ��Ĵ�С
const int bufferFileLength = 20 * 1024 * 1024;
//ÿ����ϣ�ļ���С���ֵ
const long hashFileMaxLength = (1024 * 1024 * 1024) / threadNumber;
//һ����ˢ��300M
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
	//��ȡ���ļ���ͳ�Ʋ���url����
	using partUrlCount = std::unordered_map<std::string, int>;
	//һ�ζ�ȡ�ļ��Ĵ�С
	char urlBuffer[bufferFileLength];
	//�����ļ��ľ��
	std::array<HANDLE, hashFileSize> groupHindle;
	//����ļ�д���̻���
	std::array<partUrlCount, hashFileSize> groupFileBuffer;
	//��һ�ζ�ȡδ������url
	std::string lastUrl;
	//�ļ�����Ƿ��ʼ��ʧ��
	bool isInitFileError;
	//�����ļ����ļ����
	std::vector<int> excerptFile;
	//�����ļ����
	std::vector<HANDLE> excerptFileHandle;
	//�����ļ��ϴεĲ���������
	std::string lastExcerptUrl;
	//�����ļ�ͳ�ƵĹ�ϣ��
	partUrlCount excerptFileBuffer;
};