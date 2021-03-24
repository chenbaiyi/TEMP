#include "stdafx.h"
#include"HandleUrlFile.h"


/************************************************
功能：构造函数
参数：
返回值：
作者：wangbo
**************************************************/
HandleUrlFile::HandleUrlFile()
{
	isInitFileError = initHashFile();
}

/************************************************
功能：初始化多个哈希文件句柄
参数：
返回值：bool-哈希文件句柄是否创建成功
作者：wangbo
**************************************************/
bool HandleUrlFile::initHashFile()
{
	//记录上一次读取中不完整的url
	lastUrl = "";
	//创建多个小文件
	std::string groupFileName;
	HANDLE hFile;
	for (int i = 0; i < hashFileSize; i++)
	{
		groupFileName = "E:\\tmp" + std::to_string(i) + ".txt";
		hFile = CreateFileA(groupFileName.c_str(), GENERIC_ALL, FILE_SHARE_WRITE|FILE_SHARE_READ, nullptr,CREATE_ALWAYS, NULL, nullptr);
		// 判断文件是否创建成功
		if (INVALID_HANDLE_VALUE == hFile)	
		{
			std::cout << "hash file open error";
			return true;
		}
		groupHindle.at(i) = hFile;
	}
	return false;
}

/************************************************
功能：处理100G文件
参数：url:url地址
返回值：url对应的哈希值
作者：wangbo
**************************************************/
void HandleUrlFile::handleFile()
{
	//读取超大文件
	readUrlFile();
	//读取文件结束后，再次刷盘
	flushHashFile();
	//统计超标文件
	bool ret = !checkExcerptHashFile();
	//关闭哈希文件句柄
	closeHashFileHandle();
	if (ret)
	{
		//没有超标的文件
		return;
	}
	//创建超标文件的句柄
	if (!createExcerptFileHandle())
	{
		return;
	}
	//处理超标文件
	handleExcerptFile();
}

/************************************************
功能：计算url的哈希值
参数：url-url地址
返回值：int-url对应的哈希值
作者：wangbo
**************************************************/
int HandleUrlFile::calculateHashValue( std::string &url)
{
	int res = 0;
	for (char ch : url)
	{
		res = (res * radix + unsigned(ch)) % hashFileSize;
	}
	return res;
}

/************************************************
功能：读取url文件，每次读取20M的URL处理
参数：
返回值：
作者：wangbo
**************************************************/
void HandleUrlFile::readUrlFile()
{
	HANDLE hFile = CreateFile(TEXT("E:\\da.txt"),GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (INVALID_HANDLE_VALUE == hFile)	
	{
		std::cout << "url file open error";
		return;
	}
	
	DWORD dwBytesRead = 0;
	while (true)
	{
		ReadFile(hFile, urlBuffer, bufferFileLength, &dwBytesRead, NULL);
		if (!dwBytesRead)
		{
			break;
		}
		std::string urlString = urlBuffer;
		analyseUrl(urlString, dwBytesRead);
		if (!checkHashBuffer())
		{
			continue;
		}
		flushHashFile();
	}
	CloseHandle(hFile);
}

/************************************************
功能：URL数据处理，即读取到的数据分离出URL,计算哈希值，
      根据哈希值分配到对应组中，并统计每组的URL以及URL出现次数
参数：url-待处理的数据，urlLength-处理数据的长度
返回值：
作者：wangbo
**************************************************/
void HandleUrlFile::analyseUrl(std::string& url, long urlLength)
{
	//拼接上一次未完整的url
	long pos = 0, lastPos = 0;
	if (!lastUrl.empty())
	{
		pos = url.find("\n");
		lastUrl += url.substr(lastPos, pos - lastPos);
		//1 代表\n
		lastPos = pos + 1;

		//写缓存信息
		groupFileBuffer.at(calculateHashValue(lastUrl))[lastUrl]++;
		lastUrl = "";
	}

	std::string tmpUrl;
	int hashValue;
	for (long i = 0; i < urlLength;)
	{
		pos = url.find("\n", lastPos);
		if (pos == -1)
		{
			break;
		}
		tmpUrl = url.substr(lastPos, pos - lastPos);
		//计算哈希值
		hashValue = calculateHashValue(tmpUrl);
		groupFileBuffer.at(hashValue)[tmpUrl]++;
		//继续下一个查找
		lastPos = pos + 1;
		i = lastPos;
	}

	if (urlLength <= lastPos)
	{
		return;
	}
	lastUrl = url.substr(lastPos, urlLength - lastPos);
}

/************************************************
功能：监测内存中的统计URL的内存，是否大于某值
参数：
返回值：bool-是否大于某值
作者：wangbo
**************************************************/
bool HandleUrlFile::checkHashBuffer()
{
	//待写buffer文件大小
	long groupBufferSize = 0;
	for (int i = 0; i < hashFileSize; i++)
	{
		auto iter = groupFileBuffer.at(i).begin();
		for (; iter != groupFileBuffer.at(i).end(); iter++)
		{
			groupBufferSize += (*iter).first.size();
		}
	}

	if (groupBufferSize < flushFileSize)
	{
		return false;
	}

	return true;
}

/************************************************
功能：将内存中统计的URL,刷入磁盘中
参数：
返回值：
作者：wangbo
**************************************************/
void HandleUrlFile::flushHashFile()
{
	//构造刷磁盘数据
	std::array<pairUrlCount, hashFileSize> fileBuffer;
	for (int i = 0; i < hashFileSize; i++)
	{
		fileBuffer.at(i).second = 0;

		auto iter = groupFileBuffer.at(i).begin();
		for (; iter != groupFileBuffer.at(i).end(); iter++)
		{
			//写入磁盘格式为www.baidu.com 8\n  表示:url后面一个空格，后面跟上url个数和\n
			fileBuffer.at(i).first += (*iter).first + " " + std::to_string((*iter).second) + "\n"; 
			//2表示一个空格，一个\n
			fileBuffer.at(i).second += (*iter).first.size() + std::to_string((*iter).second).size()+ 2;
		}
		groupFileBuffer.at(i).clear();
	}

	//刷磁盘
	DWORD writeSize = 0;
	for (int i = 0; i < hashFileSize; i++)
	{
		WriteFile(groupHindle.at(i), fileBuffer.at(i).first.c_str(), fileBuffer.at(i).second, &writeSize, NULL);
		FlushFileBuffers(groupHindle.at(i));
	}
}

/************************************************
功能：监测哈希文件是否大于1G/并发数大小
参数：
返回值：
作者：wangbo
**************************************************/
bool HandleUrlFile::checkExcerptHashFile()
{
	LARGE_INTEGER fileSize;
	for (int i = 0; i < hashFileSize; i++)
	{
		::GetFileSizeEx(groupHindle.at(i), &fileSize);
		if (fileSize.QuadPart < hashFileMaxLength)
		{
			continue;
		}
		excerptFile.push_back(i);
	}

	if (excerptFile.empty())
	{
		return false;
	}
	return true;
}

/************************************************
功能：关闭哈希文件的句柄
参数：
返回值：
作者：wangbo
**************************************************/
void HandleUrlFile::closeHashFileHandle()
{
	//文件初始化失败
	if (isInitFileError)
	{
		return;
	}
	//关闭文件句柄
	for (int i = 0; i < hashFileSize; i++)
	{
		CloseHandle(groupHindle.at(i));
	}
}

/************************************************
功能：创建超标文件句柄
参数：
返回值：
作者：wangbo
**************************************************/
bool HandleUrlFile::createExcerptFileHandle()
{
	int excerptFileSize = excerptFile.size();
	for (int i = 0; i < excerptFileSize; i++)
	{
		std::string fileName = "E:\\tmp" + std::to_string(excerptFile.at(i)) + ".txt";
		HANDLE hFile = CreateFileA(fileName.c_str(), GENERIC_READ,  FILE_SHARE_READ, nullptr,
			OPEN_EXISTING, NULL, nullptr);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			std::cout << "excerpt file error";
			return false;
		}
		excerptFileHandle.push_back(hFile);
	}
	return true;
}

/************************************************
功能：读取超标文件数据
参数：
返回值：
作者：wangbo
**************************************************/
void HandleUrlFile::handleExcerptFile()
{
	for (int i = 0; i < excerptFileHandle.size();i++)
	{
		DWORD dwBytesRead = 0;
		lastExcerptUrl = "";
		while (true)
		{
			ReadFile(excerptFileHandle.at(i), urlBuffer, bufferFileLength, &dwBytesRead, NULL);
			if (!dwBytesRead)
			{
				break;
			}
			std::string urlString = urlBuffer;
			analyseExcertFileUrl( urlString, dwBytesRead);
		}
		flushExcerptFile(excerptFile.at(i),i);
	}
}

/************************************************
功能：统计超标文件的URL
参数：
返回值：
作者：wangbo
**************************************************/
void HandleUrlFile::analyseExcertFileUrl(std::string& urlString, long urlLength)//处理超标文件的URL
{
	long pos = 0, lastPos = 0;
	int rowPos, urlValue;
	std::string tmpRowUrl, tmpUrl;
	if (!lastExcerptUrl.empty())
	{
		pos = urlString.find("\n");
		lastExcerptUrl += urlString.substr( 0, pos);
		rowPos = lastExcerptUrl.find(" ");
		//获取URL
		tmpUrl = lastExcerptUrl.substr(0, rowPos);
		//获取URL的个数
		urlValue = atoi(lastExcerptUrl.substr(rowPos, lastExcerptUrl.length() - rowPos).c_str());
		excerptFileBuffer[tmpUrl] += urlValue;
		//1 代表\n
		lastPos = pos + 1;
		lastExcerptUrl = "";
	}
	//
	for (long i = 0; i < urlLength;)
	{
		//分割出url字符
		pos = urlString.find("\n", lastPos);
		if (pos == -1)
		{
			break;
		}
		tmpRowUrl = urlString.substr(lastPos, pos - lastPos);
		rowPos = tmpRowUrl.find(" ");
		tmpUrl = tmpRowUrl.substr(0, rowPos);
		urlValue = atoi(tmpRowUrl.substr(rowPos, tmpRowUrl.length() - rowPos).c_str());
		//哈希表统计个数
		excerptFileBuffer[tmpUrl] += urlValue;
		//继续下一个查找
		lastPos = pos + 1; //1 代表\n
		i = lastPos;
	}
	//
	if (urlLength <= lastPos)
	{
		return;
	}
	lastExcerptUrl = urlString.substr(lastPos, urlLength - lastPos);
}

/************************************************
功能：将内存中统计超标文件的URL信息刷入磁盘
参数：
返回值：
作者：wangbo
**************************************************/
void HandleUrlFile::flushExcerptFile(int hashPos, int excerptPos)
{
	//构造刷磁盘数据
	pairUrlCount fileBuffer;
	
	auto iter = excerptFileBuffer.begin();
	for (; iter != excerptFileBuffer.end(); iter++)
	{
		//写入磁盘格式为www.baidu.com 8\n  表示:url后面一个空格，后面跟上url个数和\n
		fileBuffer.first += (*iter).first + " " + std::to_string((*iter).second) + "\n";
		//2表示一个空格，一个\n
		fileBuffer.second += (*iter).first.size() + std::to_string((*iter).second).size() + 2;
	}
	//关句柄，并创建句柄
	CloseHandle(excerptFileHandle.at(excerptPos));
	std::string fileName = "E:\\tmp" + std::to_string(hashPos) + ".txt";
	DeleteFileA(fileName.c_str());
	HANDLE hFile = CreateFileA(fileName.c_str(), GENERIC_ALL, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, CREATE_ALWAYS, NULL, nullptr);
	// 判断文件是否创建成功
	if (INVALID_HANDLE_VALUE == hFile)
	{
		std::cout << "rehash file open error";
		return ;
	}

	//刷磁盘
	DWORD writeSize = 0;
	WriteFile(hFile, fileBuffer.first.c_str(), fileBuffer.second, &writeSize, NULL);
	FlushFileBuffers(hFile);
	CloseHandle(hFile);
}
