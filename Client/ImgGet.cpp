#include "ImgGet.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>

#define  MAX_LEN 1024 * 2000

struct headInfo
{
	int cmd;
	int picSize;
	int contentSize;
	char fileName[37];
	char *content;

};

ImgGet::ImgGet(const std::string &ip, int port)
	:m_ip(ip)
	, m_port(port)
	, m_socket(NULL)
	, m_data(NULL)
{
}

ImgGet::~ImgGet()
{
	close();
}

void ImgGet::close()
{
	if (m_data != NULL)
	{
		delete []m_data;
		m_data = NULL;
	}
	if (NULL != m_socket)
	{
		shutdown(m_socket, 2);
		closesocket(m_socket);
		WSACleanup();
		m_socket = NULL;
	}
}

int ImgGet::init()
{
	m_data = new char[MAX_LEN];
	WSADATA wsaData;
	SOCKADDR_IN addrServer;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int timeout = 300000; //3s
	int ret = setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(m_port);
	inet_pton(AF_INET, m_ip.c_str(), &addrServer.sin_addr.s_addr);
	int err = connect(m_socket, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
	if (err == 0)
		return 1;
	return 0;
}

std::string  ImgGet::Recv()
{
	memset(m_data, 0, MAX_LEN);
	char *nRecvDataBuffer = new char[1024];//一包1024个字节
	bool headFlag = true;
	int nTruePicSize = 0;//图片大小
	int nTotalRecvBytes = 0;
	std::string path = "./img/";
	std::string imgName;
	std::string fileName;
	std::string imgSavePath;
	while (1)
	{
		memset(nRecvDataBuffer, 0, 1024);
		qDebug() << "recv________________________________________________";
		int recvBytes = recv(m_socket, nRecvDataBuffer, 1024, 0);
		qDebug() << "recvlen: " << recvBytes;
		if (recvBytes == 0)
		{
			return "";
		}
		if (recvBytes < 0)
		{
			return "wait";
		}

		if (headFlag)
		{
			//截取头
			headFlag = false;
			headInfo *pHeadInfo = (headInfo *)malloc(1024);
			memcpy(pHeadInfo, nRecvDataBuffer, 1024);
			nTruePicSize = pHeadInfo->picSize;
			fileName = pHeadInfo->fileName;
			char *newPath = new char[8];
			char *newImgName = new char[15];
			strcpy(newPath, fileName.substr(14, 8).c_str());
			strcpy(newImgName, fileName.substr(22, 15).c_str());
			path += newPath;
			imgSavePath = path + newImgName;
			qDebug() << path.c_str();
			QDir dir(path.c_str());
			if (!dir.exists())
			{
				qDebug() << "make";
				QDir make;
				qDebug() << make.mkdir(path.c_str());
			}
			continue;
		}

		if (nTotalRecvBytes + recvBytes <= nTruePicSize && nTotalRecvBytes + recvBytes <= MAX_LEN)
		{
			memcpy(m_data + nTotalRecvBytes, nRecvDataBuffer, recvBytes);
			nTotalRecvBytes += recvBytes;
			qDebug() << "receive ing.....";
		}
		else if (nTruePicSize > nTotalRecvBytes && nTotalRecvBytes + recvBytes > nTruePicSize)
		{
			memcpy(m_data + nTotalRecvBytes, nRecvDataBuffer, nTruePicSize - nTotalRecvBytes);
			nTotalRecvBytes = nTruePicSize;
			qDebug() << "receive all";
		}
		if (nTotalRecvBytes == nTruePicSize)
		{
			qDebug() << "____begin write";
			QFile file(imgSavePath.c_str());
			if (!file.open(QIODevice::ReadWrite))
			{
				qDebug() << "open fail";
			}
			int length = file.write(m_data, nTotalRecvBytes);
			if (length == -1)
			{
				qDebug() << "write fail";
			}
			return imgSavePath;
		}

	}

}
