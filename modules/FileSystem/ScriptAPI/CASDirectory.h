#ifndef FILESYSTEM_SCRIPTAPI_CASDIRECTORY_H
#define FILESYSTEM_SCRIPTAPI_CASDIRECTORY_H

#include "ASFileSystemConstants.h"

/**
*	Represents a single path in the virtual filesystem.
*	Uses a parent-child relationship.
*	<pre>
*			Parent
*	First Child	->	Sibling
*	</pre>
*/
class CASDirectory
{
protected:
	friend class CASDirectoryList;

public:
	/**
	*	@param pszName 
	*	@param access What kind of access is allowed.
	*	@param flags Directory flags.
	*/
	CASDirectory( const char* const pszName, const FileAccess_t access, 
		const DirectoryFlags_t flags = DirectoryFlagBit::NONE );
	~CASDirectory();

	/**
	*	@return The parent directory.
	*/
	const CASDirectory* GetParent() const { return m_pParent; }

	/**
	*	@copydoc GetParent() const
	*/
	CASDirectory* GetParent() { return m_pParent; }

	/**
	*	@return The first child directory.
	*/
	const CASDirectory* GetFirstChild() const { return m_pFirstChild; }

	/**
	*	@copydoc GetFirstChild() const
	*/
	CASDirectory* GetFirstChild() { return m_pFirstChild; }

	/**
	*	@return The next sibling to this directory in relation to the parent.
	*/
	const CASDirectory* GetNextSibling() const { return m_pNextSibling; }

	/**
	*	@copydoc GetNextSibling() const
	*/
	CASDirectory* GetNextSibling() { return m_pNextSibling; }

	/**
	*	@return Whether it has the given directory as an immediate child.
	*/
	bool HasChild( const CASDirectory* const pDirectory ) const;

	/**
	*	Finds an immediate child by name.
	*/
	const CASDirectory* FindChild( const char* const pszName ) const;

	/**
	*	@copydoc FindChild( const char* const pszName ) const
	*/
	CASDirectory* FindChild( const char* const pszName );

	/**
	*	@return The name of this directory.
	*/
	const char* GetName() const { return m_szName; }

	/**
	*	@return The bit vector containing what kind of access is allowed.
	*/
	FileAccess_t GetAccess() const { return m_Access; }

	/**
	*	@return The bit vector containing directory flags.
	*/
	DirectoryFlags_t GetFlags() const { return m_Flags; }

	/**
	*	@return Whether this directory can be written to.
	*/
	bool IsWriteable() const { return ( m_Access & FileAccessBit::WRITE ) != 0; }

	/**
	*	@return Whether this is a temporary directory. Temporary directories are cleared of all files at set moments.
	*/
	bool IsTemporary() const { return ( m_Flags & DirectoryFlagBit::TEMP ) != 0; }

private:
	/**
	*	Adds a child to the directory.
	*	@return Whether the directory was added.
	*/
	bool AddChild( CASDirectory* const pDirectory );

	/**
	*	Removes a child from the directory.
	*/
	void RemoveChild( CASDirectory* const pDirectory );

private:
	CASDirectory* m_pParent;				//!The parent directory, or nullptr if this is the top level directory
	CASDirectory* m_pFirstChild;			//!The first child that this directory has, or nullptr if it has no childrne
	CASDirectory* m_pNextSibling;			//!This directory's sibling. nullptr if it has no more siblings

	char m_szName[ PATH_MAX + 1 ];			//!Name of this directory

	const FileAccess_t m_Access;			//!What kind of access is allowed
	const DirectoryFlags_t m_Flags;			//!Directory flags

private:
	CASDirectory( const CASDirectory& ) = delete;
	CASDirectory& operator=( const CASDirectory& ) = delete;
};

#endif //FILESYSTEM_SCRIPTAPI_CASDIRECTORY_H
