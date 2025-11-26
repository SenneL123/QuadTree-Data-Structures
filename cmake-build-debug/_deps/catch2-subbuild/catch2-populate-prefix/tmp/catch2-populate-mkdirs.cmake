# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/senne/OneDrive - Universiteit Antwerpen/Schooljaar 2025-2026/1ste semester/Data Structures/Practicum/Project/cmake-build-debug/_deps/catch2-src"
  "C:/Users/senne/OneDrive - Universiteit Antwerpen/Schooljaar 2025-2026/1ste semester/Data Structures/Practicum/Project/cmake-build-debug/_deps/catch2-build"
  "C:/Users/senne/OneDrive - Universiteit Antwerpen/Schooljaar 2025-2026/1ste semester/Data Structures/Practicum/Project/cmake-build-debug/_deps/catch2-subbuild/catch2-populate-prefix"
  "C:/Users/senne/OneDrive - Universiteit Antwerpen/Schooljaar 2025-2026/1ste semester/Data Structures/Practicum/Project/cmake-build-debug/_deps/catch2-subbuild/catch2-populate-prefix/tmp"
  "C:/Users/senne/OneDrive - Universiteit Antwerpen/Schooljaar 2025-2026/1ste semester/Data Structures/Practicum/Project/cmake-build-debug/_deps/catch2-subbuild/catch2-populate-prefix/src/catch2-populate-stamp"
  "C:/Users/senne/OneDrive - Universiteit Antwerpen/Schooljaar 2025-2026/1ste semester/Data Structures/Practicum/Project/cmake-build-debug/_deps/catch2-subbuild/catch2-populate-prefix/src"
  "C:/Users/senne/OneDrive - Universiteit Antwerpen/Schooljaar 2025-2026/1ste semester/Data Structures/Practicum/Project/cmake-build-debug/_deps/catch2-subbuild/catch2-populate-prefix/src/catch2-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/senne/OneDrive - Universiteit Antwerpen/Schooljaar 2025-2026/1ste semester/Data Structures/Practicum/Project/cmake-build-debug/_deps/catch2-subbuild/catch2-populate-prefix/src/catch2-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/senne/OneDrive - Universiteit Antwerpen/Schooljaar 2025-2026/1ste semester/Data Structures/Practicum/Project/cmake-build-debug/_deps/catch2-subbuild/catch2-populate-prefix/src/catch2-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
