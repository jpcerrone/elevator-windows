# GCC
# g++ .\win_main.cpp -lgdi32 -lole32 -lavrt -l:libksguid.a -o win_main.exe -DDEV_BUILD -g -O0
# .\win_main.exe

<#
: - VS compiler switches:
:   - `-WX`, `-W4`: enable warning level 4 and treat warnings as errors
:   - `-wd`: turn off some warnings
:   - `-MT`: static link C runtime library
:   - `-Oi`: generates intrinsic functions.
:   - `-Od`: disable optimization
:   - `-GR-`: disable run-time type information, we don't need this
:   - `-EHa-`: disable exception-handling
:   - `-nologo`: don't print compiler info
:   - `-FC`: full Path of Source Code File in Diagnostics
#>

$COMMON_FLAGS = "/Febin\ /Fdbin\ /Fobin\ /nologo -W4 -Oi -Od -GR -EHa -wd4100 -wd4201"
$COMMON_LIBS = "User32.lib Gdi32.lib"

$compileGame = "cl -Zi .\windows_main.cpp $COMMON_FLAGS /LD /link $COMMON_LIBS /INCREMENTAL:NO"
Invoke-Expression $compileGame