
#include "stdafx.h"
#include"HeapSort.h"

/************************************************
���ܣ���ȡtop 100���
������
����ֵ��
���ߣ�wangbo
**************************************************/
void HeapSort::resultTopUrl()
{
	threadHandleFile();
	waitThreadFinish();
	findTopUrl();
	outputTopUrl();
}

/************************************************
���ܣ�����8���̴߳����ϣ�ļ�
������
����ֵ��
���ߣ�wangbo
**************************************************/
void HeapSort::threadHandleFile()
{
	threadCount.store(threadNumber);
	for (int i = 0; i < threadNumber;i++)
	{
		std::thread groupThread(&HeapSort::handleGroupFile, this,i,i*hanleFileNumber,(i+1)*hanleFileNumber);
		groupThread.detach();
	}
}

/************************************************
���ܣ��ȴ��߳����
������
����ֵ��
���ߣ�wangbo
**************************************************/
void HeapSort::waitThreadFinish()
{
	while (true)
	{
		//�ȴ������̴߳�����
		if (threadCount != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		}
		break;
	}
	
}

/************************************************
���ܣ�ÿ���̴߳���8����ϣ�ļ�
������threadId-�̱߳��ID��beginFilePos-��ʼ�����ڼ����ļ���endFilePos-���������ڼ����ļ�
����ֵ��
���ߣ�wangbo
**************************************************/
void HeapSort::handleGroupFile(int threadId,int beginFilePos, int endFilePos)
{
	std::string urlString;
	for (int i = beginFilePos; i < endFilePos;i++)
	{
		std::string fileName = "E:\\tmp" + std::to_string(i) + ".txt";
		HANDLE hFile = CreateFileA(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
			OPEN_EXISTING, NULL, nullptr);
		if (INVALID_HANDLE_VALUE == hFile)	
		{
			std::cout << "read file error";
			return;
		}
		//�ù�ϣ��ͳ��ÿ����ϣ�ļ���url����
		std::unordered_map<std::string, int> urlHash;

		DWORD dwBytesRead = 0;
		while (true)
		{
			ReadFile(hFile, urlBuffer, bufferFileLength, &dwBytesRead, NULL);
			if (!dwBytesRead)
			{
				break;
			}
			urlString = urlBuffer;
			analyseUrl(threadId, urlString, dwBytesRead,urlHash);
		}
		CloseHandle(hFile);
		//��ϣ��ͳ�Ƹ�����������һ��100��С�Ĵ󶥶�
		constructBigHeap(i,urlHash);
	}
	threadCount--;
}

/************************************************
���ܣ�ÿ����ϣ����һ��100��С�Ĵ󶥶�
������pos-�ڼ�����ϣ�ļ���urlHash-URLͳ�ƺ�Ĺ�ϣ��
����ֵ��
���ߣ�wangbo
**************************************************/
void HeapSort::constructBigHeap(int pos,std::unordered_map<std::string, int>& urlHash)
{
	std::priority_queue<pairUrlCount, std::vector<pairUrlCount>, greaterCmp> smallHeapUrl;//С����
	smallHeapUrl.push(std::make_pair("", 0));
	//
	for (auto iter = urlHash.begin(); iter!=urlHash.end(); iter++)
	{
		pairUrlCount minValue = smallHeapUrl.top();
		pairUrlCount countValue = *iter;

		if (minValue.second > countValue.second && smallHeapUrl.size() >= heapLength)
		{
			//�Ѵ󣬲��Ҹ�������100
			continue;
		}
		if (minValue.second < countValue.second && smallHeapUrl.size() >= heapLength)
		{
			//��С�����Ҹ�������100
			smallHeapUrl.pop();
		}
		//�����ݲ������
		smallHeapUrl.push(countValue);
	}
	//��С����ת��Ϊ�󶥶�
	std::priority_queue<pairUrlCount, std::vector<pairUrlCount>, lessCmp> groupHeap;//�󶥶�
	int heapSize = smallHeapUrl.size();
	for (int i = 0; i < heapSize;i++)
	{
		groupHeap.push(smallHeapUrl.top());
		smallHeapUrl.pop();
	}
	//
	groupTopHeap.at(pos) = groupHeap;
}


/************************************************
���ܣ�����URL
������threadId-�߳�ID��url-��ȡ��URL����,
      urlLength-���ݴ�С��urlHash-ͳ��URL�Ĺ�ϣ��
����ֵ��
���ߣ�wangbo
**************************************************/
void HeapSort::analyseUrl(int threadId, std::string& url, int urlLength, std::unordered_map<std::string, int>& urlHash)
{
	long pos = 0, lastPos = 0;
	int rowPos, urlValue;
	std::string tmpRowUrl, tmpUrl;
	if (!lastUrl.at(threadId).empty())
	{
		pos = url.find("\n");
		lastUrl.at(threadId) += url.substr( 0, pos);
		tmpRowUrl = lastUrl.at(threadId);
		rowPos = tmpRowUrl.find(" ");
		//��ȡURL
		tmpUrl = tmpRowUrl.substr(0, rowPos);
		//��ȡURL�ĸ���
		urlValue = atoi(tmpRowUrl.substr(rowPos, tmpRowUrl.length() - rowPos).c_str());
		urlHash[tmpUrl] += urlValue;
		//1 ����\n
		lastPos = pos + 1;
		lastUrl.at(threadId) = "";
	}
	//
	for (long i = 0; i < urlLength;)
	{
		//�ָ��url�ַ�
		pos = url.find("\n", lastPos);
		if (pos == -1)
		{
			break;
		}
		tmpRowUrl = url.substr(lastPos, pos - lastPos);
		rowPos = tmpRowUrl.find(" ");
		tmpUrl= tmpRowUrl.substr(0, rowPos);
		urlValue= atoi(tmpRowUrl.substr(rowPos, tmpRowUrl.length()-rowPos).c_str());
		//��ϣ��ͳ�Ƹ���
		urlHash[tmpUrl] +=urlValue;
		//������һ������
		lastPos = pos + 1; //1 ����\n
		i = lastPos;
	}
	//
	if (urlLength <= lastPos)
	{
		return;
	}
	lastUrl.at(threadId) = url.substr(lastPos, urlLength - lastPos);
}

/************************************************
���ܣ���100���󶥶�������С���ѱȽϣ��õ�TOP 100��С����
������
����ֵ��
���ߣ�wangbo
**************************************************/
void HeapSort::findTopUrl()
{
	topHeapUrl.push(std::make_pair("", 0));
	//
	for (int i = 0; i < hashFileSize; i++)
	{
		pairUrlCount minValue = topHeapUrl.top();
		int groupUrlSiz = groupTopHeap.at(i).size();
		for (int j = 0; j < groupUrlSiz; j++)
		{
			pairUrlCount groupMaxValue = groupTopHeap.at(i).top();
			if (minValue.second > groupMaxValue.second)
			{
				//��η���Ĵ󶥶����ֵ������С������СֵС���������鶼С��������һ������
				break;
			}

			groupTopHeap.at(i).pop();
			if (topHeapUrl.size() >= heapLength)
			{
				topHeapUrl.pop();
			}
			topHeapUrl.push(groupMaxValue);

		}
	}
	
}

/************************************************
���ܣ����top 100��URL�Լ�����
������
����ֵ��
���ߣ�wangbo
**************************************************/
void HeapSort::outputTopUrl()
{
	int topLength = topHeapUrl.size();
	for (int i = 0; i < topLength; i++)
	{
		std::cout << topHeapUrl.top().first << std::endl << topHeapUrl.top().second << std::endl;
		topHeapUrl.pop();
	}
}
