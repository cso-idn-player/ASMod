#include <extdll.h>
#include <meta_api.h>

#include <Angelscript/util/IASLogger.h>

#include "keyvalues/Keyvalues.h"

#include "ScriptAPI/CASDirectory.h"

#include "ASMod/Module/CASModBaseModule.h"
#include "Module.h"
#include "ASMod/IASEnvironment.h"

#include "ScriptAPI/CASVirtualFileSystem.h"

#include "ConCommands.h"

/**
*	Writes a directory structure to a KeyValues text file.
*/
struct CWriteFileSystemToFile
{
	kv::Writer& writer;

	const bool bOutputExtraInfo;

	/**
	*	@param writer Keyvalues writer.
	*	@param bOutputExtraInfo Whether to output extra info, or just the directory structure.
	*/
	CWriteFileSystemToFile( kv::Writer& writer, const bool bOutputExtraInfo )
		: writer( writer )
		, bOutputExtraInfo( bOutputExtraInfo )
	{
	}

	/**
	*	Recursively writes the directory structure.
	*/
	void WriteDirectory( const CASDirectory* pDirectory )
	{
		if( !pDirectory )
			return;

		writer.BeginBlock( pDirectory->GetName() );

		if( bOutputExtraInfo )
		{
			const FileAccess_t access = pDirectory->GetAccess();
			const DirectoryFlags_t flags = pDirectory->GetFlags();

			if( access != FileAccessBit::NONE )
			{
				writer.BeginBlock( "access" );
				{
					for( FileAccess_t accessType = FileAccess::FIRST; accessType < FileAccess::COUNT; ++accessType )
					{
						if( access & ( 1 << accessType ) )
							writer.WriteKeyvalue( "access", FileAccess::ToString( static_cast<FileAccess::FileAccess>( accessType ) ) );
					}
				}
				writer.EndBlock();
			}

			if( flags != DirectoryFlagBit::NONE )
			{
				writer.BeginBlock( "flags" );
				{
					for( DirectoryFlags_t flag = DirectoryFlag::FIRST; flag < DirectoryFlag::COUNT; ++flag )
					{
						if( flags & ( 1 << flag ) )
							writer.WriteKeyvalue( "flag", DirectoryFlag::ToString( static_cast<DirectoryFlag::DirectoryFlag>( flag ) ) );
					}
				}
				writer.EndBlock();
			}
		}

		for( auto pDir = pDirectory->GetFirstChild(); pDir; pDir = pDir->GetNextSibling() )
		{
			WriteDirectory( pDir );
		}

		writer.EndBlock();
	}
};

/**
*	Dumps the Angelscript file system and blacklist to a file.
*/
void DumpFileSystem()
{
	if( CMD_ARGC() != 2 && CMD_ARGC() != 3 )
	{
		g_pModule->GetEnvironment().GetLogger()->Msg( "asmod_fs_dumpfilesystem usage: asmod_fs_dumpfilesystem <filename> [outputExtraInfo]\n" );
		return;
	}

	const bool bOutputExtraInfo = CMD_ARGC() == 3 ? atoi( CMD_ARGV( 2 ) ) != 0 : true;

	const char* const pszFilename = CMD_ARGV( 1 );

	char szBuffer[ PATH_MAX ];

	const auto result = snprintf( szBuffer, sizeof( szBuffer ), "%s/%s.txt", gpMetaUtilFuncs->pfnGetGameInfo( PLID, GINFO_GAMEDIR ), pszFilename );

	if( !PrintfSuccess( result, sizeof szBuffer ) )
	{
		g_pModule->GetEnvironment().GetLogger()->Critical( "Filename too long!\n" );
		return;
	}

	kv::Writer writer( szBuffer );

	if( writer.IsOpen() )
	{
		writer.BeginBlock( "FileSystem" );

		const CASDirectory* pDirectory = g_pASFileSystem->GetDirectoryList().GetRootDirectory();

		CWriteFileSystemToFile fsWriter( writer, bOutputExtraInfo );

		fsWriter.WriteDirectory( pDirectory );

		const CASExtensionList& blacklist = g_pASFileSystem->GetExtensionBlackList();

		const auto& extensions = blacklist.GetExtensions();

		if( !extensions.empty() )
		{
			writer.BeginBlock( "ExtensionBlacklist" );
			{
				for( const auto extension : extensions )
				{
					writer.WriteKeyvalue( "extension", extension.c_str() );
				}
			}
			writer.EndBlock();
		}

		writer.EndBlock();
	}

	if( !writer.IsOpen() || writer.ErrorOccurred() )
		g_pModule->GetEnvironment().GetLogger()->Critical( "An error occurred while writing filesystem to file '%s'\n", szBuffer );
	else
		g_pModule->GetEnvironment().GetLogger()->Msg( "Written filesystem to file '%s'\n", writer.GetFilename() );
}

void FileSystem_RegisterConCommands()
{
	REG_SVR_COMMAND( "asmod_fs_dumpfilesystem", &::DumpFileSystem );
}
