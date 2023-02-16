// *****************************************************************************
// Source code for the MaxBIM21 ARCHICAD Add-On
// API Development Kit; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

// ---------------------------------- Includes ---------------------------------

#include	"APIEnvir.h"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"

#include	"ResourceIDs.h"

#include	"MaxBIM21.h"

#include	"UtilityFunctions.h"

//#include "WallTableformPlacer.h"
//#include "SlabTableformPlacer.h"
//#include "BeamTableformPlacer.h"
//#include "ColumnTableformPlacer.h"
//#include "LowSideTableformPlacer.h"
//
//#include "Layers.h"
//#include "Export.h"
//#include "Quantities.h"
//#include "Facility.h"
//
//#include "Information.h"


// -----------------------------------------------------------------------------
// Handles menu commands
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL MenuCommandHandler(const API_MenuParams *menuParams)
{
	GSErrCode err;
	API_MenuItemRef	itemRef;
	GSFlags			itemFlags;

	switch(menuParams->menuItemRef.menuResID) {
		// 테이블폼 배치
		case ID_MENU_STRINGS_FORMWORKS:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// 벽 테이블폼 배치
					err = ACAPI_CallUndoableCommand("테스트", [&]() -> GSErrCode {
						EasyObjectPlacement euroform;
						euroform.init(L"유로폼v2.0.gsm", 1, 0, 0, 0, 0, 0);
						euroform.placeObject(4,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"eu_wid", APIParT_CString, "500",
							"eu_hei", APIParT_CString, "900",
							"u_ins", APIParT_CString, "벽세우기");
						return NoError;
					});
					break;
				case 2:		// 슬래브 테이블폼 배치
					break;
				case 3:		// 보 테이블폼 배치
					break;
				case 4:		// 기둥 테이블폼 배치
					break;
				case 5:		// 기초 테이블폼 배치
					break;
			}
			break;

		// 레이어 도구
		case ID_MENU_STRINGS_LAYER_TOOLS:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// 레이어 쉽게 선택하기
					break;
				case 2:		// 레이어 쉽게 만들기
					break;
				case 3:		// 레이어 쉽게 지정하기
					break;
				case 4:		// 레이어 이름 검사하기
					break;
			}
			break;

		// 내보내기
		case ID_MENU_STRINGS_EXPORTING:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// 부재 정보 내보내기 (현재 선택한 것만)
					break;
				case 2:		// 부재 정보 내보내기 (보이는 레이어 별로)
					break;
				case 3:		// 부재별 선택 후 보여주기
					break;
				case 4:		// 보 테이블폼 물량표 작성
					break;
				case 5:		// 테이블폼 면적 계산
					break;
				case 6:		// 모든 입면도 PDF로 내보내기 (현재 뷰만)
					break;
				case 7:		// 모든 입면도 PDF로 내보내기 (보이는 레이어 별로)
					break;
			}
			break;

		// 반자동 배치
		case ID_MENU_STRINGS_SEMIAUTO_ARRANGE:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// 물량합판 부착하기
					break;
				case 2:		// 단열재 부착하기
					break;
			}
			break;

		// 편의 기능
		case ID_MENU_STRINGS_FACILITIES:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// 3D 품질/속도 조정하기
					break;
				case 2:		// 영역에 3D 라벨 붙이기
					break;
				case 3:		// 현재 평면도의 테이블폼에 버블 자동 배치
					break;
				case 4:		// 카메라 위치 저장하기/불러오기
					break;
			}
			break;
	}

	return NoError;
}		// MenuCommandHandler


// =============================================================================
//
// Required functions
//
// =============================================================================


// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------

API_AddonType	__ACENV_CALL	CheckEnvironment(API_EnvirParams* envir)
{
	RSGetIndString(&envir->addOnInfo.name, ID_ADDON_INFO, 1, ACAPI_GetOwnResModule());
	RSGetIndString(&envir->addOnInfo.description, ID_ADDON_INFO, 2, ACAPI_GetOwnResModule());

	return APIAddon_Normal;
}		// CheckEnvironment


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	RegisterInterface(void)
{
	GSErrCode err;
	
	err = ACAPI_Register_Menu(ID_MENU_STRINGS_FORMWORKS,			ID_MENU_PROMPT_STRINGS_FORMWORKS,			MenuCode_UserDef, MenuFlag_Default);	// 테이블폼 배치
	err = ACAPI_Register_Menu(ID_MENU_STRINGS_LAYER_TOOLS,			ID_MENU_PROMPT_STRINGS_LAYER_TOOLS,			MenuCode_UserDef, MenuFlag_Default);	// 레이어 도구
	err = ACAPI_Register_Menu(ID_MENU_STRINGS_EXPORTING,			ID_MENU_PROMPT_STRINGS_EXPORTING,			MenuCode_UserDef, MenuFlag_Default);	// 내보내기
	err = ACAPI_Register_Menu(ID_MENU_STRINGS_SEMIAUTO_ARRANGE,		ID_MENU_PROMPT_STRINGS_SEMIAUTO_ARRANGE,	MenuCode_UserDef, MenuFlag_Default);	// 반자동 배치
	err = ACAPI_Register_Menu(ID_MENU_STRINGS_FACILITIES,			ID_MENU_PROMPT_STRINGS_FACILITIES,			MenuCode_UserDef, MenuFlag_Default);	// 편의 기능

	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL Initialize(void)
{
	GSErrCode err;

	err = ACAPI_Install_MenuHandler(ID_MENU_STRINGS_FORMWORKS,			MenuCommandHandler);	// 테이블폼 배치
	err = ACAPI_Install_MenuHandler(ID_MENU_STRINGS_LAYER_TOOLS,		MenuCommandHandler);	// 레이어 도구
	err = ACAPI_Install_MenuHandler(ID_MENU_STRINGS_EXPORTING,			MenuCommandHandler);	// 내보내기
	err = ACAPI_Install_MenuHandler(ID_MENU_STRINGS_SEMIAUTO_ARRANGE,	MenuCommandHandler);	// 반자동 배치
	err = ACAPI_Install_MenuHandler(ID_MENU_STRINGS_FACILITIES,			MenuCommandHandler);	// 편의 기능

	return err;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData(void)
{
	return NoError;
}		// FreeData
