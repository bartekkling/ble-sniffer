#include <QCoreApplication>
#include <QCommandLineParser>

#include <QObject>
#include <iostream>
#include <cstdlib>

#include "usbserial.h"
#include "io_console.h"
#include"nrf_decoder.h"

int main(int argc, char *argv[])
{
    UsbSerial *port = NULL;
    NrfDecoder *nRF_decoder = NULL;
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Radio Bridge emulator");
    parser.addHelpOption();
    QCommandLineOption usbDeviceListOption(QStringList() << "l" << "list", "List all serial devcices and ends program");
    parser.addOption(usbDeviceListOption);
    QCommandLineOption usbDevicePortOption(QStringList() << "d" << "device", "COM port of serial device.", "[COM_PORT]");
    parser.addOption(usbDevicePortOption);
    parser.process(a);

    //List Serial devices
    if(parser.isSet(usbDeviceListOption))
    {
      io_console::print(UsbSerial::getDeviceList());
      return 0;
    }
    else
    {
        port = new UsbSerial();
        if(port)
        {
            if(port->open(parser.value(usbDevicePortOption)))
            {
                io_console::print("Port ready");
                nRF_decoder = new NrfDecoder(port);
            }
            else
            {
                io_console::print("Can't open port");
                return 0;
            }
        }
    }

    return a.exec();
}
