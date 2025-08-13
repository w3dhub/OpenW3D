# Based on https://github.com/cppmf/GitInfo.cmake
# ---------------------------------------------------------------------------- #
#
# MIT License
# 
# Copyright (c) 2020 C++ Modern Framework
# Copyright (c) 2025 OpenW3D Contributors
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
#
# Following variables will be set when calling GitInfo
#
#   GIT_DIR : path to the project .git directory
#   GIT_IS_DIRTY : whether or not the working tree is dirty
#   GIT_REVISION_HASH: current HEAD sha hash
#   GIT_REVISION : shorten version of GIT_REVISION_HASH
#   GIT_REVISION_COUNT : number of commits since last tag
#   GIT_AUTHOR_NAME : author name
#   GIT_AUTHOR_DATE : author date
#   GIT_COMMITTER_NAME : committer name
#   GIT_COMMITTER_DATE : committer date
#   GIT_COMMITTER_TIME_STAMP : committer time stamp
#   GIT_LATEST_TAG : most recent tagname of the current branch
#   GIT_VERSION_MAJOR : major version parsed from tag assumes format v#.#.#
#   GIT_VERSION_MINOR : minor version parsed from tag assumes format v#.#.#
#   GIT_VERSION_PATCH : patch version parsed from tag assumes format v#.#.#
#
# ---------------------------------------------------------------------------- #

# This is the main function to call in project CMakeLists.txt
# source should point to the root project directory
function(GitInfo source)

  # Check is source is a valid path
  if(NOT EXISTS ${source})
    message(FATAL_ERROR "'${source}' is not a valid path")
  endif()

  # Define the possible location of the .git directory
  set(GIT_DIR "${source}/.git")

  # Check if .git folder exist
  if(EXISTS ${GIT_DIR})

    #
    set(GIT_DIR "${GIT_DIR}" CACHE PATH "Project .git directory")

    # Check if git is installed
    if(NOT GIT_FOUND)
      find_package(Git QUIET)
    endif()
    if(NOT GIT_FOUND)
      message(AUTHOR_WARNING "Git not found, cannot get git informations")
      set(GIT_HAVE_INFO "false" CACHE INTERNAL "git info not retrieved")
      set(GIT_IS_DIRTY "false" CACHE INTERNAL "is the current branch is dirty")
      set(GIT_COMMITTER_TIME_STAMP "0" CACHE INTERNAL "git committer time stamp")
      set(GIT_REVISION_COUNT "0" CACHE INTERNAL "revision count since last tag")
      set(GIT_VERSION_MAJOR "0" CACHE INTERNAL "major version from tag")
      set(GIT_VERSION_MINOR "0" CACHE INTERNAL "minor version from tag")
      set(GIT_VERSION_PATCH "0" CACHE INTERNAL "patch version from tag")
      return()
    endif()

    set(GIT_HAVE_INFO "true" CACHE INTERNAL "git info retrieved")
    
    # whether or not the working tree is dirty
    execute_process(COMMAND ${GIT_EXECUTABLE} status --porcelain
            WORKING_DIRECTORY ${source}
            RESULT_VARIABLE exit_code
            OUTPUT_VARIABLE GIT_IS_DIRTY OUTPUT_STRIP_TRAILING_WHITESPACE)
    # the working tree is dirty when the error code is different from 0
    # or if the output is not empty
    if(NOT exit_code EQUAL 0 OR NOT ${GIT_IS_DIRTY} STREQUAL "")
      unset(GIT_IS_DIRTY)
      set(GIT_IS_DIRTY "true" CACHE INTERNAL "is the  current branch is dirty")
    else()
      unset(GIT_IS_DIRTY)
      set(GIT_IS_DIRTY "false" CACHE INTERNAL "is the  current branch is dirty")
    endif()

    # git revision full hash
    execute_process(COMMAND ${GIT_EXECUTABLE} show -s "--format=%H" HEAD
            WORKING_DIRECTORY ${source}
            OUTPUT_VARIABLE GIT_REVISION_HASH OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(GIT_REVISION_HASH "${GIT_REVISION_HASH}" CACHE INTERNAL "git revision full hash")

    # shorten version of git revision
    execute_process(COMMAND ${GIT_EXECUTABLE} show -s "--format=%h" HEAD
            WORKING_DIRECTORY ${source}
            OUTPUT_VARIABLE GIT_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(GIT_REVISION "${GIT_REVISION}" CACHE INTERNAL "shorten version of git revision")

    # author name
    execute_process(COMMAND ${GIT_EXECUTABLE} show -s "--format=%an" HEAD
            WORKING_DIRECTORY ${source}
            OUTPUT_VARIABLE GIT_AUTHOR_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(GIT_AUTHOR_NAME "${GIT_AUTHOR_NAME}" CACHE INTERNAL "git author name")

    # author date
    execute_process(COMMAND ${CMAKE_COMMAND} -E env TZ=UTC ${GIT_EXECUTABLE} show -s "--format=%ad" "--date=format-local:%Y-%m-%d %H:%M:%S" HEAD
            WORKING_DIRECTORY ${source}
            OUTPUT_VARIABLE GIT_AUTHOR_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(GIT_AUTHOR_DATE "${GIT_AUTHOR_DATE}" CACHE INTERNAL "git author date")

    # committer name
    execute_process(COMMAND ${GIT_EXECUTABLE} show -s "--format=%cn" HEAD
            WORKING_DIRECTORY ${source}
            OUTPUT_VARIABLE GIT_COMMITTER_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(GIT_COMMITTER_NAME "${GIT_COMMITTER_NAME}" CACHE INTERNAL "git committer name")

    # committer date
    execute_process(COMMAND ${CMAKE_COMMAND} -E env TZ=UTC ${GIT_EXECUTABLE} show -s "--format=%cd" "--date=format-local:%Y-%m-%d %H:%M:%S" HEAD
            WORKING_DIRECTORY ${source}
            OUTPUT_VARIABLE GIT_COMMITTER_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(GIT_COMMITTER_DATE "${GIT_COMMITTER_DATE}" CACHE INTERNAL "git committer date")

    # committer time stamp since unix epoch
    execute_process(COMMAND ${GIT_EXECUTABLE} show -s "--format=%ct" HEAD
            WORKING_DIRECTORY ${source}
            OUTPUT_VARIABLE GIT_COMMITTER_TIME_STAMP OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(GIT_COMMITTER_TIME_STAMP "${GIT_COMMITTER_TIME_STAMP}" CACHE INTERNAL "git committer time stamp")

    # most recent version tagname of the current branch
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --match=v* --abbrev=0 HEAD
            WORKING_DIRECTORY ${source}
            OUTPUT_VARIABLE GIT_LATEST_TAG OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(GIT_LATEST_TAG "${GIT_LATEST_TAG}" CACHE INTERNAL "git most recent tagname of the current branch")
    
    # revision count since last tag
    if(GIT_LATEST_TAG)
      execute_process(COMMAND ${GIT_EXECUTABLE} rev-list "${GIT_LATEST_TAG}..HEAD" --count
              WORKING_DIRECTORY ${source}
              OUTPUT_VARIABLE GIT_REVISION_COUNT OUTPUT_STRIP_TRAILING_WHITESPACE)
      set(GIT_REVISION_COUNT "${GIT_REVISION_COUNT}" CACHE INTERNAL "revision count since last tag")

      # Parse the version tag to extract version numbers.
      string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" GIT_VERSION_MAJOR "${GIT_LATEST_TAG}")
      string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" GIT_VERSION_MINOR "${GIT_LATEST_TAG}")
      string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" GIT_VERSION_PATCH "${GIT_LATEST_TAG}")
      
      set(GIT_VERSION_MAJOR "${GIT_VERSION_MAJOR}" CACHE INTERNAL "major version from tag")
      set(GIT_VERSION_MINOR "${GIT_VERSION_MINOR}" CACHE INTERNAL "minor version from tag")
      set(GIT_VERSION_PATCH "${GIT_VERSION_PATCH}" CACHE INTERNAL "patch version from tag")
    else()
      execute_process(COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
              WORKING_DIRECTORY ${source}
              OUTPUT_VARIABLE GIT_REVISION_COUNT OUTPUT_STRIP_TRAILING_WHITESPACE)
      set(GIT_REVISION_COUNT "${GIT_REVISION_COUNT}" CACHE INTERNAL "revision count since last tag")
      set(GIT_VERSION_MAJOR "0" CACHE INTERNAL "major version from tag")
      set(GIT_VERSION_MINOR "0" CACHE INTERNAL "minor version from tag")
      set(GIT_VERSION_PATCH "0" CACHE INTERNAL "patch version from tag")
    endif()
  else()
    message(AUTHOR_WARNING "Git directory not found, zip file download?")
    set(GIT_IS_DIRTY "false" CACHE INTERNAL "is the current branch is dirty")
    set(GIT_REVISION_HASH "0000000000000000000000000000000000000000" CACHE INTERNAL "git revision full hash")
    set(GIT_REVISION "deadbee" CACHE INTERNAL "shorten version of git revision")
    set(GIT_REVISION_COUNT "0" CACHE INTERNAL "revision count since last tag")
    set(GIT_AUTHOR_NAME "Not In Git" CACHE INTERNAL "git author name")
    set(GIT_AUTHOR_DATE "1970-01-01 00:00:00" CACHE INTERNAL "git author date")
    set(GIT_COMITTER_NAME "Not In Git" CACHE INTERNAL "git author name")
    set(GIT_COMITTER_DATE "1970-01-01 00:00:00" CACHE INTERNAL "git author date")
    set(GIT_COMMITTER_TIME_STAMP "0" CACHE INTERNAL "git committer time stamp")
    set(GIT_VERSION_MAJOR "0" CACHE INTERNAL "major version from tag")
    set(GIT_VERSION_MINOR "0" CACHE INTERNAL "minor version from tag")
    set(GIT_VERSION_PATCH "0" CACHE INTERNAL "patch version from tag") 
  endif()
endfunction()


# Report git information
function(GitInfoReport)
  message(STATUS "")
  message(STATUS "----------------------------------------------------")
  message(STATUS "                 GitInfo.cmake")
  message(STATUS "")
  message(STATUS "GIT_DIR : ${GIT_DIR}")
  message(STATUS "")
  message(STATUS "GIT_IS_DIRTY : ${GIT_IS_DIRTY}")
  message(STATUS "GIT_REVISION : ${GIT_REVISION}")
  message(STATUS "GIT_REVISION_HASH : ${GIT_REVISION_HASH}")
  message(STATUS "GIT_REVISION_COUNT : ${GIT_REVISION_COUNT}")
  message(STATUS "")
  message(STATUS "GIT_AUTHOR_NAME : ${GIT_AUTHOR_NAME}")
  message(STATUS "GIT_AUTHOR_DATE : ${GIT_AUTHOR_DATE}")
  message(STATUS "")
  message(STATUS "GIT_COMMITTER_NAME : ${GIT_COMMITTER_NAME}")
  message(STATUS "GIT_COMMITTER_DATE : ${GIT_COMMITTER_DATE}")
  message(STATUS "GIT_COMMITTER_TIME_STAMP : ${GIT_COMMITTER_TIME_STAMP}")
  message(STATUS "")
  message(STATUS "GIT_LATEST_TAG : ${GIT_LATEST_TAG}")
  message(STATUS "GIT_VERSION_MAJOR : ${GIT_VERSION_MAJOR}")
  message(STATUS "GIT_VERSION_MINOR : ${GIT_VERSION_MINOR}")
  message(STATUS "GIT_VERSION_PATCH : ${GIT_VERSION_PATCH}")
  message(STATUS "")
  message(STATUS "----------------------------------------------------")
  message(STATUS "")
endfunction()
