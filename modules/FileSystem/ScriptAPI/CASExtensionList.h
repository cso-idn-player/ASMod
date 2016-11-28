#ifndef FILESYSTEM_SCRIPTAPI_CASEXTENSIONLIST_H
#define FILESYSTEM_SCRIPTAPI_CASEXTENSIONLIST_H

#include <unordered_set>
#include <string>

#include "StringUtils.h"

/**
*	Contains a list of extensions.
*	Extensions are stored without the leading dot '.'.
*/
class CASExtensionList
{
public:
	typedef std::unordered_set<std::string, CStdStringHashI, CStdStringEqualToI> Extensions_t;

public:
	CASExtensionList() = default;
	~CASExtensionList() = default;

	/**
	*	@return The extensions set.
	*/
	const Extensions_t& GetExtensions() const { return m_Extensions; }

	/**
	*	Checks whether an extension is in the list.
	*	@param pszExtension Extension to check. This can be a complete filename, in which case bExtractExtension should be set to true.
	*	@param bExtractExtension Whether to extract the extension from the given string.
	*	@return Whether the extension is in the list.
	*/
	bool HasExtension( const char* const pszExtension, const bool bExtractExtension = false ) const;

	/*
	*	Adds an extension to the list.
	*	Leading and trailing whitespace is trimmed before addition.
	*/
	bool AddExtension( const char* const pszExtension );

	/**
	*	Removes an extension.
	*/
	void RemoveExtension( const char* const pszExtension );

	/**
	*	Removes all extensions.
	*/
	void RemoveAllExtensions();

private:
	Extensions_t m_Extensions;

private:
	CASExtensionList( const CASExtensionList& ) = delete;
	CASExtensionList& operator=( const CASExtensionList& ) = delete;
};

#endif //FILESYSTEM_SCRIPTAPI_CASEXTENSIONLIST_H
