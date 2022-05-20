/**
  ******************************************************************************
  * @file    app.h
  * @author  Fresher Ngo Chi Duong
  * @brief   This file contains all the functions for implementing with the FAT file systerm.
  ******************************************************************************
  */
  /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INC_App_H_
#define INC_APP_H_
#include "fatfs.h"
/*******************************************************************************
* Add macros, enumeration types, structure types, inside:
* Definitions
******************************************************************************/
typedef struct _data_app_struct
{
    uint8_t lastModiDateAndTime[30];                                            /* The buffer store date of time follow format: "month/day/year hour:minute*/
    uint8_t typeOfDataOrFolder[200];                                            /* The buffer store type of file or folder after foramting */
    uint8_t fileName[12];                                                       /* The buffer store file name after formating*/
}app_type_file_folder_struct_t;

/*******************************************************************************
* Variables
******************************************************************************/
/*******************************************************************************
 * API
 ******************************************************************************/

 /*
 * @brief Inputing from user
 * @param none
 * @retval return true if successfully and failed if failed
 */
bool app_init(void);

/*
* @brief Creating menu for user
* @param Value of total number of folder need check
* @retval return value from user if successfully and 99 if failed
*/
void app_createMenuOption(void);

/*
* @brief Deinitialize all pointer point to data memory
* @param1 none
* @retval none
*/
void app_deinit(void);;
#endif/*INC_App_H_ */
