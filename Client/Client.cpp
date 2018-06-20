#include "Client.h"
#include <windows.h>
#include <process.h>
#include <QTableWidget>
#include <QDebug>
#include <QFile>
#include <assert.h>

#define GET_HOST_COMMAND "GetIPAddr"
#define LOCAL_PORT 0x5050
#define DEST_PORT 12811
#define TRY_TIMES 1

std::string HTTPRESULT;
DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	//线程函数  
	while (!Client::GetInstance()->exit())
	{
		qDebug() << "conncet__________";
		Client::GetInstance()->Transmission();
	}
	qDebug() << "exit__________";
	return 0;
}

Client *Client::m_ImgTransmissionClient = NULL;

void Write_data(void* buffer, size_t size, size_t nmemb, void* user_p) {
	HTTPRESULT += (const char*)buffer;
}

static size_t process_data(void *data, size_t size, size_t nmemb, std::string &content)
{
	long sizes = size * nmemb;
	std::string temp;
	temp = std::string((char*)data, sizes);
	content += temp;
	return sizes;
}

Client::Client(QWidget *parent)
	: QMainWindow(parent)
	, m_conncetFlag(false)
	, m_bExit(false)
	, m_timer(NULL)
	, m_close(false)
{
	ui.setupUi(this);
	QFile file("./config/ipconfig.txt");
	if (!file.open(QIODevice::ReadOnly));
	{
		qDebug() << "config read fail";
	}
	QTextStream out(&file);
	while (!out.atEnd())
	{
		QString lineText = out.readLine();
		qDebug() << "ip: " << lineText;
		ui.ip->setText(lineText);
	}


	QFile file1("./config/url.txt");
	if (!file1.open(QIODevice::ReadOnly));
	{
		qDebug() << "config read fail";
	}
	QTextStream out1(&file1);
	while (!out1.atEnd())
	{
		QString lineText = out1.readLine();
		qDebug() << "url: " << lineText;
		ui.url->setText(lineText);
	}

	QFile file2("./config/equ.txt");
	if (!file2.open(QIODevice::ReadOnly));
	{
		qDebug() << "config read fail";
	}
	QTextStream out2(&file2);
	while (!out2.atEnd())
	{
		QString lineText = out2.readLine();
		qDebug() << "equ: " << lineText;
		ui.equ->setText(lineText);
	}
	m_timer = new QTimer;
	connect(this, SIGNAL(reConnect()), this, SLOT(ReConnect()));
	if(NULL == m_ImgTransmissionClient)
		m_ImgTransmissionClient = this;

}

Client::~Client()
{
	if (m_timer != NULL)
	{
		delete m_timer;
		m_timer = NULL;
	}

}

Client *Client::GetInstance()
{
	return m_ImgTransmissionClient;
}

//搜索设备
void Client::on_search_clicked()
{
	qDebug() << "search";
	_Init();
}

void Client::_Init()
{
	m_receiver = new QUdpSocket();
	m_receiver->bind(LOCAL_PORT, QUdpSocket::ShareAddress);
	connect(m_receiver, SIGNAL(readyRead()), this, SLOT(ProcessPendingDatagram()));
}

void Client::ProcessPendingDatagram()
{
	qDebug() << "ProcessPendingDatagram";
	while (m_receiver->hasPendingDatagrams())
	{
		QByteArray datagram;
		//datagram大小为等待处理数据报的大小才能就收数据;
		datagram.resize(m_receiver->pendingDatagramSize());
		//接收数据报
		QHostAddress addr;
		quint16 port;
		m_receiver->readDatagram(datagram.data(), datagram.size(), &addr, &port);
		if (!m_ipList.contains(addr.toString().section(":", -1)))
		{
			m_ipList.insert(addr.toString().section(":", -1), datagram);
			_addEqu(addr.toString().section(":", -1), datagram);
		}
	}
}

void Client::_addEqu(QString ip, QString equ)
{

	QTableWidgetItem *Equ = new QTableWidgetItem;
	QTableWidgetItem *EquIP = new QTableWidgetItem;
	int count = ui.tableWidget->rowCount();
	ui.tableWidget->insertRow(count);
	Equ->setText(equ);
	EquIP->setText(ip);
	ui.tableWidget->setItem(count, 0, Equ);
	ui.tableWidget->setItem(count, 1, EquIP);
}

void Client::on_connect_clicked()
{
	if (!m_conncetFlag)
	{
		//QPalette pal = ui.connect->palette();
		//pal.setColor(QPalette::Button, Qt::red);
		//ui.connect->setPalette(pal);
		//ui.connect->setAutoFillBackground(true);
		//ui.connect->setFlat(true);
		QString getIp = ui.ip->text();
		QFile file1("./config/ipconfig.txt");
		if (file1.open(QFile::WriteOnly | QIODevice::Truncate))
		{
			QTextStream out(&file1);
			out << getIp;
		}

		QString getUrl = ui.url->text();
		QFile file2("./config/url.txt");
		if (file2.open(QFile::WriteOnly | QIODevice::Truncate))
		{
			QTextStream out(&file2);
			out << getUrl;
		}

		QString getEqu;
		if (ui.equ->text().isNull())
		{
			ui.equ->setText(m_ipList[getIp]);
		}
		getEqu = ui.equ->text();
		QFile file3("./config/equ.txt");
		if (file3.open(QFile::WriteOnly | QIODevice::Truncate))
		{
			QTextStream out(&file3);
			out << getEqu;
		}
		QString EQU = getEqu;
		//m_REQUrl = REQ  + EQU;
		m_SendUrl = getUrl+ "?equ=" + EQU;
		m_imgGet = new ImgGet(getIp.toStdString(), 8080);
		int flag = m_imgGet->init();
		if (!flag)
		{
			ui.info->setText(QStringLiteral("连接设备失败"));
			return;
		}
		m_close = false;
		m_bExit = false;
		m_conncetFlag = true;
		m_handle = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
	}

}

void Client::Transmission()
{
	ui.info->setText(QStringLiteral("接收中______"));
	QString path = m_imgGet->Recv().c_str();
	ui.info->setText("");
	qDebug() << path;
	QFile img(path);
	if (img.exists())
	{
		/*下一版做*/
		//qDebug() << "img exits";
		//int resp = _sendREQ();
		//qDebug() << "resp is " << resp;
		//if(resp == 1)
		//	_sendImage(path);
		ui.info->setText(QStringLiteral("发送中______"));
		_sendImage(path);
	}
	else if (path.isEmpty())
	{
		qDebug() << "m_conncetFlag is false";
		qDebug() << "m_conncetFlag is false";
		qDebug() << "m_conncetFlag is false";
		qDebug() << "m_conncetFlag is false";
		qDebug() << "m_conncetFlag is false";
		clickClose();
	}
	else if (path == "wait")
	{
		qDebug() << "wait";
		qDebug() << "wait";
		qDebug() << "wait";
		qDebug() << "wait";
		qDebug() << "wait";
		qDebug() << "wait";
		qDebug() << "wait";
		qDebug() << "wait";
		qDebug() << "wait";
		clickClose();
		if(!m_close)
			emit reConnect();
	}
	else
	{
	}
}

void Client::clickClose()
{
	if (m_imgGet != NULL)
	{
		m_bExit = true;
		CloseHandle(m_handle);
		m_handle = NULL;
		m_imgGet->close();
		delete m_imgGet;
		m_imgGet = NULL;
		/*if (m_conncetFlag)
		{
		QPalette pal = ui.connect->palette();
		pal.setColor(QPalette::Button, Qt::green);
		ui.connect->setPalette(pal);
		ui.connect->setAutoFillBackground(true);
		}*/
		m_conncetFlag = false;
	}
}

void Client::on_close_clicked()
{
	clickClose();
	m_close = true;
}

void Client::_sendImage(QString path)
{
	qDebug() << "send__________________________________________________";
	CURL *curl = curl_easy_init();
	CURLcode res = curl_global_init(CURL_GLOBAL_WIN32);

	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;
	curl_slist* pOptionList = NULL;
	pOptionList = curl_slist_append(pOptionList, "Expect:");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pOptionList);


	char* file_data = NULL;
	long file_size = 0;
	//FILE* fp = fopen("./img/20180608/114258--OS.jpg", "rb");
	FILE* fp = fopen(path.toStdString().c_str(), "rb");
	if (fp)
	{
		qDebug() << "fopen path";
		fseek(fp, 0, SEEK_END);
		file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		file_data = new char[file_size + 1];
		fread(file_data, 1, file_size, fp);
		fclose(fp);
	}

	curl_formadd(&formpost, &lastptr,
		CURLFORM_COPYNAME, "file",
		CURLFORM_BUFFER, path.section('/', -1).toStdString().c_str(),
		CURLFORM_BUFFERPTR, file_data,
		CURLFORM_BUFFERLENGTH, file_size,
		CURLFORM_CONTENTTYPE, "image/jpeg",
		CURLFORM_END);


	if (curl) {
		/* what URL that receives this POST */
		curl_easy_setopt(curl, CURLOPT_URL, m_SendUrl.toStdString().c_str());
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &HTTPRESULT);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Write_data);

		char error[1024];
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			qDebug() << "error: " << error;

		long responseCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
		qDebug() << "responseCode :" << responseCode;
	}
	qDebug() << "url end";
	curl_easy_cleanup(curl);
	curl_formfree(formpost);
	qDebug() << "___________________________";
	qDebug() << "___________________________";
	//qDebug() << HTTPRESULT.c_str();
	ui.info->setText(QStringLiteral("发送完毕______"));

	if (file_data != NULL)
		delete[] file_data;
	return;
}

int Client::_sendREQ()
{
	CURL *curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	std::string content;
	content.clear();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, m_REQUrl.toStdString().c_str());
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			fprintf(stderr, "Curl perform failed: %s\n", curl_easy_strerror(res));
			return 0;
		}
	}
	curl_global_cleanup();
	if (content == "1")
	{
		return 1;
	}
	return 0;
}

bool Client::exit()
{
	return m_bExit;
}

void Client::ReConnect()
{
	//m_timer->stop();
	on_connect_clicked();
}


