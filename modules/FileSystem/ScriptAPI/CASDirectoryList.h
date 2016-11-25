#ifndef FILESYSTEM_SCRIPTAPI_CASDIRECTORYLIST_H
#define FILESYSTEM_SCRIPTAPI_CASDIRECTORYLIST_H

#include "ASFileSystemConstants.h"

#ifdef CreateDirectory
#undef CreateDirectory
#endif

#ifdef RemoveDirectory
#undef RemoveDirectory
#endif

class CASDirectory;

/**
*	Stores a list of directories known to the virtual filesystem.
*/
class CASDirectoryList
{
public:
	CASDirectoryList();
	~CASDirectoryList();

	/**
	*	@return The root directory.
	*/
	const CASDirectory* GetRootDirectory() const { return m_pRoot; }

	/**
	*	Finds a directory.
	*	@return nullptr if the directory could not be found.
	*		If bAllowTruncation is true, returns the last directory that was found in the path.
	*		If pbWasTruncated is set, contains whether trunctions occurred or not.
	*/
	const CASDirectory* FindDirectory( const char* const pszPath, const bool bAllowTruncation = false, bool* pbWasTruncated = nullptr ) const;

	/**
	*	@copydoc FindDirectory( const char* const pszPath, const bool bAllowTruncation = false, bool* pbWasTruncated = nullptr ) const
	*/
	CASDirectory* FindDirectory( const char* const pszPath, const bool bAllowTruncation = false, bool* pbWasTruncated = nullptr );

	/**
	*	Checks whether the given directory exists.
	*/
	bool HasDirectory( const char* const pszPath ) const { return FindDirectory( pszPath ) != nullptr; }

	/**
	*	Creates a new directory.
	*	The parent directory must exist.
	*	If the directory already exists, the existing directory is returned.
	*/
	CASDirectory* CreateDirectory( const char* const pszPath, const FileAccess_t access, 
								   const DirectoryFlags_t flags = DirectoryFlagBit::NONE );

	/**
	*	Removes a virtual directory and all of its children. Does not remove the directories on disk.
	*	The name is expected to be the real name, not a symbolic link.
	*/
	void RemoveDirectory( const char* const pszPath );

	/**
	*	Removes all virtual directories from the list. Does not remove the directories on disk.
	*/
	void RemoveAllDirectories();

	/**
	*	Creates the directory hierarchies for all writable directories.
	*/
	void CreateWritableDirectories();

	/**
	*	Clears all directories marked as temporary.
	*/
	void ClearTemporaryDirectories();

	/**
	*	Determines if the given directory can be accessed by the given module type and access mode
	*	If the path exists, and ppDirectory is non-null, it will contain the path
	*/
	bool CanAccessDirectory( const char* const pszPath, const FileAccessBit::FileAccessBit access, const CASDirectory** ppDirectory = nullptr ) const;

	/**
	*	Whether the given directory can be accessed at all using the given access mode.
	*/
	bool CanAccessDirectory( const CASDirectory* const pDirectory, const FileAccessBit::FileAccessBit access ) const;

private:
	/**
	*	Recursively removes a directory and its children.
	*/
	void RemoveDirectory( CASDirectory* pDirectory );

private:
	CASDirectory* m_pRoot;

private:
	CASDirectoryList( const CASDirectoryList& ) = delete;
	CASDirectoryList& operator=( const CASDirectoryList& ) = delete;
};

template<typename FUNCTOR>
bool EnumerateDirectories( FUNCTOR& callback, size_t& uiDepth, const CASDirectory* pDirectory, const char* const pszBuffer, char* pszCursor, const size_t uiBufferSize )
{
	if( !pDirectory ) 
		return false;

	if( !UTIL_SafeStrnCat( pszCursor, pDirectory->GetName(), uiBufferSize ) )
	{
		g_pModule->GetEnvironment().GetLogger()->Critical( "CASDirectoryList::ClearTempDirectories: Failed to append name '%s'!\n", pDirectory->GetName() );
		return false;
	}

	callback( pDirectory, pszBuffer, uiDepth );

	const size_t uiLength = strlen( pDirectory->GetName() );

	if( uiBufferSize < uiLength + 1 )
	{
		g_pModule->GetEnvironment().GetLogger()->Critical( "CASDirectoryList::ClearTempDirectories: Failed to append directory separator!\n" );
		return false;
	}

	UTIL_SafeStrnCat( pszCursor, "/", uiBufferSize );

	++uiDepth;

	for( const CASDirectory* pChild = pDirectory->GetFirstChild(); pChild; pChild = pChild->GetNextSibling() )
	{
		if( !EnumerateDirectories( callback, uiDepth, pChild, pszBuffer, pszCursor + uiLength + 1, uiBufferSize - ( uiLength + 1 ) ) )
			return false;
	}

	--uiDepth;

	//Reset for next
	*pszCursor = '\0';

	return true;
}

/*
*	Enumerates all directories for a given root directory
*	callback is expected to be a functor with the following arguments:
*	const CASDirectory* pDirectory, const char* const pszPath, const size_t uiDepth
*	Depth starts at 0 for the root
*/
template<typename FUNCTOR>
bool EnumerateDirectories( FUNCTOR& callback, const CASDirectory* pRoot )
{
	if( !pRoot )
		return false;

	char szPath[ PATH_MAX + 1 ];

	szPath[ 0 ] = '\0';

	size_t uiDepth = 0;

	return EnumerateDirectories( callback, uiDepth, pRoot, szPath, szPath, sizeof( szPath ) );
}

#endif //FILESYSTEM_SCRIPTAPI_CASDIRECTORYLIST_H
