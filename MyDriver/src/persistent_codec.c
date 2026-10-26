#include "persistent_codec.h"

void Persistent_WriteInt16(unsigned char *output, int value)
{
    output[0] = (unsigned char)value;
    output[1] = (unsigned char)(value >> 8);
}

void Persistent_WriteLong32(unsigned char *output, long value)
{
    unsigned long bits;

    bits = (unsigned long)value;
    output[0] = (unsigned char)bits;
    output[1] = (unsigned char)(bits >> 8);
    output[2] = (unsigned char)(bits >> 16);
    output[3] = (unsigned char)(bits >> 24);
}

unsigned short Persistent_ReadUInt16(const unsigned char *input)
{
    unsigned short value;

    value = input[1];
    value = (unsigned short)(value << 8);
    value = (unsigned short)(value | input[0]);
    return value;
}

long Persistent_ReadLong32(const unsigned char *input)
{
    unsigned long value;

    value = input[3];
    value = value << 8;
    value |= input[2];
    value = value << 8;
    value |= input[1];
    value = value << 8;
    value |= input[0];
    return (long)value;
}

int Persistent_NormalizeWarning(unsigned char index, int value)
{
    if ((value <= 999) && (value >= 0))
    {
        return value;
    }
    if (index == 0)
    {
        return 700;
    }
    if (index == 1)
    {
        return 400;
    }
    return 500;
}

unsigned char Persistent_IsO2CalibrationInvalid(int k_o2)
{
    return (unsigned char)((k_o2 > 2000) || (k_o2 < 0));
}
