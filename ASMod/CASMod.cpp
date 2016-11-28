#include <extdll.h>			// always
#include <meta_api.h>

#include <Angelscript/util/ASLogging.h>
#include <Angelscript/util/CASFileLogger.h>

#include "FileSystem.h"

#include "ASMod/IASModModule.h"
#include "CASModModuleInfo.h"
#include "CASModLogger.h"

#include "keyvalues/Keyvalues.h"

#include "SvenCoopSupport.h"

#include "CASMod.h"

IFileSystem* g_pFileSystem = nullptr;

CASMod g_ASMod;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CASMod, IASMod, IASMOD_NAME, g_ASMod );

bool CASMod::Initialize()
{
	LOG_MESSAGE( PLID, "Initializing AngelScript Mod Loader" );

	{
		const char* pszPath = gpMetaUtilFuncs->pfnGetPluginPath( PLID );

		if( !pszPath )
		{
			LOG_ERROR( PLID, "Couldn't get plugin path" );
			return false;
		}

		UTIL_SafeStrncpy( m_szLoaderDir, pszPath, sizeof( m_szLoaderDir ) );
		UTIL_FixSlashes( m_szLoaderDir );

		//The path ends with dlls/ASMod.ext, so strip that part. -Solokiller

		{
			char* pszEnd = strrchr( m_szLoaderDir, '/' );

			//If this can't be found, something is very wrong since there should always be at least 3 separators (addons/plugin_name/dlls/lib.ext)
			if( !pszEnd )
			{
				LOG_ERROR( PLID, "Error while parsing plugin path \"%s\"", pszPath );
				return false;
			}

			*pszEnd = '\0';

			pszEnd = strrchr( m_szLoaderDir, '/' );

			if( !pszEnd )
			{
				LOG_ERROR( PLID, "Error while parsing plugin path \"%s\"", pszPath );
				return false;
			}

			*pszEnd = '\0';
		}

		//Should never be an empty string.
		if( !( *m_szLoaderDir ) )
		{
			LOG_ERROR( PLID, "Plugin path \"%s\" is invalid", pszPath );
			return false;
		}
	}

	if( !LoadConfig( ASMOD_CONFIG_FILENAME, false ) )
		return false;

	if( !LoadGameModule() )
		return false;

	if( !LoadFileSystemModule() )
		return false;

	if( !SetupEnvironment() )
		return false;

	if( !LoadModules() )
		return false;

	if( !LoadPlugins() )
		return false;

	LOG_MESSAGE( PLID, "Finished initializing AngelScript Mod Loader" );

	m_bFullyInitialized = true;

	return true;
}

void CASMod::Shutdown()
{
	LOG_MESSAGE( PLID, "Shutting down AngelScript Mod Loader" );

	m_bFullyInitialized = false;

	UnloadPlugins();

	UnloadModules();

	if( m_Logger )
	{
		as::SetLogger( nullptr );
		m_Logger.Reset();
	}

	if( m_FileLogger )
	{
		m_FileLogger.Reset();
	}

	if( UsingLocalEnvironment() )
	{
		//If we're handling the engine locally, shut it down and release it.
		auto pEngine = m_Environment.GetScriptEngine();

		if( pEngine )
		{
			pEngine->ShutDownAndRelease();
		}

		m_bUsingLocalEnvironment = false;
	}

	//Reset the environment to release any ref counted objects.
	m_Environment = std::move( CASSimpleEnvironment() );

	if( m_hFileSystem != nullptr )
	{
		g_pFileSystem = nullptr;
		Sys_UnloadModule( m_hFileSystem );
		m_hFileSystem = nullptr;
	}

	if( m_hGame != nullptr )
	{
		Sys_UnloadModule( m_hGame );
		m_hGame = nullptr;
		m_pGameFactory = nullptr;
	}
}

void CASMod::Think()
{
	if( !m_bFullyInitialized )
		return;

	for( auto& module : m_Modules )
	{
		module.GetModule()->Think();
	}
}

IBaseInterface* CASMod::QueryGameFactory( const char* pszName, int* pReturnCode )
{
	IBaseInterface* pInstance = nullptr;

	if( m_pGameFactory != nullptr )
	{
		pInstance = m_pGameFactory( pszName, pReturnCode );
	}
	else
	{
		if( m_hGame == nullptr )
		{
			LOG_ERROR( PLID, "Treid to query game factory with no game loaded!" );
		}

		if( pReturnCode )
			*pReturnCode = IFACE_FAILED;
	}

	return pInstance;
}

bool CASMod::LoadConfig( const char* pszConfigFilename, const bool bOptional )
{
	LOG_MESSAGE( PLID, "Loading configuration" );

	if( !pszConfigFilename || !( *pszConfigFilename ) )
	{
		LOG_ERROR( PLID, "CASMod::LoadConfig: Invalid filename" );
		return false;
	}

	char szConfigFilename[ PATH_MAX ];

	{
		const auto result = snprintf( szConfigFilename, sizeof( szConfigFilename ), "%s/%s", GetLoaderDirectory(), pszConfigFilename );

		if( !PrintfSuccess( result, sizeof( szConfigFilename ) ) )
		{
			LOG_ERROR( PLID, "Error while formatting configuration filename" );
			return false;
		}
	}

	UTIL_FixSlashes( szConfigFilename );

	kv::Parser parser( szConfigFilename );

	if( !parser.HasInputData() )
	{
		if( !bOptional )
		{
			LOG_ERROR( PLID, "Config file \"%s\" is required and could not be opened for reading", pszConfigFilename );
		}

		return false;
	}

	//Convert escape sequences.
	parser.SetEscapeSeqConversion( GetEscapeSeqConversion() );

	const auto parseResult = parser.Parse();

	if( parseResult != kv::Parser::ParseResult::SUCCESS )
	{
		LOG_ERROR( PLID, "Error while parsing config \"%s\": %s", pszConfigFilename, kv::Parser::ParseResultToString( parseResult ) );
		return false;
	}

	auto pConfig = parser.GetKeyvalues()->FindFirstChild<kv::Block>( "ASModConfig" );

	if( pConfig )
	{
		ApplyConfig( *pConfig );

		LOG_MESSAGE( PLID, "Config \"%s\" loaded", pszConfigFilename );
	}
	else
	{
		LOG_ERROR( PLID, "Config \"%s\" does not contain configuration data for ASMod", pszConfigFilename );
	}

	return true;
}

void CASMod::ApplyConfig( kv::Block& block )
{
	auto pLoader = block.FindFirstChild<kv::Block>( "loader" );

	if( pLoader )
	{
		auto pForceLocalEnvironment = pLoader->FindFirstChild<kv::KV>( "forceLocalEnvironment" );

		if( pForceLocalEnvironment )
		{
			m_EnvType = atoi( pForceLocalEnvironment->GetValue().CStr() ) != 0 ? EnvType::LOCAL : EnvType::DEFAULT;
		}

		auto pSvenCoopHack = pLoader->FindFirstChild<kv::KV>( "svenCoopHack" );

		if( pSvenCoopHack )
		{
			m_EnvType = atoi( pSvenCoopHack->GetValue().CStr() ) != 0 ? EnvType::SVENCOOP_HACK : m_EnvType;
		}
	}
}

bool CASMod::LoadGameModule()
{
	LOG_MESSAGE( PLID, "Loading game module" );

	const char* pszLibPath = GET_GAME_INFO( PLID, GINFO_REALDLL_FULLPATH );

	if( !pszLibPath )
	{
		LOG_ERROR( PLID, "Failed to retrieve real game path" );
		return false;
	}

	m_hGame = Sys_LoadModule( pszLibPath );

	if( m_hGame == nullptr )
	{
		LOG_ERROR( PLID, "Failed to load game module at \"%s\"", pszLibPath );
		return false;
	}

	//Very old mods won't have the factory function, so make sure it's an optional check.
	m_pGameFactory = Sys_GetFactory( m_hGame );

	LOG_MESSAGE( PLID, "Loaded game module factory: %s", m_pGameFactory ? "yes" : "no" );

	return true;
}

bool CASMod::LoadFileSystemModule()
{
	// Determine which filesystem to use.
#if defined ( _WIN32 )
	const char *szFsModule = "filesystem_stdio.dll";
#elif defined(OSX)
	const char *szFsModule = "filesystem_stdio.dylib";
#elif defined(LINUX)
	const char *szFsModule = "filesystem_stdio.so";
#else
#error
#endif

	LOG_MESSAGE( PLID, "Loading GoldSource FileSystem" );

	// Get filesystem interface.
	m_hFileSystem = Sys_LoadModule( szFsModule );

	assert( m_hFileSystem );

	if( !m_hFileSystem )
	{
		LOG_ERROR( PLID, "Couldn't load filesystem library" );
		return false;
	}

	CreateInterfaceFn fileSystemFactory = Sys_GetFactory( m_hFileSystem );

	if( !fileSystemFactory )
	{
		LOG_ERROR( PLID, "Couldn't get filesystem factory" );
		return false;
	}

	g_pFileSystem = static_cast<IFileSystem*>( fileSystemFactory( FILESYSTEM_INTERFACE_VERSION, nullptr ) );
	
	assert( g_pFileSystem );
	
	if( !g_pFileSystem )
	{
		LOG_ERROR( PLID, "Couldn't get filesystem" );
		return false;
	}

	return true;
}

bool CASMod::SetupEnvironment()
{
	LOG_MESSAGE( PLID, "Setting up environment" );

	//Reset to empty.
	m_Environment = std::move( CASSimpleEnvironment() );

	bool bGotEnvironment = false;

	if( m_Logger )
	{
		as::SetLogger( nullptr );
		m_Logger.Reset();
	}

	if( m_EnvType == EnvType::DEFAULT )
	{
		if( HasGameFactory() )
		{
			LOG_MESSAGE( PLID, "Querying game for environment" );

			auto pEnvironment = static_cast<IASEnvironment*>( QueryGameFactory( IASENVIRONMENT_NAME ) );

			if( pEnvironment )
			{
				m_Environment = std::move( CASSimpleEnvironment( *pEnvironment ) );

				bGotEnvironment = true;
				m_bUsingLocalEnvironment = false;
			}
			else
			{
				LOG_MESSAGE( PLID, "Game didn't provide an environment" );
			}
		}
		else
		{
			LOG_MESSAGE( PLID, "Game didn't provide a factory" );
		}
	}
	else if( m_EnvType == EnvType::LOCAL )
	{
		LOG_MESSAGE( PLID, "Forcing use of local environment" );
	}
	else if( m_EnvType == EnvType::SVENCOOP_HACK )
	{
		LOG_MESSAGE( PLID, "Using Sven Co-op hack to acquire environment" );

		sc::CSvenCoopSupport support( ASMOD_SCSUPPORT_FILENAME );

		if( !support.IsConfigLoaded() )
		{
			LOG_ERROR( PLID, "Couldn't load Sven Co-op Support config, aborting" );
			return false;
		}

		m_Environment = support.GetEnvironment();

		LOG_MESSAGE( PLID, "Acquired Sven Co-op Angelscript engine at %p\n\tVersion: %s\n\tLibrary options: %s", 
					 m_Environment.GetScriptEngine(),
					 m_Environment.GetLibVersionFunc()(),
					 m_Environment.GetLibOptionsFunc()()
		);

		bGotEnvironment = true;
	}
	else
	{
		LOG_ERROR( PLID, "Unknown environment setting" );
		return false;
	}

	if( !bGotEnvironment )
	{
		LOG_MESSAGE( PLID, "Using local environment" );

		m_Environment.SetAllocFunc( ::operator new );
		m_Environment.SetFreeFunc( ::operator delete );
		m_Environment.SetArrayAllocFunc( ::operator new[] );
		m_Environment.SetArrayFreeFunc( ::operator delete[] );

		asSetGlobalMemoryFunctions( m_Environment.GetAllocFunc(), m_Environment.GetFreeFunc() );

		m_Environment.SetScriptEngine( asCreateScriptEngine() );

		//TODO: configure engine.

		m_bUsingLocalEnvironment = true;
	}

	m_Logger = m_Environment.GetLogger();

	//Provide a logger if the game didn't.
	if( !m_Logger )
	{
		//Create the file logger.
		{
			char szLogPath[ PATH_MAX ];

			const auto result = snprintf( szLogPath, sizeof( szLogPath ), "%s/logs/LASMod", gpMetaUtilFuncs->pfnGetGameInfo( PLID, GINFO_GAMEDIR ) );

			if( !PrintfSuccess( result, sizeof( szLogPath ) ) )
			{
				//Fallback: log dir in main game directory.
				UTIL_SafeStrncpy( szLogPath, "logs/LASMod", sizeof( szLogPath ) );
			}

			m_FileLogger.Set( new CASFileLogger( szLogPath, CASFileLogger::Flag::USE_DATESTAMP | CASFileLogger::Flag::USE_TIMESTAMP | CASFileLogger::Flag::OUTPUT_LOG_LEVEL ), true );
		}

		//Combined file/console logging.
		m_Logger.Set( new CASModLogger( m_FileLogger.Get() ), true );

		m_Environment.SetLogger( m_Logger );
	}

	as::SetLogger( m_Logger );

	return m_Environment.IsValid();
}
