#include "dialog.h"
#include "terror.h"

#include <QApplication>
#include <QTextCodec>
#include <QErrorMessage>
#include <iostream>
#include <QFile>
#include <QFileInfo>
#include "utils.h"
#include <QThread>
int main(int argc, char *argv[])
{
    //для поддержки рускоязычных каталогов и файлов
    QTextCodec* codec = QTextCodec::codecForName("UTF_8");
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);
    QThread thread;

    QApplication a(argc, argv);
 //   try
 //   {

        Dialog dialog;
        dialog.show();


 //   }
/*
    catch(TError& err)
    {
        //(new QErrorMessage)->showMessage(err.msg);
    }

    catch(...)
    {

    }
    */

    return a.exec();
}
