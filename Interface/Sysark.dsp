# Microsoft Developer Studio Project File - Name="Sysark" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Sysark - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Sysark.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Sysark.mak" CFG="Sysark - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Sysark - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Sysark - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Sysark - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libcd.lib"

!ELSEIF  "$(CFG)" == "Sysark - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Sysark - Win32 Release"
# Name "Sysark - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Control"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Control\ReportCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\Control\ReportCtrl.h
# End Source File
# End Group
# Begin Group "Registry"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Registry\DataFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\Registry\DataFormat.h
# End Source File
# Begin Source File

SOURCE=.\Registry\HiveAnysBase.cpp
# End Source File
# Begin Source File

SOURCE=.\Registry\HiveAnysBase.h
# End Source File
# Begin Source File

SOURCE=.\Registry\ntreg.h
# End Source File
# Begin Source File

SOURCE=.\Registry\RegEditEx.cpp
# End Source File
# Begin Source File

SOURCE=.\Registry\RegEditEx.h
# End Source File
# Begin Source File

SOURCE=.\Registry\RegSuport.cpp
# End Source File
# Begin Source File

SOURCE=.\Registry\RegSuport.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\CommonDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DataDef.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgInit.cpp
# End Source File
# Begin Source File

SOURCE=.\FileDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FileDlgInit.cpp
# End Source File
# Begin Source File

SOURCE=.\FirstLittleDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadServer.cpp
# End Source File
# Begin Source File

SOURCE=.\ProcessRMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Style1Dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Sysark.cpp
# End Source File
# Begin Source File

SOURCE=.\Sysark.rc
# End Source File
# Begin Source File

SOURCE=.\SysarkDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Import"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Import\tinystr.h
# End Source File
# Begin Source File

SOURCE=.\Import\tinyxml.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\CommonDlg.h
# End Source File
# Begin Source File

SOURCE=.\DataDef.h
# End Source File
# Begin Source File

SOURCE=.\DlgInit.h
# End Source File
# Begin Source File

SOURCE=.\FileDlg.h
# End Source File
# Begin Source File

SOURCE=.\FileDlgInit.h
# End Source File
# Begin Source File

SOURCE=.\FirstLittleDlg.h
# End Source File
# Begin Source File

SOURCE=.\Ioctrls.h
# End Source File
# Begin Source File

SOURCE=.\LoadServer.h
# End Source File
# Begin Source File

SOURCE=.\ProcessRMenu.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Style1Dlg.h
# End Source File
# Begin Source File

SOURCE=.\Sysark.h
# End Source File
# Begin Source File

SOURCE=.\SysarkDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\Sysark.ico
# End Source File
# Begin Source File

SOURCE=.\res\Sysark.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
