#ifndef SQL_MODULE_H
#define SQL_MODULE_H

class CASModBaseModule;

/**
*	This module's instance.
*/
extern CASModBaseModule* g_pModule;

/**
*	Config file that contains the MySQL settings to use for default connections.
*/
extern cvar_t* as_mysql_config;

#endif //SQL_MODULE_H
