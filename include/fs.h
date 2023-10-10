#ifndef _FS_H
#define _FS_H

#include <stdint.h>
#include <cuchar>
#include "guid.h"

#define MBR_SIGNATURE 0xAA55
#define GPT_HEADER_SIZE 92
#define GPT_SIGNATURE 0x5452415020494645
#define GPT_REVISION 0x00010000
#define OSTYPE_UEFI 0xEF
#define OSTYPE_PMBR 0xEE
#define BLOCK_SIZE 512
#define GPT_PARTITION_ENTRY_SIZE 128
#define GPT_PARTITION_TABLE_MIN_SIZE 16384
#define GPT_PARTITION_ALIGNMENT 1048576
#define GPT_PARTITION_TABLE_ENTRIES 128
#define ALIGNMENT_LBA (GPT_PARTITION_ALIGNMENT / BLOCK_SIZE)

const GUID ESP_GUID = {0xC12A7328, 0xF81F, 0x11D2, 0xBA, 0x4B, {0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B}};
const GUID FAT32_GUID = {0xEBD0A0A2, 0xB9E5, 0x4433, 0x87, 0xC0, {0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7}};

// MBR partition entry
typedef struct _MBR_PARTITION_ENTRY
{
    uint8_t status;              // 0x80 - active partition
    uint8_t startingGeometry[3]; // first cylinder-head-sector address
    uint8_t type;                // partition type
    uint8_t endingGeometry[3];   // last cylinder-head-sector address
    uint32_t firstLogicalBlockAddress;
    uint32_t sizeInLogicalBlocks;
} __attribute__((packed)) MBR_PARTITION_ENTRY;

// MBR header
typedef struct _MBR
{
    uint8_t bootstrap[446];            // bootstrap code
    MBR_PARTITION_ENTRY partitions[4]; // partition entries
    uint16_t signature;                // 0xaa55
} __attribute__((packed)) MBR;

// GPT partition entry
typedef struct _GPT_PARTITION_ENTRY
{
    GUID partitionType;                // partition type GUID
    GUID uniqueIdentifier;             // unique partition GUID
    uint64_t firstLogicalBlockAddress; // first LBA (little endian)
    uint64_t lastLogicalBlockAddress;  // last LBA (inclusive, usually odd)
    uint64_t flags;                    // attribute flags (e.g. bit 60 denotes read-only)
    char16_t name[36];                 // partition name (36 UTF-16LE code units)
} __attribute__((packed)) GPT_PARTITION_ENTRY;

// GPT header
typedef struct _GPT_HEADER
{
    uint64_t signature;                              // 0x5452415020494645 ("EFI PART", little endian)
    uint32_t revision;                               // 0x00000100 for (EFI) GPT
    uint32_t headerSize;                             // size of this header (usually 92 bytes)
    uint32_t crc32;                                  // CRC32 of this header (offset +0 up to headerSize)
    uint32_t reserved;                               // must be zero
    uint64_t headerLogicalBlockAddress;              // logical block address of this header
    uint64_t alternateLogicalBlockAddress;           // logical block address of the alternate header
    uint64_t firstUsableLogicalBlockAddress;         // first logical block address useable by partitions
    uint64_t lastUsableLogicalBlockAddress;          // last logical block address useable by partitions
    GUID diskIdentifier;                             // disk GUID
    uint64_t partitionTableLogicalBlockAddress;      // logical block address of the partition entries
    uint32_t numberOfPartitionEntries;               // number of partition entries
    uint32_t partitionEntrySize;                     // size of a single partition entry
    uint32_t partitionTableCrc32;                    // CRC32 of the partition array
    uint8_t reserved2[BLOCK_SIZE - GPT_HEADER_SIZE]; // must be zero
} __attribute__((packed)) GPT_HEADER;

#endif // _FS_H