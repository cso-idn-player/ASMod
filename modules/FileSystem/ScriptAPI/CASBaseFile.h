#ifndef FILESYSTEM_CASBASEFILE_H
#define FILESYSTEM_CASBASEFILE_H

#include <angelscript.h>

#include <cassert>
#include <sstream>
#include <string>

#include <Angelscript/util/ASLogging.h>
#include <Angelscript/util/CASBaseClass.h>

#include "Module.h"

#include "ASFileSystemConstants.h"
#include "CASBLOB.h"

/**
*	Baseclass for Angelscript files.
*	This class defines wrapper methods that call the subclass, do not use as a pointer type.
*
*	Subclasses must provide:
*	IsOpen
*	GetSize
*	Tell
*	Error
*	EOFReached
*	Flush
*	void CloseFile(): Closes the file. Called only if IsOpen() returns true.
*	WriteBytes
*	ReadBytes
*	TODO: add seeking support
*/
template<typename SUBCLASS>
class CASBaseFile : public CASRefCountedBaseClass
{
public:
	typedef SUBCLASS SubClass_t;

protected:
	/**
	*	Constructs a new file handle that points to the file named by pszFilename, which can be accessed using pFile.
	*	@param pszFilename Name of the file.
	*	@param uiOpenFlags Flags that were used to open this file.
	*/
	CASBaseFile( const char* const pszFilename, const OpenFileFlags_t uiOpenFlags );

public:
	~CASBaseFile();

	/**
	*	@return The filename.
	*/
	const std::string& GetFilename() const { return m_szFilename; }

	/**
	*	@return The open flags.
	*/
	OpenFileFlags_t GetOpenFlags() const { return m_uiFlags; }

	/**
	*	@return Whether this file is open.
	*/
	bool IsOpen() const
	{
		return static_cast<const SubClass_t*>( this )->IsOpen();
	}

	/**
	*	@return Whether this file was opened for reading.
	*/
	bool IsReading() const { return ( m_uiFlags & OpenFileBit::IMASK ) != 0; }

	/**
	*	@return Whether this file was opened for writing.
	*/
	bool IsWriting() const { return ( m_uiFlags & OpenFileBit::OMASK ) != 0; }

	/**
	*	@return Whether this file was opened in binary mode.
	*/
	bool IsBinary() const { return ( m_uiFlags & OpenFileBit::BINARY ) != 0; }

	/**
	*	Closes the file if it is open.
	*/
	void Close();

	/**
	*	@return The size of the file.
	*/
	uint64_t GetSize() const
	{
		return static_cast<const SubClass_t*>( this )->GetSize();
	}

	/**
	*	@return The current position of the read/write pointer.
	*/
	uint64_t Tell() const
	{
		return static_cast<const SubClass_t*>( this )->Tell();
	}

	/**
	*	@return Whether any errors occurred.
	*/
	bool Error() const
	{
		return static_cast<const SubClass_t*>( this )->Error();
	}

	/**
	*	@return Whether the end of the file has been reached. Only valid when reading.
	*/
	bool EOFReached() const
	{
		return static_cast<const SubClass_t*>( this )->EOFReached();
	}

	/**
	*	Writes a string.
	*	@return Number of bytes written.
	*/
	uint64_t Write( const std::string& szString );

	/**
	*	Writes the contents of the given blob to the file.
	*	@return Number of bytes written.
	*/
	uint64_t Write( const CASBLOB* pBlob );

	/**
	*	Flushes any pending writes to disk.
	*/
	void Flush()
	{
		static_cast<SubClass_t*>( this )->Flush();
	}

	/**
	*	Reads a single character. Returns '\0' if nothing could be read. (an empty string)
	*/
	std::string ReadCharacter();

	/**
	*	Reads a line. Reads until either the end of the file is reached, or until the given delimiter is reached.
	*	The delimiter is a string because characters are not supported.
	*/
	void ReadLine( std::string& szOutLine, const std::string& szDelim = "\n" );

	/**
	*	Reads up to uiSizeInBytes from the file into the given blob.
	*	@return Whether the read operation succeeded.
	*/
	bool Read( CASBLOB* pBlob, size_t uiSizeInBytes );

	/**
	*	Reads as much as possible data from the file into the given blob.
	*	@return Whether the read operation succeeded.
	*/
	bool Read( CASBLOB* pBlob );

	/**
	*	Reads data from the file into a blob.
	*	@param uiSizeInBytes How many bytes to read.
	*	@return The BLOB. If nothing could be read, the blob's size will be 0.
	*/
	CASBLOB* ReadBlob( size_t uiSizeInBytes );

	/**
	*	Reads data from the file into a blob. Reads as much data as possible.
	*	@return The BLOB. If nothing could be read, the blob's size will be 0.
	*/
	CASBLOB* ReadBlob();

private:
	/**
	*	Writes data to the file.
	*	@param pData Buffer to write to the file.
	*	@param uiSizeInBytes Number of bytes to write.
	*	@return Number of bytes written.
	*/
	uint64_t WriteBytes( const void* pData, size_t uiSizeInBytes )
	{
		return static_cast<SubClass_t*>( this )->WriteBytes( pData, uiSizeInBytes );
	}

	/**
	*	Reads data to the destination buffer.
	*	@param pDest Buffer to write data to.
	*	@param uiSizeInBytes Size of one element.
	*	@param uiElementCount Number of Elements to read.
	*	@return Number of bytes read.
	*/
	uint64_t ReadBytes( void* pDest, size_t uiSizeInBytes, size_t uiElementCount )
	{
		return static_cast<SubClass_t*>( this )->ReadBytes( pDest, uiSizeInBytes, uiElementCount );
	}

private:
	const std::string m_szFilename;
	const OpenFileFlags_t m_uiFlags;

private:
	CASBaseFile( const CASBaseFile& ) = delete;
	CASBaseFile& operator=( const CASBaseFile& ) = delete;
};

template<typename SUBCLASS>
CASBaseFile<SUBCLASS>::CASBaseFile( const char* const pszFilename, const OpenFileFlags_t uiOpenFlags )
	: m_szFilename( pszFilename )
	, m_uiFlags( uiOpenFlags )
{
	assert( pszFilename != nullptr );

	if( ASFileSystem_LogFileAccess() )
	{
		as::Diagnostic( "File \"%s\" opened\n", pszFilename );
	}
}

template<typename SUBCLASS>
CASBaseFile<SUBCLASS>::~CASBaseFile()
{
	Close();
}

template<typename SUBCLASS>
void CASBaseFile<SUBCLASS>::Close()
{
	if( IsOpen() )
	{
		if( ASFileSystem_LogFileAccess() )
		{
			as::Diagnostic( "File \"%s\" closed\n", m_szFilename.c_str() );
		}

		static_cast<SubClass_t*>( this )->CloseFile();
	}
}

template<typename SUBCLASS>
uint64_t CASBaseFile<SUBCLASS>::Write( const std::string& szString )
{
	assert( IsOpen() );

	if( !IsWriting() )
		return 0;

	return WriteBytes( szString.c_str(), szString.length() );
}

template<typename SUBCLASS>
uint64_t CASBaseFile<SUBCLASS>::Write( const CASBLOB* pBlob )
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

template<typename SUBCLASS>
std::string CASBaseFile<SUBCLASS>::ReadCharacter()
{
	assert( IsOpen() );

	if( !IsReading() )
		return "";

	if( EOFReached() )
		return "";

	char character[ 2 ] = { '\0', '\0' };

	ReadBytes( character, 1, 1 );

	return character;
}

template<typename SUBCLASS>
void CASBaseFile<SUBCLASS>::ReadLine( std::string& szOutLine, const std::string& szDelim )
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
		if( Error() || EOFReached() )
			break;

		if( ReadBytes( &character, 1, 1 ) == 0 )
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

template<typename SUBCLASS>
bool CASBaseFile<SUBCLASS>::Read( CASBLOB* pBlob, size_t uiSizeInBytes )
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

				const auto written = ReadBytes( pData, 1, uiSizeInBytes );

				//Resize it in case less was read than was requested.
				pBlob->Resize( startOffset + static_cast<size_t>( written ) );

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

template<typename SUBCLASS>
bool CASBaseFile<SUBCLASS>::Read( CASBLOB* pBlob )
{
	//Read in everything you can
	return Read( pBlob, static_cast<size_t>( GetSize() - Tell() ) );
}

template<typename SUBCLASS>
CASBLOB* CASBaseFile<SUBCLASS>::ReadBlob( size_t uiSizeInBytes )
{
	assert( IsOpen() );

	//Always return a valid BLOB.
	CASBLOB* pBlob = new CASBLOB( uiSizeInBytes );

	if( IsReading() )
	{
		//Have to addref here because Read releases the reference it holds
		pBlob->AddRef();

		Read( pBlob, uiSizeInBytes );
	}

	return pBlob;
}

template<typename SUBCLASS>
CASBLOB* CASBaseFile<SUBCLASS>::ReadBlob()
{

	assert( IsOpen() );

	//Always return a valid BLOB.
	CASBLOB* pBlob = new CASBLOB();

	if( IsReading() )
	{
		//Have to addref here because Read releases the reference it holds
		pBlob->AddRef();

		Read( pBlob );
	}

	return pBlob;
}

/**
*	Registers shared methods for file classes.
*	@param scriptEngine Script engine.
*	@param pszObjectName Name of the class.
*/
template<typename CLASS>
void RegisterScriptBaseFile( asIScriptEngine& scriptEngine, const char* const pszObjectName )
{
	scriptEngine.RegisterObjectType( pszObjectName, 0, asOBJ_REF );

	as::RegisterRefCountedBaseClass<CLASS>( &scriptEngine, pszObjectName );

	//Do not let the user close files. This class is a RAII style wrapper around files, so its destructor will close it.

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool IsOpen() const",
		asMETHOD( CLASS, IsOpen ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint64 GetSize() const",
		asMETHOD( CLASS, GetSize ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint64 Tell() const",
		asMETHOD( CLASS, Tell ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Error() const",
		asMETHOD( CLASS, Error ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool EOFReached() const",
		asMETHOD( CLASS, EOFReached ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint64 Write(const " AS_STRING_OBJNAME "& in szString)",
		asMETHODPR( CLASS, Write, ( const std::string& ), uint64_t ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "uint64 Write(const BLOB@ pBlob)",
		asMETHODPR( CLASS, Write, ( const CASBLOB* ), uint64_t ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "void Flush()",
		asMETHOD( CLASS, Flush ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, AS_STRING_OBJNAME " ReadCharacter()",
		asMETHOD( CLASS, ReadCharacter ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "void ReadLine(" AS_STRING_OBJNAME "& out szOutLine, const " AS_STRING_OBJNAME "& in szDelim = '\n')",
		asMETHOD( CLASS, ReadLine ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Read(BLOB@ pBlob, uint uiSizeInBytes)",
		asMETHODPR( CLASS, Read, ( CASBLOB*, size_t ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "bool Read(BLOB@ pBlob)",
		asMETHODPR( CLASS, Read, ( CASBLOB* ), bool ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "BLOB@ ReadBlob(uint uiSizeInBytes)",
		asMETHODPR( CLASS, ReadBlob, ( size_t ), CASBLOB* ), asCALL_THISCALL );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "BLOB@ ReadBlob()",
		asMETHODPR( CLASS, ReadBlob, (), CASBLOB* ), asCALL_THISCALL );
}

#endif //FILESYSTEM_CASBASEFILE_H
