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

#include <stdexcept>
#include <sstream>

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


// Possible file description for 3.4.0
// Description of the CalChart file format layout, in modified Extended Backus–Naur Form
// version 3.4.0 to current
//   Where {} means 0 or 1;
//         {}* means 0 or more;
// show               = START , SHOW ;
// START              = INGL_INGL , INGL_VERS ;
// SHOW               = INGL_SHOW , BigEndianInt32(DataTill_SHOW_END) , SHOW_DATA , SHOW_END ;
// SHOW_DATA          = NUM_MARCH , { LABEL } , { DESCRIPTION } , { SHEET }* ;
// SHOW_END           = INGL_END , INGL_SHOW ;
// NUM_MARCH          = INGL_NUMM , BigEndianInt32(4) , NUM_MARCH_DATA , NUM_MARCH_END;
// NUM_MARCH_DATA     = BigEndianInt32( number of marchers ) ;
// NUM_MARCH_END      = INGL_END , INGL_NUMM ;
// LABEL              = INGL_LABL , BigEndianInt32(DataTill_LABEL_END) , LABEL_DATA , LABEL_END;
// LABEL_DATA         = { Null-terminated char* }* ;
// LABEL_END          = INGL_END , INGL_LABL ;
// DESCRIPTION        = INGL_DESC , BigEndianInt32(DataTill_DESCRIPTION_END) , DESCRIPTION_DATA , DESCRIPTION_END ;
// DESCRIPTION_DATA   = { Null-terminated char* } ;
// DESCRIPTION_END    = INGL_END , INGL_DESC ;
// SHEET              = INGL_SHET , BigEndianInt32(DataTill_SHEET_END) , SHEET_DATA , SHEET_END ;
// SHEET              = NAME , DURATION , ALL_POINTS , POSITION , [ REF_POSITION ] , [ POINT_SYMBOL ] , [ POINT_CONT_INDEX ] , [ POINT_LABEL_FLIP ] , { CONTINUITY } ;
// SHEET_END          = INGL_END , INGL_SHET ;
// NAME               = INGL_NAME , BigEndianInt32(DataTill_NAME_END) , NAME_DATA , NAME_END ;
// NAME_DATA          = { Null-terminated char* } ;
// NAME_END           = INGL_END , INGL_NAME ;
// DURATION           = INGL_DURA , BigEndianInt32(4) , DURATION_DATA , DURATION_END;
// DURATION_DATA      = BigEndianInt32(number of beats) ;
// DURATION_END       = INGL_END , INGL_DURA ;
// ALL_POINTS         = INGL_PNTS , BigEndianInt32(DataTill_ALL_POINTS_END) , ALL_POINTS_DATA , ALL_POINTS_END ;
// ALL_POINTS_DATA    = { POSITION , [ REF_POSITION ] , [ POINT_SYMBOL ] , [ POINT_CONT_INDEX ] , [ POINT_LABEL_FLIP ] } x num_march ;
// ALL_POINTS_END     = INGL_END , INGL_PNTS ;
// POSITION           = INGL_POS , BigEndianInt32(DataTill_POSITION_END) , POSITION_DATA , POSITION_END ;
// POSITION_DATA      = { BigEndianInt16( x ) , BigEndianInt16( y ) } ;
// POSITION_END       = INGL_END , INGL_POS ;
// REF_POSITION       = INGL_REFP , BigEndianInt32(DataTill_REF_POSITION_END) , REF_POSITION_DATA , REF_POSITION_END ;
// REF_POSITION_DATA  = BigEndianInt16( which reference point ) , { BigEndianInt16( x ) , BigEndianInt16( y ) } ;
// REF_POSITION_END   = INGL_END , INGL_REFP ;
// POINT_SYMBOL       = INGL_SYMB , BigEndianInt32(DataTill_POINT_SYMBOL_END) , POINT_SYMBOL_DATA , POINT_SYMBOL_END ;
// POINT_SYMBOL_DATA  = { BigEndianInt8( which symbol type ) } ;
// POINT_SYMBOL_END   = INGL_END , INGL_SYMB ;
// POINT_CONT_INDEX   = INGL_TYPE , BigEndianInt32(DataTill_POINT_CONT_INDEX_END)) , POINT_CONT_INDEX_DATA , POINT_CONT_INDEX_END ;
// POINT_CONT_INDEX_DATA = { BigEndianInt8( which continuity index ) } ;
// POINT_CONT_INDEX_END = INGL_END , INGL_TYPE ;
// POINT_LABEL_FLIP   = INGL_LABL , BigEndianInt32(DataTill_POINT_LABEL_FLIP_END)) , POINT_LABEL_FLIP_DATA , POINT_LABEL_FLIP_END ;
// POINT_LABEL_FLIP_DATA = { BigEndianInt8( label flipped ) } ;
// POINT_LABEL_FLIP_END = INGL_END , INGL_LABL ;
// CONTINUITY         = INGL_CONT , BigEndianInt32(DataTill_CONTINUITY_END)) , CONTINUITY_DATA , CONTINUITY_END;
// CONTINUITY_DATA    = BigEndianInt8( index ) , Null-terminated char* , Null-terminated char* ;
// CONTINUITY_END     = INGL_END , INGL_CONT ;
//
// INGL_INGL = 'I','N','G','L' ;
// INGL_GURK = 'G','U','R','K' ;
// INGL_SHOW = 'S','H','O','W' ;
// INGL_SHET = 'S','H','E','T' ;
// INGL_NUMM = 'S','I','Z','E' ;
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
	ptr[0] = v>>8;
	ptr[1] = v;
}

inline void put_big_long(void* p, uint32_t v)
{
	uint8_t* ptr = static_cast<uint8_t*>(p);
	ptr[0] = v>>24;
	ptr[1] = v>>16;
	ptr[2] = v>>8;
	ptr[3] = v;
}

class CC_FileException : public std::runtime_error
{
	static std::string GetErrorFromID(uint32_t nameID)
	{
		uint8_t rawd[4];
		put_big_long(rawd, nameID);
		std::stringstream buf;
		buf<<"Wrong ID read:  Read "<<rawd[0]<<rawd[1]<<rawd[2]<<rawd[3];
		return std::string(buf.str());
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
	return get_big_long(rawd);
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
		std::stringstream buf;
		buf<<"Wrong size "<<name<<" for name "<<rawd[0]<<rawd[1]<<rawd[2]<<rawd[3];
		throw CC_FileException(buf.str());
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
