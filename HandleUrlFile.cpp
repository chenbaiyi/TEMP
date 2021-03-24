#include "stdafx.h"
#include"HandleUrlFile.h"


/************************************************
���ܣ����캯��
������
����ֵ��
���ߣ�wangbo
**************************************************/
HandleUrlFile::HandleUrlFile()
{
	isInitFileError = initHashFile();
}

/************************************************
���ܣ���ʼ�������ϣ�ļ����
������
����ֵ��bool-��ϣ�ļ�����Ƿ񴴽��ɹ�
���ߣ�wangbo
**************************************************/
bool HandleUrlFile::initHashFile()
{
	//��¼��һ�ζ�ȡ�в�������url
	lastUrl = "";
	//�������С�ļ�
	std::string groupFileName;
	HANDLE hFile;
	for (int i = 0; i < hashFileSize; i++)
	{
		groupFileName = "E:\\tmp" + std::to_string(i) + ".txt";
		hFile = CreateFileA(groupFileName.c_str(), GENERIC_ALL, FILE_SHARE_WRITE|FILE_SHARE_READ, nullptr,CREATE_ALWAYS, NULL, nullptr);
		// �ж��ļ��Ƿ񴴽��ɹ�
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
���ܣ�����100G�ļ�
������url:url��ַ
����ֵ��url��Ӧ�Ĺ�ϣֵ
���ߣ�wangbo
**************************************************/
void HandleUrlFile::handleFile()
{
	//��ȡ�����ļ�
	readUrlFile();
	//��ȡ�ļ��������ٴ�ˢ��
	flushHashFile();
	//ͳ�Ƴ����ļ�
	bool ret = !checkExcerptHashFile();
	//�رչ�ϣ�ļ����
	closeHashFileHandle();
	if (ret)
	{
		//û�г�����ļ�
		return;
	}
	//���������ļ��ľ��
	if (!createExcerptFileHandle())
	{
		return;
	}
	//�������ļ�
	handleExcerptFile();
}

/************************************************
���ܣ�����url�Ĺ�ϣֵ
������url-url��ַ
����ֵ��int-url��Ӧ�Ĺ�ϣֵ
���ߣ�wangbo
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
���ܣ���ȡurl�ļ���ÿ�ζ�ȡ20M��URL����
������
����ֵ��
���ߣ�wangbo
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
���ܣ�URL���ݴ�������ȡ�������ݷ����URL,�����ϣֵ��
      ���ݹ�ϣֵ���䵽��Ӧ���У���ͳ��ÿ���URL�Լ�URL���ִ���
������url-����������ݣ�urlLength-�������ݵĳ���
����ֵ��
���ߣ�wangbo
**************************************************/
void HandleUrlFile::analyseUrl(std::string& url, long urlLength)
{
	//ƴ����һ��δ������url
	long pos = 0, lastPos = 0;
	if (!lastUrl.empty())
	{
		pos = url.find("\n");
		lastUrl += url.substr(lastPos, pos - lastPos);
		//1 ����\n
		lastPos = pos + 1;

		//д������Ϣ
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
		//�����ϣֵ
		hashValue = calculateHashValue(tmpUrl);
		groupFileBuffer.at(hashValue)[tmpUrl]++;
		//������һ������
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
���ܣ�����ڴ��е�ͳ��URL���ڴ棬�Ƿ����ĳֵ
������
����ֵ��bool-�Ƿ����ĳֵ
���ߣ�wangbo
**************************************************/
bool HandleUrlFile::checkHashBuffer()
{
	//��дbuffer�ļ���С
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
���ܣ����ڴ���ͳ�Ƶ�URL,ˢ�������
������
����ֵ��
���ߣ�wangbo
**************************************************/
void HandleUrlFile::flushHashFile()
{
	//����ˢ��������
	std::array<pairUrlCount, hashFileSize> fileBuffer;
	for (int i = 0; i < hashFileSize; i++)
	{
		fileBuffer.at(i).second = 0;

		auto iter = groupFileBuffer.at(i).begin();
		for (; iter != groupFileBuffer.at(i).end(); iter++)
		{
			//д����̸�ʽΪwww.baidu.com 8\n  ��ʾ:url����һ���ո񣬺������url������\n
			fileBuffer.at(i).first += (*iter).first + " " + std::to_string((*iter).second) + "\n"; 
			//2��ʾһ���ո�һ��\n
			fileBuffer.at(i).second += (*iter).first.size() + std::to_string((*iter).second).size()+ 2;
		}
		groupFileBuffer.at(i).clear();
	}

	//ˢ����
	DWORD writeSize = 0;
	for (int i = 0; i < hashFileSize; i++)
	{
		WriteFile(groupHindle.at(i), fileBuffer.at(i).first.c_str(), fileBuffer.at(i).second, &writeSize, NULL);
		FlushFileBuffers(groupHindle.at(i));
	}
}

/************************************************
���ܣ�����ϣ�ļ��Ƿ����1G/��������С
������
����ֵ��
���ߣ�wangbo
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
���ܣ��رչ�ϣ�ļ��ľ��
������
����ֵ��
���ߣ�wangbo
**************************************************/
void HandleUrlFile::closeHashFileHandle()
{
	//�ļ���ʼ��ʧ��
	if (isInitFileError)
	{
		return;
	}
	//�ر��ļ����
	for (int i = 0; i < hashFileSize; i++)
	{
		CloseHandle(groupHindle.at(i));
	}
}

/************************************************
���ܣ����������ļ����
������
����ֵ��
���ߣ�wangbo
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
���ܣ���ȡ�����ļ�����
������
����ֵ��
���ߣ�wangbo
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
���ܣ�ͳ�Ƴ����ļ���URL
������
����ֵ��
���ߣ�wangbo
**************************************************/
void HandleUrlFile::analyseExcertFileUrl(std::string& urlString, long urlLength)//�������ļ���URL
{
	long pos = 0, lastPos = 0;
	int rowPos, urlValue;
	std::string tmpRowUrl, tmpUrl;
	if (!lastExcerptUrl.empty())
	{
		pos = urlString.find("\n");
		lastExcerptUrl += urlString.substr( 0, pos);
		rowPos = lastExcerptUrl.find(" ");
		//��ȡURL
		tmpUrl = lastExcerptUrl.substr(0, rowPos);
		//��ȡURL�ĸ���
		urlValue = atoi(lastExcerptUrl.substr(rowPos, lastExcerptUrl.length() - rowPos).c_str());
		excerptFileBuffer[tmpUrl] += urlValue;
		//1 ����\n
		lastPos = pos + 1;
		lastExcerptUrl = "";
	}
	//
	for (long i = 0; i < urlLength;)
	{
		//�ָ��url�ַ�
		pos = urlString.find("\n", lastPos);
		if (pos == -1)
		{
			break;
		}
		tmpRowUrl = urlString.substr(lastPos, pos - lastPos);
		rowPos = tmpRowUrl.find(" ");
		tmpUrl = tmpRowUrl.substr(0, rowPos);
		urlValue = atoi(tmpRowUrl.substr(rowPos, tmpRowUrl.length() - rowPos).c_str());
		//��ϣ��ͳ�Ƹ���
		excerptFileBuffer[tmpUrl] += urlValue;
		//������һ������
		lastPos = pos + 1; //1 ����\n
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
���ܣ����ڴ���ͳ�Ƴ����ļ���URL��Ϣˢ�����
������
����ֵ��
���ߣ�wangbo
**************************************************/
void HandleUrlFile::flushExcerptFile(int hashPos, int excerptPos)
{
	//����ˢ��������
	pairUrlCount fileBuffer;
	
	auto iter = excerptFileBuffer.begin();
	for (; iter != excerptFileBuffer.end(); iter++)
	{
		//д����̸�ʽΪwww.baidu.com 8\n  ��ʾ:url����һ���ո񣬺������url������\n
		fileBuffer.first += (*iter).first + " " + std::to_string((*iter).second) + "\n";
		//2��ʾһ���ո�һ��\n
		fileBuffer.second += (*iter).first.size() + std::to_string((*iter).second).size() + 2;
	}
	//�ؾ�������������
	CloseHandle(excerptFileHandle.at(excerptPos));
	std::string fileName = "E:\\tmp" + std::to_string(hashPos) + ".txt";
	DeleteFileA(fileName.c_str());
	HANDLE hFile = CreateFileA(fileName.c_str(), GENERIC_ALL, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, CREATE_ALWAYS, NULL, nullptr);
	// �ж��ļ��Ƿ񴴽��ɹ�
	if (INVALID_HANDLE_VALUE == hFile)
	{
		std::cout << "rehash file open error";
		return ;
	}

	//ˢ����
	DWORD writeSize = 0;
	WriteFile(hFile, fileBuffer.first.c_str(), fileBuffer.second, &writeSize, NULL);
	FlushFileBuffers(hFile);
	CloseHandle(hFile);
}
