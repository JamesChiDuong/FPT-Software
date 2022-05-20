/*******************************************************************************
* Add public API in header files inside :
* API
******************************************************************************/
#include "fatfs.h"
/*******************************************************************************
* Add macros, enumeration types, structure types, inside:
* Definitions
******************************************************************************/
#define FATFS_SECTOR_OF_BOOT_SECTOR 0U                                                      /* Define sector of boot sector which is alway 0*/
#define FATFS_CONVER_TO_16_BIT(x,y) (uint16_t)(((uint8_t)(x)<< (8U)) | ((uint8_t)(y)))      /* Define macro like function to convert 1 byte to 2 byte(8 bit - 16 bit)*/
#define FATFS_CONVER_TO_32_BIT(x,y) (uint32_t)(((uint16_t)(x)<< (16U)) | ((uint16_t)(y)))   /* Define macrolike function to convert 2 byte to 4 byte(16 bit - 32 bit*/
#define FATFS_PHYSICAL_SECTOR_NUMBER(x) (uint32_t)(33U+((uint32_t)x)-2U)
/*******************************************************************************
* Variables
******************************************************************************/
static fats_boot_infor_struct_t s_boot_info;                                                /* Declare static variable to store information of boot sector*/
static fatfs_entry_list_struct_t* s_rootDirList = NULL;                                     /* Declare static variable to store informattion of root Directory*/
static fatfs_entry_list_struct_t* s_subDirList = NULL;                                      /* Declare static variable to store information of sub directory*/
static fatfs_fattpye_enum_t s_fattype;                                                      /* Declare static variable to store type of FAT*/
static fatfs_data_struct_t s_fatfsdata;                                                     /* Declare static variable to store different value relating FAT or roof/sub directory*/
/*******************************************************************************
* Add static API prototype in C file inside :
* Prototypes
******************************************************************************/

/*
* @brief Reading entry of Directory with 32bytes
* @param1 Buffer
* @param2 Ordinal number of Entry
* @retval Pointer point value of buffer
*/
static uint8_t* __fatfs_readEntryOfDirectory(uint8_t* const buff, const uint8_t entryNumber);

/*
* @brief Creating node of Folder in linked list
* @param entryDir - entry folder with type entry_struct
* @retval linked list of folder
*/
static fatfs_entry_list_struct_t* __fatfs_createNodeOfFolder(const fatfs_entry_struct_t entryDir);

/*
* @brief Adding node of Folder in tail of linked list
* @param1 Pointer of head in linkedlist
* @param entryDir - entry folder with type entry_struct
* @retval pointer of head
*/
static fatfs_entry_list_struct_t* __fatfs_addTailDirList(fatfs_entry_list_struct_t* headDir, const fatfs_entry_struct_t entryDir);

/*
* @brief Read roof directory
* @param1 firstCluster - this is a first cluster in roof directory
* @param2 pointer of head directory
* @retval true if read successfully and faild if read faild
*/
static bool __fatfs_readRoofDirectory(const uint32_t firstCluster, fatfs_entry_list_struct_t** headDir);

/*
* @brief Read information of FAT Table
* @param1 pointer of FAT Information
* @retval true if read successfully and faild if read faild
*/
static bool __fatfs_readFATTable(uint8_t** const FATTable_info);

/*
* @brief Get the next cluster in FAT Table
* @param1 pointer of FAT Information
* @param1 currentCluster this is a current cluster
* @retval return next cluster
*/
static uint32_t __fatfs_getNextCluster(uint8_t* const FATTable_info, const uint32_t currentCluster);

/*
* @brief Check poperty of entry
* @param1 Variable of entry Directory
* @param2 Pointer to pointer of head Directory
* @retval return true if doing successfully and false if doing failed
*/
static bool __faffs_checkPropertyOfEntry(fatfs_entry_struct_t entryDir, fatfs_entry_list_struct_t** const headDir);
/*******************************************************************************
* Code
******************************************************************************/

/*
* @brief Reading entry of Directory with 32bytes
* @param1 Buffer
* @param2 Ordinal number of Entry
* @retval Pointer point value of buffer
*/
static uint8_t* __fatfs_readEntryOfDirectory(uint8_t* const buff, const uint8_t entryNumber)
{
    /* Declare index*/
    uint64_t i = 0U;
    /* Declare static buffer to store value of buffer*/
    static uint8_t tempBuffer[50] = { 0 };

    /* For loop to store 32 byte of entry directory at entryNumber into tempBuffer*/
    for (i = ((uint64_t)entryNumber * 32U); i < ((uint64_t)entryNumber * 32U + 32U); i++)
    {
        tempBuffer[i - ((uint64_t)entryNumber * 32U)] = buff[i];
    }

    return tempBuffer;
}

/*
* @brief Creating node of Folder in linked list
* @param entryDir - entry folder with type entry_struct
* @retval linked list of folder
*/
static fatfs_entry_list_struct_t* __fatfs_createNodeOfFolder(const fatfs_entry_struct_t entryDir)
{
    /* Allocate memory for pointer temp*/
    fatfs_entry_list_struct_t* temp = (fatfs_entry_list_struct_t*)malloc(sizeof(struct _entry_list));

    if (temp != NULL)
    {
        temp->entry = entryDir;
        temp->next = NULL;
    }
    else
    {
        exit(1);
    }

    return temp;
}

/*
* @brief Adding node of Folder in tail of linked list
* @param1 Pointer of head in linkedlist
* @param entryDir - entry folder with type entry_struct
* @retval pointer of head
*/
static fatfs_entry_list_struct_t* __fatfs_addTailDirList(fatfs_entry_list_struct_t* headDir, const fatfs_entry_struct_t entryDir)
{
    fatfs_entry_list_struct_t* temp = NULL;
    fatfs_entry_list_struct_t* pNode = NULL;

    temp = __fatfs_createNodeOfFolder(entryDir);
    if (headDir == NULL)
    {
        headDir = temp;
    }
    else
    {
        pNode = headDir;
        while (pNode->next != NULL)
        {
            pNode = pNode->next;
        }
        pNode->next = temp;
    }

    return headDir;
}

/*
* @brief Read roof directory
* @param1 firstCluster - this is a first cluster in roof directory
* @param2 pointer of head directory
* @retval true if read successfully and faild if read faild
*/
static bool __fatfs_readRoofDirectory(const uint32_t firstCluster, fatfs_entry_list_struct_t** headDir)
{
    /* Declard variable of entry number in directory entry */
    uint8_t entryNumber = 0U;
    uint8_t* bufferFolderTemp = NULL;
    /* Declare variable size byte of root directory, default: 512 byte*/
    uint16_t rootDirecByteSize = (s_boot_info.bytePerSector * (uint16_t)s_boot_info.sectorPerCluster);
    /* Declare variable temp of head directory*/
    fatfs_entry_struct_t entryDir;
    /* Allocate buffer of folder to store infor of root Directory*/
    bufferFolderTemp = (uint8_t*)malloc(rootDirecByteSize);
    bool result = true;

    /* Read multi sector, with sector per cluster is read by root sector, default = 1*/
    kmc_read_multi_sector(firstCluster, s_boot_info.sectorPerCluster, bufferFolderTemp);
    /* Read each entry, with entry is 32 byte, so we have 16 total entry in sector*/
    for (entryNumber = 0u; entryNumber < 16U; entryNumber++)
    {
        /* Coppy all byte in 1 entry into buffer entryDir.entryDirectory*/
        memcpy(entryDir.entryDirectory, __fatfs_readEntryOfDirectory(bufferFolderTemp, entryNumber), 32U);
        /* Chec properties of entries*/
        result = __faffs_checkPropertyOfEntry(entryDir, headDir);
    }
    /* Assign pointer head for static  variable pointer of root Directory*/
    s_rootDirList = *headDir;
    free(bufferFolderTemp);

    return result;
}

/*
* @brief Read information of FAT Table
* @param1 pointer of FAT Information
* @retval true if read successfully and faild if read faild
*/
static bool __fatfs_readFATTable(uint8_t** const FATTable_info)
{
    bool result = true;
    /* Assign Size of FAT Table, with FAT12 = 512*9   */
    uint16_t FATTableSize = (s_boot_info.bytePerSector * s_boot_info.sectorPerFat);
    /* Declare vairable of index starting of FAT Table*/
    uint32_t startFATIndex = (uint32_t)s_boot_info.numberOfReservedSector;
    /* Declare variable to read number of byte in FAT Table*/
    int32_t numberOfByte = 0U;
    *FATTable_info = (uint8_t*)malloc(FATTableSize);

    /* Read 9 sector of FAT Table*/
    numberOfByte = kmc_read_multi_sector(startFATIndex, s_boot_info.sectorPerFat, *FATTable_info);
    /* Check if number of Byte equal with caculated number of byte in FAT(512*9)*/
    if (numberOfByte == (s_boot_info.bytePerSector * s_boot_info.sectorPerFat))
    {
        result = true;
    }
    else
    {
        result = false;
    }

    return result;
}

/*
* @brief Get the next cluster in FAT Table
* @param1 pointer of FAT Information
* @param1 currentCluster this is a current cluster
* @retval return next cluster
*/
static uint32_t __fatfs_getNextCluster(uint8_t* const FATTable_info, const uint32_t currentCluster)
{
    /* Derclare a variable return next value of current cluster*/
    uint32_t retval = 0U;
    /* Position of element in FAT corresponding with cluster in Data area*/
    uint32_t FATOffset = 0U;

    /* Checking this is a FAT 12 type or FAT16 or FAT32*/
    if (s_fattype == FAT12)
    {
        /* Find postion in FAT equal with current cluster, because each element is 1.5 byte in FAT*/
        FATOffset = (currentCluster * 3U) / 2U;
        /* If current Clust is odd number, it will change */
        if (currentCluster % 2U != 0)
        {
            retval = (uint32_t)(FATFS_CONVER_TO_16_BIT(FATTable_info[FATOffset + 1U], FATTable_info[FATOffset]) >> 4U);
        }
        /* If current Clust is even number, it will change */
        else
        {
            retval = (uint32_t)(FATFS_CONVER_TO_16_BIT(FATTable_info[FATOffset + 1U], FATTable_info[FATOffset]) & 0x0FFF);
        }
    }

    return retval;
}

/*
* @brief Check poperty of entry
* @param1 Variable of entry Directory
* @param2 Pointer to pointer of head Directory
* @retval return true if doing successfully and false if doing failed
*/
static bool __faffs_checkPropertyOfEntry(fatfs_entry_struct_t entryDir, fatfs_entry_list_struct_t** const headDir)
{
    bool result = true;
    /* Assign value of byte at position 11 for arributes*/
    entryDir.atrributes = entryDir.entryDirectory[11];
    /* Check entry is long file name?, I'm haven't time to to this*/

    if (0x0FU == entryDir.atrributes)
    {
        result = false;
    }
    /* Check entry is file name with atriibutes is 0x00, 0x20, 0x28*/
    else if (0x00U == entryDir.atrributes || 0x20U == entryDir.atrributes || 0x28U == entryDir.atrributes)
    {
        memset(entryDir.fileName, 0U, sizeof(entryDir.fileName));
        /* Coppy 8 first byte of entryDir.entryDirectory into file name, it mean short file name is a 8 first byte */
        memcpy(entryDir.fileName, entryDir.entryDirectory, 11U);
        /* Check first byte of file name, if it is a 0x00 it mean emptry entry or 0xE5 it mean file/subdirectory have deleted*/
        if (0x00U == entryDir.fileName[0] || 0xE5U == entryDir.fileName[0])
        {
            return false;
        }
        /* Coppy 3 next byte in to extension*/
        memset(entryDir.extension, 0, sizeof(entryDir.extension));
        memcpy(entryDir.extension, entryDir.entryDirectory + 8, 3);
        /*Convert byte last modified time 16 bit */
        entryDir.lastModiTime = FATFS_CONVER_TO_16_BIT(entryDir.entryDirectory[23], entryDir.entryDirectory[22]);
        entryDir.lastModiDate = FATFS_CONVER_TO_16_BIT(entryDir.entryDirectory[25], entryDir.entryDirectory[24]);
        entryDir.firstCluster = FATFS_CONVER_TO_16_BIT(entryDir.entryDirectory[27], entryDir.entryDirectory[26]);
        entryDir.fileSize = FATFS_CONVER_TO_32_BIT((FATFS_CONVER_TO_16_BIT(entryDir.entryDirectory[31], entryDir.entryDirectory[30])), (FATFS_CONVER_TO_16_BIT(entryDir.entryDirectory[29], entryDir.entryDirectory[28])));
        /* Add value into linked list*/
        *headDir = __fatfs_addTailDirList(*headDir, entryDir);
        //break;
    }
    /* Check entry is folder type*/
    else if (0x10U == entryDir.atrributes)
    {
        memset(entryDir.fileName, 0, sizeof(entryDir.fileName));
        memcpy(entryDir.fileName, entryDir.entryDirectory, 11);
        if (0x2EU == entryDir.fileName[0])
        {
            return false;
        }
        /* Coppy 3 next byte in to extension*/
        memset(entryDir.extension, 0, sizeof(entryDir.extension));
        memcpy(entryDir.extension, entryDir.entryDirectory + 8, 3);
        /*Convert byte last modified time 16 bit */
        entryDir.lastModiTime = FATFS_CONVER_TO_16_BIT(entryDir.entryDirectory[23], entryDir.entryDirectory[22]);
        entryDir.lastModiDate = FATFS_CONVER_TO_16_BIT(entryDir.entryDirectory[25], entryDir.entryDirectory[24]);
        entryDir.firstCluster = FATFS_CONVER_TO_16_BIT(entryDir.entryDirectory[27], entryDir.entryDirectory[26]);
        entryDir.fileSize = FATFS_CONVER_TO_32_BIT((FATFS_CONVER_TO_16_BIT(entryDir.entryDirectory[31], entryDir.entryDirectory[30])), (FATFS_CONVER_TO_16_BIT(entryDir.entryDirectory[29], entryDir.entryDirectory[28])));
        /* Add value into linked list*/
        *headDir = __fatfs_addTailDirList(*headDir, entryDir);
    }

    return true;
}

/*
* @brief Initialize information of root directory and boot sector
* @param1 pointer of filePath this is pointer of file name
* @param2 pointer to pointer of headDirectory to store linked list
* @retval true if doing  successfully or false if doing failed
*/
bool fatfs_init(const uint8_t* const filePath, fats_boot_infor_struct_t** const bootInfor, fatfs_entry_list_struct_t** headDir)
{
    uint8_t tempBuffer[KMC_DEFAULT_NUMBER_OF_BYTE_PER_SECTOR_IN_ROOTDIRECTORY] = { 0 };
    bool result = true;

    if (kmc_init(filePath) == true)
    {
        kmc_read_sector(FATFS_SECTOR_OF_BOOT_SECTOR, tempBuffer);
        s_boot_info.bytePerSector = FATFS_CONVER_TO_16_BIT(tempBuffer[12], tempBuffer[11]);
        s_boot_info.sectorPerCluster = tempBuffer[13];
        s_boot_info.numberOfReservedSector = FATFS_CONVER_TO_16_BIT(tempBuffer[15], tempBuffer[14]);
        s_boot_info.numberOfFat = tempBuffer[16];
        s_boot_info.maximumRootDirEntry = FATFS_CONVER_TO_16_BIT(tempBuffer[18], tempBuffer[17]);
        s_boot_info.totalSectorCount = FATFS_CONVER_TO_16_BIT(tempBuffer[20], tempBuffer[19]);
        s_boot_info.sectorPerFat = FATFS_CONVER_TO_16_BIT(tempBuffer[23], tempBuffer[22]);
        memcpy(s_boot_info.fileSytermType, tempBuffer + 54, 8);
        kmc_update_sector_szie(s_boot_info.bytePerSector);
        if (strstr(s_boot_info.fileSytermType, "FAT12") != NULL)
        {
            s_fattype = FAT12;
        }
        else if (strstr(s_boot_info.fileSytermType, "FAT16") != NULL)
        {
            s_fattype = FAT16;
        }
        else if (strstr(s_boot_info.fileSytermType, "FAT32") != NULL)
        {
            s_fattype = FAT32;
        }
        /* Read FAT table and store in s_FATTable_info*/
        if (__fatfs_readFATTable(&s_fatfsdata.FATTable_info) == true)
        {
            /* Assign value end of file is 0xfff*/
            s_fatfsdata.endOfFile = (uint32_t)0x0FFFU;
        }
        /* determined value sector of Root Directory*/
        s_fatfsdata.sectorOfRootDir = (uint32_t)(s_boot_info.numberOfReservedSector + s_boot_info.sectorPerFat * (uint8_t)s_boot_info.numberOfFat);
        /* determined value sector of Data Area*/
        s_fatfsdata.sectorOfDataArea = (uint32_t)(s_boot_info.numberOfReservedSector + (s_boot_info.sectorPerFat * (uint8_t)s_boot_info.numberOfFat) + (s_boot_info.maximumRootDirEntry / 16U));
        __fatfs_readRoofDirectory(s_fatfsdata.sectorOfRootDir, headDir);
    }
    // kiem tra du lieu dau vao cua ng dung
    else
    {
        result = false;
    }
    *bootInfor = &s_boot_info;

    return result;
}

/*
* @brief Read directory
* @param1 first cluster which need to access directory
* @param2 pointer to pointer of headDirectory to store linked list
* @retval true if doing  successfully or false if doing failed
*/
bool fatfs_readDirectory(const uint32_t firstCluster, fatfs_entry_list_struct_t** headDir)
{
    /* Declare variable size of cluster. In FAT12, size of cluster = size of sector = 512 byte*/
    uint32_t clusterSize = (uint32_t)((uint16_t)s_boot_info.sectorPerCluster * s_boot_info.bytePerSector);
    /* Declare next cluster after checking element of FAT*/
    uint32_t nextCluster = 0U;
    /* Declare current cluster*/
    uint32_t currentCluster = firstCluster;;
    /* Initialize buffer to store byte of entry sub directory*/
    uint8_t* bufferFolderTemp =NULL;
    /* Declare Physical Sector, in FAT12 = sector of Data Area + FATentry - 2*/
    uint32_t physicalSector = 0U;
    /* Declare number of Entry which need to read in multi sector, It mean is sector per cluster*/
    uint32_t numberOfSector = 1U;
    /* Declare temporary pointer is pNode to check all node in linked list*/
    fatfs_entry_list_struct_t* pNode = NULL;
    fatfs_entry_list_struct_t* pNode1 = NULL;
    /* Declare temporary pointer is pNode1 to check first Cluster*/
  //  fatfs_entry_list_struct_t* pNode1 = NULL;
    /* Declare variable to store value of entry*/
    fatfs_entry_struct_t entryDir;
    /* Declard variable of entry number in directory entry */
    uint8_t entryNumber = 0U;
    bool result = true;

    /* To delete if *head point into memory have data*/
    if (*headDir != NULL)
    {
        free(*headDir);
        *headDir = NULL;
    }
    /* if user slect option in application, it will select with first is read directory and print, second is read subdirectory */
    pNode1 = s_rootDirList;
    while (pNode1 != NULL)
    {
        if (firstCluster == pNode1->entry.firstCluster)
        {
            pNode = s_rootDirList;
            break;
        }
        else
        {
            pNode = s_subDirList;
        }
        pNode1 = pNode1->next;
    }
    /* Initialize buffer to store cluster in Data area corresponding with fat element*/
    bufferFolderTemp = (uint8_t*)malloc(clusterSize);
    if (bufferFolderTemp != NULL && pNode !=NULL)
    {
        /* Brow the linked list of Folder and file in roof directory*/
        while (pNode != NULL)
        {
            /* check this node is folder and first cluster in linked list == wanting first cluster?*/

            if ((0x10U == pNode->entry.atrributes) && (firstCluster == pNode->entry.firstCluster))
            {
                /* Assign current cluster by first Cluster of pNode*/
                nextCluster = currentCluster;
                /* if nextCluster == 0xFFF it will end of while loop
                 * if nextCluster != 0xFFF it will read multi sector with physical sector which have calculated with number of Entry
                 * and store in bufferFolderTemp
                 */
                while (nextCluster != s_fatfsdata.endOfFile)
                {
                    /* Assign the previous nextCluster for current cluster*/
                    currentCluster = nextCluster;
                    /* Caculate value of physical sector follow the previous nexcluster*/
                    physicalSector = s_fatfsdata.sectorOfDataArea + (currentCluster - 2U);
                    /* Read sector at physical sector*/
                    kmc_read_multi_sector(physicalSector, numberOfSector, bufferFolderTemp);
                    /* Caculate next cluster follow on FAT table*/
                    nextCluster = __fatfs_getNextCluster(s_fatfsdata.FATTable_info, currentCluster);
                    /* Count sector to need how many sector which need to read*/
                    numberOfSector++;
                }
                /*Read each entry, with entry is 32 byte, so we have 16 total entry in sector */
                for (entryNumber = 0U; entryNumber < 16U; entryNumber++)
                {
                    /* Coppy all byte in 1 entry into buffer entryDir.entryDirectory*/
                    memcpy(entryDir.entryDirectory, __fatfs_readEntryOfDirectory(bufferFolderTemp, entryNumber), 32U);
                    /* Store all properties of entry in linked list*/
                    result = __faffs_checkPropertyOfEntry(entryDir, headDir);
                }
            }
            pNode = pNode->next;
        }
        result = true;
        free(bufferFolderTemp);
    }
    else
    {
        result = false;
    }
    /* Assign head Dir for pointer of Sub Directory*/
    s_subDirList = *headDir;

    return result;
}

/*
* @brief Read information of file
* @param1 first cluster which need to access directory
* @param2 pointer to pointer of buffer file to store linked list
* @param3 pointer of head directory of directory linked list
* @retval true if doing  successfully or false if doing failed
*/
uint32_t fatfs_readFile(const uint32_t firstCluster, uint8_t** bufferFileTemp, fatfs_entry_list_struct_t* headDir)
{
    /* Declare variable size of cluster. In FAT12, size of cluster = size of sector = 512 byte*/
    uint32_t clusterSize = (uint32_t)((uint16_t)s_boot_info.sectorPerCluster * s_boot_info.bytePerSector);
    /* Declare next cluster after checking element of FAT*/
    uint32_t nextCluster = 0U;
    /* Declare current cluster*/
    uint32_t currentCluster = 0U;
    /* Declare Physical Sector, in FAT12 = sector of Data Area + FATentry - 2*/
    uint32_t physicalSector = 0U;
    /* Declare number of Entry which need to read in multi sector, It mean is sector per cluster*/
    uint32_t numberOfSector = 0U;
    /* Declare temporary pointer is pNode to check all node in linked list*/
    fatfs_entry_list_struct_t* pNode = NULL;
    /* Declare temporary pointer is pNode1 to check first Cluster*/
    fatfs_entry_list_struct_t* pNode1 = NULL;
    /* Declard variable of entry number in directory entry */
    uint8_t entryNumber = 0U;
    /* Declard variable of entry number in directory entry */
    uint32_t sizeOfFile = 0U;
    uint32_t numberOfByte = 0U;
    /* Check first cluster == first cluse in root Directory?. If have assign pNode for linked list of rootDirectory.
     *If not assign pNode for linked list of Sub Directory
     */
     /* Assign headDir fo pNode*/
    pNode = headDir;
    /* Brow the linked list of Folder and file in roof directory*/
    while (pNode != NULL)
    {
        if ((0x00U == pNode->entry.atrributes || 0x20U == pNode->entry.atrributes || 0x28U == pNode->entry.atrributes) && (firstCluster == pNode->entry.firstCluster))
        {
            /* Assign current cluster by first Cluster of pNode*/
            currentCluster = firstCluster;
            nextCluster = currentCluster;
            /* if nextCluster == 0xFFF it will end of while loop
             * if nextCluster != 0xFFF it will read multi sector with physical sector which have calculated with number of Entry
             * and store in bufferFolderTemp
             */
            while (nextCluster != s_fatfsdata.endOfFile)
            {
                /* Assign the previous nextCluster for current cluster*/
                currentCluster = nextCluster;
                /* Caculate next cluster follow on FAT table*/
                nextCluster = __fatfs_getNextCluster(s_fatfsdata.FATTable_info, currentCluster);
                /* Count sector to need how many sector which need to read*/
                numberOfSector++;
            }
            /* Size of byte in file to allocate for memory*/
            sizeOfFile = numberOfSector * clusterSize;
            /* Allocate memory*/
            *bufferFileTemp = (uint8_t*)malloc(sizeOfFile);
            if (*bufferFileTemp != NULL)
            {
                /* Caculate start physical sector*/
                physicalSector = s_fatfsdata.sectorOfDataArea + (firstCluster - 2U);
                /* Read multi sector from starting cluster to end of cluster*/
                numberOfByte = kmc_read_multi_sector(physicalSector, numberOfSector, *bufferFileTemp);
            }
        }
        pNode = pNode->next;
    }
    /* Do not free memory pointed by bufferFileTemp because app class need data here to print */

    return numberOfByte;
}

/*
* @brief Deinitialize all pointer point to data memory
* @param1 none
* @retval none
*/
void fatfs_deinit(void)
{
    /*Free linked list of FAT Table*/
    free(s_fatfsdata.FATTable_info);
    /* Free linked list of root Directory*/
    free(s_rootDirList);
    /* Free linked list of Sub Directroy*/
    free(s_subDirList);
    kmc_deinit();
}
