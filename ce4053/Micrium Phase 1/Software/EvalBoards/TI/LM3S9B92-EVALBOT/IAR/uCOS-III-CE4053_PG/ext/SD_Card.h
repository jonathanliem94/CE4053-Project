/*
This file is made in CHiPES. It contains some definition from TI as it is, some 
handwritten.

This file contains necessary function declaration, variable and Array declaration
which is being used by Sd card and SPI Driver.
*/

#ifndef __SD_CARD_H__
#define __SD_CARD_H__

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/ssi.h"
#include "cmdline.h"
#include "ff.h"
#include "diskio.h"
#include <string.h>
#include <stdio.h>

#include <bsp.h>      // Anand

//*****************************************************************************
//
// Defines the size of the buffers that hold the path, or temporary data from
// the memory card.  There are two buffers allocated of this size.  The buffer
// size must be large enough to hold the longest expected full path name,
// including the file name, and a trailing null character.
//
//*****************************************************************************
#define PATH_BUF_SIZE   64

//*****************************************************************************
//
// Defines the size of the buffer that holds the command line.
//
//*****************************************************************************
#define CMD_BUF_SIZE    64

//*****************************************************************************
//
// Defines the size of the tag, Data1 and Data2 that is written in the file.
// User can change it accordingly.
//
//*****************************************************************************
#define TAG_SIZE   7
#define DATA1_SIZE 11
#define DATA2_SIZE 11

//*****************************************************************************
//
// Defines the size of a line that is to be written to or read from the file.
//
//*****************************************************************************
#define FILE_LINE_SIZE  TAG_SIZE+DATA1_SIZE+DATA2_SIZE+4

//*****************************************************************************
//
// This buffer holds the full path to the current working directory.  Initially
// it is root ("/").
//
//*****************************************************************************
static char g_cCwdBuf[PATH_BUF_SIZE] = "/";

//*****************************************************************************
//
// A temporary data buffer used when manipulating file paths, or reading data
// from the memory card.
//
//*****************************************************************************
static char g_cTmpBuf[PATH_BUF_SIZE];

//*****************************************************************************
//
// Current FAT fs state.
//
//*****************************************************************************
static FATFS g_sFatFs;
static FIL g_sFileObject;

/* Some functions that will be used by User */
CPU_INT32S SDCardInit(void);                            // To initialize the SD card
CPU_INT32S ReadFromFile(char*, char*, char*, char*);    // To Read data from File
CPU_INT32S WriteToFile(char*, char*, char*, char*);     // To Write data into File

#endif // __SD_CARD_H__