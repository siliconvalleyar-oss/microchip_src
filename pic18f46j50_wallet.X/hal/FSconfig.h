/******************************************************************************
 * FSconfig.h - Configuración del File System para PIC18F46J50
 *****************************************************************************/

#ifndef FSCONFIG_H
#define FSCONFIG_H

#include "hal_config.h"

//=============================================================================
// Configuración del File System
//=============================================================================
#define FS_MAX_FILES_OPEN       1
#define MEDIA_SECTOR_SIZE       512
#define MEDIA_SOFT_DETECT

//=============================================================================
// Características habilitadas
//=============================================================================
#define ALLOW_FILESEARCH
//#define ALLOW_WRITES          // Descomentar si se necesitan escrituras
#define ALLOW_FORMATS
#define ALLOW_DIRS
#define SUPPORT_FAT32

//=============================================================================
// Timestamp
//=============================================================================
#define INCREMENTTIMESTAMP

//=============================================================================
// Prototipos de funciones físicas (implementadas en spi_sd.c)
//=============================================================================
uint8_t MDD_MediaDetect(void);
void MDD_MediaInitialize(void);
uint8_t MDD_SectorRead(uint32_t sector, uint8_t* buffer);
uint8_t MDD_SectorWrite(uint32_t sector, uint8_t* buffer);
uint32_t MDD_ReadCapacity(void);
uint16_t MDD_ReadSectorSize(void);
uint8_t MDD_WriteProtectState(void);

#endif
