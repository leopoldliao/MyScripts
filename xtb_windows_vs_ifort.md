# How to compile xtb on windows by using Visual Studio 2019 + Intel Fortran Compiler 19.1

## Requirements
* Visual Studio 2019

It provides Cl compiler, Nmake and cmake tools
* Intel Parallel Studio XE 2020

It provides Intel Fortran Compiler 19.1, including blas and lapack libraries, openmp

## Modified xtb 6.3.2 source
* Modify `xtb\CMakeLists.txt` file

Need add a line below exe link:
```cmake
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:9999999999")
```
* Modify `xtb\cmake\CMakeLists.txt` file

original part:
```cmake
  set(dialect "-axAVX2 -r8 -traceback")
  set(bounds "-check bounds")
```
should change into 
```cmake
  set(dialect "-4R8 -traceback")
  set(bounds "-check:bounds")
```
of course can add SIMD instruction, but here I don't.
* Modify `xtb\TESTSUITE\c_api_example.c` file

We should directly define array：
```c
double q[7];
double wbo[7*7];
char buffer[512];
```
Because Cl compiler can not identify Variable Length Array in C， only work correctly in C++
* Modify path for Windows

My old method, change some path for gfn0 in `\src\main\setup.f90`
```fortran
logical :: alive

if (present(fname)) then
   if(index(fname, 'param_gfn0-xtb.txt') .ne. 0) then
       inquire(file=fname, exist=alive)
       if(alive) then
           filename = fname
       else
           filename = env%xtbpath//'/param_gfn0-xtb.txt'
       end if
   else
       filename = fname
   end if
else
```

Official method see:
https://github.com/grimme-lab/xtb/pull/320/commits/ba7578c9e9542ce3641eefb71ae775cc4ad96a41

* Modify `xtb\src\mctc\mctc_init.f90`

Because about signal processing of `call signal` parts is not work on Windows, I think can be modified as following:
```fortran
!...
!  signal processing
   external :: warning_sigint
   external :: warning_sigterm
   call signal(2,warning_sigint)
   !call signal(15,warning_sigterm)
!...
!...
subroutine warning_sigint
    print *
    print *, 'recieved SIGINT, terminating...'
    print *, 'external termination of xtb'
    call exit(666)
end subroutine warning_sigint

subroutine warning_sigterm
    print *
    print *, 'recieved SIGTERM, terminating...'
    print *, 'external termination of xtb'
    call exit(666)
end subroutine warning_sigterm
!...
```
only `call signal(2,warning_sigint)` is work, not `call signal(15,warning_sigterm)`. Maybe you can change it.

## Compile
Now we can compile it according to standard steps

uncompress xtb source package, open intel compiler 19.1 Intel 64 visual studio 2019 cmd terminal from start menu, then as following:
```cmd
cd xtb-6.3.2
md build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=E:/yourwantinstallpath -G "NMake Makefiles"
nmake
nmake test
nmake intall
```
Here must use `-G "NMake Makefiles"`, because default `-G "Visual Studio 16 2019"` can not configure both C and fortan source

# Release
Pack relevant dll, we can use it on other Windows computer.

# Usage
Must add `bin` full path into Path environment variable of windows, and create new variable named `XTBPATH`, value set as path of `\share\xtb`
For example:
```
D:\xtb6.3.2_win64\share\xtb
```


