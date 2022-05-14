#ifndef FILESERVER_H
#define FILESERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QDataStream>
#include <QByteArray>
#include <QDir>
#include <QDirIterator>
#include <QDateTime>

class FileServer : public QTcpServer
{
    Q_OBJECT
public:
    FileServer(QString folder);

private:
    QTcpSocket *socket;
    QVector<QTcpSocket*> sockets;
    QByteArray Data;
    QString folder;
    qint32 blockSize;
    int fileCounter;

    void updateFileTable(QTcpSocket* socket);
    void ReadFile(QDataStream &in);
    void WriteFile(QDataStream &in);

public slots:
    void slotReadyRead();

private slots:

    void incomingConnection(qintptr socketDescriptor);
};

#endif // FILESERVER_H
