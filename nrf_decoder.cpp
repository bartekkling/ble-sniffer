#include "nrf_decoder.h"
#include <QDebug>
#include <QException>
#include "io_console.h"
#include "QTime"
#include "QRegExp"

#define REQ_FOLLOW          0x00
#define EVENT_FOLLOW        0x01
#define EVENT_CONNECT       0x05
#define EVENT_PACKET        0x06
#define REQ_SCAN_CONT       0x07
#define EVENT_DISCONNET     0x09
#define SET_TEMP_KEY        0x0c
#define PING_REQ            0x0d
#define PING_RESP           0x0e
#define SET_ADV_CH_HOP_SEQ  0x17

static QMap<quint8, QString> packet_names{
    {REQ_FOLLOW,        "REQ_FOLLOW"},
    {EVENT_FOLLOW,      "EVENT_FOLLOW"},
    {EVENT_CONNECT,     "EVENT_CONNECT"},
    {EVENT_PACKET,      "EVENT_PACKET"},
    {REQ_SCAN_CONT,     "REQ_SCAN_CONT"},
    {EVENT_DISCONNET,   "EVENT_DISCONNET"},
    {SET_TEMP_KEY,      "SET_TEMP_KEY"},
    {PING_REQ,          "PING_REQ"},
    {PING_RESP,         "PING_RESP"},
    {SET_ADV_CH_HOP_SEQ,"SET_ADV_CH_HOP_SEQ"}
};

NrfDecoder::NrfDecoder(UsbSerial *interface, bool air_quaility, bool silent, bool crc_err, ScanChannel ch_mask, QObject *parent) : QObject(parent)
{
    m_interface = interface;
    if(m_interface)
    {
        connect(m_interface, SIGNAL(newData(QByteArray)),this, SLOT(proccess(QByteArray)));
    }
    m_rx_counter = 0;
    m_crc_err_counter = 0;
    m_rx_counter_window = 0;
    m_crc_err_counter_window = 0;
    m_air_quality_mode = air_quaility;
    m_silent_mode = silent;
    m_crc_err = crc_err;
    if(m_air_quality_mode)
    {
        m_window_timer = new QTimer();
        connect(this->m_window_timer, SIGNAL(timeout()),this, SLOT(scan_window()));
        m_window_timer->setSingleShot(false);
        m_window_timer->start(1000);
    }
    setAdvChannelHopSeq(ch_mask);
    m_white_list.clear();
}

void NrfDecoder::setAdvChannelHopSeq(ScanChannel channel_mask)
{
    QByteArray hop_seq;
    quint8 num_of_ch = 0;
    if(channel_mask & SCAN_CH_37)
    {
        num_of_ch++;
        hop_seq.append(37);
    }
    if(channel_mask & SCAN_CH_38)
    {
        num_of_ch++;
        hop_seq.append(38);
    }
    if(channel_mask & SCAN_CH_39)
    {
        num_of_ch++;
        hop_seq.append(39);
    }
    hop_seq.prepend(num_of_ch);
    if(num_of_ch != 0)
    {
        sendCommand(SET_ADV_CH_HOP_SEQ, hop_seq);
    }
    else
    {
        qDebug() << "Invalid channel selection";
    }
}

NrfDecoder::ScanChannel NrfDecoder::channelMaskFromString(QString channels)
{
    ScanChannel ch_mask = (ScanChannel)0;
    if(channels.contains("37"))
    {
      ch_mask = ScanChannel(ch_mask | SCAN_CH_37);
    }
    if(channels.contains("38"))
    {
      ch_mask = ScanChannel(ch_mask | SCAN_CH_38);
    }
    if(channels.contains("39"))
    {
      ch_mask = ScanChannel(ch_mask | SCAN_CH_39);
    }
    return ch_mask;
}

bool NrfDecoder::setWhiteList(QStringList white_list)
{
    bool ret_val = true;
    m_white_list = white_list;
    QRegExp rx_no_colon("([0-9A-Fa-f]{2}){6}");
    QRegExp rx_colon("([0-9A-Fa-f]{2}[:]){5}([0-9A-Fa-f]{2})");

    for(int i = 0; i < m_white_list.size(); i++)
    {
        if(rx_no_colon.indexIn(m_white_list.at(i)) >= 0)
        {
            QString temp = m_white_list.at(i);
            for(int j = 2; j < temp.size(); j+=3)
                temp.insert(j,QChar(':'));
            m_white_list.replace(i,temp);
        }
        else if(rx_colon.indexIn(m_white_list.at(i)) >= 0)
        {
            ret_val = true;
        }
        else
        {
            io_console::print(m_white_list.at(i) + " has invalid format!\r\n");
            ret_val = false;
            break;
        }
    }
//    foreach (const QString &str, m_white_list)
//    {
//        qDebug() << "Filter: " << str;
//    }
    return ret_val;
}

void NrfDecoder::proccess(QByteArray frame)
{
    NrfPacket *nrf_packet = new NrfPacket(frame);
    //qDebug() << "nrf decode: " << frame.toHex();
    //qDebug() << packet->getPacketName();
    switch(nrf_packet->getPacketType())
    {
    case EVENT_PACKET:
        NrfPacketBle *ble_packet;
        ble_packet = new NrfPacketBle(nrf_packet);

        if(!m_silent_mode)
        {
            if(ble_packet->isValid() || m_crc_err)
            {
                if(m_white_list.isEmpty())
                {
                    io_console::print(ble_packet->toString());
                }
                else if(m_white_list.contains(ble_packet->getMacAddr(),Qt::CaseInsensitive))
                {
                    io_console::print(ble_packet->toString());
                }
            }

        }

        if(m_air_quality_mode)
        {
            m_rx_counter++;
            m_rx_counter_window++;
            if(!ble_packet->isValid())
            {
                m_crc_err_counter++;
                m_crc_err_counter_window++;
            }
        }

        delete(ble_packet);
        break;
    default:
        break;
    }
    delete(nrf_packet);
}

void NrfDecoder::sendCommand(quint8 id, QByteArray payload)
{
    QByteArray usb_command;
    usb_command.append(0x06); //header len
    usb_command.append(payload.length());  // payload lenght
    usb_command.append(0x01); //prot ver
    usb_command.append(char(0x00)); //packet counter LSB
    usb_command.append(char(0x00)); //packet counter MSB
    usb_command.append(id); //packet type
    usb_command.append(payload);

    //qDebug() << usb_command.toHex();
    m_interface->send(usb_command);
}

void NrfDecoder::scan_window()
{
    QString report;

    report.append(QString("Packets/s: %1   ").arg(m_rx_counter_window, 4));
    report.append(QString("CRC_ERR/s: %1   ").arg(m_crc_err_counter_window, 4));
    report.append(QString("Packets/session: %1   ").arg((float)m_rx_counter, 10));
    report.append(QString("CRC_ERR/session: %1%   \r\n").arg((float)m_crc_err_counter*100/m_rx_counter, 2));

    io_console::print(report);

    m_rx_counter_window = 0;
    m_crc_err_counter_window = 0;
}

NrfPacket::NrfPacket(QByteArray frame)
{
    this->m_header = new NrfPacketHeader(frame.left(6));
    //qDebug() << "m_header_pc" << m_header->getPacketCounter();
    this->m_payload = new QByteArray(frame.remove(0,6));
}

NrfPacket::~NrfPacket()
{
    delete(m_header);
    delete(m_payload);
}

QString NrfPacket::getPacketName()
{
    return packet_names.value(m_header->getPacketType());
}

quint8 NrfPacket::getPacketType()
{
    return m_header->getPacketType();
}

QByteArray *NrfPacket::getPayload()
{
    return m_payload;
}

NrfPacketHeader::NrfPacketHeader(QByteArray header)
{
    if(header.length() == 6)
    {
        m_header_len = header.at(0);
        m_payload_len = header.at(1);
        m_protover = header.at(2);
        m_packet_counter = (((quint16)header.at(3)) & 0x00FF) + ((((quint16)header.at(4)) << 8) & 0xFF00);
        m_packet_type = header.at(5);
    }
    else
    {
        try
        {
            throw header.length();
        }
        catch(int param)
        {
            qDebug() << "Invalid nRF packet header len: " << param;
        }
    }

}

quint8 NrfPacketHeader::getPacketType()
{
    return m_packet_type;
}

quint8 NrfPacketHeader::getPaylaodLen()
{
    return m_payload_len;
}

quint8 NrfPacketHeader::getProtcolVersion()
{
    return m_protover;
}

quint16 NrfPacketHeader::getPacketCounter()
{
    return m_packet_counter;
}

NrfPacketBle::NrfPacketBle(NrfPacket *packet)
{
    if(packet->getPacketType() == EVENT_PACKET)
    {
        m_header_len = packet->getPayload()->at(0);
        m_header_flags = packet->getPayload()->at(1);
        m_header_channel = packet->getPayload()->at(2);
        m_header_rssi = packet->getPayload()->at(3);
        //ec [5][4]
        //        quint16 ec = packet->getPayload()->at(4) + (packet->getPayload()->at(5)<<8);
        //        qDebug() << "Ec: " << ec;
        m_header_time_diff = (((quint32)packet->getPayload()->at(6)) & 0x000000FF) + ((((quint32)packet->getPayload()->at(7)) << 8) & 0x0000FF00) + ((((quint32)packet->getPayload()->at(8)) << 16) & 0x00FF0000) + ((((quint32)packet->getPayload()->at(9)) << 24) & 0x0FF000000);

        m_acces_address = packet->getPayload()->mid(10,4);
        m_pdu_type = (((quint8)(packet->getPayload()->at(14)))) & 0x0F;
        m_tx_add = (((quint8)(packet->getPayload()->at(14))) >> 7) & 0x01;
        m_rx_add = (((quint8)(packet->getPayload()->at(14))) >> 6) & 0x01;
        m_length = ((quint8)(packet->getPayload()->at(15)));
        //padding [16]
        m_bd_address = packet->getPayload()->mid(17,6);
        m_payload = packet->getPayload()->mid(23,m_length-6);
        m_crc = packet->getPayload()->right(3);
        //        qDebug() <<"RX: " << packet->getPayload()->toHex();
        //        qDebug() << "m_length:" << m_length;
        //        qDebug() << "bd addr:"  << m_bd_address.toHex();
        //        qDebug() << "payload:"  << m_payload.toHex();
        //        qDebug() << "crc:"  << m_crc.toHex();
        //        qDebug() << "time diff: " << m_header_time_diff;
        //        qDebug() << "ch" << m_header_channel;
    }
}

QString NrfPacketBle::getMacAddr()
{
    QString mac_addr = QString("%1:%2:%3:%4:%5:%6").arg((quint8)m_bd_address.at(5),2,16,QChar('0')).arg((quint8)m_bd_address.at(4),2,16,QChar('0')).arg((quint8)m_bd_address.at(3),2,16,QChar('0')).arg((quint8)m_bd_address.at(2),2,16,QChar('0')).arg((quint8)m_bd_address.at(1),2,16,QChar('0')).arg((quint8)m_bd_address.at(0),2,16,QChar('0'));
    return mac_addr;
}

QByteArray NrfPacketBle::getBdAddr()
{
    return m_bd_address;
}

quint8 NrfPacketBle::getChannel()
{
    return m_header_channel;
}

quint32 NrfPacketBle::getTimeDiff()
{
    return m_header_time_diff;
}

QString NrfPacketBle::getPduTypeName()
{
    switch(m_pdu_type)
    {
    case 0x00:
        return "ADV_IND";
        break;
    case 0x01:
        return "ADV_DIRECT_IND";
        break;
    case 0x02:
        return "ADV_NON_CONN_IND";
        break;
    case 0x03:
        return "SCAN_REQ";
        break;
    case 0x04:
        return "SCAN_RSP";
        break;
    case 0x05:
        return "CONNECT_REQ";
        break;
    case 0x06:
        return "ADV_SCAN_IND";
        break;
    default:
        return "ADV_UNKNOWN";
        break;
    }
}

QByteArray NrfPacketBle::getPayload()
{
    return m_payload;
}

quint8 NrfPacketBle::getFlags()
{
    return m_header_flags;
}

qint8 NrfPacketBle::getRssi()
{
    return m_header_rssi;
}

QString NrfPacketBle::toString()
{
    QString ret_val;

//    ret_val.append(QString("%1(+%2)   ").arg(QTime::currentTime().toString("hh:mm:ss:zzz")).arg((float)this->getTimeDiff()/1000,8));
    ret_val.append(QString("%1   ").arg(QTime::currentTime().toString("hh:mm:ss:zzz")));
    ret_val.append(QString("[%1]   ").arg(this->getChannel(),2));
    ret_val.append(QString("-%1dbm   ").arg(this->getRssi(),2));
    if(this->isValid() & 0x01)
    {
        ret_val.append(QString("%1   ").arg(this->getPduTypeName(),20));
        ret_val.append(QString("%1   ").arg(this->getMacAddr()));
        ret_val.append(QString("%1\r\n").arg(QString(this->getPayload().toHex())));
    }
    else
    {
        ret_val.append(QString("CRC_ERROR\r\n"));
    }
    return ret_val;
}

bool NrfPacketBle::isValid()
{
    return (this->m_header_flags & 0x01);
}
