#ifndef KEYVALUESHELPERS_H
#define KEYVALUESHELPERS_H

#include <memory>
#include <utility>

#include <Angelscript/util/ASLogging.h>

#include "keyvalues/Keyvalues.h"

/**
*	Logs keyvalues log messages to the Angelscript log.
*/
void LogKeyvaluesMessage( void*, const char* pszFormat, ... );

/**
*	Loads a keyvalues file and returns the data.
*	@param pszBaseDirectory Base directory to load files from.
*	@param pszFilename Name of the file to load. Can contain directories.
*	@param bOptional Whether this is an optional file.
*	@param logFn Logging function. Can be null.
*	@return Pair containing whether loading succeeded, and the parsed keyvalues data. If the file was empty, the data will be null.
*/
std::pair<bool, std::unique_ptr<kv::Block>> LoadKeyvaluesFile( 
	const char* const pszBaseDirectory, 
	const char* const pszFilename, 
	const bool bOptional, 
	const kv::LogFn logFn = &LogKeyvaluesMessage );

/**
*	Parses a set of flags from a keyvalues block.
*	Expects the constant NONE to be defined.
*	@param block Keyvalues block that has child keyvalues containing flags.
*	@param pszKeyName name of the keys that contain flags.
*	@return Bit vector containing all flags that were parsed in.
*	@tparam ENUM Enum type declared using Enum.h helper macros.
*/
template<typename ENUM>
inline typename ENUM::BitVec_t ParseFlagsFromKeyvalues( kv::Block& block, const char* const pszKeyName )
{
	ENUM::BitVec_t flags = ENUM::Bit::NONE;

	for( auto pFlagNode : block.GetChildrenByKey( pszKeyName ) )
	{
		if( pFlagNode->GetType() != kv::NodeType::KEYVALUE )
		{
			as::Diagnostic( "Found non-keyvalue \"flag\" node while parsing flags\n" );
			continue;
		}

		auto flag = ENUM::FromString( static_cast<kv::KV*>( pFlagNode )->GetValue().c_str() );

		if( flag != ENUM::NONE )
			flags |= 1 << flag;
	}

	return flags;
}

/**
*	Writes a bit vector to a keyvalues file, writing each flag as its own keyvalue.
*	Expects the constants FIRST and COUNT to be defined, which define a range [ FIRST, COUNT [.
*	@param writer Writer to use.
*	@param flags Flags to write.
*	@param pszKeyName Name of the keyvalues to write.
*	@tparam ENUM Enum type declared using Enum.h helper macros.
*/
template<typename ENUM>
void WriteFlagsToKeyvalues( kv::Writer& writer, typename ENUM::BitVec_t flags, const char* const pszKeyName )
{
	for( ENUM::BitVec_t flag = ENUM::FIRST; flag < ENUM::COUNT; ++flag )
	{
		if( flags & ( 1 << flag ) )
			writer.WriteKeyvalue( pszKeyName, ENUM::ToString( static_cast<ENUM::Enum>( flag ) ) );
	}
}

#endif //KEYVALUESHELPERS_H
