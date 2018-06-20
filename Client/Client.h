#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Client.h"
#include <QMap>
#include <QtNetwork/qudpsocket.h>
#include "ImgGet.h"
#include <curl\curl.h>
#include <QTimer>


class Client : public QMainWindow
{
	Q_OBJECT

public:
	Client(QWidget *parent = Q_NULLPTR);

	~Client();

	static Client *GetInstance();

	bool exit();

	void Transmission();

private:

	void _Init();

	void _addEqu(QString equ, QString ip);

	void _sendImage(QString path);

	int _sendREQ();


signals:
	void reConnect();

private slots:

	void ProcessPendingDatagram();

	void on_search_clicked();

	void on_connect_clicked();

	void on_close_clicked();

	void ReConnect();

	void clickClose();

private:
	Ui::ClientClass ui;
	QMap<QString, QString> m_ipList;
	QUdpSocket *m_receiver;
	int m_ipListCount;
	bool m_conncetFlag;
	bool m_bExit;
	bool m_close;
	HANDLE m_handle;
	static Client *m_ImgTransmissionClient;
	ImgGet *m_imgGet;
	QString m_equName;
	QString m_REQUrl;
	QString m_SendUrl;
	QTimer *m_timer;
};
