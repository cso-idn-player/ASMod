#ifndef FILESYSTEM_SCRIPTAPI_CASFILE_H
#define FILESYSTEM_SCRIPTAPI_CASFILE_H

#include <cstdint>
#include <cstdio>
#include <string>

#include <Angelscript/util/CASBaseClass.h>

#include "ASFileSystemConstants.h"

#include "CASBaseFile.h"

class asIScriptEngine;
class CASVirtualFileSystem;
class CASDirectory;
class CASBLOB;

/**
*	A file that scripts can open to read from or write to. Uses stdio to access files.
*/
class CASSTDIOFile : public CASBaseFile<CASSTDIOFile>
{
public:
	typedef CASBaseFile<CASSTDIOFile> BaseClass;
	typedef CASSTDIOFile ThisClass;

protected:
	template<typename SUBCLASS>
	friend class CASBaseFile;

public:
	/**
	*	Constructs a new file handle that points to the file named by pszFilename, which can be accessed using pFile.
	*	@param pszFilename Name of the file.
	*	@param uiOpenFlags Flags that were used to open this file.
	*	@param pFile File pointer.
	*/
	CASSTDIOFile( const char* const pszFilename, const OpenFileFlags_t uiOpenFlags, FILE* pFile );
	~CASSTDIOFile();

	void Release() const;

	/**
	*	@return Whether this file is open.
	*/
	bool IsOpen() const { return m_pFile != nullptr; }

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
	FILE* m_pFile;

private:
	CASSTDIOFile( const CASSTDIOFile& ) = delete;
	CASSTDIOFile& operator=( const CASSTDIOFile& ) = delete;
};

/**
*	Registers the File class using a stdio based file API.
*	@param scriptEngine Script engine.
*/
void RegisterScriptSTDIOFile( asIScriptEngine& scriptEngine );

#endif //FILESYSTEM_SCRIPTAPI_CASFILE_H
