/*
 * Copyright (c) 2007-2008, Orlando Bassotto. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the distribution.
 *   * Neither the name of the author nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __laoder_elf_common_h
#define __laoder_elf_common_h

#include "loader.h"

typedef loader_uint8_t  elf_small_t;
typedef loader_uint16_t elf_half_t;
typedef loader_sint16_t elf_shalf_t;
typedef loader_uint32_t elf_word_t;
typedef loader_sint32_t elf_sword_t;
typedef loader_uint64_t elf_xword_t;
typedef loader_sint64_t elf_sxword_t;

typedef elf_word_t elf32_address_t;
typedef elf_word_t elf32_offset_t;

typedef elf_xword_t elf64_address_t;
typedef elf_xword_t elf64_offset_t;

/*
 * Size (in bytes) of the ident field in
 * the ELF header.
 */
#define ELF_EI_NIDENT 16

/*
 * ELF header ident fields.
 */
#define ELF_EI_MAG0       0
#define ELF_EI_MAG1       1
#define ELF_EI_MAG2       2
#define ELF_EI_MAG3       3
#define ELF_EI_CLASS      4
#define ELF_EI_DATA       5
#define ELF_EI_VERSION    6
#define ELF_EI_OSABI      7
#define ELF_EI_ABIVERSION 8
#define ELF_EI_PAD        9

/*
 * ELF magic.
 */
#define ELF_ELFMAG0 0x7f
#define ELF_ELFMAG1 'E'
#define ELF_ELFMAG2 'L'
#define ELF_ELFMAG3 'F'

/*
 * ELF class.
 */
#define ELF_ELFCLASSNONE 0
#define ELF_ELFCLASS32 1
#define ELF_ELFCLASS64 2

/*
 * ELF data endian.
 */
#define ELF_ELFDATANONE 0
#define ELF_ELFDATA2LSB 1
#define ELF_ELFDATA2MSB 2

/*
 * ELF OS ABI.
 */
#define ELF_ELFOSABI_SYSV       0
#define ELF_ELFOSABI_HPUX       1
#define ELF_ELFOSABI_NETBSD     2
#define ELF_ELFOSABI_LINUX      3
#define ELF_ELFOSABI_HURD       4
#define ELF_ELFOSABI_SOLARIS    6
#define ELF_ELFOSABI_AIX        7
#define ELF_ELFOSABI_IRIX       8
#define ELF_ELFOSABI_FREEBSD    9
#define ELF_ELFOSABI_TRU64      10
#define ELF_ELFOSABI_MODESTO    11
#define ELF_ELFOSABI_OPENBSD    12
#define ELF_ELFOSABI_OPENVMS    13
#define ELF_ELFOSABI_NSK        14
#define ELF_ELFOSABI_AROS       15
#define ELF_ELFOSABI_ARM        97
#define ELF_ELFOSABI_STANDALONE 255

/*
 * ELF header machines.
 */
#define ELF_EM_NONE           0   /* No machine */
#define ELF_EM_M32            1   /* AT&T WE 32100 */
#define ELF_EM_SPARC          2   /* SPARC */
#define ELF_EM_386            3   /* Intel 80386 */
#define ELF_EM_68K            4   /* Motorola 68000 */
#define ELF_EM_88K            5   /* Motorola 88000 */
#define ELF_EM_486            6   /* Intel 80486 */
#define ELF_EM_860            7   /* Intel 80860 */
#define ELF_EM_MIPS           8   /* MIPS I Architecture */
#define ELF_EM_S370           9   /* IBM System/370 Processor */
#define ELF_EM_MIPS_RS3_LE    10  /* MIPS RS4000 Big-endian */
#define ELF_EM_MIPS_RS4_BE    10  /* MIPS RS3000 Little-endian */
#define ELF_EM_PARISC         15  /* Hewlett-Packard PA-RISC */
#define ELF_EM_VPP500         17  /* Fujitsu VPP500 */
#define ELF_EM_SPARC32PLUS    18  /* Enhanced instruction set SPARC */
#define ELF_EM_960            19  /* Intel 80960 */
#define ELF_EM_PPC            20  /* PowerPC */
#define ELF_EM_PPC64          21  /* 64-bit PowerPC */
#define ELF_EM_S390           22  /* IBM System/390 Processor */
#define ELF_EM_SPU            23  /* [BINUTILS] Sony/Toshiba/IBM SPU */
#define ELF_EM_V800           36  /* NEC V800 */
#define ELF_EM_FR20           37  /* Fujitsu FR20 */
#define ELF_EM_RH32           38  /* TRW RH-32 */
#define ELF_EM_MCORE          39  /* [NEW NAME] Motorola RCE */
#define ELF_EM_RCE            39  /* [OLD NAME] Motorola RCE */
#define ELF_EM_ARM            40  /* Advanced RISC Machines ARM */

#define ELF_EM_SH             42  /* Hitachi SH */
#define ELF_EM_SPARCV9        43  /* SPARC Version 9 */
#define ELF_EM_TRICORE        44  /* Siemens TriCore embedded processor */
#define ELF_EM_ARC            45  /* Argonaut RISC Core, Argonaut Technologies Inc. */
#define ELF_EM_H8_300         46  /* Hitachi H8/300 */
#define ELF_EM_H8_300H        47  /* Hitachi H8/300H */
#define ELF_EM_H8S            48  /* Hitachi H8S */
#define ELF_EM_H8_500         49  /* Hitachi H8/500 */
#define ELF_EM_IA_64          50  /* Intel IA-64 processor architecture */
#define ELF_EM_MIPS_X         51  /* Stanford MIPS-X */
#define ELF_EM_COLDFIRE       52  /* Motorola ColdFire */
#define ELF_EM_68HC12         53  /* Motorola M68HC12 */
#define ELF_EM_MMA            54  /* Fujitsu MMA Multimedia Accelerator */
#define ELF_EM_PCP            55  /* Siemens PCP */
#define ELF_EM_NCPU           56  /* Sony nCPU embedded RISC processor */
#define ELF_EM_NDR1           57  /* Denso NDR1 microprocessor */
#define ELF_EM_STARCORE       58  /* Motorola Star*Core processor */
#define ELF_EM_ME16           59  /* Toyota ME16 processor */
#define ELF_EM_ST100          60  /* STMicroelectronics ST100 processor */
#define ELF_EM_TINYJ          61  /* Advanced Logic Corp. TinyJ embedded processor */
#define ELF_EM_X86_64         62  /* AMD x86-64 architecture */
#define ELF_EM_PDSP           63  /* Sony DSP Processor */
#define ELF_EM_PDP10          64  /* Digital Equipment Corp. PDP-10 */
#define ELF_EM_PDP11          65  /* Digital Equipment Corp. PDP-11 */
#define ELF_EM_FX66           66  /* Siemens FX66 microcontroller */
#define ELF_EM_ST9PLUS        67  /* STMicroelectronics ST9+ 8/16 bit microcontroller */
#define ELF_EM_ST7            68  /* STMicroelectronics ST7 8-bit microcontroller */
#define ELF_EM_68HC16         69  /* Motorola MC68HC16 Microcontroller */
#define ELF_EM_68HC11         70  /* Motorola MC68HC11 Microcontroller */
#define ELF_EM_68HC08         71  /* Motorola MC68HC08 Microcontroller */
#define ELF_EM_68HC05         72  /* Motorola MC68HC05 Microcontroller */
#define ELF_EM_SVX            73  /* Silicon Graphics SVx */
#define ELF_EM_ST19           74  /* STMicroelectronics ST19 8-bit microcontroller */
#define ELF_EM_VAX            75  /* Digital VAX */
#define ELF_EM_CRIS           76  /* Axis Communications 32-bit embedded processor */
#define ELF_EM_JAVELIN        77  /* Infineon Technologies 32-bit embedded processor */
#define ELF_EM_FIREPATH       78  /* Element 14 64-bit DSP Processor */
#define ELF_EM_ZSP            79  /* LSI Logic 16-bit DSP Processor */
#define ELF_EM_MMIX           80  /* Donald Knuth's educational 64-bit processor */
#define ELF_EM_HUANY          81  /* Harvard University machine-independent object files */
#define ELF_EM_PRISM          82  /* SiTera Prism */
#define ELF_EM_AVR            83  /* Atmel AVR 8-bit microcontroller */
#define ELF_EM_FR30           84  /* Fujitsu FR30 */
#define ELF_EM_D10V           85  /* Mitsubishi D10V */
#define ELF_EM_D30V           86  /* Mitsubishi D30V */
#define ELF_EM_V850           87  /* NEC v850 */
#define ELF_EM_M32R           88  /* Mitsubishi M32R */
#define ELF_EM_MN10300        89  /* Matsushita MN10300 */
#define ELF_EM_MN10200        90  /* Matsushita MN10200 */
#define ELF_EM_PJ             91  /* picoJava */
#define ELF_EM_OPENRISC       92  /* OpenRISC 32-bit embedded processor */
#define ELF_EM_ARC_A5         93  /* ARC Cores Tangent-A5 */
#define ELF_EM_XTENSA         94  /*Tensilica Xtensa Architecture */
#define ELF_EM_VIDEOCORE      95  /* Alphamosaic VideoCore processor */
#define ELF_EM_TMM_GPP        96  /* Thompson Multimedia General Purpose Processor */
#define ELF_EM_NS32K          97  /* National Semiconductor 32000 series */
#define ELF_EM_TPC            98  /* Tenor Network TPC processor */
#define ELF_EM_SNP1K          99  /* Trebia SNP 1000 processor */
#define ELF_EM_ST200          100 /* STMicroelectronics ST200 microcontroller */
#define ELF_EM_IP2K           101 /* Ubicom IP2xxx microcontroller */
#define ELF_EM_MAX            102 /* MAX Processor */
#define ELF_EM_CR             103 /* National Semiconductor CompactRISC microprocessor */
#define ELF_EM_F2MC16         104 /* Fujitsu F2MC16 */
#define ELF_EM_MSP430         105 /* Texas Instruments embedded microcontroller */
#define ELF_EM_BLACKFIN       106 /* Analog Devices Blackfin (DSP) processor */
#define ELF_EM_SE_CE33        107 /* S1C33 Family of Seiko Epson processors */
#define ELF_EM_SEP            108 /* Sharp embedded microprocessor */
#define ELF_EM_ARCA           109 /* Arca RISC Microprocessor */
#define ELF_EM_UNICORE        110 /* Microprocessor series from PKU-Unity Ltd.
                                   * and MPRC of Peking University
                                   */
#define EM_ALTERA_NIOS2       113 /* [BINUTILS] Altera Nios II soft-core processor */
#define EM_CRX                114 /* [BINUTILS] National Semiconductor CRX */
#define EM_CR16               115 /* [BINUTILS] National Semiconductor CompactRISC - CR16 */
#define EM_SCORE              135 /* [BINUTILS] Sunplus Score */

/* [BINUTILS] Unofficial */
#define ELF_EM_MT             0x2530 /* Morpho MT */
#define ELF_EM_DLX            0x5aa5 /* Patterson's LX */
#define ELF_EM_XC16X          0x4688 /* Infineon Technologies 16-bit microcontroller with C166-V2 core */
#define ELF_EM_ALPHA          0x9026 /* Alpha w/o ABI (Oddly, ELF_EM_OLD_ALPHA has never been used!) */
#define ELF_EM_XSTORMY16      0xad45 /* XStormy16 w/o ABI */
#define ELF_EM_M32C           0xfeb0 /* Reneseas M32C/M16C */
#define ELF_EM_IQ2000         0xfeb0 /* Vitesse IQ2000 */
#define ELF_EM_NIOS32         0xfebb /* NIOS w/o ABI */

#define ELF_EM_CYGNUS_FR30    0x3330 /* FR30 w/o ABI */
#define ELF_EM_CYGNUS_FRV     0x5441 /* FRV w/o ABI */
#define ELF_EM_CYGNUS_D10V    0x7650 /* D10V w/o ABI */
#define ELF_EM_CYGNUS_D30V    0x7676 /* D30V w/o ABI */
#define ELF_EM_CYGNUS_POWERPC 0x7676 /* PowerPC w/o ABI */
#define ELF_EM_CYGNUS_M32R    0x9041 /* M32R w/o ABI */
#define ELF_EM_CYGNUS_V850    0x9080 /* V850 w/o ABI */
#define ELF_EM_CYGNUS_MN10300 0xbeef /* MN10300 w/o ABI */
#define ELF_EM_CYGNUS_MN10200 0xdead /* MN10300 w/o ABI */
#define ELF_EM_CYGNUS_MEP     0xf00d /* Toshiba MeP w/o ABI */

/* [BINUTILS] Obsoleted */
#define ELF_EM_OLD_SPARCV9    11     /* SPARC64 */
#define ELF_EM_OLD_PPC        17     /* PowerPC */
#define ELF_EM_OLD_ALPHA      41     /* Digital Alpha */
#define ELF_EM_OLD_PJ         99     /* picoJava */
#define ELF_EM_OLD_AVR        0x1057 /* AVR w/o ABI */
#define ELF_EM_OLD_MSP430     0x1059 /* MSP430 w/o anything */
#define ELF_EM_OLD_OPENRISC   0x3426 /* OpenRISC w/o ABI */
#define ELF_EM_OLD_IP2K       0x7676 /* Ubicom IP2xxx w/o ABI */
#define ELF_EM_OLD_OR32       0x8472 /* (Deprecatd) OpenRISC */
#define ELF_EM_OLD_S390       0xa390 /* ESA/390 w/o ABI */
#define ELF_EM_OLD_XTENSA     0xabc7 /* Xtensa w/o ABI */

/*
 * ELF header version.
 */
#define ELF_EV_NONE    0
#define ELF_EV_CURRENT 1

/*
 * ELF types.
 */
#define ELF_ET_NONE   0
#define ELF_ET_REL    1
#define ELF_ET_EXEC   2
#define ELF_ET_DYN    3
#define ELF_ET_CORE   4
#define ELF_ET_LOOS   0xfe00
#define ELF_ET_HIOS   0xfeff
#define ELF_ET_LOPROC 0xff00
#define ELF_ET_HIPROC 0xffff

/*
 * ELF section types.
 */
#define ELF_SHT_NULL     0
#define ELF_SHT_PROGBITS 1
#define ELF_SHT_SYMTAB   2
#define ELF_SHT_STRTAB   3
#define ELF_SHT_RELA     4
#define ELF_SHT_HASH     5
#define ELF_SHT_DYNAMIC  6
#define ELF_SHT_NOTE     7
#define ELF_SHT_NOBITS   8
#define ELF_SHT_REL      9
#define ELF_SHT_SHLIB    10
#define ELF_SHT_DYNSYM   11
#define ELF_SHT_LOOS     0x60000000
#define ELF_SHT_HIOS     0x6fffffff
#define ELF_SHT_LOPROC   0x70000000
#define ELF_SHT_HIPROC   0x7fffffff
#define ELF_SHT_LOUSER   0x80000000
#define ELF_SHT_HIUSER   0xffffffff

/*
 * ELF section names.
 */
#define ELF_SHN_UNDEF     0x0000
#define ELF_SHN_LORESERVE 0xff00
#define ELF_SHN_LOPROC    0xff00
#define ELF_SHN_HIPROC    0xff1f
#define ELF_SHN_ABS       0xfff1
#define ELF_SHN_COMMON    0xfff2
#define ELF_SHN_HIRESERVE 0xffff

/*
 * ELF section flags.
 */
#define ELF_SHF_WRITE     0x00000001
#define ELF_SHF_ALLOC     0x00000002
#define ELF_SHF_EXECINSTR 0x00000004
#define ELF_SHF_MASKPROC  0xf0000000

/*
 * ELF symbol section indexes.
 */
#define ELF_STN_UNDEF 0

/*
 * ELF symbol binding.
 */
#define ELF_STB_LOCAL   0
#define ELF_STB_GLOBAL  1
#define ELF_STB_WEAK    2
#define ELF_STB_LOOS    10
#define ELF_STB_HIOS    12
#define ELF_STB_LOPROC  13
#define ELF_STB_HIPROC  15

/*
 * ELF symbol type.
 */
#define ELF_STT_NOTYPE  0
#define ELF_STT_OBJECT  1
#define ELF_STT_FUNC    2
#define ELF_STT_SECTION 3
#define ELF_STT_FILE    4
#define ELF_STT_LOOS    10
#define ELF_STT_HIOS    12
#define ELF_STT_LOPROC  13
#define ELF_STT_HIPROC  15

/*
 * ELF symbol type/bind macros.
 */
#define ELF_ST_BIND(i)    ( (i) >> 4)
#define ELF_ST_TYPE(i)    ( ( (i) & 0xf))
#define ELF_ST_INFO(b, t) ( ( ( (b) & 0xf ) << 4) | ( (t) & 0xf ) )

/*
 * ELF relocations - by processor.
 */

/* I386 */
#define ELF_R_386_NONE         0  /* none */
#define ELF_R_386_32           1  /* S + A */ 
#define ELF_R_386_PC32         2  /* S + A - PC */ 
#define ELF_R_386_GOT32        3  /* G + A - PC */ 
#define ELF_R_386_PLT32        4  /* L + A - PC */ 
#define ELF_R_386_COPY         5  /* none */
#define ELF_R_386_GLOB_DAT     6  /* S */
#define ELF_R_386_JMP_SLOT     7  /* S */
#define ELF_R_386_RELATIVE     8  /* B + A */
#define ELF_R_386_GOTOFF       9  /* S + A - GOT */
#define ELF_R_386_GOTPC        10 /* GOT + A - PC */

/* X86_64 */
#define ELF_R_X86_64_NONE      0  /* none */
#define ELF_R_X86_64_64        1  /* S + A */
#define ELF_R_X86_64_PC32      2  /* S + A - PC */
#define ELF_R_X86_64_GOT32     3  /* G + A */
#define ELF_R_X86_64_PLT32     4  /* L + A - PC */
#define ELF_R_X86_64_COPY      5  /* none */
#define ELF_R_X86_64_GLOB_DAT  6  /* S */
#define ELF_R_X86_64_JUMP_SLOT 7  /* S */
#define ELF_R_X86_64_RELATIVE  8  /* B + A */
#define ELF_R_X86_64_GOTPCREL  9  /* G + GOT + A - PC */
#define ELF_R_X86_64_32        10 /* S + A */
#define ELF_R_X86_64_32S       11 /* S + A */
#define ELF_R_X86_64_16        12 /* S + A */
#define ELF_R_X86_64_PC16      13 /* S + A - PC */
#define ELF_R_X86_64_8         14 /* S + A */
#define ELF_R_X86_64_PC8       15 /* S + A - PC */
#define ELF_R_X86_64_DTPMOD64  16 /* ? */
#define ELF_R_X86_64_DTPOFF64  17 /* ? */
#define ELF_R_X86_64_TPOFF64   18 /* ? */
#define ELF_R_X86_64_TLSGD     19 /* ? */
#define ELF_R_X86_64_TLSLD     20 /* ? */
#define ELF_R_X86_64_DTPOFF32  21 /* ? */
#define ELF_R_X86_64_GOTTPOFF  22 /* ? */
#define ELF_R_X86_64_TPOFF32   23 /* ? */
#define ELF_R_X86_64_PC64      24 /* S + A - PC */
#define ELF_R_X86_64_GOTOFF64  25 /* S + A - GOT */
#define ELF_R_X86_64_GOTPC32   26 /* GOT + A - PC */
/**/
#define ELF_R_X86_64_SIZE32    32 /* Z + A */
#define ELF_R_X86_64_SIZE64    33 /* Z + A */
#define ELF_R_X86_64_GOTPCREL32_TLSDESC 34 /* ? */
#define ELF_R_X86_64_TLSDESC_CALL 35 /* ? */
#define ELF_R_X86_64_TLSDESC   36 /* [2*word64] */

/* PPC */
#define ELF_R_PPC_NONE            0  /* none */
#define ELF_R_PPC_ADDR32          1  /* S + A */
#define ELF_R_PPC_ADDR24          2  /* (S + A) >> 2 */
#define ELF_R_PPC_ADDR16          3  /* S + A */
#define ELF_R_PPC_ADDR16_LO       4  /* #lo (S + A) */
#define ELF_R_PPC_ADDR16_HI       5  /* #hi (S + A) */
#define ELF_R_PPC_ADDR16_HA       6  /* #ha (S + A) */
#define ELF_R_PPC_ADDR14          7  /* (S + A) >> 2 */
#define ELF_R_PPC_ADDR14_BRTAKEN  8  /* (S + A) >> 2 */
#define ELF_R_PPC_ADDR14_BRNTAKEN 9  /* (S + A) >> 2 */
#define ELF_R_PPC_REL24           10 /* (S + A - PC) >> 2 */
#define ELF_R_PPC_REL14           11 /* (S + A - PC) >> 2 */
#define ELF_R_PPC_REL14_BRTAKEN   12 /* (S + A - PC) >> 2 */
#define ELF_R_PPC_REL14_BRNTAKEN  13 /* (S + A - PC) >> 2 */
#define ELF_R_PPC_GOT16           14 /* G + A */
#define ELF_R_PPC_GOT16_LO        15 /* #lo (G + A) */
#define ELF_R_PPC_GOT16_HI        16 /* #hi (G + A) */
#define ELF_R_PPC_GOT16_HA        17 /* #ha (G + A) */
#define ELF_R_PPC_PLTREL24        18 /* (L + A - PC) >> 2 */
#define ELF_R_PPC_COPY            19 /* none */
#define ELF_R_PPC_GLOB_DAT        20 /* S + A */
#define ELF_R_PPC_JMP_SLOT        21 /* see ABI supplement */
#define ELF_R_PPC_RELATIVE        22 /* B + A */
#define ELF_R_PPC_LOCAL24PC       23 /* see ABI supplement */
#define ELF_R_PPC_UADDR32         24 /* S + A */
#define ELF_R_PPC_UADDR16         25 /* S + A */
#define ELF_R_PPC_REL32           26 /* S + A - PC */
#define ELF_R_PPC_PLT32           27 /* L + A */
#define ELF_R_PPC_PLTREL32        28 /* L + A - PC */
#define ELF_R_PPC_PLT16_LO        29 /* #lo (L + A) */
#define ELF_R_PPC_PLT16_HI        30 /* #hi (L + A) */
#define ELF_R_PPC_PLT16_HA        31 /* #ha (L + A) */
#define ELF_R_PPC_SDAREL16        32 /* S + A - _SDA_BASE_ */
#define ELF_R_PPC_SECTOFF         33 /* R + A */
#define ELF_R_PPC_SECTOFF_LO      34 /* #lo (R + A) */
#define ELF_R_PPC_SECTOFF_HI      35 /* #hi (R + A) */
#define ELF_R_PPC_SECTOFF_HA      36 /* #ha (R + A) */
#define ELF_R_PPC_ADDR30          37 /* (S + A - PC) >> 2 */

/* PPC64 */
#define ELF_R_PPC64_NONE               0   /* none */
#define ELF_R_PPC64_ADDR32             1   /* S + A */
#define ELF_R_PPC64_ADDR24             2   /* (S + A) >> 2 */
#define ELF_R_PPC64_ADDR16             3   /* S + A */
#define ELF_R_PPC64_ADDR16_LO          4   /* #lo(S + A) */
#define ELF_R_PPC64_ADDR16_HI          5   /* #hi(S + A) */
#define ELF_R_PPC64_ADDR16_HA          6   /* #ha(S + A) */
#define ELF_R_PPC64_ADDR14             7   /* (S + A) >> 2 */
#define ELF_R_PPC64_ADDR14_BRTAKEN     8   /* (S + A) >> 2 */
#define ELF_R_PPC64_ADDR14_BRNTAKEN    9   /* (S + A) >> 2 */
#define ELF_R_PPC64_REL24              10  /* (S + A - P) >> 2 */
#define ELF_R_PPC64_REL14              11  /* (S + A - P) >> 2 */
#define ELF_R_PPC64_REL14_BRTAKEN      12  /* (S + A - P) >> 2 */
#define ELF_R_PPC64_REL14_BRNTAKEN     13  /* (S + A - P) >> 2 */
#define ELF_R_PPC64_GOT16              14  /* G */
#define ELF_R_PPC64_GOT16_LO           15  /* #lo(G) */
#define ELF_R_PPC64_GOT16_HI           16  /* #hi(G) */
#define ELF_R_PPC64_GOT16_HA           17  /* #ha(G) */
/**/
#define ELF_R_PPC64_COPY               19  /* none */
#define ELF_R_PPC64_GLOB_DAT           20  /* S + A */
#define ELF_R_PPC64_JMP_SLOT           21  /* see below */
#define ELF_R_PPC64_RELATIVE           22  /* B + A */
/**/
#define ELF_R_PPC64_UADDR32            24  /* S + A */
#define ELF_R_PPC64_UADDR16            25  /* S + A */
#define ELF_R_PPC64_REL32              26  /* S + A - P */
#define ELF_R_PPC64_PLT32              27  /* L */
#define ELF_R_PPC64_PLTREL32           28  /* L - P */
#define ELF_R_PPC64_PLT16_LO           29  /* #lo(L) */
#define ELF_R_PPC64_PLT16_HI           30  /* #hi(L) */
#define ELF_R_PPC64_PLT16_HA           31  /* #ha(L) */
/**/
#define ELF_R_PPC64_SECTOFF            33  /* R + A */
#define ELF_R_PPC64_SECTOFF_LO         34  /* #lo(R + A) */
#define ELF_R_PPC64_SECTOFF_HI         35  /* #hi(R + A) */
#define ELF_R_PPC64_SECTOFF_HA         36  /* #ha(R + A) */
#define ELF_R_PPC64_ADDR30             37  /* (S + A - P) >> 2 */
#define ELF_R_PPC64_ADDR64             38  /* S + A */
#define ELF_R_PPC64_ADDR16_HIGHER      39  /* #higher(S + A) */
#define ELF_R_PPC64_ADDR16_HIGHERA     40  /* #highera(S + A) */
#define ELF_R_PPC64_ADDR16_HIGHEST     41  /* #highest(S + A) */
#define ELF_R_PPC64_ADDR16_HIGHESTA    42  /* #highesta(S + A) */
#define ELF_R_PPC64_UADDR64            43  /* S + A */
#define ELF_R_PPC64_REL64              44  /* S + A - P */
#define ELF_R_PPC64_PLT64              45  /* L */
#define ELF_R_PPC64_PLTREL64           46  /* L - P */
#define ELF_R_PPC64_TOC16              47  /* S + A - .TOC. */
#define ELF_R_PPC64_TOC16_LO           48  /* #lo(S + A - .TOC.) */
#define ELF_R_PPC64_TOC16_HI           49  /* #hi(S + A - .TOC.) */
#define ELF_R_PPC64_TOC16_HA           50  /* #ha(S + A - .TOC.) */
#define ELF_R_PPC64_TOC                51  /* .TOC. */
#define ELF_R_PPC64_PLTGOT16           52  /* M */
#define ELF_R_PPC64_PLTGOT16_LO        53  /* #lo(M) */
#define ELF_R_PPC64_PLTGOT16_HI        54  /* #hi(M) */
#define ELF_R_PPC64_PLTGOT16_HA        55  /* #ha(M) */
#define ELF_R_PPC64_ADDR16_DS          56  /* (S + A) >> 2 */
#define ELF_R_PPC64_ADDR16_LO_DS       57  /* #lo(S + A) >> 2 */
#define ELF_R_PPC64_GOT16_DS           58  /* G >> 2 */
#define ELF_R_PPC64_GOT16_LO_DS        59  /* #lo(G) >> 2 */
#define ELF_R_PPC64_PLT16_LO_DS        60  /* #lo(L) >> 2 */
#define ELF_R_PPC64_SECTOFF_DS         61  /* (R + A) >> 2 */
#define ELF_R_PPC64_SECTOFF_LO_DS      62  /* #lo(R + A) >> 2 */
#define ELF_R_PPC64_TOC16_DS           63  /* (S + A - .TOC.) >> 2 */
#define ELF_R_PPC64_TOC16_LO_DS        64  /* #lo(S + A - .TOC.) >> 2 */
#define ELF_R_PPC64_PLTGOT16_DS        65  /* M >> 2 */
#define ELF_R_PPC64_PLTGOT16_LO_DS     66  /* #lo(M) >> 2 */
#define ELF_R_PPC64_TLS                67  /* none */
#define ELF_R_PPC64_DTPMOD64           68  /* @dtpmod */
#define ELF_R_PPC64_TPREL16            69  /* @tprel */
#define ELF_R_PPC64_TPREL16_LO         60  /* #lo(@tprel) */
#define ELF_R_PPC64_TPREL16_HI         71  /* #hi(@tprel) */
#define ELF_R_PPC64_TPREL16_HA         72  /* #ha(@tprel) */
#define ELF_R_PPC64_TPREL64            73  /* @tprel */
#define ELF_R_PPC64_DTPREL16           74  /* @dtprel */
#define ELF_R_PPC64_DTPREL16_LO        75  /* #lo(@dtprel) */
#define ELF_R_PPC64_DTPREL16_HI        76  /* #hi(@dtprel) */
#define ELF_R_PPC64_DTPREL16_HA        77  /* #ha(@dtprel) */
#define ELF_R_PPC64_DTPREL64           78  /* @dtprel */
#define ELF_R_PPC64_GOT_TLSGD16        79  /* @got@tlsgd */
#define ELF_R_PPC64_GOT_TLSGD16_LO     80  /* #lo(@got@tlsgd) */
#define ELF_R_PPC64_GOT_TLSGD16_HI     81  /* #hi(@got@tlsgd) */
#define ELF_R_PPC64_GOT_TLSGD16_HA     82  /* #ha(@got@tlsgd) */
#define ELF_R_PPC64_GOT_TLSLD16        83  /* @got@tlsld */
#define ELF_R_PPC64_GOT_TLSLD16_LO     84  /* #lo(@got@tlsld) */
#define ELF_R_PPC64_GOT_TLSLD16_HI     85  /* #hi(@got@tlsld) */
#define ELF_R_PPC64_GOT_TLSLD16_HA     86  /* #ha(@got@tlsld) */
#define ELF_R_PPC64_GOT_TPREL16_DS     87  /* @got@tprel */
#define ELF_R_PPC64_GOT_TPREL16_LO_DS  88  /* #lo(@got@tprel) */
#define ELF_R_PPC64_GOT_TPREL16_HI     89  /* #hi(@got@tprel) */
#define ELF_R_PPC64_GOT_TPREL16_HA     90  /* #ha(@got@tprel) */
#define ELF_R_PPC64_GOT_DTPREL16_DS    91  /* @got@dtprel */
#define ELF_R_PPC64_GOT_DTPREL16_LO_DS 92  /* #lo(@got@dtprel) */
#define ELF_R_PPC64_GOT_DTPREL16_HI    93  /* #hi(@got@dtprel) */
#define ELF_R_PPC64_GOT_DTPREL16_HA    94  /* #ha(@got@dtprel) */
#define ELF_R_PPC64_TPREL16_DS         95  /* @tprel */
#define ELF_R_PPC64_TPREL16_LO_DS      96  /* #lo(@tprel) */
#define ELF_R_PPC64_TPREL16_HIGHER     97  /* #higher(@tprel) */
#define ELF_R_PPC64_TPREL16_HIGHERA    98  /* #highera(@tprel) */
#define ELF_R_PPC64_TPREL16_HIGHEST    99  /* #highest(@tprel) */
#define ELF_R_PPC64_TPREL16_HIGHESTA   100 /* #highesta(@tprel) */
#define ELF_R_PPC64_DTPREL16_DS        101 /* @dtprel */
#define ELF_R_PPC64_DTPREL16_LO_DS     102 /* #lo(@dtprel) */
#define ELF_R_PPC64_DTPREL16_HIGHER    103 /* #higher(@dtprel) */
#define ELF_R_PPC64_DTPREL16_HIGHERA   104 /* #highera(@dtprel) */
#define ELF_R_PPC64_DTPREL16_HIGHEST   105 /* #highest(@dtprel) */
#define ELF_R_PPC64_DTPREL16_HIGHESTA  106 /* #highesta(@dtprel) */

/* MIPS */
#define ELF_R_MIPS_NONE        0  /* none */
#define ELF_R_MIPS_16          1  /* S + sext(A) */
#define ELF_R_MIPS_32          2  /* S + A */
#define ELF_R_MIPS_REL32       3  /* A - EA + S */
#define ELF_R_MIPS_26          4  /* local: (((A << 2) | (PC & 0xf0000000)) + S) >> 2
                                   * extrn: (sext (A << 2) + S) >> 2
                                   */
#define ELF_R_MIPS_HI16        5  /* loc/ext: ((AHL + S) - (sint16)(AHL + S)) >> 16 
                                   * gp_disp: ((AHL + GP - PC) - (sint16)(AHL + GP - PC)) >> 16
                                   */
#define ELF_R_MIPS_LO16        6  /* loc/ext: AHL + S 
                                   * gp_disp: AHL + GP - PC + 4
                                   */
#define ELF_R_MIPS_GPREL16     7  /* local: sext (A) + S + GP0 - GP
                                   * extrn: sext (A) + S + GP
                                   */
#define ELF_R_MIPS_LITERAL     8  /* sext (A) + L */
#define ELF_R_MIPS_GOT16       9  /* extrn: G
                                   * local: see ABI supplement, shall be followed
                                   *        by an ELF_R_MIPS_LO16.
                                   */
#define ELF_R_MIPS_PC16        10 /* sext (A) + S - PC */
#define ELF_R_MIPS_CALL16      11 /* G */
#define ELF_R_MIPS_GPREL32     12 /* A + S + GP0 - GP */
/**/
#define ELF_R_MIPS_GOTHI16     21 /* ( (G - (sint16)G) >> 16) + A */
#define ELF_R_MIPS_GOTLO16     22 /* G & 0xffff */
/**/
#define ELF_R_MIPS_CALLHI16    30 /* ( (G - (sint16)G) >> 16) + A */
#define ELF_R_MIPS_CALLLO16    31 /* G & 0xffff */

/* ARM */
#define ELF_R_ARM_NONE              0   /* none */
#define ELF_R_ARM_PC24              1   /* ( (S + A) | T) - PC */
#define ELF_R_ARM_ABS32             2   /* (S + A) | T) */
#define ELF_R_ARM_REL32             3   /* ( (S + A) | T) - PC */
#define ELF_R_ARM_LDR_PC_G0         4   /* S + A - PC */
#define ELF_R_ARM_ABS16             5   /* S + A */
#define ELF_R_ARM_ABS12             6   /* S + A */
#define ELF_R_ARM_THM_ABS5          7   /* S + A */
#define ELF_R_ARM_ABS8              8   /* S + A */
#define ELF_R_ARM_SBREL32           9   /* ( (S + A) | T) - B (S) */
#define ELF_R_ARM_THM_CALL          10  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_THM_PC8           11  /* (S + A) - PCa */
#define ELF_R_ARM_BREL_ADJ          12  /* dB (S) + A */
#define ELF_R_ARM_TLS_DESC          13  /* ? */
/**/
#define ELF_R_ARM_TLS_DTPMOD32      17  /* Module[S] */
#define ELF_R_ARM_TLS_DTPOFF32      18  /* S + A - TLS */
#define ELF_R_ARM_TLS_TPOFF32       19  /* S + A - TP */
#define ELF_R_ARM_TLS_COPY          20  /* none */
#define ELF_R_ARM_GLOB_DAT          21  /* (S + A) | T */
#define ELF_R_ARM_JUMP_SLOT         22  /* (S + A) | T */
#define ELF_R_ARM_RELATIVE          23  /* B (S) + A (see ABI supplement) */
#define ELF_R_ARM_GOTOFF32          24  /* ( (S + A) | T) - GOT_ORG */
#define ELF_R_ARM_BASE_PREL         25  /* B (S) + A - PC */
#define ELF_R_ARM_GOT_BREL          26  /* GOT (S) + A - GOT_ORG */
#define ELF_R_ARM_PLT32             27  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_CALL              28  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_JUMP24            29  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_THM_JUMP24        30  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_BASE_ABS          31  /* B (S) + A */
/**/
#define ELF_R_ARM_SBREL_11_0_NC     35  /* S + A - B (S) */
#define ELF_R_ARM_SBREL_19_12_NC    36  /* S + A - B (S) */
#define ELF_R_ARM_SBREL_27_20_CK    37  /* S + A - B (S) */
#define ELF_R_ARM_TARGET1           38  /* (S + A) | T or ( (S + A) | T) - PC */
#define ELF_R_ARM_SBREL31           39  /* ( (S + A) | T) - B (S) */
#define ELF_R_ARM_V4BX              40  /* ? */
#define ELF_R_ARM_TARGET2           41  /* ? */
#define ELF_R_ARM_PREL31            42  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_MOVW_ABS_NC       43  /* (S + A) | T */
#define ELF_R_ARM_MOVT_ABS          44  /* S + A */
#define ELF_R_ARM_MOVW_PREL_NC      45  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_MOVT_PREL         46  /* S + A - PC */
#define ELF_R_ARM_THM_MOVW_ABS_NC   47  /* (S + A) | T */
#define ELF_R_ARM_THM_MOVT_ABS      48  /* S + A */
#define ELF_R_ARM_THM_MOVW_PREL_NC  49  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_THM_MOVT_PREL     50  /* S + A - PC */
#define ELF_R_ARM_THM_JUMP19        51  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_THM_JUMP6         52  /* S + A - PC */
#define ELF_R_ARM_THM_ALU_PREL_11_0 53  /* ( (S + A) | T) - PCa */
#define ELF_R_ARM_THM_PC12          54  /* S + A - PCa */
#define ELF_R_ARM_ABS32_NOI         55  /* S + A */
#define ELF_R_ARM_REL32_NOI         56  /* S + A - PC */
#define ELF_R_ARM_ALU_PC_G0_NC      57  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_ALU_PC_G0         58  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_ALU_PC_G1_NC      59  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_ALU_PC_G1         60  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_ALU_PC_G2         61  /* ( (S + A) | T) - PC */
#define ELF_R_ARM_LDR_PC_G1         62  /* S + A - PC */
#define ELF_R_ARM_LDR_PC_G2         63  /* S + A - PC */
#define ELF_R_ARM_LDRS_PC_G0        64  /* S + A - PC */
#define ELF_R_ARM_LDRS_PC_G1        65  /* S + A - PC */
#define ELF_R_ARM_LDRS_PC_G2        66  /* S + A - PC */
#define ELF_R_ARM_LDC_PC_G0         67  /* S + A - PC */
#define ELF_R_ARM_LDC_PC_G1         68  /* S + A - PC */
#define ELF_R_ARM_LDC_PC_G2         69  /* S + A - PC */
#define ELF_R_ARM_ALU_SB_G0_NC      70  /* ( (S + A) | T) - B (S) */
#define ELF_R_ARM_ALU_SB_G0         71  /* ( (S + A) | T) - B (S) */
#define ELF_R_ARM_ALU_SB_G1_NC      72  /* ( (S + A) | T) - B (S) */
#define ELF_R_ARM_ALU_SB_G1         73  /* ( (S + A) | T) - B (S) */
#define ELF_R_ARM_ALU_SB_G2         74  /* ( (S + A) | T) - B (S) */
#define ELF_R_ARM_LDR_SB_G0         75  /* S + A - B (S) */
#define ELF_R_ARM_LDR_SB_G1         76  /* S + A - B (S) */
#define ELF_R_ARM_LDR_SB_G2         77  /* S + A - B (S) */
#define ELF_R_ARM_LDRS_SB_G0        78  /* S + A - B (S) */
#define ELF_R_ARM_LDRS_SB_G1        79  /* S + A - B (S) */
#define ELF_R_ARM_LDRS_SB_G2        80  /* S + A - B (S) */
#define ELF_R_ARM_LDC_SB_G0         81  /* S + A - B (S) */
#define ELF_R_ARM_LDC_SB_G1         82  /* S + A - B (S) */
#define ELF_R_ARM_LDC_SB_G2         83  /* S + A - B (S) */
#define ELF_R_ARM_MOVW_BREL_NC      84  /* ( (S + A) | T) - B (S) */
#define ELF_R_ARM_MOVT_BREL         85  /* S + A - B (S) */
#define ELF_R_ARM_MOVW_BREL         86  /* ( (S + A) | T) - B (S) */
#define ELF_R_ARM_THM_MOVW_BREL_NC  87  /* ( (S + A) | T) - B (S) */
#define ELF_R_ARM_THM_MOVT_BREL     88  /* S + A - B (S) */
#define ELF_R_ARM_THM_MOVW_BREL     89  /* ( (S + A) | T) - B (S) */
#define ELF_R_ARM_TLS_GOTDESC       90  /* ? */
#define ELF_R_ARM_TLS_CALL          91  /* ? */
#define ELF_R_ARM_TLS_DESCSEQ       92  /* TLS relaxation */
#define ELF_R_ARM_THM_TLS_CALL      93  /* ? */
#define ELF_R_ARM_PLT32_ABS         94  /* PLT (S) + A */
#define ELF_R_ARM_GOT_ABS           95  /* GOT (S) + A */
#define ELF_R_ARM_GOT_PREL          96  /* GOT (S) + A - PC */
#define ELF_R_ARM_GOT_BREL12        97  /* GOT (S) + A - GOT_ORG */
#define ELF_R_ARM_GOTOFF12          98  /* S + A - GOT_ORG */
#define ELF_R_ARM_GOTRELAX          99  /* ? */
/**/
#define ELF_R_ARM_THM_JUMP11        102 /* S + A - PC */
#define ELF_R_ARM_THM_JUMP8         103 /* S + A - PC */
#define ELF_R_ARM_TLS_GD32          104 /* GOT (S) + A - PC */
#define ELF_R_ARM_TLS_LDM32         105 /* GOT (S) + A - PC */
#define ELF_R_ARM_TLS_LDO32         106 /* S + A - TLS */
#define ELF_R_ARM_TLS_IE32          107 /* GOT (S) + A - PC */
#define ELF_R_ARM_TLS_LE32          108 /* S + A - TP */
#define ELF_R_ARM_TLS_LDO12         109 /* S + A - TLS */
#define ELF_R_ARM_TLS_LE12          110 /* S + A - TP */
#define ELF_R_ARM_TLS_IE12GP        111 /* GOT (S) + A - GOT_ORG */
#define ELF_R_ARM_PRIVATE_0         112
#define ELF_R_ARM_PRIVATE_1         113
#define ELF_R_ARM_PRIVATE_2         114
#define ELF_R_ARM_PRIVATE_3         115
#define ELF_R_ARM_PRIVATE_4         116
#define ELF_R_ARM_PRIVATE_5         117
#define ELF_R_ARM_PRIVATE_6         118
#define ELF_R_ARM_PRIVATE_7         119
#define ELF_R_ARM_PRIVATE_8         120
#define ELF_R_ARM_PRIVATE_9         121
#define ELF_R_ARM_PRIVATE_10        122
#define ELF_R_ARM_PRIVATE_11        123
#define ELF_R_ARM_PRIVATE_12        124
#define ELF_R_ARM_PRIVATE_13        125
#define ELF_R_ARM_PRIVATE_14        126
#define ELF_R_ARM_PRIVATE_15        127
#define ELF_R_ARM_ME_TOO            128 /* ? */
#define ELF_R_ARM_THM_TLS_DESCSEQ16 129 /* ? */
#define ELF_R_ARM_THM_TLS_DESCSEQ32 130 /* ? */

/* SPARC */
#define ELF_R_SPARC_NONE     0  /* none */
#define ELF_R_SPARC_8        1  /* S + A */
#define ELF_R_SPARC_16       2  /* S + A */
#define ELF_R_SPARC_32       3  /* S + A */
#define ELF_R_SPARC_DISP8    4  /* S + A - PC */
#define ELF_R_SPARC_DISP16   5  /* S + A - PC */
#define ELF_R_SPARC_DISP32   6  /* S + A - PC */
#define ELF_R_SPARC_WDISP30  7  /* (S + A - PC) >> 2 */
#define ELF_R_SPARC_WDISP22  8  /* (S + A - PC) >> 2 */
#define ELF_R_SPARC_HI22     9  /* (S + A) >> 10 */
#define ELF_R_SPARC_22       10 /* S + A */
#define ELF_R_SPARC_13       11 /* S + A */
#define ELF_R_SPARC_LO10     12 /* (S + A) & 0x3ff */
#define ELF_R_SPARC_GOT10    13 /* G & 0x3ff */
#define ELF_R_SPARC_GOT13    14 /* G */
#define ELF_R_SPARC_GOT22    15 /* G >> 10 */
#define ELF_R_SPARC_PC10     16 /* (S + A - PC) & 0x3ff */
#define ELF_R_SPARC_PC22     17 /* (S + A - PC) >> 10 */
#define ELF_R_SPARC_WPLT30   18 /* (L + A - PC) >> 2 */
#define ELF_R_SPARC_COPY     19 /* none */
#define ELF_R_SPARC_GLOB_DAT 20 /* S + A */
#define ELF_R_SPARC_JMP_SLOT 21 /* see ABI supplement */
#define ELF_R_SPARC_RELATIVE 22 /* B + A */
#define ELF_R_SPARC_UA32     23 /* S + A */

/* S/390 */
#define ELF_R_390_NONE     0  /* none */
#define ELF_R_390_8        1  /* S + A */
#define ELF_R_390_12       2  /* S + A */
#define ELF_R_390_16       3  /* S + A */
#define ELF_R_390_32       4  /* S + A */
#define ELF_R_390_PC32     5  /* S + A - PC */
#define ELF_R_390_GOT12    6  /* O + A */
#define ELF_R_390_GOT32    7  /* O + A */
#define ELF_R_390_PLT32    8  /* L + A */
#define ELF_R_390_COPY     9  /* none (see ABI supplement) */
#define ELF_R_390_GLOB_DAT 10 /* S + A (see ABI supplement) */
#define ELF_R_390_JMP_SLOT 11 /* none (see ABI supplement) */
#define ELF_R_390_RELATIVE 12 /* B + A (see ABI supplement) */
#define ELF_R_390_GOTOFF   13 /* S + A - G */
#define ELF_R_390_GOTPC    14 /* G + A - PC */
#define ELF_R_390_GOT16    15 /* O + A */
#define ELF_R_390_PC16     16 /* S + A - PC */
#define ELF_R_390_PC16DBL  17 /* (S + A - PC) >> 1 */
#define ELF_R_390_PLT16DBL 18 /* (L + A - PC) >> 1 */
#define ELF_R_390_PC32DBL  19 /* (S + A - PC) >> 1 */
#define ELF_R_390_PLT32DBL 20 /* (L + A - PC) >> 1 */
#define ELF_R_390_GOTPCDBL 21 /* (G + A - PC) >> 1 */
#define ELF_R_390_64       22 /* S + A */
#define ELF_R_390_PC64     23 /* S + A - PC */
#define ELF_R_390_GOT64    24 /* O + A */
#define ELF_R_390_PLT64    25 /* L + A */
#define ELF_R_390_GOTENT   26 /* (G + O + A - PC) >> 1 */

/* VAX */
#define ELF_R_VAX_NONE     0  /* none */
#define ELF_R_VAX_32       1  /* S + A */
#define ELF_R_VAX_16       2  /* S + A */
#define ELF_R_VAX_8        3  /* S + A */
#define ELF_R_VAX_PC32     4  /* S + A - PC */
#define ELF_R_VAX_PC16     5  /* S + A - PC */
#define ELF_R_VAX_PC8      6  /* S + A - PC */
#define ELF_R_VAX_GOT32    7  /* G + A - PC */
/**/
#define ELF_R_VAX_PLT32    13 /* L + A - PC */
/**/
#define ELF_R_VAX_COPY     19 /* none */
#define ELF_R_VAX_GLOB_DAT 20 /* S */
#define ELF_R_VAX_JMP_SLOT 21 /* S */
#define ELF_R_VAX_RELATIVE 22 /* B + A */

#ifdef __cplusplus
extern "C" {
#endif

loader_status_t
LOADERCALL
elf_get_loader_arch(elf_half_t        machine,
                    loader_boolean_t  big_endian,
                    loader_uint32_t  *format);

loader_status_t
LOADERCALL
elf_get_loader_type(elf_half_t       type,
                    loader_uint32_t *format);

elf_word_t
LOADERCALL
elf_hash(char const *name);

#ifdef __cplusplus
}
#endif

__LOADER_FAST_INLINE loader_boolean_t
elf_is_valid(elf_small_t const *ident)
{
  return (   ident[ELF_EI_MAG0] == ELF_ELFMAG0
          && ident[ELF_EI_MAG1] == ELF_ELFMAG1
          && ident[ELF_EI_MAG2] == ELF_ELFMAG2
          && ident[ELF_EI_MAG3] == ELF_ELFMAG3);
}

#endif  /* !__laoder_elf_common_h */
