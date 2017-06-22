#include "usbserial.h"
#include <QDebug>
#include <QSerialPortInfo>

UsbSerial::UsbSerial(QObject *parent) : QObject(parent)
{
    port = new QSerialPort();
    frame = NULL;
}

QString UsbSerial::getDeviceList()
{
    QString list;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        list.append(info.portName() + " ");
        list.append(QString("[%1] ").arg(info.vendorIdentifier(),4,16,QChar('0')));
        list.append(QString("[%1] ").arg(info.productIdentifier(),4,16,QChar('0')));
        list.append(info.description() + "\r\n");
    }
    return list;
}

bool UsbSerial::open(QString port_name)
{
    if(port)
    {
        port->setPortName(port_name);
        port->setBaudRate(460800);
        port->setFlowControl(QSerialPort::HardwareControl);
        port->setDataBits(QSerialPort::Data8);
        port->open(QIODevice::ReadWrite);
        if(port->isOpen())
        {
            connect(port, SIGNAL(readyRead()),this, SLOT(newData()));
            return true;
        }
    }
    return false;
}

void UsbSerial::newData()
{
    quint8 rx;
    while(port->read((char *)&rx,1) > 0)
    {
        if(frame == NULL)
        {
            //wait for SLIP_CODE_START
            if(rx == 0xab)
            {
                frame = new QByteArray();
                frame->append(rx);
            }
        }
        else
        {
            //chek if SLIP_CODE_START
            if(rx == 0xab)
            {
                frame->clear();
            }
            frame->append(rx);

            if(rx == 0xBC)
            {
                //qDebug() << "BEFOR: " << frame->toHex();
                decodeSLIP();
                //qDebug() << "AFTER: " << frame->toHex();
                emit newData(*frame);
                frame->clear();
                delete(frame);
                frame = NULL;
            }

        }
    }
}

void UsbSerial::decodeSLIP()
{
    const char ab[][2] = {{(char)0xcd, (char)0xac},{(char)0xab,(char)0x00}};
    const char bc[][2] = {{(char)0xcd, (char)0xbd},{(char)0xbc,(char)0x00}};
    const char cd[][2] = {{(char)0xcd, (char)0xce},{(char)0xcd, (char)0x00}};
    frame->remove(0,1);
    frame->replace(QByteArray(ab[0],2), QByteArray(ab[1],1));
    frame->replace(QByteArray(bc[0],2), QByteArray(bc[1],1));
    frame->replace(QByteArray(cd[0],2), QByteArray(cd[1],1));
    frame->chop(1);
}

//void UsbSerial::send(quint16 id, QByteArray data)
//{
//  Q_UNUSED(id);
//  if(port)
//  {
//    if(port->isWritable())
//    {
//      port->write(data);
//      port->flush();
//      qDebug() << "serial data send";
//      return;
//    }
//  }
//  qDebug() << "Can't send serial data";
//}
