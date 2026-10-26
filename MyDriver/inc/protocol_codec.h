#ifndef PROTOCOL_CODEC_H
#define PROTOCOL_CODEC_H

unsigned char Protocol_ChecksumByte(const unsigned char *data,
                                    unsigned short length);
unsigned char Protocol_IsChecksumValid(const unsigned char *frame,
                                       unsigned short length);
void Protocol_EncodeLengthError(unsigned char *frame);
unsigned short Protocol_EncodeLiveDataYY(unsigned char *frame,
                                         int o2_output,
                                         int flow_output,
                                         int o2_instant,
                                         int temperature_ntc);
unsigned short Protocol_EncodeLiveDataMT(unsigned char *frame,
                                         int o2_output,
                                         int flow_output,
                                         int o2_instant,
                                         int temperature_ntc,
                                         float pressure_in,
                                         float pressure_out);

#endif /* PROTOCOL_CODEC_H */
