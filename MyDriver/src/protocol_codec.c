#include "protocol_codec.h"

#define PROTOCOL_ACK                 0x16
#define PROTOCOL_LIVE_COMMAND        0x01
#define PROTOCOL_O2_MIN              205
#define PROTOCOL_O2_MAX              956
#define PROTOCOL_AIR_O2              0x00D2

static void Protocol_WriteBe16(unsigned char *output, int value)
{
    output[0] = (unsigned char)(value >> 8);
    output[1] = (unsigned char)(value & 0xff);
}

unsigned char Protocol_ChecksumByte(const unsigned char *data,
                                    unsigned short length)
{
    unsigned char checksum;
    unsigned short index;

    checksum = 0;
    for (index = 0; index < length; index++)
    {
        checksum += data[index];
    }
    return (unsigned char)(0 - checksum);
}

unsigned char Protocol_IsChecksumValid(const unsigned char *frame,
                                       unsigned short length)
{
    unsigned char checksum;
    unsigned short index;

    checksum = 0;
    for (index = 0; index < length; index++)
    {
        checksum += frame[index];
    }
    return (unsigned char)(checksum == 0);
}

void Protocol_EncodeLengthError(unsigned char *frame)
{
    frame[0] = 0x06;
    frame[1] = 2;
    frame[3] = 0x01;
}

static int Protocol_ClampO2(int value)
{
    if (value < PROTOCOL_O2_MIN)
    {
        value = PROTOCOL_O2_MIN;
    }
    else if (value > PROTOCOL_O2_MAX)
    {
        value = PROTOCOL_O2_MAX;
    }
    return value;
}

static int Protocol_ClampFlow(int value)
{
    if (value < 0)
    {
        value = 0;
    }
    return value;
}

static void Protocol_ApplyLowFlowAirRule(unsigned char *frame,
                                         int flow_output,
                                         int o2_instant)
{
    if ((flow_output < 2) && (o2_instant <= 240))
    {
        Protocol_WriteBe16(&frame[3], PROTOCOL_AIR_O2);
    }
}

unsigned short Protocol_EncodeLiveDataYY(unsigned char *frame,
                                         int o2_output,
                                         int flow_output,
                                         int o2_instant,
                                         int temperature_ntc)
{
    int temperature;

    frame[0] = PROTOCOL_ACK;
    frame[1] = 9;
    frame[2] = PROTOCOL_LIVE_COMMAND;

    o2_output = Protocol_ClampO2(o2_output);
    Protocol_WriteBe16(&frame[3], o2_output);

    flow_output = Protocol_ClampFlow(flow_output);
    Protocol_ApplyLowFlowAirRule(frame, flow_output, o2_instant);
    Protocol_WriteBe16(&frame[5], flow_output);

    temperature = (int)((temperature_ntc / 10.0) + 0.5);
    Protocol_WriteBe16(&frame[7], temperature);
    Protocol_WriteBe16(&frame[9], 0);
    return 11;
}

unsigned short Protocol_EncodeLiveDataMT(unsigned char *frame,
                                         int o2_output,
                                         int flow_output,
                                         int o2_instant,
                                         int temperature_ntc,
                                         float pressure_in,
                                         float pressure_out)
{
    int temperature;

    frame[0] = PROTOCOL_ACK;
    frame[1] = 11;
    frame[2] = PROTOCOL_LIVE_COMMAND;

    o2_output = Protocol_ClampO2(o2_output);
    Protocol_WriteBe16(&frame[3], o2_output);

    flow_output = Protocol_ClampFlow(flow_output / 10);
    Protocol_ApplyLowFlowAirRule(frame, flow_output, o2_instant);
    Protocol_WriteBe16(&frame[5], flow_output);

    temperature = (int)((temperature_ntc / 10.0) + 0.5);
    Protocol_WriteBe16(&frame[7], temperature);
    Protocol_WriteBe16(&frame[9], (int)(pressure_in * 10));
    Protocol_WriteBe16(&frame[11], (int)(pressure_out * 10));
    return 13;
}
