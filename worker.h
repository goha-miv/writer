#ifndef WORKER_H
#define WORKER_H
#include <QFile>
#include <iostream>

//-----------------------------------------------
//TaskWriter для осуществления копирования файлов
//-----------------------------------------------
class TaskWriter : public QObject
{
  Q_OBJECT
public:
    TaskWriter();
    ~TaskWriter();
    void setSourceFile(QString path);       //устанавливает полное имя исходного файла
    void setDestinationFile(QString path);  //устанавливает полное имя файла назначения
    const QString sourceName();
    const QString destinationName();

signals:
    void progress(qint64);                     //сигнализирует о состоянии прогресса записи
    void finished();                        //сигнализирует об окончании записи в не зависимости от успеха


public slots:
    void doWrite();                         //выполняет основную задачу копирования файлов
    void doStop() { is_stop = true; }       //инициирует останов задачи записи
private slots:
    void doFinal();                         //инициирует окончание записи вне зависимости от успеха
private:
    volatile bool is_stop;
    QFile* fsrc;                            //дескриптор файла источника
    QFile* fdst;                            //дескриптор файла назначения
};

//---------------------------------------------------
//TaskScanner для поиска подключенных usb накопителей
//---------------------------------------------------
class TaskScanner : public QObject
{
  Q_OBJECT

public:
    TaskScanner();
    ~TaskScanner();

signals:
    void finished();                    //сигнализирует о завершении сканирования
    void refrash_device();              //сигнализирует о изменении списка подключенных устройств

public slots:
    void doScan();                        //выполняет основную задачу обнаружения подключения/отключения usb накопителей
    void doStop() {is_stop = true;}       //инициирует останов задачи сканирования
private:
    volatile bool is_stop;

};

#endif // WORKER_H
