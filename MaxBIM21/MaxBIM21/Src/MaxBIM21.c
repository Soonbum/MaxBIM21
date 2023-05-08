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

#include	"WallTableformPlacer.h"
#include	"SlabTableformPlacer.h"
#include	"BeamTableformPlacer.h"
#include	"ColumnTableformPlacer.h"
#include	"LowSideTableformPlacer.h"
#include	"SupportingPostForBeam.h"

#include	"Layers.h"
#include	"Export.h"
#include	"Quantities.h"
#include	"Facility.h"


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
		case 32011:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// 벽 테이블폼 배치
					err = ACAPI_CallUndoableCommand(L"벽 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnWall();
						return err;
					});
					break;
				case 2:		// 슬래브 테이블폼 배치
					err = ACAPI_CallUndoableCommand(L"슬래브 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnSlabBottom();
						return err;
					});
					break;
				case 3:		// 보 테이블폼 배치
					err = ACAPI_CallUndoableCommand(L"보 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnBeam();
						return err;
					});
					break;
				case 4:		// 기둥 테이블폼 배치
					err = ACAPI_CallUndoableCommand(L"기둥 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnColumn();
						return err;
					});
					break;
				case 5:		// 기초 테이블폼 배치
					err = ACAPI_CallUndoableCommand(L"기초 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnLowSide();
						return err;
					});
					break;
				case 6:
					// 보 전용 동바리 세트 배치
					err = ACAPI_CallUndoableCommand(L"보 전용 동바리 세트 배치", [&] () -> GSErrCode {
						err = placeSupportingPostForBeam();
						return err;
					});
					break;
			}
			break;

		// 레이어 도구
		case 32005:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// 레이어 쉽게 선택하기
					err = showLayersEasily();
					break;
				case 2:		// 레이어 쉽게 만들기
					err = makeLayersEasily();
					break;
				case 3:		// 레이어 쉽게 지정하기
					err = ACAPI_CallUndoableCommand(L"레이어 쉽게 지정하기", [&]() -> GSErrCode {
						err = assignLayerEasily();
						return err;
					});
					break;
				case 4:		// 레이어 이름 검사하기
					err = inspectLayerNames();
					break;
			}
			break;

		// 내보내기
		case 32007:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// 부재 정보 내보내기 (현재 선택한 것만)
					err = exportSelectedElementInfo();
					break;
				case 2:		// 부재 정보 내보내기 (보이는 레이어 별로)
					err = exportElementInfoOnVisibleLayers();
					break;
				case 3:		// 부재별 선택 후 보여주기
					err = filterSelection();
					break;
				case 4:		// 보 테이블폼 물량표 작성
					err = exportBeamTableformInformation();
					break;
				case 5:		// 테이블폼 면적 계산
					err = calcTableformArea();
					break;
				case 6:		// 물량합판 면적 계산 (선택한 부재에 한해)
					err = exportSelectedQuantityPlywoodArea();
					break;
				case 7:		// 물량합판 면적 계산 (보이는 레이어에 한해)
					err = exportQuantityPlywoodAreaOnVisibleLayers();
					break;
				case 8:		// 모든 입면도 PDF로 내보내기 (현재 뷰만)
					err = exportAllElevationsToPDFSingleMode();
					break;
				case 9:		// 모든 입면도 PDF로 내보내기 (보이는 레이어 별로)
					err = exportAllElevationsToPDFMultiMode();
					break;
			}
			break;

		// 반자동 배치
		case 32009:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// 물량합판 부착하기
					BNZeroMemory(&itemRef, sizeof(API_MenuItemRef));
					itemRef.menuResID = 32009;
					itemRef.itemIndex = 1;
					itemFlags = 0;
					ACAPI_Interface(APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);

					extern qElem qElemInfo;

					if (qElemInfo.dialogID == 0) {
						err = placeQuantityPlywood();
						itemFlags |= API_MenuItemChecked;
					}
					else {
						if ((qElemInfo.dialogID != 0) || DGIsDialogOpen(qElemInfo.dialogID)) {
							DGModelessClose(qElemInfo.dialogID);
							qElemInfo.dialogID = 0;
							itemFlags &= ~API_MenuItemChecked;
						}
					}
					ACAPI_Interface(APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);
					return NoError;

					break;

				case 2:		// 단열재 부착하기
					BNZeroMemory(&itemRef, sizeof(API_MenuItemRef));
					itemRef.menuResID = 32009;
					itemRef.itemIndex = 2;
					itemFlags = 0;
					ACAPI_Interface(APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);

					extern insulElem insulElemInfo;

					if (insulElemInfo.dialogID == 0) {
						err = placeInsulation();
						itemFlags |= API_MenuItemChecked;
					}
					else {
						if ((insulElemInfo.dialogID != 0) || DGIsDialogOpen(insulElemInfo.dialogID)) {
							DGModelessClose(insulElemInfo.dialogID);
							insulElemInfo.dialogID = 0;
							itemFlags &= ~API_MenuItemChecked;
						}
					}
					ACAPI_Interface(APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);
					return NoError;

					break;
			}
			break;

		// 자동 배치
		case 32015:
			switch (menuParams->menuItemRef.itemIndex) {
			case 1:		// 물량합판 자동 부착
				err = ACAPI_CallUndoableCommand(L"물량합판 자동 부착", [&]() -> GSErrCode {
					err = placeQuantityPlywoodAutomatic();
					return err;
				});
				break;
			}
			break;

		// 편의 기능
		case 32013:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// 3D 품질/속도 조정하기
					err = ACAPI_CallUndoableCommand(L"3D 품질/속도 조정하기", [&]() -> GSErrCode {
						err = select3DQuality();
						return err;
					});
					break;
				case 2:		// 영역에 3D 라벨 붙이기
					err = ACAPI_CallUndoableCommand(L"영역에 3D 라벨 붙이기", [&]() -> GSErrCode {
						err = attach3DLabelOnZone();
						return err;
					});
					break;
				case 3:		// 현재 평면도의 테이블폼에 버블 자동 배치
					err = ACAPI_CallUndoableCommand(L"현재 평면도의 테이블폼에 버블 자동 배치", [&]() -> GSErrCode {
						err = attachBubbleOnCurrentFloorPlan();
						return err;
					});
					break;
				case 4:		// 카메라 위치 저장하기/불러오기
					err = manageCameraInfo();
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
	
	err = ACAPI_Register_Menu(32011, 32012, MenuCode_UserDef, MenuFlag_Default);	// 테이블폼 배치
	err = ACAPI_Register_Menu(32005, 32006, MenuCode_UserDef, MenuFlag_Default);	// 레이어 도구
	err = ACAPI_Register_Menu(32007, 32008, MenuCode_UserDef, MenuFlag_Default);	// 내보내기
	err = ACAPI_Register_Menu(32009, 32010, MenuCode_UserDef, MenuFlag_Default);	// 반자동 배치
	err = ACAPI_Register_Menu(32015, 32016, MenuCode_UserDef, MenuFlag_Default);	// 자동 배치
	err = ACAPI_Register_Menu(32013, 32014, MenuCode_UserDef, MenuFlag_Default);	// 편의 기능

	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL Initialize(void)
{
	GSErrCode err;

	err = ACAPI_Install_MenuHandler(32011, MenuCommandHandler);	// 테이블폼 배치
	err = ACAPI_Install_MenuHandler(32005, MenuCommandHandler);	// 레이어 도구
	err = ACAPI_Install_MenuHandler(32007, MenuCommandHandler);	// 내보내기
	err = ACAPI_Install_MenuHandler(32009, MenuCommandHandler);	// 반자동 배치
	err = ACAPI_Install_MenuHandler(32015, MenuCommandHandler);	// 자동 배치
	err = ACAPI_Install_MenuHandler(32013, MenuCommandHandler);	// 편의 기능

	return err;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData(void)
{
	extern qElem qElemInfo;
	if ((qElemInfo.dialogID != 0) || DGIsDialogOpen(qElemInfo.dialogID)) {
		DGModelessClose(qElemInfo.dialogID);
		qElemInfo.dialogID = 0;
	}

	extern insulElem insulElemInfo;
	if ((insulElemInfo.dialogID != 0) || DGIsDialogOpen(insulElemInfo.dialogID)) {
		DGModelessClose(insulElemInfo.dialogID);
		insulElemInfo.dialogID = 0;
	}

	return NoError;
}		// FreeData
