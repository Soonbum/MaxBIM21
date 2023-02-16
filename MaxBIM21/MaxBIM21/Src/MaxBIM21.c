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
		// ���̺��� ��ġ
		case ID_MENU_STRINGS_FORMWORKS:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// �� ���̺��� ��ġ
					err = ACAPI_CallUndoableCommand("�׽�Ʈ", [&]() -> GSErrCode {
						EasyObjectPlacement euroform;
						euroform.init(L"������v2.0.gsm", 1, 0, 0, 0, 0, 0);
						euroform.placeObject(4,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"eu_wid", APIParT_CString, "500",
							"eu_hei", APIParT_CString, "900",
							"u_ins", APIParT_CString, "�������");
						return NoError;
					});
					break;
				case 2:		// ������ ���̺��� ��ġ
					break;
				case 3:		// �� ���̺��� ��ġ
					break;
				case 4:		// ��� ���̺��� ��ġ
					break;
				case 5:		// ���� ���̺��� ��ġ
					break;
			}
			break;

		// ���̾� ����
		case ID_MENU_STRINGS_LAYER_TOOLS:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// ���̾� ���� �����ϱ�
					break;
				case 2:		// ���̾� ���� �����
					break;
				case 3:		// ���̾� ���� �����ϱ�
					break;
				case 4:		// ���̾� �̸� �˻��ϱ�
					break;
			}
			break;

		// ��������
		case ID_MENU_STRINGS_EXPORTING:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// ���� ���� �������� (���� ������ �͸�)
					break;
				case 2:		// ���� ���� �������� (���̴� ���̾� ����)
					break;
				case 3:		// ���纰 ���� �� �����ֱ�
					break;
				case 4:		// �� ���̺��� ����ǥ �ۼ�
					break;
				case 5:		// ���̺��� ���� ���
					break;
				case 6:		// ��� �Ը鵵 PDF�� �������� (���� �丸)
					break;
				case 7:		// ��� �Ը鵵 PDF�� �������� (���̴� ���̾� ����)
					break;
			}
			break;

		// ���ڵ� ��ġ
		case ID_MENU_STRINGS_SEMIAUTO_ARRANGE:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// �������� �����ϱ�
					break;
				case 2:		// �ܿ��� �����ϱ�
					break;
			}
			break;

		// ���� ���
		case ID_MENU_STRINGS_FACILITIES:
			switch(menuParams->menuItemRef.itemIndex) {
				case 1:		// 3D ǰ��/�ӵ� �����ϱ�
					break;
				case 2:		// ������ 3D �� ���̱�
					break;
				case 3:		// ���� ��鵵�� ���̺����� ���� �ڵ� ��ġ
					break;
				case 4:		// ī�޶� ��ġ �����ϱ�/�ҷ�����
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
	
	err = ACAPI_Register_Menu(ID_MENU_STRINGS_FORMWORKS,			ID_MENU_PROMPT_STRINGS_FORMWORKS,			MenuCode_UserDef, MenuFlag_Default);	// ���̺��� ��ġ
	err = ACAPI_Register_Menu(ID_MENU_STRINGS_LAYER_TOOLS,			ID_MENU_PROMPT_STRINGS_LAYER_TOOLS,			MenuCode_UserDef, MenuFlag_Default);	// ���̾� ����
	err = ACAPI_Register_Menu(ID_MENU_STRINGS_EXPORTING,			ID_MENU_PROMPT_STRINGS_EXPORTING,			MenuCode_UserDef, MenuFlag_Default);	// ��������
	err = ACAPI_Register_Menu(ID_MENU_STRINGS_SEMIAUTO_ARRANGE,		ID_MENU_PROMPT_STRINGS_SEMIAUTO_ARRANGE,	MenuCode_UserDef, MenuFlag_Default);	// ���ڵ� ��ġ
	err = ACAPI_Register_Menu(ID_MENU_STRINGS_FACILITIES,			ID_MENU_PROMPT_STRINGS_FACILITIES,			MenuCode_UserDef, MenuFlag_Default);	// ���� ���

	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL Initialize(void)
{
	GSErrCode err;

	err = ACAPI_Install_MenuHandler(ID_MENU_STRINGS_FORMWORKS,			MenuCommandHandler);	// ���̺��� ��ġ
	err = ACAPI_Install_MenuHandler(ID_MENU_STRINGS_LAYER_TOOLS,		MenuCommandHandler);	// ���̾� ����
	err = ACAPI_Install_MenuHandler(ID_MENU_STRINGS_EXPORTING,			MenuCommandHandler);	// ��������
	err = ACAPI_Install_MenuHandler(ID_MENU_STRINGS_SEMIAUTO_ARRANGE,	MenuCommandHandler);	// ���ڵ� ��ġ
	err = ACAPI_Install_MenuHandler(ID_MENU_STRINGS_FACILITIES,			MenuCommandHandler);	// ���� ���

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
