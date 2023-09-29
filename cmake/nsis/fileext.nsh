!include nsDialogs.nsh
!include WinMessages.nsh
!ifndef LVM_GETITEMTEXT
!define /math LVM_GETITEMTEXTA ${LVM_FIRST} + 45
!define /math LVM_GETITEMTEXTW ${LVM_FIRST} + 115
${_NSIS_DEFAW} LVM_GETITEMTEXT
!endif
Var hListCtl
Var bCheckAll
Var bUnCheckAll

!include mrv2_translations.nsh

Page Custom LVPageCreate LVPageLeave




Function AddCheckedListViewItemWith1SubItem
System::Store S
Pop $4
Pop $3
Pop $2
Pop $1
System::Call '*(i ${LVIF_TEXT},i 0x7fffffff,i 0,i,&i${NSIS_PTR_SIZE},tr2,i,i,p)p.r9'
SendMessage $1 ${LVM_INSERTITEM} "" $9 $0
System::Call '*$9(i${LVIF_STATE},i,i,i0x2000,&i${NSIS_PTR_SIZE} ${LVIS_STATEIMAGEMASK},p,i,i,p)'
IntCmpU $4 0 +2
SendMessage $1 ${LVM_SETITEMSTATE} $0 $9 $8
System::Call '*$9(i,i 0x7fffffff,i 1,i,i,tr3,i,i,p)'
SendMessage $1 ${LVM_SETITEMTEXT} $0 $9
System::Free $9
System::Store L
FunctionEnd

!macro LVUnCheckAll hLV tempvar
System::Call '*(i ${LVIF_STATE},i,i 0,i0x1000,&i${NSIS_PTR_SIZE} ${LVIS_STATEIMAGEMASK},p0,i0,i,p)p.s'
Pop ${tempvar}
SendMessage ${hLV} ${LVM_SETITEMSTATE} -1 ${tempvar}
System::Free ${tempvar}
!macroend

Function UnCheckAll
  !insertmacro LVUnCheckAll $hListCtl $0
FunctionEnd

!macro LVCheckAll hLV tempvar
System::Call '*(i ${LVIF_STATE},i,i 0,i0x2000,&i${NSIS_PTR_SIZE} ${LVIS_STATEIMAGEMASK},p0,i0,i,p)p.s'
Pop ${tempvar}
SendMessage ${hLV} ${LVM_SETITEMSTATE} -1 ${tempvar}
System::Free ${tempvar}
!macroend

Function CheckAll
  !insertmacro LVCheckAll $hListCtl $0
FunctionEnd

!macro AddCheckedListViewItemWith1SubItem hLV txt sub1 checked
Push ${hLV}
Push "${txt}"
Push "${sub1}"
Push "${checked}"
Call AddCheckedListViewItemWith1SubItem
!macroend

Function LVPageCreate
; MessageBox MB_YESNO "Do you want to set file associations?" IDYES yes
;      Abort
; yes:
MessageBox MB_YESNO $(kFileAssocQuestion) IDYES yes
     Abort
yes:

nsDialogs::Create 1018
Pop $0

${NSD_CreateLabel} 0 0 100% 12u $(kFileAssoc)

nsDialogs::CreateControl "SysListView32" ${DEFAULT_STYLES}|${WS_TABSTOP}|${WS_VSCROLL}|${LVS_REPORT} ${WS_EX_WINDOWEDGE}|${WS_EX_CLIENTEDGE} 0 30% 100% 70% ""
Pop $hListCtl
IntOp $0 ${LVS_EX_FULLROWSELECT} | ${LVS_EX_CHECKBOXES}
SendMessage $hListCtl ${LVM_SETEXTENDEDLISTVIEWSTYLE} 0 $0
System::Call '*(i${LVCF_TEXT}|${LVCF_SUBITEM},i,i,t "Extension",i,i 0)p.r9'
SendMessage $hListCtl ${LVM_INSERTCOLUMN} 0x7fffffff $9
System::Call '*$9(i,i,i,t "Description",i,i 1)'
SendMessage $hListCtl ${LVM_INSERTCOLUMN} 0x7fffffff $9
System::Free $9

!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".3fr" "Hasselblad RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".arw" "Sony RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".bay" "Phase One RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".bmp" "Bitmap Image File" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".bmq" "RAW Image File" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".cap" "RAW Image File" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".cin" "Kodak Cineon" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".cine" "Vision Research's Phantom RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".cr2" "Canon RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".cr3" "Canon RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".crw" "Canon RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".cs1" "CaptureShop 1-shot RAW Image" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".dc2" "Kodak RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".dcr" "Kodak RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".dng" "Digital Negative Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".dpx" "Digital Picture Exchange" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".drf" "RAW Image Filr" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".dsc" "Digital Still RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".erf" "Epson RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".exr" "ILM OpenEXR" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".fff" "Hasselblad RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".ia" "RAW Image File" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".iiq" "Phase One RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".jpeg" "JPEG File Interchange Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".jpg" "JPEG File Interchange Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".jfif" "JPEG File Interchange Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".kdc" "Kodak RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mdc" "Minolta RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mef" "Mamiya Electronic Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mos" "Leaf RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mrw" "Minolta RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".nef" "Nikon Electronic Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".nrw" "Nikon RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".orf" "Olympus RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".otio" "OpenTimelineIO Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".pbm" "Portable Bitmap" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".pgm" "Portable Graymap" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".pef" "Pentax Electronic File (Ricoh)" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".png" "Portable Network Graphics" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".pnm" "Portable Any Map" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".ppm" "Portable Pixmap" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".sxr" "ILM Stereo OpenEXR Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".ptx" "Pentax RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".pxn" "Logitech Fotoman Pixtura Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".qtk" "Apple QuickTake Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".raf" "Fujifilm RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".raw" "RAW Image File" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".rdc" "Red Digital Clip" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".rgb" "Silicon Graphics RGB Picture" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".rgba" "Silicon Graphics RGBA Picture" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".rw2" "Panasonic RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".rwl" "Leica RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".rwz" "RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".sgi" "Silicon Graphics Image" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".sr2" "Sony RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".srf" "Sony RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".srw" "Samsung RAW Camera Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".sti" "RAW Image File" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".sxr" "Stereo OpenEXR" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".tga" "Truevision Graphics Adapter" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".tiff" "Tagged Image File Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".tif" "Tagged Image File Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".x3f" "Sigma (Foveon) RAW Camera Format" 1


!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".avi" "Audio Video Interleave" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".divx" "DIVX Media Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".dv" "Digital Video Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".flv" "Flash Video Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".m4v" "Apple M4V Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mkv" "Matroska Video Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mov" "Apple's Quicktime Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mp4" "MPEG4 Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mpg" "MPEG Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mpeg" "MPEG Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mpeg2" "MPEG2 Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mpeg3" "MPEG3 Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mpeg4" "MPEG4 Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mxf" "Material eXchange Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".qt" "Apple's Quicktime" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".rm" "RealNetwork's Real Media" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".usd" "Universal Scene Description" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".usda" "Universal Scene Description ASCII" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".usdc" "Universal Scene Description Binary" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".usdz" "Universal Scene Description ZIP" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".vob" "DVD Video Object Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".vp9" "WebM Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".webm" "WebM Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".wmv" "Windows Media Video" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".y4m" "YUV4MPEG2 Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mp3" "MPEG1/2 Audio Layer III" 0
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".ogg" "Ogg Audio Format" 0
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".vorbis" "Ogg Vorbis Audio Format" 0
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".wav" "Waveform Audio Format" 0

SendMessage $hListCtl ${LVM_SETCOLUMNWIDTH} 0 -1
SendMessage $hListCtl ${LVM_SETCOLUMNWIDTH} 1 -1

${NSD_CreateButton} 0 15u 30% 10% $(kCheckAll)
Pop $bCheckAll
${NSD_OnClick} $bCheckAll checkAll

${NSD_CreateButton} 70% 15u 30% 10% $(kUncheckAll)
Pop $bUnCheckAll
${NSD_OnClick} $bUnCheckAll unCheckAll

System::Call 'USER32::PostMessage(p $hwndparent, i ${WM_NEXTDLGCTL}, p $hListCtl, i1)'
nsDialogs::Show
FunctionEnd

Function LVPageLeave
System::Call '*(&t${NSIS_MAX_STRLEN},i)p.r8'
System::Call '*(i ${LVIF_TEXT},i,i 0,i,&i${NSIS_PTR_SIZE},pr8,i${NSIS_MAX_STRLEN},i,p)p.r9'
SendMessage $hListCtl ${LVM_GETITEMCOUNT} "" "" $1
StrCpy $0 0
${DoWhile} $0 < $1
    SendMessage $hListCtl ${LVM_GETITEMSTATE} $0 ${LVIS_STATEIMAGEMASK} $2
    IntOp $2 $2 & 0x2000
    ${If} $2 <> 0
	SendMessage $hListCtl ${LVM_GETITEMTEXT} $0 $9 $2
	System::Call '*$8(&t${NSIS_MAX_STRLEN}.r7)'
	WriteRegStr HKCR '$7' '' 'mrv2'
	WriteINIStr '$INSTDIR\\fileext.ini' ext '$7' 1
    ${EndIf}
    IntOp $0 $0 + 1
${Loop}
System::Free $8
System::Free $9
FunctionEnd
