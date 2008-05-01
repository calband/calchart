/* platconf.h
 * Configuration of additional platform dependencies
 *
 * Modification history:
 * 4-6-95     Garrick Meeker              Created
 *
 */

/*
   Copyright (C) 1995-2008  Garrick Brian Meeker

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _PLATCONF_H_
#define _PLATCONF_H_

#if defined(_MSC_VER) && !defined(__MWERKS__)
#include <stddef.h>
#include <sys/types.h>
#if !defined(INT8_MIN) && !defined(_STDINT_H)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
#define INT8_MIN		(-128)
#define INT16_MIN		(-32767-1)
#define INT32_MIN		(-2147483647-1)
#define INT8_MAX		(127)
#define INT16_MAX		(32767)
#define INT32_MAX		(2147483647)
#define UINT8_MAX		(255)
#define UINT16_MAX		(65535)
#define UINT32_MAX		(4294967295U)
#endif
#elif sgi
#include <sys/types.h>
#include <inttypes.h>
#else
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
#endif
#include <stddef.h>
#include <stdint.h>
#endif

#define cpp_cat(a,b) a ## b

// For creating icons and bitmaps
// There are separate macros because XPM is used for icons
#ifdef __WXMOTIF__
#define __CC_INCLUDE_BITMAPS__
#define ICON_NAME(name) cpp_cat(name,_xpm)
#define BITMAP_NAME(name) (char *)cpp_cat(name,_bits), cpp_cat(name,_width), cpp_cat(name,_height)
#endif
#ifdef __WXMSW__
#define ICON_NAME(name) #name
#define BITMAP_NAME(name) #name
#endif

// Run external programs to print or just write to files
#ifdef __WXMOTIF__
#define PRINT__RUN_CMD
#endif

// Character used in paths
#ifdef __WXMSW__
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

#ifdef __WXMSW__
#define AUTOSAVE_VAR "$TEMP"
#define AUTOSAVE_DIR "C:"
#else
#define AUTOSAVE_VAR "$TMPDIR"
#define AUTOSAVE_DIR "/tmp"
#endif

// Wildcard in file selector
#ifdef __WXMSW__
#define FILE_WILDCARDS "All shows (*.shw;*.mas)|*.shw;*.mas|New shows (*.shw)|*.shw|Old shows (*.mas)|*.mas"
#define FILE_SAVE_WILDCARDS "New shows (*.shw)|*.shw"
#else
#define FILE_WILDCARDS "*.shw"
#define FILE_SAVE_WILDCARDS "*.shw"
#endif

// Need to handle DOS-style text
#ifdef __WXMSW__
#define TEXT_DOS_STYLE
#endif

/*****************************************
 * platform independent definitions follow
 *****************************************/

// random definitions that might be missing

#ifndef ABS
#define ABS(x) (((x) < 0) ? -(x):(x))
#endif
#ifndef MIN
#define MIN(a,b) ((a) > (b) ? (b):(a))
#endif
#ifndef MAX
#define MAX(a,b) ((a) < (b) ? (b):(a))
#endif
#ifndef PI
#define PI 3.1415927
#endif
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

// little/big endian conversion

#define put_big_word(a,v) {((uint8_t *)(a))[0]=((uint16_t)(v))>>8;((uint8_t *)(a))[1]=((uint16_t)(v));}
#define put_big_long(a,v) {((uint8_t *)(a))[0]=((uint32_t)(v))>>24;((uint8_t *)(a))[1]=((uint32_t)(v))>>16;((uint8_t *)(a))[2]=((uint32_t)(v))>>8;((uint8_t *)(a))[3]=((uint32_t)(v));}

#define get_big_word(a) (((((uint8_t *)(a))[0] & 0xFF) << 8) | (((uint8_t *)(a))[1] & 0xFF))
#define get_big_long(a) (((((uint8_t *)(a))[0] & 0xFF) << 24) | ((((uint8_t *)(a))[1] & 0xFF) << 16) | ((((uint8_t *)(a))[2] & 0xFF) << 8) | (((uint8_t *)(a))[3] & 0xFF))

#define put_lil_word(a,v) {((uint8_t *)(a))[0]=((uint16_t)(v));((uint8_t *)(a))[1]=((uint16_t)(v))>>8;}
#define get_lil_word(a) ((((uint8_t *)(a))[0] & 0xFF) | ((((uint8_t *)(a))[1] & 0xFF) << 8))

#endif
