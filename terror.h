#ifndef TERROR_H
#define TERROR_H
#include <QString>

class TError
{
  public:
    TError(): msg("")
    {}
    TError(QString _msg): msg(_msg)
    {}
    QString msg;
};

#endif // TERROR_H
