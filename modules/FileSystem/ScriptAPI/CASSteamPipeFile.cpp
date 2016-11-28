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

#include "CASSteamPipeFile.h"

CASSteamPipeFile::CASSteamPipeFile( const char* const pszFilename, const OpenFileFlags_t uiOpenFlags, FileHandle_t hFile )
	: BaseClass( pszFilename, uiOpenFlags )
	, m_hFile( hFile )
{
	assert( hFile != FILESYSTEM_INVALID_HANDLE );
}

void CASSteamPipeFile::Release() const
{
	if( InternalRelease() )
		delete this;
}

uint64_t CASSteamPipeFile::GetSize() const
{
	assert( IsOpen() );

	return g_pFileSystem->Size( m_hFile );
}

uint64_t CASSteamPipeFile::Tell() const
{
	assert( IsOpen() );

	return g_pFileSystem->Tell( m_hFile );
}

bool CASSteamPipeFile::Error() const
{
	assert( IsOpen() );

	return !g_pFileSystem->IsOk( m_hFile );
}

bool CASSteamPipeFile::EOFReached() const
{
	assert( IsOpen() );

	return g_pFileSystem->EndOfFile( m_hFile );
}

void CASSteamPipeFile::Flush()
{
	assert( IsOpen() );

	if( !IsWriting() )
		return;

	g_pFileSystem->Flush( m_hFile );
}

void CASSteamPipeFile::CloseFile()
{
	g_pFileSystem->Close( m_hFile );
	m_hFile = FILESYSTEM_INVALID_HANDLE;
}

uint64_t CASSteamPipeFile::WriteBytes( const void* pData, size_t uiSizeInBytes )
{
	assert( IsOpen() );
	assert( pData );

	if( !pData || uiSizeInBytes == 0 )
		return 0;

	if( !IsWriting() )
		return 0;

	return g_pFileSystem->Write( pData, uiSizeInBytes, m_hFile );
}

uint64_t CASSteamPipeFile::ReadBytes( void* pDest, size_t uiSizeInBytes, size_t uiElementCount )
{
	assert( IsOpen() );
	assert( pDest );

	if( !pDest || uiSizeInBytes == 0 || uiElementCount == 0 )
		return 0;

	if( !IsReading() )
		return 0;

	return g_pFileSystem->Read( pDest, uiSizeInBytes * uiElementCount, m_hFile );
}

void RegisterScriptSteamPipeFile( asIScriptEngine& scriptEngine )
{
	const char* pszObjectName = "File";

	RegisterScriptBaseFile<CASSteamPipeFile>( scriptEngine, pszObjectName );
}
