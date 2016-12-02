#include <extdll.h>
#include <meta_api.h>

#include <Angelscript/util/ASLogging.h>

#include "keyvalues/Keyvalues.h"
#include "KeyvaluesHelpers.h"

#include "ScriptAPI/CASDirectory.h"

#include "ScriptAPI/CASVirtualFileSystem.h"

#include "Module.h"

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
*	Dumps the Angelscript file system and filter list to a file.
*/
void DumpFileSystem()
{
	if( CMD_ARGC() != 2 && CMD_ARGC() != 3 )
	{
		as::Msg( "asmod_fs_dumpfilesystem usage: asmod_fs_dumpfilesystem <filename> [outputExtraInfo]\n" );
		return;
	}

	const bool bOutputExtraInfo = CMD_ARGC() == 3 ? atoi( CMD_ARGV( 2 ) ) != 0 : true;

	const char* const pszFilename = CMD_ARGV( 1 );

	char szBuffer[ PATH_MAX ];

	const auto result = snprintf( szBuffer, sizeof( szBuffer ), "%s/%s.txt", gpMetaUtilFuncs->pfnGetGameInfo( PLID, GINFO_GAMEDIR ), pszFilename );

	if( !PrintfSuccess( result, sizeof szBuffer ) )
	{
		as::Critical( "Filename too long!\n" );
		return;
	}

	kv::Writer writer( szBuffer, GetEscapeSeqConversion() );

	if( writer.IsOpen() )
	{
		writer.BeginBlock( "FileSystem" );

		const CASDirectory* pDirectory = g_pASFileSystem->GetDirectoryList().GetRootDirectory();

		writer.BeginBlock( "directories" );
			CWriteFileSystemToFile fsWriter( writer, bOutputExtraInfo );

			fsWriter.WriteDirectory( pDirectory->GetFirstChild() );
		writer.EndBlock();

		const auto& filterlist = g_pASFileSystem->GetFilterList();

		const auto& filters = filterlist.GetFilters();

		if( !filters.empty() )
		{
			writer.BeginBlock( "filters" );
			{
				for( const auto& filter : filters )
				{
					writer.BeginBlock( "filter" );
						writer.WriteKeyvalue( "expression", filter.GetExpressionString().c_str() );
						WriteFlagsToKeyvalues<FilterFlag>( writer, filter.GetFlags(), "flag" );
					writer.EndBlock();
				}
			}
			writer.EndBlock();
		}

		writer.EndBlock();
	}

	if( !writer.IsOpen() || writer.ErrorOccurred() )
		as::Critical( "An error occurred while writing filesystem to file '%s'\n", szBuffer );
	else
		as::Msg( "Written filesystem to file '%s'\n", writer.GetFilename() );
}

void FileSystem_RegisterConCommands()
{
	REG_SVR_COMMAND( "asmod_fs_dumpfilesystem", &::DumpFileSystem );
}
