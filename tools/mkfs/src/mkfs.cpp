#include <iostream>
#include <fstream>
#include <fs.h>
#include "fat.h"

void printUsage()
{
    std::cout << "Usage: mkfs [options] target" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -p\t\t\tPartition number address (i.e. 1, 2, etc.)" << std::endl;
    std::cout << "  -t\t\t\tPartition type (i.e. vfat, ext4, g2fs)" << std::endl;
    std::cout << "  -F\t\t\tFAT size (i.e. 12, 16, 32)" << std::endl;
}

bool makeFileSystem(std::fstream &diskImage, uint64_t partitionStartingLogicalBlockAddress, std::string partitionType, uint8_t fatSize, uint32_t totalSectors)
{
    // switch on partition type
    if (strcmp(partitionType.c_str(), "vfat") == 0)
    {
        // make FAT file system
        if (!FAT::makeFileSystem(diskImage, fatSize, partitionStartingLogicalBlockAddress, totalSectors))
        {
            return false;
        }
    }
    else if (strcmp(partitionType.c_str(), "ext4") == 0)
    {
        // TODO: make ext4 file system
        return true;
    }
    else if (strcmp(partitionType.c_str(), "g2fs") == 0)
    {
        // TODO: make g2fs file system
        return true;
    }
    else
    {
        // error
        std::cout << "Error: partition type must be \"vfat\", \"ext4\", or \"g2fs\"" << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 8)
    {
        printUsage();
        return EXIT_FAILURE;
    }

    // iterate thru args
    std::string diskImageName;
    uint16_t partitionNumber = 1;
    std::string partitionType = "vfat";
    uint8_t fatSize = 32;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-p") == 0)
        {
            partitionNumber = std::stoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            partitionType = argv[i + 1];
        }
        else if (strcmp(argv[i], "-F") == 0)
        {
            fatSize = std::stoi(argv[i + 1]);
        }
        else
        {
            diskImageName = argv[i];
        }
    }

    std::fstream diskImage(diskImageName, std::ios::in | std::ios::out | std::ios::binary);
    if (!diskImage.is_open())
    {
        std::cout << "Error: \"" << diskImageName << "\" does not exist" << std::endl;
        return EXIT_FAILURE;
    }

    // if partitionStartingLogicalBlockAddress is < 0 or > diskImage size, error
    diskImage.seekg(0, std::ios::end);
    uint64_t diskImageSize = diskImage.tellg() / BLOCK_SIZE;

    // if partitionType is not "vfat", "ext4", or "g2fs", error
    if (strcmp(partitionType.c_str(), "vfat") != 0 && strcmp(partitionType.c_str(), "ext4") != 0 && strcmp(partitionType.c_str(), "g2fs") != 0)
    {
        std::cout << "Error: partition type must be \"vfat\", \"ext4\", or \"g2fs\"" << std::endl;
        return EXIT_FAILURE;
    }

    if (strcmp(partitionType.c_str(), "vfat") == 0 && (fatSize != 12 && fatSize != 16 && fatSize != 32))
    {
        std::cout << "Error: FAT size must be 12, 16, or 32" << std::endl;
        return EXIT_FAILURE;
    }

    // read PMBR
    diskImage.seekg(0, std::ios::beg);
    MBR pmbr;
    if (!diskImage.read(reinterpret_cast<char *>(&pmbr), sizeof(pmbr)))
    {
        std::cout << "Error: failed to read PMBR" << std::endl;
        return EXIT_FAILURE;
    }

    // make sure signature matches
    if (pmbr.signature != 0xAA55)
    {
        std::cout << "Error: invalid PMBR signature" << std::endl;
        return EXIT_FAILURE;
    }

    // make sure partition 0 type is OSTYPE_PMBR
    if (pmbr.partitions[0].type != OSTYPE_PMBR)
    {
        std::cout << "Error: invalid partition 0 type" << std::endl;
        return EXIT_FAILURE;
    }

    // make sure firstLogicalBlockAddress in partition 0 is 1
    if (pmbr.partitions[0].firstLogicalBlockAddress != 1)
    {
        std::cout << "Error: invalid first logical block address in partition 0 of protected master boot record" << std::endl;
        return EXIT_FAILURE;
    }

    // read GPT header
    diskImage.seekg(1 * BLOCK_SIZE, std::ios::beg);
    GPT_HEADER gpt_header;
    if (!diskImage.read(reinterpret_cast<char *>(&gpt_header), sizeof(gpt_header)))
    {
        std::cout << "Error: failed to read GPT header" << std::endl;
        return EXIT_FAILURE;
    }

    // make sure GTP header signature is GPT_SIGNATURE
    if (gpt_header.signature != GPT_SIGNATURE)
    {
        std::cout << "Error: invalid GPT header signature" << std::endl;
        return EXIT_FAILURE;
    }

    // make sure partitionNumber is >= 0 and <= number of partition entries
    if (partitionNumber < 1 || partitionNumber > gpt_header.numberOfPartitionEntries)
    {
        std::cout << "Error: partition number must be between 1 and " << gpt_header.numberOfPartitionEntries << std::endl;
        return EXIT_FAILURE;
    }

    // seek to start of partition table entry
    if (!diskImage.seekg((gpt_header.partitionTableLogicalBlockAddress * BLOCK_SIZE) + ((partitionNumber - 1) * sizeof(GPT_PARTITION_ENTRY)), std::ios::beg))
    {
        std::cout << "Error: failed to seek to start of partition table" << std::endl;
        return EXIT_FAILURE;
    }

    // read partition table entry
    GPT_PARTITION_ENTRY partitionEntry;
    if (!diskImage.read(reinterpret_cast<char *>(&partitionEntry), sizeof(partitionEntry)))
    {
        std::cout << "Error: failed to read partition table entry" << std::endl;
        return EXIT_FAILURE;
    }

    uint64_t partitionStartingLogicalBlockAddress = partitionEntry.firstLogicalBlockAddress;
    if (partitionStartingLogicalBlockAddress < 2048 || partitionStartingLogicalBlockAddress > diskImageSize)
    {
        std::cout << "Error: partition starting block must be between 2048 and " << diskImageSize << std::endl;
        return EXIT_FAILURE;
    }

    uint64_t partitionEndingLogicalBlockAddress = partitionEntry.lastLogicalBlockAddress;
    // make sure partitionEndingLogicalBlockAddress is > partitionStartingLogicalBlockAddress and <= diskImageSize
    if (partitionEndingLogicalBlockAddress <= partitionStartingLogicalBlockAddress || partitionEndingLogicalBlockAddress > diskImageSize)
    {
        std::cout << "Error: partition ending block must be between " << partitionStartingLogicalBlockAddress << " and " << diskImageSize << std::endl;
        return EXIT_FAILURE;
    }

    uint32_t totalSectors = partitionEndingLogicalBlockAddress - partitionStartingLogicalBlockAddress;
    if (!makeFileSystem(diskImage, partitionStartingLogicalBlockAddress, partitionType, fatSize, totalSectors))
    {
        std::cout << "Error: failed to make file system" << std::endl;
        return EXIT_FAILURE;
    }

    // close file
    diskImage.close();
    if (!diskImage.good())
    {
        std::cerr << "Error: failed to close file" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
