#include "fileserver.h"

void FileServer::slotReadyRead()
{
    socket = (QTcpSocket*)sender();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_4);


    if (blockSize == 0) {
        in >> blockSize;
    }
    if (socket->bytesAvailable()  != blockSize)
        return;
    qDebug() << "All Bytes come" << socket->bytesAvailable();
    qint8 command;
    in >> command;
    switch (command) {
        case (0) :
            ReadFile(in);
            break;
        case (1) :
            WriteFile(in);
            break;
    }
    blockSize = 0;
    fileCounter = 0;
}

void FileServer::ReadFile(QDataStream &in)
{
    QString fileName;
    in >> fileName;
    QByteArray line = socket->readAll();
    fileName = fileName.section("/", -1);
    QFile target(folder + "/" + fileName);
    if (!target.open(QIODevice::WriteOnly)) {
        qDebug() << "Can't open file for written";
        return;
    }
    target.write(line);
    target.close();
    for(int i = 0; i < sockets.size(); ++i) {
        updateFileTable(sockets[i]);
    }
}

void FileServer::WriteFile(QDataStream &in)
{
    QString fileName;
    in >> fileName;
    QFile file(folder + "/" + fileName);
    qDebug() << folder + "/" + fileName;

    if (file.open(QIODevice::ReadOnly)){

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_4);

        out << (quint32)0 << (qint8)0 << file.fileName();
        block.append(file.readAll());
        file.close();

        out.device()->seek(0);
        out << (quint32)(block.size() - sizeof(quint32));

        socket->write(block);
        qDebug() << "Bytes wite: " << (quint32)(block.size() - sizeof(quint32));

    }
    else {
        qDebug() << "Error Open File form Client";
    }
}

void FileServer::updateFileTable(QTcpSocket* s) {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_4);

    out << (quint32)0 << (qint8)3;

    QDir dir(folder);

    for(QFileInfo& f : dir.entryInfoList(QDir::Filter::Files)) {
        qDebug() << f.fileName();
        out << f.fileName()<< f.absoluteFilePath() << f.created().toString();
    }

    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));

    s->write(block);
    qDebug() << "Bytes wite: " << (quint32)(block.size() - sizeof(quint32));

}

FileServer::FileServer(QString folder) : folder(folder)
{
    if (this->listen(QHostAddress::Any, 5555))
        qDebug() << "Listening";
    else
        qDebug() << "Error: listening";
    if (!QDir(folder).exists())
        QDir().mkdir(folder);
    blockSize = 0;
}

void FileServer::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &FileServer::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    sockets.push_back(socket);
    qDebug() << "Client connected " << socketDescriptor;
    updateFileTable(socket);
}
