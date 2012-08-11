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

#define Make4CharWord(a,b,c,d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define INGL_INGL Make4CharWord('I','N','G','L')
#define INGL_GURK Make4CharWord('G','U','R','K')
#define INGL_SHOW Make4CharWord('S','H','O','W')
#define INGL_SHET Make4CharWord('S','H','E','T')
#define INGL_SIZE Make4CharWord('S','I','Z','E')
#define INGL_LABL Make4CharWord('L','A','B','L')
#define INGL_MODE Make4CharWord('M','O','D','E')
#define INGL_DESC Make4CharWord('D','E','S','C')
#define INGL_NAME Make4CharWord('N','A','M','E')
#define INGL_DURA Make4CharWord('D','U','R','A')
#define INGL_POS  Make4CharWord('P','O','S',' ')
#define INGL_SYMB Make4CharWord('S','Y','M','B')
#define INGL_TYPE Make4CharWord('T','Y','P','E')
#define INGL_REFP Make4CharWord('R','E','F','P')
#define INGL_CONT Make4CharWord('C','O','N','T')
#define INGL_PCNT Make4CharWord('P','C','N','T')
#define INGL_END  Make4CharWord('E','N','D',' ')


class CC_FileException : public std::runtime_error
{
	static std::string GetErrorFromID(uint32_t nameID)
	{
		uint8_t rawd[4];
		put_big_long(rawd, nameID);
		char s[128];
		snprintf(s, sizeof(s), "Wrong ID read:  Read %c%c%c%c", rawd[0], rawd[1], rawd[2], rawd[3]);
		return std::string(s);
	}
	
public:
	CC_FileException(const std::string& reason) : std::runtime_error(reason) {}
	CC_FileException(uint32_t nameID) : std::runtime_error(GetErrorFromID(nameID)) {};
};




template <typename T>
void ReadLong(T& stream, uint32_t& d)
{
	uint8_t rawd[4];
	stream.Read(rawd, sizeof(rawd));
	d = get_big_long(&rawd);
}

template <>
void ReadLong<wxSTD istream>(wxSTD istream& stream, uint32_t& d)
{
	char rawd[4];
	stream.read(rawd, sizeof(rawd));
	d = get_big_long(&rawd);
}

// return false if you don't read the inname
template <typename T>
void ReadAndCheckID(T& stream, uint32_t inname)
{
	uint32_t name;
	ReadLong(stream, name);
	if (inname != name)
	{
		throw CC_FileException(inname);
	}
}

// return false if you don't read the inname
template <typename T>
void ReadCheckIDandSize(T& stream, uint32_t inname, uint32_t& size)
{
	uint32_t name;
	ReadLong(stream, name);
	if (inname != name)
	{
		throw CC_FileException(inname);
	}
	ReadLong(stream, name);
	if (4 != name)
	{
		uint8_t rawd[4];
		put_big_long(rawd, inname);
		char s[128];
		snprintf(s, sizeof(s), "Wrong size %d for name %c%c%c%c", name, rawd[0], rawd[1], rawd[2], rawd[3]);
		throw CC_FileException(s);
	}
	ReadLong(stream, size);
}

// return false if you don't read the inname
template <typename T>
void ReadCheckIDandFillData(T& stream, uint32_t inname, std::vector<uint8_t>& data)
{
	uint32_t name;
	ReadLong(stream, name);
	if (inname != name)
	{
		throw CC_FileException(inname);
	}
	ReadLong(stream, name);
	data.resize(name);
	stream.Read(&data[0], name);
}

template <>
void ReadCheckIDandFillData<wxSTD istream>(wxSTD istream& stream, uint32_t inname, std::vector<uint8_t>& data)
{
	uint32_t name;
	ReadLong(stream, name);
	if (inname != name)
	{
		throw CC_FileException(inname);
	}
	ReadLong(stream, name);
	data.resize(name);
	stream.read(reinterpret_cast<char*>(&data[0]), name);
}

// Just fill the data
template <typename T>
void FillData(T& stream, std::vector<uint8_t>& data)
{
	uint32_t name;
	ReadLong(stream, name);
	data.resize(name);
	stream.Read(&data[0], name);
}

template <>
void FillData<wxSTD istream>(wxSTD istream& stream, std::vector<uint8_t>& data)
{
	uint32_t name;
	ReadLong(stream, name);
	data.resize(name);
	stream.read(reinterpret_cast<char*>(&data[0]), name);
}

template <typename T>
void WriteLong(T& stream, uint32_t d)
{
	uint8_t rawd[4];
	put_big_long(rawd, d);
	stream.Write(rawd, sizeof(rawd));
}

template <>
void WriteLong<wxSTD ostream>(wxSTD ostream& stream, uint32_t d)
{
	char rawd[4];
	put_big_long(rawd, d);
	stream.write(rawd, sizeof(rawd));
}

template <typename T>
void WriteHeader(T& stream)
{
	WriteLong(stream, INGL_INGL);
}


template <typename T>
void WriteGurk(T& stream, uint32_t name)
{
	WriteLong(stream, INGL_GURK);
	WriteLong(stream, name);
}


template <typename T>
void WriteChunkHeader(T& stream, uint32_t name, uint32_t size)
{
	WriteLong(stream, name);
	WriteLong(stream, size);
}


template <typename T>
void WriteChunk(T& stream, uint32_t name, uint32_t size, const void *data)
{
	WriteLong(stream, name);
	WriteLong(stream, size);
	if (size > 0)
		stream.Write(data, size);
}

template <>
void WriteChunk<wxSTD ostream>(wxSTD ostream& stream, uint32_t name, uint32_t size, const void *data)
{
	WriteLong(stream, name);
	WriteLong(stream, size);
	if (size > 0)
		stream.write(reinterpret_cast<const char*>(data), size);
}

template <typename T>
void WriteChunkStr(T& stream, uint32_t name, const char *str)
{
	WriteChunk(stream, name, strlen(str)+1, reinterpret_cast<const unsigned char *>(str));
}

template <typename T>
void WriteEnd(T& stream, uint32_t name)
{
	WriteLong(stream, INGL_END);
	WriteLong(stream, name);
}


template <typename T>
void WriteStr(T& stream, const char *str)
{
	stream.Write(str, strlen(str)+1);
}

template <>
void WriteStr<wxSTD ostream>(wxSTD ostream& stream, const char *str)
{
	stream.write(str, strlen(str)+1);
}

template <typename T>
void Write(T& stream, const void *data, uint32_t size)
{
	stream.Write(data, size);
}

template <>
void Write<wxSTD ostream>(wxSTD ostream& stream, const void *data, uint32_t size)
{
	stream.write(reinterpret_cast<const char*>(data), size);
}




#endif // _CC_FILEFORMAT_H_
