###################################################
#                                                 #
#                                                 #
#   AngelScript Mod main loader CMake build file  #
#                                                 #
#                                                 #
###################################################

#Change this to match your plugin's directory structure.
set( PLUGIN_LIB_DIR ${META_BASE_DIRECTORY}/ASMod/dlls )

set( PLUGIN_NAME ASMod )

#Set plugin specific linker flags here.
set( PLUGIN_LINK_FLAGS )

#Add plugin sources here, or using add_subdirectory.
add_sources(
	${SHARED_SOURCES}
	${SHARED_PLUGIN_SOURCES}
	dllapi.cpp
	dllapi_post.cpp
	ASMod.h
	ASMod.rc
	CASMod.h
	CASMod.cpp
	CASMod.modules.cpp
	CASModLogger.h
	CASModModuleInfo.h
	CASModModuleInfo.cpp
	CASModModuleLogger.h
	CASPluginBuilder.h
	CASPluginBuilder.cpp
	CASPluginManager.h
	CASPluginManager.cpp
	CMetaSteamworksListener.h
	CMetaSteamworksListener.cpp
	CreateInterface_api.cpp
	engine_api.cpp
	h_export.cpp
	info_name.h
	KeyvaluesLogging.h
	KeyvaluesLogging.cpp
	meta_api.cpp
	sdk_util.cpp
	SvenCoopSupport.h
	SvenCoopSupport.cpp
)

#Add public plugin headers here, if any.
#add_includes(
#
#)

#Process source files for inclusion.
preprocess_sources()
	
#Metamod-P's Makefile forces meta_api to recompile if info_name has changed.
set_property(SOURCE ${CURRENT_CMAKE_SOURCE_DIR}/meta_api.cpp APPEND PROPERTY OBJECT_DEPENDS ${CURRENT_CMAKE_SOURCE_DIR}/info_name.h )

add_library( ${PLUGIN_NAME} SHARED ${PREP_SRCS} )

#Add include paths here.
target_include_directories( ${PLUGIN_NAME} PRIVATE
	.
	${SHARED_INCLUDE_PATHS}
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/shared
	${CMAKE_SOURCE_DIR}/external/Angelscript/include
)

#Define preprocessor symbols here.
target_compile_definitions( ${PLUGIN_NAME} PRIVATE
	${SHARED_DEFINITIONS}
	${SHARED_PLUGIN_DEFS}
	ASMOD_PLUGIN
	ASMOD_BASE_DIR="${META_BASE_DIR_RELATIVE}/${PLUGIN_NAME}"
)

#Add library dependencies here.
find_library( AngelscriptLib Angelscript PATHS ${CMAKE_SOURCE_DIR}/external/ANGELSCRIPT/lib )
find_library( AngelscriptUtilsLib AngelscriptUtils PATHS ${CMAKE_SOURCE_DIR}/external/ANGELSCRIPT/lib )
find_library( AngelscriptLibD Angelscriptd PATHS ${CMAKE_SOURCE_DIR}/external/ANGELSCRIPT/lib )
find_library( AngelscriptUtilsLibD AngelscriptUtilsd PATHS ${CMAKE_SOURCE_DIR}/external/ANGELSCRIPT/lib )

target_link_libraries( ${PLUGIN_NAME}
	${SHARED_LIBRARY_DEPS}
	${STEAM_API_LIB}
	optimized ${AngelscriptLib}
	optimized ${AngelscriptUtilsLib}
	debug ${AngelscriptLibD}
	debug ${AngelscriptUtilsLibD}
	${UNIX_FS_LIB}
)

#If the user wants automatic deployment to a game directory, set the output directory paths.
if( DEPLOY_TO_GAME )
	#CMake places libraries in /Debug or /Release on Windows, so explicitly set the paths for both.
	#On Linux, it uses LIBRARY_OUTPUT_DIRECTORY
	set_target_properties( ${PLUGIN_NAME} PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_LIB_DIR}
		RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PLUGIN_LIB_DIR}
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PLUGIN_LIB_DIR}
	)
endif()

#Set 32 bit flag, any plugin specific flags.
set_target_properties( ${PLUGIN_NAME} 
	PROPERTIES COMPILE_FLAGS "${LINUX_32BIT_FLAG}" 
	LINK_FLAGS "${SHARED_LINKER_FLAGS} ${PLUGIN_LINK_FLAGS} ${LINUX_32BIT_FLAG}"
)

#No lib prefix.
SET_TARGET_PROPERTIES( ${PLUGIN_NAME} PROPERTIES PREFIX "" )

#Create filters.
create_source_groups( "${CMAKE_SOURCE_DIR}" )

#Clear sources list for next target.
clear_sources()
