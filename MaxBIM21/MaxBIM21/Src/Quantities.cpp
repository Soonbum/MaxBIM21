#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "MaxBIM21.h"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.h"
#include "Quantities.h"
#include "Export.h"

using namespace quantitiesDG;

qElem	qElemInfo;
insulElem	insulElemInfo;
static GS::Array<API_Guid>	elemList;	// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������

// ���������� ������ �� �ִ� �ȷ�Ʈ�� ���
GSErrCode	placeQuantityPlywood (void)
{
	GSErrCode	err = NoError;

	qElemInfo.dialogID = 0;

	if ((qElemInfo.dialogID == 0) || !DGIsDialogOpen (qElemInfo.dialogID)) {
		qElemInfo.dialogID = DGModelessInit (ACAPI_GetOwnResModule (), 32504, ACAPI_GetOwnResModule (), qElemDlgCallBack, (DGUserData) &qElemInfo, 1);
	}

	return	err;
}

// �ȷ�Ʈ�� ���� �ݹ� �Լ� 1
static short DGCALLBACK qElemDlgCallBack (short message, short dialID, short item, DGUserData userData, DGMessageData /*msgData*/)
{
	qElem		*dlgData = (qElem *) userData;
	API_UCCallbackType	ucb;
	GSErrCode	err = NoError;
	GSErrCode	inputErr;
	//bool	regenerate = true;
	short	xx;

	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("��������(�ֽ������).gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				tempStr [256];
	double				horLen, verLen;
	bool				bValid;

	// ���� �Է� ����
	API_GetPointType	pointInfo;
	double				dx, dy, dz;

	API_Coord3D		p1, p2, p3, p4, p5;
	double			angle1, angle2;

	API_StoryInfo	storyInfo;
	double			storyLevel;
	char			floorName [256];

	
	err = ACAPI_CallUndoableCommand (L"�������� �����ϱ�", [&] () -> GSErrCode {
		switch (message) {
			case DG_MSG_INIT:
				// ��
				DGSetItemText (dialID, LABEL_EXPLANATION_QELEM, L"3�� Ŭ��: ���簢��, 5�� Ŭ��: â����");

				// ���̾�
				BNZeroMemory (&ucb, sizeof (ucb));
				ucb.dialogID = dialID;
				ucb.type	 = APIUserControlType_Layer;
				ucb.itemID	 = USERCONTROL_QPLYWOOD_LAYER;
				ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
				DGSetItemValLong (dialID, USERCONTROL_QPLYWOOD_LAYER, 1);

				// �˾� ��Ʈ�� (�������� Ÿ��)
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"��");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"���(����)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"���(��ü)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"��ü(����)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"��ü(�ܺ�)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"��ü(�պ�)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"��ü(�Ķ���)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"��ü(�����)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"������(����)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"������(RC)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"������(��ũ)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"������(����)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"���");

				// �� ���� ����
				BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
				ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
				for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
					sprintf (floorName, "%d. %s", storyInfo.data [0][xx].index, wcharToChar(storyInfo.data[0][xx].uName));
					DGPopUpInsertItem (dialID, POPUP_FLOOR_QELEM, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialID, POPUP_FLOOR_QELEM, DG_POPUP_BOTTOM, charToWchar(floorName));
				}
				for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
					if (storyInfo.data [0][xx].index == 0) {
						DGPopUpSelectItem (dialID, POPUP_FLOOR_QELEM, xx+1);
					}
				}
				qElemInfo.floorInd = storyInfo.data [0][DGPopUpGetSelected (dialID, POPUP_FLOOR_QELEM) - 1].index;
				BMKillHandle ((GSHandle *) &storyInfo.data);

				// ��ư
				DGSetItemText (dialID, BUTTON_DRAW_RECT_QELEM, L"�������� �׸���\n(���簢��)");
				DGSetItemText (dialID, BUTTON_DRAW_WINDOW_QELEM, L"�������� �׸���\n(â����)");

				// ���̾� ���� ����
				qElemInfo.layerInd = (short)DGGetItemValLong (dialID, USERCONTROL_QPLYWOOD_LAYER);

				// �з� ���� ����
				if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_BEAM) {
					strcpy (qElemInfo.m_type, "��");
					qElemInfo.panel_mat = 78;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_COLUMN_ISOLATION) {
					strcpy (qElemInfo.m_type, "���(����)");
					qElemInfo.panel_mat = 20;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_COLUMN_IN_WALL) {
					strcpy (qElemInfo.m_type, "���(��ü)");
					qElemInfo.panel_mat = 77;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_INSIDE) {
					strcpy (qElemInfo.m_type, "��ü(����)");
					qElemInfo.panel_mat = 75;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_OUTSIDE) {
					strcpy (qElemInfo.m_type, "��ü(�ܺ�)");
					qElemInfo.panel_mat = 76;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_DOUBLE) {
					strcpy (qElemInfo.m_type, "��ü(�պ�)");
					qElemInfo.panel_mat = 72;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_PARAPET) {
					strcpy (qElemInfo.m_type, "��ü(�Ķ���)");
					qElemInfo.panel_mat = 32;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_WATERBLOCK) {
					strcpy (qElemInfo.m_type, "��ü(�����)");
					qElemInfo.panel_mat = 12;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_BASE) {
					strcpy (qElemInfo.m_type, "�����(����)");
					qElemInfo.panel_mat = 66;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_RC) {
					strcpy (qElemInfo.m_type, "�����(RC)");
					qElemInfo.panel_mat = 100;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_DECK) {
					strcpy (qElemInfo.m_type, "�����(��ũ)");
					qElemInfo.panel_mat = 99;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_RAMP) {
					strcpy (qElemInfo.m_type, "�����(����)");
					qElemInfo.panel_mat = 3;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_STAIR) {
					strcpy (qElemInfo.m_type, "���");
					qElemInfo.panel_mat = 73;
				}
 
				// �ȷ�Ʈ â ���
				if (ACAPI_RegisterModelessWindow (dialID, qElemPaletteAPIControlCallBack,
							API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation +
							API_PalEnabled_InteriorElevation + API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_3D + API_PalEnabled_Layout) != NoError)
					DBPrintf ("Test:: ACAPI_RegisterModelessWindow failed\n");

				break;

			case DG_MSG_CHANGE:

				switch (item) {
					case USERCONTROL_QPLYWOOD_LAYER:
						qElemInfo.layerInd = (short)DGGetItemValLong (dialID, USERCONTROL_QPLYWOOD_LAYER);
						break;

					case POPUP_QPLYWOOD_TYPE:
						if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_BEAM) {
							strcpy (qElemInfo.m_type, "��");
							qElemInfo.panel_mat = 78;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_COLUMN_ISOLATION) {
							strcpy (qElemInfo.m_type, "���(����)");
							qElemInfo.panel_mat = 20;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_COLUMN_IN_WALL) {
							strcpy (qElemInfo.m_type, "���(��ü)");
							qElemInfo.panel_mat = 77;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_INSIDE) {
							strcpy (qElemInfo.m_type, "��ü(����)");
							qElemInfo.panel_mat = 75;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_OUTSIDE) {
							strcpy (qElemInfo.m_type, "��ü(�ܺ�)");
							qElemInfo.panel_mat = 76;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_DOUBLE) {
							strcpy (qElemInfo.m_type, "��ü(�պ�)");
							qElemInfo.panel_mat = 72;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_PARAPET) {
							strcpy (qElemInfo.m_type, "��ü(�Ķ���)");
							qElemInfo.panel_mat = 32;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_WATERBLOCK) {
							strcpy (qElemInfo.m_type, "��ü(�����)");
							qElemInfo.panel_mat = 12;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_BASE) {
							strcpy (qElemInfo.m_type, "�����(����)");
							qElemInfo.panel_mat = 66;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_RC) {
							strcpy (qElemInfo.m_type, "�����(RC)");
							qElemInfo.panel_mat = 100;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_DECK) {
							strcpy (qElemInfo.m_type, "�����(��ũ)");
							qElemInfo.panel_mat = 99;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_RAMP) {
							strcpy (qElemInfo.m_type, "�����(����)");
							qElemInfo.panel_mat = 3;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_STAIR) {
							strcpy (qElemInfo.m_type, "���");
							qElemInfo.panel_mat = 73;
						}

						break;

					case POPUP_FLOOR_QELEM:
						// �� ���� ����
						BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
						ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
						qElemInfo.floorInd = storyInfo.data [0][DGPopUpGetSelected (dialID, POPUP_FLOOR_QELEM) - 1].index;
						BMKillHandle ((GSHandle *) &storyInfo.data);

						break;
				}

				break;

			case DG_MSG_CLICK:
				switch (item) {
					case BUTTON_DRAW_RECT_QELEM:
						do {
							BNZeroMemory (&pointInfo, sizeof (API_GetPointType));

							CHCopyC (convertStr(GS::UniString(L"�ٰ����� 1��° ���(���ϴ�)�� Ŭ���Ͻʽÿ�.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p1.x = pointInfo.pos.x;
							p1.y = pointInfo.pos.y;
							p1.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"�ٰ����� 2��° ���(���ϴ�)�� Ŭ���Ͻʽÿ�.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p2.x = pointInfo.pos.x;
							p2.y = pointInfo.pos.y;
							p2.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"�ٰ����� 3��° ���(����)�� Ŭ���Ͻʽÿ�.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p3.x = pointInfo.pos.x;
							p3.y = pointInfo.pos.y;
							p3.z = pointInfo.pos.z;

							// ��ü �ε�
							BNZeroMemory (&elem, sizeof (API_Element));
							BNZeroMemory (&memo, sizeof (API_ElementMemo));
							BNZeroMemory (&libPart, sizeof (libPart));
							GS::ucscpy (libPart.file_UName, gsmName);
							err = ACAPI_LibPart_Search (&libPart, false);
							if (libPart.location != NULL)
								delete libPart.location;
							if (err != NoError)
								break;

							ACAPI_LibPart_Get (&libPart);

							elem.header.typeID = API_ObjectID;
							elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

							ACAPI_Element_GetDefaults (&elem, &memo);
							ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

							// �� ����
							BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
							ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
							for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
								if (storyInfo.data [0][xx].index == qElemInfo.floorInd) {
									storyLevel = storyInfo.data [0][xx].level;
									break;
								}
							}
							BMKillHandle ((GSHandle *) &storyInfo.data);

							// ���̺귯���� �Ķ���� �� �Է� (����)
							elem.header.floorInd = qElemInfo.floorInd;
							elem.object.reflected = false;
							elem.object.libInd = libPart.index;
							elem.object.xRatio = aParam;
							elem.object.yRatio = bParam;
							elem.header.layer = qElemInfo.layerInd;
							elem.object.level = storyLevel;

							setParameterByName (&memo, "m_type", qElemInfo.m_type);
							setParameterByName (&memo, "PANEL_MAT", qElemInfo.panel_mat);

							// ���ǵβ� (���ڿ�)
							strcpy (tempStr, "12");		// 12mm
							setParameterByName (&memo, "m_size", tempStr);

							// ǰ��
							strcpy (tempStr, "���Ǹ���");
							setParameterByName (&memo, "m_size2", tempStr);

							dx = p2.x - p1.x;
							dy = p2.y - p1.y;

							elem.object.angle = atan2 (dy, dx);
							elem.object.pos.x = p1.x;
							elem.object.pos.y = p1.y;
							elem.object.level = p1.z;
							elem.object.level -= storyLevel;

							// ���α���
							horLen = GetDistance (p1, p2);
							setParameterByName (&memo, "NO_WD", horLen);
							elem.object.xRatio = horLen;

							// ���α���
							verLen = GetDistance (p2, p3);
							setParameterByName (&memo, "no_lg1", verLen);
							elem.object.yRatio = verLen;

							bValid = false;
							strcpy (tempStr, "���� �����");	// �⺻��

							// ��ġ��ġ
							strcpy (tempStr, "��԰�");
							setParameterByName (&memo, "m_size1", tempStr);

							// p1, p2, p3 ���� ���� ��� ���� ��
							if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
								dx = p2.x - p1.x;
								dy = p2.y - p1.y;
								angle1 = RadToDegree (atan2 (dy, dx));

								dx = p3.x - p2.x;
								dy = p3.y - p2.y;
								angle2 = RadToDegree (atan2 (dy, dx));

								// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ
								if (abs (angle2 - angle1 - 90) < EPS) {
									strcpy (tempStr, "�ٴڵ���");
									bValid = true;
								}

								// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ����
								if (abs (angle1 - angle2 - 90) < EPS) {
									strcpy (tempStr, "�ٴڱ��");
									moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
									bValid = true;
								}
							// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
							} else {
								// p2�� p3�� x, y ��ǥ�� ����
								if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
									// ???
									strcpy (tempStr, "���� �����");
									setParameterByName (&memo, "ZZYZX", verLen);
									bValid = true;
								
								// p2�� p3�� x, y ��ǥ�� �ٸ�
								} else {
									strcpy (tempStr, "��缳ġ");
									bValid = true;

									// ��ġ����: asin ((p3.z - p2.z) / (p3�� p2 ���� �Ÿ�))
									// ��ġ����: acos ((p3�� p2 ���� ��� ���� �Ÿ�) / (p3�� p2 ���� �Ÿ�))
									// ��ġ����: atan2 ((p3.z - p2.z) / (p3�� p2 ���� ��� ���� �Ÿ�))
									dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
									dy = abs (p3.z - p2.z);
									dz = verLen;
									setParameterByName (&memo, "cons_ang", DegreeToRad (180.0) - acos (dx/dz));
								}
							}
							setParameterByName (&memo, "CONS_DR", tempStr);

							// ��ü ��ġ
							if ((horLen > EPS) && (verLen > EPS) && (bValid == true)) {
								ACAPI_Element_Create (&elem, &memo);
							}

							ACAPI_DisposeElemMemoHdls (&memo);

						} while (inputErr != APIERR_CANCEL);

						break;

					case BUTTON_DRAW_WINDOW_QELEM:
						do {
							BNZeroMemory (&pointInfo, sizeof (API_GetPointType));

							CHCopyC (convertStr(GS::UniString(L"�ٰ����� 1��° ���(���ϴ�)�� Ŭ���Ͻʽÿ�.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p1.x = pointInfo.pos.x;
							p1.y = pointInfo.pos.y;
							p1.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"�ٰ����� 2��° ���(���ϴ�)�� Ŭ���Ͻʽÿ�.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p2.x = pointInfo.pos.x;
							p2.y = pointInfo.pos.y;
							p2.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"�ٰ����� 3��° ���(����)�� Ŭ���Ͻʽÿ�.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p3.x = pointInfo.pos.x;
							p3.y = pointInfo.pos.y;
							p3.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"�ٰ����� 4��° ���(������ ���ϴ�)�� Ŭ���Ͻʽÿ�.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p4.x = pointInfo.pos.x;
							p4.y = pointInfo.pos.y;
							p4.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"�ٰ����� 5��° ���(������ ����)�� Ŭ���Ͻʽÿ�.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p5.x = pointInfo.pos.x;
							p5.y = pointInfo.pos.y;
							p5.z = pointInfo.pos.z;

							// ��ü �ε�
							BNZeroMemory (&elem, sizeof (API_Element));
							BNZeroMemory (&memo, sizeof (API_ElementMemo));
							BNZeroMemory (&libPart, sizeof (libPart));
							GS::ucscpy (libPart.file_UName, gsmName);
							err = ACAPI_LibPart_Search (&libPart, false);
							if (libPart.location != NULL)
								delete libPart.location;
							if (err != NoError)
								break;

							ACAPI_LibPart_Get (&libPart);

							elem.header.typeID = API_ObjectID;
							elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

							ACAPI_Element_GetDefaults (&elem, &memo);
							ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

							// ���̺귯���� �Ķ���� �� �Է� (����)
							elem.header.floorInd = qElemInfo.floorInd;
							elem.object.reflected = false;
							elem.object.libInd = libPart.index;
							elem.object.xRatio = aParam;
							elem.object.yRatio = bParam;
							elem.header.layer = qElemInfo.layerInd;

							setParameterByName (&memo, "m_type", qElemInfo.m_type);
							setParameterByName (&memo, "PANEL_MAT", qElemInfo.panel_mat);

							// ���ǵβ� (���ڿ�)
							strcpy (tempStr, "12");		// 12mm
							setParameterByName (&memo, "m_size", tempStr);

							// ǰ��
							strcpy (tempStr, "���Ǹ���");
							setParameterByName (&memo, "m_size2", tempStr);

							dx = p2.x - p1.x;
							dy = p2.y - p1.y;

							elem.object.angle = atan2 (dy, dx);
							elem.object.pos.x = p1.x;
							elem.object.pos.y = p1.y;
							elem.object.level = p1.z;
							elem.object.level -= storyLevel;

							// ���α���
							horLen = GetDistance (p1, p2);
							setParameterByName (&memo, "NO_WD", horLen);
							elem.object.xRatio = horLen;

							// ���α���
							verLen = GetDistance (p2, p3);
							setParameterByName (&memo, "no_lg1", verLen);
							elem.object.yRatio = verLen;

							bValid = false;
							strcpy (tempStr, "���� �����");	// �⺻��

							// ��ġ��ġ
							strcpy (tempStr, "â����");
							setParameterByName (&memo, "m_size1", tempStr);

							// ��ġ����
							API_Coord3D		origin;
							API_Coord3D		target_before, target_after;
								
							// p1, p2, p3 ���� ���� ��� ���� ��
							if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
								dx = p2.x - p1.x;
								dy = p2.y - p1.y;
								angle1 = RadToDegree (atan2 (dy, dx));

								dx = p3.x - p2.x;
								dy = p3.y - p2.y;
								angle2 = RadToDegree (atan2 (dy, dx));

								// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ
								if (abs (angle2 - angle1 - 90) < EPS) {
									origin = p1;
									target_before = p4;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw1", abs (origin.x - target_after.x));				// ��1: vw1
									setParameterByName (&memo, "vh1", abs (origin.y - target_after.y));				// ����1: vh1

									target_before = p5;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw2", abs (origin.x + horLen - target_after.x));	// ��2: vw2
									setParameterByName (&memo, "vh2", abs (origin.y + verLen - target_after.y));	// ����2: vh2

									strcpy (tempStr, "�ٴڵ���");
									bValid = true;
								}

								// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ����
								if (abs (angle1 - angle2 - 90) < EPS) {
									origin = p1;
									target_before = p4;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw1", abs (origin.x - target_after.x));				// ��1: vw1
									setParameterByName (&memo, "vh2", abs (origin.y - target_after.y));				// ����2: vh2

									target_before = p5;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw2", abs (origin.x + horLen - target_after.x));	// ��2: vw2
									setParameterByName (&memo, "vh1", verLen - abs (origin.y - target_after.y));	// ����1: vh1

									strcpy (tempStr, "�ٴڱ��");
									moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
									bValid = true;
								}
								
							// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
							} else {
								if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
									// p2�� p3�� x, y ��ǥ�� ����
									strcpy (tempStr, "���� �����");
									setParameterByName (&memo, "ZZYZX", verLen);
									bValid = true;

									origin = p1;
									target_before = p4;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw1", target_after.x - origin.x);						// ��1: vw1
									setParameterByName (&memo, "vh1", target_after.z - origin.z);						// ����1: vh1

									target_before = p5;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw2", origin.x + horLen - target_after.x);				// ��2: vw2
									setParameterByName (&memo, "vh2", origin.z + verLen - target_after.z);				// ����2: vh2
								}
							}
							setParameterByName (&memo, "CONS_DR", tempStr);

							// ��ü ��ġ
							if ((horLen > EPS) && (verLen > EPS) && (bValid == true)) {
								ACAPI_Element_Create (&elem, &memo);
							}

							ACAPI_DisposeElemMemoHdls (&memo);

						} while (inputErr != APIERR_CANCEL);

						break;

					case DG_CLOSEBOX:
						return item;	// �̰��� DG_MSG_CLOSE �޽����� ����
				}
				break;

			case DG_MSG_DOUBLECLICK:
				break;

			case DG_MSG_CLOSE:
				ACAPI_UnregisterModelessWindow (dlgData->dialogID);
				dlgData->dialogID = 0;
				break;
		}

		return NoError;
	});

	return 0;
}

// �ȷ�Ʈ�� ���� �ݹ� �Լ� 2
static GSErrCode __ACENV_CALL	qElemPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/)
{
	if (referenceID == qElemInfo.dialogID) {
		switch (messageID) {
			case APIPalMsg_ClosePalette:		DGModelessClose (qElemInfo.dialogID);
												break;
			case APIPalMsg_HidePalette_Begin:	DGHideModelessDialog (qElemInfo.dialogID);
												break;
			case APIPalMsg_HidePalette_End:		DGShowModelessDialog (qElemInfo.dialogID, DG_DF_FIRST);
												break;
			case APIPalMsg_DisableItems_Begin:	EnablePaletteControls (qElemInfo.dialogID, false);
												break;
			case APIPalMsg_DisableItems_End:	EnablePaletteControls (qElemInfo.dialogID, true);
												break;
			case APIPalMsg_IsPaletteVisible:	DGModelessClose (qElemInfo.dialogID);
												break;
			case APIPalMsg_OpenPalette:			break;
			default:							break;
		}
	}

	return NoError;
}


// �ܿ��縦 ������ �� �ִ� �ȷ�Ʈ�� ���
GSErrCode	placeInsulation (void)
{
	GSErrCode	err = NoError;

	insulElemInfo.dialogID = 0;

	if ((insulElemInfo.dialogID == 0) || !DGIsDialogOpen (insulElemInfo.dialogID)) {
		insulElemInfo.dialogID = DGModelessInit (ACAPI_GetOwnResModule (), 32507, ACAPI_GetOwnResModule (), insulElemDlgCallBack, (DGUserData) &insulElemInfo, 1);
	}

	return	err;
}

// �ȷ�Ʈ�� ���� �ݹ� �Լ� 1
static short DGCALLBACK insulElemDlgCallBack (short message, short dialID, short item, DGUserData userData, DGMessageData /*msgData*/)
{
	insulElem		*dlgData = (insulElem *) userData;
	API_UCCallbackType	ucb;
	GSErrCode	err = NoError;
	GSErrCode	inputErr;
	//bool	regenerate = true;

	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("�ܿ���v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	short				xx, yy;
	short				totalXX, totalYY;
	double				horLen, verLen;
	double				remainHorLen, remainVerLen;

	// ���� �Է� ����
	API_GetPointType	pointInfo;
	double				dx, dy, dz;

	// �� 3���� ��ġ�� ������ ����
	API_Coord3D		p1, p2, p3;
	double			angle1, angle2;

	// �� ����
	API_StoryInfo	storyInfo;
	double			storyLevel;
	char			floorName [256];

	
	err = ACAPI_CallUndoableCommand (L"�ܿ��� �����ϱ�", [&] () -> GSErrCode {
		switch (message) {
			case DG_MSG_INIT:
				// ��
				DGSetItemText (dialID, LABEL_EXPLANATION_INS, L"3�� Ŭ��: ���簢��");
				DGSetItemText (dialID, LABEL_INSULATION_THK, L"�β�");
				DGSetItemText (dialID, LABEL_INS_HORLEN, L"����");
				DGSetItemText (dialID, LABEL_INS_VERLEN, L"����");
				DGSetItemText (dialID, LABEL_FLOOR_INS, L"�� ����");

				// üũ�ڽ�
				DGSetItemText (dialID, CHECKBOX_INS_LIMIT_SIZE, L"����/���� ũ�� ����");
				DGSetItemValLong (dialID, CHECKBOX_INS_LIMIT_SIZE, TRUE);

				// Edit ��Ʈ��
				DGSetItemValDouble (dialID, EDITCONTROL_INSULATION_THK, 0.120);
				DGSetItemValDouble (dialID, EDITCONTROL_INS_HORLEN, 0.900);
				DGSetItemValDouble (dialID, EDITCONTROL_INS_VERLEN, 1.800);

				// ���̾�
				BNZeroMemory (&ucb, sizeof (ucb));
				ucb.dialogID = dialID;
				ucb.type	 = APIUserControlType_Layer;
				ucb.itemID	 = USERCONTROL_INSULATION_LAYER;
				ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
				DGSetItemValLong (dialID, USERCONTROL_INSULATION_LAYER, 1);

				// ��ư
				DGSetItemText (dialID, BUTTON_DRAW_INSULATION, L"�ܿ��� �׸���");

				// �� ���� ����
				BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
				ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
				for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
					sprintf (floorName, "%d. %s", storyInfo.data [0][xx].index, wcharToChar(storyInfo.data [0][xx].uName));
					DGPopUpInsertItem (dialID, POPUP_FLOOR_INS, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialID, POPUP_FLOOR_INS, DG_POPUP_BOTTOM, charToWchar(floorName));
				}
				for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
					if (storyInfo.data [0][xx].index == 0) {
						DGPopUpSelectItem (dialID, POPUP_FLOOR_INS, xx+1);
					}
				}
				insulElemInfo.floorInd = storyInfo.data [0][DGPopUpGetSelected (dialID, POPUP_FLOOR_INS) - 1].index;
				BMKillHandle ((GSHandle *) &storyInfo.data);

				// ���̾� ���� ����
				insulElemInfo.layerInd = (short)DGGetItemValLong (dialID, USERCONTROL_INSULATION_LAYER);

				// �β�, ����, ���� ����
				insulElemInfo.thk = DGGetItemValDouble (dialID, EDITCONTROL_INSULATION_THK);
				insulElemInfo.maxHorLen = DGGetItemValDouble (dialID, EDITCONTROL_INS_HORLEN);
				insulElemInfo.maxVerLen = DGGetItemValDouble (dialID, EDITCONTROL_INS_VERLEN);
				if (DGGetItemValLong (dialID, CHECKBOX_INS_LIMIT_SIZE) == TRUE)
					insulElemInfo.bLimitSize = true;
				else
					insulElemInfo.bLimitSize = false;
 
				// �ȷ�Ʈ â ���
				if (ACAPI_RegisterModelessWindow (dialID, insulElemPaletteAPIControlCallBack,
							API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation +
							API_PalEnabled_InteriorElevation + API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_3D + API_PalEnabled_Layout) != NoError)
					DBPrintf ("Test:: ACAPI_RegisterModelessWindow failed\n");

				break;

			case DG_MSG_CHANGE:
				switch (item) {
					case CHECKBOX_INS_LIMIT_SIZE:
						if (DGGetItemValLong (dialID, CHECKBOX_INS_LIMIT_SIZE) == TRUE) {
							DGEnableItem (dialID, EDITCONTROL_INS_HORLEN);
							DGEnableItem (dialID, EDITCONTROL_INS_VERLEN);
						} else {
							DGDisableItem (dialID, EDITCONTROL_INS_HORLEN);
							DGDisableItem (dialID, EDITCONTROL_INS_VERLEN);
						}
						break;
				}

				// �� ���� ����
				BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
				ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
				insulElemInfo.floorInd = storyInfo.data [0][DGPopUpGetSelected (dialID, POPUP_FLOOR_INS) - 1].index;
				BMKillHandle ((GSHandle *) &storyInfo.data);

				// ���̾� ���� ����
				insulElemInfo.layerInd = (short)DGGetItemValLong (dialID, USERCONTROL_INSULATION_LAYER);

				// �β�, ����, ���� ����
				insulElemInfo.thk = DGGetItemValDouble (dialID, EDITCONTROL_INSULATION_THK);
				insulElemInfo.maxHorLen = DGGetItemValDouble (dialID, EDITCONTROL_INS_HORLEN);
				insulElemInfo.maxVerLen = DGGetItemValDouble (dialID, EDITCONTROL_INS_VERLEN);
				if (DGGetItemValLong (dialID, CHECKBOX_INS_LIMIT_SIZE) == TRUE)
					insulElemInfo.bLimitSize = true;
				else
					insulElemInfo.bLimitSize = false;
 
				break;

			case DG_MSG_CLICK:
				switch (item) {
					case BUTTON_DRAW_INSULATION:
						do {
							BNZeroMemory (&pointInfo, sizeof (API_GetPointType));

							CHCopyC (convertStr(GS::UniString(L"�ٰ����� 1��° ���(���ϴ�)�� Ŭ���Ͻʽÿ�.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p1.x = pointInfo.pos.x;
							p1.y = pointInfo.pos.y;
							p1.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"�ٰ����� 2��° ���(���ϴ�)�� Ŭ���Ͻʽÿ�.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p2.x = pointInfo.pos.x;
							p2.y = pointInfo.pos.y;
							p2.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"�ٰ����� 3��° ���(����)�� Ŭ���Ͻʽÿ�.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p3.x = pointInfo.pos.x;
							p3.y = pointInfo.pos.y;
							p3.z = pointInfo.pos.z;

							// ���� ���
							dx = p2.x - p1.x;
							dy = p2.y - p1.y;
							angle1 = RadToDegree (atan2 (dy, dx));

							dx = p3.x - p2.x;
							dy = p3.y - p2.y;
							angle2 = RadToDegree (atan2 (dy, dx));

							// ��ü �ε�
							BNZeroMemory (&elem, sizeof (API_Element));
							BNZeroMemory (&memo, sizeof (API_ElementMemo));
							BNZeroMemory (&libPart, sizeof (libPart));
							GS::ucscpy (libPart.file_UName, gsmName);
							err = ACAPI_LibPart_Search (&libPart, false);
							if (libPart.location != NULL)
								delete libPart.location;
							if (err != NoError)
								break;

							ACAPI_LibPart_Get (&libPart);

							elem.header.typeID = API_ObjectID;
							elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

							ACAPI_Element_GetDefaults (&elem, &memo);
							ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

							// �� ����
							BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
							ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
							for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
								if (storyInfo.data [0][xx].index == insulElemInfo.floorInd) {
									storyLevel = storyInfo.data [0][xx].level;
									break;
								}
							}
							BMKillHandle ((GSHandle *) &storyInfo.data);

							// ���̺귯���� �Ķ���� �� �Է� (����)
							elem.header.floorInd = insulElemInfo.floorInd;
							elem.object.libInd = libPart.index;
							elem.object.reflected = false;
							elem.object.xRatio = aParam;
							elem.object.yRatio = bParam;
							elem.header.layer = insulElemInfo.layerInd;

							setParameterByName (&memo, "bRestrictSize", insulElemInfo.bLimitSize);	// ũ�� ���� ����
							setParameterByName (&memo, "maxHorLen", insulElemInfo.maxHorLen);		// �ܿ��� �ִ� ���� ����
							setParameterByName (&memo, "maxVerLen", insulElemInfo.maxVerLen);		// �ܿ��� �ִ� ���� ����
							setParameterByName (&memo, "ZZYZX", insulElemInfo.thk);					// �ܿ��� �β�
							setParameterByName (&memo, "bLShape", 0.0);								// �ܿ���� ���簢�� ���·� ����ϹǷ� L������ Off

							// ����/���� ũ�� ������ true�� ��
							if (insulElemInfo.bLimitSize == true) {
								// p1-p2�� p2-p3���� ��ų� ���� ��� (���� ����)
								if (GetDistance (p1, p2) >= GetDistance (p2, p3)) {
									// ���� ũ�� >= ���� ũ�� (�״�� ��ġ)
									if (insulElemInfo.maxHorLen >= insulElemInfo.maxVerLen) {
										// p1, p2, p3 ���� ���� ��� ���� ��
										if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ (�ٴڸ� ����)
											if (abs (angle2 - angle1 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('y', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ���� (õ��� ����)
											if (abs (angle1 - angle2 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;

														setParameterByName (&memo, "angX", DegreeToRad (180.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										
										// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
										} else {
											// p2�� p3�� x, y ��ǥ�� ���� (����)
											if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (90.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('z', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											
											// p2�� p3�� x, y ��ǥ�� �ٸ� (���)
											} else {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z - insulElemInfo.thk;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
												dy = abs (p3.z - p2.z);
												dz = GetDistance (p2, p3);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (180.0) - acos (dx/dz));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 1.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('y', elem.object.angle, -verLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														moveIn3D ('z', elem.object.angle, verLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3) * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3) * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										}
									
									// ���� ũ�� < ���� ũ�� (������ ��ġ)
									} else {
										// p1, p2, p3 ���� ���� ��� ���� ��
										if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ (�ٴڸ� ����)
											if (abs (angle2 - angle1 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;

														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('y', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (-90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (-90.0);
															moveIn3D ('y', elem.object.angle, -horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('y', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ���� (õ��� ����)
											if (abs (angle1 - angle2 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;

														setParameterByName (&memo, "angX", DegreeToRad (180.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (-90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (-90.0);
															moveIn3D ('x', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('y', elem.object.angle, -horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										
										// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
										} else {
											// p2�� p3�� x, y ��ǥ�� ���� (����)
											if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", DegreeToRad (90.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('z', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (-90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (-90.0);
															moveIn3D ('z', elem.object.angle, -horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('z', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											
											// p2�� p3�� x, y ��ǥ�� �ٸ� (���)
											} else {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z - insulElemInfo.thk;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
												dy = abs (p3.z - p2.z);
												dz = GetDistance (p2, p3);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", DegreeToRad (180.0) - acos (dx/dz));
														setParameterByName (&memo, "bVerticalCut", 1.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('z', elem.object.angle, horLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															moveIn3D ('y', elem.object.angle, -horLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (-90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (-90.0);
															moveIn3D ('z', elem.object.angle, -horLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															moveIn3D ('y', elem.object.angle, horLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('y', elem.object.angle, -horLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														moveIn3D ('z', elem.object.angle, horLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3) * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3) * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										}
									}
								
								// p1-p2�� p2-p3���� ª�� ��� (���� ����)
								} else {
									// ���� ũ�� >= ���� ũ�� (������ ��ġ)
									if (insulElemInfo.maxHorLen >= insulElemInfo.maxVerLen) {
										// p1, p2, p3 ���� ���� ��� ���� ��
										if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ (�ٴڸ� ����)
											if (abs (angle2 - angle1 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;

														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (90.0);
															moveIn3D ('x', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('y', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ���� (õ��� ����)
											if (abs (angle1 - angle2 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;

														setParameterByName (&memo, "angX", DegreeToRad (180.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (-90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (-90.0);
															moveIn3D ('x', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('y', elem.object.angle, -horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										
										// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
										} else {
											// p2�� p3�� x, y ��ǥ�� ���� (����)
											if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (90.0));
														setParameterByName (&memo, "angY", DegreeToRad (-90.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															moveIn3D ('x', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('z', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											
											// p2�� p3�� x, y ��ǥ�� �ٸ� (���)
											} else {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z - insulElemInfo.thk;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
												dy = abs (p3.z - p2.z);
												dz = GetDistance (p2, p3);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", acos (dx/dz));
														setParameterByName (&memo, "bVerticalCut", 1.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('z', elem.object.angle, horLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															moveIn3D ('y', elem.object.angle, -horLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (90.0);
															moveIn3D ('z', elem.object.angle, -horLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															moveIn3D ('y', elem.object.angle, horLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															moveIn3D ('x', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('y', elem.object.angle, -horLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														moveIn3D ('z', elem.object.angle, horLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3) * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3) * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										}
									
									// ���� ũ�� < ���� ũ�� (�״�� ��ġ)
									} else {
										// p1, p2, p3 ���� ���� ��� ���� ��
										if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ (�ٴڸ� ����)
											if (abs (angle2 - angle1 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('y', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ���� (õ��� ����)
											if (abs (angle1 - angle2 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;

														setParameterByName (&memo, "angX", DegreeToRad (180.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										
										// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
										} else {
											// p2�� p3�� x, y ��ǥ�� ���� (����)
											if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (90.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('z', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											
											// p2�� p3�� x, y ��ǥ�� �ٸ� (���)
											} else {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z - insulElemInfo.thk;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
												dy = abs (p3.z - p2.z);
												dz = GetDistance (p2, p3);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (180.0) - acos (dx/dz));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 1.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('y', elem.object.angle, -verLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														moveIn3D ('z', elem.object.angle, verLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3) * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3) * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										}
									}
								}
							
							// ����/���� ũ�� ������ false�� ��
							} else {
								// p1, p2, p3 ���� ���� ��� ���� ��
								if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
									// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ (�ٴڸ� ����)
									if (abs (angle2 - angle1 - 90) < EPS) {
										// ��ü�� �ʱ� ��ġ �� ����
										elem.object.pos.x = p1.x;
										elem.object.pos.y = p1.y;
										elem.object.level = p1.z;
										elem.object.level -= storyLevel;
										elem.object.angle = DegreeToRad (angle1);

										horLen = GetDistance (p1, p2);
										verLen = GetDistance (p2, p3);

										setParameterByName (&memo, "angX", DegreeToRad (0.0));
										setParameterByName (&memo, "angY", DegreeToRad (0.0));
										setParameterByName (&memo, "bVerticalCut", 0.0);
										setParameterByName (&memo, "A", horLen);
										setParameterByName (&memo, "B", verLen);
										elem.object.xRatio = horLen;
										elem.object.yRatio = verLen;

										// ��ü ��ġ
										if ((horLen > EPS) && (verLen > EPS)) {
											ACAPI_Element_Create (&elem, &memo);
											elemList.Push (elem.header.guid);
										}

										ACAPI_DisposeElemMemoHdls (&memo);
									}
									// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ���� (õ��� ����)
									if (abs (angle1 - angle2 - 90) < EPS) {
										// ��ü�� �ʱ� ��ġ �� ����
										elem.object.pos.x = p1.x;
										elem.object.pos.y = p1.y;
										elem.object.level = p1.z;
										elem.object.level -= storyLevel;
										elem.object.angle = DegreeToRad (angle1);

										horLen = GetDistance (p1, p2);
										verLen = GetDistance (p2, p3);

										setParameterByName (&memo, "angX", DegreeToRad (180.0));
										setParameterByName (&memo, "angY", DegreeToRad (0.0));
										setParameterByName (&memo, "bVerticalCut", 0.0);
										setParameterByName (&memo, "A", horLen);
										setParameterByName (&memo, "B", verLen);
										elem.object.xRatio = horLen;
										elem.object.yRatio = verLen;

										// ��ü ��ġ
										if ((horLen > EPS) && (verLen > EPS)) {
											ACAPI_Element_Create (&elem, &memo);
											elemList.Push (elem.header.guid);
										}

										ACAPI_DisposeElemMemoHdls (&memo);
									}
										
								// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
								} else {
									// p2�� p3�� x, y ��ǥ�� ���� (����)
									if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
										// ��ü�� �ʱ� ��ġ �� ����
										elem.object.pos.x = p1.x;
										elem.object.pos.y = p1.y;
										elem.object.level = p1.z;
										elem.object.level -= storyLevel;
										elem.object.angle = DegreeToRad (angle1);

										horLen = GetDistance (p1, p2);
										verLen = GetDistance (p2, p3);

										setParameterByName (&memo, "angX", DegreeToRad (90.0));
										setParameterByName (&memo, "angY", DegreeToRad (0.0));
										setParameterByName (&memo, "bVerticalCut", 0.0);
										setParameterByName (&memo, "A", horLen);
										setParameterByName (&memo, "B", verLen);
										elem.object.xRatio = horLen;
										elem.object.yRatio = verLen;

										// ��ü ��ġ
										if ((horLen > EPS) && (verLen > EPS)) {
											ACAPI_Element_Create (&elem, &memo);
											elemList.Push (elem.header.guid);
										}

										ACAPI_DisposeElemMemoHdls (&memo);
											
									// p2�� p3�� x, y ��ǥ�� �ٸ� (���)
									} else {
										// ��ü�� �ʱ� ��ġ �� ����
										elem.object.pos.x = p1.x;
										elem.object.pos.y = p1.y;
										elem.object.level = p1.z - insulElemInfo.thk;
										elem.object.level -= storyLevel;
										elem.object.angle = DegreeToRad (angle1);

										horLen = GetDistance (p1, p2);
										verLen = GetDistance (p2, p3);

										dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
										dy = abs (p3.z - p2.z);
										dz = GetDistance (p2, p3);

										setParameterByName (&memo, "angX", DegreeToRad (180.0) - acos (dx/dz));
										setParameterByName (&memo, "angY", DegreeToRad (0.0));
										setParameterByName (&memo, "bVerticalCut", 1.0);
										setParameterByName (&memo, "A", horLen);
										setParameterByName (&memo, "B", verLen);
										elem.object.xRatio = horLen;
										elem.object.yRatio = verLen;

										// ��ü ��ġ
										if ((horLen > EPS) && (verLen > EPS)) {
											ACAPI_Element_Create (&elem, &memo);
											elemList.Push (elem.header.guid);
										}

										ACAPI_DisposeElemMemoHdls (&memo);
									}
								}
							}

							// �׷�ȭ�ϱ�
							groupElements (elemList);
							elemList.Clear (false);

						} while (inputErr != APIERR_CANCEL);

						break;

					case DG_CLOSEBOX:
						return item;	// �̰��� DG_MSG_CLOSE �޽����� ����
				}
				break;

			case DG_MSG_DOUBLECLICK:
				break;

			case DG_MSG_CLOSE:
				ACAPI_UnregisterModelessWindow (dlgData->dialogID);
				dlgData->dialogID = 0;
				break;
		}

		return NoError;
	});

	return 0;
}

// �ȷ�Ʈ�� ���� �ݹ� �Լ� 2
static GSErrCode __ACENV_CALL	insulElemPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/)
{
	if (referenceID == insulElemInfo.dialogID) {
		switch (messageID) {
			case APIPalMsg_ClosePalette:		DGModelessClose (insulElemInfo.dialogID);
												break;
			case APIPalMsg_HidePalette_Begin:	DGHideModelessDialog (insulElemInfo.dialogID);
												break;
			case APIPalMsg_HidePalette_End:		DGShowModelessDialog (insulElemInfo.dialogID, DG_DF_FIRST);
												break;
			case APIPalMsg_DisableItems_Begin:	EnablePaletteControls (insulElemInfo.dialogID, false);
												break;
			case APIPalMsg_DisableItems_End:	EnablePaletteControls (insulElemInfo.dialogID, true);
												break;
			case APIPalMsg_IsPaletteVisible:	DGModelessClose (insulElemInfo.dialogID);
												break;
			case APIPalMsg_OpenPalette:			break;
			default:							break;
		}
	}

	return NoError;
}

// �������� �ڵ� �����ϱ�
GSErrCode	placeQuantityPlywoodAutomatic(void)
{
	GSErrCode	err = NoError;

	API_Element			elem;
	API_ElementMemo		memo;

	// ���̾� ���� ����
	short			nLayers;
	API_Attribute	attrib;
	API_AttributeDef  defs;
	API_Attribute	attrib_new;
	API_AttributeDef  defs_new;
	short			nVisibleLayers = 0;
	short			visLayerList[1024];
	char			fullLayerName[512];
	vector<LayerList>	layerList;

	// �۾� �� ����
	double			workLevel_object;		// ��ü�� �۾� �� ����

	// ��, ���, ��, ������ ���� ����
	GS::Array<API_Guid>	elemList_Beam;
	GS::Array<API_Guid>	elemList_Column;
	GS::Array<API_Guid>	elemList_Wall;
	GS::Array<API_Guid>	elemList_Slab;

	long nBeams;
	long nColumns;
	long nWalls;
	long nSlabs;

	// ����� ��ġ, ũ�� ����
	double leftBottomX;
	double leftBottomY;
	double leftBottomZ;
	double rightTopX;
	double rightTopY;
	double dx;
	double dy;
	double ang;
	double len1;
	double len2;
	double len3;


	// �׷�ȭ �Ͻ����� ON
	suspendGroups(true);

	// ������Ʈ �� ���̾� ������ �˾Ƴ�
	nLayers = getLayerCount();

	// ���̴� ���̾���� ��� �����ϱ�
	for (short xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList[nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// ���̾� �̸��� �ε��� ����
	for (short xx = 0; xx < nVisibleLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList[xx];
		err = ACAPI_Attribute_Get(&attrib);

		sprintf(fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName[strlen(fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList[xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back(newLayerItem);
	}

	// ���̾� �̸� �������� �����Ͽ� ���̾� �ε��� ���� ����
	sort(layerList.begin(), layerList.end(), compareLayerName);		// ���̾� �̸� ���� �������� ����

	// �Ͻ������� ��� ���̾� �����
	for (short xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
		}
	}

	// ���̴� ���̾���� �ϳ��� ��ȸ
	for (short mm = 1; mm <= nVisibleLayers; ++mm) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = layerList[mm - 1].layerInd;
		err = ACAPI_Attribute_Get(&attrib);

		// �ʱ�ȭ
		elemList_Beam.Clear();
		elemList_Column.Clear();
		elemList_Wall.Clear();
		elemList_Slab.Clear();

		if (err == NoError) {
			// ���̾� ���̱�
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify(&attrib, NULL);
			}

			// ��, ���, ��, ������ ��������
			ACAPI_Element_GetElemList(API_BeamID, &elemList_Beam, APIFilt_OnVisLayer);		// ���̴� ���̾ ����
			ACAPI_Element_GetElemList(API_ColumnID, &elemList_Column, APIFilt_OnVisLayer);	// ���̴� ���̾ ����
			ACAPI_Element_GetElemList(API_WallID, &elemList_Wall, APIFilt_OnVisLayer);		// ���̴� ���̾ ����
			ACAPI_Element_GetElemList(API_SlabID, &elemList_Slab, APIFilt_OnVisLayer);		// ���̴� ���̾ ����

			nBeams = elemList_Beam.GetSize();
			nColumns = elemList_Column.GetSize();
			nWalls = elemList_Wall.GetSize();
			nSlabs = elemList_Slab.GetSize();

			// ��, ���, ��, ������ �� �ϳ��� ���� ��� ���̾� �����ϱ�
			if ((!elemList_Beam.IsEmpty() || !elemList_Column.IsEmpty() || !elemList_Wall.IsEmpty() || !elemList_Slab.IsEmpty()) == true) {
				// ���̾� �̸� ������
				sprintf(fullLayerName, "%s", attrib.layer.head.name);
				fullLayerName[strlen(fullLayerName)] = '\0';
				GS::UniString fullLayerNameUniStr = (fullLayerName);
				GS::UniString newFullLayerNameUniStr;

				// ���̾� �̸��� "01-S"�� ������ ���, "07-Q"�� ġȯ�� ���̾� �̸��� ������ �� (�� ���� ��� "���� ���̾� �̸� - ��������" ���̾� ������ ��)
				if (fullLayerNameUniStr.Contains("01-S") == TRUE) {
					fullLayerNameUniStr.ReplaceFirst("01-S", "07-Q");
				}
				else {
					fullLayerNameUniStr = fullLayerNameUniStr + L" - ��������";
				}

				newFullLayerNameUniStr = fullLayerNameUniStr.ToUStr().Get();

				BNZeroMemory(&attrib_new, sizeof(API_Attribute));
				BNZeroMemory(&defs_new, sizeof(API_AttributeDef));

				attrib_new.header.typeID = API_LayerID;
				attrib_new.layer.conClassId = 1;
				if ((attrib_new.layer.head.flags & APILay_Hidden) == true)	attrib_new.layer.head.flags ^= APILay_Hidden;
				attrib_new.header.uniStringNamePtr = &fullLayerNameUniStr;
				ACAPI_Attribute_Create(&attrib_new, &defs_new);

				ACAPI_DisposeAttrDefsHdls(&defs_new);

				// ���� ���
				for (int i = 0; i < nBeams; i++) {
					// ��� ���� ��������
					BNZeroMemory(&elem, sizeof(API_Element));
					BNZeroMemory(&memo, sizeof(API_ElementMemo));
					elem.header.guid = elemList_Beam.Pop();
					ACAPI_Element_Get(&elem);

					// �� ���� ��������
					workLevel_object = getWorkLevel(elem.header.floorInd);

					// ��ü�� ���� �� ũ�� ���� ��������
					leftBottomX = elem.beam.begC.x;
					leftBottomY = elem.beam.begC.y;
					leftBottomZ = elem.beam.level - elem.beam.height;
					dx = elem.beam.endC.x - elem.beam.begC.x;
					dy = elem.beam.endC.y - elem.beam.begC.y;
					ang = atan2(dy, dx);
					len1 = GetDistance(elem.beam.begC, elem.beam.endC);
					len2 = elem.beam.width;
					len3 = elem.beam.height;

					// �������� �����ϱ�
					EasyObjectPlacement qPlywood;
					qPlywood.init(L"��������(�ֽ������).gsm", attrib_new.layer.head.index, elem.header.floorInd, leftBottomX, leftBottomY, leftBottomZ, ang);
					moveIn3D('y', qPlywood.radAng, -len2/2 + elem.beam.offset, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "��������",
						"m_type", APIParT_CString, "��",
						"PANEL_MAT", APIParT_Mater, "78",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "��԰�",
						"m_size2", APIParT_CString, "���Ǹ���",
						"CONS_DR", APIParT_CString, "�ٴڱ��",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len2).c_str());
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "��������",
						"m_type", APIParT_CString, "��",
						"PANEL_MAT", APIParT_Mater, "78",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "��԰�",
						"m_size2", APIParT_CString, "���Ǹ���",
						"CONS_DR", APIParT_CString, "���� �����",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					moveIn3D('y', qPlywood.radAng, len1, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "��������",
						"m_type", APIParT_CString, "��",
						"PANEL_MAT", APIParT_Mater, "78",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "��԰�",
						"m_size2", APIParT_CString, "���Ǹ���",
						"CONS_DR", APIParT_CString, "���� �����",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
				}

				// ����� ���
				for (int i = 0; i < nColumns; i++) {
					// ��� ���� ��������
					BNZeroMemory(&elem, sizeof(API_Element));
					BNZeroMemory(&memo, sizeof(API_ElementMemo));
					elem.header.guid = elemList_Column.Pop();
					ACAPI_Element_Get(&elem);

					// �� ���� ��������
					workLevel_object = getWorkLevel(elem.header.floorInd);

					// ��ü�� ���� �� ũ�� ���� ��������
					leftBottomX = elem.column.origoPos.x;
					leftBottomY = elem.column.origoPos.y;
					leftBottomZ = elem.column.bottomOffset;
					ang = elem.column.angle + elem.column.slantDirectionAngle;
					len1 = elem.column.coreWidth;
					len2 = elem.column.coreDepth;
					len3 = elem.column.height;

					// �������� �����ϱ�
					EasyObjectPlacement qPlywood;
					qPlywood.init(L"��������(�ֽ������).gsm", attrib_new.layer.head.index, elem.header.floorInd, leftBottomX, leftBottomY, leftBottomZ, ang);
					moveIn3D('x', qPlywood.radAng, -len1 / 2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					moveIn3D('y', qPlywood.radAng, -len2 / 2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "��������",
						"m_type", APIParT_CString, "���(����)",
						"PANEL_MAT", APIParT_Mater, "20",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "��԰�",
						"m_size2", APIParT_CString, "���Ǹ���",
						"CONS_DR", APIParT_CString, "���� �����",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "��������",
						"m_type", APIParT_CString, "���(����)",
						"PANEL_MAT", APIParT_Mater, "20",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "��԰�",
						"m_size2", APIParT_CString, "���Ǹ���",
						"CONS_DR", APIParT_CString, "���� �����",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len2).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len1, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "��������",
						"m_type", APIParT_CString, "���(����)",
						"PANEL_MAT", APIParT_Mater, "20",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "��԰�",
						"m_size2", APIParT_CString, "���Ǹ���",
						"CONS_DR", APIParT_CString, "���� �����",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "��������",
						"m_type", APIParT_CString, "���(����)",
						"PANEL_MAT", APIParT_Mater, "20",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "��԰�",
						"m_size2", APIParT_CString, "���Ǹ���",
						"CONS_DR", APIParT_CString, "���� �����",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len2).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
				}

				// ���� ���
				for (int i = 0; i < nWalls; i++) {
					// ��� ���� ��������
					BNZeroMemory(&elem, sizeof(API_Element));
					BNZeroMemory(&memo, sizeof(API_ElementMemo));
					elem.header.guid = elemList_Wall.Pop();
					ACAPI_Element_Get(&elem);

					// �� ���� ��������
					workLevel_object = getWorkLevel(elem.header.floorInd);

					// ��ü�� ���� �� ũ�� ���� ��������
					leftBottomX = elem.wall.begC.x;
					leftBottomY = elem.wall.begC.y;
					leftBottomZ = elem.wall.bottomOffset;
					dx = elem.wall.endC.x - elem.wall.begC.x;
					dy = elem.wall.endC.y - elem.wall.begC.y;
					ang = atan2(dy, dx);
					len1 = GetDistance(elem.wall.begC, elem.wall.endC);
					len2 = elem.wall.thickness;
					len3 = elem.wall.height;

					// �������� �����ϱ�
					EasyObjectPlacement qPlywood;
					qPlywood.init(L"��������(�ֽ������).gsm", attrib_new.layer.head.index, elem.header.floorInd, leftBottomX, leftBottomY, leftBottomZ, ang);
					moveIn3D('y', qPlywood.radAng, -elem.wall.offsetFromOutside, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "��������",
						"m_type", APIParT_CString, "��ü(����)",
						"PANEL_MAT", APIParT_Mater, "75",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "��԰�",
						"m_size2", APIParT_CString, "���Ǹ���",
						"CONS_DR", APIParT_CString, "���� �����",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "��������",
						"m_type", APIParT_CString, "��ü(����)",
						"PANEL_MAT", APIParT_Mater, "75",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "��԰�",
						"m_size2", APIParT_CString, "���Ǹ���",
						"CONS_DR", APIParT_CString, "���� �����",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len2).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len1, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "��������",
						"m_type", APIParT_CString, "��ü(����)",
						"PANEL_MAT", APIParT_Mater, "75",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "��԰�",
						"m_size2", APIParT_CString, "���Ǹ���",
						"CONS_DR", APIParT_CString, "���� �����",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "��������",
						"m_type", APIParT_CString, "��ü(����)",
						"PANEL_MAT", APIParT_Mater, "75",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "��԰�",
						"m_size2", APIParT_CString, "���Ǹ���",
						"CONS_DR", APIParT_CString, "���� �����",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len2).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
				}

				// �������� ���
				for (int i = 0; i < nSlabs; i++) {
					// ��� ���� ��������
					BNZeroMemory(&elem, sizeof(API_Element));
					BNZeroMemory(&memo, sizeof(API_ElementMemo));
					elem.header.guid = elemList_Slab.Pop();
					ACAPI_Element_Get(&elem);
					ACAPI_Element_GetMemo(elem.header.guid, &memo);

					// �� ���� ��������
					workLevel_object = getWorkLevel(elem.header.floorInd);

					// ���� ���� X, Y ���� ã�Ƴ�
					leftBottomX = 0.0;
					leftBottomY = 0.0;
					rightTopX = 0.0;
					rightTopY = 0.0;
					for (int j = 1; j < elem.slab.poly.nCoords; j++) {
						if (j == 1) {
							leftBottomX = memo.coords[0][j].x;
							leftBottomY = memo.coords[0][j].y;
							rightTopX = memo.coords[0][j].x;
							rightTopY = memo.coords[0][j].y;
						}
						else {
							if (leftBottomX > memo.coords[0][j].x)	leftBottomX = memo.coords[0][j].x;
							if (leftBottomY > memo.coords[0][j].y)	leftBottomY = memo.coords[0][j].y;
							if (rightTopX < memo.coords[0][j].x)	rightTopX = memo.coords[0][j].x;
							if (rightTopY < memo.coords[0][j].y)	rightTopY = memo.coords[0][j].y;
						}
					}

					// ��ü�� ���� �� ũ�� ���� ��������
					leftBottomZ = elem.slab.level + elem.slab.offsetFromTop - elem.slab.thickness;
					ang = 0.0;
					len1 = rightTopX - leftBottomX;
					len2 = rightTopY - leftBottomY;
					len3 = elem.slab.thickness;

					// �������� �����ϱ�
					EasyObjectPlacement qPlywood;
					qPlywood.init(L"��������(�ֽ������).gsm", attrib_new.layer.head.index, elem.header.floorInd, leftBottomX, leftBottomY, leftBottomZ, ang);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "��������",
						"m_type", APIParT_CString, "�����(RC)",
						"PANEL_MAT", APIParT_Mater, "100",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "��԰�",
						"m_size2", APIParT_CString, "���Ǹ���",
						"CONS_DR", APIParT_CString, "�ٴڱ��",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len2).c_str());
				}
			}
			
			// ���̾� �����
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
		}
	}

	// ��� ���μ����� ��ġ�� ó���� �����ߴ� ���̴� ���̾���� �ٽ� �ѳ��� ��
	for (short xx = 1; xx <= nVisibleLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList[xx - 1];
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify(&attrib, NULL);
			}
		}
	}

	// �׷�ȭ �Ͻ����� OFF
	suspendGroups(false);

	return err;
}