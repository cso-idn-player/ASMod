#include <cassert>
#include <cstring>
#include <memory>
#include <sstream>

#include <angelscript.h>

#include <extdll.h>

#include "CASBLOB.h"

CASBLOB::CASBLOB( size_t uiCapacity )
{
	Reserve( uiCapacity );
}

CASBLOB::CASBLOB( const CASBLOB& other )
{
	Copy( other );
}

CASBLOB& CASBLOB::operator=( const CASBLOB& other )
{
	Copy( other );

	return *this;
}

void CASBLOB::Copy( const CASBLOB& other )
{
	if( this != &other )
	{
		Clear();

		m_Buffer = other.m_Buffer;
	}
}

CASBLOB::~CASBLOB()
{
	Clear();
}

void CASBLOB::Release() const
{
	if( InternalRelease() )
		delete this;
}

bool CASBLOB::Write( const void* pData, size_t uiSizeInBytes )
{
	assert( pData );

	if( !pData )
		return false;

	if( uiSizeInBytes > 0 )
	{
		const auto currentWritePos = m_Buffer.size();

		m_Buffer.resize( m_Buffer.size() + uiSizeInBytes );

		memcpy( m_Buffer.data() + currentWritePos, pData, uiSizeInBytes );
	}

	return true;
}

bool CASBLOB::WriteBytes( size_t uiSizeInBytes, int iValue )
{
	if( uiSizeInBytes > 0 )
	{
		const auto currentWritePos = m_Buffer.size();

		m_Buffer.resize( m_Buffer.size() + uiSizeInBytes );

		memset( m_Buffer.data() + currentWritePos, iValue, uiSizeInBytes );
	}

	return true;
}

bool CASBLOB::WriteBytesUntil( size_t uiEndOffset, int iValue )
{
	if( uiEndOffset <= m_Buffer.size() )
		return false;

	return WriteBytes( uiEndOffset - m_Buffer.size(), iValue );
}

bool CASBLOB::Write( const std::string& szString, size_t uiSizeInBytes, const char pad )
{
	//Can't write a 0 byte string.
	if( uiSizeInBytes == 0 )
		return false;

	size_t uiLength = szString.length();

	//Clamp to maximum requested size. - 1 to ensure null termination.
	if( uiLength >= uiSizeInBytes )
		uiLength = uiSizeInBytes - 1;

	const auto currentWritePos = m_Buffer.size();

	m_Buffer.resize( m_Buffer.size() + uiSizeInBytes );

	memcpy( m_Buffer.data() + currentWritePos, szString.c_str(), uiLength );
	//- 1 because see below.
	memset( m_Buffer.data() + currentWritePos + uiLength, pad, uiSizeInBytes - uiLength - 1 );

	//If pad is not \0, this will require null termination.
	m_Buffer[ currentWritePos + uiSizeInBytes - 1 ] = '\0';

	return true;
}

bool CASBLOB::Write( const std::string& szString )
{
	return Write( szString.c_str(), szString.length() + 1 );
}

bool CASBLOB::ReadData( void* pData, size_t uiSizeInBytes )
{
	assert( pData );

	if( !pData || ReadReachedEnd() || !ReadSizeValid( uiSizeInBytes ) )
		return false;

	//Always valid as long as the above checks are valid.
	if( uiSizeInBytes == 0 )
		return true;

	memcpy( pData, m_Buffer.data() + m_uiReadOffset, uiSizeInBytes );

	m_uiReadOffset += uiSizeInBytes;

	return true;
}

std::string CASBLOB::ReadString( size_t uiSizeInBytes, bool& bSuccess )
{
	if( uiSizeInBytes == 0 )
	{
		bSuccess = true;
		return "";
	}

	if( !ReadSizeValid( uiSizeInBytes ) )
	{
		bSuccess = false;
		return "";
	}

	//std::string is not guaranteed to use an array internally, so allocate it manually.
	auto szString = std::make_unique<char[]>( uiSizeInBytes );

	memset( szString.get(), 0, uiSizeInBytes );

	//Don't read the null terminator (there might not be one).
	bSuccess = ReadData( szString.get(), uiSizeInBytes - 1 );

	//Always null terminate.
	szString[ uiSizeInBytes - 1 ] = '\0';

	return szString.get();
}

std::string CASBLOB::ReadString( size_t uiSizeInBytes )
{
	bool bSuccess;

	return ReadString( uiSizeInBytes, bSuccess );
}

std::string CASBLOB::ReadString( bool& bSuccess )
{
	if( ReadReachedEnd() )
	{
		bSuccess = false;
		return "";
	}

	std::ostringstream outStream;

	while( m_uiReadOffset < m_Buffer.size() )
	{
		auto character = static_cast<char>( m_Buffer.data()[ m_uiReadOffset++ ] );

		if( character == '\0' )
			break;

		outStream << character;
	}

	bSuccess = true;

	return outStream.str();
}

std::string CASBLOB::ReadString()
{
	bool bSuccess;

	return ReadString( bSuccess );
}

void CASBLOB::Reserve( size_t uiMinimumCapacity )
{
	m_Buffer.reserve( uiMinimumCapacity );
}

void CASBLOB::Resize( size_t uiNewSize )
{
	if( m_Buffer.size() == uiNewSize )
		return;

	m_Buffer.resize( uiNewSize );

	if( uiNewSize > m_uiReadOffset )
		m_uiReadOffset = uiNewSize;
}

void CASBLOB::ShrinkToFit()
{
	m_Buffer.shrink_to_fit();
}

void CASBLOB::Clear()
{
	m_Buffer.clear();
	m_uiReadOffset = 0;
}

static CASBLOB* CASBLOB_CASBLOB( const size_t uiCapacity )
{
	return new CASBLOB( uiCapacity );
}

static CASBLOB* CASBLOB_CASBLOB( const CASBLOB& other )
{
	return new CASBLOB( other );
}

//Wrapper to convert std::string to char.
//TODO: need a stdChar API type. - Solokiller
static bool CASBLOB_Write( CASBLOB* pThis, const std::string& szString, size_t uiSizeInBytes, const std::string& szPad = "\0" )
{
	const char pad = szPad.empty() ? '\0' : szPad[ 0 ];

	return pThis->Write( szString, uiSizeInBytes, pad );
}

void RegisterScriptBLOB( asIScriptEngine& scriptEngine )
{
	const char* const pszObjectName = "BLOB";

	scriptEngine.RegisterObjectType( pszObjectName, 0, asOBJ_REF );

	as::RegisterRefCountedBaseClass<CASBLOB>( &scriptEngine, pszObjectName );

	scriptEngine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_FACTORY, "BLOB@ BLOB(uint uiCapacity = 0)",
		asFUNCTIONPR( CASBLOB_CASBLOB, ( size_t ), CASBLOB* ), asCALL_CDECL );

	scriptEngine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_FACTORY, "BLOB@ BLOB(const BLOB& in other)",
		asFUNCTIONPR( CASBLOB_CASBLOB, ( const CASBLOB& ), CASBLOB* ), asCALL_CDECL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint GetCapacity() const",
		asMETHOD( CASBLOB, GetCapacity ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint GetSize() const",
		asMETHOD( CASBLOB, GetSize ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint GetReadOffset() const",
		asMETHOD( CASBLOB, GetReadOffset ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool ReadReachedEnd() const",
		asMETHOD( CASBLOB, ReadReachedEnd ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool ReadSizeValid(uint uiSizeInBytes) const",
		asMETHOD( CASBLOB, ReadSizeValid ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool WriteBytes(uint uiSizeInBytes, int iValue = 0)",
		asMETHOD( CASBLOB, WriteBytes ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool WriteBytesUntil(uint uiEndOffset, int iValue = 0)",
		asMETHOD( CASBLOB, WriteBytesUntil ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Write(int8 data)",
		asMETHODPR( CASBLOB, Write, ( asBYTE ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Write(int16 data)",
		asMETHODPR( CASBLOB, Write, ( asWORD ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Write(int32 data)",
		asMETHODPR( CASBLOB, Write, ( asDWORD ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Write(int64 data)",
		asMETHODPR( CASBLOB, Write, ( asQWORD ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Write(uint8 data)",
		asMETHODPR( CASBLOB, Write, ( asBYTE ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Write(uint16 data)",
		asMETHODPR( CASBLOB, Write, ( asWORD ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Write(uint32 data)",
		asMETHODPR( CASBLOB, Write, ( asDWORD ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Write(uint64 data)",
		asMETHODPR( CASBLOB, Write, ( asQWORD ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Write(float data)",
		asMETHODPR( CASBLOB, Write, ( float ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Write(double data)",
		asMETHODPR( CASBLOB, Write, ( double ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Write(const " AS_STRING_OBJNAME "& in szString, uint uiSizeInBytes, const " AS_STRING_OBJNAME "& in pad = \"\\0\")",
		asFUNCTION( CASBLOB_Write ), asCALL_CDECL_OBJFIRST );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Write(const " AS_STRING_OBJNAME "& in szString)",
		asMETHODPR( CASBLOB, Write, ( const std::string& ), bool ), asCALL_THISCALL );

	//Note: these use ReadUInt* methods, but return signed integers. This is intentional. - Solokiller
	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int8 ReadInt8()",
		asMETHODPR( CASBLOB, ReadUInt8, (), asBYTE ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int16 ReadInt16()",
		asMETHODPR( CASBLOB, ReadUInt16, (), asWORD ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int32 ReadInt32()",
		asMETHODPR( CASBLOB, ReadUInt32, (), asDWORD ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int64 ReadInt64()",
		asMETHODPR( CASBLOB, ReadUInt64, (), asQWORD ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint8 ReadUInt8()",
		asMETHODPR( CASBLOB, ReadUInt8, (), asBYTE ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int16 ReadUInt16()",
		asMETHODPR( CASBLOB, ReadUInt16, (), asWORD ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int32 ReadUInt32()",
		asMETHODPR( CASBLOB, ReadUInt32, (), asDWORD ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int64 ReadUInt64()",
		asMETHODPR( CASBLOB, ReadUInt64, (), asQWORD ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "float ReadFloat()",
		asMETHODPR( CASBLOB, ReadFloat, (), float ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "double ReadDouble()",
		asMETHODPR( CASBLOB, ReadDouble, (), double ), asCALL_THISCALL );

	//Note: these use ReadUInt* methods, but return signed integers. This is intentional. - Solokiller
	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int8 ReadInt8(bool& out bSuccess)",
		asMETHODPR( CASBLOB, ReadUInt8, ( bool& ), asBYTE ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int16 ReadInt16(bool& out bSuccess)",
		asMETHODPR( CASBLOB, ReadUInt16, ( bool& ), asWORD ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int32 ReadInt32(bool& out bSuccess)",
		asMETHODPR( CASBLOB, ReadUInt32, ( bool& ), asDWORD ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int64 ReadInt64(bool& out bSuccess)",
		asMETHODPR( CASBLOB, ReadUInt64, ( bool& ), asQWORD ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint8 ReadUInt8(bool& out bSuccess)",
		asMETHODPR( CASBLOB, ReadUInt8, ( bool& ), asBYTE ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int16 ReadUInt16(bool& out bSuccess)",
		asMETHODPR( CASBLOB, ReadUInt16, ( bool& ), asWORD ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int32 ReadUInt32(bool& out bSuccess)",
		asMETHODPR( CASBLOB, ReadUInt32, ( bool& ), asDWORD ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "int64 ReadUInt64(bool& out bSuccess)",
		asMETHODPR( CASBLOB, ReadUInt64, ( bool& ), asQWORD ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "float ReadFloat(bool& out bSuccess)",
		asMETHODPR( CASBLOB, ReadFloat, ( bool& ), float ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "double ReadDouble(bool& out bSuccess)",
		asMETHODPR( CASBLOB, ReadDouble, ( bool& ), double ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, AS_STRING_OBJNAME " ReadString(uint uiSizeInBytes)",
		asMETHODPR( CASBLOB, ReadString, ( size_t ), std::string ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, AS_STRING_OBJNAME " ReadString(uint uiSizeInBytes, bool& out bSuccess)",
		asMETHODPR( CASBLOB, ReadString, ( size_t, bool& ), std::string ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, AS_STRING_OBJNAME " ReadString(bool& out bSuccess)",
		asMETHODPR( CASBLOB, ReadString, ( bool& ), std::string ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, AS_STRING_OBJNAME " ReadString()",
		asMETHODPR( CASBLOB, ReadString, (), std::string ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "void Reserve(uint uiMinimumSize)",
		asMETHOD( CASBLOB, Reserve ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "void Resize(uint uiNewSize)",
		asMETHOD( CASBLOB, Resize ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "void ShrinkToFit()",
		asMETHOD( CASBLOB, ShrinkToFit ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "void Clear()",
		asMETHOD( CASBLOB, Clear ), asCALL_THISCALL );
}
