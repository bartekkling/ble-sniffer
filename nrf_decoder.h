#ifndef NRF_DECODER_H
#define NRF_DECODER_H
#include <QObject>
#include "usbserial.h"

class NrfPacketHeader
{
public:
    NrfPacketHeader (QByteArray header);
    quint8 getPacketType(void);
    quint8 getPaylaodLen(void);
    quint8 getProtcolVersion(void);
    quint16 getPacketCounter(void);

private:
    quint8 m_header_len;
    quint8 m_payload_len;
    quint8 m_protover;
    quint16 m_packet_counter;
    quint8  m_packet_type;

};

class NrfPacket
{
public:
    NrfPacket (QByteArray frame);
    ~NrfPacket();
    QString getPacketName(void);
    quint8 getPacketType(void);
    QByteArray *getPayload(void);
private:
    NrfPacketHeader *m_header;
    QByteArray *m_payload;

};

class NrfDecoder : public QObject
{
    Q_OBJECT
public:
    explicit NrfDecoder(UsbSerial *interface, QObject *parent = 0);

private:
    UsbSerial *m_interface;
    quint32 m_rx_counter;
    quint32 m_crc_err_counter;
signals:

public slots:
    void proccess(QByteArray frame);
};


class NrfPacketBle
{
public:
    NrfPacketBle(NrfPacket* packet);
    QString getMacAddr(void);
    QByteArray getBdAddr(void);
    quint8 getChannel(void);
    quint32 getTimeDiff(void);
    QString getPduTypeName(void);
    QByteArray getPayload(void);
    quint8 getFlags(void);
    qint8 getRssi(void);
private:
    //header
        quint8 m_header_len;
        quint8 m_header_flags;
        quint8 m_header_channel;
        qint8 m_header_rssi;
        quint16 m_header_event_counter;
        quint32 m_header_time_diff;
   //packet
        QByteArray m_acces_address;
        quint8 m_pdu_type;
        quint8 m_tx_add;
        quint8 m_rx_add;
        quint8 m_length;
        QByteArray m_bd_address;
        QByteArray m_payload;
        QByteArray m_crc;

};
#endif // NRF_DECODER_H