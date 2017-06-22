#include "usbserial.h"
#include <QDebug>
#include <QSerialPortInfo>

#define SLIP_CODE_START 0xAB
#define SLIP_CODE_END   0xBC
#define SLIP_CODE_ESC   0xCD

UsbSerial::UsbSerial(QObject *parent) : QObject(parent)
{
    port = new QSerialPort();
    rx_frame = NULL;
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
        if(rx_frame == NULL)
        {
            //wait for SLIP_CODE_START
            if(rx == 0xab)
            {
                rx_frame = new QByteArray();
                rx_frame->append(rx);
            }
        }
        else
        {
            //chek if SLIP_CODE_START
            if(rx == 0xab)
            {
                rx_frame->clear();
            }
            rx_frame->append(rx);

            if(rx == 0xBC)
            {
                //qDebug() << "BEFOR: " << frame->toHex();
                decodeSLIP(rx_frame);
                //qDebug() << "AFTER: " << frame->toHex();
                emit newData(*rx_frame);
                rx_frame->clear();
                delete(rx_frame);
                rx_frame = NULL;
            }

        }
    }
}

void UsbSerial::decodeSLIP(QByteArray *frame)
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

void UsbSerial::encodeSLIP(QByteArray *frame)
{
    const char ab[][2] = {{(char)SLIP_CODE_ESC, (char)0xac},{(char)0xab,(char)0x00}};
    const char bc[][2] = {{(char)SLIP_CODE_ESC, (char)0xbd},{(char)0xbc,(char)0x00}};
    const char cd[][2] = {{(char)SLIP_CODE_ESC, (char)0xce},{(char)0xcd, (char)0x00}};
    frame->replace(QByteArray(cd[1],1), QByteArray(cd[0],2));
    frame->replace(QByteArray(ab[1],1), QByteArray(ab[0],2));
    frame->replace(QByteArray(bc[1],1), QByteArray(bc[0],2));
    frame->prepend(SLIP_CODE_START);
    frame->append(SLIP_CODE_END);
}
void UsbSerial::send(QByteArray frame)
{
  if(port)
  {
    if(port->isWritable())
    {
      encodeSLIP(&frame);
      port->write(frame);
      port->flush();
      //qDebug() << "serial data send: " << frame.toHex();
      return;
    }
  }
  qDebug() << "Can't send serial data";
}
