!include LogicLib.nsh

Function un.BorrarKey
Pop $1

ReadINIStr $0 '$INSTDIR\\fileext.ini' ext $1
${If} $0 <> ""
   DeleteRegKey HKCR $1
${EndIf}

FunctionEnd

!macro BorrarKey myExtension
Push ${myExtension}
Call un.BorrarKey
!macroend

Function un.BorrarKeys
Pop $0

!insertmacro BorrarKey '.bmp'
!insertmacro BorrarKey '.bit'
!insertmacro BorrarKey '.bw' 
!insertmacro BorrarKey '.cin'
!insertmacro BorrarKey '.ct' 
!insertmacro BorrarKey '.dpx'
!insertmacro BorrarKey '.exr'
!insertmacro BorrarKey '.gif'
!insertmacro BorrarKey '.hdr'
!insertmacro BorrarKey '.hdri'
!insertmacro BorrarKey '.iff'
!insertmacro BorrarKey '.jpeg'
!insertmacro BorrarKey '.jpg'
!insertmacro BorrarKey '.jfif'
!insertmacro BorrarKey '.nt'
!insertmacro BorrarKey '.mt'
!insertmacro BorrarKey '.pbm'
!insertmacro BorrarKey '.pgm'
!insertmacro BorrarKey '.pic'
!insertmacro BorrarKey '.png'
!insertmacro BorrarKey '.pnm'
!insertmacro BorrarKey '.ppm'
!insertmacro BorrarKey '.rgb'
!insertmacro BorrarKey '.rgba'
!insertmacro BorrarKey '.rla'
!insertmacro BorrarKey '.rpf'
!insertmacro BorrarKey '.sgi'
!insertmacro BorrarKey '.st' 
!insertmacro BorrarKey '.sxr'
!insertmacro BorrarKey '.tga'
!insertmacro BorrarKey '.tiff'
!insertmacro BorrarKey '.tif'
!insertmacro BorrarKey '.z'

!insertmacro BorrarKey '.avi'
!insertmacro BorrarKey '.divx'
!insertmacro BorrarKey '.dv'
!insertmacro BorrarKey '.flv'
!insertmacro BorrarKey '.m4v'
!insertmacro BorrarKey '.mkv'
!insertmacro BorrarKey '.mov'
!insertmacro BorrarKey '.mpg'
!insertmacro BorrarKey '.mpeg'
!insertmacro BorrarKey '.mp4'
!insertmacro BorrarKey '.mxf'


Delete $INSTDIR\fileext.ini
SetOutPath $TEMP
RMDir $INSTDIR

FunctionEnd

Section "un.DelKeys"
Call un.BorrarKeys
SectionEnd
