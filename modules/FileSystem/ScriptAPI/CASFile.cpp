#include <cassert>
#include <memory>
#include <sstream>

#include <angelscript.h>

#include <extdll.h>
#include <meta_api.h>

#include <Angelscript/util/IASLogger.h>

#include "Platform.h"

#include "ASMod/Module/CASModBaseModule.h"
#include "ASMod/IASEnvironment.h"
#include "Module.h"

#include "ASFileSystemUtils.h"

#include "CASVirtualFileSystem.h"
#include "CASDirectory.h"
#include "CASBLOB.h"

#include "CASFile.h"

CASFile::CASFile( const char* const pszFilename, const OpenFileFlags_t uiOpenFlags, FILE* pFile )
	: m_szFilename( pszFilename )
	, m_uiFlags( uiOpenFlags )
	, m_pFile( pFile )
{
	assert( pszFilename != nullptr );
	assert( pFile != nullptr );

	if( ASFileSystem_LogFileAccess() )
	{
		g_pModule->GetEnvironment().GetLogger()->Diagnostic( "File \"%s\" opened\n", pszFilename );
	}
}

CASFile::~CASFile()
{
	Close();
}

void CASFile::Release() const
{
	if( InternalRelease() )
		delete this;
}

void CASFile::Close()
{
	if( IsOpen() )
	{
		if( ASFileSystem_LogFileAccess() )
		{
			g_pModule->GetEnvironment().GetLogger()->Diagnostic( "File \"%s\" closed\n", m_szFilename.c_str() );
		}

		fclose( m_pFile );
		m_pFile = nullptr;
	}
}

uint64_t CASFile::GetSize() const
{
	assert( IsOpen() );

	//TODO: fileno isn't in the standard. - Solokiller
	auto fileNo = fileno( m_pFile );

	struct stat buf;

	if( !fstat( fileNo, &buf ) )
		return buf.st_size;

	return 0;
}

uint64_t CASFile::Tell() const
{
	assert( IsOpen() );
	return ftell( m_pFile );
}

bool CASFile::EOFReached() const
{
	assert( IsOpen() );

	return !!feof( m_pFile );
}

uint64_t CASFile::Write( const std::string& szString )
{
	assert( IsOpen() );

	if( !IsWriting() )
		return 0;

	return WriteBytes( szString.c_str(), szString.length() );
}

uint64_t CASFile::Write( const CASBLOB* pBlob )
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

std::string CASFile::ReadCharacter()
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

void CASFile::ReadLine( std::string& szOutLine, const std::string& szDelim )
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

bool CASFile::Read( CASBLOB* pBlob, size_t uiSizeInBytes )
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

bool CASFile::Read( CASBLOB* pBlob )
{
	//Read in everything you can
	return Read( pBlob, static_cast<size_t>( GetSize() - Tell() ) );
}

CASBLOB* CASFile::ReadBlob( size_t uiSizeInBytes )
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

uint64_t CASFile::WriteBytes( const void* pData, size_t uiSizeInBytes )
{
	assert( IsOpen() );
	assert( pData );

	if( !pData || uiSizeInBytes == 0 )
		return 0;

	if( !IsWriting() )
		return 0;

	return fwrite( pData, 1, uiSizeInBytes, m_pFile );
}

void RegisterScriptFile( asIScriptEngine& scriptEngine )
{
	const char* pszObjectName = "File";

	scriptEngine.RegisterObjectType( pszObjectName, 0, asOBJ_REF );

	as::RegisterRefCountedBaseClass<CASFile>( &scriptEngine, pszObjectName );

	//Do not let the user close files. This class is a RAII style wrapper around files, so its destructor will close it.

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool IsOpen() const",
		asMETHOD( CASFile, IsOpen ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint64 GetSize() const",
		asMETHOD( CASFile, GetSize ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint64 Tell() const",
		asMETHOD( CASFile, Tell ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool EOFReached() const",
		asMETHOD( CASFile, EOFReached ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, AS_STRING_OBJNAME " ReadCharacter()",
		asMETHOD( CASFile, ReadCharacter ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "void ReadLine(" AS_STRING_OBJNAME "& out szOutLine, const " AS_STRING_OBJNAME "& in szDelim = '\n')",
		asMETHOD( CASFile, ReadLine ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Read(BLOB@ pBlob, size_t uiSizeInBytes)",
		asMETHODPR( CASFile, Read, ( CASBLOB*, size_t ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Read(BLOB@ pBlob)",
		asMETHODPR( CASFile, Read, ( CASBLOB* ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "BLOB@ ReadBlob(size_t uiSizeInBytes = 0)",
		asMETHODPR( CASFile, ReadBlob, ( size_t ), CASBLOB* ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint64 Write(const " AS_STRING_OBJNAME "& in szString)",
		asMETHODPR( CASFile, Write, ( const std::string& ), uint64_t ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint64 Write(const BLOB@ pBlob)",
		asMETHODPR( CASFile, Write, ( const CASBLOB* ), uint64_t ), asCALL_THISCALL );
}