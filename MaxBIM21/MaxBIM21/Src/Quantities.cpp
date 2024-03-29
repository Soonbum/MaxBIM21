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
static GS::Array<API_Guid>	elemList;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함

// 물량합판을 부착할 수 있는 팔레트를 띄움
GSErrCode	placeQuantityPlywood (void)
{
	GSErrCode	err = NoError;

	qElemInfo.dialogID = 0;

	if ((qElemInfo.dialogID == 0) || !DGIsDialogOpen (qElemInfo.dialogID)) {
		qElemInfo.dialogID = DGModelessInit (ACAPI_GetOwnResModule (), 32504, ACAPI_GetOwnResModule (), qElemDlgCallBack, (DGUserData) &qElemInfo, 1);
	}

	return	err;
}

// 팔레트에 대한 콜백 함수 1
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

	const GS::uchar_t*	gsmName = L("물량합판(핫스팟축소).gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				tempStr [256];
	double				horLen, verLen;
	bool				bValid;

	// 라인 입력 변수
	API_GetPointType	pointInfo;
	double				dx, dy, dz;

	API_Coord3D		p1, p2, p3, p4, p5;
	double			angle1, angle2;

	API_StoryInfo	storyInfo;
	double			storyLevel;
	char			floorName [256];

	
	err = ACAPI_CallUndoableCommand (L"물량합판 부착하기", [&] () -> GSErrCode {
		switch (message) {
			case DG_MSG_INIT:
				// 라벨
				DGSetItemText (dialID, LABEL_EXPLANATION_QELEM, L"3점 클릭: 직사각형, 5점 클릭: 창문형");

				// 레이어
				BNZeroMemory (&ucb, sizeof (ucb));
				ucb.dialogID = dialID;
				ucb.type	 = APIUserControlType_Layer;
				ucb.itemID	 = USERCONTROL_QPLYWOOD_LAYER;
				ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
				DGSetItemValLong (dialID, USERCONTROL_QPLYWOOD_LAYER, 1);

				// 팝업 컨트롤 (물량합판 타입)
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"보");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"기둥(독립)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"기둥(벽체)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"벽체(내벽)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"벽체(외벽)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"벽체(합벽)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"벽체(파라펫)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"벽체(방수턱)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"슬래브(기초)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"슬래브(RC)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"슬래브(데크)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"슬래브(램프)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, L"계단");

				// 층 정보 저장
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

				// 버튼
				DGSetItemText (dialID, BUTTON_DRAW_RECT_QELEM, L"물량합판 그리기\n(직사각형)");
				DGSetItemText (dialID, BUTTON_DRAW_WINDOW_QELEM, L"물량합판 그리기\n(창문형)");

				// 레이어 정보 저장
				qElemInfo.layerInd = (short)DGGetItemValLong (dialID, USERCONTROL_QPLYWOOD_LAYER);

				// 분류 정보 저장
				if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_BEAM) {
					strcpy (qElemInfo.m_type, "보");
					qElemInfo.panel_mat = 78;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_COLUMN_ISOLATION) {
					strcpy (qElemInfo.m_type, "기둥(독립)");
					qElemInfo.panel_mat = 20;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_COLUMN_IN_WALL) {
					strcpy (qElemInfo.m_type, "기둥(벽체)");
					qElemInfo.panel_mat = 77;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_INSIDE) {
					strcpy (qElemInfo.m_type, "벽체(내벽)");
					qElemInfo.panel_mat = 75;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_OUTSIDE) {
					strcpy (qElemInfo.m_type, "벽체(외벽)");
					qElemInfo.panel_mat = 76;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_DOUBLE) {
					strcpy (qElemInfo.m_type, "벽체(합벽)");
					qElemInfo.panel_mat = 72;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_PARAPET) {
					strcpy (qElemInfo.m_type, "벽체(파라펫)");
					qElemInfo.panel_mat = 32;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_WATERBLOCK) {
					strcpy (qElemInfo.m_type, "벽체(방수턱)");
					qElemInfo.panel_mat = 12;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_BASE) {
					strcpy (qElemInfo.m_type, "스라브(기초)");
					qElemInfo.panel_mat = 66;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_RC) {
					strcpy (qElemInfo.m_type, "스라브(RC)");
					qElemInfo.panel_mat = 100;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_DECK) {
					strcpy (qElemInfo.m_type, "스라브(데크)");
					qElemInfo.panel_mat = 99;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_RAMP) {
					strcpy (qElemInfo.m_type, "스라브(램프)");
					qElemInfo.panel_mat = 3;
				} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_STAIR) {
					strcpy (qElemInfo.m_type, "계단");
					qElemInfo.panel_mat = 73;
				}
 
				// 팔레트 창 등록
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
							strcpy (qElemInfo.m_type, "보");
							qElemInfo.panel_mat = 78;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_COLUMN_ISOLATION) {
							strcpy (qElemInfo.m_type, "기둥(독립)");
							qElemInfo.panel_mat = 20;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_COLUMN_IN_WALL) {
							strcpy (qElemInfo.m_type, "기둥(벽체)");
							qElemInfo.panel_mat = 77;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_INSIDE) {
							strcpy (qElemInfo.m_type, "벽체(내벽)");
							qElemInfo.panel_mat = 75;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_OUTSIDE) {
							strcpy (qElemInfo.m_type, "벽체(외벽)");
							qElemInfo.panel_mat = 76;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_DOUBLE) {
							strcpy (qElemInfo.m_type, "벽체(합벽)");
							qElemInfo.panel_mat = 72;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_PARAPET) {
							strcpy (qElemInfo.m_type, "벽체(파라펫)");
							qElemInfo.panel_mat = 32;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_WALL_WATERBLOCK) {
							strcpy (qElemInfo.m_type, "벽체(방수턱)");
							qElemInfo.panel_mat = 12;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_BASE) {
							strcpy (qElemInfo.m_type, "스라브(기초)");
							qElemInfo.panel_mat = 66;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_RC) {
							strcpy (qElemInfo.m_type, "스라브(RC)");
							qElemInfo.panel_mat = 100;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_DECK) {
							strcpy (qElemInfo.m_type, "스라브(데크)");
							qElemInfo.panel_mat = 99;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_SLAB_RAMP) {
							strcpy (qElemInfo.m_type, "스라브(램프)");
							qElemInfo.panel_mat = 3;
						} else if (DGPopUpGetSelected (dialID, POPUP_QPLYWOOD_TYPE) == TYPE_STAIR) {
							strcpy (qElemInfo.m_type, "계단");
							qElemInfo.panel_mat = 73;
						}

						break;

					case POPUP_FLOOR_QELEM:
						// 층 정보 저장
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

							CHCopyC (convertStr(GS::UniString(L"다각형의 1번째 노드(좌하단)를 클릭하십시오.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p1.x = pointInfo.pos.x;
							p1.y = pointInfo.pos.y;
							p1.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"다각형의 2번째 노드(우하단)를 클릭하십시오.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p2.x = pointInfo.pos.x;
							p2.y = pointInfo.pos.y;
							p2.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"다각형의 3번째 노드(우상단)를 클릭하십시오.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p3.x = pointInfo.pos.x;
							p3.y = pointInfo.pos.y;
							p3.z = pointInfo.pos.z;

							// 객체 로드
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

							// 고도 조정
							BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
							ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
							for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
								if (storyInfo.data [0][xx].index == qElemInfo.floorInd) {
									storyLevel = storyInfo.data [0][xx].level;
									break;
								}
							}
							BMKillHandle ((GSHandle *) &storyInfo.data);

							// 라이브러리의 파라미터 값 입력 (공통)
							elem.header.floorInd = qElemInfo.floorInd;
							elem.object.reflected = false;
							elem.object.libInd = libPart.index;
							elem.object.xRatio = aParam;
							elem.object.yRatio = bParam;
							elem.header.layer = qElemInfo.layerInd;
							elem.object.level = storyLevel;

							setParameterByName (&memo, "m_type", qElemInfo.m_type);
							setParameterByName (&memo, "PANEL_MAT", qElemInfo.panel_mat);

							// 합판두께 (문자열)
							strcpy (tempStr, "12");		// 12mm
							setParameterByName (&memo, "m_size", tempStr);

							// 품명
							strcpy (tempStr, "합판면적");
							setParameterByName (&memo, "m_size2", tempStr);

							dx = p2.x - p1.x;
							dy = p2.y - p1.y;

							elem.object.angle = atan2 (dy, dx);
							elem.object.pos.x = p1.x;
							elem.object.pos.y = p1.y;
							elem.object.level = p1.z;
							elem.object.level -= storyLevel;

							// 가로길이
							horLen = GetDistance (p1, p2);
							setParameterByName (&memo, "NO_WD", horLen);
							elem.object.xRatio = horLen;

							// 세로길이
							verLen = GetDistance (p2, p3);
							setParameterByName (&memo, "no_lg1", verLen);
							elem.object.yRatio = verLen;

							bValid = false;
							strcpy (tempStr, "벽에 세우기");	// 기본값

							// 설치위치
							strcpy (tempStr, "비규격");
							setParameterByName (&memo, "m_size1", tempStr);

							// p1, p2, p3 점의 고도가 모두 같을 때
							if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
								dx = p2.x - p1.x;
								dy = p2.y - p1.y;
								angle1 = RadToDegree (atan2 (dy, dx));

								dx = p3.x - p2.x;
								dy = p3.y - p2.y;
								angle2 = RadToDegree (atan2 (dy, dx));

								// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 큼
								if (abs (angle2 - angle1 - 90) < EPS) {
									strcpy (tempStr, "바닥덮기");
									bValid = true;
								}

								// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 작음
								if (abs (angle1 - angle2 - 90) < EPS) {
									strcpy (tempStr, "바닥깔기");
									moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
									bValid = true;
								}
							// p1, p2 점의 고도가 같고 p3 점의 고도가 다를 때
							} else {
								// p2와 p3의 x, y 좌표는 같음
								if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
									// ???
									strcpy (tempStr, "벽에 세우기");
									setParameterByName (&memo, "ZZYZX", verLen);
									bValid = true;
								
								// p2와 p3의 x, y 좌표는 다름
								} else {
									strcpy (tempStr, "경사설치");
									bValid = true;

									// 설치각도: asin ((p3.z - p2.z) / (p3와 p2 간의 거리))
									// 설치각도: acos ((p3와 p2 간의 평면 상의 거리) / (p3와 p2 간의 거리))
									// 설치각도: atan2 ((p3.z - p2.z) / (p3와 p2 간의 평면 상의 거리))
									dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
									dy = abs (p3.z - p2.z);
									dz = verLen;
									setParameterByName (&memo, "cons_ang", DegreeToRad (180.0) - acos (dx/dz));
								}
							}
							setParameterByName (&memo, "CONS_DR", tempStr);

							// 객체 배치
							if ((horLen > EPS) && (verLen > EPS) && (bValid == true)) {
								ACAPI_Element_Create (&elem, &memo);
							}

							ACAPI_DisposeElemMemoHdls (&memo);

						} while (inputErr != APIERR_CANCEL);

						break;

					case BUTTON_DRAW_WINDOW_QELEM:
						do {
							BNZeroMemory (&pointInfo, sizeof (API_GetPointType));

							CHCopyC (convertStr(GS::UniString(L"다각형의 1번째 노드(좌하단)를 클릭하십시오.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p1.x = pointInfo.pos.x;
							p1.y = pointInfo.pos.y;
							p1.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"다각형의 2번째 노드(우하단)를 클릭하십시오.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p2.x = pointInfo.pos.x;
							p2.y = pointInfo.pos.y;
							p2.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"다각형의 3번째 노드(우상단)를 클릭하십시오.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p3.x = pointInfo.pos.x;
							p3.y = pointInfo.pos.y;
							p3.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"다각형의 4번째 노드(개구부 좌하단)를 클릭하십시오.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p4.x = pointInfo.pos.x;
							p4.y = pointInfo.pos.y;
							p4.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"다각형의 5번째 노드(개구부 우상단)를 클릭하십시오.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p5.x = pointInfo.pos.x;
							p5.y = pointInfo.pos.y;
							p5.z = pointInfo.pos.z;

							// 객체 로드
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

							// 라이브러리의 파라미터 값 입력 (공통)
							elem.header.floorInd = qElemInfo.floorInd;
							elem.object.reflected = false;
							elem.object.libInd = libPart.index;
							elem.object.xRatio = aParam;
							elem.object.yRatio = bParam;
							elem.header.layer = qElemInfo.layerInd;

							setParameterByName (&memo, "m_type", qElemInfo.m_type);
							setParameterByName (&memo, "PANEL_MAT", qElemInfo.panel_mat);

							// 합판두께 (문자열)
							strcpy (tempStr, "12");		// 12mm
							setParameterByName (&memo, "m_size", tempStr);

							// 품명
							strcpy (tempStr, "합판면적");
							setParameterByName (&memo, "m_size2", tempStr);

							dx = p2.x - p1.x;
							dy = p2.y - p1.y;

							elem.object.angle = atan2 (dy, dx);
							elem.object.pos.x = p1.x;
							elem.object.pos.y = p1.y;
							elem.object.level = p1.z;
							elem.object.level -= storyLevel;

							// 가로길이
							horLen = GetDistance (p1, p2);
							setParameterByName (&memo, "NO_WD", horLen);
							elem.object.xRatio = horLen;

							// 세로길이
							verLen = GetDistance (p2, p3);
							setParameterByName (&memo, "no_lg1", verLen);
							elem.object.yRatio = verLen;

							bValid = false;
							strcpy (tempStr, "벽에 세우기");	// 기본값

							// 설치위치
							strcpy (tempStr, "창문형");
							setParameterByName (&memo, "m_size1", tempStr);

							// 설치방향
							API_Coord3D		origin;
							API_Coord3D		target_before, target_after;
								
							// p1, p2, p3 점의 고도가 모두 같을 때
							if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
								dx = p2.x - p1.x;
								dy = p2.y - p1.y;
								angle1 = RadToDegree (atan2 (dy, dx));

								dx = p3.x - p2.x;
								dy = p3.y - p2.y;
								angle2 = RadToDegree (atan2 (dy, dx));

								// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 큼
								if (abs (angle2 - angle1 - 90) < EPS) {
									origin = p1;
									target_before = p4;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw1", abs (origin.x - target_after.x));				// 폭1: vw1
									setParameterByName (&memo, "vh1", abs (origin.y - target_after.y));				// 높이1: vh1

									target_before = p5;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw2", abs (origin.x + horLen - target_after.x));	// 폭2: vw2
									setParameterByName (&memo, "vh2", abs (origin.y + verLen - target_after.y));	// 높이2: vh2

									strcpy (tempStr, "바닥덮기");
									bValid = true;
								}

								// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 작음
								if (abs (angle1 - angle2 - 90) < EPS) {
									origin = p1;
									target_before = p4;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw1", abs (origin.x - target_after.x));				// 폭1: vw1
									setParameterByName (&memo, "vh2", abs (origin.y - target_after.y));				// 높이2: vh2

									target_before = p5;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw2", abs (origin.x + horLen - target_after.x));	// 폭2: vw2
									setParameterByName (&memo, "vh1", verLen - abs (origin.y - target_after.y));	// 높이1: vh1

									strcpy (tempStr, "바닥깔기");
									moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
									bValid = true;
								}
								
							// p1, p2 점의 고도가 같고 p3 점의 고도가 다를 때
							} else {
								if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
									// p2와 p3의 x, y 좌표는 같음
									strcpy (tempStr, "벽에 세우기");
									setParameterByName (&memo, "ZZYZX", verLen);
									bValid = true;

									origin = p1;
									target_before = p4;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw1", target_after.x - origin.x);						// 폭1: vw1
									setParameterByName (&memo, "vh1", target_after.z - origin.z);						// 높이1: vh1

									target_before = p5;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw2", origin.x + horLen - target_after.x);				// 폭2: vw2
									setParameterByName (&memo, "vh2", origin.z + verLen - target_after.z);				// 높이2: vh2
								}
							}
							setParameterByName (&memo, "CONS_DR", tempStr);

							// 객체 배치
							if ((horLen > EPS) && (verLen > EPS) && (bValid == true)) {
								ACAPI_Element_Create (&elem, &memo);
							}

							ACAPI_DisposeElemMemoHdls (&memo);

						} while (inputErr != APIERR_CANCEL);

						break;

					case DG_CLOSEBOX:
						return item;	// 이것은 DG_MSG_CLOSE 메시지와 같음
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

// 팔레트에 대한 콜백 함수 2
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


// 단열재를 부착할 수 있는 팔레트를 띄움
GSErrCode	placeInsulation (void)
{
	GSErrCode	err = NoError;

	insulElemInfo.dialogID = 0;

	if ((insulElemInfo.dialogID == 0) || !DGIsDialogOpen (insulElemInfo.dialogID)) {
		insulElemInfo.dialogID = DGModelessInit (ACAPI_GetOwnResModule (), 32507, ACAPI_GetOwnResModule (), insulElemDlgCallBack, (DGUserData) &insulElemInfo, 1);
	}

	return	err;
}

// 팔레트에 대한 콜백 함수 1
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

	const GS::uchar_t*	gsmName = L("단열재v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	short				xx, yy;
	short				totalXX, totalYY;
	double				horLen, verLen;
	double				remainHorLen, remainVerLen;

	// 라인 입력 변수
	API_GetPointType	pointInfo;
	double				dx, dy, dz;

	// 점 3개의 위치와 선분의 각도
	API_Coord3D		p1, p2, p3;
	double			angle1, angle2;

	// 층 정보
	API_StoryInfo	storyInfo;
	double			storyLevel;
	char			floorName [256];

	
	err = ACAPI_CallUndoableCommand (L"단열재 부착하기", [&] () -> GSErrCode {
		switch (message) {
			case DG_MSG_INIT:
				// 라벨
				DGSetItemText (dialID, LABEL_EXPLANATION_INS, L"3점 클릭: 직사각형");
				DGSetItemText (dialID, LABEL_INSULATION_THK, L"두께");
				DGSetItemText (dialID, LABEL_INS_HORLEN, L"가로");
				DGSetItemText (dialID, LABEL_INS_VERLEN, L"세로");
				DGSetItemText (dialID, LABEL_FLOOR_INS, L"층 선택");

				// 체크박스
				DGSetItemText (dialID, CHECKBOX_INS_LIMIT_SIZE, L"가로/세로 크기 제한");
				DGSetItemValLong (dialID, CHECKBOX_INS_LIMIT_SIZE, TRUE);

				// Edit 컨트롤
				DGSetItemValDouble (dialID, EDITCONTROL_INSULATION_THK, 0.120);
				DGSetItemValDouble (dialID, EDITCONTROL_INS_HORLEN, 0.900);
				DGSetItemValDouble (dialID, EDITCONTROL_INS_VERLEN, 1.800);

				// 레이어
				BNZeroMemory (&ucb, sizeof (ucb));
				ucb.dialogID = dialID;
				ucb.type	 = APIUserControlType_Layer;
				ucb.itemID	 = USERCONTROL_INSULATION_LAYER;
				ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
				DGSetItemValLong (dialID, USERCONTROL_INSULATION_LAYER, 1);

				// 버튼
				DGSetItemText (dialID, BUTTON_DRAW_INSULATION, L"단열재 그리기");

				// 층 정보 저장
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

				// 레이어 정보 저장
				insulElemInfo.layerInd = (short)DGGetItemValLong (dialID, USERCONTROL_INSULATION_LAYER);

				// 두께, 가로, 세로 저장
				insulElemInfo.thk = DGGetItemValDouble (dialID, EDITCONTROL_INSULATION_THK);
				insulElemInfo.maxHorLen = DGGetItemValDouble (dialID, EDITCONTROL_INS_HORLEN);
				insulElemInfo.maxVerLen = DGGetItemValDouble (dialID, EDITCONTROL_INS_VERLEN);
				if (DGGetItemValLong (dialID, CHECKBOX_INS_LIMIT_SIZE) == TRUE)
					insulElemInfo.bLimitSize = true;
				else
					insulElemInfo.bLimitSize = false;
 
				// 팔레트 창 등록
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

				// 층 정보 저장
				BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
				ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
				insulElemInfo.floorInd = storyInfo.data [0][DGPopUpGetSelected (dialID, POPUP_FLOOR_INS) - 1].index;
				BMKillHandle ((GSHandle *) &storyInfo.data);

				// 레이어 정보 저장
				insulElemInfo.layerInd = (short)DGGetItemValLong (dialID, USERCONTROL_INSULATION_LAYER);

				// 두께, 가로, 세로 저장
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

							CHCopyC (convertStr(GS::UniString(L"다각형의 1번째 노드(좌하단)를 클릭하십시오.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p1.x = pointInfo.pos.x;
							p1.y = pointInfo.pos.y;
							p1.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"다각형의 2번째 노드(우하단)를 클릭하십시오.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p2.x = pointInfo.pos.x;
							p2.y = pointInfo.pos.y;
							p2.z = pointInfo.pos.z;

							CHCopyC (convertStr(GS::UniString(L"다각형의 3번째 노드(우상단)를 클릭하십시오.")), pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p3.x = pointInfo.pos.x;
							p3.y = pointInfo.pos.y;
							p3.z = pointInfo.pos.z;

							// 각도 계산
							dx = p2.x - p1.x;
							dy = p2.y - p1.y;
							angle1 = RadToDegree (atan2 (dy, dx));

							dx = p3.x - p2.x;
							dy = p3.y - p2.y;
							angle2 = RadToDegree (atan2 (dy, dx));

							// 객체 로드
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

							// 고도 조정
							BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
							ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
							for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
								if (storyInfo.data [0][xx].index == insulElemInfo.floorInd) {
									storyLevel = storyInfo.data [0][xx].level;
									break;
								}
							}
							BMKillHandle ((GSHandle *) &storyInfo.data);

							// 라이브러리의 파라미터 값 입력 (공통)
							elem.header.floorInd = insulElemInfo.floorInd;
							elem.object.libInd = libPart.index;
							elem.object.reflected = false;
							elem.object.xRatio = aParam;
							elem.object.yRatio = bParam;
							elem.header.layer = insulElemInfo.layerInd;

							setParameterByName (&memo, "bRestrictSize", insulElemInfo.bLimitSize);	// 크기 제한 여부
							setParameterByName (&memo, "maxHorLen", insulElemInfo.maxHorLen);		// 단열재 최대 가로 길이
							setParameterByName (&memo, "maxVerLen", insulElemInfo.maxVerLen);		// 단열재 최대 세로 길이
							setParameterByName (&memo, "ZZYZX", insulElemInfo.thk);					// 단열재 두께
							setParameterByName (&memo, "bLShape", 0.0);								// 단열재는 직사각형 형태로 사용하므로 L형상은 Off

							// 가로/세로 크기 제한이 true일 때
							if (insulElemInfo.bLimitSize == true) {
								// p1-p2가 p2-p3보다 길거나 같을 경우 (가로 방향)
								if (GetDistance (p1, p2) >= GetDistance (p2, p3)) {
									// 가로 크기 >= 세로 크기 (그대로 배치)
									if (insulElemInfo.maxHorLen >= insulElemInfo.maxVerLen) {
										// p1, p2, p3 점의 고도가 모두 같을 때
										if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
											// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 큼 (바닥면 부착)
											if (abs (angle2 - angle1 - 90) < EPS) {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
											// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 작음 (천장면 부착)
											if (abs (angle1 - angle2 - 90) < EPS) {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
										
										// p1, p2 점의 고도가 같고 p3 점의 고도가 다를 때
										} else {
											// p2와 p3의 x, y 좌표는 같음 (수직)
											if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
											
											// p2와 p3의 x, y 좌표는 다름 (경사)
											} else {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
									
									// 가로 크기 < 세로 크기 (눕혀서 배치)
									} else {
										// p1, p2, p3 점의 고도가 모두 같을 때
										if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
											// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 큼 (바닥면 부착)
											if (abs (angle2 - angle1 - 90) < EPS) {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
											// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 작음 (천장면 부착)
											if (abs (angle1 - angle2 - 90) < EPS) {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
										
										// p1, p2 점의 고도가 같고 p3 점의 고도가 다를 때
										} else {
											// p2와 p3의 x, y 좌표는 같음 (수직)
											if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
											
											// p2와 p3의 x, y 좌표는 다름 (경사)
											} else {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
								
								// p1-p2가 p2-p3보다 짧을 경우 (세로 방향)
								} else {
									// 가로 크기 >= 세로 크기 (눕혀서 배치)
									if (insulElemInfo.maxHorLen >= insulElemInfo.maxVerLen) {
										// p1, p2, p3 점의 고도가 모두 같을 때
										if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
											// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 큼 (바닥면 부착)
											if (abs (angle2 - angle1 - 90) < EPS) {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
											// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 작음 (천장면 부착)
											if (abs (angle1 - angle2 - 90) < EPS) {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
										
										// p1, p2 점의 고도가 같고 p3 점의 고도가 다를 때
										} else {
											// p2와 p3의 x, y 좌표는 같음 (수직)
											if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
											
											// p2와 p3의 x, y 좌표는 다름 (경사)
											} else {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
									
									// 가로 크기 < 세로 크기 (그대로 배치)
									} else {
										// p1, p2, p3 점의 고도가 모두 같을 때
										if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
											// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 큼 (바닥면 부착)
											if (abs (angle2 - angle1 - 90) < EPS) {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
											// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 작음 (천장면 부착)
											if (abs (angle1 - angle2 - 90) < EPS) {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
										
										// p1, p2 점의 고도가 같고 p3 점의 고도가 다를 때
										} else {
											// p2와 p3의 x, y 좌표는 같음 (수직)
											if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
											
											// p2와 p3의 x, y 좌표는 다름 (경사)
											} else {
												// 객체의 초기 위치 및 각도
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

														// 객체 배치
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
							
							// 가로/세로 크기 제한이 false일 때
							} else {
								// p1, p2, p3 점의 고도가 모두 같을 때
								if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
									// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 큼 (바닥면 부착)
									if (abs (angle2 - angle1 - 90) < EPS) {
										// 객체의 초기 위치 및 각도
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

										// 객체 배치
										if ((horLen > EPS) && (verLen > EPS)) {
											ACAPI_Element_Create (&elem, &memo);
											elemList.Push (elem.header.guid);
										}

										ACAPI_DisposeElemMemoHdls (&memo);
									}
									// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 작음 (천장면 부착)
									if (abs (angle1 - angle2 - 90) < EPS) {
										// 객체의 초기 위치 및 각도
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

										// 객체 배치
										if ((horLen > EPS) && (verLen > EPS)) {
											ACAPI_Element_Create (&elem, &memo);
											elemList.Push (elem.header.guid);
										}

										ACAPI_DisposeElemMemoHdls (&memo);
									}
										
								// p1, p2 점의 고도가 같고 p3 점의 고도가 다를 때
								} else {
									// p2와 p3의 x, y 좌표는 같음 (수직)
									if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
										// 객체의 초기 위치 및 각도
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

										// 객체 배치
										if ((horLen > EPS) && (verLen > EPS)) {
											ACAPI_Element_Create (&elem, &memo);
											elemList.Push (elem.header.guid);
										}

										ACAPI_DisposeElemMemoHdls (&memo);
											
									// p2와 p3의 x, y 좌표는 다름 (경사)
									} else {
										// 객체의 초기 위치 및 각도
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

										// 객체 배치
										if ((horLen > EPS) && (verLen > EPS)) {
											ACAPI_Element_Create (&elem, &memo);
											elemList.Push (elem.header.guid);
										}

										ACAPI_DisposeElemMemoHdls (&memo);
									}
								}
							}

							// 그룹화하기
							groupElements (elemList);
							elemList.Clear (false);

						} while (inputErr != APIERR_CANCEL);

						break;

					case DG_CLOSEBOX:
						return item;	// 이것은 DG_MSG_CLOSE 메시지와 같음
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

// 팔레트에 대한 콜백 함수 2
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

// 물량합판 자동 부착하기
GSErrCode	placeQuantityPlywoodAutomatic(void)
{
	GSErrCode	err = NoError;

	API_Element			elem;
	API_ElementMemo		memo;

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	API_AttributeDef  defs;
	API_Attribute	attrib_new;
	API_AttributeDef  defs_new;
	short			nVisibleLayers = 0;
	short			visLayerList[1024];
	char			fullLayerName[512];
	vector<LayerList>	layerList;

	// 작업 층 정보
	double			workLevel_object;		// 객체의 작업 층 높이

	// 보, 기둥, 벽, 슬래브 정보 저장
	GS::Array<API_Guid>	elemList_Beam;
	GS::Array<API_Guid>	elemList_Column;
	GS::Array<API_Guid>	elemList_Wall;
	GS::Array<API_Guid>	elemList_Slab;

	long nBeams;
	long nColumns;
	long nWalls;
	long nSlabs;

	// 요소의 위치, 크기 정보
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


	// 그룹화 일시정지 ON
	suspendGroups(true);

	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount();

	// 보이는 레이어들의 목록 저장하기
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

	// 레이어 이름과 인덱스 저장
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

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort(layerList.begin(), layerList.end(), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

	// 일시적으로 모든 레이어 숨기기
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

	// 보이는 레이어들을 하나씩 순회
	for (short mm = 1; mm <= nVisibleLayers; ++mm) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = layerList[mm - 1].layerInd;
		err = ACAPI_Attribute_Get(&attrib);

		// 초기화
		elemList_Beam.Clear();
		elemList_Column.Clear();
		elemList_Wall.Clear();
		elemList_Slab.Clear();

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify(&attrib, NULL);
			}

			// 보, 기둥, 벽, 슬래브 가져오기
			ACAPI_Element_GetElemList(API_BeamID, &elemList_Beam, APIFilt_OnVisLayer);		// 보이는 레이어에 있음
			ACAPI_Element_GetElemList(API_ColumnID, &elemList_Column, APIFilt_OnVisLayer);	// 보이는 레이어에 있음
			ACAPI_Element_GetElemList(API_WallID, &elemList_Wall, APIFilt_OnVisLayer);		// 보이는 레이어에 있음
			ACAPI_Element_GetElemList(API_SlabID, &elemList_Slab, APIFilt_OnVisLayer);		// 보이는 레이어에 있음

			nBeams = elemList_Beam.GetSize();
			nColumns = elemList_Column.GetSize();
			nWalls = elemList_Wall.GetSize();
			nSlabs = elemList_Slab.GetSize();

			// 보, 기둥, 벽, 슬래브 중 하나라도 있을 경우 레이어 생성하기
			if ((!elemList_Beam.IsEmpty() || !elemList_Column.IsEmpty() || !elemList_Wall.IsEmpty() || !elemList_Slab.IsEmpty()) == true) {
				// 레이어 이름 가져옴
				sprintf(fullLayerName, "%s", attrib.layer.head.name);
				fullLayerName[strlen(fullLayerName)] = '\0';
				GS::UniString fullLayerNameUniStr = (fullLayerName);
				GS::UniString newFullLayerNameUniStr;

				// 레이어 이름이 "01-S"로 시작할 경우, "07-Q"로 치환한 레이어 이름을 생성할 것 (그 외의 경우 "기존 레이어 이름 - 물량합판" 레이어 생성할 것)
				if (fullLayerNameUniStr.Contains("01-S") == TRUE) {
					fullLayerNameUniStr.ReplaceFirst("01-S", "07-Q");
				}
				else {
					fullLayerNameUniStr = fullLayerNameUniStr + L" - 물량합판";
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

				// 보의 경우
				for (int i = 0; i < nBeams; i++) {
					// 요소 정보 가져오기
					BNZeroMemory(&elem, sizeof(API_Element));
					BNZeroMemory(&memo, sizeof(API_ElementMemo));
					elem.header.guid = elemList_Beam.Pop();
					ACAPI_Element_Get(&elem);

					// 층 정보 가져오기
					workLevel_object = getWorkLevel(elem.header.floorInd);

					// 객체의 원점 및 크기 정보 가져오기
					leftBottomX = elem.beam.begC.x;
					leftBottomY = elem.beam.begC.y;
					leftBottomZ = elem.beam.level - elem.beam.height;
					dx = elem.beam.endC.x - elem.beam.begC.x;
					dy = elem.beam.endC.y - elem.beam.begC.y;
					ang = atan2(dy, dx);
					len1 = GetDistance(elem.beam.begC, elem.beam.endC);
					len2 = elem.beam.width;
					len3 = elem.beam.height;

					// 물량합판 부착하기
					EasyObjectPlacement qPlywood;
					qPlywood.init(L"물량합판(핫스팟축소).gsm", attrib_new.layer.head.index, elem.header.floorInd, leftBottomX, leftBottomY, leftBottomZ, ang);
					moveIn3D('y', qPlywood.radAng, -len2/2 + elem.beam.offset, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "물량합판",
						"m_type", APIParT_CString, "보",
						"PANEL_MAT", APIParT_Mater, "78",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "비규격",
						"m_size2", APIParT_CString, "합판면적",
						"CONS_DR", APIParT_CString, "바닥깔기",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len2).c_str());
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "물량합판",
						"m_type", APIParT_CString, "보",
						"PANEL_MAT", APIParT_Mater, "78",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "비규격",
						"m_size2", APIParT_CString, "합판면적",
						"CONS_DR", APIParT_CString, "벽에 세우기",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					moveIn3D('y', qPlywood.radAng, len1, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "물량합판",
						"m_type", APIParT_CString, "보",
						"PANEL_MAT", APIParT_Mater, "78",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "비규격",
						"m_size2", APIParT_CString, "합판면적",
						"CONS_DR", APIParT_CString, "벽에 세우기",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
				}

				// 기둥의 경우
				for (int i = 0; i < nColumns; i++) {
					// 요소 정보 가져오기
					BNZeroMemory(&elem, sizeof(API_Element));
					BNZeroMemory(&memo, sizeof(API_ElementMemo));
					elem.header.guid = elemList_Column.Pop();
					ACAPI_Element_Get(&elem);

					// 층 정보 가져오기
					workLevel_object = getWorkLevel(elem.header.floorInd);

					// 객체의 원점 및 크기 정보 가져오기
					leftBottomX = elem.column.origoPos.x;
					leftBottomY = elem.column.origoPos.y;
					leftBottomZ = elem.column.bottomOffset;
					ang = elem.column.angle + elem.column.slantDirectionAngle;
					len1 = elem.column.coreWidth;
					len2 = elem.column.coreDepth;
					len3 = elem.column.height;

					// 물량합판 부착하기
					EasyObjectPlacement qPlywood;
					qPlywood.init(L"물량합판(핫스팟축소).gsm", attrib_new.layer.head.index, elem.header.floorInd, leftBottomX, leftBottomY, leftBottomZ, ang);
					moveIn3D('x', qPlywood.radAng, -len1 / 2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					moveIn3D('y', qPlywood.radAng, -len2 / 2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "물량합판",
						"m_type", APIParT_CString, "기둥(독립)",
						"PANEL_MAT", APIParT_Mater, "20",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "비규격",
						"m_size2", APIParT_CString, "합판면적",
						"CONS_DR", APIParT_CString, "벽에 세우기",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "물량합판",
						"m_type", APIParT_CString, "기둥(독립)",
						"PANEL_MAT", APIParT_Mater, "20",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "비규격",
						"m_size2", APIParT_CString, "합판면적",
						"CONS_DR", APIParT_CString, "벽에 세우기",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len2).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len1, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "물량합판",
						"m_type", APIParT_CString, "기둥(독립)",
						"PANEL_MAT", APIParT_Mater, "20",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "비규격",
						"m_size2", APIParT_CString, "합판면적",
						"CONS_DR", APIParT_CString, "벽에 세우기",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "물량합판",
						"m_type", APIParT_CString, "기둥(독립)",
						"PANEL_MAT", APIParT_Mater, "20",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "비규격",
						"m_size2", APIParT_CString, "합판면적",
						"CONS_DR", APIParT_CString, "벽에 세우기",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len2).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
				}

				// 벽의 경우
				for (int i = 0; i < nWalls; i++) {
					// 요소 정보 가져오기
					BNZeroMemory(&elem, sizeof(API_Element));
					BNZeroMemory(&memo, sizeof(API_ElementMemo));
					elem.header.guid = elemList_Wall.Pop();
					ACAPI_Element_Get(&elem);

					// 층 정보 가져오기
					workLevel_object = getWorkLevel(elem.header.floorInd);

					// 객체의 원점 및 크기 정보 가져오기
					leftBottomX = elem.wall.begC.x;
					leftBottomY = elem.wall.begC.y;
					leftBottomZ = elem.wall.bottomOffset;
					dx = elem.wall.endC.x - elem.wall.begC.x;
					dy = elem.wall.endC.y - elem.wall.begC.y;
					ang = atan2(dy, dx);
					len1 = GetDistance(elem.wall.begC, elem.wall.endC);
					len2 = elem.wall.thickness;
					len3 = elem.wall.height;

					// 물량합판 부착하기
					EasyObjectPlacement qPlywood;
					qPlywood.init(L"물량합판(핫스팟축소).gsm", attrib_new.layer.head.index, elem.header.floorInd, leftBottomX, leftBottomY, leftBottomZ, ang);
					moveIn3D('y', qPlywood.radAng, -elem.wall.offsetFromOutside, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "물량합판",
						"m_type", APIParT_CString, "벽체(내벽)",
						"PANEL_MAT", APIParT_Mater, "75",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "비규격",
						"m_size2", APIParT_CString, "합판면적",
						"CONS_DR", APIParT_CString, "벽에 세우기",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "물량합판",
						"m_type", APIParT_CString, "벽체(내벽)",
						"PANEL_MAT", APIParT_Mater, "75",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "비규격",
						"m_size2", APIParT_CString, "합판면적",
						"CONS_DR", APIParT_CString, "벽에 세우기",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len2).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len1, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "물량합판",
						"m_type", APIParT_CString, "벽체(내벽)",
						"PANEL_MAT", APIParT_Mater, "75",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "비규격",
						"m_size2", APIParT_CString, "합판면적",
						"CONS_DR", APIParT_CString, "벽에 세우기",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
					moveIn3D('y', qPlywood.radAng, len2, &qPlywood.posX, &qPlywood.posY, &qPlywood.posZ);
					qPlywood.radAng -= DegreeToRad(90.0);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "물량합판",
						"m_type", APIParT_CString, "벽체(내벽)",
						"PANEL_MAT", APIParT_Mater, "75",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "비규격",
						"m_size2", APIParT_CString, "합판면적",
						"CONS_DR", APIParT_CString, "벽에 세우기",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len2).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len3).c_str());
				}

				// 슬래브의 경우
				for (int i = 0; i < nSlabs; i++) {
					// 요소 정보 가져오기
					BNZeroMemory(&elem, sizeof(API_Element));
					BNZeroMemory(&memo, sizeof(API_ElementMemo));
					elem.header.guid = elemList_Slab.Pop();
					ACAPI_Element_Get(&elem);
					ACAPI_Element_GetMemo(elem.header.guid, &memo);

					// 층 정보 가져오기
					workLevel_object = getWorkLevel(elem.header.floorInd);

					// 가장 작은 X, Y 점을 찾아냄
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

					// 객체의 원점 및 크기 정보 가져오기
					leftBottomZ = elem.slab.level + elem.slab.offsetFromTop - elem.slab.thickness;
					ang = 0.0;
					len1 = rightTopX - leftBottomX;
					len2 = rightTopY - leftBottomY;
					len3 = elem.slab.thickness;

					// 물량합판 부착하기
					EasyObjectPlacement qPlywood;
					qPlywood.init(L"물량합판(핫스팟축소).gsm", attrib_new.layer.head.index, elem.header.floorInd, leftBottomX, leftBottomY, leftBottomZ, ang);
					qPlywood.placeObject(10,
						"sup_type", APIParT_CString, "물량합판",
						"m_type", APIParT_CString, "스라브(RC)",
						"PANEL_MAT", APIParT_Mater, "100",
						"m_size", APIParT_CString, "12",
						"m_size1", APIParT_CString, "비규격",
						"m_size2", APIParT_CString, "합판면적",
						"CONS_DR", APIParT_CString, "바닥깔기",
						"mir", APIParT_Boolean, "0.0",
						"NO_WD", APIParT_Length, format_string("%f", len1).c_str(),
						"no_lg1", APIParT_Length, format_string("%f", len2).c_str());
				}
			}
			
			// 레이어 숨기기
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
		}
	}

	// 모든 프로세스를 마치면 처음에 수집했던 보이는 레이어들을 다시 켜놓을 것
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

	// 그룹화 일시정지 OFF
	suspendGroups(false);

	return err;
}