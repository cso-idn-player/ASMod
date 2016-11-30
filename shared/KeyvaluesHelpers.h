#ifndef KEYVALUESHELPERS_H
#define KEYVALUESHELPERS_H

#include <memory>
#include <utility>

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

#endif //KEYVALUESHELPERS_H
