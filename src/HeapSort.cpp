
#include "stdafx.h"
#include"HeapSort.h"

/************************************************
功能：获取top 100结果
参数：
返回值：
作者：wangbo
**************************************************/
void HeapSort::resultTopUrl()
{
	threadHandleFile();
	waitThreadFinish();
	findTopUrl();
	outputTopUrl();
}

/************************************************
功能：开启8个线程处理哈希文件
参数：
返回值：
作者：wangbo
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
功能：等待线程完成
参数：
返回值：
作者：wangbo
**************************************************/
void HeapSort::waitThreadFinish()
{
	while (true)
	{
		//等待所有线程处理完
		if (threadCount != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		}
		break;
	}
	
}

/************************************************
功能：每个线程处理8个哈希文件
参数：threadId-线程标记ID，beginFilePos-开始遍历第几个文件，endFilePos-结束遍历第几个文件
返回值：
作者：wangbo
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
		//用哈希表统计每个哈希文件中url个数
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
		//哈希表统计个数后，在生成一个100大小的大顶堆
		constructBigHeap(i,urlHash);
	}
	threadCount--;
}

/************************************************
功能：每个哈希构建一个100大小的大顶堆
参数：pos-第几个哈希文件，urlHash-URL统计后的哈希表
返回值：
作者：wangbo
**************************************************/
void HeapSort::constructBigHeap(int pos,std::unordered_map<std::string, int>& urlHash)
{
	std::priority_queue<pairUrlCount, std::vector<pairUrlCount>, greaterCmp> smallHeapUrl;//小顶堆
	smallHeapUrl.push(std::make_pair("", 0));
	//
	for (auto iter = urlHash.begin(); iter!=urlHash.end(); iter++)
	{
		pairUrlCount minValue = smallHeapUrl.top();
		pairUrlCount countValue = *iter;

		if (minValue.second > countValue.second && smallHeapUrl.size() >= heapLength)
		{
			//堆大，并且个数等于100
			continue;
		}
		if (minValue.second < countValue.second && smallHeapUrl.size() >= heapLength)
		{
			//堆小，并且个数等于100
			smallHeapUrl.pop();
		}
		//将数据插入堆中
		smallHeapUrl.push(countValue);
	}
	//将小顶堆转化为大顶堆
	std::priority_queue<pairUrlCount, std::vector<pairUrlCount>, lessCmp> groupHeap;//大顶堆
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
功能：分析URL
参数：threadId-线程ID，url-读取的URL数据,
      urlLength-数据大小，urlHash-统计URL的哈希表
返回值：
作者：wangbo
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
		//获取URL
		tmpUrl = tmpRowUrl.substr(0, rowPos);
		//获取URL的个数
		urlValue = atoi(tmpRowUrl.substr(rowPos, tmpRowUrl.length() - rowPos).c_str());
		urlHash[tmpUrl] += urlValue;
		//1 代表\n
		lastPos = pos + 1;
		lastUrl.at(threadId) = "";
	}
	//
	for (long i = 0; i < urlLength;)
	{
		//分割出url字符
		pos = url.find("\n", lastPos);
		if (pos == -1)
		{
			break;
		}
		tmpRowUrl = url.substr(lastPos, pos - lastPos);
		rowPos = tmpRowUrl.find(" ");
		tmpUrl= tmpRowUrl.substr(0, rowPos);
		urlValue= atoi(tmpRowUrl.substr(rowPos, tmpRowUrl.length()-rowPos).c_str());
		//哈希表统计个数
		urlHash[tmpUrl] +=urlValue;
		//继续下一个查找
		lastPos = pos + 1; //1 代表\n
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
功能：将100个大顶堆依次与小顶堆比较，得到TOP 100的小顶堆
参数：
返回值：
作者：wangbo
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
				//如何分组的大顶堆最大值比最终小顶堆最小值小，则整个组都小，继续下一个分组
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
功能：输出top 100的URL以及次数
参数：
返回值：
作者：wangbo
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
