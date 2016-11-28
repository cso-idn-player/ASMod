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

#include "CASBLOB.h"

#include "CASSTDIOFile.h"

CASSTDIOFile::CASSTDIOFile( const char* const pszFilename, const OpenFileFlags_t uiOpenFlags, FILE* pFile )
	: BaseClass( pszFilename, uiOpenFlags )
	, m_pFile( pFile )
{
	assert( pFile != nullptr );
}

void CASSTDIOFile::Release() const
{
	if( InternalRelease() )
		delete this;
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

bool CASSTDIOFile::Error() const
{
	assert( IsOpen() );

	return !!ferror( m_pFile );
}

bool CASSTDIOFile::EOFReached() const
{
	assert( IsOpen() );

	return !!feof( m_pFile );
}

void CASSTDIOFile::Flush()
{
	assert( IsOpen() );

	if( !IsWriting() )
		return;

	fflush( m_pFile );
}

void CASSTDIOFile::CloseFile()
{
	fclose( m_pFile );
	m_pFile = nullptr;
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

uint64_t CASSTDIOFile::ReadBytes( void* pDest, size_t uiSizeInBytes, size_t uiElementCount )
{
	assert( IsOpen() );
	assert( pDest );

	if( !pDest || uiSizeInBytes == 0 || uiElementCount == 0 )
		return 0;

	if( !IsReading() )
		return 0;

	return fread( pDest, uiSizeInBytes, uiElementCount, m_pFile );
}

void RegisterScriptSTDIOFile( asIScriptEngine& scriptEngine )
{
	const char* pszObjectName = "File";

	RegisterScriptBaseFile<CASSTDIOFile>( scriptEngine, pszObjectName );
}
