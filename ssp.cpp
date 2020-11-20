#include "ssp.h"

ssp_t::ssp_t(QObject *parent) : QObject(parent)
{
    size = 4096;
    buff = new uint8_t[size];

    write = buff;
    magic = 0;
    state = SPP_STATE_WAIT;
    len = 0;

    us_err_cnt = 0;
    len_err_cnt = 0;
    cc_err_cnt = 0;

    pck_cnt = 0;
}

ssp_t::~ssp_t()
{
    delete buff;
}

int ssp_t::send_packet(uint8_t *data, uint8_t len)
{
    uint16_t i, cc;
    uint8_t byte;
    uint8_t *buff, *buff_p;

    if(4096 < len + 1 + 2 + 2) return 0;

    buff_p = buff = packet_buff;

    *buff++ = SPP_MAGIC;

    *buff++ = len & 0xFF;
    *buff++ = len >> 8;

    cc = 0;

    for(i = 0; i < len; i++)
    {
      byte = *data++;

      cc += byte;
      *buff++ = byte;
      if(byte == SPP_MAGIC) *buff++ = byte;

      if(4096 < buff - buff_p + 2) return 0;
    }

    byte = cc & 0xFF;
    *buff++ = byte;
    if(byte == SPP_MAGIC) *buff++ = byte;

    byte = cc >> 8;
    *buff++ = byte;
    if(byte == SPP_MAGIC) *buff++ = byte;

    emit write_data(buff_p, buff - buff_p);
    return buff - buff_p;
}

void ssp_t::read_data(uint8_t *data, uint16_t len)
{
    int i;

    for(i = 0; i < len; i++)
    {
       rx_byte(data[i]);
    }
}

int ssp_t::rx_byte(uint8_t b)
{
    if(magic == 1)
    {
      magic = 0;

      if(b != SPP_MAGIC)
      {
        write = buff;
        if(state != SPP_STATE_WAIT) us_err_cnt++;

        state = SPP_STATE_LEN2;
        len = b;

        cc = 0;
        ccc = 0;

        return 0;
      }
    }
    else
    {
      if(b == SPP_MAGIC)
      {
        magic = 1;
        return 0;
      }
    }

    switch(state)
    {
      case SPP_STATE_LEN2:
        len += (uint16_t)b << 8;
        state = SPP_STATE_DATA;
        break;

      case SPP_STATE_DATA:
        *write++ = b;
        ccc += b;

        if(write - buff >= len)
        {
          state = SPP_STATE_CC1;
        }
        else if(write - buff >= size)
        {
          len_err_cnt++;
          state = SPP_STATE_WAIT;
          return 0;
        }
        break;

      case SPP_STATE_CC1:
        cc = b;
        state = SPP_STATE_CC2;
        break;

      case SPP_STATE_CC2:
        cc += (uint16_t)b << 8;

        if(cc == ccc)
        {
          pck_cnt++;
          state = SPP_STATE_WAIT;

          emit read_packet(buff, len);
          return 1;
        }
        else
        {
          cc_err_cnt++;
        }

        state = SPP_STATE_WAIT;
        break;

      default:
        // WAITING....
        break;
    }

    return 0;
}
