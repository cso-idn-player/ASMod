#include <Angelscript/util/ASLogging.h>

#include "Platform.h"

#include "StringUtils.h"

#include "KeyvaluesHelpers.h"

void LogKeyvaluesMessage( void*, const char* pszFormat, ... )
{
	char szBuffer[ 1024 ];
	va_list list;

	va_start( list, pszFormat );
	const auto result = vsnprintf( szBuffer, sizeof( szBuffer ), pszFormat, list );
	va_end( list );

	if( !PrintfSuccess( result, sizeof( szBuffer ) ) )
	{
		as::Critical( "ASModLogKeyvaluesMessage: Couldn't format error message\n" );
		return;
	}

	//Strip newlines so we can handle them properly below.
	if( szBuffer[ result - 1 ] == '\n' )
		szBuffer[ result - 1 ] = '\0';

	as::Critical( "%s\n", szBuffer );
}

std::pair<bool, std::unique_ptr<kv::Block>> LoadKeyvaluesFile( const char* const pszBaseDirectory, const char* const pszFilename, const bool bOptional, const kv::LogFn logFn )
{
	assert( pszBaseDirectory );
	assert( pszFilename );
	assert( logFn );

	kv::CLogger logger{ logFn };

	logger( "Loading configuration file \"%s\"", pszFilename );

	if( !pszFilename || !( *pszFilename ) )
	{
		logger( "LoadKeyvaluesFile: Invalid filename" );
		return { false, nullptr };
	}

	char szConfigFilename[ MAX_PATH ];

	{
		const auto result = snprintf( szConfigFilename, sizeof( szConfigFilename ), "%s/%s", pszBaseDirectory, pszFilename );

		if( !PrintfSuccess( result, sizeof( szConfigFilename ) ) )
		{
			logger( "Error while formatting configuration filename \"%s\"", pszFilename );
			return{ false, nullptr };
		}
	}

	UTIL_FixSlashes( szConfigFilename );

	kv::Parser parser( szConfigFilename );

	if( !parser.HasInputData() )
	{
		if( !bOptional )
		{
			logger( "Config file \"%s\" is required and could not be opened for reading", pszFilename );
		}

		return{ false, nullptr };
	}

	//Convert escape sequences.
	parser.SetEscapeSeqConversion( GetEscapeSeqConversion() );

	parser.SetLogger( kv::CLogger( logger ) );

	const auto parseResult = parser.Parse();

	if( parseResult != kv::Parser::ParseResult::SUCCESS )
	{
		logger( "Error while parsing config \"%s\": %s", pszFilename, kv::Parser::ParseResultToString( parseResult ) );
		return{ false, nullptr };
	}

	return { true, std::unique_ptr<kv::Block>( parser.ReleaseKeyvalues() ) };
}
