#ifndef NRF_DECODER_H
#define NRF_DECODER_H
#include <QObject>
#include <QTimer>
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
    enum ScanChannel
    {
        SCAN_CH_NONE = 0,
        SCAN_CH_37   = (1<<0),
        SCAN_CH_38   = (1<<1),
        SCAN_CH_39   = (1<<2),
        SCAN_CH_ALL  = (SCAN_CH_37 | SCAN_CH_38 | SCAN_CH_39)
    };
    explicit NrfDecoder(UsbSerial *interface, bool air_quaility = false, bool silent = false, bool crc_err = false, ScanChannel ch_mask = SCAN_CH_ALL, QObject *parent = 0);
    void setAdvChannelHopSeq(ScanChannel channel_mask);
    static ScanChannel channelMaskFromString(QString channels);
    bool setWhiteList(QStringList white_list);
    void setDeltaResolution(bool us);
    void setUseSystemTime(bool system_time);
    void close_port();
private:
    UsbSerial *m_interface;
    quint64 m_rx_counter;
    quint64 m_crc_err_counter;
    quint32 m_rx_counter_window;
    quint32 m_crc_err_counter_window;
    bool m_air_quality_mode;
    bool m_silent_mode;
    bool m_crc_err;
    QTimer *m_window_timer;
    QStringList m_white_list;
    quint64 m_hw_time;
    quint64 m_last_print;
    bool m_use_system_time;
    bool m_delta_us;

    void print_delta();
    void print_timestamp();
signals:

public slots:
    void proccess(QByteArray frame);
    void sendCommand(quint8 id, QByteArray payload);

private slots:
    void scan_window(void);
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
    QString toString(void);
    bool isValid(void);
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
