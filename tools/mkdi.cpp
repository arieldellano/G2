#include <iostream>
#include <fstream> 
#include <kernel/fs.h>
#include <kernel/guid.h>
#include <cuchar>

uint64_t imageSizeInBytes = 0, espSizeInBytes = 0, espSizeInLogicalBlocks = 0, espStartingLogicalBlockAddress = 0, espEndingLBA = 0;
uint64_t dataSizeInBytes = 0, dataSizeInLogicalBlocks = 0, dataStartingLogicalBlockAddress = 0, dataEndingLBA = 0;

/**
 * CRC32 lookup table
 */
static uint32_t crc32LookupTable[256] = {
    0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,
    0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
    0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,
    0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
    0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,
    0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
    0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
    0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
    0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,
    0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
    0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,
    0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
    0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,
    0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
    0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
    0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
    0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,
    0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
    0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,
    0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
    0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
    0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
    0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,
    0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
    0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,
    0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
    0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,
    0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
    0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,
    0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
    0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,
    0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D,
};

/**
 * @brief Calcuate CRC32 checksum of data buffer
 * @note Taken from https://create.stephan-brumme.com/crc32/
 * @param  *data: the data buffer
 * @param  length: the length of the data buffer
 * @param  previousCrc32: the previous CRC32 checksum
 * @retval The CRC32 checksum of the data buffer
*/
uint32_t crc32(const void* data, size_t length, uint32_t previousCrc32 = 0)
{
  uint32_t crc = ~previousCrc32;
  uint8_t* current = (uint8_t*) data;
  
  while (length--)
    crc = (crc >> 8) ^ crc32LookupTable[(crc & 0xFF) ^ *current++];
  
  return ~crc;
}

/**
 * @brief Generate a new GUID
 * @retval A new GUID
 */
GUID newGuid()
{
    uint8_t randomBytes[16] = { 0 };

    for(uint8_t i = 0; i < sizeof(randomBytes); i++) {
        randomBytes[i] = rand() % (UINT8_MAX + 1);
    }

    GUID result = {
        .timeLow = *(uint32_t *)&randomBytes[0],
        .timeMid = *(uint16_t *)&randomBytes[4],
        .timeHiAndVersion = *(uint16_t *)&randomBytes[6],
        .clockSeqHiAndReserved = *(uint8_t *)&randomBytes[8],
        .clockSeqLow = *(uint8_t *)&randomBytes[9],
        .node = {
            randomBytes[10], randomBytes[11], randomBytes[12],
            randomBytes[13], randomBytes[14], randomBytes[15]
        }
    };

    result.timeHiAndVersion &= ~(1 << 15);
    result.timeHiAndVersion |= (1 << 14);
    result.timeHiAndVersion &= ~(1 << 13);
    result.timeHiAndVersion &= ~(1 << 12);

    result.clockSeqHiAndReserved |= ~(1 << 7);
    result.clockSeqHiAndReserved |= ~(1 << 6);
    result.clockSeqHiAndReserved &= ~(1 << 5);

    return result;
}

/**
 * @brief Convert a logical block address to cylinder, head, and sector geometry
 * @note Taken from https://wiki.osdev.org/ATA_PIO_Mode#LBA_to_CHS_conversion
 * @param  logicalBlockAddress: the logical block address
 * @param  geometry[3]: the cylinder, head, and sector geometry 
 * @retval None
 */
void convertLBAtoCHS(const uint32_t logicalBlockAddress, uint8_t (&geometry)[3])
{
    uint32_t cylinder = logicalBlockAddress / (2 * 18);
    uint32_t head = (logicalBlockAddress / 18) % 2;
    uint32_t sector = (logicalBlockAddress % 18) + 1;

    geometry[0] = cylinder & 0xff;
    geometry[1] = head & 0xff;
    geometry[2] = sector & 0xff;
}

/**
 * @brief Get the next aligned logical block address
 * @param  logicalBlockAddress: the logical block address
 * @retval The next aligned logical block address
 */
uint64_t getNextAlignedLBA(const uint64_t logicalBlockAddress) {
    return logicalBlockAddress - (logicalBlockAddress % ALIGNMENT_LBA) + ALIGNMENT_LBA;
}

/**
 * @brief Convert bytes to a logical block address
 * @param  bytes: the bytes
 * @retval The logical block address
 */
uint64_t convertBytesToLogicalBlockAddress(const uint64_t bytes) {
    return bytes / BLOCK_SIZE + (bytes % BLOCK_SIZE ? 1 : 0);
}

/**
 * @brief Pad to a full logical block size with zeros
 * @param  *fp: the file pointer
 * @retval true if successful, false otherwise
 */
bool fillToALogicalBlockSizeWithZeros(std::ofstream& outfile)
{
    uint8_t zero[512];
    for (int i = 0; i < (BLOCK_SIZE - sizeof(zero)) / sizeof(zero); i++)
    {
        if (!outfile.write(reinterpret_cast<const char*>(&zero), sizeof(zero)))
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief Write the master boot record to file
 * @param  *fp: the file pointer
 * @retval true if successful, false otherwise
 */
bool writeMasterBootRecord(std::ofstream& outfile) 
{
    uint64_t sizeInLogicalBlocks = convertBytesToLogicalBlockAddress(imageSizeInBytes);
    if (sizeInLogicalBlocks > 0xFFFFFFFF) sizeInLogicalBlocks = 0x100000000;

    MBR mbr = {
        .bootstrap = { 0 },
        .partitions = {
            {
                .status = 0x01,
                .startingGemory = {0x00, 0x02, 0x00},
                .type = OSTYPE_PMBR,
                .endingGeometry = {0xFF, 0xFF, 0xFF},
                .firstLogicalBlockAddress = 1,
                .sizeInLogicalBlocks = (uint32_t)sizeInLogicalBlocks - 1
            },
            { 0 },
            { 0 },
            { 0 }
        },
        .signature = MBR_SIGNATURE,
    };

    if (!outfile.write(reinterpret_cast<const char*>(&mbr), sizeof(mbr))) {
        return false;
    }

    return fillToALogicalBlockSizeWithZeros(outfile);
}

/**
 * @brief Write a global partition table header and partition entry table to file
 * @param  *fp: the file pointer
 * @retval true if successful, false otherwise
 */
bool writeGlobalPartitionTableHeaderAndEntries(std::ofstream& outfile) {
    // fill out primary GPT header
    GPT_HEADER primaryGPTHeader = {
        .signature = GPT_SIGNATURE,
        .revision = GPT_REVISION,
        .headerSize = GPT_HEADER_SIZE,
        .crc32 = 0,
        .reserved = 0,
        .headerLogicalBlockAddress = 1,
        .alternateLogicalBlockAddress = convertBytesToLogicalBlockAddress(imageSizeInBytes) - 1,
        .firstUsableLogicalBlockAddress = 34, // MBR + GPT + Primary GPT table
        .lastUsableLogicalBlockAddress = convertBytesToLogicalBlockAddress(imageSizeInBytes) - 34, 
        .diskIdentifier = newGuid(),
        .partitionTableLogicalBlockAddress = 2,
        .numberOfPartitionEntries = GPT_PARTITION_TABLE_ENTRIES,
        .partitionEntrySize = sizeof(GPT_PARTITION_ENTRY),
        .partitionTableCrc32 = 0,
        .reserved2 = { 0 }
    };

    // fill out GPT partition table
    GPT_PARTITION_ENTRY partitions[GPT_PARTITION_TABLE_ENTRIES] = {
        {
            .partitionType = ESP_GUID,
            .uniqueIdentifier = newGuid(),
            .firstLogicalBlockAddress = espStartingLogicalBlockAddress,
            .lastLogicalBlockAddress = espStartingLogicalBlockAddress + espSizeInLogicalBlocks,
            .flags = 0,
            .name = u"EFI System Partition"
        },
        {
            .partitionType = FAT32_GUID,
            .uniqueIdentifier = newGuid(),
            .firstLogicalBlockAddress = dataStartingLogicalBlockAddress,
            .lastLogicalBlockAddress = dataStartingLogicalBlockAddress + dataSizeInLogicalBlocks,
            .flags = 0,
            .name = u"Root Partition"
        }
     };

    // fill out primary GPT header partition table CRC32
    primaryGPTHeader.partitionTableCrc32 = crc32(partitions, sizeof(partitions));

    // fill out primary GPT header CRC32
    primaryGPTHeader.crc32 = crc32(&primaryGPTHeader, primaryGPTHeader.headerSize);

    // write primary GPT header to file
    if (!outfile.write(reinterpret_cast<const char*>(&primaryGPTHeader), sizeof(primaryGPTHeader))) {
        return false;
    }

    // pad to a full logical block size
    if (!fillToALogicalBlockSizeWithZeros(outfile)) {
        return false;
    }

    // write primary GPT partition table to file
    if (!outfile.write(reinterpret_cast<const char*>(partitions), sizeof(partitions))) {
        return false;
    }

    // fill out the secondary GPT header
    GPT_HEADER secondaryGPTHeader = primaryGPTHeader;
    secondaryGPTHeader.crc32 = 0;
    secondaryGPTHeader.partitionTableCrc32 = primaryGPTHeader.partitionTableCrc32;
    secondaryGPTHeader.headerLogicalBlockAddress = primaryGPTHeader.alternateLogicalBlockAddress;
    secondaryGPTHeader.alternateLogicalBlockAddress = primaryGPTHeader.headerLogicalBlockAddress;
    secondaryGPTHeader.partitionTableLogicalBlockAddress = convertBytesToLogicalBlockAddress(imageSizeInBytes) - 33;

    // fill out the secondary GPT header CRC32
    secondaryGPTHeader.crc32 = crc32(&secondaryGPTHeader, secondaryGPTHeader.headerSize);

    // seek to position of secondary GPT partition table
    if (!outfile.seekp(secondaryGPTHeader.partitionTableLogicalBlockAddress * BLOCK_SIZE)) {
        return false;
    }

    // write secondary GPT partition table to file
    if (!outfile.write(reinterpret_cast<const char*>(partitions), sizeof(partitions))) {
        return false;
    }
    
    // write secondary GPT header to file
    if (!outfile.write(reinterpret_cast<const char*>(&secondaryGPTHeader), sizeof(secondaryGPTHeader))) {
        return false;
    }

     // pad to a full logical block size
    return fillToALogicalBlockSizeWithZeros(outfile);
}

/**
 * @brief Main entry point
 * @param  argc: the number of arguments
 * @param  argv: the arguments
 * @retval EXIT_SUCCESS if successful, EXIT_FAILURE otherwise
 */
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: mkfs <image file name>" << std::endl;
        return EXIT_FAILURE;
    }

    espSizeInBytes = (1024 * 1024 * 100);
    espSizeInLogicalBlocks = convertBytesToLogicalBlockAddress(espSizeInBytes) - 1;
    espStartingLogicalBlockAddress = ALIGNMENT_LBA;
    
    dataSizeInBytes = (1024 * 1024 * 360),
    dataStartingLogicalBlockAddress = getNextAlignedLBA(espStartingLogicalBlockAddress + espSizeInLogicalBlocks);
    dataSizeInLogicalBlocks = getNextAlignedLBA(convertBytesToLogicalBlockAddress(dataSizeInBytes)) - 1;

    imageSizeInBytes = (
        (BLOCK_SIZE * 1) +                       // Protected Master Boot Record
        (BLOCK_SIZE * 1) +                       // Primary GPT Header
        (BLOCK_SIZE * 32) +                      // Primary GPT Partition Table
        (espSizeInLogicalBlocks * BLOCK_SIZE) +  // ESP partition
        (dataSizeInLogicalBlocks * BLOCK_SIZE) + // Data partition
        (GPT_PARTITION_ALIGNMENT * 2) +          // Padding
        (BLOCK_SIZE * 32) +                      // Secondary GPT Partition Table
        (BLOCK_SIZE * 1)                         // Secondary GPT Header
    );

    srand(time(NULL));

    // open file for writing
    std::ofstream outfile(argv[1], std::ios::binary);
    if (!outfile)
    {
        std::cerr << "Error: could not open file " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    // write master boot record to file
    if (!writeMasterBootRecord(outfile)) {
        outfile.close();
        std::cerr << "Error: could not write MBR to file " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    // write global partition table header and entries to file
    if (!writeGlobalPartitionTableHeaderAndEntries(outfile)) {
        outfile.close();
        std::cerr << "Error: could not write GTP header/tables" << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    // close file
    outfile.close();
    if (!outfile.good())
    {
        std::cerr << "Error: could not close file " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}