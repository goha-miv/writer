#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QThread>
#include <worker.h>
class Flash;

class QFileDialog;

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    Ui::Dialog *ui;
    QFileDialog* fdlg;
    QVector<QThread*> threads;
    TaskWriter* writer;
    TaskScanner* scanner;
    std::vector<Flash*> vt_flash;               //список usb накопителей в системе
    Flash* flash;

public slots:
    void writeFile();                           //инициализирует запись файла на накопитель и старт потока с задачей writer
    void writeCancel();                         //инициализирует отмену записи
    void writeFinish();                         //инициализирует окончание записи и завершение потока с задачей writer

private slots:
    void changeState();                         //изменяет состояние элементов формы
    void updateFlash();                         //обновляет список подключенных накопителей
    void updateProgres(qint64);                 //обновляем прогресс записи
    void setFlash(const QString& model);        //устанавливаем текущий накопитель по модели в ComboBox
    void mountDev();                            //монтирует выбранный накопитель
};

#endif // DIALOG_H
