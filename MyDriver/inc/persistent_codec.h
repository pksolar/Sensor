#ifndef PERSISTENT_CODEC_H
#define PERSISTENT_CODEC_H

void Persistent_WriteInt16(unsigned char *output, int value);
void Persistent_WriteLong32(unsigned char *output, long value);
unsigned short Persistent_ReadUInt16(const unsigned char *input);
long Persistent_ReadLong32(const unsigned char *input);
int Persistent_NormalizeWarning(unsigned char index, int value);
unsigned char Persistent_IsO2CalibrationInvalid(int k_o2);

#endif /* PERSISTENT_CODEC_H */
