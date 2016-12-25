###################################################
#                                                 #
#                                                 #
#   ASMod SQL module CMake build file             #
#                                                 #
#                                                 #
###################################################

#Change this to match your module's directory structure.
set( MODULE_LIB_DIR ${MODULE_BASE_DIRECTORY}/SQL/dlls )

set( MODULE_NAME SQL )

#Set module specific linker flags here.
#Delay load SQL libraries
set( MODULE_LINK_FLAGS )

if( MSVC )
	set( MODULE_LINK_FLAGS
		${MODULE_LINK_FLAGS}
		"/DELAYLOAD:sqlite3.dll /DELAYLOAD:libmariadb.dll"
	)
endif()

#Add module sources here, or using add_subdirectory.
add_sources(
	${SHARED_SOURCES}
	${SHARED_MODULE_SOURCES}
	ASHLSQL.h
	ASHLSQL.cpp
	CASSQLModule.h
	CASSQLModule.cpp
	Module.h
	Module.cpp
)

#Add public module headers here, if any.
#add_includes(
#
#)

#Process source files for inclusion.
preprocess_sources()

add_library( ${MODULE_NAME} SHARED ${PREP_SRCS} )

#Add include paths here.
target_include_directories( ${MODULE_NAME} PRIVATE
	.
	${SHARED_INCLUDE_PATHS}
	${SHARED_MODULE_INCLUDES}
	${CMAKE_SOURCE_DIR}/external/ASSQL/include
	${CMAKE_SOURCE_DIR}/external/CTPL/include
	${CMAKE_SOURCE_DIR}/external/SQLite/include
	${CMAKE_SOURCE_DIR}/external/MariaDB/include
)

#Define preprocessor symbols here.
target_compile_definitions( ${MODULE_NAME} PRIVATE
	${SHARED_DEFINITIONS}
	${SHARED_MODULE_DEFS}
)

#Add library dependencies here.
find_library( ASSQL AngelscriptSQL PATHS ${CMAKE_SOURCE_DIR}/external/ASSQL/lib/ )
find_library( ASSQLD AngelscriptSQLd PATHS ${CMAKE_SOURCE_DIR}/external/ASSQL/lib/ )

if( WIN32 )
	find_library( SQLITE3 "sqlite3${CMAKE_STATIC_LIBRARY_SUFFIX}" PATHS ${CMAKE_SOURCE_DIR}/external/SQLite/lib )
	find_library( MARIADB "mariadbclient${CMAKE_STATIC_LIBRARY_SUFFIX}" PATHS ${CMAKE_SOURCE_DIR}/external/MariaDB/lib )
	find_library( MARIADBD "mariadbclientd${CMAKE_STATIC_LIBRARY_SUFFIX}" PATHS ${CMAKE_SOURCE_DIR}/external/MariaDB/lib )
	set( PTHREAD "" )
else()
	#The first time you generate the CMake projects there won't be libraries in the destination directory, so we'll link with the local ones instead.
	#After you've built the project the first time, regenerate the CMake project to link with the correct ones.
	#Avoid linking with libraries installed in the local system's library directories, always use the version distributed with this repository.
	find_library( SQLITE3 "${CMAKE_SHARED_LIBRARY_PREFIX}sqlite3${CMAKE_SHARED_LIBRARY_SUFFIX}" PATHS "${ASMOD_BIN_DIR}" ${CMAKE_SOURCE_DIR}/external/SQLite/lib NO_DEFAULT_PATH )
	find_library( MARIADB "${CMAKE_SHARED_LIBRARY_PREFIX}mariadb.so.2" PATHS "${ASMOD_BIN_DIR}" ${CMAKE_SOURCE_DIR}/external/MariaDB/lib NO_DEFAULT_PATH )
	set( MARIADBD ${MARIADB} )
	set( PTHREAD "pthread" )
	
	#This mucking about is needed because CMake can't figure out how to handle libraries not named lib<name>.a
	set( ASSQL "-l:\"${ASSQL}\"" )
	set( ASSQLD "-l:\"${ASSQLD}\"" )
endif()

#ASSQL depends on AngelscriptUtils, so ensure the link order is correct.
target_link_libraries( ${MODULE_NAME}
	"${SQLITE3}"
	optimized "${MARIADB}"
	debug "${MARIADBD}"
	optimized "${ASSQL}"
	debug "${ASSQLD}"
	${SHARED_LIBRARY_DEPS}
	${SHARED_MODULE_LIBRARIES}
)

#If the user wants automatic deployment to a game directory, set the output directory paths.
if( DEPLOY_TO_GAME )
	#CMake places libraries in /Debug or /Release on Windows, so explicitly set the paths for both.
	#On Linux, it uses LIBRARY_OUTPUT_DIRECTORY
	set_target_properties( ${MODULE_NAME} PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${MODULE_LIB_DIR}
		RUNTIME_OUTPUT_DIRECTORY_DEBUG ${MODULE_LIB_DIR}
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${MODULE_LIB_DIR}
	)
endif()

#Set 32 bit flag, any module specific flags.
set_target_properties( ${MODULE_NAME} 
	PROPERTIES COMPILE_FLAGS "${LINUX_32BIT_FLAG}" 
	LINK_FLAGS "${SHARED_LINKER_FLAGS} ${MODULE_LINK_FLAGS} ${LINUX_32BIT_FLAG}"
)

#No lib prefix.
SET_TARGET_PROPERTIES( ${MODULE_NAME} PROPERTIES PREFIX "" )

#Create filters.
create_source_groups( "${CMAKE_SOURCE_DIR}" )

#Clear sources list for next target.
clear_sources()

#Make the bin directory if it doesn't exist.
add_custom_command( TARGET ${MODULE_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${ASMOD_BIN_DIR}
)

if( WIN32 )
	#Copy the sqlite3 dll so the exe can get to it
	add_custom_command( TARGET ${MODULE_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/external/SQLite/lib/sqlite3.dll"
		"${ASMOD_BIN_DIR}"
	)

	#Copy the mariadb dll so the exe can get to it
	add_custom_command( TARGET ${MODULE_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/external/MariaDB/lib/libmariadb.dll"
		"${ASMOD_BIN_DIR}"
	)

	#Copy the mariadbd dll so the exe can get to it
	add_custom_command( TARGET ${MODULE_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/external/MariaDB/lib/libmariadbd.dll"
		"${ASMOD_BIN_DIR}"
	)
else()
	#Copy the sqlite3 library so the exe can get to it
	add_custom_command( TARGET ${MODULE_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/external/SQLite/lib/libsqlite3.so"
		"${ASMOD_BIN_DIR}"
	)

	#Copy the mariadb library so the exe can get to it
	add_custom_command( TARGET ${MODULE_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/external/MariaDB/lib/libmariadb.so.2"
		"${ASMOD_BIN_DIR}"
	)
endif()