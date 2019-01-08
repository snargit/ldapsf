cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

include (GenerateExportHeader)

set(name ldapsf)
project(${name})

find_package(Boost 1.47 REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc i18n)

include_directories(${Boost_INCLUDE_DIRS}
                    ${ICU_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_SOURCE_DIR}
					${PROJECT_BINARY_DIR})

add_library(${name} SHARED ldap_sf_ast.cpp 
                           ldap_sf_grammar.cpp 
                           ldap_sf_eval.cpp 
                           ldap_sf.cpp)

GENERATE_EXPORT_HEADER(${name}
             BASE_NAME ${name}
             EXPORT_MACRO_NAME ${name}_EXPORT
             EXPORT_FILE_NAME ${name}_export.h
             STATIC_DEFINE ${name}_BUILT_AS_STATIC
)
ADD_COMPILER_EXPORT_FLAGS()

target_link_libraries(${name} ${ICU_LIBRARIES})

install(TARGETS ${name} DESTINATION lib)
install(FILES ldap_sf_ast.h
              ldap_sf_grammar.h
              ldap_sf_eval.h
              ldap_records.h
              ldap_sf.h
              ldap_sf_ptr.h
              ${PROJECT_BINARY_DIR}/${name}_export.h
			  DESTINATION include/ldapsf)

set(CPACK_PACKAGE_NAME "libldapsf")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LDAP Search Filter library")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
include(CPack)

