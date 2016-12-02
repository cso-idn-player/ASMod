#ifndef FILESYSTEM_SCRIPTAPI_CASVIRTUALFILESYSTEM_H
#define FILESYSTEM_SCRIPTAPI_CASVIRTUALFILESYSTEM_H

#include "ASFileSystemConstants.h"

#include "CASDirectoryList.h"
#include "CASFilterList.h"

class asIScriptEngine;
class CASSteamPipeFile;

/**
*	This class represents the virtual filesystem used by all Angelscript scripts.
*	The filesystem has a global access mode that controls what kind of access scripts can have at any given time.
*	This lets you restrict all input and/or output operations whenever you need to.
*
*	Access to files is controlled with a directory list.
*	The directory list creates a root node that restricts all access to itself.
*	You can add subdirectories that can have access enabled without affecting other directories (unless you set the IMPLICIT_SUBDIRECTORIES flag).
*	You can control whether scripts can read from and/or write to directories,
*	whether a directory's properties should implicitly apply to subdirectories that are not added to the list,
*	and whether the directory contains temporary data (cleared on startup and shutdown).
*
*	You can also add filters to restrict access to files based on regular expressions.
*/
class CASVirtualFileSystem
{
public:
	CASVirtualFileSystem() = default;
	~CASVirtualFileSystem() = default;

	/**
	*	@return The directory list.
	*/
	const CASDirectoryList& GetDirectoryList() const { return m_DirectoryList; }

	/**
	*	@copydoc GetDirectoryList() const
	*/
	CASDirectoryList& GetDirectoryList() { return m_DirectoryList; }

	/**
	*	@return The filter list.
	*/
	const CASFilterList& GetFilterList() const { return m_FilterList; }

	/**
	*	@copydoc GetFilterList() const
	*/
	CASFilterList& GetFilterList() { return m_FilterList; }

	/**
	*	@return The allowed file access modes.
	*/
	FileAccess_t GetAllowedAccess() const { return m_AllowedAccess; }

	/**
	*	Sets the allowed file access modes.
	*/
	void SetAllowedAccess( const FileAccess_t allowedAccess );

	/**
	*	Opens a file.
	*	@param pszFilename Name of the file to open.
	*	@param uiOpenFlags Open flags.
	*	@return If the file could be opened, the file. otherwise, null.
	*/
	CASSteamPipeFile* OpenFile( const char* const pszFilename, const OpenFileFlags_t uiOpenFlags );

	/**
	*	Removes the given file if you have permission to do so.
	*/
	void RemoveFile( const char* const pszFilename );

	/**
	*	Call on shutdown, clears filesystem data.
	*/
	void Shutdown();

private:
	CASDirectoryList m_DirectoryList;
	CASFilterList m_FilterList;

	FileAccess_t m_AllowedAccess = FileAccessBit::NONE;

private:
	CASVirtualFileSystem( const CASVirtualFileSystem& ) = delete;
	CASVirtualFileSystem& operator=( const CASVirtualFileSystem& ) = delete;
};

void RegisterScriptVirtualFileSystem( asIScriptEngine& engine );

#endif //FILESYSTEM_SCRIPTAPI_CASVIRTUALFILESYSTEM_H
