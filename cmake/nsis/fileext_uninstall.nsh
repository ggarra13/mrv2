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

!insertmacro BorrarKey '.3fr'
!insertmacro BorrarKey '.arw'
!insertmacro BorrarKey '.bay'
!insertmacro BorrarKey '.bmp'
!insertmacro BorrarKey '.bmq'
!insertmacro BorrarKey '.cap'
!insertmacro BorrarKey '.cin'
!insertmacro BorrarKey '.cine'
!insertmacro BorrarKey '.cr2'
!insertmacro BorrarKey '.cr3'
!insertmacro BorrarKey '.crw'
!insertmacro BorrarKey '.cs1'
!insertmacro BorrarKey '.dc2'
!insertmacro BorrarKey '.dcr'
!insertmacro BorrarKey '.dng'
!insertmacro BorrarKey '.dpx'
!insertmacro BorrarKey '.drf'
!insertmacro BorrarKey '.dsc'
!insertmacro BorrarKey '.erf'
!insertmacro BorrarKey '.exr'
!insertmacro BorrarKey '.fff'
!insertmacro BorrarKey '.ia'
!insertmacro BorrarKey '.iiq'
!insertmacro BorrarKey '.jpeg'
!insertmacro BorrarKey '.jpg'
!insertmacro BorrarKey '.jfif'
!insertmacro BorrarKey '.kdc'
!insertmacro BorrarKey '.mdc'
!insertmacro BorrarKey '.mef'
!insertmacro BorrarKey '.mos'
!insertmacro BorrarKey '.mrw'
!insertmacro BorrarKey '.nef'
!insertmacro BorrarKey '.nrw'
!insertmacro BorrarKey '.orf'
!insertmacro BorrarKey '.pef'
!insertmacro BorrarKey '.png'
!insertmacro BorrarKey '.ppm'
!insertmacro BorrarKey '.psd'
!insertmacro BorrarKey '.tiff'
!insertmacro BorrarKey '.tif'
!insertmacro BorrarKey '.x3f'

!insertmacro BorrarKey '.avi'
!insertmacro BorrarKey '.divx'
!insertmacro BorrarKey '.dv'
!insertmacro BorrarKey '.flv'
!insertmacro BorrarKey '.gif'
!insertmacro BorrarKey '.otio'
!insertmacro BorrarKey '.otioz'
!insertmacro BorrarKey '.m4v'
!insertmacro BorrarKey '.mk3d'
!insertmacro BorrarKey '.mka'
!insertmacro BorrarKey '.mkv'
!insertmacro BorrarKey '.mov'
!insertmacro BorrarKey '.mpg'
!insertmacro BorrarKey '.mpeg'
!insertmacro BorrarKey '.mpeg2'
!insertmacro BorrarKey '.mpeg3'
!insertmacro BorrarKey '.mpeg4'
!insertmacro BorrarKey '.mp4'
!insertmacro BorrarKey '.mxf'
!insertmacro BorrarKey '.qt'
!insertmacro BorrarKey '.rm'
!insertmacro BorrarKey '.ts'
!insertmacro BorrarKey '.vob'
!insertmacro BorrarKey '.vp9'
!insertmacro BorrarKey '.webm'
!insertmacro BorrarKey '.wmv'

!insertmacro BorrarKey '.usd'
!insertmacro BorrarKey '.usda'
!insertmacro BorrarKey '.usdc'
!insertmacro BorrarKey '.usdz'



Delete $INSTDIR\fileext.ini
SetOutPath $TEMP
RMDir $INSTDIR

FunctionEnd

Section "un.DelKeys"
Call un.BorrarKeys
SectionEnd
