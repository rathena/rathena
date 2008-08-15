# Microsoft Developer Studio Project File - Name="map_txt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=map_txt - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "map-server_txt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "map-server_txt.mak" CFG="map_txt - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "map_txt - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W2 /GX /O2 /I "..\3rdparty\zlib\include" /I "..\3rdparty\pcre\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_WIN32" /D "TXT_ONLY" /D "__WIN32" /D "PCRE_SUPPORT" /D FD_SETSIZE=4096 /D "DB_MANUAL_CAST_TO_UNION" /Fp"tmp/map_txt/map-server_txt.pch" /YX /Fo"tmp/map_txt/" /Fd"tmp/map_txt/" /FD /c
# ADD BASE RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 WSOCK32.lib zdll.lib pcre.lib ws2_32.lib /nologo /subsystem:console /pdb:"tmp/map_txt/map-server_txt.pdb" /machine:I386 /out:"../map-server_txt.exe" /libpath:"..\3rdparty\zlib\lib" /libpath:"..\3rdparty\pcre\lib"
# SUBTRACT LINK32 /pdb:none
# Begin Target

# Name "map_txt - Win32 Release"
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\common\cbasetypes.h
# End Source File
# Begin Source File

SOURCE=..\src\common\core.c
# End Source File
# Begin Source File

SOURCE=..\src\common\core.h
# End Source File
# Begin Source File

SOURCE=..\src\common\db.c
# End Source File
# Begin Source File

SOURCE=..\src\common\db.h
# End Source File
# Begin Source File

SOURCE=..\src\common\ers.c
# End Source File
# Begin Source File

SOURCE=..\src\common\ers.h
# End Source File
# Begin Source File

SOURCE=..\src\common\grfio.c
# End Source File
# Begin Source File

SOURCE=..\src\common\grfio.h
# End Source File
# Begin Source File

SOURCE=..\src\common\lock.c
# End Source File
# Begin Source File

SOURCE=..\src\common\lock.h
# End Source File
# Begin Source File

SOURCE=..\src\common\malloc.c
# End Source File
# Begin Source File

SOURCE=..\src\common\malloc.h
# End Source File
# Begin Source File

SOURCE=..\src\common\mapindex.c
# End Source File
# Begin Source File

SOURCE=..\src\common\mapindex.h
# End Source File
# Begin Source File

SOURCE=..\src\common\mmo.h
# End Source File
# Begin Source File

SOURCE=..\src\common\nullpo.c
# End Source File
# Begin Source File

SOURCE=..\src\common\nullpo.h
# End Source File
# Begin Source File

SOURCE=..\src\common\plugin.h
# End Source File
# Begin Source File

SOURCE=..\src\common\plugins.c
# End Source File
# Begin Source File

SOURCE=..\src\common\plugins.h
# End Source File
# Begin Source File

SOURCE=..\src\common\showmsg.c
# End Source File
# Begin Source File

SOURCE=..\src\common\showmsg.h
# End Source File
# Begin Source File

SOURCE=..\src\common\socket.c
# End Source File
# Begin Source File

SOURCE=..\src\common\socket.h
# End Source File
# Begin Source File

SOURCE=..\src\common\strlib.c
# End Source File
# Begin Source File

SOURCE=..\src\common\strlib.h
# End Source File
# Begin Source File

SOURCE=..\src\common\svnversion.h
# End Source File
# Begin Source File

SOURCE=..\src\common\timer.c
# End Source File
# Begin Source File

SOURCE=..\src\common\timer.h
# End Source File
# Begin Source File

SOURCE=..\src\common\utils.c
# End Source File
# Begin Source File

SOURCE=..\src\common\utils.h
# End Source File
# Begin Source File

SOURCE=..\src\common\version.h
# End Source File
# End Group
# Begin Group "map"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\map\atcommand.c
# End Source File
# Begin Source File

SOURCE=..\src\map\battle.c
# End Source File
# Begin Source File

SOURCE=..\src\map\charcommand.c
# End Source File
# Begin Source File

SOURCE=..\src\map\chat.c
# End Source File
# Begin Source File

SOURCE=..\src\map\chrif.c
# End Source File
# Begin Source File

SOURCE=..\src\map\clif.c
# End Source File
# Begin Source File

SOURCE=..\src\map\date.c
# End Source File
# Begin Source File

SOURCE=..\src\map\guild.c
# End Source File
# Begin Source File

SOURCE=..\src\map\intif.c
# End Source File
# Begin Source File

SOURCE=..\src\map\itemdb.c
# End Source File
# Begin Source File

SOURCE=..\src\map\log.c
# End Source File
# Begin Source File

SOURCE=..\src\map\mail.c
# End Source File
# Begin Source File

SOURCE=..\src\map\map.c
# End Source File
# Begin Source File

SOURCE=..\src\map\mercenary.c
# End Source File
# Begin Source File

SOURCE=..\src\map\mob.c
# End Source File
# Begin Source File

SOURCE=..\src\map\npc.c
# End Source File
# Begin Source File

SOURCE=..\src\map\npc_chat.c
# End Source File
# Begin Source File

SOURCE=..\src\map\party.c
# End Source File
# Begin Source File

SOURCE=..\src\map\path.c
# End Source File
# Begin Source File

SOURCE=..\src\map\pc.c
# End Source File
# Begin Source File

SOURCE=..\src\map\pet.c
# End Source File
# Begin Source File

SOURCE=..\src\map\quest.c
# End Source File
# Begin Source File

SOURCE=..\src\map\script.c
# End Source File
# Begin Source File

SOURCE=..\src\map\skill.c
# End Source File
# Begin Source File

SOURCE=..\src\map\status.c
# End Source File
# Begin Source File

SOURCE=..\src\map\storage.c
# End Source File
# Begin Source File

SOURCE=..\src\map\trade.c
# End Source File
# Begin Source File

SOURCE=..\src\map\unit.c
# End Source File
# Begin Source File

SOURCE=..\src\map\vending.c
# End Source File
# Begin Source File

SOURCE=..\src\map\atcommand.h
# End Source File
# Begin Source File

SOURCE=..\src\map\battle.h
# End Source File
# Begin Source File

SOURCE=..\src\map\charcommand.h
# End Source File
# Begin Source File

SOURCE=..\src\map\chat.h
# End Source File
# Begin Source File

SOURCE=..\src\map\chrif.h
# End Source File
# Begin Source File

SOURCE=..\src\map\clif.h
# End Source File
# Begin Source File

SOURCE=..\src\map\date.h
# End Source File
# Begin Source File

SOURCE=..\src\map\guild.h
# End Source File
# Begin Source File

SOURCE=..\src\map\intif.h
# End Source File
# Begin Source File

SOURCE=..\src\map\itemdb.h
# End Source File
# Begin Source File

SOURCE=..\src\map\log.h
# End Source File
# Begin Source File

SOURCE=..\src\map\mail.h
# End Source File
# Begin Source File

SOURCE=..\src\map\map.h
# End Source File
# Begin Source File

SOURCE=..\src\map\mapreg.h
# End Source File
# Begin Source File

SOURCE=..\src\map\mapreg_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\map\mercenary.h
# End Source File
# Begin Source File

SOURCE=..\src\map\mob.h
# End Source File
# Begin Source File

SOURCE=..\src\map\npc.h
# End Source File
# Begin Source File

SOURCE=..\src\map\party.h
# End Source File
# Begin Source File

SOURCE=..\src\map\path.h
# End Source File
# Begin Source File

SOURCE=..\src\map\pc.h
# End Source File
# Begin Source File

SOURCE=..\src\map\pet.h
# End Source File
# Begin Source File

SOURCE=..\src\map\quest.h
# End Source File
# Begin Source File

SOURCE=..\src\map\script.h
# End Source File
# Begin Source File

SOURCE=..\src\map\skill.h
# End Source File
# Begin Source File

SOURCE=..\src\map\status.h
# End Source File
# Begin Source File

SOURCE=..\src\map\storage.h
# End Source File
# Begin Source File

SOURCE=..\src\map\trade.h
# End Source File
# Begin Source File

SOURCE=..\src\map\unit.h
# End Source File
# Begin Source File

SOURCE=..\src\map\vending.h
# End Source File
# End Group
# End Target
# End Project
