#include <cassert>
#include <memory>
#include <sstream>

#include <angelscript.h>

#include <extdll.h>
#include <meta_api.h>

#include <Angelscript/util/ASLogging.h>

#include "Platform.h"

#include "Module.h"

#include "ASFileSystemUtils.h"

#include "CASVirtualFileSystem.h"
#include "CASDirectory.h"
#include "CASBLOB.h"

#include "CASSTDIOFile.h"

CASSTDIOFile::CASSTDIOFile( const char* const pszFilename, const OpenFileFlags_t uiOpenFlags, FILE* pFile )
	: m_szFilename( pszFilename )
	, m_uiFlags( uiOpenFlags )
	, m_pFile( pFile )
{
	assert( pszFilename != nullptr );
	assert( pFile != nullptr );

	if( ASFileSystem_LogFileAccess() )
	{
		as::Diagnostic( "File \"%s\" opened\n", pszFilename );
	}
}

CASSTDIOFile::~CASSTDIOFile()
{
	Close();
}

void CASSTDIOFile::Release() const
{
	if( InternalRelease() )
		delete this;
}

void CASSTDIOFile::Close()
{
	if( IsOpen() )
	{
		if( ASFileSystem_LogFileAccess() )
		{
			as::Diagnostic( "File \"%s\" closed\n", m_szFilename.c_str() );
		}

		fclose( m_pFile );
		m_pFile = nullptr;
	}
}

uint64_t CASSTDIOFile::GetSize() const
{
	assert( IsOpen() );

	//TODO: fileno isn't in the standard. - Solokiller
	auto fileNo = fileno( m_pFile );

	struct stat buf;

	if( !fstat( fileNo, &buf ) )
		return buf.st_size;

	return 0;
}

uint64_t CASSTDIOFile::Tell() const
{
	assert( IsOpen() );
	return ftell( m_pFile );
}

bool CASSTDIOFile::EOFReached() const
{
	assert( IsOpen() );

	return !!feof( m_pFile );
}

uint64_t CASSTDIOFile::Write( const std::string& szString )
{
	assert( IsOpen() );

	if( !IsWriting() )
		return 0;

	return WriteBytes( szString.c_str(), szString.length() );
}

uint64_t CASSTDIOFile::Write( const CASBLOB* pBlob )
{
	assert( IsOpen() );

	if( !pBlob )
		return 0;

	uint64_t uiBytesWritten = 0;

	if( IsWriting() )
	{
		uiBytesWritten = WriteBytes( pBlob->GetData(), pBlob->GetSize() );
	}

	pBlob->Release();

	return uiBytesWritten;
}

void CASSTDIOFile::Flush()
{
	assert( IsOpen() );

	if( !IsWriting() )
		return;

	fflush( m_pFile );
}

std::string CASSTDIOFile::ReadCharacter()
{
	assert( IsOpen() );

	if( !IsReading() )
		return "";

	if( EOFReached() )
		return "";

	char character[ 2 ] = { '\0', '\0' };

	fread( character, 1, 1, m_pFile );

	return character;
}

void CASSTDIOFile::ReadLine( std::string& szOutLine, const std::string& szDelim )
{
	assert( IsOpen() );

	if( !IsReading() )
		return;

	if( szDelim.empty() )
	{
		szOutLine = "";
		return;
	}

	const char cDelim = szDelim[ 0 ];

	std::ostringstream outStream;

	char character;

	while( true )
	{
		if( ferror( m_pFile ) || feof( m_pFile ) )
			break;

		if( fread( &character, 1, 1, m_pFile ) == 0 )
			break;

		if( character == cDelim )
			break;

		outStream << character;
	}

	szOutLine = outStream.str().c_str();

	//Strip any carriage feeds that might have been read if it's part of a newline.
	//TODO: always strip carriage feeds? - Solokiller
	if( cDelim == '\n' && !szOutLine.empty() && szOutLine.back() == '\r' )
	{
		szOutLine.resize( szOutLine.length() - 1 );
	}
}

bool CASSTDIOFile::Read( CASBLOB* pBlob, size_t uiSizeInBytes )
{
	assert( IsOpen() );

	if( !pBlob )
		return false;

	bool bSuccess = false;

	if( IsReading() )
	{
		if( uiSizeInBytes > 0 )
		{
			if( GetSize() - Tell() >= uiSizeInBytes )
			{
				const auto startOffset = pBlob->GetSize();

				pBlob->Resize( startOffset + uiSizeInBytes );

				auto pData = pBlob->GetData() + startOffset;

				memset( pData, 0, uiSizeInBytes );

				const auto written = fread( pData, 1, uiSizeInBytes, m_pFile );

				//Resize it in case less was read than was requested.
				pBlob->Resize( startOffset + written );

				bSuccess = true;
			}
		}
		else
		{
			//You read 0 bytes
			bSuccess = true;
		}
	}

	pBlob->Release();

	return bSuccess;
}

bool CASSTDIOFile::Read( CASBLOB* pBlob )
{
	//Read in everything you can
	return Read( pBlob, static_cast<size_t>( GetSize() - Tell() ) );
}

CASBLOB* CASSTDIOFile::ReadBlob( size_t uiSizeInBytes )
{
	assert( IsOpen() );

	//Always return a valid BLOB.
	CASBLOB* pBlob = new CASBLOB( uiSizeInBytes );

	if( IsReading() )
	{
		//Have to addref here because Read releases the reference it holds
		pBlob->AddRef();

		if( uiSizeInBytes > 0 )
			Read( pBlob, uiSizeInBytes );
		else
			Read( pBlob );
	}

	return pBlob;
}

uint64_t CASSTDIOFile::WriteBytes( const void* pData, size_t uiSizeInBytes )
{
	assert( IsOpen() );
	assert( pData );

	if( !pData || uiSizeInBytes == 0 )
		return 0;

	if( !IsWriting() )
		return 0;

	return fwrite( pData, 1, uiSizeInBytes, m_pFile );
}

void RegisterScriptSTDIOFile( asIScriptEngine& scriptEngine )
{
	const char* pszObjectName = "File";

	scriptEngine.RegisterObjectType( pszObjectName, 0, asOBJ_REF );

	as::RegisterRefCountedBaseClass<CASSTDIOFile>( &scriptEngine, pszObjectName );

	//Do not let the user close files. This class is a RAII style wrapper around files, so its destructor will close it.

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool IsOpen() const",
		asMETHOD( CASSTDIOFile, IsOpen ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint64 GetSize() const",
		asMETHOD( CASSTDIOFile, GetSize ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint64 Tell() const",
		asMETHOD( CASSTDIOFile, Tell ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool EOFReached() const",
		asMETHOD( CASSTDIOFile, EOFReached ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint64 Write(const " AS_STRING_OBJNAME "& in szString)",
		asMETHODPR( CASSTDIOFile, Write, ( const std::string& ), uint64_t ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint64 Write(const BLOB@ pBlob)",
		asMETHODPR( CASSTDIOFile, Write, ( const CASBLOB* ), uint64_t ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "void Flush()",
		asMETHOD( CASSTDIOFile, Flush ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, AS_STRING_OBJNAME " ReadCharacter()",
		asMETHOD( CASSTDIOFile, ReadCharacter ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "void ReadLine(" AS_STRING_OBJNAME "& out szOutLine, const " AS_STRING_OBJNAME "& in szDelim = '\n')",
		asMETHOD( CASSTDIOFile, ReadLine ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Read(BLOB@ pBlob, size_t uiSizeInBytes)",
		asMETHODPR( CASSTDIOFile, Read, ( CASBLOB*, size_t ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Read(BLOB@ pBlob)",
		asMETHODPR( CASSTDIOFile, Read, ( CASBLOB* ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "BLOB@ ReadBlob(size_t uiSizeInBytes = 0)",
		asMETHODPR( CASSTDIOFile, ReadBlob, ( size_t ), CASBLOB* ), asCALL_THISCALL );
}