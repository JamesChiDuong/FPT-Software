/**
  ******************************************************************************
  * @file    fatfs.h
  * @author  Fresher Ngo Chi Duong
  * @brief   This file contains all the functions prototypes for the FAT file systerm.
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INC_FATFS_H_
#define INC_FATFS_H_
#include "HAL.h"
/*******************************************************************************
* Add macros, enumeration types, structure types, inside:
* Definitions
******************************************************************************/
/*******************************************************************************
* Variables
******************************************************************************/
/*
Struct to store bytes in entry of root directory
*/
typedef struct _entry_struct
{
    uint8_t fileName[11];               /* The long file Name of directory, offset: 0x00 - 0x08*/
    uint8_t extension[3];               /* The extension of file name, offset: 0x08 - 0x0A */
    uint8_t atrributes;                 /* The atrribute of directory entry, offset: 0x0B*/
    uint8_t reserved[2];                /* Reserved for use by Windown NT, offset: 0x0C*/
    uint16_t lastModiTime;              /* The last time modified of directory, offset: 0x22 - 0x23*/
    uint16_t lastModiDate;              /* The last date modified of directory, offset: 0x24 - 0x25*/
    uint32_t firstCluster;              /* The first cluster of entry directory: offset: 0x26 - 0x27*/
    uint32_t fileSize;                  /* The file size of directory, offset: 0x28 - 0x31*/
    uint8_t entryDirectory[32];         /* The buffer to store all data of entry directory*/
}fatfs_entry_struct_t;

/*
Struct to store bytes in boot sector
*/
typedef struct _boot_sector_struct
{
    uint16_t bytePerSector;             /* The byte per sector in FAT, offset: 0x0B - 0x0C*/
    uint8_t sectorPerCluster;           /* The sector per cluster in FAT, offset: 0x13  */
    uint16_t numberOfReservedSector;    /* The number or reserverd sector, offset: 0x14 - 0x15*/
    uint8_t numberOfFat;                /* The Number of FAT, offset: 0x16*/
    uint16_t maximumRootDirEntry;       /* The Maximum of number of root directory entries, offset: 0x17 - 0x18*/
    uint16_t sectorPerFat;              /* The number of sector per FAT, offset: 0x22 - 0x24*/
    uint32_t totalSectorCount;          /* The total of sector count for FAT, offset: 0x32 - 0x35*/
    uint32_t volumeID;                  /* The volume ID to FAT detect wrong disk, offset: 0x39 - 0x41*/
    uint8_t fileSytermType[8];          /* The type of FAT system, offset: 0x54 - 0x64*/
}fats_boot_infor_struct_t;

/*
Linked list to store all folder
*/
typedef struct _entry_list
{
    fatfs_entry_struct_t entry;         /* The entry of Directory*/
    struct _entry_list* next;           /* Pointer next in node*/
}fatfs_entry_list_struct_t;

/*
Struct to store bytes relating FAT table or sector of root directory and data area
*/
typedef struct _data_fatfs_struct
{
    uint8_t* FATTable_info;             /* The storage area of FAT table when reading floopy disk*/
    uint32_t sectorOfRootDir;           /* The sector of root directory*/
    uint32_t sectorOfDataArea;          /* The sector of Data Area*/
    uint32_t endOfFile;                 /* The end of file, FAT12 and FAT16: 0xFFF*/
}fatfs_data_struct_t;
/*
Define type of FATTYPE
*/
typedef enum _fattype
{
    FAT12 = 1,                          /* FAT type 12*/
    FAT16 = 2,                          /* FAT type 16*/
    FAT32 = 3                           /* FAT type 32*/
}fatfs_fattpye_enum_t;
/*******************************************************************************
 * API
 ******************************************************************************/


/*
* @brief Initialize information of root directory and boot sector
* @param1 pointer of filePath this is pointer of file name
* @param2 pointer to pointer of headDirectory to store linked list
* @retval true if doing  successfully or false if doing failed
*/
bool fatfs_init(const uint8_t* const filePath, fats_boot_infor_struct_t** const bootInfor, fatfs_entry_list_struct_t** headDir);

/*
*@brief Read directory
* @param1 first cluster which need to access directory
* @param2 pointer of headDirectory to store linked list
* @retval true if doing  successfully or false if doing failed
*/
bool fatfs_readDirectory(const uint32_t firstCluster, fatfs_entry_list_struct_t** headDir);

/*
* @brief Read information of file
* @param1 first cluster which need to access directory
* @param2 pointer to pointer of buffer file to store linked list
* @param3 pointer of head directory of directory linked list
* @retval true if doing  successfully or false if doing failed
*/
uint32_t fatfs_readFile(const uint32_t firstCluster, uint8_t** bufferFileTemp, fatfs_entry_list_struct_t* headDir);

/*
* @brief Deinitialize all pointer point to data memory
* @param1 none
* @retval none
*/
void fatfs_deinit(void);
#endif/* INC_FATFS_H_*/
