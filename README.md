# Building
To build, you should use clang-cl or MSYS2/clang. It isn't adapted for building on any other compilers, so for building you should use **one of** these cmake commands:

`
cmake -DCMAKE_MAKE_PROGRAM="Path/To/ninja.exe" -DCMAKE_C_COMPILER="Path/To/clang.exe" -DCMAKE_CXX_COMPILER="Path/To/clang++.exe" -G Ninja -S "Path/To/Project" -B "Path/Where/To/Build"
`

`
cmake -DCMAKE_MAKE_PROGRAM="Path/To/ninja.exe" -DCMAKE_C_COMPILER="Path/To/clang-cl.exe" -DCMAKE_CXX_COMPILER="Path/To/clang-cl.exe" -G Ninja -S "Path/To/Project" -B "Path/Where/To/Build"
`

Clang compiler for MSYS2 can be installed with its utilite, pacman [(described here)](https://www.mingw-w64.org/getting-started/msys2-llvm/)

Clang-cl can be installed using Visual Studio Installer [(described here)](https://learn.microsoft.com/en-us/cpp/build/clang-support-msbuild)
