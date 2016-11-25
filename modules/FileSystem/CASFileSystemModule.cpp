#include <extdll.h>
#include <meta_api.h>

#include "interface.h"

#include <Angelscript/util/IASLogger.h>

#include "ASMod/IASEnvironment.h"

#include "ScriptAPI/ASFileSystem.h"
#include "ScriptAPI/CASVirtualFileSystem.h"

#include "Module.h"
#include "ConCommands.h"

#include "MetaHelpers.h"

#include "CASFileSystemModule.h"

EXPOSE_SINGLE_INTERFACE( CASFileSystemModule, IASModModule, IASMODMODULE_NAME );

const char* CASFileSystemModule::GetName() const
{
	return "stub_module";
}

bool CASFileSystemModule::Initialize( const CreateInterfaceFn* pFactories, const size_t uiNumFactories )
{
	if( !BaseClass::Initialize( pFactories, uiNumFactories ) )
		return false;

	GetEnvironment().GetLogger()->Diagnostic( "Registering FileSystem API\n" );

	auto& scriptEngine = *GetEnvironment().GetScriptEngine();

	RegisterScriptFileSystem( scriptEngine, "FS" );

	g_pASFileSystem = new CASVirtualFileSystem();

	scriptEngine.RegisterGlobalProperty( "FS::CVirtualFileSystem FileSystem", g_pASFileSystem );

	SetupDirectoryAccess( *g_pASFileSystem );

	//Clear temporary directories now.
	GetEnvironment().GetLogger()->Diagnostic( "Clearing temporary directories\n" );
	g_pASFileSystem->GetDirectoryList().ClearTemporaryDirectories();

	FileSystem_RegisterConCommands();

	asmod_fs_logfileaccess = Meta_RegCVar( "asmod_fs_logfileaccess", "0", FCVAR_PROTECTED );

	return true;
}

bool CASFileSystemModule::Shutdown()
{
	GetEnvironment().GetLogger()->Diagnostic( "Clearing temporary directories\n" );

	g_pASFileSystem->GetDirectoryList().ClearTemporaryDirectories();

	g_pASFileSystem->Shutdown();

	delete g_pASFileSystem;
	g_pASFileSystem = nullptr;

	return BaseClass::Shutdown();
}

void CASFileSystemModule::SetupDirectoryAccess( CASVirtualFileSystem& fileSystem )
{
	//Do whatever you want, as long as the individual directories allow it.
	fileSystem.SetAllowedAccess( FileAccessBit::ALL );

	auto& dirList = fileSystem.GetDirectoryList();

	//TODO: letting server operators configure this would be good. - Solokiller

	//Allow reading from scripts/ and any subdirectories (config files, etc).
	//Allow write access to scripts/<module type>/temp for temporary files.
	//Allow persistent write access to scripts/<module type>/store for persistent files.
	//Common
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

	//Create all writable directories now.
	dirList.CreateWritableDirectories();
}
