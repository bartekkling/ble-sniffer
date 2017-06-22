#ifndef USBSERIAL_H
#define USBSERIAL_H

#include <QObject>
#include <QSerialPort>
class UsbSerial : public QObject
{
  Q_OBJECT
public:
  explicit UsbSerial(QObject *parent = 0);
  static QString getDeviceList(void);
signals:
  void newData(QByteArray data);
public slots:
  bool open(QString port_name);
  void newData(void);
  //void send(quint16 id,QByteArray data);
private:
  QSerialPort *port;
  QByteArray *frame;

  void decodeSLIP(void);
};

#endif // USBSERIAL_H
