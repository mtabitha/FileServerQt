#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::slotReadyRead()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_4);

    if (blockSize == 0)
        in >> blockSize;

    qDebug() << "Requared bytes: " << blockSize
             << "\nSocket available bytes" << socket->bytesAvailable();

    if (socket->bytesAvailable() != blockSize)
        return;

    qDebug() << "All Bytes come" << socket->bytesAvailable();

    qint8 command;
    in >> command;

    switch (command) {
        case (0) :
            qDebug() << "ReadFileFromServer";
            ReadFile(in);
            break;
        case (3) :
            qDebug() << "Updating Table";
            UpdatingTable(in);
    }

    blockSize = 0;
}

void MainWindow::UpdatingTable(QDataStream &in)
{
    ui->tableWidget->clear();
    QTableWidget* table = ui->tableWidget;
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels(QStringList() << QString("fileName") << QString("reference") << QString("date"));

    int row = 0;
    QString str;
    QTableWidgetItem* item;
    while (!in.atEnd()){
        table->setRowCount(++row);
        for(int column = 0; column < table->columnCount(); ++column){
            table->setRowCount(row);
            str.clear();
            in >> str;
            qDebug() << str;
            item = new QTableWidgetItem(tr("%1").arg(str));
            table->setItem(row - 1, column, item);
        }
    }
}

void MainWindow::ReadFile(QDataStream &in)
{
    QString fileName;
    in >> fileName;
    QByteArray line = socket->readAll();
    fileName = fileName.section("/", -1);
    QString path = folder + "/" + fileName;
    QFile target(path);
    if (!target.open(QIODevice::WriteOnly)) {
        qDebug() << "Can't open file for written";
        return;
    }
    target.write(line);
    target.close();

    AppendFilesInformation(path);
}

void MainWindow::AppendFilesInformation(QString& downloadedFileName){
    QFile infoFile(folder + "/" + "files.txt");
    infoFile.open(QFile::Append | QIODevice::Text);
    QTextStream out(&infoFile);
    out.setFieldAlignment(QTextStream::AlignLeft);

    QFileInfo newFile(downloadedFileName);
    static int maxLineSize = MaxLineSize(newFile) + 1;

    if (infoFile.size() == 0) {
       CreateFilesInformationTitle(out, maxLineSize);
    }

    for(int i = 0; i < maxLineSize * 4; ++i ) out << "-"; out << "\n";
    out.setFieldWidth(maxLineSize);
    out << "|" + newFile.fileName();
    out << "|" + newFile.absoluteFilePath();
    out << "|" + newFile.created().toString();
    out << "|" + QString::number(newFile.size());
    out.setFieldWidth(0);
    out << "|\n" ;
}

void MainWindow::CreateFilesInformationTitle(QTextStream& infoFileStream, int& maxLineSize) {
    for(int i = 0; i < maxLineSize * 4; ++i ) infoFileStream << "-"; infoFileStream << "\n";
    infoFileStream.setFieldWidth(maxLineSize);
    infoFileStream << "|fileName " << "|reference " << "|date " << "|size ";
    infoFileStream.setFieldWidth(0);
    infoFileStream << "|\n" ;
}

int MainWindow::MaxLineSize(QFileInfo &downloadedFileInfo) {
    int maxLineSize = downloadedFileInfo.fileName().size();
    maxLineSize = maxLineSize < downloadedFileInfo.absoluteFilePath().size() ? downloadedFileInfo.absoluteFilePath().size() : maxLineSize;
    maxLineSize = maxLineSize < downloadedFileInfo.created().toString().size() ? downloadedFileInfo.created().toString().size() : maxLineSize;
    maxLineSize = maxLineSize < QString::number(downloadedFileInfo.size()).size() ? QString::number(downloadedFileInfo.size()).size() : maxLineSize;
    return maxLineSize;
}

void MainWindow::WriteFile(QString &fileName)
{
    QFile file(fileName);

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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    blockSize = 0;

    ui->tableWidget->setEditTriggers(QTableView::NoEditTriggers);

    folder = "tmp";
    if (!QDir(folder).exists())
        QDir().mkdir(folder);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    socket->connectToHost("127.0.0.1", 5555);
}

void MainWindow::on_pushButton_2_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file", "C:\\");
    ui->label->setText(fileName);
}

void MainWindow::on_pushButton_3_clicked()
{
    QString fileName = ui->label->text();
    if (!fileName.isNull() && !fileName.isEmpty()) {
        WriteFile(fileName);
    }
    ui->label->clear();
}

void MainWindow::on_pushButton_4_clicked()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_4);

    QString fileName = ui->lineEdit->text();
    if ( !fileName.isEmpty()){
        out << (quint32)0 << (qint8)1 << fileName;
        out.device()->seek(0);
        out << (quint32)(block.size() - sizeof(quint32));
        socket->write(block);
    }
    ui->lineEdit->clear();
}

void MainWindow::on_pushButton_5_clicked()
{
    folder = QFileDialog::getExistingDirectory(this, "Select folder","C:\\");
    ui->label_2->setText(folder);
}

