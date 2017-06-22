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
    QCommandLineOption usbDevicePortOption(QStringList() << "d" << "device", "COM port of serial device.", "COM_PORT");
    parser.addOption(usbDevicePortOption);
    QCommandLineOption airQualityOption(QStringList() << "air-quality", "Enable air quality monitor");
    parser.addOption(airQualityOption);
    QCommandLineOption silentOption(QStringList() << "silent", "Don't report ble packets");
    parser.addOption(silentOption);
    QCommandLineOption channelOption(QStringList() << "ch-list", "Comma separated scan channels. Avaiable channels: 37,38,39. Default: 37,38,39", "channel_mask");
    parser.addOption(channelOption);
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
                NrfDecoder::ScanChannel ch = NrfDecoder::SCAN_CH_ALL;
                if(parser.isSet(channelOption))
                {
                    ch = NrfDecoder::channelMaskFromString(parser.value(channelOption));
                }
                nRF_decoder = new NrfDecoder(port, parser.isSet(airQualityOption), parser.isSet(silentOption), ch);
                io_console::print("Sniffer started...\r\n");
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
