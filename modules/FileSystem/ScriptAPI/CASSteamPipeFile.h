#ifndef FILESYSTEM_SCRIPTAPI_CASSTEAMPIPEFILE_H
#define FILESYSTEM_SCRIPTAPI_CASSTEAMPIPEFILE_H

#include <cstdint>
#include <cstdio>
#include <string>

#include <Angelscript/util/CASBaseClass.h>

#include "FileSystem.h"

#include "ASFileSystemConstants.h"

#include "CASBaseFile.h"

class asIScriptEngine;
class CASBLOB;

/**
*	A file that scripts can open to read from or write to. Uses stdio to access files.
*/
class CASSteamPipeFile : public CASBaseFile<CASSteamPipeFile>
{
public:
	typedef CASBaseFile<CASSteamPipeFile> BaseClass;
	typedef CASSteamPipeFile ThisClass;

protected:
	friend class CASBaseFile<CASSteamPipeFile>;

public:
	/**
	*	Constructs a new file handle that points to the file named by pszFilename, which can be accessed using hFile.
	*	@param pszFilename Name of the file.
	*	@param uiOpenFlags Flags that were used to open this file.
	*	@param hFile File handle.
	*/
	CASSteamPipeFile( const char* const pszFilename, const OpenFileFlags_t uiOpenFlags, FileHandle_t hFile );
	~CASSteamPipeFile() = default;

	void Release() const;

	/**
	*	@return Whether this file is open.
	*/
	bool IsOpen() const { return m_hFile != FILESYSTEM_INVALID_HANDLE; }

	/**
	*	@return The size of the file.
	*/
	uint64_t GetSize() const;

	/**
	*	@return The current position of the read/write pointer.
	*/
	uint64_t Tell() const;

	/**
	*	@return Whether any errors occurred.
	*/
	bool Error() const;

	/**
	*	@return Whether the end of the file has been reached. Only valid when reading.
	*/
	bool EOFReached() const;

	/**
	*	Flushes any pending writes to disk.
	*/
	void Flush();

protected:
	void CloseFile();

	uint64_t WriteBytes( const void* pData, size_t uiSizeInBytes );

	uint64_t ReadBytes( void* pDest, size_t uiSizeInBytes, size_t uiElementCount );

private:
	FileHandle_t m_hFile;

private:
	CASSteamPipeFile( const CASSteamPipeFile& ) = delete;
	CASSteamPipeFile& operator=( const CASSteamPipeFile& ) = delete;
};

/**
*	Registers the File class using a SteamPipe based file API.
*	@param scriptEngine Script engine.
*/
void RegisterScriptSteamPipeFile( asIScriptEngine& scriptEngine );

#endif //FILESYSTEM_SCRIPTAPI_CASSTEAMPIPEFILE_H
