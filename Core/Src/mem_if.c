//// FILE: bootloader/mem_if.c
//
///*
// * Implements DFU memory interface using HAL flash routines.
// * This version assumes STM32F407 (1MB) sector layout.
// * Adapt sector mapping if you have a different device.
// */
//
//#include "mem_if.h"
//#include <string.h>
//
//// === Exported to DFU stack ===
//USBD_DFU_MediaTypeDef USBD_DFU_fops_FS  =
//{
//    (uint8_t*)"@Internal Flash   /0x08000000/03*016Ka,01*016Kg,01*064Kg,07*128Kg,04*016Kg,01*064Kg,07*128Kg",   // String descriptor shown in DFU tools
//    MEM_If_Init_FS,
//    MEM_If_DeInit_FS,
//    MEM_If_Erase_FS,
//    MEM_If_Write_FS,
//    MEM_If_Read_FS,
//    MEM_If_GetStatus_FS
//};
//
//// Helper: address to flash sector mapping for STM32F407 (1MB)
//static uint32_t GetSector(uint32_t Address)
//{
//    uint32_t sector = 0;
//
//    if((Address < 0x08004000U) && (Address >= 0x08000000U))
//        sector = FLASH_SECTOR_0;
//    else if((Address < 0x08008000U) && (Address >= 0x08004000U))
//        sector = FLASH_SECTOR_1;
//    else if((Address < 0x0800C000U) && (Address >= 0x08008000U))
//        sector = FLASH_SECTOR_2;
//    else if((Address < 0x08010000U) && (Address >= 0x0800C000U))
//        sector = FLASH_SECTOR_3;
//    else if((Address < 0x08020000U) && (Address >= 0x08010000U))
//        sector = FLASH_SECTOR_4;
//    else if((Address < 0x08040000U) && (Address >= 0x08020000U))
//        sector = FLASH_SECTOR_5;
//    else if((Address < 0x08060000U) && (Address >= 0x08040000U))
//        sector = FLASH_SECTOR_6;
//    else if((Address < 0x08080000U) && (Address >= 0x08060000U))
//        sector = FLASH_SECTOR_7;
//    else if((Address < 0x080A0000U) && (Address >= 0x08080000U))
//        sector = FLASH_SECTOR_8;
//    else if((Address < 0x080C0000U) && (Address >= 0x080A0000U))
//        sector = FLASH_SECTOR_9;
//    else if((Address < 0x080E0000U) && (Address >= 0x080C0000U))
//        sector = FLASH_SECTOR_10;
//    else
//        sector = FLASH_SECTOR_11;
//
//    return sector;
//}
//
//uint16_t MEM_If_Init_FS(void)
//{
//    // Nothing special to init for internal flash
//    return USBD_OK;
//}
//
//uint16_t MEM_If_DeInit_FS(void)
//{
//    return USBD_OK;
//}
//
//uint16_t MEM_If_Erase_FS(uint32_t Add)
//{
//    HAL_StatusTypeDef status;
//    FLASH_EraseInitTypeDef EraseInitStruct;
//    uint32_t FirstSector = GetSector(Add);
//
//    // For simplicity, erase from FirstSector to end of flash (or just required sectors)
//    // Compute number of sectors to erase from FirstSector up to sector 11
//    uint32_t NbSectors = (FLASH_SECTOR_11 - FirstSector) + 1U;
//
//    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
//    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3; // 2.7 to 3.6V
//    EraseInitStruct.Sector = FirstSector;
//    EraseInitStruct.NbSectors = NbSectors;
//
//    HAL_FLASH_Unlock();
//    uint32_t SectorError = 0U;
//    status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
//    HAL_FLASH_Lock();
//
//    return (status == HAL_OK) ? USBD_OK : USBD_FAIL;
//}
//
//uint16_t MEM_If_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len)
//{
//    HAL_StatusTypeDef status = HAL_OK;
//    uint32_t address = (uint32_t)dest;
//
//    HAL_FLASH_Unlock();
//
//    // Program by 32-bit words
//    for (uint32_t i = 0; i < Len; i += 4)
//    {
//        uint32_t word = 0xFFFFFFFFU;
//        // copy up to 4 bytes into word, little-endian
//        uint8_t b0 = (i + 0 < Len) ? src[i + 0] : 0xFF;
//        uint8_t b1 = (i + 1 < Len) ? src[i + 1] : 0xFF;
//        uint8_t b2 = (i + 2 < Len) ? src[i + 2] : 0xFF;
//        uint8_t b3 = (i + 3 < Len) ? src[i + 3] : 0xFF;
//        word = ((uint32_t)b3 << 24) | ((uint32_t)b2 << 16) | ((uint32_t)b1 << 8) | ((uint32_t)b0);
//
//        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i, word) != HAL_OK)
//        {
//            status = HAL_ERROR;
//            break;
//        }
//    }
//
//    HAL_FLASH_Lock();
//
//    return (status == HAL_OK) ? USBD_OK : USBD_FAIL;
//}
//
//uint8_t *MEM_If_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len)
//{
//    memcpy(dest, src, Len);
//    return dest;
//}
//
//uint16_t MEM_If_GetStatus_FS(uint32_t Add, uint8_t Cmd, uint8_t *buffer)
//{
//    // DFU wants a poll timeout — put a reasonable guess.
//    // buffer points to a 3-byte little-endian timeout value
//    uint32_t timeout = 0U;
//
//    if (Cmd == DFU_MEDIA_ERASE)
//    {
//        // Erase typically takes longer
//        timeout = 500U; // 500 ms
//    }
//    else if (Cmd == DFU_MEDIA_PROGRAM)
//    {
//        timeout = 50U; // 50 ms
//    }
//    else
//    {
//        timeout = 0U;
//    }
//
//    buffer[0] = (uint8_t)(timeout & 0xFFU);
//    buffer[1] = (uint8_t)((timeout >> 8) & 0xFFU);
//    buffer[2] = (uint8_t)((timeout >> 16) & 0xFFU);
//
//    return USBD_OK;
//}
