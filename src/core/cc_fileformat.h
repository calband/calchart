#pragma once
/*
 * cc_fileformat.h
 * File format layout and utilities
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace CalChart {

struct Version_3_3_and_earlier {
};
struct Current_version_and_later {
};

// clang-format off
// Description of the CalChart file format layout, in modified Extended
// Backus–Naur Form
// version 3.4.0 to current
//
// Most all blocks are generally in the form:
// {4-char block name} {4-byte size of block} {BlockData} {'E''N''D'' '} {4-char
// block name}
//   This allows a parser to quickly jump to the end of a block it doesn't know
//   about, to allow
//   for future improvements.
//
//   Where {}* means 0 or more;
//         [] means 0 or 1;
//         /**/ means could contain future blocks that could be ignored.
//
// show               = START , SHOW ;
// START              = INGL_INGL , INGL_VERS ;
// SHOW               = INGL_SHOW , BigEndianInt32(DataTill_SHOW_END) , SHOW_DATA , SHOW_END ;
// SHOW_DATA          = NUM_MARCH , LABEL , [ DESCRIPTION ] , { SHEET }* , [ SELECTION ], CURRENT_SHEET, /**/ ;
// SHOW_END           = INGL_END , INGL_SHOW ;
// NUM_MARCH          = INGL_SIZE , BigEndianInt32(4) , NUM_MARCH_DATA , NUM_MARCH_END ;
// NUM_MARCH_DATA     = BigEndianInt32( number of marchers ) ;
// NUM_MARCH_END      = INGL_END , INGL_SIZE ;
// LABEL              = INGL_LABL , BigEndianInt32(DataTill_LABEL_END) , LABEL_DATA , LABEL_END ;
// LABEL_DATA         = { Null-terminated_char* }* ;
// LABEL_END          = INGL_END , INGL_LABL ;
// DESCRIPTION        = INGL_DESC , BigEndianInt32(DataTill_DESCRIPTION_END) , DESCRIPTION_DATA , DESCRIPTION_END ;
// DESCRIPTION_DATA   = Null-terminated_char* ;
// DESCRIPTION_END    = INGL_END , INGL_DESC ;
// SHOW_MODE          = INGL_MODE , BigEndianInt32(DataTill_SHOW_MODE_END) , SHOW_MODE_DATA , SHOW_MODE_END ;
// SHOW_MODE_DATA     = ShowModeParseData ;
// SHOW_MODE_END      = INGL_END , INGL_MODE ;
// SHEET              = INGL_SHET , BigEndianInt32(DataTill_SHEET_END) , SHEET_DATA , SHEET_END ;
// SHEET_DATA         = NAME , DURATION , ALL_POINTS , CONTINUITY , PRINT_CONTINUITY , /**/ ;
// SHEET_END          = INGL_END , INGL_SHET ;
// SELECTION          = INGL_SELE , BigEndianInt32(DataTill_SELECTION_END) , SELECTION_DATA , SELECTION_END ;
// SELECTION_DATA     = BigEndianInt32(SelectedPoint)* ;
// SELECTION_END      = INGL_END , INGL_SELE ;
// CURRENT_SHEET      = INGL_CURR , BigEndianInt32(4) , CURRENT_SHEET_DATA , NUM_MARCH_END ;
// CURRENT_SHEET_DATA = BigEndianInt32( page selected ) ;
// CURRENT_SHEET_END  = INGL_END , INGL_CURR ;
// NAME               = INGL_NAME , BigEndianInt32(DataTill_NAME_END) , NAME_DATA , NAME_END ;
// NAME_DATA          = Null-terminated_char* ;
// NAME_END           = INGL_END , INGL_NAME ;
// DURATION           = INGL_DURA , BigEndianInt32(4) , DURATION_DATA , DURATION_END;
// DURATION_DATA      = BigEndianInt32(number of beats) ;
// DURATION_END       = INGL_END , INGL_DURA ;
// ALL_POINTS         = INGL_PNTS , BigEndianInt32(DataTill_ALL_POINTS_END) , ALL_POINTS_DATA , ALL_POINTS_END ;
// ALL_POINTS_DATA    = { EACH_POINT_DATA }* ;
// ALL_POINTS_END     = INGL_END , INGL_PNTS ;
// EACH_POINT_DATA    = BigEndianInt8(Size_rest_of_EACH_POINT_DATA) , POSITION_DATA , REF_POSITION_DATA , POINT_SYMBOL_DATA , POINT_LABEL_FLIP ;
// POSITION_DATA      = BigEndianInt16( x ) , BigEndianInt16( y ) ;
// REF_POSITION_DATA  = BigEndianInt8( num ref pts ) , { BigEndianInt8( which reference point ) , BigEndianInt16( x ) , BigEndianInt16( y ) }* ;
// POINT_SYMBOL_DATA  = BigEndianInt8( which symbol type ) ;
// POINT_LABEL_FLIP_DATA = BigEndianInt8( label flipped ) ;
// CONTINUITY         = INGL_CONT , BigEndianInt32(DataTill_CONTINUITY_END)) , CONTINUITY_DATA , CONTINUITY_END ;
// CONTINUITY_DATA    = { EACH_CONTINUITY }* ;
// CONTINUITY_END     = INGL_END , INGL_CONT ;
// EACH_CONTINUITY    = INGL_ECNT , BigEndianInt32(DataTill_EACH_CONTINUITY_END)) , EACH_CONTINUITY_DATA , EACH_CONTINUITY_END ;
// EACH_CONTINUITY_DATA = BigEndianInt8( symbol ) , Null-terminated char* ;
// EACH_CONTINUITY_END  INGL_END , INGL_ECONT ;
// VCONTINUITY         = INGL_VCNT , BigEndianInt32(DataTill_VCONTINUITY_END)) , VCONTINUITY_DATA , VCONTINUITY_END ;
// VCONTINUITY_DATA    = { EACH_VCONTINUITY }* ;
// VCONTINUITY_END     = INGL_END , INGL_VCNT ;
// EACH_VCONTINUITY    = INGL_EVCT , BigEndianInt32(DataTill_EACH_VCONTINUITY_END)) , EACH_VCONTINUITY_DATA , EACH_VCONTINUITY_END ;
// EACH_VCONTINUITY_DATA = VisualContParseData ;
// EACH_VCONTINUITY_END  INGL_END , INGL_EVCT ;
// PRINT_CONTINUITY   = INGL_PCNT , BigEndianInt32(DataTill_PRINT_CONTINUITY_END)) , PRINT_CONTINUITY_DATA , PRINT_CONTINUITY_END ;
// PRINT_CONTINUITY_DATA = { Null-terminated char* }* ;
// PRINT_CONTINUITY_END = INGL_END , INGL_PCNT ;
//
// INGL_INGL = 'I','N','G','L' ;
// INGL_GURK = 'G','U','R','K' ;
// INGL_SHOW = 'S','H','O','W' ;
// INGL_SHET = 'S','H','E','T' ;
// INGL_SELE = 'S','E','L','E' ;
// INGL_SIZE = 'S','I','Z','E' ;
// INGL_CURR = 'C','U','R','R' ;
// INGL_LABL = 'L','A','B','L' ;
// INGL_MODE = 'M','O','D','E' ;
// INGL_DESC = 'D','E','S','C' ;
// INGL_NAME = 'N','A','M','E' ;
// INGL_DURA = 'D','U','R','A' ;
// INGL_POS  = 'P','O','S',' ' ;
// INGL_SYMB = 'S','Y','M','B' ;
// INGL_TYPE = 'T','Y','P','E' ;
// INGL_REFP = 'R','E','F','P' ;
// INGL_CONT = 'C','O','N','T' ;
// INGL_PCNT = 'P','C','N','T' ;
// INGL_VCNT = 'V','C','N','T' ;
// INGL_EVCT = 'E','V','C','T' ;
// INGL_END  = 'E','N','D',' ' ;

// Description of the CalChart file format layout, in modified Extended
// Backus–Naur Form
// version 3.1.0 to 3.3.5
// show = START , SHOW ;
// START = INGL_INGL , INGL_GURK ;
// SHOW = INGL_SHOW , SIZE , { LABEL } , { DESCRIPTION } , { SHEET } , SHOW_END ;
// SIZE = INGL_SIZE , BigEndianInt32(4) , BigEndianInt32( number of marchers ) ;
// LABEL = INGL_LABL , BigEndianInt32(Sizeof(LABEL_DATA)) , LABEL_DATA ;
// LABEL_DATA = { Null-terminated char* } ;
// DESCRIPTION = INGL_DESC , BigEndianInt32(Sizeof(DESCRIPTION_DATA)) , DESCRIPTION_DATA ;
// DESCRIPTION_DATA = Null-terminated char* ;
// SHEET = INGL_GURK , INGL_SHET , NAME , DURATION , POSITION , [ REF_POSITION ] , [ POINT_SYMBOL ] , [ POINT_CONT_INDEX ] , [ POINT_LABEL_FLIP ] ,  CONTINUITY } , SHEET_END ;
// NAME = INGL_NAME , BigEndianInt32(Sizeof(NAME_DATA)) , NAME_DATA ;
// NAME_DATA = Null-terminated char* ;
// DURATION = INGL_DURA , BigEndianInt32(4) , BigEndianInt32(number of beats) ;
// POSITION = INGL_POS , BigEndianInt32(Sizeof(POSITION_DATA)) , POSITION_DATA ;
// POSITION_DATA = { BigEndianInt16( x ) , BigEndianInt16( y ) } ;
// REF_POSITION = INGL_REFP , BigEndianInt32(Sizeof(REF_POSITION_DATA)) , REF_POSITION_DATA ;
// REF_POSITION_DATA = BigEndianInt16( which reference point ) , { BigEndianInt16( x ) , BigEndianInt16( y ) } ;
// POINT_SYMBOL = INGL_SYMB , BigEndianInt32(Sizeof(POINT_SYMBOL_DATA)) , POINT_SYMBOL_DATA ;
// POINT_SYMBOL_DATA = { BigEndianInt8( which symbol type ) } ;
// POINT_CONT_INDEX = INGL_TYPE , BigEndianInt32(Sizeof(POINT_CONT_INDEX_DATA)) , POINT_CONT_INDEX_DATA ;
// POINT_CONT_INDEX_DATA = { BigEndianInt8( which continuity index ) } ;
// POINT_LABEL_FLIP = INGL_LABL , BigEndianInt32(Sizeof(POINT_LABEL_FLIP_DATA)) , POINT_LABEL_FLIP_DATA ;
// POINT_LABEL_FLIP_DATA = { BigEndianInt8( label flipped ) } ;
// CONTINUITY = INGL_CONT , BigEndianInt32(Sizeof(CONTINUITY_DATA)) , CONTINUITY_DATA ;
// CONTINUITY_DATA = CONTINUITY_INDEX , CONTINUITY_NAME , CONTINUITY_TEXT ;
// CONTINUITY_INDEX = BigEndianInt8( index );
// CONTINUITY_NAME = Null-terminated char* ;
// CONTINUITY_TEXT = Null-terminated char* ;
// SHEET_END = INGL_END , INGL_SHET ;
// SHOW_END = INGL_END , INGL_SHOW ;
// INGL_INGL = 'I','N','G','L' ;
// INGL_GURK = 'G','U','R','K' ;
// INGL_SHOW = 'S','H','O','W' ;
// INGL_SHET = 'S','H','E','T' ;
// INGL_SIZE = 'S','I','Z','E' ;
// INGL_LABL = 'L','A','B','L' ;
// INGL_MODE = 'M','O','D','E' ;
// INGL_DESC = 'D','E','S','C' ;
// INGL_NAME = 'N','A','M','E' ;
// INGL_DURA = 'D','U','R','A' ;
// INGL_POS  = 'P','O','S',' ' ;
// INGL_SYMB = 'S','Y','M','B' ;
// INGL_TYPE = 'T','Y','P','E' ;
// INGL_REFP = 'R','E','F','P' ;
// INGL_CONT = 'C','O','N','T' ;
// INGL_PCNT = 'P','C','N','T' ;
// INGL_END  = 'E','N','D',' ' ;
// Some assumptions:
// strings are ascii 8-bit chars.
// Error if LABEL_DATA does not contain N null terminated strings, where N ==
// number of marchers
// Error if POSITION_DATA does not contain N*2 values, where N == number of
// marchers
// Error if REF_POSITION_DATA does not contain N*2 + 1 values, where N == number
// of marchers
// Error if POINT_SYMBOL_DATA does not contain N values, where N == number of
// marchers
// Error if POINT_CONT_INDEX_DATA does not contain N values, where N == number
// of marchers
// Error if POINT_LABEL_FLIP_DATA does not contain N values, where N == number
// of marchers
// If REF_POSITION is not supplied, all reference points are assumed to be set
// to the point value
// If POINT_SYMBOL is not supplied, all points assumed to be symbol 0
// If POINT_CONT_INDEX is not supplied, all points assumed to be index 0
// If POINT_LABEL_FLIP is not supplied, all points assumed to be not flipped

// version 0 to 3.1 unknown
// clang-format on

#define Make4CharWord(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define INGL_INGL Make4CharWord('I', 'N', 'G', 'L')
#define INGL_GURK Make4CharWord('G', 'U', 'R', 'K')
#define INGL_SHOW Make4CharWord('S', 'H', 'O', 'W')
#define INGL_SHET Make4CharWord('S', 'H', 'E', 'T')
#define INGL_SELE Make4CharWord('S', 'E', 'L', 'E')
#define INGL_SIZE Make4CharWord('S', 'I', 'Z', 'E')
#define INGL_CURR Make4CharWord('C', 'U', 'R', 'R')
#define INGL_LABL Make4CharWord('L', 'A', 'B', 'L')
#define INGL_INST Make4CharWord('I', 'N', 'S', 'T')
#define INGL_MODE Make4CharWord('M', 'O', 'D', 'E')
#define INGL_DESC Make4CharWord('D', 'E', 'S', 'C')
#define INGL_NAME Make4CharWord('N', 'A', 'M', 'E')
#define INGL_DURA Make4CharWord('D', 'U', 'R', 'A')
#define INGL_POS Make4CharWord('P', 'O', 'S', ' ')
#define INGL_SYMB Make4CharWord('S', 'Y', 'M', 'B')
#define INGL_TYPE Make4CharWord('T', 'Y', 'P', 'E')
#define INGL_REFP Make4CharWord('R', 'E', 'F', 'P')
#define INGL_CONT Make4CharWord('C', 'O', 'N', 'T')
#define INGL_ECNT Make4CharWord('E', 'C', 'N', 'T')
#define INGL_VCNT Make4CharWord('V', 'C', 'N', 'T')
#define INGL_EVCT Make4CharWord('E', 'V', 'C', 'T')
#define INGL_PCNT Make4CharWord('P', 'C', 'N', 'T')
#define INGL_BACK Make4CharWord('B', 'A', 'C', 'K')
#define INGL_PNTS Make4CharWord('P', 'N', 'T', 'S')
#define INGL_PONT Make4CharWord('P', 'O', 'N', 'T')
#define INGL_END Make4CharWord('E', 'N', 'D', ' ')

template <typename T>
uint16_t get_big_word(const T* ptr)
{
    return ((ptr[0] & 0xFF) << 8) | ((ptr[1] & 0xFF));
}

template <typename T>
uint32_t get_big_long(const T* ptr)
{
    return ((ptr[0] & 0xFF) << 24) | ((ptr[1] & 0xFF) << 16) | ((ptr[2] & 0xFF) << 8) | ((ptr[3] & 0xFF));
}

inline void put_big_word(void* p, uint16_t v)
{
    uint8_t* ptr = static_cast<uint8_t*>(p);
    ptr[0] = v >> 8;
    ptr[1] = v;
}

inline void put_big_long(void* p, uint32_t v)
{
    uint8_t* ptr = static_cast<uint8_t*>(p);
    ptr[0] = v >> 24;
    ptr[1] = v >> 16;
    ptr[2] = v >> 8;
    ptr[3] = v;
}

class CC_FileException : public std::runtime_error {
    static std::string GetErrorFromID(uint32_t nameID)
    {
        uint8_t rawd[4];
        put_big_long(rawd, nameID);
        std::stringstream buf;
        buf << "Wrong ID read:  Read " << rawd[0] << rawd[1] << rawd[2] << rawd[3];
        buf << "\n";
        buf << "Check to make sure you are on the latest version of CalChart\n";
        return std::string(buf.str());
    }

public:
    CC_FileException(const std::string& reason)
        : std::runtime_error(reason)
    {
    }
    CC_FileException(uint32_t nameID)
        : std::runtime_error(GetErrorFromID(nameID)){};
    CC_FileException(const std::string& reason, uint32_t nameID)
        : std::runtime_error(reason + " : " + GetErrorFromID(nameID)){};
};

template <typename T>
inline uint8_t ReadByte(T& stream)
{
    char data;
    stream.read(&data, sizeof(data));
    return data;
}

template <typename T>
inline uint32_t ReadLong(T& stream)
{
    char rawd[4];
    stream.read(rawd, sizeof(rawd));
    return get_big_long(rawd);
}

// return false if you don't read the inname
template <typename T>
inline void ReadAndCheckID(T& stream, uint32_t inname)
{
    uint32_t name = ReadLong(stream);
    if (inname != name) {
        throw CC_FileException(inname);
    }
}

// return false if you don't read the inname
template <typename T>
inline uint32_t ReadGurkSymbolAndGetVersion(T& stream, uint32_t inname)
{
    uint32_t name = ReadLong(stream);
    if (inname == name) {
        // if we have an exact match, this is version 0
        return 0x0;
    }
    if ((((name >> 24) & 0xFF) == ((inname >> 24) & 0xFF)) && (((name >> 16) & 0xFF) == ((inname >> 16) & 0xFF))) {
        char major_vers = (name >> 8) & 0xFF;
        char minor_vers = (name)&0xFF;
        if (isdigit(major_vers) && isdigit(minor_vers)) {
            return ((major_vers - '0') << 8) | (minor_vers - '0');
        }
    }
    throw CC_FileException(inname);
}

// return false if you don't read the inname
template <typename T>
inline uint32_t ReadCheckIDandSize(T& stream, uint32_t inname)
{
    uint32_t name = ReadLong(stream);
    if (inname != name) {
        throw CC_FileException(inname);
    }
    name = ReadLong(stream);
    if (4 != name) {
        uint8_t rawd[4];
        put_big_long(rawd, inname);
        std::stringstream buf;
        buf << "Wrong size " << name << " for name " << rawd[0] << rawd[1]
            << rawd[2] << rawd[3];
        throw CC_FileException(buf.str());
    }
    return ReadLong(stream);
}

// return false if you don't read the inname
template <typename T>
inline std::vector<uint8_t> ReadCheckIDandFillData(T& stream, uint32_t inname)
{
    uint32_t name = ReadLong(stream);
    if (inname != name) {
        throw CC_FileException(inname);
    }
    std::vector<uint8_t> data(ReadLong(stream));
    stream.read(reinterpret_cast<char*>(&data[0]), data.size());
    return data;
}

// Just fill the data
template <typename T>
inline std::vector<uint8_t> FillData(T& stream, size_t size)
{
    std::vector<uint8_t> data(size);
    stream.read(reinterpret_cast<char*>(&data[0]), data.size());
    return data;
}

// Just fill the data
template <typename T>
inline std::vector<uint8_t> FillData(T& stream)
{
    uint32_t name = ReadLong(stream);
    return FillData(stream, name);
}

template <typename T>
inline void Write(T& stream, const void* data, uint32_t size)
{
    stream.write(reinterpret_cast<const char*>(data), size);
}

template <typename T>
inline void WriteLong(T& stream, uint32_t d)
{
    char rawd[4];
    put_big_long(rawd, d);
    Write(stream, rawd, sizeof(rawd));
}

template <typename T>
inline void WriteHeader(T& stream)
{
    WriteLong(stream, INGL_INGL);
}

template <typename T>
inline void WriteGurk(T& stream, uint32_t name)
{
    WriteLong(stream, INGL_GURK);
    WriteLong(stream, name);
}

template <typename T>
inline void WriteGurkAndVersion(T& stream, uint8_t major, uint8_t minor)
{
    uint32_t gurk_version = ((INGL_GURK)&0xFFFF0000) | (('0' + major) << 8) | ('0' + minor);

    WriteLong(stream, gurk_version);
}

template <typename T>
inline void WriteChunkHeader(T& stream, uint32_t name, uint32_t size)
{
    WriteLong(stream, name);
    WriteLong(stream, size);
}

template <typename T>
inline void WriteChunk(T& stream, uint32_t name, uint32_t size,
    const void* data)
{
    WriteLong(stream, name);
    WriteLong(stream, size);
    if (size > 0)
        Write(stream, data, size);
}

template <typename T>
inline void WriteChunkStr(T& stream, uint32_t name, const char* str)
{
    WriteChunk(stream, name, strlen(str) + 1,
        reinterpret_cast<const unsigned char*>(str));
}

template <typename T>
inline void WriteEnd(T& stream, uint32_t name)
{
    WriteLong(stream, INGL_END);
    WriteLong(stream, name);
}

template <typename T>
inline void WriteStr(T& stream, const char* str)
{
    Write(stream, str, strlen(str) + 1);
}

namespace Parser {

    template <typename Iter>
    uint32_t get_big_long(Iter ptr)
    {
        uint32_t result = (*ptr++ & 0xFF);
        result = (result << 8) | (*ptr++ & 0xFF);
        result = (result << 8) | (*ptr++ & 0xFF);
        result = (result << 8) | (*ptr++ & 0xFF);
        return result;
    }

    // oh ick, casting floating point to raw values... Sure hope things are ieee...
    template <typename Iter>
    float get_big_float(Iter ptr)
    {
        uint32_t result = (*ptr++ & 0xFF);
        result = (result << 8) | (*ptr++ & 0xFF);
        result = (result << 8) | (*ptr++ & 0xFF);
        result = (result << 8) | (*ptr++ & 0xFF);
        return *((float*)&result);
    }

    template <typename Iter>
    std::vector<std::tuple<uint32_t, Iter, size_t>> ParseOutLabels(Iter begin,
        Iter end)
    {
        std::vector<std::tuple<uint32_t, Iter, size_t>> result;
        while (begin != end) {
            auto length = std::distance(begin, end);
            if (length < 8) {
                return result;
            }
            auto name = get_big_long(begin);
            begin += 4;
            auto size = get_big_long(begin);
            begin += 4;
            length = std::distance(begin, end);
            if (static_cast<uint32_t>(length) < size + 8) {
                return result;
            }
            auto data = begin;
            begin += size;
            auto end = get_big_long(begin);
            begin += 4;
            auto end_name = get_big_long(begin);
            begin += 4;
            if ((end != INGL_END) || (end_name != name)) {
                return result;
            }
            result.push_back(std::tuple<uint32_t, Iter, size_t>(name, data, size));
        }
        return result;
    }

    struct PrintHeader {
        uint32_t d;
    };

    static inline std::ostream& operator<<(std::ostream& os, PrintHeader p)
    {
        return os << char(p.d >> 24) << char(p.d >> 16) << char(p.d >> 8)
                  << char(p.d >> 0);
    }

    template <typename Iter>
    void DoRecursiveParsing(std::ostream& os, const std::string& prefix, Iter begin,
        Iter end)
    {
        auto table = ParseOutLabels(begin, end);
        std::map<uint32_t, int> counter;
        for (auto& i : table) {
            os << prefix << "found " << PrintHeader{ std::get<0>(i) } << "\n";
            os << prefix << counter[std::get<0>(i)]++ << "\tsize " << std::get<2>(i)
               << "\n";
            DoRecursiveParsing(os, prefix + "  ", std::get<1>(i),
                std::get<1>(i) + std::get<2>(i));
        }
    }

    template <typename T, typename U>
    void Append(T& d, const U& s)
    {
        d.insert(d.end(), s.begin(), s.end());
    }

    template <typename T>
    void Append(T& d, uint32_t v)
    {
        char rawd[4];
        put_big_long(rawd, v);
        d.insert(d.end(), std::begin(rawd), std::end(rawd));
    }

    template <typename T>
    void Append(T& d, uint16_t v)
    {
        char rawd[2];
        put_big_word(rawd, v);
        d.insert(d.end(), std::begin(rawd), std::end(rawd));
    }

    template <typename T>
    void Append(T& d, uint8_t v) { d.insert(d.end(), v); }

    template <typename T>
    void Append(T& d, float v)
    {
        Append(d, *((uint32_t*)&v));
    }

    template <typename T, typename U>
    void AppendAndNullTerminate(T& d, const U& s)
    {
        Append(d, s);
        Append(d, uint8_t{ 0 });
    }

    template <typename T>
    auto Construct_block(uint32_t type, const T& data)
    {
        std::vector<uint8_t> result;
        Append(result, uint32_t{ type });
        Append(result, static_cast<uint32_t>(data.size()));
        Append(result, data);
        Append(result, uint32_t{ INGL_END });
        Append(result, uint32_t{ type });
        return result;
    }

    static inline auto Construct_block(uint32_t type, uint32_t data)
    {
        std::vector<uint8_t> result;
        Append(result, uint32_t{ type });
        Append(result, uint32_t{ sizeof(data) });
        Append(result, data);
        Append(result, uint32_t{ INGL_END });
        Append(result, uint32_t{ type });
        return result;
    }
}
}
