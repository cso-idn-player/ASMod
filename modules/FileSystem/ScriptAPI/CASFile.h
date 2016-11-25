#ifndef FILESYSTEM_SCRIPTAPI_CASFILE_H
#define FILESYSTEM_SCRIPTAPI_CASFILE_H

#include <cstdint>
#include <cstdio>
#include <string>

#include <Angelscript/util/CASBaseClass.h>

#include "ASFileSystemConstants.h"

class asIScriptEngine;
class CASVirtualFileSystem;
class CASDirectory;
class CASBLOB;

/**
*	A file that scripts can open to read from or write to.
*	TODO: if the SteamPipe filesystem is available, its extra directories need to be searched as well.
*/
class CASFile : public CASRefCountedBaseClass
{
public:
	/**
	*	Constructs a new file handle that points to the file named by pszFilename, which can be accessed using pFile.
	*	@param pszFilename Name of the file.
	*	@param uiOpenFlags Flags that were used to open this file.
	*	@param pFile File pointer.
	*/
	CASFile( const char* const pszFilename, const OpenFileFlags_t uiOpenFlags, FILE* pFile );
	~CASFile();

	/**
	*	@return The filename.
	*/
	const std::string& GetFilename() const { return m_szFilename; }

	/**
	*	@return The open flags.
	*/
	OpenFileFlags_t GetOpenFlags() const { return m_uiFlags; }

	void Release() const;

	/**
	*	@return Whether this file is open.
	*/
	bool IsOpen() const { return m_pFile != nullptr; }

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
	uint64_t GetSize() const;

	/**
	*	@return The current position of the read/write pointer.
	*/
	uint64_t Tell() const;

	/**
	*	@return Whether the end of the file has been reached. Only valid when reading.
	*/
	bool EOFReached() const;

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
	void Flush();

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
	*	@return Whether anything could be read.
	*/
	bool Read( CASBLOB* pBlob, size_t uiSizeInBytes );

	/**
	*	Reads as much as possible data from the file into the given blob.
	*	@return Whether anything could be read.
	*/
	bool Read( CASBLOB* pBlob );

	/**
	*	Reads data from the file into a blob.
	*	@param uiSizeInBytes How many bytes to read. If 0, reads in as much data as possible.
	*	@return The BLOB. If nothing could be read, the blob's size will be 0.
	*/
	CASBLOB* ReadBlob( size_t uiSizeInBytes = 0 );

protected:
	/**
	*	Writes data to the buffer.
	*	@param pData Buffer to write to the internal buffer.
	*	@param uiSizeInBytes Number of bytes to write.
	*	@return Number of bytes written.
	*/
	uint64_t WriteBytes( const void* pData, size_t uiSizeInBytes );

private:
	const std::string m_szFilename;
	const OpenFileFlags_t m_uiFlags;

	FILE* m_pFile;

private:
	CASFile( const CASFile& ) = delete;
	CASFile& operator=( const CASFile& ) = delete;
};

/**
*	Registers the File class.
*	@param scriptEngine Script engine.
*/
void RegisterScriptFile( asIScriptEngine& scriptEngine );

#endif //FILESYSTEM_SCRIPTAPI_CASFILE_H
