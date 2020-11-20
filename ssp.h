#ifndef SSP_T_H
#define SSP_T_H

#include <QObject>

#include <QTime>
#include <QString>

#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>

#include <QTcpSocket>
#include <QAbstractSocket>

/* Defines -------------------------------------------------------------------*/
#define SPP_MAGIC           0xA3

#define SPP_STATE_WAIT      0
#define SPP_STATE_LEN1      1
#define SPP_STATE_LEN2      2
#define SPP_STATE_DATA      3
#define SPP_STATE_CC1       4
#define SPP_STATE_CC2       5
/* ---------------------------------------------------------------------------*/

class ssp_t : public QObject
{
    Q_OBJECT
public:
    explicit ssp_t(QObject *parent = nullptr);
    ~ssp_t();

    int send_packet(uint8_t *data, uint8_t len);

public slots:
    void read_data(uint8_t *data, uint16_t len);

signals:
    void read_packet(uint8_t *data, uint16_t len);
    void write_data(uint8_t *data, uint16_t len);

private:
    char read_buff[4096];
    uint8_t packet_buff[4096];

    int rx_byte(uint8_t b);

    uint8_t *buff;        //ukazatel na pocatek bufferu

    uint8_t *write;       //zapisovaci ukazatel
    uint16_t size;        //velikost bufferu

    uint8_t magic;        //stav parseru
    uint8_t state;        //stav parseru
    uint16_t len;         //stav parseru
    uint16_t cc;          //stav parseru
    uint16_t ccc;         //stav parseru

    uint32_t us_err_cnt;  //stav parseru
    uint32_t len_err_cnt; //stav parseru
    uint32_t cc_err_cnt;  //stav parseru

    uint32_t pck_cnt;     //stav parseru
};

#endif // SSP_T_H
