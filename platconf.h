/* platconf.h
 * Configuration of additional platform dependencies
 *
 * Modification history:
 * 4-6-95     Garrick Meeker              Created
 *
 */

#ifndef _PLATCONF_H_
#define _PLATCONF_H_

#define CC_USE_BTNBAR

#define cpp_cat(a,b) a ## b

// For creating icons and bitmaps
// There are separate macros because XPM is used for icons
#ifdef wx_x
#define ICON_NAME(name) cpp_cat(name,_xpm)
#define BITMAP_NAME(name) (char *)cpp_cat(name,_bits), cpp_cat(name,_width), cpp_cat(name,_height)
#endif
#ifdef wx_msw
#define ICON_NAME(name) #name
#define BITMAP_NAME(name) #name
#endif

// Run external programs to print or just write to files
#ifdef wx_x
#define PRINT__RUN_CMD
#endif

// Character used in paths
#ifdef wx_msw
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

#define AUTOSAVE_CHAR "#"

// Wildcard in file selector
#ifdef wx_msw
#define FILE_WILDCARDS "All shows (*.shw;*.mas)|*.shw;*.mas|New shows (*.shw)|*.shw|Old shows (*.mas)|*.mas"
#define FILE_SAVE_WILDCARDS "New shows (*.shw)|*.shw"
#else
#define FILE_WILDCARDS "*.shw"
#define FILE_SAVE_WILDCARDS "*.shw"
#endif

// Need to handle DOS-style text
#ifdef wx_msw
#define TEXT_DOS_STYLE
#endif

// SetSizeHints doesn't work in Watcom for Win 3.1
#ifdef wx_msw
#ifndef WIN32
#define BUGGY_SIZE_HINTS
#endif
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

#define put_big_word(a,v) {((unsigned char *)(a))[0]=((unsigned short)(v))>>8;((unsigned char *)(a))[1]=((unsigned short)(v));}
#define put_big_long(a,v) {((unsigned char *)(a))[0]=((unsigned long)(v))>>24;((unsigned char *)(a))[1]=((unsigned long)(v))>>16;((unsigned char *)(a))[2]=((unsigned long)(v))>>8;((unsigned char *)(a))[3]=((unsigned long)(v));}

#define get_big_word(a) (((((unsigned char *)(a))[0] & 0xFF) << 8) | (((unsigned char *)(a))[1] & 0xFF))
#define get_big_long(a) (((((unsigned char *)(a))[0] & 0xFF) << 24) | ((((unsigned char *)(a))[1] & 0xFF) << 16) | ((((unsigned char *)(a))[2] & 0xFF) << 8) | (((unsigned char *)(a))[3] & 0xFF))

#define put_lil_word(a,v) {((unsigned char *)(a))[0]=((unsigned short)(v));((unsigned char *)(a))[1]=((unsigned short)(v))>>8;}
#define get_lil_word(a) ((((unsigned char *)(a))[0] & 0xFF) | ((((unsigned char *)(a))[1] & 0xFF) << 8))

#endif
