# Microsoft Developer Studio Project File - Name="char_sql" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=char_sql - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "char-server_sql.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "char-server_sql.mak" CFG="char_sql - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "char_sql - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "char_sql - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "char_sql - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".."
# PROP Intermediate_Dir "tmp\char_sql\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /O2 /I "..\3rdparty\msinttypes\include" /I "..\3rdparty\mysql\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_WIN32" /D "__WIN32" /D FD_SETSIZE=4096 /D "DB_MANUAL_CAST_TO_UNION" /FD /GF /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib ws2_32.lib libmysql.lib /nologo /subsystem:console /debug /machine:I386 /libpath:"..\3rdparty\mysql\lib"

!ELSEIF  "$(CFG)" == "char_sql - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".."
# PROP Intermediate_Dir "tmp\char_sql\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /Gi /ZI /Od /I "..\3rdparty\msinttypes\include" /I "..\3rdparty\mysql\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_WIN32" /D "__WIN32" /D FD_SETSIZE=4096 /D "DB_MANUAL_CAST_TO_UNION" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x405 /d "_DEBUG"
# ADD RSC /l 0x417 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib ws2_32.lib libmysql.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\3rdparty\mysql\lib"

!ENDIF 

# Begin Target

# Name "char_sql - Win32 Release"
# Name "char_sql - Win32 Debug"
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

SOURCE=..\src\common\sql.c
# End Source File
# Begin Source File

SOURCE=..\src\common\sql.h
# End Source File
# Begin Source File

SOURCE=..\src\common\strlib.c
# End Source File
# Begin Source File

SOURCE=..\src\common\strlib.h
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
# Begin Group "char_sql"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\char_sql\char.c
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\char.h
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_auction.c
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_auction.h
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_guild.c
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_guild.h
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_homun.c
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_homun.h
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_mail.c
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_mail.h
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_mercenary.c
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_mercenary.h
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_party.c
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_party.h
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_pet.c
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_pet.h
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_quest.c
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_quest.h
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_storage.c
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_storage.h
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\inter.c
# End Source File
# Begin Source File

SOURCE=..\src\char_sql\inter.h
# End Source File
# End Group
# End Target
# End Project
