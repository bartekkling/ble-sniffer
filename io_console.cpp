#include "io_console.h"
#include <QTextStream>
io_console::io_console(QObject *parent) : QObject(parent)
{

}

void io_console::print(QString text)
{
    QTextStream out(stdout);
    out << QString(text);
}
