# Set source files
set(WWTRANSLATEDB_SRC
    stringtwiddler.cpp
    tdbcategory.cpp
    translatedb.cpp
    translateobj.cpp
    stringtwiddler.h
    tdbcategories.h
    tdbcategory.h
    translatedb.h
    translatedbids.h
    translateobj.h
)

# Targets to build.
add_library(wwtranslatedb STATIC)

target_link_libraries(wwtranslatedb PRIVATE
    wwcommon
)

target_sources(wwtranslatedb PRIVATE ${WWTRANSLATEDB_SRC})
