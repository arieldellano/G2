#ifndef __FAT_H
#define __FAT_H

#include <stdint.h>
#include <iostream>

typedef struct _VOLUME_BOOT_RECORD
{
    uint8_t BS_jmpBoot[3];
    uint8_t BS_OEMName[8];
    uint16_t BPB_BytsPerSec;
    uint8_t BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;
    uint32_t BPB_FATSz32;
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    uint8_t BPB_Reserved[12];
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig;
    uint32_t BS_VolID;
    uint8_t BS_VolLab[11];
    uint8_t BS_FilSysType[8];
    uint8_t padding[420];
    uint16_t signature;
} __attribute__((packed)) VOLUME_BOOT_RECORD;

typedef struct _FS_INFO
{
    uint32_t FSI_LeadSig;
    uint8_t FSI_Reserved1[480];
    uint32_t FSI_StrucSig;
    uint32_t FSI_FreeCount;
    uint32_t FSI_NxtFree;
    uint8_t FSI_Reserved2[12];
    uint32_t FSI_TrailSig;
} __attribute__((packed)) FS_INFO;

typedef struct _FAT32_DIRECTORY_ENTRY
{
    uint8_t DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
} __attribute__((packed)) FAT32_DIRECTORY_ENTRY;

typedef enum {
    ATTR_READ_ONLY = 0x01,
    ATTR_HIDDEN = 0x02,
    ATTR_SYSTEM = 0x04,
    ATTR_VOLUME_ID = 0x08,
    ATTR_DIRECTORY = 0x10,
    ATTR_ARCHIVE = 0x20,
    ATTR_LONG_NAME = 0x0F
    

} FAT32_DIR_ATTR;


class FAT
{
public:
    static bool makeFileSystem(std::fstream &diskImage, uint8_t fatSize, uint64_t partitionStartingLogicalBlockAddress, uint32_t totalSectors);

private:
    static bool writeVolumeBootRecord(std::fstream &diskImage, uint8_t fatSize, uint32_t totalSectors, VOLUME_BOOT_RECORD &vbr);
    static bool writeFSInfo(std::fstream &diskImage);
    static bool writeFATs(std::fstream &diskImage, uint64_t partitionStartingLogicalBlockAddress, const VOLUME_BOOT_RECORD &vbr);
    static bool writeFileDirectoryEntries(std::fstream &diskImage, uint64_t partitionStartingLogicalBlockAddress, const VOLUME_BOOT_RECORD &vbr);
    static void getFATDirEntryTimeAndDate(uint16_t &time, uint16_t &date);
};
#endif // __FAT_H
