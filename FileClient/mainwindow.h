#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QFileDialog>
#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <QTextCodec>
#include <QDialog>
#include <QFileInfo>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:

    QTcpSocket *socket;
    qint32 blockSize;
    QString folder;

    void UpdatingTable(QDataStream &in);
    void ReadFile(QDataStream &in);
    void WriteFile(QString &fileName);
private slots:
    void slotReadyRead();


    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
