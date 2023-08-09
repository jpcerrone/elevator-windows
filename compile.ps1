<#
: - VS compiler switches:
:   - `-WX`, `-W4`: enable warning level 4 and treat warnings as errors
:   - `-wd`: turn off some warnings
:   - `-Oi`: generates intrinsic functions.
:   - `-Od`: disable optimization
:   - `-GR-`: disable run-time type information, we don't need this
:   - `-EHa-`: disable exception-handling
:   - `-nologo`: don't print compiler info
#>

<# - Debug flags
	-DSHOWFPS
	-DSHOWBUTTONSTATES
	-DSHOWELEVATORSTATS
	-DSHOWGUYSSTATS
	-DDEBUG -- assertions
	-DDONTSPAWN
#>

$COMMON_FLAGS = "-DSHOWELEVATORSTATS -DDEBUG /Febin\ /Fdbin\ /Fobin\ /nologo -W4 -Oi -Od -GR -EHa -wd4100 -wd4201"
$COMMON_LIBS = "User32.lib Gdi32.lib Winmm.lib"

$compileGame = "cl -Zi .\windows_main.cpp $COMMON_FLAGS /link $COMMON_LIBS /INCREMENTAL:NO"
Invoke-Expression $compileGame