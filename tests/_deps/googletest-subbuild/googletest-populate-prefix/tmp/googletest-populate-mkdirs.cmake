# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/lot399/ai-mmorpg-world/rathena-AI-world/tests/_deps/googletest-src"
  "/home/lot399/ai-mmorpg-world/rathena-AI-world/tests/_deps/googletest-build"
  "/ai-mmorpg-world/rathena-AI-world/tests/_deps/googletest-subbuild/googletest-populate-prefix"
  "/ai-mmorpg-world/rathena-AI-world/tests/_deps/googletest-subbuild/googletest-populate-prefix/tmp"
  "/ai-mmorpg-world/rathena-AI-world/tests/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
  "/ai-mmorpg-world/rathena-AI-world/tests/_deps/googletest-subbuild/googletest-populate-prefix/src"
  "/ai-mmorpg-world/rathena-AI-world/tests/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/ai-mmorpg-world/rathena-AI-world/tests/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/ai-mmorpg-world/rathena-AI-world/tests/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
