/*------------------------------------------------------------------------------
 * uVision/ARM development tools
 * Copyright (C) 2015 - 2016 ARM Ltd and ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    web.c
 * Purpose: ROM Image generated from user specified files.
 * Note:    Generated by FCARM FILE CONVERTER V2.56.0.1, do not modify!
 *----------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdint.h>

/* File information */
typedef struct _imageFileItem {
  uint32_t       id;            /* Name identifier (CRC32 value of file name) */
  const uint8_t *data;          /* Data start address in ROM                  */
} imageFileItem;

#define IMAGE_FILE_COUNT 2U

/* Last-Modified: Tue, 16 May 2017 05:24:38 GMT */
const uint32_t imageLastModified = 1494912278U;

static const uint8_t imageFileData[59U] = {

  /*-- File: loc.cgx, 3 bytes --*/
  0x02U,0x4CU,0x00U,

  /*-- File: index.html, 56 bytes --*/
  0x3CU,0x68U,0x74U,0x6DU,0x6CU,0x3EU,0x0DU,0x0AU,0x09U,0x3CU,0x68U,0x65U,0x61U,
  0x64U,0x3EU,0x0DU,0x0AU,0x09U,0x09U,0x3CU,0x68U,0x31U,0x3EU,0x48U,0x65U,0x6CU,
  0x6CU,0x6FU,0x3CU,0x2FU,0x68U,0x31U,0x3EU,0x0DU,0x0AU,0x0DU,0x0AU,0x0DU,0x0AU,
  0x09U,0x3CU,0x2FU,0x68U,0x65U,0x61U,0x64U,0x3EU,0x0DU,0x0AU,0x3CU,0x2FU,0x68U,
  0x74U,0x6DU,0x6CU,0x3EU
};

static const imageFileItem imageFileTable[2U+1U] = {
  { 0x65C757F1U, &imageFileData[0U]  },
  { 0xE84188E2U, &imageFileData[3U]  },
  { 0x00000000U, &imageFileData[59U] }
};

/*
 * Calculate 32-bit CRC (Polynom: 0x04C11DB7)
 *   Parameters:
 *     crc32:       CRC initial value
 *     val:         Input value
 *   Return value:  Calculated CRC value
 */
static uint32_t crc32_8bit (uint32_t crc32, uint8_t val) {
  uint32_t n;

  crc32 ^= ((uint32_t)val) << 24U;
  for (n = 8U; n; n--) {
    if (crc32 & 0x80000000U) {
      crc32 <<= 1U;
      crc32  ^= 0x04C11DB7U;
    } else {
      crc32 <<= 1U;
    }
  }
  return (crc32);
}

/*
 * Get file information from ROM image
 *   Parameters:
 *     name:        File name
 *     data:        Pointer where file data pointer will be written
 *   Return value:  File size
 */
uint32_t imageFileInfo (const char *name, const uint8_t **data) {
  uint32_t id, n;

  if ((name == NULL) || (data == NULL)) return 0U;

  id = 0xFFFFFFFFU;
  for (; *name; name++) {
    id = crc32_8bit(id, *name);
  }

  for (n = 0U; n < IMAGE_FILE_COUNT; n++) {
    if (imageFileTable[n].id == id) {
      *data = imageFileTable[n].data;
      return (imageFileTable[n+1].data - imageFileTable[n].data);
    }
  }
  return 0U;
}
