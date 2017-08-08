#include <QCoreApplication>
#include <QCommandLineParser>

#include <QObject>
#include <iostream>
#include <cstdlib>

#include "usbserial.h"
#include "io_console.h"
#include "nrf_decoder.h"
#include "QDebug"

int main(int argc, char *argv[])
{
    UsbSerial *port = NULL;
    NrfDecoder *nRF_decoder = NULL;
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("BLE sniffer");
    parser.addPositionalArgument("Device port", "COM port for serial device.", "COM_PORT");

    QCommandLineOption usbDeviceListOption(QStringList() << "list", "List all serial devices and exit");
    parser.addOption(usbDeviceListOption);
    QCommandLineOption airQualityOption(QStringList() << "air-quality", "Enable air quality monitor");
    parser.addOption(airQualityOption);
    QCommandLineOption silentOption(QStringList() << "silent", "Don't report ble packets");
    parser.addOption(silentOption);
    QCommandLineOption channelOption(QStringList() << "cl" << "ch-list", "Comma separated scan channels\r\n Default: 37,38,39", "channel_mask");
    parser.addOption(channelOption);
    QCommandLineOption whiteList(QStringList() << "wl" << "white-list", "MAC address of device to add to white list\r\nEx.: 0102ABCD44, AA:BB:44:53:78:A4, aabb554433dd, aa:dd:54:22:33:de", "mac_address");
    parser.addOption(whiteList);
    parser.addHelpOption();

    parser.process(a);
    const QStringList args = parser.positionalArguments();

    //List Serial devices
    if(parser.isSet(usbDeviceListOption))
    {
      io_console::print(UsbSerial::getDeviceList());
      return 0;
    }
    else if(args.isEmpty())
    {
        io_console::print("Please specify device. Avaiable devices: \r\n");
        io_console::print(UsbSerial::getDeviceList());
        return 0;
    }
    else
    {
        port = new UsbSerial();
        if(port)
        {
            if(port->open(args.at(0)))
            {
                NrfDecoder::ScanChannel ch = NrfDecoder::SCAN_CH_ALL;
                if(parser.isSet(channelOption))
                {
                    ch = NrfDecoder::channelMaskFromString(parser.value(channelOption));
                }
                nRF_decoder = new NrfDecoder(port, parser.isSet(airQualityOption), parser.isSet(silentOption), ch);
                if(nRF_decoder->setWhiteList(parser.values(whiteList)))
                {
                    io_console::print("Sniffer started...\r\n");
                }
                else
                {
                    return 0;
                }
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
