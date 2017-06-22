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
  void send(QByteArray frame);
private:
  QSerialPort *port;
  QByteArray *rx_frame;

  void decodeSLIP(QByteArray *frame);
  void encodeSLIP(QByteArray *frame);

};

#endif // USBSERIAL_H
