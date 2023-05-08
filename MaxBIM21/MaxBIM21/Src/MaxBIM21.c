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
		// ���̺��� ��ġ
		case 32011:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// �� ���̺��� ��ġ
					err = ACAPI_CallUndoableCommand(L"�� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnWall();
						return err;
					});
					break;
				case 2:		// ������ ���̺��� ��ġ
					err = ACAPI_CallUndoableCommand(L"������ ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnSlabBottom();
						return err;
					});
					break;
				case 3:		// �� ���̺��� ��ġ
					err = ACAPI_CallUndoableCommand(L"�� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnBeam();
						return err;
					});
					break;
				case 4:		// ��� ���̺��� ��ġ
					err = ACAPI_CallUndoableCommand(L"��� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnColumn();
						return err;
					});
					break;
				case 5:		// ���� ���̺��� ��ġ
					err = ACAPI_CallUndoableCommand(L"���� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnLowSide();
						return err;
					});
					break;
				case 6:
					// �� ���� ���ٸ� ��Ʈ ��ġ
					err = ACAPI_CallUndoableCommand(L"�� ���� ���ٸ� ��Ʈ ��ġ", [&] () -> GSErrCode {
						err = placeSupportingPostForBeam();
						return err;
					});
					break;
			}
			break;

		// ���̾� ����
		case 32005:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// ���̾� ���� �����ϱ�
					err = showLayersEasily();
					break;
				case 2:		// ���̾� ���� �����
					err = makeLayersEasily();
					break;
				case 3:		// ���̾� ���� �����ϱ�
					err = ACAPI_CallUndoableCommand(L"���̾� ���� �����ϱ�", [&]() -> GSErrCode {
						err = assignLayerEasily();
						return err;
					});
					break;
				case 4:		// ���̾� �̸� �˻��ϱ�
					err = inspectLayerNames();
					break;
			}
			break;

		// ��������
		case 32007:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// ���� ���� �������� (���� ������ �͸�)
					err = exportSelectedElementInfo();
					break;
				case 2:		// ���� ���� �������� (���̴� ���̾� ����)
					err = exportElementInfoOnVisibleLayers();
					break;
				case 3:		// ���纰 ���� �� �����ֱ�
					err = filterSelection();
					break;
				case 4:		// �� ���̺��� ����ǥ �ۼ�
					err = exportBeamTableformInformation();
					break;
				case 5:		// ���̺��� ���� ���
					err = calcTableformArea();
					break;
				case 6:		// �������� ���� ��� (������ ���翡 ����)
					err = exportSelectedQuantityPlywoodArea();
					break;
				case 7:		// �������� ���� ��� (���̴� ���̾ ����)
					err = exportQuantityPlywoodAreaOnVisibleLayers();
					break;
				case 8:		// ��� �Ը鵵 PDF�� �������� (���� �丸)
					err = exportAllElevationsToPDFSingleMode();
					break;
				case 9:		// ��� �Ը鵵 PDF�� �������� (���̴� ���̾� ����)
					err = exportAllElevationsToPDFMultiMode();
					break;
			}
			break;

		// ���ڵ� ��ġ
		case 32009:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// �������� �����ϱ�
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

				case 2:		// �ܿ��� �����ϱ�
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

		// �ڵ� ��ġ
		case 32015:
			switch (menuParams->menuItemRef.itemIndex) {
			case 1:		// �������� �ڵ� ����
				err = ACAPI_CallUndoableCommand(L"�������� �ڵ� ����", [&]() -> GSErrCode {
					err = placeQuantityPlywoodAutomatic();
					return err;
				});
				break;
			}
			break;

		// ���� ���
		case 32013:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// 3D ǰ��/�ӵ� �����ϱ�
					err = ACAPI_CallUndoableCommand(L"3D ǰ��/�ӵ� �����ϱ�", [&]() -> GSErrCode {
						err = select3DQuality();
						return err;
					});
					break;
				case 2:		// ������ 3D �� ���̱�
					err = ACAPI_CallUndoableCommand(L"������ 3D �� ���̱�", [&]() -> GSErrCode {
						err = attach3DLabelOnZone();
						return err;
					});
					break;
				case 3:		// ���� ��鵵�� ���̺����� ���� �ڵ� ��ġ
					err = ACAPI_CallUndoableCommand(L"���� ��鵵�� ���̺����� ���� �ڵ� ��ġ", [&]() -> GSErrCode {
						err = attachBubbleOnCurrentFloorPlan();
						return err;
					});
					break;
				case 4:		// ī�޶� ��ġ �����ϱ�/�ҷ�����
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
	
	err = ACAPI_Register_Menu(32011, 32012, MenuCode_UserDef, MenuFlag_Default);	// ���̺��� ��ġ
	err = ACAPI_Register_Menu(32005, 32006, MenuCode_UserDef, MenuFlag_Default);	// ���̾� ����
	err = ACAPI_Register_Menu(32007, 32008, MenuCode_UserDef, MenuFlag_Default);	// ��������
	err = ACAPI_Register_Menu(32009, 32010, MenuCode_UserDef, MenuFlag_Default);	// ���ڵ� ��ġ
	err = ACAPI_Register_Menu(32015, 32016, MenuCode_UserDef, MenuFlag_Default);	// �ڵ� ��ġ
	err = ACAPI_Register_Menu(32013, 32014, MenuCode_UserDef, MenuFlag_Default);	// ���� ���

	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL Initialize(void)
{
	GSErrCode err;

	err = ACAPI_Install_MenuHandler(32011, MenuCommandHandler);	// ���̺��� ��ġ
	err = ACAPI_Install_MenuHandler(32005, MenuCommandHandler);	// ���̾� ����
	err = ACAPI_Install_MenuHandler(32007, MenuCommandHandler);	// ��������
	err = ACAPI_Install_MenuHandler(32009, MenuCommandHandler);	// ���ڵ� ��ġ
	err = ACAPI_Install_MenuHandler(32015, MenuCommandHandler);	// �ڵ� ��ġ
	err = ACAPI_Install_MenuHandler(32013, MenuCommandHandler);	// ���� ���

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
