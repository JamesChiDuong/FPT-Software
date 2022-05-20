/**
  ******************************************************************************
  * @file    HAL.h
  * @author  Fresher Ngo Chi Duong
  * @brief   This file contains all the functions prototypes for the HAL
  *          module driver.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INC_HAL_H_
#define INC_HAL_H_
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
/*******************************************************************************
* Add macros, enumeration types, structure types, inside:
* Definitions
******************************************************************************/
#define KMC_DEFAULT_NUMBER_OF_BYTE_PER_SECTOR_IN_ROOTDIRECTORY 512                  /* Define number byte of boot sector, default 512 byte*/
/*******************************************************************************
 * API
 ******************************************************************************/

/*
* @brief Initialization of pointer file and size of sector
* @param pointer of file with type is constant
* @retval true if open file successfully or false if open file failed
*/
bool kmc_init(const uint8_t* const filePath);

/*
* @brief Updating sector of size, it will apply for different type FAT
* @param sector of Size need to update
* @retval return false if having a error or return true if it successful
*/
bool kmc_update_sector_szie(uint32_t sectorSize);

/*
* @brief Reading byte of one sector
* @param1 Index of sector which need to access
* @param2 Buffer need to stored some byte
* @retval Number of Byte in sector
*/
int32_t kmc_read_sector(const uint32_t index, uint8_t* const buff);

/*
* @brief Reading byte of multi sector
* @param1 Index of sector which need to access
* @param2 Number of sector which need to read
* @param3 Buffer need to stored some byte
* @retval Number of Byte in sector
*/
int32_t kmc_read_multi_sector(const uint32_t index, const uint32_t num, uint8_t* const buff);

/*
* @brief Deleting initialization of file and pointer of file including close of file
* @param none
* @retval None
*/
bool kmc_deinit(void);
#endif/* INC_HAL_H_*/
