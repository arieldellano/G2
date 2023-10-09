#include <iostream>
#include <fstream>
#include <kernel/fs.h>
#include <kernel/fat.h>


/**
 * @brief Pad to a full logical block size with zeros
 * @param  *fp: the file pointer
 * @retval true if successful, false otherwise
 */
bool fillToALogicalBlockSizeWithZeros(std::fstream &outfile)
{
    uint8_t zero[512];
    for (int i = 0; i < (BLOCK_SIZE - sizeof(zero)) / sizeof(zero); i++)
    {
        if (!outfile.write(reinterpret_cast<const char *>(&zero), sizeof(zero)))
        {
            return false;
        }
    }

    return true;
}

bool FAT::writeVolumeBootRecord(std::fstream &diskImage, uint8_t fatSize, uint64_t partitionStartingLogicalBlockAddress, uint32_t totalSectors, VOLUME_BOOT_RECORD &vbr)
{
    vbr = {
        .BS_jmpBoot = {0xEB, 0x3C, 0x90},
        .BS_OEMName = {'T', 'H', 'I', 'S', 'D', 'I', 'S', 'K'},
        .BPB_BytsPerSec = BLOCK_SIZE,
        .BPB_SecPerClus = 1,
        .BPB_RsvdSecCnt = 32,
        .BPB_NumFATs = 2,
        .BPB_RootEntCnt = 0,
        .BPB_TotSec16 = 0,
        .BPB_Media = 0xF8, // hard coded to 0xF8 for fixed disk
        .BPB_FATSz16 = 0,
        .BPB_SecPerTrk = 0,
        .BPB_NumHeads = 0,
        .BPB_HiddSec = 2048 - 1,
        .BPB_TotSec32 = totalSectors,
        .BPB_FATSz32 = (2048 - 32) / 2,
        .BPB_ExtFlags = 0,
        .BPB_FSVer = 0,
        .BPB_RootClus = 2,
        .BPB_FSInfo = 1,
        .BPB_BkBootSec = 6,
        .BPB_Reserved = {0},
        .BS_DrvNum = 0x80, // hard coded to 0x80 for fixed disk
        .BS_Reserved1 = 0,
        .BS_BootSig = 0x29, // hard coded to 0x29
        .BS_VolID = 0,
        .BS_VolLab = {'N', 'O', ' ', 'N', 'A', 'M', 'E', ' ', ' ', ' ', ' '},
        .BS_FilSysType = {'F', 'A', 'T', static_cast<uint8_t>(48 + (fatSize / 10)), static_cast<uint8_t>(48 + (fatSize % 10)), ' ', ' ', ' '},
        .padding = {0},
        .signature = 0xAA55};

    // write VBR to disk image, if error return false
    if (!diskImage.write(reinterpret_cast<const char *>(&vbr), sizeof(vbr)))
    {
        std::cerr << "Error: failed to write volume boot record" << std::endl;
        return false;
    }

    // fill to a full logical block size with zeros
    if (!fillToALogicalBlockSizeWithZeros(diskImage))
    {
        std::cerr << "Error: failed to fill to a full logical block size with zeros" << std::endl;
        return false;
    }

    return true;
}

bool FAT::writeFSInfo(std::fstream &diskImage, uint8_t fatSize, uint64_t partitionStartingLogicalBlockAddress, uint32_t totalSectors)
{
    FS_INFO fsInfo = {
        .FSI_LeadSig = 0x41615252,
        .FSI_Reserved1 = {0},
        .FSI_StrucSig = 0x61417272,
        .FSI_FreeCount = 0xFFFFFFFF,
        .FSI_NxtFree = 0xFFFFFFFF,
        .FSI_Reserved2 = {0},
        .FSI_TrailSig = 0xAA550000};

    // write FSInfo to disk image
    if (!diskImage.write(reinterpret_cast<const char *>(&fsInfo), sizeof(fsInfo)))
    {
        std::cerr << "Error: failed to write FSInfo: " << std::endl;
        return false;
    }

    // fill to a full logical block size with zeros
    if (!fillToALogicalBlockSizeWithZeros(diskImage))
    {
        std::cerr << "Error: failed to fill to a full logical block size with zeros" << std::endl;
        return false;
    }

    return true;
}

bool FAT::writeFATs(std::fstream &diskImage, uint8_t fatSize, uint64_t partitionStartingLogicalBlockAddress, uint32_t totalSectors, const VOLUME_BOOT_RECORD &vbr)
{
    for (uint8_t i = 0; i < vbr.BPB_NumFATs; i++)
    {
        // seek to location of FAT [i]
        if (!diskImage.seekp((partitionStartingLogicalBlockAddress + vbr.BPB_RsvdSecCnt + (i * vbr.BPB_FATSz32)) * BLOCK_SIZE, std::ios::beg))
        {
            return false;
        }

        // write FAT [i] to disk image, if error return false
        // FAT Identifier
        uint32_t fatEntry = 0x0FFFFFFF | vbr.BPB_Media;
        if (!diskImage.write(reinterpret_cast<const char *>(&fatEntry), sizeof(fatEntry)))
        {
            std::cerr << "Error: failed to write FAT" << std::endl;
            return false;
        }

        // Cluster 1
        fatEntry = 0x0FFFFFFF;
        if (!diskImage.write(reinterpret_cast<const char *>(&fatEntry), sizeof(fatEntry)))
        {
            std::cerr << "Error: failed to write FAT" << std::endl;
            return false;
        }

        // Cluster 2; Root Dir '/'
        fatEntry = 0x0FFFFFFF | vbr.BPB_Media;
        if (!diskImage.write(reinterpret_cast<const char *>(&fatEntry), sizeof(fatEntry)))
        {
            std::cerr << "Error: failed to write FAT" << std::endl;
            return false;
        }

        // Cluster 3; 'EFI' directory
        fatEntry = 0x0FFFFFFF | vbr.BPB_Media;
        if (!diskImage.write(reinterpret_cast<const char *>(&fatEntry), sizeof(fatEntry)))
        {
            std::cerr << "Error: failed to write FAT" << std::endl;
            return false;
        }
    }

    return true;
}

bool FAT::writeFileDirectoryEntries(std::fstream &diskImage, uint8_t fatSize, uint64_t partitionStartingLogicalBlockAddress, uint32_t totalSectors, const VOLUME_BOOT_RECORD &vbr)
{
    // compute starting logical block address of FAT
    uint32_t fatStartingLogicalBlockAddress = partitionStartingLogicalBlockAddress + vbr.BPB_RsvdSecCnt;

    // compute starting logical blook address of data region
    uint32_t dataStartingLogicalBlockAddress = fatStartingLogicalBlockAddress + (vbr.BPB_NumFATs * vbr.BPB_FATSz32);

    // seek to starting logical blook address of data region
    if (!diskImage.seekp(dataStartingLogicalBlockAddress * BLOCK_SIZE, std::ios::beg))
    {
        std::cerr << "Error: failed to seek to starting logical blook address of data region" << std::endl;
        return false;
    }

    // "/EFI" directory entry
    FAT32_DIRECTORY_ENTRY dirEntry = {
        .DIR_Name = {'E', 'F', 'I', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        .DIR_Attr = ATTR_DIRECTORY,
        .DIR_NTRes = 0,
        .DIR_CrtTimeTenth = 0,
        .DIR_CrtTime = 0,
        .DIR_CrtDate = 0,
        .DIR_LstAccDate = 0,
        .DIR_FstClusHI = 0,
        .DIR_WrtTime = 0,
        .DIR_WrtDate = 0,
        .DIR_FstClusLO = 3,
        .DIR_FileSize = 0};

    uint16_t time, date;
    getFATDirEntryTimeAndDate(time, date);
    dirEntry.DIR_CrtTime = time;
    dirEntry.DIR_CrtDate = date;
    dirEntry.DIR_WrtTime = time;
    dirEntry.DIR_WrtDate = date;

    // write dir entry to disk image
    if (!diskImage.write(reinterpret_cast<const char *>(&dirEntry), sizeof(dirEntry)))
    {
        std::cerr << "Error: failed to write dir entry" << std::endl;
        return false;
    }

    // 'EFI' directory contents
    if (!diskImage.seekp((dataStartingLogicalBlockAddress + 1) * BLOCK_SIZE, std::ios::beg))
    {
        std::cerr << "Error: failed to seek to EFI directory contents" << std::endl;
        return false;
    }

    memcpy(dirEntry.DIR_Name, ".           ", 11);
    if (!diskImage.write(reinterpret_cast<const char *>(&dirEntry), sizeof(dirEntry)))
    {
        std::cerr << "Error: failed to write dir entry" << std::endl;
        return false;
    }

    // root directory doesn't have a cluster value
    dirEntry.DIR_FstClusLO = 0;
    memcpy(dirEntry.DIR_Name, "..          ", 11);
    if (!diskImage.write(reinterpret_cast<const char *>(&dirEntry), sizeof(dirEntry)))
    {
        std::cerr << "Error: failed to write dir entry" << std::endl;
        return false;
    }

    return true;
}

void FAT::getFATDirEntryTimeAndDate(uint16_t &time, uint16_t &date)
{
    std::time_t now = std::time(nullptr);
    std::tm tm = *std::localtime(&now);

    // seconds is # of 2 second increments (0..29)
    if (tm.tm_sec == 60)
    {
        tm.tm_sec = 59;
    }

    time = ((tm.tm_hour & 0x1F) << 11) | ((tm.tm_min & 0x3F) << 5) | ((tm.tm_sec / 2) & 0x1F);
    date = ((tm.tm_year - 80) << 9) | ((tm.tm_mon + 1) << 5) | (tm.tm_mday & 0x1F);
}

bool FAT::makeFileSystem(std::fstream &diskImage, uint8_t fatSize, uint64_t partitionStartingLogicalBlockAddress, uint32_t totalSectors)
{
    // seek to partition starting block
    if (!diskImage.seekp(partitionStartingLogicalBlockAddress * BLOCK_SIZE, std::ios::beg))
    {
        return false;
    }

    // write MBR to disk image
    VOLUME_BOOT_RECORD vbr;
    if (!writeVolumeBootRecord(diskImage, fatSize, partitionStartingLogicalBlockAddress, totalSectors, vbr))
    {
        std::cerr << "Error: failed to write volume boot record" << std::endl;
        return false;
    }

    // write FSInfo to disk image
    if (!writeFSInfo(diskImage, fatSize, partitionStartingLogicalBlockAddress, totalSectors))
    {
        std::cerr << "Error: failed to write FSInfo" << std::endl;
        return false;
    }

    // seek to location of backup VBR
    if (!diskImage.seekp((partitionStartingLogicalBlockAddress + vbr.BPB_BkBootSec) * BLOCK_SIZE, std::ios::beg))
    {
        std::cerr << "Error: failed to seek to location of backup VBR" << std::endl;
        return false;
    }

    // write backup VBR to disk image
    if (!writeVolumeBootRecord(diskImage, fatSize, partitionStartingLogicalBlockAddress, totalSectors, vbr))
    {
        std::cerr << "Error: failed to write volume boot record" << std::endl;
        return false;
    }

    // write backup FSInfo to disk image
    if (!writeFSInfo(diskImage, fatSize, partitionStartingLogicalBlockAddress, totalSectors))
    {
        std::cerr << "Error: failed to write FSInfo" << std::endl;
        return false;
    }

    // write FATs to disk image
    if (!writeFATs(diskImage, fatSize, partitionStartingLogicalBlockAddress, totalSectors, vbr))
    {
        std::cerr << "Error: failed to write FATs" << std::endl;
        return false;
    }

    // TODO: remove once create directory/file works

    // // Write File/Directory Entries
    // if (!writeFileDirectoryEntries(diskImage, fatSize, partitionStartingLogicalBlockAddress, totalSectors, vbr))
    // {
    //     std::cerr << "Error: failed to write file/directory entries" << std::endl;
    //     return false;
    // }

    return true;
}
