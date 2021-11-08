/*
 *  dna_compiler.h -- provide dna-system GCC compiler attribute options management.
 *  
 *  ORIGINAL AUTHOR: Xu Chun (chun.xu@broadlink.com.cn)
 *
 *  Copyright (c) 2016 Broadlink Corporation
 */

#ifndef __DNA_COMPILER_H
#define __DNA_COMPILER_H

#if DNA_COMPILER_SECTION_SRAM_EN

#if defined(__ssv6060__) || defined(__bl0907__)
#define DNA_COMPILER_SECTION_SRAM  __attribute__((section("copy_to_sram")))
#endif

#if defined(__mt7687__) || defined(__mt7682__) || defined(__mt7697__)
#define DNA_COMPILER_SECTION_SRAM  __attribute__((__section__(".ramTEXT")))
#endif

#endif /* DNA_COMPILER_SECTION_SRAM_EN */

#ifndef DNA_COMPILER_SECTION_SRAM
#define DNA_COMPILER_SECTION_SRAM
#endif

#if defined(__ICCARM__)
#define DNA_WEAK                   __weak
#define DNA_PACKED_START           __packed
#define DNA_PACKED_END
#define DNA_WARN_UNUSED_RET
#define DNA_ALIGNED(n)
#define DNA_UNUSED
typedef char *                     DNA_PVOID;
#else
#define DNA_WEAK                   __attribute__((weak))
#define DNA_PACKED_START
#define DNA_PACKED_END             __attribute__((packed))
#define DNA_WARN_UNUSED_RET        __attribute__((warn_unused_result))
#define DNA_ALIGNED(n)             __attribute__((aligned(n)))
#define DNA_UNUSED                 __attribute__((unused))
typedef void *                     DNA_PVOID;
#endif

#endif

