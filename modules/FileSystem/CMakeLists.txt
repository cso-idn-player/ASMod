###################################################
#                                                 #
#                                                 #
#   ASMod FileSystem module CMake build file      #
#                                                 #
#                                                 #
###################################################

#Change this to match your module's directory structure.
set( MODULE_LIB_DIR ${MODULE_BASE_DIRECTORY}/FileSystem/dlls )

set( MODULE_NAME FileSystem )

#Set module specific linker flags here.
set( MODULE_LINK_FLAGS )

#Add module sources here, or using add_subdirectory.
add_sources(
	${SHARED_SOURCES}
	${SHARED_MODULE_SOURCES}
	CASFileSystemModule.h
	CASFileSystemModule.cpp
	ConCommands.h
	ConCommands.cpp
	Module.h
	Module.cpp
)

add_subdirectory( ScriptAPI )

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
)

#Define preprocessor symbols here.
target_compile_definitions( ${MODULE_NAME} PRIVATE
	${SHARED_DEFINITIONS}
	${SHARED_MODULE_DEFS}
)

#Add library dependencies here.
target_link_libraries( ${MODULE_NAME}
	${SHARED_LIBRARY_DEPS}
	${SHARED_MODULE_LIBRARIES}
	${UNIX_FS_LIB}
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
