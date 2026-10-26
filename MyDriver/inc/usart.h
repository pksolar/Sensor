#ifndef __USART_H_
#define __USART_H_

#define RXBUFSSIZE  25

void UsartInit(int baudrate);
void Usart1SendByte(unsigned char ByteData);

extern unsigned char rxBuffer[RXBUFSSIZE];
extern unsigned char rxCount;
extern unsigned char rx232Flag;
#endif
