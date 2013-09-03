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

#ifndef _CC_FILEFORMAT_H_
#define _CC_FILEFORMAT_H_

#include "platconf.h"

#include <stdexcept>

// Description of the CalChart file format layout, in modified Extended Backus–Naur Form
// version 3.1.0 to current
// show = START , SHOW ;
// START = INGL_INGL , INGL_GURK ;
// SHOW = INGL_SHOW , SIZE , { LABEL } , { DESCRIPTION } , { SHEET } , SHOW_END ;
// SIZE = INGL_SIZE , BigEndianInt32(4) , BigEndianInt32( number of marchers ) ;
// LABEL = INGL_LABL , BigEndianInt32(Sizeof(LABEL_DATA)) , LABEL_DATA ;
// LABEL_DATA = { Null-terminated char* } ;
// DESCRIPTION = INGL_DESC , BigEndianInt32(Sizeof(DESCRIPTION_DATA)) , DESCRIPTION_DATA ;
// DESCRIPTION_DATA = Null-terminated char* ;
// SHEET = INGL_GURK , INGL_SHET , NAME , DURATION , POSITION , [ REF_POSITION ] , [ POINT_SYMBOL ] , [ POINT_CONT_INDEX ] , [ POINT_LABEL_FLIP ] , { CONTINUITY } , SHEET_END ;
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
// Error if LABEL_DATA does not contain N null terminated strings, where N == number of marchers
// Error if POSITION_DATA does not contain N*2 values, where N == number of marchers
// Error if REF_POSITION_DATA does not contain N*2 + 1 values, where N == number of marchers
// Error if POINT_SYMBOL_DATA does not contain N values, where N == number of marchers
// Error if POINT_CONT_INDEX_DATA does not contain N values, where N == number of marchers
// Error if POINT_LABEL_FLIP_DATA does not contain N values, where N == number of marchers
// If REF_POSITION is not supplied, all reference points are assumed to be set to the point value
// If POINT_SYMBOL is not supplied, all points assumed to be symbol 0
// If POINT_CONT_INDEX is not supplied, all points assumed to be index 0
// If POINT_LABEL_FLIP is not supplied, all points assumed to be not flipped

// version 0 to 3.1 unknown

class CC_FileException : public std::runtime_error
{
	static std::string GetErrorFromID(uint32_t nameID)
	{
		uint8_t rawd[4];
		put_big_long(rawd, nameID);
		char s[128];
		_snprintf(s, sizeof(s), "Wrong ID read:  Read %c%c%c%c", rawd[0], rawd[1], rawd[2], rawd[3]);
		return std::string(s);
	}
	
public:
	CC_FileException(const std::string& reason) : std::runtime_error(reason) {}
	CC_FileException(uint32_t nameID) : std::runtime_error(GetErrorFromID(nameID)) {};
};




template <typename T>
inline uint32_t ReadLong(T& stream)
{
	char rawd[4];
	stream.read(rawd, sizeof(rawd));
	return get_big_long(&rawd);
}

// return false if you don't read the inname
template <typename T>
inline void ReadAndCheckID(T& stream, uint32_t inname)
{
	uint32_t name = ReadLong(stream);
	if (inname != name)
	{
		throw CC_FileException(inname);
	}
}

// return false if you don't read the inname
template <typename T>
inline uint32_t ReadCheckIDandSize(T& stream, uint32_t inname)
{
	uint32_t name = ReadLong(stream);
	if (inname != name)
	{
		throw CC_FileException(inname);
	}
	name = ReadLong(stream);
	if (4 != name)
	{
		uint8_t rawd[4];
		put_big_long(rawd, inname);
		char s[128];
		_snprintf(s, sizeof(s), "Wrong size %d for name %c%c%c%c", name, rawd[0], rawd[1], rawd[2], rawd[3]);
		throw CC_FileException(s);
	}
	return ReadLong(stream);
}

// return false if you don't read the inname
template <typename T>
inline std::vector<uint8_t> ReadCheckIDandFillData(T& stream, uint32_t inname)
{
	uint32_t name = ReadLong(stream);
	if (inname != name)
	{
		throw CC_FileException(inname);
	}
	std::vector<uint8_t> data(ReadLong(stream));
	stream.read(reinterpret_cast<char*>(&data[0]), data.size());
	return data;
}

// Just fill the data
template <typename T>
inline std::vector<uint8_t> FillData(T& stream)
{
	uint32_t name = ReadLong(stream);
	std::vector<uint8_t> data(name);
	stream.read(reinterpret_cast<char*>(&data[0]), data.size());
	return data;
}

template <typename T>
inline void Write(T& stream, const void *data, uint32_t size)
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
inline void WriteChunkHeader(T& stream, uint32_t name, uint32_t size)
{
	WriteLong(stream, name);
	WriteLong(stream, size);
}


template <typename T>
inline void WriteChunk(T& stream, uint32_t name, uint32_t size, const void *data)
{
	WriteLong(stream, name);
	WriteLong(stream, size);
	if (size > 0)
		Write(stream, data, size);
}

template <typename T>
inline void WriteChunkStr(T& stream, uint32_t name, const char *str)
{
	WriteChunk(stream, name, strlen(str)+1, reinterpret_cast<const unsigned char *>(str));
}

template <typename T>
inline void WriteEnd(T& stream, uint32_t name)
{
	WriteLong(stream, INGL_END);
	WriteLong(stream, name);
}

template <typename T>
inline void WriteStr(T& stream, const char *str)
{
	Write(stream, str, strlen(str)+1);
}




#endif // _CC_FILEFORMAT_H_
