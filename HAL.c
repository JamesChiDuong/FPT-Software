#include "HAL.h"
/*******************************************************************************
* Add macros, enumeration types, structure types, inside:
* Definitions
******************************************************************************/
/*******************************************************************************
* Variables
******************************************************************************/
static FILE* sp_ptrFile = NULL;                                                     /* Declare pointer of File*/
static uint32_t s_sectorSize = 0U;                                                  /* Declare variable of sectoSize*/

/*******************************************************************************
* Add static API prototype in C file inside :
* Prototypes
******************************************************************************/
/*******************************************************************************
* Code
******************************************************************************/

/*
* @brief Initialization of pointer file and size of sector
* @param pointer of file with type is constant
* @retval true if open file successfully or false if open file failed
*/
bool kmc_init(const uint8_t* const filePath)
{
   /* Initialize pointer for file
    * Check Pointer, if pointer null is return false
    * Assign size 512 for static variable
    */
    bool result = true;

    sp_ptrFile = fopen(filePath,"rb");
    if (NULL == sp_ptrFile)
    {
        result = false;
    }
    else
    {
        s_sectorSize = KMC_DEFAULT_NUMBER_OF_BYTE_PER_SECTOR_IN_ROOTDIRECTORY;
    }

    return result;
}

/*
* @brief Updating sector of size, it will apply for different type FAT
* @param sector of Size need to update, byte of size per sector is 4 byte
* @retval return false if having a error or return true if it successful
*/
bool kmc_update_sector_szie(const uint32_t sectorSize)
{
   /* Check sector of Size, if sectoc divided by 512 is 0 it will assign value for s_sectorSize
    */
    bool result = true;

    if (0 != sectorSize % 512)
    {
        result = false;
    }
    else
    {
        s_sectorSize = sectorSize;
    }

    return result;
}

/*
* @brief Reading byte of one sector
* @param1 Index of sector which need to access
* @param2 Buffer need to stored some byte
* @retval Number of Byte in sector
*/
int32_t kmc_read_sector(const uint32_t index, uint8_t* const buff)
{
   /* Initialize variable of number of byte which received for fread
    * Check function fseek have do successful, and pick at index need to read
    * Assign pointer at set position
    */
    size_t numberOfByte = 0;

    if (0 == fseek(sp_ptrFile, (s_sectorSize * index), SEEK_SET))
    {
        numberOfByte = fread(buff, 1U, (size_t)s_sectorSize, sp_ptrFile);
        if (numberOfByte != s_sectorSize)
        {
            numberOfByte = -1;
        }
    }

    return (int32_t)numberOfByte;
}

/*
* @brief Reading byte of multi sector
* @param1 Index of sector which need to access
* @param2 Number of sector which need to read
* @param3 Buffer need to stored some byte
* @retval Number of Byte in sector
*/
int32_t kmc_read_multi_sector(const uint32_t index,const uint32_t num, uint8_t* const buff)
{
    /* To return value of byte which have read*/
    size_t numberOfByte = 0;
    /* To count byte number when reading multi sector*/
    size_t numberOfByteTemp = 0;
    /* Position need to read byte*/
    uint64_t offset = 0U;
    uint32_t i = 0U;

    if (0 == fseek(sp_ptrFile, (s_sectorSize * index), SEEK_SET))
    {
        for (i = 0U; i < num; i++)
        {
            offset = (uint64_t)i * s_sectorSize;
            numberOfByteTemp += fread(&buff[offset], 1U, (size_t)s_sectorSize, sp_ptrFile);
        }
        if (numberOfByteTemp == (size_t)num * s_sectorSize)
        {
            numberOfByte = numberOfByteTemp;
        }
    }

    return (int32_t)numberOfByte;
}

/*
* @brief Deleting initialization of file and pointer of file including close of file
* @param none
* @retval None
*/
bool kmc_deinit(void)
{
    bool result = true;

    if (0 == fclose(sp_ptrFile))
    {
        result = true;
        sp_ptrFile = NULL;
        s_sectorSize = 512U;
    }

    return result;
}
