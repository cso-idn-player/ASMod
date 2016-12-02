#ifndef FILESYSTEM_CASFILESYSTEMMODULE_H
#define FILESYSTEM_CASFILESYSTEMMODULE_H

#include "keyvalues/KVForward.h"

#include "ASMod/ASModConstants.h"

#include "ASMod/Module/CASModBaseModule.h"

#include "ScriptAPI/ASFileSystemConstants.h"

class CASVirtualFileSystem;
class CASDirectoryList;
class CASFilterList;

/**
*	The file that defines filesystem directory access and the filter list.
*/
#define ASMOD_CFG_FILESYSTEM ( ASMOD_CFG_DIR "FileSystem.txt" )

class CASFileSystemModule : public CASModBaseModule
{
public:
	typedef CASFileSystemModule ThisClass;
	typedef CASModBaseModule BaseClass;

public:
	CASFileSystemModule() = default;
	~CASFileSystemModule() = default;

	const char* GetName() const override final;

	const char* GetLogTag() const override final;

	bool Initialize( const CreateInterfaceFn* pFactories, const size_t uiNumFactories, IASLogger* pLogger ) override;

	bool Shutdown() override;

private:
	/**
	*	Sets up directory access for the given filesystem.
	*	@param fileSystem FileSystem to configure.
	*/
	void SetupDirectoryAccess( CASVirtualFileSystem& fileSystem );

	/**
	*	Sets up the default directory access settings for the given filesystem.
	*	@param fileSystem FileSystem to configure.
	*	@param bResetToDefaults Whether to reset the FileSystem to its defaults before applying settings.
	*/
	void SetupDefaultDirectoryAccess( CASVirtualFileSystem& fileSystem, const bool bResetToDefaults = true );

	/**
	*	Sets up the directory access settings for the given filesystem from Keyvalues data.
	*	@param fileSystem FileSystem to configure.
	*	@param bResetToDefaults Whether to reset the FileSystem to its defaults before applying settings.
	*/
	void SetupDirectoryAccessFromKeyvalues( CASVirtualFileSystem& fileSystem, kv::Block& block, const bool bResetToDefaults = true );

	/**
	*	Parses the file access flags from a config file.
	*/
	static FileAccess_t ParseFileAccessFlags( kv::Block& block );

	/**
	*	Loads directory access from a config file.
	*	@param fileSystem FileSystem to configure.
	*/
	void LoadDirectoryAccessFromKeyvalues( CASVirtualFileSystem& fileSystem, kv::Block& block );

	void LoadSingleDirectoryFromKeyvalues( CASDirectoryList& dirList, kv::Block& block, const char* pszOverridePath = nullptr );

	/**
	*	Loads a filter list from a config file.
	*/
	void LoadFilterListFromKeyvalues( CASFilterList& filters, kv::Block& block );

private:
	CASFileSystemModule( const CASFileSystemModule& ) = delete;
	CASFileSystemModule& operator=( const CASFileSystemModule& ) = delete;
};

#endif //FILESYSTEM_CASFILESYSTEMMODULE_H
