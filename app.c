/**
  ******************************************************************************
  * @file    fatfs.h
  * @author  Fresher Ngo Chi Duong
  * @brief   This file contains all the functions prototypes for the FAT file systerm.
  ******************************************************************************
  */
  /* Define to prevent recursive inclusion -------------------------------------*/
#include "app.h"
/*******************************************************************************
* Add macros, enumeration types, structure types, inside:
* Definitions
******************************************************************************/
/*******************************************************************************
* Variables
******************************************************************************/
/*******************************************************************************
* Variables
******************************************************************************/
/* Declare variable of boot Sector*/
static fats_boot_infor_struct_t* s_bootInfor = NULL;                            /* Pointer of Boot sector to mange memory which store value of information of boot sector*/
static uint8_t FilePath[45] = { 0 };                                            /* Array to store path of file which is inputed by user*/
static fatfs_entry_list_struct_t* s_entrySubDir = NULL;                         /* Pointer of head node in linked list of fisrt SubDir*/
static fatfs_entry_list_struct_t* s_entrySubDir1 = NULL;                        /* Pointer of head node second in linked list SubDir*/
static fatfs_entry_list_struct_t* s_entrySubDir2 = NULL;                        /* Pointer of head node third in linked list SubDir*/
static fatfs_entry_list_struct_t* s_entryRootDir = NULL;                        /* Pointer of root head node in linked list directory*/
static app_type_file_folder_struct_t s_dataDisk;                                /* Variable of data Disk to format data from fatfs library*/
static int8_t g_optionOfUser = 0;                                               /* Variable to store option from user*/
static uint8_t s_FileOrFolderType = 0U;                                         /* Variable to check type is folder or file*/
static int8_t s_userOption = 0;                                                 /* To count total of all option in root Directory*/
static int8_t s_userOption1 = 0;                                                /* To count total of all option in Sub Directory*/
/*******************************************************************************
* Add static API prototype in C file inside :
* Prototypes
******************************************************************************/

/*
* @brief Formating all data which read from fatfs
* @param Pointer of head Node in list
* @retval None
*/
static void __app_formartData(fatfs_entry_struct_t* entryDir);

/*
* @brief Checking valid input from user
* @param1 Pointer of buffer which stored data from user
* @param2 Variable to check error from user
* @retval true if success and false if failed
*/
static bool __app_IO_checkValidInput(int8_t* inputBuff, const int8_t checkErrorInput);

/*
* @brief Inputing from user
* @param Value of total number of folder need check
* @retval return value from user if successfully and 99 if failed
*/
static int8_t __app_IO__readFirstChar(const int8_t checkErrorInput);

/*
* @brief Printing all file or folder
* @param1 linked list of father sub directory
* @param2 Check type, if type == 1, this is a folder, else this is a file
* @param3 Linked list of son sub directory
* @retval none
*/
static void __app_createMenuOnRootDir(fatfs_entry_list_struct_t* const list,const int8_t option, uint8_t* Type, fatfs_entry_list_struct_t** listSub);

/*
* @brief Printing information of root directory
* @param1 none
* @retval none
*/
static void __app_IO_inforOfRootDir(void);

/*
* @brief Printing information of Sub directory
* @param head Node of sub Dir
* @retval none
*/
static void __app_IO_inforOfSubDir(fatfs_entry_list_struct_t* listSub);
/*******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
* @brief Formating all data which read from fatfs
* @param Pointer of head Node in list
* @retval None
*/
static void __app_formartData(fatfs_entry_struct_t* entryDir)
{
    /* Declare variable of hour, minute, year, month, day*/
    uint8_t hour = 0U;
    uint8_t minute = 0U;
    uint16_t year = 0U;
    uint8_t month = 0U;
    uint8_t day = 0U;

    /* Format Data Time*/
    hour = (uint8_t)(entryDir->lastModiTime >> 11U);
    minute = (uint8_t)((entryDir->lastModiTime >> 5U) & 0x0F);
    year = (entryDir->lastModiDate >> 9U) + 1980U;
    month = (uint8_t)((entryDir->lastModiDate >> 5U) & 0x0FU);
    day = (uint8_t)(entryDir->lastModiDate & 0x1FU);
    memset(s_dataDisk.lastModiDateAndTime, 0, strlen(s_dataDisk.lastModiDateAndTime));
    snprintf(s_dataDisk.lastModiDateAndTime, 30, "%d/%d/%d %d:0%d", month, day, year, hour, minute);
    /* Format Type of File or Directory*/
    /* This is for file*/
    if (0x00U == entryDir->atrributes || 0x20U == entryDir->atrributes || 0x28U == entryDir->atrributes)
    {
        if (strstr(entryDir->extension, "TXT") != NULL)
        {
            memset(s_dataDisk.typeOfDataOrFolder, 0, strlen(s_dataDisk.typeOfDataOrFolder));
            memcpy(s_dataDisk.typeOfDataOrFolder, "Text Document", strlen("Text Document"));
        }
        else if (strstr(entryDir->extension, "DOC") != NULL)
        {
            memset(s_dataDisk.typeOfDataOrFolder, 0, strlen(s_dataDisk.typeOfDataOrFolder));
            memcpy(s_dataDisk.typeOfDataOrFolder, "Microsoft Word 97", strlen("Microsoft Word 97"));
        }
        else if (strstr(entryDir->extension, "PDF") != NULL)
        {
            memset(s_dataDisk.typeOfDataOrFolder, 0, strlen(s_dataDisk.typeOfDataOrFolder));
            memcpy(s_dataDisk.typeOfDataOrFolder, "Foxit PhantomPDF PDF", strlen("Foxit PhantomPDF PDF Document"));
        }
        else if (strstr(entryDir->extension, "PNG") != NULL)
        {
            memset(s_dataDisk.typeOfDataOrFolder, 0, strlen(s_dataDisk.typeOfDataOrFolder));
            memcpy(s_dataDisk.typeOfDataOrFolder, "PNG File", strlen("PNG File"));
        }
        else
        {
            memset(s_dataDisk.typeOfDataOrFolder, 0, strlen(s_dataDisk.typeOfDataOrFolder));
            memcpy(s_dataDisk.typeOfDataOrFolder, "Unknow", strlen("Unknow"));
        }

    }
    /* This is for directory*/
    else if (0x10U == entryDir->atrributes )
    {
        memset(s_dataDisk.typeOfDataOrFolder, 0, strlen(s_dataDisk.typeOfDataOrFolder));
        memcpy(s_dataDisk.typeOfDataOrFolder, "File Folder", strlen("File Folder"));
    }
    /* Format File Name*/
    memcpy(s_dataDisk.fileName, entryDir->fileName, 11);
}

/*
* @brief Checking valid input from user
* @param1 Pointer of buffer which stored data from user
* @param2 Variable to check error from user
* @retval true if success and false if failed
*/
static bool __app_IO_checkValidInput(int8_t* inputBuff, const int8_t checkErrorInput)
{
    bool result = true;
    uint8_t i = 0U;

    /* Check value input <0 or > total number file or folder or > 100*/
    if ((int8_t)atoi(inputBuff) < 0 || (int8_t)atoi(inputBuff) > checkErrorInput || (int8_t)atoi(inputBuff) > 100)
    {
        printf("\n\n\tERROR: Only enter value from 0 - %d!!", checkErrorInput);
        result = false;
    }
    /* Chec value input have alphabet data?*/
    for(i = 0; i < strlen(inputBuff); i++)
    {
        if ((0 != isalpha(inputBuff[i])) || '.' == inputBuff[i])
        {
            printf("\n\n\tERROR: Only enter integer Type!!");
            result = false;
            break;
        }
    }

    return result;
}

/*
* @brief Inputing from user
* @param Value of total number of folder need check
* @retval return value from user if successfully and 99 if failed
*/
static int8_t __app_IO__readFirstChar(const int8_t checkErrorInput)
{
    int8_t inputBuff[100] = { 0 };
    int8_t value = 0;
    bool result = false;

    printf("\n\n\t<>Choose one option: ");
    rewind(stdin);
    fgets(inputBuff, sizeof(inputBuff), stdin);
    result = __app_IO_checkValidInput(inputBuff, checkErrorInput);
    if (result == true)
    {
        value = (int8_t)atoi(inputBuff);
    }
    else
    {
        value = 99;
    }

    return value;
}

/*
* @brief Printing all file or folder
* @param1 linked list of father sub directory
* @param2 Check type, if type == 1, this is a folder, else this is a file
* @param3 Linked list of son sub directory
* @retval none
*/
static void __app_createMenuOnRootDir(fatfs_entry_list_struct_t* const list, const int8_t option, uint8_t* Type, fatfs_entry_list_struct_t** listSub)
{
    /* Declare variable checkNode is a index to find right node in linked list*/
    int8_t checkNode = 1;
    /* Declare variable position in linked, it is assigned by option from user*/
    int8_t position = option;
    /* Declare pointer pNode to brow all node in linked lit*/
    fatfs_entry_list_struct_t* pNode = NULL;
    /* Declare variable to check folder which have initialize before*/
    static uint8_t check = 0U;
    /* Assign pNode for head node of father sub*/
    pNode = list;
    uint32_t i = 0U;
    uint8_t* bufferFileTemp = NULL;
    int8_t userOptionTemp = 0;

    /* Brow all node in linked to find right position*/
    while (pNode != NULL && checkNode != position)
    {
        pNode = pNode->next;
        checkNode++;
    }
    if (pNode != NULL)
    {
        /* if right node which need to read*/
        if (checkNode == position)
        {
            /* If node is a file*/
            if (0x00U == pNode->entry.atrributes ||0x20U == pNode->entry.atrributes || 0x28U == pNode->entry.atrributes)
            {
                /* initialize buffer to store data, and size is a fileSize of node in linked list*/
                bufferFileTemp = (uint8_t*)malloc((pNode->entry.fileSize));
                fatfs_readFile(pNode->entry.firstCluster, &bufferFileTemp, list);
                for (i = 0; i < pNode->entry.fileSize; i++)
                {
                    printf("%c", bufferFileTemp[i]);
                }
                /* Assign type = 1 if this is a file*/
                *Type = 0U;
                free(bufferFileTemp);
            }
            /* if node is a folder*/
            else if (0x10U == pNode->entry.atrributes )
            {
                /* read directory with first cluster, and store son folder into list sub*/
                fatfs_readDirectory(pNode->entry.firstCluster, listSub);
                printf("\n\n\t\t%70s", "*********************Sub Directory********************\n\n");
                printf("\t\t%-16s %-30s %-30s %-30s %s\n", "Option", "Name", "Date modified", "Type", "Size");
                /* Brow pNode into list sub to printf data of son folder*/
                for (pNode = *listSub; pNode != NULL; pNode = pNode->next)
                {
                    __app_formartData(&pNode->entry);
                    printf("\t\t%-16d %-30s %-30s %-30s %d\n", (userOptionTemp + 1), (int8_t*)s_dataDisk.fileName, (int8_t*)s_dataDisk.lastModiDateAndTime, (int8_t*)s_dataDisk.typeOfDataOrFolder, pNode->entry.fileSize);
                    /* Count total file or folder in son directory*/
                    userOptionTemp++;
                }
                /* Assign type = 1 if this is a folder*/
                *Type = 1U;
                check++;
                s_userOption1 = userOptionTemp;
                printf("\n\tNOTICE: Choose 0 to back root directory");
            }
        }
    }
}

/*
* @brief Printing information of root directory
* @param1 none
* @retval none
*/
static void __app_IO_inforOfRootDir(void)
{
    uint8_t i = 1U;
    fatfs_entry_list_struct_t* pNode = NULL;
    int8_t userOptionTemp = 0;

    printf("\n\n\t\t%70s", "*********************Root Directory********************\n\n");
    printf("\t\t%-16s %-30s %-30s %-30s %s\n", "Option", "Name", "Date modified", "Type", "Size");
    for (pNode = s_entryRootDir; pNode != NULL; pNode = pNode->next)
    {
        __app_formartData(&pNode->entry);
        printf("\t\t%-16d %-30s %-30s %-30s %d\n", (userOptionTemp + 1), (int8_t*)s_dataDisk.fileName, (int8_t*)s_dataDisk.lastModiDateAndTime, (int8_t*)s_dataDisk.typeOfDataOrFolder, pNode->entry.fileSize);
        userOptionTemp++;
    }
    printf("\n\tNOTICE: Choose 0 to exit program");
    s_userOption = userOptionTemp;
}

/*
* @brief Printing information of Sub directory
* @param1 none
* @retval none
*/
static void __app_IO_inforOfSubDir(fatfs_entry_list_struct_t* listSub)
{
    uint8_t i = 1U;
    fatfs_entry_list_struct_t* pNode = NULL;
    int8_t userOptionTemp = 0;

    printf("\n\n\t\t%70s", "*********************Sub Directory********************\n\n");
    printf("\t\t%-16s %-30s %-30s %-30s %s\n", "Option", "Name", "Date modified", "Type", "Size");
    /* Brow pNode into list sub to printf data of son folder*/
    for (pNode = listSub; pNode != NULL; pNode = pNode->next)
    {
        __app_formartData(&pNode->entry);
        printf("\t\t%-16d %-30s %-30s %-30s %d\n", (userOptionTemp + 1), (int8_t*)s_dataDisk.fileName, (int8_t*)s_dataDisk.lastModiDateAndTime, (int8_t*)s_dataDisk.typeOfDataOrFolder, pNode->entry.fileSize);
        /* Count total file or folder in son directory*/
        userOptionTemp++;
    }
}

/*
* @brief Inputing from user
* @param none
* @retval return true if successfully and failed if failed
*/
bool app_init(void)
{
    uint8_t i = 0U;
    bool result = true;

    rewind(stdin);
    printf("\n\t<>Please put your path file: ");
    scanf("%s", FilePath);
    /* Reading from fatfs init to read infor of boot sector and root directory*/
    result = fatfs_init(FilePath, &s_bootInfor, &s_entryRootDir);
    if (true != result)
    {
        printf("\n\tERROR: Can not open file\r\n");
    }
    /* Printing value of boot sector*/
    else
    {
        printf("\n\t\t%70s", "*********************Boot Sector Information********************\n\n");
        printf("\n\t- FAT Type: ");
        for (i = 0U; (i < strlen(s_bootInfor->fileSytermType)); i++)
        {
            printf("%c", s_bootInfor->fileSytermType[i]);
        }
        printf("\n\t- Byte Per Sector: %d byte", s_bootInfor->bytePerSector);
        printf("\n\t- Sector Per Cluster: %d byte", s_bootInfor->sectorPerCluster);
        printf("\n\t- Number of reserved sectors: %d byte", s_bootInfor->numberOfReservedSector);
        printf("\n\t- Number of FAT on disk: %d", s_bootInfor->numberOfFat);
        printf("\n\t- Maximum number of root directory entries: %d", s_bootInfor->maximumRootDirEntry);
        printf("\n\t- Sectors per FAT: %d", s_bootInfor->sectorPerFat);
        printf("\n\t- Total sector count: %d sectors", s_bootInfor->totalSectorCount);
        printf("\n\t- VolumeID: %d", s_bootInfor->volumeID);
    }

    return result;
}

/*
* @brief Creating menu for user
* @param Value of total number of folder need check
* @retval return value from user if successfully and 99 if failed
*/
void app_createMenuOption(void)
{
    uint8_t i = 0U;
    uint8_t j = 0U;

    /* Print infor of Root Directory*/
    __app_IO_inforOfRootDir();
    /* Waiting user input*/
    g_optionOfUser = __app_IO__readFirstChar(s_userOption);
    /* if user input valid value, user will input again*/
    while (99U == g_optionOfUser)
    {
        g_optionOfUser = __app_IO__readFirstChar(s_userOption);
    }
    /* if user input right*/
    while(g_optionOfUser > 0)
    {
        /* Print file of foler, those which user want to choose, then assign s_entrySubDir with head of Sub Dir Linked List
        * with father sub is a rootDir and son Sub is s_entrySubDir
        */
        __app_createMenuOnRootDir(s_entryRootDir, g_optionOfUser, &s_FileOrFolderType, &s_entrySubDir);
        /* if node in list is folder, user need to option in new folder
        * if not, this is a file, just print and user need to option in current folder- at that time this is a root dir
        */
        if (1 == s_FileOrFolderType)
        {
            g_optionOfUser = __app_IO__readFirstChar(s_userOption1);
            /* User choose one otion in new dir, if value == 0 it will back father folder*/
            while (0 != g_optionOfUser)
            {
                /* Print infor of file of folder in next SubDir
                and assign for next son SubDir
                */
                __app_createMenuOnRootDir(s_entrySubDir, g_optionOfUser, &s_FileOrFolderType, &s_entrySubDir1);
                /* If sub folder have file, user will choose option , but if folder empty, user jsut back current father Dir*/
                if (1 == s_FileOrFolderType && 0!=s_userOption1)
                {
                    g_optionOfUser = __app_IO__readFirstChar(s_userOption1);
                    while (0 != g_optionOfUser)
                    {
                        __app_createMenuOnRootDir(s_entrySubDir1, g_optionOfUser, &s_FileOrFolderType, &s_entrySubDir2);
                        if (1 == s_FileOrFolderType)
                        {
                            g_optionOfUser = __app_IO__readFirstChar(s_userOption1);
                            while (0 != g_optionOfUser)
                            {
                                g_optionOfUser = __app_IO__readFirstChar(s_userOption1);
                            }
                        }
                        __app_IO_inforOfSubDir(s_entrySubDir1);
                        s_userOption1 = 1;
                        g_optionOfUser = __app_IO__readFirstChar(s_userOption1);
                    }
                     s_userOption1 = 3;
                    __app_IO_inforOfSubDir(s_entrySubDir);
                }
                g_optionOfUser = __app_IO__readFirstChar(s_userOption1);
            }
            __app_IO_inforOfRootDir();
        }
        g_optionOfUser = __app_IO__readFirstChar(s_userOption);
        /* If user choose 0, it will exit program*/
        if (0 == g_optionOfUser)
        {
            printf("\n\t<>Exit program");
        }
        /* if user input wrong value, user need to input again*/
        while (99 == g_optionOfUser)
        {
            g_optionOfUser = __app_IO__readFirstChar(s_userOption);
        }
    }
}

/*
* @brief Deinitialize all pointer point to data memory
* @param1 none
* @retval none
*/
void app_deinit(void)
{
    free(s_entrySubDir1);
    free(s_entrySubDir2);
    fatfs_deinit();
}
