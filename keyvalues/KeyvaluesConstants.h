#ifndef KEYVALUES_KEYVALUESCONSTANTS_H
#define KEYVALUES_KEYVALUESCONSTANTS_H

namespace keyvalues
{
/**
*	Node types.
*/
enum class NodeType
{
	KEYVALUE,
	BLOCK
};

/**
*	The control character used for quoted strings.
*/
const char CONTROL_QUOTE		= '\"';

/**
*	The control character used for block open statements.
*/
const char CONTROL_BLOCK_OPEN	= '{';

/**
*	The control character used for block close statements.
*/
const char CONTROL_BLOCK_CLOSE	= '}';

/**
*	Tab width for pretty printing.
*/
const size_t KEYVALUE_TAB_WIDTH = 4;
}

#endif //KEYVALUES_KEYVALUESCONSTANTS_H
