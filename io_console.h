#ifndef IO_CONSOLE_H
#define IO_CONSOLE_H
#include <QObject>

class io_console : public QObject
{
    Q_OBJECT
public:
    explicit io_console(QObject *parent = 0);
    static void print(QString text);
signals:

public slots:
};

#endif // IO_CONSOLE_H
