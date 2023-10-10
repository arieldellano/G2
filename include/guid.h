#ifndef _GUID_H
#define _GUID_H

#include <stdlib.h>

typedef struct _GUID
{
    uint32_t timeLow;
    uint16_t timeMid;
    uint16_t timeHiAndVersion;
    uint8_t clockSeqHiAndReserved;
    uint8_t clockSeqLow;
    uint8_t node[6];
} __attribute__((packed)) GUID;

#endif // _GUID_H