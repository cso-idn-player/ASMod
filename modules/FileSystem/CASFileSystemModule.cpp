#include <extdll.h>
#include <meta_api.h>

#include "interface.h"

#include <Angelscript/util/ASLogging.h>

#include "ASMod/IASEnvironment.h"
#include "ASMod/IASMod.h"

#include "keyvalues/Keyvalues.h"
#include "KeyvaluesHelpers.h"

#include "ScriptAPI/ASFileSystem.h"
#include "ScriptAPI/CASVirtualFileSystem.h"

#include "Module.h"
#include "ConCommands.h"

#include "MetaHelpers.h"

#include "CASFileSystemModule.h"

EXPOSE_SINGLE_INTERFACE( CASFileSystemModule, IASModModule, IASMODMODULE_NAME );

const char* CASFileSystemModule::GetName() const
{
	return "FileSystem";
}

const char* CASFileSystemModule::GetLogTag() const
{
	return "FILESYSTEM";
}

bool CASFileSystemModule::Initialize( const CreateInterfaceFn* pFactories, const size_t uiNumFactories, IASLogger* pLogger )
{
	if( !BaseClass::Initialize( pFactories, uiNumFactories, pLogger ) )
		return false;

	as::Diagnostic( "Registering FileSystem API\n" );

	auto& scriptEngine = *GetEnvironment().GetScriptEngine();

	RegisterScriptFileSystem( scriptEngine, "FS" );

	g_pASFileSystem = new CASVirtualFileSystem();

	scriptEngine.RegisterGlobalProperty( "FS::CVirtualFileSystem FileSystem", g_pASFileSystem );

	SetupDirectoryAccess( *g_pASFileSystem );

	//Clear temporary directories now.
	as::Diagnostic( "Clearing temporary directories\n" );
	g_pASFileSystem->GetDirectoryList().ClearTemporaryDirectories();

	FileSystem_RegisterConCommands();

	asmod_fs_logfileaccess = Meta_RegCVar( "asmod_fs_logfileaccess", "0", FCVAR_PROTECTED );

	return true;
}

bool CASFileSystemModule::Shutdown()
{
	as::Diagnostic( "Clearing temporary directories\n" );

	g_pASFileSystem->GetDirectoryList().ClearTemporaryDirectories();

	g_pASFileSystem->Shutdown();

	delete g_pASFileSystem;
	g_pASFileSystem = nullptr;

	return BaseClass::Shutdown();
}

void CASFileSystemModule::SetupDirectoryAccess( CASVirtualFileSystem& fileSystem )
{
	auto result = LoadKeyvaluesFile( GetASMod().GetLoaderDirectory(), ASMOD_CFG_FILESYSTEM, true );

	bool bInitializedFromFile = false;

	if( result.first )
	{
		auto pConfig = result.second.get();

		if( pConfig )
		{
			SetupDirectoryAccessFromKeyvalues( fileSystem, *pConfig );

			bInitializedFromFile = true;
		}
		else
		{
			as::Verbose( "Filesystem config was empty, setting up default directory access\n" );
		}
	}
	else
	{
		as::Verbose( "No Filesystem config file found, setting up default directory access\n" );
	}

	if( !bInitializedFromFile )
		SetupDefaultDirectoryAccess( fileSystem );

	//Create all writable directories now.
	fileSystem.GetDirectoryList().CreateWritableDirectories();
}

void CASFileSystemModule::SetupDefaultDirectoryAccess( CASVirtualFileSystem& fileSystem, const bool bResetToDefaults )
{
	//Do whatever you want, as long as the individual directories allow it.
	fileSystem.SetAllowedAccess( FileAccessBit::ALL );

	auto& dirList = fileSystem.GetDirectoryList();

	if( bResetToDefaults )
		dirList.RemoveAllDirectories();

	//Allow reading from scripts/ and any subdirectories (config files, etc).
	//Allow write access to scripts/<module type>/temp for temporary files.
	//Allow persistent write access to scripts/<module type>/store for persistent files.
	//Common
	dirList.CreateDirectory( ".", FileAccessBit::NONE );
	dirList.CreateDirectory( "scripts", FileAccessBit::READ, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES );

	//Plugins
	dirList.CreateDirectory( "scripts/plugins", FileAccessBit::READ, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES );
	dirList.CreateDirectory( "scripts/plugins/store", FileAccessBit::READ | FileAccessBit::WRITE, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES );
	dirList.CreateDirectory( "scripts/plugins/temp", FileAccessBit::READ | FileAccessBit::WRITE, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES | DirectoryFlagBit::TEMP );

	//Maps
	dirList.CreateDirectory( "scripts/maps", FileAccessBit::READ, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES );
	dirList.CreateDirectory( "scripts/maps/store", FileAccessBit::READ | FileAccessBit::WRITE, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES );
	dirList.CreateDirectory( "scripts/maps/temp", FileAccessBit::READ | FileAccessBit::WRITE, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES | DirectoryFlagBit::TEMP );

	//Allow reading from resource directories so config files can be read.
	//Allow access to subdirectories as well. (node graphs, models in subdirs, etc)
	dirList.CreateDirectory( "maps", FileAccessBit::READ, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES );
	dirList.CreateDirectory( "models", FileAccessBit::READ, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES );
	dirList.CreateDirectory( "sounds", FileAccessBit::READ, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES );
	dirList.CreateDirectory( "sprites", FileAccessBit::READ, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES );

	//Allow access to the addons directory so Metamod data can be read, but only from the ASMod scripts/ directory.
	dirList.CreateDirectory( "addons", FileAccessBit::NONE );
	dirList.CreateDirectory( "addons/ASMod", FileAccessBit::NONE );
	dirList.CreateDirectory( "addons/ASMod/scripts", FileAccessBit::READ, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES );
	dirList.CreateDirectory( "addons/ASMod/scripts/store", FileAccessBit::READ | FileAccessBit::WRITE, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES );
	dirList.CreateDirectory( "addons/ASMod/scripts/temp", FileAccessBit::READ | FileAccessBit::WRITE, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES | DirectoryFlagBit::TEMP );

	//Separate directory for configurations if they want it.
	dirList.CreateDirectory( "addons/ASMod/configs", FileAccessBit::READ, DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES );

	auto& blacklist = fileSystem.GetExtensionBlackList();

	if( bResetToDefaults )
		blacklist.RemoveAllExtensions();
}

void CASFileSystemModule::SetupDirectoryAccessFromKeyvalues( CASVirtualFileSystem& fileSystem, kv::Block& block, const bool bResetToDefaults )
{
	//Read in global access flags.
	auto pGlobalAccess = block.FindFirstChild<kv::Block>( "globalAccess" );

	if( pGlobalAccess )
	{
		const FileAccess_t globalAccess = ParseFileAccessFlags( *pGlobalAccess );

		fileSystem.SetAllowedAccess( globalAccess );
	}
	else
	{
		as::Verbose( "No global access settings found, filesystem access will be restricted\n" );

		if( bResetToDefaults )
			fileSystem.SetAllowedAccess( FileAccessBit::NONE );
	}

	as::Verbose( "Loading directory list\n" );

	if( bResetToDefaults )
		fileSystem.GetDirectoryList().RemoveAllDirectories();

	LoadDirectoryAccessFromKeyvalues( fileSystem, block );

	auto pBlacklist = block.FindFirstChild<kv::Block>( "extensionBlacklist" );

	if( bResetToDefaults )
		fileSystem.GetExtensionBlackList().RemoveAllExtensions();

	if( pBlacklist )
	{
		as::Verbose( "Loading extension blacklist\n" );

		LoadExtensionListFromKeyvalues( fileSystem.GetExtensionBlackList(), *pBlacklist );
	}
}

FileAccess_t CASFileSystemModule::ParseFileAccessFlags( kv::Block& block )
{
	FileAccess_t accessFlags = FileAccessBit::NONE;

	for( auto pAccessNode : block.GetChildrenByKey( "access" ) )
	{
		if( pAccessNode->GetType() != kv::NodeType::KEYVALUE )
		{
			as::Diagnostic( "Found non-keyvalue \"access\" node while parsing file access flags\n" );
			continue;
		}

		auto flag = FileAccess::FromString( static_cast<kv::KV*>( pAccessNode )->GetValue().c_str() );

		if( flag != FileAccess::NONE )
			accessFlags |= 1 << flag;
	}

	return accessFlags;
}

void CASFileSystemModule::LoadDirectoryAccessFromKeyvalues( CASVirtualFileSystem& fileSystem, kv::Block& block )
{
	auto pDirectories = block.FindFirstChild<kv::Block>( "directories" );

	//No access whatsoever.
	if( !pDirectories )
	{
		as::Verbose( "No directories to load\n" );
		return;
	}

	auto& dirList = fileSystem.GetDirectoryList();

	//Server operator can set their own access rules for the root directory.
	auto pRootDir = pDirectories->FindFirstChild<kv::Block>( "rootDirectory" );

	if( pRootDir )
	{
		as::Verbose( "Loading root directory settings\n" );
		LoadSingleDirectoryFromKeyvalues( dirList, *pRootDir, "." );
	}
	else
	{
		as::Verbose( "Using default root directory settings\n" );
		dirList.CreateDirectory( ".", FileAccessBit::NONE );
	}

	//The root directory already exists, so it can't be created by a regular directory entry.
	for( auto pNode : pDirectories->GetChildrenByKey( "directory" ) )
	{
		if( pNode->GetType() != kv::NodeType::BLOCK )
		{
			as::Diagnostic( "Found non-block \"directory\" node while loading directory access\n" );
			continue;
		}

		LoadSingleDirectoryFromKeyvalues( dirList, *static_cast<kv::Block*>( pNode ) );
	}
}

void CASFileSystemModule::LoadSingleDirectoryFromKeyvalues( CASDirectoryList& dirList, kv::Block& block, const char* pszOverridePath )
{
	if( !pszOverridePath )
	{
		auto pPath = block.FindFirstChild<kv::KV>( "path" );

		if( !pPath )
		{
			as::Diagnostic( "Couldn't find \"path\" for directory\n" );
			return;
		}

		if( pPath->GetType() != kv::NodeType::KEYVALUE )
		{
			as::Diagnostic( "Found non-keyvalue \"path\" node while loading directory access\n" );
			return;
		}

		pszOverridePath = pPath->GetValue().c_str();
	}

	const FileAccess_t accessFlags = ParseFileAccessFlags( block );

	DirectoryFlags_t dirFlags = DirectoryFlagBit::NONE;

	//TODO: could be refactored into a template function.
	for( auto pFlagNode : block.GetChildrenByKey( "flag" ) )
	{
		if( pFlagNode->GetType() != kv::NodeType::KEYVALUE )
		{
			as::Diagnostic( "Found non-keyvalue \"flag\" node while parsing directory flags\n" );
			continue;
		}

		auto flag = DirectoryFlag::FromString( static_cast<kv::KV*>( pFlagNode )->GetValue().c_str() );

		if( flag != DirectoryFlag::NONE )
			dirFlags |= 1 << flag;
	}

	dirList.CreateDirectory( pszOverridePath, accessFlags, dirFlags );
}

void CASFileSystemModule::LoadExtensionListFromKeyvalues( CASExtensionList& extensions, kv::Block& block )
{
	size_t uiCount = 0;

	for( auto pNode : block.GetChildrenByKey( "extension" ) )
	{
		if( pNode->GetType() != kv::NodeType::KEYVALUE )
		{
			as::Diagnostic( "Found non-keyvalue \"extension\" node while loading extension list\n" );
			continue;
		}

		if( extensions.AddExtension( static_cast<kv::KV*>( pNode )->GetValue().c_str() ) )
			++uiCount;
	}

	as::Verbose( "Loaded %u file extensions\n", uiCount );
}
