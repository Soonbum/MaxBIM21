#include <cstdio>
#include <cstdlib>
#include "MaxBIM21.h"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.h"
#include "BeamTableformPlacer.h"

namespace beamTableformPlacerDG {
	BeamTableformPlacingZone	placingZone;			// 기본 보 영역 정보
	InfoBeam					infoBeam;				// 보 객체 정보
	insulElemForBeamTableform	insulElem;				// 단열재 정보
	AutoArray					autoArrayInfo;			// 자동배치 정보
	API_Guid					structuralObject_forTableformBeam;	// 구조 객체의 GUID

	short	layerInd_Euroform;			// 레이어 번호: 유로폼
	short	layerInd_Plywood;			// 레이어 번호: 합판
	short	layerInd_Timber;			// 레이어 번호: 각재
	short	layerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
	short	layerInd_Fillerspacer;		// 레이어 번호: 휠러스페이서
	short	layerInd_Rectpipe;			// 레이어 번호: 비계파이프
	short	layerInd_RectpipeHanger;	// 레이어 번호: 각파이프행거
	short	layerInd_Pinbolt;			// 레이어 번호: 핀볼트
	short	layerInd_EuroformHook;		// 레이어 번호: 유로폼 후크
	short	layerInd_BlueClamp;			// 레이어 번호: 블루클램프
	short	layerInd_BlueTimberRail;	// 레이어 번호: 블루목심

	short	layerInd_VerticalPost;		// 레이어 번호: PERI동바리 수직재
	short	layerInd_HorizontalPost;	// 레이어 번호: PERI동바리 수평재
	short	layerInd_Girder;			// 레이어 번호: GT24 거더
	short	layerInd_BeamBracket;		// 레이어 번호: 보 브라켓
	short	layerInd_Yoke;				// 레이어 번호: 보 멍에제
	short	layerInd_JackSupport;		// 레이어 번호: 잭 서포트

	short	clickedBtnItemIdx;			// 그리드 버튼에서 클릭한 버튼의 인덱스 번호를 저장
	bool	clickedOKButton;			// OK 버튼을 눌렀습니까?
	bool	clickedPrevButton;			// 이전 버튼을 눌렀습니까?

	GS::Array<API_Guid>	elemList_LeftEnd;			// 그룹화 (좌측 끝)
	GS::Array<API_Guid>	elemList_RightEnd;			// 그룹화 (우측 끝)
	GS::Array<API_Guid>	elemList_Plywood[10];		// 그룹화 (합판 셀 및 잭 서포트)
	GS::Array<API_Guid>	elemList_Tableform[10];		// 그룹화 (테이블폼)
	GS::Array<API_Guid>	elemList_SupportingPost;	// 그룹화 (동바리/멍에제)
	GS::Array<API_Guid>	elemList_Insulation;		// 그룹화 (단열재)
};

using namespace beamTableformPlacerDG;

// 보에 테이블폼을 배치하는 통합 루틴
GSErrCode	placeTableformOnBeam (void)
{
	GSErrCode		err = NoError;
	short			xx, yy;
	double			dx, dy;
	short			result;

	GS::Array<API_Guid>		morphs;
	GS::Array<API_Guid>		beams;
	long					nMorphs = 0;
	long					nBeams = 0;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// 모프 3D 구성요소 가져오기
	GS::Array<API_Coord3D>	coords;
	long					nNodes;

	// 모프 객체 정보
	InfoMorphForBeamTableform	infoMorph [2];
	API_Coord3D					morph_p1 [2][2];	// 모프 2개의 윗쪽/아래쪽 시작점
	API_Coord3D					morph_p2 [2][2];	// 모프 2개의 윗쪽/아래쪽 끝점
	double						morph_height [2];	// 모프 2개의 높이

	// 포인트 임시 변수
	API_Coord3D	pT;

	// 작업 층 정보
	double					workLevel_beam;


	// 선택한 요소 가져오기 (보 1개, 모프 1~2개 선택해야 함)
	err = getGuidsOfSelection (&morphs, API_MorphID, &nMorphs);
	err = getGuidsOfSelection (&beams, API_BeamID, &nBeams);
	if (err == APIERR_NOPLAN) {
		DGAlert (DG_ERROR, L"오류", L"열린 프로젝트 창이 없습니다.", "", L"확인", "", "");
	}
	if (err == APIERR_NOSEL) {
		DGAlert (DG_ERROR, L"오류", L"아무 것도 선택하지 않았습니다.\n필수 선택: 보 (1개), 보 측면(전체/일부)을 덮는 모프 (1개)\n옵션 선택 (1): 보 반대쪽 측면을 덮는 모프 (1개)", "", L"확인", "", "");
	}

	// 보가 1개인가?
	if (nBeams != 1) {
		DGAlert (DG_ERROR, L"오류", L"보를 1개 선택해야 합니다.", "", L"확인", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1~2개인가?
	if ( !((nMorphs >= 1) && (nMorphs <= 2)) ) {
		DGAlert (DG_ERROR, L"오류", L"보 측면(전체/일부)을 덮는 모프를 1개 선택하셔야 합니다.\n덮는 높이가 비대칭이면 보 반대쪽 측면을 덮는 모프도 있어야 합니다.", "", L"확인", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// 보 정보 저장
	infoBeam.guid = beams.Pop ();

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = infoBeam.guid;
	structuralObject_forTableformBeam = elem.header.guid;
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);
	
	infoBeam.floorInd	= elem.header.floorInd;
	infoBeam.height		= elem.beam.height;		// elem.beamSegment.assemblySegmentData.nominalHeight;
	infoBeam.width		= elem.beam.width;		// elem.beamSegment.assemblySegmentData.nominalWidth;
	infoBeam.offset		= elem.beam.offset;
	infoBeam.level		= elem.beam.level;
	infoBeam.begC		= elem.beam.begC;
	infoBeam.endC		= elem.beam.endC;
	infoBeam.slantAngle	= elem.beam.slantAngle;

	ACAPI_DisposeElemMemoHdls (&memo);

	for (xx = 0 ; xx < nMorphs ; ++xx) {
		// 모프 정보를 가져옴
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = morphs.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

		// 모프의 정보 저장
		infoMorph [xx].guid		= elem.header.guid;
		infoMorph [xx].floorInd	= elem.header.floorInd;
		infoMorph [xx].level	= info3D.bounds.zMin;

		nNodes = getVerticesOfMorph (elem.header.guid, &coords);

		// 영역 모프의 시작점/끝점과 높이를 알아냄
		short ind = 0;
		morph_p1 [xx][0] = coords [0];		// 1번째 노드는 무조건 저장

		for (yy = 1 ; yy < nNodes ; ++yy) {
			// x, y 좌표 값이 같으면 p1에 저장하고, 다르면 p2에 저장
			if ( (abs (morph_p1 [xx][0].x - coords [yy].x) < EPS) && (abs (morph_p1 [xx][0].y - coords [yy].y) < EPS) )
				morph_p1 [xx][1] = coords [yy];
			else {
				if (ind < 2) {
					morph_p2 [xx][ind] = coords [yy];
					ind ++;
				}
			}
		}

		// 점 고도 조정
		// morph_p1, morph_p2의 [xx][0]: 낮은 점
		// morph_p1, morph_p2의 [xx][1]: 높은 점
		if (morph_p1 [xx][0].z > morph_p1 [xx][1].z) {
			pT = morph_p1 [xx][0];
			morph_p1 [xx][0] = morph_p1 [xx][1];
			morph_p1 [xx][1] = pT;
		}

		if (morph_p2 [xx][0].z > morph_p2 [xx][1].z) {
			pT = morph_p2 [xx][0];
			morph_p2 [xx][0] = morph_p2 [xx][1];
			morph_p2 [xx][1] = pT;
		}

		morph_height [xx] = abs (morph_p1 [xx][1].z - morph_p1 [xx][0].z);

		// 모프의 좌하단/우상단 점 가져오기
		if (abs (elem.morph.tranmat.tmx [11] - info3D.bounds.zMin) < EPS) {
			// 좌하단 좌표 결정
			infoMorph [xx].leftBottomX = elem.morph.tranmat.tmx [3];
			infoMorph [xx].leftBottomY = elem.morph.tranmat.tmx [7];
			infoMorph [xx].leftBottomZ = elem.morph.tranmat.tmx [11];

			// 우상단 좌표는?
			if (abs (infoMorph [xx].leftBottomX - info3D.bounds.xMin) < EPS)
				infoMorph [xx].rightTopX = info3D.bounds.xMax;
			else
				infoMorph [xx].rightTopX = info3D.bounds.xMin;
			if (abs (infoMorph [xx].leftBottomY - info3D.bounds.yMin) < EPS)
				infoMorph [xx].rightTopY = info3D.bounds.yMax;
			else
				infoMorph [xx].rightTopY = info3D.bounds.yMin;
			if (abs (infoMorph [xx].leftBottomZ - info3D.bounds.zMin) < EPS)
				infoMorph [xx].rightTopZ = info3D.bounds.zMax;
			else
				infoMorph [xx].rightTopZ = info3D.bounds.zMin;
		} else {
			// 우상단 좌표 결정
			infoMorph [xx].rightTopX = elem.morph.tranmat.tmx [3];
			infoMorph [xx].rightTopY = elem.morph.tranmat.tmx [7];
			infoMorph [xx].rightTopZ = elem.morph.tranmat.tmx [11];

			// 좌하단 좌표는?
			if (abs (infoMorph [xx].rightTopX - info3D.bounds.xMin) < EPS)
				infoMorph [xx].leftBottomX = info3D.bounds.xMax;
			else
				infoMorph [xx].leftBottomX = info3D.bounds.xMin;
			if (abs (infoMorph [xx].rightTopY - info3D.bounds.yMin) < EPS)
				infoMorph [xx].leftBottomY = info3D.bounds.yMax;
			else
				infoMorph [xx].leftBottomY = info3D.bounds.yMin;
			if (abs (infoMorph [xx].rightTopZ - info3D.bounds.zMin) < EPS)
				infoMorph [xx].leftBottomZ = info3D.bounds.zMax;
			else
				infoMorph [xx].leftBottomZ = info3D.bounds.zMin;
		}

		// 모프의 Z축 회전 각도 (벽의 설치 각도)
		dx = infoMorph [xx].rightTopX - infoMorph [xx].leftBottomX;
		dy = infoMorph [xx].rightTopY - infoMorph [xx].leftBottomY;
		infoMorph [xx].ang = RadToDegree (atan2 (dy, dx));

		// 모프의 좌하단 점과 가까운 점을 시작점, 먼 점을 끝점으로 지정함
		if ( GetDistance (infoMorph [xx].leftBottomX, infoMorph [xx].leftBottomY, infoMorph [xx].leftBottomZ, morph_p1 [xx][0].x, morph_p1 [xx][0].y, morph_p1 [xx][0].z)
		   > GetDistance (infoMorph [xx].leftBottomX, infoMorph [xx].leftBottomY, infoMorph [xx].leftBottomZ, morph_p2 [xx][0].x, morph_p2 [xx][0].y, morph_p2 [xx][0].z)) {

			   for (yy = 0 ; yy < 2 ; ++yy) {
				   pT = morph_p1 [xx][yy];
				   morph_p1 [xx][yy] = morph_p2 [xx][yy];
				   morph_p2 [xx][yy] = pT;
			   }
		}

		// 저장된 좌표값 버리기
		coords.Clear ();

		// 영역 모프 제거
		GS::Array<API_Element>	elems;
		elems.Push (elem);
		deleteElements (elems);
	}

	// 아래쪽 좌하단 좌표를 시작점, 아래쪽 끝점을 끝점으로 지정
	placingZone.begC = morph_p1 [0][0];
	placingZone.endC = morph_p2 [0][0];

	// 영역 회전 각도를 저장함
	placingZone.ang = DegreeToRad (infoMorph [0].ang);

	// 영역 높이를 저장함
	if (nMorphs == 2) {
		if (morph_height [0] > morph_height [1]) {
			placingZone.areaHeight_Left = morph_height [0];
			placingZone.areaHeight_Right = morph_height [1];
		} else {
			placingZone.areaHeight_Left = morph_height [1];
			placingZone.areaHeight_Right = morph_height [0];
		}
	} else {
		placingZone.areaHeight_Left = morph_height [0];
		placingZone.areaHeight_Right = morph_height [0];
	}

	// 배치 기준 시작점, 끝점 (영역 모프의 높이가 높은쪽이 기준이 됨)
	if (nMorphs == 2) {
		if (morph_height [0] > morph_height [1]) {
			placingZone.begC.x = morph_p1 [0][0].x;
			placingZone.begC.y = morph_p1 [0][0].y;
			placingZone.begC.z = morph_p1 [0][0].z;

			placingZone.endC.x = morph_p2 [0][0].x;
			placingZone.endC.y = morph_p2 [0][0].y;
			placingZone.endC.z = morph_p2 [0][0].z;
		} else {
			placingZone.begC.x = morph_p1 [1][0].x;
			placingZone.begC.y = morph_p1 [1][0].y;
			placingZone.begC.z = morph_p1 [1][0].z;

			placingZone.endC.x = morph_p2 [1][0].x;
			placingZone.endC.y = morph_p2 [1][0].y;
			placingZone.endC.z = morph_p2 [1][0].z;
		}
	} else {
		placingZone.begC.x = morph_p1 [0][0].x;
		placingZone.begC.y = morph_p1 [0][0].y;
		placingZone.begC.z = morph_p1 [0][0].z;

		placingZone.endC.x = morph_p2 [0][0].x;
		placingZone.endC.y = morph_p2 [0][0].y;
		placingZone.endC.z = morph_p2 [0][0].z;
	}

	// 보 길이
	API_Coord3D	p1 = (morph_p1 [0][0].z < morph_p1 [0][1].z) ? morph_p1 [0][0] : morph_p1 [0][1];
	API_Coord3D	p2 = (morph_p2 [0][0].z < morph_p2 [0][1].z) ? morph_p2 [0][0] : morph_p2 [0][1];
	placingZone.beamLength = GetDistance (p1, p2);

	// 보 너비
	placingZone.areaWidth_Bottom = infoBeam.width;

	// 보 오프셋
	placingZone.offset = infoBeam.offset;

	// 보 윗면 고도
	placingZone.level = infoBeam.level;

	// 경사 각도 저장
	placingZone.slantAngle = infoBeam.slantAngle;
	if (morph_p1 [0][0].z > morph_p2 [0][0].z)
		placingZone.slantAngle = -placingZone.slantAngle;

	// 영역 높이 보정
	placingZone.areaHeight_Left *= cos (placingZone.slantAngle);
	placingZone.areaHeight_Right *= cos (placingZone.slantAngle);

	// 작업 층 높이 반영
	workLevel_beam = getWorkLevel (infoBeam.floorInd);

	placingZone.begC.z -= workLevel_beam;
	placingZone.endC.z -= workLevel_beam;

FIRST:

	// placingZone의 Cell 정보 초기화
	placingZone.initCells (&placingZone);

	// [DIALOG] 1번째 다이얼로그에서 유로폼 정보 입력 받음
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32506, ACAPI_GetOwnResModule (), beamTableformPlacerHandler1, 0);

	if (result == DG_CANCEL)
		return err;

	// 총 길이에서 유로폼 높이 값으로 나누어서 대략적인 셀 개수(nCells) 초기 지정
	placingZone.nCells = (short)(floor (placingZone.beamLength / 1.200));

SECOND:

	// [DIALOG] 2번째 다이얼로그에서 유로폼 배치를 수정합니다.
	clickedOKButton = false;
	clickedPrevButton = false;
	result = DGBlankModalDialog (600, 360, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, beamTableformPlacerHandler2, 0);
	//
	// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
	if (clickedPrevButton == true)
		goto FIRST;

	// 2번째 다이얼로그에서 OK 버튼을 눌러야만 다음 단계로 넘어감
	if (clickedOKButton != true)
		return err;

	// 객체 위치 재조정
	placingZone.alignPlacingZone (&placingZone);

	// [DIALOG] 동바리/멍에제 프리셋을 배치할지 여부를 물어봄
	clickedOKButton = false;
	clickedPrevButton = false;
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32509, ACAPI_GetOwnResModule (), beamTableformPlacerHandler3, 0);

	// 이전 버튼을 누르면 2번째 다이얼로그 다시 실행
	if (clickedPrevButton == true)
		goto SECOND;

	// 기본 객체 배치하기
	err = placingZone.placeBasicObjects (&placingZone);

	// 부자재 배치하기
	if (placingZone.tableformType == 1)
		err = placingZone.placeAuxObjectsA (&placingZone);
	else if (placingZone.tableformType == 2)
		err = placingZone.placeAuxObjectsB (&placingZone);


	// 결과물 전체 그룹화
	groupElements (elemList_LeftEnd);
	groupElements (elemList_RightEnd);

	for (xx = 0 ; xx < 10 ; ++xx) {
		groupElements (elemList_Tableform [xx]);
		groupElements (elemList_Plywood [xx]);
	}

	// 3번째 다이얼로그에서 OK 버튼을 눌러야만 다음 단계로 넘어감
	if (clickedOKButton == true) {
		err = placingZone.placeSupportingPostPreset (&placingZone);		// 동바리/멍에제 프리셋을 배치함
		groupElements (elemList_SupportingPost);						// 동바리 그룹화
	}

	// 단열재 채우기
	std::wstring title;

	if (placingZone.gapSideLeft > EPS) {
		title = L"단열재 배치 (보 좌측면)";
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32512, ACAPI_GetOwnResModule (), beamTableformPlacerHandler4_Insulation, (DGUserData) &title);
		
		if (result == DG_OK)
			placingZone.placeInsulationsSide (&placingZone, &infoBeam, &insulElem, true);
	}

	if (placingZone.gapSideRight > EPS) {
		title = L"단열재 배치 (보 우측면)";
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32512, ACAPI_GetOwnResModule (), beamTableformPlacerHandler4_Insulation, (DGUserData) &title);
		
		if (result == DG_OK)
			placingZone.placeInsulationsSide (&placingZone, &infoBeam, &insulElem, true);
	}

	if (placingZone.gapBottom > EPS) {
		title = L"단열재 배치 (보 하부)";
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32512, ACAPI_GetOwnResModule (), beamTableformPlacerHandler4_Insulation, (DGUserData) &title);
		
		if (result == DG_OK)
			placingZone.placeInsulationsBottom (&placingZone, &infoBeam, &insulElem, false);
	}

	return	err;
}

// Cell 배열을 초기화함
void	BeamTableformPlacingZone::initCells (BeamTableformPlacingZone* placingZone)
{
	short xx, yy;

	// 보 측면/하부 요소 여부 초기화
	placingZone->bFillBottomFormAtLSide = false;
	placingZone->bFillFillerAtLSide = false;
	placingZone->bFillTopFormAtLSide = false;
	placingZone->bFillWoodAtLSide = false;

	placingZone->bFillBottomFormAtRSide = false;
	placingZone->bFillFillerAtRSide = false;
	placingZone->bFillTopFormAtRSide = false;
	placingZone->bFillWoodAtRSide = false;

	placingZone->bFillLeftFormAtBottom = false;
	placingZone->bFillFillerAtBottom = false;
	placingZone->bFillRightFormAtBottom = false;

	// 영역 정보의 여백 채움 여부 초기화
	placingZone->bFillMarginBegin = false;
	placingZone->bFillMarginEnd = false;

	// 영역 정보의 여백 설정 초기화
	placingZone->marginBegin = 0.0;
	placingZone->marginEnd = 0.0;

	// 보 양끝 셀
	placingZone->beginCellAtLSide.objType = PLYWOOD;
	placingZone->beginCellAtLSide.leftBottomX = 0.0;
	placingZone->beginCellAtLSide.leftBottomY = 0.0;
	placingZone->beginCellAtLSide.leftBottomZ = 0.0;
	placingZone->beginCellAtLSide.ang = 0.0;
	placingZone->beginCellAtLSide.dirLen = 0.0;
	placingZone->beginCellAtLSide.perLen = 0.0;

	placingZone->beginCellAtRSide.objType = PLYWOOD;
	placingZone->beginCellAtRSide.leftBottomX = 0.0;
	placingZone->beginCellAtRSide.leftBottomY = 0.0;
	placingZone->beginCellAtRSide.leftBottomZ = 0.0;
	placingZone->beginCellAtRSide.ang = 0.0;
	placingZone->beginCellAtRSide.dirLen = 0.0;
	placingZone->beginCellAtRSide.perLen = 0.0;

	placingZone->beginCellAtBottom.objType = PLYWOOD;
	placingZone->beginCellAtBottom.leftBottomX = 0.0;
	placingZone->beginCellAtBottom.leftBottomY = 0.0;
	placingZone->beginCellAtBottom.leftBottomZ = 0.0;
	placingZone->beginCellAtBottom.ang = 0.0;
	placingZone->beginCellAtBottom.dirLen = 0.0;
	placingZone->beginCellAtBottom.perLen = 0.0;

	placingZone->endCellAtLSide.objType = PLYWOOD;
	placingZone->endCellAtLSide.leftBottomX = 0.0;
	placingZone->endCellAtLSide.leftBottomY = 0.0;
	placingZone->endCellAtLSide.leftBottomZ = 0.0;
	placingZone->endCellAtLSide.ang = 0.0;
	placingZone->endCellAtLSide.dirLen = 0.0;
	placingZone->endCellAtLSide.perLen = 0.0;

	placingZone->endCellAtRSide.objType = PLYWOOD;
	placingZone->endCellAtRSide.leftBottomX = 0.0;
	placingZone->endCellAtRSide.leftBottomY = 0.0;
	placingZone->endCellAtRSide.leftBottomZ = 0.0;
	placingZone->endCellAtRSide.ang = 0.0;
	placingZone->endCellAtRSide.dirLen = 0.0;
	placingZone->endCellAtRSide.perLen = 0.0;

	placingZone->endCellAtBottom.objType = PLYWOOD;
	placingZone->endCellAtBottom.leftBottomX = 0.0;
	placingZone->endCellAtBottom.leftBottomY = 0.0;
	placingZone->endCellAtBottom.leftBottomZ = 0.0;
	placingZone->endCellAtBottom.ang = 0.0;
	placingZone->endCellAtBottom.dirLen = 0.0;
	placingZone->endCellAtBottom.perLen = 0.0;

	// 셀 개수 초기화
	placingZone->nCells = 0;

	// 셀 정보 초기화
	for (xx = 0 ; xx < 4 ; ++xx) {
		for (yy = 0 ; yy < 50 ; ++yy) {
			placingZone->cellsAtLSide [xx][yy].objType = NONE;
			placingZone->cellsAtLSide [xx][yy].leftBottomX = 0.0;
			placingZone->cellsAtLSide [xx][yy].leftBottomY = 0.0;
			placingZone->cellsAtLSide [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsAtLSide [xx][yy].ang = 0.0;
			placingZone->cellsAtLSide [xx][yy].dirLen = 0.0;
			placingZone->cellsAtLSide [xx][yy].perLen = 0.0;

			placingZone->cellsAtRSide [xx][yy].objType = NONE;
			placingZone->cellsAtRSide [xx][yy].leftBottomX = 0.0;
			placingZone->cellsAtRSide [xx][yy].leftBottomY = 0.0;
			placingZone->cellsAtRSide [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsAtRSide [xx][yy].ang = 0.0;
			placingZone->cellsAtRSide [xx][yy].dirLen = 0.0;
			placingZone->cellsAtRSide [xx][yy].perLen = 0.0;
		}
	}

	for (xx = 0 ; xx < 3 ; ++xx) {
		for (yy = 0 ; yy < 50 ; ++yy) {
			placingZone->cellsAtBottom [xx][yy].objType = NONE;
			placingZone->cellsAtBottom [xx][yy].leftBottomX = 0.0;
			placingZone->cellsAtBottom [xx][yy].leftBottomY = 0.0;
			placingZone->cellsAtBottom [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsAtBottom [xx][yy].ang = 0.0;
			placingZone->cellsAtBottom [xx][yy].dirLen = 0.0;
			placingZone->cellsAtBottom [xx][yy].perLen = 0.0;
		}
	}
}

// 셀(0-기반 인덱스 번호)의 좌하단 점 위치 X 좌표를 구함
double	BeamTableformPlacingZone::getCellPositionLeftBottomX (BeamTableformPlacingZone* placingZone, short idx)
{
	double	distance = (placingZone->bFillMarginBegin == true) ? placingZone->marginBegin : 0;

	for (short xx = 0 ; xx < idx ; ++xx)
		distance += placingZone->cellsAtLSide [0][xx].dirLen;

	return distance;
}

// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
void	BeamTableformPlacingZone::alignPlacingZone (BeamTableformPlacingZone* placingZone)
{
	short	xx;

	// 좌측 합판
	placingZone->beginCellAtLSide.ang = placingZone->ang;
	placingZone->beginCellAtLSide.leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
	placingZone->beginCellAtLSide.leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
	placingZone->beginCellAtLSide.leftBottomZ = placingZone->begC.z - placingZone->gapBottom;

	placingZone->beginCellAtRSide.ang = placingZone->ang;
	placingZone->beginCellAtRSide.leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
	placingZone->beginCellAtRSide.leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
	placingZone->beginCellAtRSide.leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
	moveIn3D ('y', placingZone->beginCellAtRSide.ang, placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight), &placingZone->beginCellAtRSide.leftBottomX, &placingZone->beginCellAtRSide.leftBottomY, &placingZone->beginCellAtRSide.leftBottomZ);

	placingZone->beginCellAtBottom.ang = placingZone->ang;
	placingZone->beginCellAtBottom.leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
	placingZone->beginCellAtBottom.leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
	placingZone->beginCellAtBottom.leftBottomZ = placingZone->begC.z - placingZone->gapBottom;

	// 우측 합판
	placingZone->endCellAtLSide.ang = placingZone->ang;
	placingZone->endCellAtLSide.leftBottomX = placingZone->endC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
	placingZone->endCellAtLSide.leftBottomY = placingZone->endC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
	placingZone->endCellAtLSide.leftBottomZ = placingZone->endC.z - placingZone->gapBottom;

	placingZone->endCellAtRSide.ang = placingZone->ang;
	placingZone->endCellAtRSide.leftBottomX = placingZone->endC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
	placingZone->endCellAtRSide.leftBottomY = placingZone->endC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
	placingZone->endCellAtRSide.leftBottomZ = placingZone->endC.z - placingZone->gapBottom;
	moveIn3D ('y', placingZone->endCellAtRSide.ang, placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight), &placingZone->endCellAtRSide.leftBottomX, &placingZone->endCellAtRSide.leftBottomY, &placingZone->endCellAtRSide.leftBottomZ);

	placingZone->endCellAtBottom.ang = placingZone->ang;
	placingZone->endCellAtBottom.leftBottomX = placingZone->endC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
	placingZone->endCellAtBottom.leftBottomY = placingZone->endC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
	placingZone->endCellAtBottom.leftBottomZ = placingZone->endC.z - placingZone->gapBottom;

	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		// 측면 (아래쪽 유로폼 라인)
		placingZone->cellsAtLSide [0][xx].ang = placingZone->ang;
		placingZone->cellsAtLSide [0][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
		placingZone->cellsAtLSide [0][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
		placingZone->cellsAtLSide [0][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3DSlope ('x', placingZone->cellsAtLSide [0][xx].ang, placingZone->slantAngle, placingZone->getCellPositionLeftBottomX (placingZone, xx), &placingZone->cellsAtLSide [0][xx].leftBottomX, &placingZone->cellsAtLSide [0][xx].leftBottomY, &placingZone->cellsAtLSide [0][xx].leftBottomZ);

		placingZone->cellsAtRSide [0][xx].ang = placingZone->ang;
		placingZone->cellsAtRSide [0][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
		placingZone->cellsAtRSide [0][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
		placingZone->cellsAtRSide [0][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3D ('y', placingZone->cellsAtRSide [0][xx].ang, placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight), &placingZone->cellsAtRSide [0][xx].leftBottomX, &placingZone->cellsAtRSide [0][xx].leftBottomY, &placingZone->cellsAtRSide [0][xx].leftBottomZ);
		moveIn3DSlope ('x', placingZone->cellsAtRSide [0][xx].ang, placingZone->slantAngle, placingZone->getCellPositionLeftBottomX (placingZone, xx), &placingZone->cellsAtRSide [0][xx].leftBottomX, &placingZone->cellsAtRSide [0][xx].leftBottomY, &placingZone->cellsAtRSide [0][xx].leftBottomZ);

		// 측면 (휠러스페이서 라인)
		placingZone->cellsAtLSide [1][xx].ang = placingZone->ang;
		placingZone->cellsAtLSide [1][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
		placingZone->cellsAtLSide [1][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
		placingZone->cellsAtLSide [1][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3D ('z', placingZone->cellsAtLSide [1][xx].ang, (placingZone->cellsAtLSide [0][xx].perLen) * cos (placingZone->slantAngle), &placingZone->cellsAtLSide [1][xx].leftBottomX, &placingZone->cellsAtLSide [1][xx].leftBottomY, &placingZone->cellsAtLSide [1][xx].leftBottomZ);
		moveIn3D ('x', placingZone->cellsAtLSide [1][xx].ang, -(placingZone->cellsAtLSide [0][xx].perLen) * sin (placingZone->slantAngle), &placingZone->cellsAtLSide [1][xx].leftBottomX, &placingZone->cellsAtLSide [1][xx].leftBottomY, &placingZone->cellsAtLSide [1][xx].leftBottomZ);
		moveIn3DSlope ('x', placingZone->cellsAtLSide [1][xx].ang, placingZone->slantAngle, placingZone->getCellPositionLeftBottomX (placingZone, xx), &placingZone->cellsAtLSide [1][xx].leftBottomX, &placingZone->cellsAtLSide [1][xx].leftBottomY, &placingZone->cellsAtLSide [1][xx].leftBottomZ);

		placingZone->cellsAtRSide [1][xx].ang = placingZone->ang;
		placingZone->cellsAtRSide [1][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
		placingZone->cellsAtRSide [1][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
		placingZone->cellsAtRSide [1][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3D ('y', placingZone->cellsAtRSide [1][xx].ang, placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight), &placingZone->cellsAtRSide [1][xx].leftBottomX, &placingZone->cellsAtRSide [1][xx].leftBottomY, &placingZone->cellsAtRSide [1][xx].leftBottomZ);
		moveIn3D ('z', placingZone->cellsAtRSide [1][xx].ang, (placingZone->cellsAtRSide [0][xx].perLen) * cos (placingZone->slantAngle), &placingZone->cellsAtRSide [1][xx].leftBottomX, &placingZone->cellsAtRSide [1][xx].leftBottomY, &placingZone->cellsAtRSide [1][xx].leftBottomZ);
		moveIn3D ('x', placingZone->cellsAtRSide [1][xx].ang, -(placingZone->cellsAtRSide [0][xx].perLen) * sin (placingZone->slantAngle), &placingZone->cellsAtRSide [1][xx].leftBottomX, &placingZone->cellsAtRSide [1][xx].leftBottomY, &placingZone->cellsAtRSide [1][xx].leftBottomZ);
		moveIn3DSlope ('x', placingZone->cellsAtRSide [1][xx].ang, placingZone->slantAngle, placingZone->getCellPositionLeftBottomX (placingZone, xx), &placingZone->cellsAtRSide [1][xx].leftBottomX, &placingZone->cellsAtRSide [1][xx].leftBottomY, &placingZone->cellsAtRSide [1][xx].leftBottomZ);

		// 측면 (위쪽 유로폼 라인)
		placingZone->cellsAtLSide [2][xx].ang = placingZone->ang;
		placingZone->cellsAtLSide [2][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
		placingZone->cellsAtLSide [2][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
		placingZone->cellsAtLSide [2][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3D ('z', placingZone->cellsAtLSide [2][xx].ang, (placingZone->cellsAtLSide [0][xx].perLen + placingZone->cellsAtLSide [1][xx].perLen) * cos (placingZone->slantAngle), &placingZone->cellsAtLSide [2][xx].leftBottomX, &placingZone->cellsAtLSide [2][xx].leftBottomY, &placingZone->cellsAtLSide [2][xx].leftBottomZ);
		moveIn3D ('x', placingZone->cellsAtLSide [2][xx].ang, -(placingZone->cellsAtLSide [0][xx].perLen + placingZone->cellsAtLSide [1][xx].perLen) * sin (placingZone->slantAngle), &placingZone->cellsAtLSide [2][xx].leftBottomX, &placingZone->cellsAtLSide [2][xx].leftBottomY, &placingZone->cellsAtLSide [2][xx].leftBottomZ);
		moveIn3DSlope ('x', placingZone->cellsAtLSide [2][xx].ang, placingZone->slantAngle, placingZone->getCellPositionLeftBottomX (placingZone, xx), &placingZone->cellsAtLSide [2][xx].leftBottomX, &placingZone->cellsAtLSide [2][xx].leftBottomY, &placingZone->cellsAtLSide [2][xx].leftBottomZ);

		placingZone->cellsAtRSide [2][xx].ang = placingZone->ang;
		placingZone->cellsAtRSide [2][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
		placingZone->cellsAtRSide [2][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
		placingZone->cellsAtRSide [2][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3D ('y', placingZone->cellsAtRSide [2][xx].ang, placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight), &placingZone->cellsAtRSide [2][xx].leftBottomX, &placingZone->cellsAtRSide [2][xx].leftBottomY, &placingZone->cellsAtRSide [2][xx].leftBottomZ);
		moveIn3D ('z', placingZone->cellsAtRSide [2][xx].ang, (placingZone->cellsAtRSide [0][xx].perLen + placingZone->cellsAtRSide [1][xx].perLen) * cos (placingZone->slantAngle), &placingZone->cellsAtRSide [2][xx].leftBottomX, &placingZone->cellsAtRSide [2][xx].leftBottomY, &placingZone->cellsAtRSide [2][xx].leftBottomZ);
		moveIn3D ('x', placingZone->cellsAtRSide [2][xx].ang, -(placingZone->cellsAtRSide [0][xx].perLen + placingZone->cellsAtRSide [1][xx].perLen) * sin (placingZone->slantAngle), &placingZone->cellsAtRSide [2][xx].leftBottomX, &placingZone->cellsAtRSide [2][xx].leftBottomY, &placingZone->cellsAtRSide [2][xx].leftBottomZ);
		moveIn3DSlope ('x', placingZone->cellsAtRSide [2][xx].ang, placingZone->slantAngle, placingZone->getCellPositionLeftBottomX (placingZone, xx), &placingZone->cellsAtRSide [2][xx].leftBottomX, &placingZone->cellsAtRSide [2][xx].leftBottomY, &placingZone->cellsAtRSide [2][xx].leftBottomZ);

		// 측면 (각재 라인)
		placingZone->cellsAtLSide [3][xx].ang = placingZone->ang;
		placingZone->cellsAtLSide [3][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
		placingZone->cellsAtLSide [3][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
		placingZone->cellsAtLSide [3][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3D ('z', placingZone->cellsAtLSide [3][xx].ang, (placingZone->cellsAtLSide [0][xx].perLen + placingZone->cellsAtLSide [1][xx].perLen + placingZone->cellsAtLSide [2][xx].perLen) * cos (placingZone->slantAngle), &placingZone->cellsAtLSide [3][xx].leftBottomX, &placingZone->cellsAtLSide [3][xx].leftBottomY, &placingZone->cellsAtLSide [3][xx].leftBottomZ);
		moveIn3D ('x', placingZone->cellsAtLSide [3][xx].ang, -(placingZone->cellsAtLSide [0][xx].perLen + placingZone->cellsAtLSide [1][xx].perLen + placingZone->cellsAtLSide [2][xx].perLen) * sin (placingZone->slantAngle), &placingZone->cellsAtLSide [3][xx].leftBottomX, &placingZone->cellsAtLSide [3][xx].leftBottomY, &placingZone->cellsAtLSide [3][xx].leftBottomZ);
		moveIn3DSlope ('x', placingZone->cellsAtLSide [3][xx].ang, placingZone->slantAngle, placingZone->getCellPositionLeftBottomX (placingZone, xx), &placingZone->cellsAtLSide [3][xx].leftBottomX, &placingZone->cellsAtLSide [3][xx].leftBottomY, &placingZone->cellsAtLSide [3][xx].leftBottomZ);

		placingZone->cellsAtRSide [3][xx].ang = placingZone->ang;
		placingZone->cellsAtRSide [3][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
		placingZone->cellsAtRSide [3][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
		placingZone->cellsAtRSide [3][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3D ('y', placingZone->cellsAtRSide [3][xx].ang, placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight), &placingZone->cellsAtRSide [3][xx].leftBottomX, &placingZone->cellsAtRSide [3][xx].leftBottomY, &placingZone->cellsAtRSide [3][xx].leftBottomZ);
		moveIn3D ('z', placingZone->cellsAtRSide [3][xx].ang, (placingZone->cellsAtRSide [0][xx].perLen + placingZone->cellsAtRSide [1][xx].perLen + placingZone->cellsAtRSide [2][xx].perLen) * cos (placingZone->slantAngle), &placingZone->cellsAtRSide [3][xx].leftBottomX, &placingZone->cellsAtRSide [3][xx].leftBottomY, &placingZone->cellsAtRSide [3][xx].leftBottomZ);
		moveIn3D ('x', placingZone->cellsAtRSide [3][xx].ang, -(placingZone->cellsAtRSide [0][xx].perLen + placingZone->cellsAtRSide [1][xx].perLen + placingZone->cellsAtRSide [2][xx].perLen) * sin (placingZone->slantAngle), &placingZone->cellsAtRSide [3][xx].leftBottomX, &placingZone->cellsAtRSide [3][xx].leftBottomY, &placingZone->cellsAtRSide [3][xx].leftBottomZ);
		moveIn3DSlope ('x', placingZone->cellsAtRSide [3][xx].ang, placingZone->slantAngle, placingZone->getCellPositionLeftBottomX (placingZone, xx), &placingZone->cellsAtRSide [3][xx].leftBottomX, &placingZone->cellsAtRSide [3][xx].leftBottomY, &placingZone->cellsAtRSide [3][xx].leftBottomZ);

		// 하부 (유로폼, 휠러, 유로폼)
		placingZone->cellsAtBottom [0][xx].ang = placingZone->ang;
		placingZone->cellsAtBottom [0][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
		placingZone->cellsAtBottom [0][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
		placingZone->cellsAtBottom [0][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3DSlope ('x', placingZone->cellsAtBottom [0][xx].ang, placingZone->slantAngle, placingZone->getCellPositionLeftBottomX (placingZone, xx), &placingZone->cellsAtBottom [0][xx].leftBottomX, &placingZone->cellsAtBottom [0][xx].leftBottomY, &placingZone->cellsAtBottom [0][xx].leftBottomZ);

		placingZone->cellsAtBottom [1][xx].ang = placingZone->ang;
		placingZone->cellsAtBottom [1][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
		placingZone->cellsAtBottom [1][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
		placingZone->cellsAtBottom [1][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3D ('y', placingZone->cellsAtBottom [1][xx].ang, placingZone->cellsAtBottom [0][xx].perLen, &placingZone->cellsAtBottom [1][xx].leftBottomX, &placingZone->cellsAtBottom [1][xx].leftBottomY, &placingZone->cellsAtBottom [1][xx].leftBottomZ);
		moveIn3DSlope ('x', placingZone->cellsAtBottom [1][xx].ang, placingZone->slantAngle, placingZone->getCellPositionLeftBottomX (placingZone, xx), &placingZone->cellsAtBottom [1][xx].leftBottomX, &placingZone->cellsAtBottom [1][xx].leftBottomY, &placingZone->cellsAtBottom [1][xx].leftBottomZ);

		placingZone->cellsAtBottom [2][xx].ang = placingZone->ang;
		placingZone->cellsAtBottom [2][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSideLeft * sin(placingZone->ang));
		placingZone->cellsAtBottom [2][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSideLeft * cos(placingZone->ang));
		placingZone->cellsAtBottom [2][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3D ('y', placingZone->cellsAtBottom [2][xx].ang, placingZone->cellsAtBottom [0][xx].perLen + placingZone->cellsAtBottom [1][xx].perLen, &placingZone->cellsAtBottom [2][xx].leftBottomX, &placingZone->cellsAtBottom [2][xx].leftBottomY, &placingZone->cellsAtBottom [2][xx].leftBottomZ);
		moveIn3DSlope ('x', placingZone->cellsAtBottom [2][xx].ang, placingZone->slantAngle, placingZone->getCellPositionLeftBottomX (placingZone, xx), &placingZone->cellsAtBottom [2][xx].leftBottomX, &placingZone->cellsAtBottom [2][xx].leftBottomY, &placingZone->cellsAtBottom [2][xx].leftBottomZ);
	}
}

// 유로폼/휠러/합판/각재를 배치함
GSErrCode	BeamTableformPlacingZone::placeBasicObjects (BeamTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx, yy;

	// 원장 사이즈 길이 계산
	bool	bShow;
	bool	bBeginFound;
	short	beginIndex, endIndex;
	double	remainLengthDouble;
	double	lengthDouble;

	// 각재 및 합판 배치에 사용됨
	double	horLen, verLen;
	bool	bTimberMove;
	double	moveZ;
	short	addedPlywood;

	// 회전 각도가 양수, 음수에 따라 파라미터에 전달할 경사 각도 변수
	double	slantAngle = (placingZone->slantAngle >= EPS) ? placingZone->slantAngle : DegreeToRad (360.0) + placingZone->slantAngle;
	double	minusSlantAngle = (placingZone->slantAngle >= EPS) ? DegreeToRad (360.0) - placingZone->slantAngle : -placingZone->slantAngle;

	EasyObjectPlacement euroform, fillersp, plywood, timber;
	EasyObjectPlacement plywood1, plywood2, plywood3;

	if (placingZone->bFillMarginBegin == true) {
		// 시작 부분 측면(L) 합판 배치
		if ((placingZone->bFillBottomFormAtLSide == true) && (placingZone->beginCellAtLSide.dirLen > EPS)) {
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
			moveIn3D ('z', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, 0.0615 * sin (placingZone->slantAngle), &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList_LeftEnd.Push (plywood.placeObject (14,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "벽눕히기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->beginCellAtLSide.perLen + 0.0615 - placingZone->hRest_Left).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->beginCellAtLSide.dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.070).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}

		// 시작 부분 측면(R) 합판 배치
		if ((placingZone->bFillBottomFormAtRSide == true) && (placingZone->beginCellAtRSide.dirLen > EPS)) {
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->beginCellAtRSide.leftBottomX, placingZone->beginCellAtRSide.leftBottomY, placingZone->beginCellAtRSide.leftBottomZ, placingZone->beginCellAtRSide.ang);
			moveIn3D ('z', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, 0.0615 * sin (placingZone->slantAngle), &plywood.posX, &plywood.posY, &plywood.posZ);
			plywood.radAng += DegreeToRad (180.0);
			elemList_LeftEnd.Push (plywood.placeObjectMirrored (14,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "벽눕히기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->beginCellAtRSide.perLen + 0.0615 - placingZone->hRest_Right).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->beginCellAtRSide.dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.070).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}

		// 시작 부분 하부 합판 배치
		if (placingZone->beginCellAtBottom.dirLen > EPS) {
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->beginCellAtBottom.leftBottomX, placingZone->beginCellAtBottom.leftBottomY, placingZone->beginCellAtBottom.leftBottomZ, placingZone->beginCellAtBottom.ang);
			elemList_LeftEnd.Push (plywood.placeObject (14,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "바닥깔기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->beginCellAtBottom.perLen).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->beginCellAtBottom.dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.070).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}
	}

	if (placingZone->bFillMarginEnd == true) {
		// 끝 부분 측면(L) 합판 배치
		if ((placingZone->bFillBottomFormAtLSide == true) && (placingZone->endCellAtLSide.dirLen > EPS)) {
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->endCellAtLSide.leftBottomX, placingZone->endCellAtLSide.leftBottomY, placingZone->endCellAtLSide.leftBottomZ, placingZone->endCellAtLSide.ang);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, -placingZone->endCellAtLSide.dirLen, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3D ('z', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, 0.0615 * sin (placingZone->slantAngle), &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList_RightEnd.Push (plywood.placeObject (14,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "벽눕히기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->endCellAtLSide.perLen + 0.0615 - placingZone->hRest_Left).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->endCellAtLSide.dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.070).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}

		// 끝 부분 측면(R) 합판 배치
		if ((placingZone->bFillBottomFormAtRSide == true) && (placingZone->endCellAtRSide.dirLen > EPS)) {
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->endCellAtRSide.leftBottomX, placingZone->endCellAtRSide.leftBottomY, placingZone->endCellAtRSide.leftBottomZ, placingZone->endCellAtRSide.ang);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, -placingZone->endCellAtRSide.dirLen, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3D ('z', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, 0.0615 * sin (placingZone->slantAngle), &plywood.posX, &plywood.posY, &plywood.posZ);
			plywood.radAng += DegreeToRad (180.0);
			elemList_RightEnd.Push (plywood.placeObjectMirrored (14,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "벽눕히기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->endCellAtRSide.perLen + 0.0615 - placingZone->hRest_Right).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->endCellAtRSide.dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.070).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}

		// 끝 부분 하부 합판 배치
		if (placingZone->endCellAtBottom.dirLen > EPS) {
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->endCellAtBottom.leftBottomX, placingZone->endCellAtBottom.leftBottomY, placingZone->endCellAtBottom.leftBottomZ, placingZone->endCellAtBottom.ang);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, -placingZone->endCellAtBottom.dirLen, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList_RightEnd.Push (plywood.placeObject (14,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "바닥깔기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->endCellAtBottom.perLen).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->endCellAtBottom.dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.070).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}
	}

	// 측면(L) 유로폼 1단 배치
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);

		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > EPS) && (placingZone->cellsAtLSide [0][xx].dirLen > EPS)) {
			moveIn3DSlope ('x', euroform.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);

			double dirLen = placingZone->cellsAtLSide[0][xx].dirLen;
			if ((abs(dirLen - 1.200) < EPS) || (abs(dirLen - 0.900) < EPS) || (abs(dirLen - 0.600) < EPS)) {
				elemList_Tableform[getAreaSeqNumOfCell(placingZone, true, true, xx)].Push(euroform.placeObject(6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string("%.0f", placingZone->cellsAtLSide[0][xx].perLen * 1000.0).c_str(),
					"eu_hei", APIParT_CString, format_string("%.0f", placingZone->cellsAtLSide[0][xx].dirLen * 1000.0).c_str(),
					"u_ins", APIParT_CString, "벽눕히기",
					"ang_x", APIParT_Angle, format_string("%f", DegreeToRad(90.0)).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str()));
			}
			else {
				elemList_Tableform[getAreaSeqNumOfCell(placingZone, true, true, xx)].Push(euroform.placeObject(6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string("%f", placingZone->cellsAtLSide[0][xx].perLen).c_str(),
					"eu_hei2", APIParT_Length, format_string("%f", placingZone->cellsAtLSide[0][xx].dirLen).c_str(),
					"u_ins", APIParT_CString, "벽눕히기",
					"ang_x", APIParT_Angle, format_string("%f", DegreeToRad(90.0)).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str()));
			}
		}
	}

	// 측면(L) 휠러스페이서 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [1][xx].objType == FILLERSP) && (placingZone->cellsAtLSide [1][xx].perLen > 0) && (placingZone->cellsAtLSide [1][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [1][xx].objType != FILLERSP) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [1][yy].dirLen;

			fillersp.init (L("휠러스페이서v1.0.gsm"), layerInd_Fillerspacer, infoBeam.floorInd, placingZone->cellsAtLSide [1][beginIndex].leftBottomX, placingZone->cellsAtLSide [1][beginIndex].leftBottomY, placingZone->cellsAtLSide [1][beginIndex].leftBottomZ, placingZone->cellsAtLSide [1][beginIndex].ang);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 2.400)
					lengthDouble = 2.400;
				else
					lengthDouble = remainLengthDouble;

				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (fillersp.placeObject (4,
					"f_thk", APIParT_Length, format_string ("%f", placingZone->cellsAtLSide [1][beginIndex].perLen).c_str(),
					"f_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(),
					"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + placingZone->slantAngle).c_str(),
					"f_rota", APIParT_Angle, format_string ("%f", 0.0).c_str()));
				moveIn3DSlope ('x', fillersp.radAng, placingZone->slantAngle, lengthDouble, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

				remainLengthDouble -= 2.400;
			}

			bBeginFound = false;
		}
	}

	// 측면(L) 유로폼 2단 배치
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtLSide [2][xx].leftBottomX, placingZone->cellsAtLSide [2][xx].leftBottomY, placingZone->cellsAtLSide [2][xx].leftBottomZ, placingZone->cellsAtLSide [2][xx].ang);

		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > EPS) && (placingZone->cellsAtLSide [2][xx].dirLen > EPS)) {
			moveIn3DSlope ('x', euroform.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [2][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);

			double dirLen = placingZone->cellsAtLSide[2][xx].dirLen;
			if ((abs(dirLen - 1.200) < EPS) || (abs(dirLen - 0.900) < EPS) || (abs(dirLen - 0.600) < EPS)) {
				elemList_Tableform[getAreaSeqNumOfCell(placingZone, true, true, xx)].Push(euroform.placeObject(6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string("%.0f", placingZone->cellsAtLSide[2][xx].perLen * 1000.0).c_str(),
					"eu_hei", APIParT_CString, format_string("%.0f", placingZone->cellsAtLSide[2][xx].dirLen * 1000.0).c_str(),
					"u_ins", APIParT_CString, "벽눕히기",
					"ang_x", APIParT_Angle, format_string("%f", DegreeToRad(90.0)).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str()));
			}
			else {
				elemList_Tableform[getAreaSeqNumOfCell(placingZone, true, true, xx)].Push(euroform.placeObject(6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string("%f", placingZone->cellsAtLSide[2][xx].perLen).c_str(),
					"eu_hei2", APIParT_Length, format_string("%f", placingZone->cellsAtLSide[2][xx].dirLen).c_str(),
					"u_ins", APIParT_CString, "벽눕히기",
					"ang_x", APIParT_Angle, format_string("%f", DegreeToRad(90.0)).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str()));
			}
		}
	}

	// 측면(L) 각재/합판 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if (getObjectType (placingZone, true, 3) == TIMBER) {
			if ((placingZone->cellsAtLSide [3][xx].objType == TIMBER) && (placingZone->cellsAtLSide [3][xx].perLen > 0) && (placingZone->cellsAtLSide [3][xx].dirLen > 0)) {
				// 연속적인 인덱스 범위 찾기
				if (bBeginFound == false) {
					beginIndex = xx;
					bBeginFound = true;
					bShow = true;

					bTimberMove = false;
					moveZ = 0.0;
					addedPlywood = 0;
					if (placingZone->cellsAtLSide [3][xx].perLen < 0.040 - EPS) {
						// 40mm 미만이면 앞쪽으로(!) 50*80 각재
						horLen = 0.050;
						verLen = 0.080;
						bTimberMove = true;

						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.010) < EPS)	addedPlywood = 1;	// 10mm 이면 합판 1장 얹음
						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.020) < EPS)	addedPlywood = 2;	// 20mm 이면 합판 2장 얹음
						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.030) < EPS)	addedPlywood = 3;	// 30mm 이면 합판 3장 얹음
					} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.050 - EPS)) {
						// 40mm 이상 50mm 미만이면, 50*40 각재
						horLen = 0.050;
						verLen = 0.040;
					} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.080 - EPS)) {
						// 50mm 이상 80mm 미만이면, 80*50 각재
						horLen = 0.080;
						verLen = 0.050;
						moveZ = verLen;

						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.060) < EPS)	addedPlywood = 1;	// 60mm 이면 합판 1장 얹음
						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.070) < EPS)	addedPlywood = 2;	// 70mm 이면 합판 2장 얹음
					} else {
						// 80mm 이상 90mm 미만이면, 80*80 각재
						horLen = 0.080;
						verLen = 0.080;
						moveZ = verLen;

						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.090) < EPS)	addedPlywood = 1;	// 90mm 이면 합판 1장 얹음
						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.100) < EPS)	addedPlywood = 2;	// 100mm 이면 합판 2장 얹음
					}
				}
				endIndex = xx;
			}

			if (((placingZone->cellsAtLSide [3][xx].objType != TIMBER) || (xx == placingZone->nCells-1)) && (bShow == true)) {
				// 원장 사이즈 단위로 끊어서 배치하기 (각재)
				remainLengthDouble = 0.0;
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtLSide [3][yy].dirLen;

				timber.init (L("목재v1.0.gsm"), layerInd_Timber, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ, placingZone->cellsAtLSide [3][beginIndex].ang);

				while (remainLengthDouble > EPS) {
					if (remainLengthDouble > 3.600)
						lengthDouble = 3.600;
					else
						lengthDouble = remainLengthDouble;

					// 각재 설치
					if (bTimberMove == true) {
						moveIn3D ('y', timber.radAng, -0.067, &timber.posX, &timber.posY, &timber.posZ);
						moveIn3D ('z', timber.radAng, -0.080 * cos (placingZone->slantAngle), &timber.posX, &timber.posY, &timber.posZ);
						moveIn3D ('x', timber.radAng, 0.080 * sin (placingZone->slantAngle), &timber.posX, &timber.posY, &timber.posZ);
					}
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (timber.placeObject (6,
						"w_ins", APIParT_CString, "벽세우기",
						"w_w", APIParT_Length, format_string ("%f", horLen).c_str(),
						"w_h", APIParT_Length, format_string ("%f", verLen).c_str(),
						"w_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(),
						"w_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(),
						"torsion_ang", APIParT_Angle, format_string ("%f", 0.0).c_str()));
					if (bTimberMove == true) {
						moveIn3D ('y', timber.radAng, 0.067, &timber.posX, &timber.posY, &timber.posZ);
						moveIn3D ('z', timber.radAng, 0.080 * cos (placingZone->slantAngle), &timber.posX, &timber.posY, &timber.posZ);
						moveIn3D ('x', timber.radAng, -0.080 * sin (placingZone->slantAngle), &timber.posX, &timber.posY, &timber.posZ);
					}
					moveIn3DSlope ('x', timber.radAng, placingZone->slantAngle, lengthDouble, &timber.posX, &timber.posY, &timber.posZ);

					remainLengthDouble -= 3.600;
				}

				// 원장 사이즈 단위로 끊어서 배치하기 (추가 합판)
				remainLengthDouble = 0.0;
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtLSide [3][yy].dirLen;

				plywood1.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ, placingZone->cellsAtLSide [3][beginIndex].ang);
				plywood2.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ, placingZone->cellsAtLSide [3][beginIndex].ang);
				plywood3.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ, placingZone->cellsAtLSide [3][beginIndex].ang);

				while (remainLengthDouble > EPS) {
					if (remainLengthDouble > 2.400)
						lengthDouble = 2.400;
					else
						lengthDouble = remainLengthDouble;
						
					if (addedPlywood >= 1) {
						moveIn3D ('y', plywood1.radAng, -0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3D ('z', plywood1.radAng, moveZ * cos (placingZone->slantAngle), &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3D ('x', plywood1.radAng, -moveZ * sin (placingZone->slantAngle), &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood1.placeObject (8, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070).c_str(), "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(), "p_rot", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "sogak", APIParT_Boolean, "0.0"));
						moveIn3D ('y', plywood1.radAng, 0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3D ('z', plywood1.radAng, -moveZ * cos (placingZone->slantAngle), &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3D ('x', plywood1.radAng, moveZ * sin (placingZone->slantAngle), &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3DSlope ('x', plywood1.radAng, placingZone->slantAngle, lengthDouble, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
					}
					if (addedPlywood >= 2) {
						moveIn3D ('y', plywood2.radAng, -0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3D ('z', plywood2.radAng, (moveZ + 0.0115) * cos (placingZone->slantAngle), &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3D ('x', plywood2.radAng, -(moveZ + 0.0115) * sin (placingZone->slantAngle), &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood2.placeObject (8, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070).c_str(), "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(), "p_rot", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "sogak", APIParT_Boolean, "0.0"));
						moveIn3D ('y', plywood2.radAng, 0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3D ('z', plywood2.radAng, -(moveZ + 0.0115) * cos (placingZone->slantAngle), &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3D ('x', plywood2.radAng, (moveZ + 0.0115) * sin (placingZone->slantAngle), &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3DSlope ('x', plywood2.radAng, placingZone->slantAngle, lengthDouble, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
					}
					if (addedPlywood >= 3) {
						moveIn3D ('y', plywood3.radAng, -0.070, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						moveIn3D ('z', plywood3.radAng, (moveZ + 0.0115*2) * cos (placingZone->slantAngle), &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						moveIn3D ('x', plywood3.radAng, -(moveZ + 0.0115*2) * sin (placingZone->slantAngle), &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood3.placeObject (8, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070).c_str(), "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(), "p_rot", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "sogak", APIParT_Boolean, "0.0"));
						moveIn3D ('y', plywood3.radAng, 0.070, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						moveIn3D ('z', plywood3.radAng, -(moveZ + 0.0115*2) * cos (placingZone->slantAngle), &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						moveIn3D ('x', plywood3.radAng, (moveZ + 0.0115*2) * sin (placingZone->slantAngle), &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						moveIn3DSlope ('x', plywood3.radAng, placingZone->slantAngle, lengthDouble, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
					}

					remainLengthDouble -= 2.400;
				}

				bBeginFound = false;
			}
		} else if (getObjectType (placingZone, true, 3) == PLYWOOD) {
			// 100mm 초과이면
			if ((placingZone->cellsAtLSide [3][xx].objType == PLYWOOD) && (placingZone->cellsAtLSide [3][xx].perLen > 0) && (placingZone->cellsAtLSide [3][xx].dirLen > 0)) {
				// 연속적인 인덱스 범위 찾기
				if (bBeginFound == false) {
					beginIndex = xx;
					bBeginFound = true;
					bShow = true;
				}
				endIndex = xx;
			}

			if (((placingZone->cellsAtLSide [3][xx].objType != PLYWOOD) || (xx == placingZone->nCells-1)) && (bShow == true)) {
				// 원장 사이즈 단위로 끊어서 배치하기 (합판)
				remainLengthDouble = 0.0;
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtLSide [3][yy].dirLen;

				plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ, placingZone->cellsAtLSide [3][beginIndex].ang);

				while (remainLengthDouble > EPS) {
					if (remainLengthDouble > 2.400)
						lengthDouble = 2.400;
					else
						lengthDouble = remainLengthDouble;

					// 합판 설치
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood.placeObject (14,
						"p_stan", APIParT_CString, "비규격",
						"w_dir", APIParT_CString, "벽눕히기",
						"p_thk", APIParT_CString, "11.5T",
						"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtLSide [3][beginIndex].perLen).c_str(),
						"p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(),
						"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
						"sogak", APIParT_Boolean, "1.0",
						"bInverseSogak", APIParT_Boolean, "1.0",
						"prof", APIParT_CString, "소각",
						"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
						"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
						"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
						"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
					moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, lengthDouble, &plywood.posX, &plywood.posY, &plywood.posZ);

					remainLengthDouble -= 2.400;
				}
				bBeginFound = false;
			}
		}
	}
	
	// 측면(R) 유로폼 1단 배치
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);

		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > EPS) && (placingZone->cellsAtRSide [0][xx].dirLen > EPS)) {
			moveIn3DSlope ('x', euroform.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [0][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			euroform.radAng += DegreeToRad (180.0);

			double dirLen = placingZone->cellsAtRSide[0][xx].dirLen;
			if ((abs(dirLen - 1.200) < EPS) || (abs(dirLen - 0.900) < EPS) || (abs(dirLen - 0.600) < EPS)) {
				elemList_Tableform[getAreaSeqNumOfCell(placingZone, true, true, xx)].Push(euroform.placeObjectMirrored(6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string("%.0f", placingZone->cellsAtRSide[0][xx].perLen * 1000.0).c_str(),
					"eu_hei", APIParT_CString, format_string("%.0f", placingZone->cellsAtRSide[0][xx].dirLen * 1000.0).c_str(),
					"u_ins", APIParT_CString, "벽눕히기",
					"ang_x", APIParT_Angle, format_string("%f", DegreeToRad(90.0)).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str()));
			}
			else {
				elemList_Tableform[getAreaSeqNumOfCell(placingZone, true, true, xx)].Push(euroform.placeObjectMirrored(6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string("%f", placingZone->cellsAtRSide[0][xx].perLen).c_str(),
					"eu_hei2", APIParT_Length, format_string("%f", placingZone->cellsAtRSide[0][xx].dirLen).c_str(),
					"u_ins", APIParT_CString, "벽눕히기",
					"ang_x", APIParT_Angle, format_string("%f", DegreeToRad(90.0)).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str()));
			}
		}
	}

	// 측면(R) 휠러스페이서 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [1][xx].objType == FILLERSP) && (placingZone->cellsAtRSide [1][xx].perLen > 0) && (placingZone->cellsAtRSide [1][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [1][xx].objType != FILLERSP) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [1][yy].dirLen;

			fillersp.init (L("휠러스페이서v1.0.gsm"), layerInd_Fillerspacer, infoBeam.floorInd, placingZone->cellsAtRSide [1][beginIndex].leftBottomX, placingZone->cellsAtRSide [1][beginIndex].leftBottomY, placingZone->cellsAtRSide [1][beginIndex].leftBottomZ, placingZone->cellsAtRSide [1][beginIndex].ang);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 2.400)
					lengthDouble = 2.400;
				else
					lengthDouble = remainLengthDouble;

				fillersp.radAng += DegreeToRad (180.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (fillersp.placeObjectMirrored (4,
					"f_thk", APIParT_Length, format_string ("%f", placingZone->cellsAtRSide [1][beginIndex].perLen).c_str(),
					"f_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(),
					"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + placingZone->slantAngle).c_str(),
					"f_rota", APIParT_Angle, format_string ("%f", 0.0).c_str()));
				fillersp.radAng -= DegreeToRad (180.0);
				moveIn3DSlope ('x', fillersp.radAng, placingZone->slantAngle, lengthDouble, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

				remainLengthDouble -= 2.400;
			}

			bBeginFound = false;
		}
	}

	// 측면(R) 유로폼 2단 배치
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtRSide [2][xx].leftBottomX, placingZone->cellsAtRSide [2][xx].leftBottomY, placingZone->cellsAtRSide [2][xx].leftBottomZ, placingZone->cellsAtRSide [2][xx].ang);

		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > EPS) && (placingZone->cellsAtRSide [2][xx].dirLen > EPS)) {
			moveIn3DSlope ('x', euroform.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [2][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			euroform.radAng += DegreeToRad (180.0);

			double dirLen = placingZone->cellsAtRSide[2][xx].dirLen;
			if ((abs(dirLen - 1.200) < EPS) || (abs(dirLen - 0.900) < EPS) || (abs(dirLen - 0.600) < EPS)) {
				elemList_Tableform[getAreaSeqNumOfCell(placingZone, true, true, xx)].Push(euroform.placeObjectMirrored(6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string("%.0f", placingZone->cellsAtRSide[2][xx].perLen * 1000.0).c_str(),
					"eu_hei", APIParT_CString, format_string("%.0f", placingZone->cellsAtRSide[2][xx].dirLen * 1000.0).c_str(),
					"u_ins", APIParT_CString, "벽눕히기",
					"ang_x", APIParT_Angle, format_string("%f", DegreeToRad(90.0)).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str()));
			}
			else {
				elemList_Tableform[getAreaSeqNumOfCell(placingZone, true, true, xx)].Push(euroform.placeObjectMirrored(6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string("%f", placingZone->cellsAtRSide[2][xx].perLen).c_str(),
					"eu_hei2", APIParT_Length, format_string("%f", placingZone->cellsAtRSide[2][xx].dirLen).c_str(),
					"u_ins", APIParT_CString, "벽눕히기",
					"ang_x", APIParT_Angle, format_string("%f", DegreeToRad(90.0)).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str()));
			}
		}
	}

	// 측면(R) 각재/합판 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if (getObjectType (placingZone, false, 3) == TIMBER) {
			if ((placingZone->cellsAtRSide [3][xx].objType == TIMBER) && (placingZone->cellsAtRSide [3][xx].perLen > 0) && (placingZone->cellsAtRSide [3][xx].dirLen > 0)) {
				// 연속적인 인덱스 범위 찾기
				if (bBeginFound == false) {
					beginIndex = xx;
					bBeginFound = true;
					bShow = true;

					bTimberMove = false;
					moveZ = 0.0;
					addedPlywood = 0;
					if (placingZone->cellsAtRSide [3][xx].perLen < 0.040 - EPS) {
						// 40mm 미만이면 앞쪽으로(!) 50*80 각재
						horLen = 0.050;
						verLen = 0.080;
						bTimberMove = true;

						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.010) < EPS)	addedPlywood = 1;	// 10mm 이면 합판 1장 얹음
						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.020) < EPS)	addedPlywood = 2;	// 20mm 이면 합판 2장 얹음
						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.030) < EPS)	addedPlywood = 3;	// 30mm 이면 합판 3장 얹음
					} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.050 - EPS)) {
						// 40mm 이상 50mm 미만이면, 50*40 각재
						horLen = 0.050;
						verLen = 0.040;
					} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.080 - EPS)) {
						// 50mm 이상 80mm 미만이면, 80*50 각재
						horLen = 0.080;
						verLen = 0.050;
						moveZ = verLen;

						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.060) < EPS)	addedPlywood = 1;	// 60mm 이면 합판 1장 얹음
						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.070) < EPS)	addedPlywood = 2;	// 70mm 이면 합판 2장 얹음
					} else {
						// 80mm 이상 90mm 미만이면, 80*80 각재
						horLen = 0.080;
						verLen = 0.080;
						moveZ = verLen;

						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.090) < EPS)	addedPlywood = 1;	// 90mm 이면 합판 1장 얹음
						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.100) < EPS)	addedPlywood = 2;	// 100mm 이면 합판 2장 얹음
					}
				}
				endIndex = xx;
			}

			if (((placingZone->cellsAtRSide [3][xx].objType != TIMBER) || (xx == placingZone->nCells-1)) && (bShow == true)) {
				// 원장 사이즈 단위로 끊어서 배치하기 (각재)
				remainLengthDouble = 0.0;
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtRSide [3][yy].dirLen;

				timber.init (L("목재v1.0.gsm"), layerInd_Timber, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ, placingZone->cellsAtRSide [3][beginIndex].ang);

				while (remainLengthDouble > EPS) {
					if (remainLengthDouble > 3.600)
						lengthDouble = 3.600;
					else
						lengthDouble = remainLengthDouble;

					// 각재 설치
					if (bTimberMove == true) {
						moveIn3D ('y', timber.radAng, 0.067, &timber.posX, &timber.posY, &timber.posZ);
						moveIn3D ('z', timber.radAng, -0.080 * cos (placingZone->slantAngle), &timber.posX, &timber.posY, &timber.posZ);
						moveIn3D ('x', timber.radAng, 0.080 * sin (placingZone->slantAngle), &timber.posX, &timber.posY, &timber.posZ);
					}
					timber.radAng += DegreeToRad (180.0);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (timber.placeObjectMirrored (6,
						"w_ins", APIParT_CString, "벽세우기",
						"w_w", APIParT_Length, format_string ("%f", horLen).c_str(),
						"w_h", APIParT_Length, format_string ("%f", verLen).c_str(),
						"w_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(),
						"w_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(),
						"torsion_ang", APIParT_Angle, format_string ("%f", 0.0).c_str()));
					timber.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', timber.radAng, placingZone->slantAngle, lengthDouble, &timber.posX, &timber.posY, &timber.posZ);
					if (bTimberMove == true) {
						moveIn3D ('y', timber.radAng, -0.067, &timber.posX, &timber.posY, &timber.posZ);
						moveIn3D ('z', timber.radAng, 0.080 * cos (placingZone->slantAngle), &timber.posX, &timber.posY, &timber.posZ);
						moveIn3D ('x', timber.radAng, -0.080 * sin (placingZone->slantAngle), &timber.posX, &timber.posY, &timber.posZ);
					}

					remainLengthDouble -= 3.600;
				}

				// 원장 사이즈 단위로 끊어서 배치하기 (추가 합판)
				remainLengthDouble = 0.0;
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtRSide [3][yy].dirLen;

				plywood1.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ, placingZone->cellsAtRSide [3][beginIndex].ang);
				plywood2.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ, placingZone->cellsAtRSide [3][beginIndex].ang);
				plywood3.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ, placingZone->cellsAtRSide [3][beginIndex].ang);

				while (remainLengthDouble > EPS) {
					if (remainLengthDouble > 2.400)
						lengthDouble = 2.400;
					else
						lengthDouble = remainLengthDouble;
						
					if (addedPlywood >= 1) {
						moveIn3D ('y', plywood1.radAng, 0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3D ('z', plywood1.radAng, moveZ * cos (placingZone->slantAngle), &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3D ('x', plywood1.radAng, -moveZ * sin (placingZone->slantAngle), &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						plywood1.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood1.placeObjectMirrored (8, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070).c_str(), "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(), "p_rot", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "sogak", APIParT_Boolean, "0.0"));
						plywood1.radAng -= DegreeToRad (180.0);
						moveIn3DSlope ('x', plywood1.radAng, placingZone->slantAngle, lengthDouble, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3D ('y', plywood1.radAng, -0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3D ('z', plywood1.radAng, -moveZ * cos (placingZone->slantAngle), &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3D ('x', plywood1.radAng, moveZ * sin (placingZone->slantAngle), &plywood1.posX, &plywood1.posY, &plywood1.posZ);
					}
					if (addedPlywood >= 2) {
						moveIn3D ('y', plywood2.radAng, 0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3D ('z', plywood2.radAng, (moveZ + 0.0115) * cos (placingZone->slantAngle), &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3D ('x', plywood2.radAng, -(moveZ + 0.0115) * sin (placingZone->slantAngle), &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						plywood2.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood2.placeObjectMirrored (8, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070).c_str(), "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(), "p_rot", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "sogak", APIParT_Boolean, "0.0"));
						plywood2.radAng -= DegreeToRad (180.0);
						moveIn3DSlope ('x', plywood2.radAng, placingZone->slantAngle, lengthDouble, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3D ('y', plywood2.radAng, -0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3D ('z', plywood2.radAng, -(moveZ + 0.0115) * cos (placingZone->slantAngle), &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3D ('x', plywood2.radAng, (moveZ + 0.0115) * sin (placingZone->slantAngle), &plywood2.posX, &plywood2.posY, &plywood2.posZ);
					}
					if (addedPlywood >= 3) {
						moveIn3D ('y', plywood3.radAng, 0.070, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						moveIn3D ('z', plywood3.radAng, (moveZ + 0.0115*2) * cos (placingZone->slantAngle), &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						moveIn3D ('x', plywood3.radAng, -(moveZ + 0.0115*2) * sin (placingZone->slantAngle), &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						plywood3.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood3.placeObjectMirrored (8, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070).c_str(), "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(), "p_rot", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "sogak", APIParT_Boolean, "0.0"));
						plywood3.radAng -= DegreeToRad (180.0);
						moveIn3DSlope ('x', plywood3.radAng, placingZone->slantAngle, lengthDouble, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						moveIn3D ('y', plywood3.radAng, -0.070, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						moveIn3D ('z', plywood3.radAng, -(moveZ + 0.0115*2) * cos (placingZone->slantAngle), &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						moveIn3D ('x', plywood3.radAng, (moveZ + 0.0115*2) * sin (placingZone->slantAngle), &plywood3.posX, &plywood3.posY, &plywood3.posZ);
					}

					remainLengthDouble -= 2.400;
				}

				bBeginFound = false;
			}
		} else if (getObjectType (placingZone, false, 3) == PLYWOOD) {
			// 100mm 초과이면
			if ((placingZone->cellsAtRSide [3][xx].objType == PLYWOOD) && (placingZone->cellsAtRSide [3][xx].perLen > 0) && (placingZone->cellsAtRSide [3][xx].dirLen > 0)) {
				// 연속적인 인덱스 범위 찾기
				if (bBeginFound == false) {
					beginIndex = xx;
					bBeginFound = true;
					bShow = true;
				}
				endIndex = xx;
			}

			if (((placingZone->cellsAtRSide [3][xx].objType != PLYWOOD) || (xx == placingZone->nCells-1)) && (bShow == true)) {
				// 원장 사이즈 단위로 끊어서 배치하기 (합판)
				remainLengthDouble = 0.0;
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtRSide [3][yy].dirLen;

				plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ, placingZone->cellsAtRSide [3][beginIndex].ang);

				while (remainLengthDouble > EPS) {
					if (remainLengthDouble > 2.400)
						lengthDouble = 2.400;
					else
						lengthDouble = remainLengthDouble;

					// 합판 설치
					plywood.radAng += DegreeToRad (180.0);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood.placeObjectMirrored (14,
						"p_stan", APIParT_CString, "비규격",
						"w_dir", APIParT_CString, "벽눕히기",
						"p_thk", APIParT_CString, "11.5T",
						"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtRSide [3][beginIndex].perLen).c_str(),
						"p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(),
						"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
						"sogak", APIParT_Boolean, "1.0",
						"bInverseSogak", APIParT_Boolean, "1.0",
						"prof", APIParT_CString, "소각",
						"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
						"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
						"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
						"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
					plywood.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, lengthDouble, &plywood.posX, &plywood.posY, &plywood.posZ);

					remainLengthDouble -= 2.400;
				}
				bBeginFound = false;
			}
		}
	}

	// 하부 유로폼(L) 배치
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);

		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > EPS) && (placingZone->cellsAtBottom [0][xx].dirLen > EPS)) {
			moveIn3DSlope ('x', euroform.radAng, placingZone->slantAngle, placingZone->cellsAtBottom [0][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			moveIn3D ('y', euroform.radAng, placingZone->cellsAtBottom [0][xx].perLen, &euroform.posX, &euroform.posY, &euroform.posZ);

			double dirLen = placingZone->cellsAtBottom[0][xx].dirLen;
			if ((abs(dirLen - 1.200) < EPS) || (abs(dirLen - 0.900) < EPS) || (abs(dirLen - 0.600) < EPS)) {
				elemList_Tableform[getAreaSeqNumOfCell(placingZone, true, true, xx)].Push(euroform.placeObject(6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string("%.0f", placingZone->cellsAtBottom[0][xx].perLen * 1000.0).c_str(),
					"eu_hei", APIParT_CString, format_string("%.0f", placingZone->cellsAtBottom[0][xx].dirLen * 1000.0).c_str(),
					"u_ins", APIParT_CString, "벽눕히기",
					"ang_x", APIParT_Angle, format_string("%f", DegreeToRad(0.0)).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str()));
			}
			else {
				elemList_Tableform[getAreaSeqNumOfCell(placingZone, true, true, xx)].Push(euroform.placeObject(6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string("%f", placingZone->cellsAtBottom[0][xx].perLen).c_str(),
					"eu_hei2", APIParT_Length, format_string("%f", placingZone->cellsAtBottom[0][xx].dirLen).c_str(),
					"u_ins", APIParT_CString, "벽눕히기",
					"ang_x", APIParT_Angle, format_string("%f", DegreeToRad(0.0)).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str()));
			}
		}
	}

	// 하부 휠러스페이서 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [1][xx].objType == FILLERSP) && (placingZone->cellsAtBottom [1][xx].perLen > 0) && (placingZone->cellsAtBottom [1][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [1][xx].objType != FILLERSP) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [1][yy].dirLen;

			fillersp.init (L("휠러스페이서v1.0.gsm"), layerInd_Fillerspacer, infoBeam.floorInd, placingZone->cellsAtBottom [1][beginIndex].leftBottomX, placingZone->cellsAtBottom [1][beginIndex].leftBottomY, placingZone->cellsAtBottom [1][beginIndex].leftBottomZ, placingZone->cellsAtBottom [1][beginIndex].ang);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 2.400)
					lengthDouble = 2.400;
				else
					lengthDouble = remainLengthDouble;

				moveIn3D ('y', fillersp.radAng, placingZone->cellsAtBottom [1][beginIndex].perLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (fillersp.placeObject (4,
					"f_thk", APIParT_Length, format_string ("%f", placingZone->cellsAtBottom [1][beginIndex].perLen).c_str(),
					"f_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(),
					"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + placingZone->slantAngle).c_str(),
					"f_rota", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
				moveIn3D ('y', fillersp.radAng, -placingZone->cellsAtBottom [1][beginIndex].perLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
				moveIn3DSlope ('x', fillersp.radAng, placingZone->slantAngle, lengthDouble, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

				remainLengthDouble -= 2.400;
			}

			bBeginFound = false;
		}
	}

	// 하부 유로폼(R) 배치
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtBottom [2][xx].leftBottomX, placingZone->cellsAtBottom [2][xx].leftBottomY, placingZone->cellsAtBottom [2][xx].leftBottomZ, placingZone->cellsAtBottom [2][xx].ang);

		if ((placingZone->cellsAtBottom [2][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [2][xx].perLen > EPS) && (placingZone->cellsAtBottom [2][xx].dirLen > EPS)) {
			moveIn3DSlope ('x', euroform.radAng, placingZone->slantAngle, placingZone->cellsAtBottom [2][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			moveIn3D ('y', euroform.radAng, placingZone->cellsAtBottom [2][xx].perLen, &euroform.posX, &euroform.posY, &euroform.posZ);

			double dirLen = placingZone->cellsAtBottom[2][xx].dirLen;
			if ((abs(dirLen - 1.200) < EPS) || (abs(dirLen - 0.900) < EPS) || (abs(dirLen - 0.600) < EPS)) {
				elemList_Tableform[getAreaSeqNumOfCell(placingZone, true, true, xx)].Push(euroform.placeObject(6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string("%.0f", placingZone->cellsAtBottom[2][xx].perLen * 1000.0).c_str(),
					"eu_hei", APIParT_CString, format_string("%.0f", placingZone->cellsAtBottom[2][xx].dirLen * 1000.0).c_str(),
					"u_ins", APIParT_CString, "벽눕히기",
					"ang_x", APIParT_Angle, format_string("%f", DegreeToRad(0.0)).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str()));
			}
			else {
				elemList_Tableform[getAreaSeqNumOfCell(placingZone, true, true, xx)].Push(euroform.placeObject(6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string("%f", placingZone->cellsAtBottom[2][xx].perLen).c_str(),
					"eu_hei2", APIParT_Length, format_string("%f", placingZone->cellsAtBottom[2][xx].dirLen).c_str(),
					"u_ins", APIParT_CString, "벽눕히기",
					"ang_x", APIParT_Angle, format_string("%f", DegreeToRad(0.0)).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str()));
			}
		}
	}

	// 합판 셀 배치 (측면 뷰에서 합판으로 선택한 경우)
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [0][xx].objType == PLYWOOD) && (placingZone->cellsAtLSide [0][xx].perLen > EPS) && (placingZone->cellsAtLSide [0][xx].dirLen > EPS)) {
			// 측면(L) 합판 배치
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);
			elemList_Plywood [getAreaSeqNumOfCell (placingZone, true, false, xx)].Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "벽눕히기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtLSide [0][xx].perLen - placingZone->hRest_Left).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->cellsAtLSide [0][xx].dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));

			// 측면(R) 합판 배치
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);
			plywood.radAng += DegreeToRad (180.0);
			elemList_Plywood [getAreaSeqNumOfCell (placingZone, true, false, xx)].Push (plywood.placeObjectMirrored (13,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "벽눕히기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtRSide [0][xx].perLen - placingZone->hRest_Right).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->cellsAtRSide [0][xx].dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));

			// 하부 합판 배치
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);
			moveIn3D ('y', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList_Plywood [getAreaSeqNumOfCell (placingZone, true, false, xx)].Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "바닥깔기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtBottom [0][xx].perLen + 0.0615*2).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->cellsAtBottom [0][xx].dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}
	}

	// !!! -- 10mm씩 쪼개야 함
	// 휠러 셀 배치 (측면 뷰에서 휠러로 선택한 경우)
	for (xx = 0; xx < placingZone->nCells; ++xx) {
		if ((placingZone->cellsAtLSide[0][xx].objType == FILLERSP) && (placingZone->cellsAtLSide[0][xx].perLen > EPS) && (placingZone->cellsAtLSide[0][xx].dirLen > EPS)) {
			// 측면(L) 휠러스페이서 배치
			fillersp.init(L("휠러스페이서v1.0.gsm"), layerInd_Fillerspacer, infoBeam.floorInd, placingZone->cellsAtLSide[0][xx].leftBottomX, placingZone->cellsAtLSide[0][xx].leftBottomY, placingZone->cellsAtLSide[0][xx].leftBottomZ, placingZone->cellsAtLSide[0][xx].ang);
			moveIn3DSlope('x', fillersp.radAng, placingZone->slantAngle, placingZone->cellsAtLSide[0][xx].dirLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
			elemList_Plywood [getAreaSeqNumOfCell (placingZone, true, false, xx)].Push(fillersp.placeObject (4,
				"f_thk", APIParT_Length, format_string("%f", placingZone->cellsAtLSide[0][xx].dirLen).c_str(),
				"f_leng", APIParT_Length, format_string("%f", placingZone->cellsAtLSide[0][xx].perLen).c_str(),
				"f_ang", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str(),
				"f_rota", APIParT_Angle, format_string("%f", 0.0).c_str()));

			// 측면(R) 휠러스페이서 배치
			fillersp.init(L("휠러스페이서v1.0.gsm"), layerInd_Fillerspacer, infoBeam.floorInd, placingZone->cellsAtRSide[0][xx].leftBottomX, placingZone->cellsAtRSide[0][xx].leftBottomY, placingZone->cellsAtRSide[0][xx].leftBottomZ, placingZone->cellsAtRSide[0][xx].ang);
			moveIn3DSlope('x', fillersp.radAng, placingZone->slantAngle, placingZone->cellsAtLSide[0][xx].dirLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
			fillersp.radAng += DegreeToRad(180.0);
			elemList_Plywood[getAreaSeqNumOfCell(placingZone, true, false, xx)].Push(fillersp.placeObjectMirrored(4,
				"f_thk", APIParT_Length, format_string("%f", placingZone->cellsAtRSide[0][xx].dirLen).c_str(),
				"f_leng", APIParT_Length, format_string("%f", placingZone->cellsAtRSide[0][xx].perLen).c_str(),
				"f_ang", APIParT_Angle, format_string("%f", DegreeToRad(90.0) + placingZone->slantAngle).c_str(),
				"f_rota", APIParT_Angle, format_string("%f", 0.0).c_str()));

			// 하부 휠러스페이서 배치
			// !!!
		}
	}

	return	err;
}

// 유로폼/휠러/합판/각재를 채운 후 부자재 설치 (아웃코너앵글, 비계파이프, 핀볼트, 각파이프행거, 블루클램프, 블루목심)
GSErrCode	BeamTableformPlacingZone::placeAuxObjectsA (BeamTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx, yy, zz;

	// 원장 사이즈 길이 계산
	bool	bShow;
	bool	bBeginFound;
	short	beginIndex, endIndex;
	double	remainLengthDouble;
	double	lengthDouble;
	double	tempLengthDouble;

	// 회전 각도가 양수, 음수에 따라 파라미터에 전달할 경사 각도 변수
	double	slantAngle = (placingZone->slantAngle >= EPS) ? (placingZone->slantAngle) : (DegreeToRad (360.0) + placingZone->slantAngle);
	double	minusSlantAngle = (placingZone->slantAngle >= EPS) ? (DegreeToRad (360.0) - placingZone->slantAngle) : (-placingZone->slantAngle);

	EasyObjectPlacement outangle, hanger, blueClamp, blueTimberRail;
	EasyObjectPlacement pipe1, pipe2;
	EasyObjectPlacement pinbolt1, pinbolt2;

	// 아웃코너앵글 (L) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			outangle.init (L("아웃코너앵글v1.0.gsm"), layerInd_OutcornerAngle, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 2.400)
					lengthDouble = 2.400;
				else
					lengthDouble = remainLengthDouble;

				moveIn3DSlope ('x', outangle.radAng, placingZone->slantAngle, lengthDouble, &outangle.posX, &outangle.posY, &outangle.posZ);
				outangle.radAng += DegreeToRad (180.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (outangle.placeObject (2, "a_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "a_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
				outangle.radAng -= DegreeToRad (180.0);

				remainLengthDouble -= 2.400;
			}

			bBeginFound = false;
		}
	}

	// 아웃코너앵글 (R) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			outangle.init (L("아웃코너앵글v1.0.gsm"), layerInd_OutcornerAngle, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3D ('y', outangle.radAng, placingZone->cellsAtBottom [0][xx].perLen + placingZone->cellsAtBottom [1][xx].perLen + placingZone->cellsAtBottom [2][xx].perLen, &outangle.posX, &outangle.posY, &outangle.posZ);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 2.400)
					lengthDouble = 2.400;
				else
					lengthDouble = remainLengthDouble;

				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (outangle.placeObject (2, "a_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "a_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
				moveIn3DSlope ('x', outangle.radAng, placingZone->slantAngle, lengthDouble, &outangle.posX, &outangle.posY, &outangle.posZ);

				remainLengthDouble -= 2.400;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (L) 배치 - 1단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > 0) && (placingZone->cellsAtLSide [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [0][yy].dirLen;

			// 1번 파이프
			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, -0.0635 - 0.025, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.050 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.050 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			}

			// 2번 파이프
			pipe2.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, -0.050, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			moveIn3D ('y', pipe2.radAng, -0.0635 - 0.025, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.300 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.300 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.200 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.200 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			}

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				// 1번 파이프
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, 0.062 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, -0.062 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				// 2번 파이프
				//if ((abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, 0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, 0.062 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, -0.062 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, 0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				//}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (L) 배치 - 2단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > 0) && (placingZone->cellsAtLSide [2][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [2][yy].dirLen;

			// 1번 파이프
			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, -0.0635 - 0.025, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.050 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.050 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			}

			// 2번 파이프
			pipe2.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);
			moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, -0.050, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			moveIn3D ('y', pipe2.radAng, -0.0635 - 0.025, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.300 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.300 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.200 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.200 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			}

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				// 1번 파이프
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, 0.062 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, -0.062 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				// 2번 파이프
				//if ((abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, 0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, 0.062 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, -0.062 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, 0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				//}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (R) 배치 - 1단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > 0) && (placingZone->cellsAtRSide [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [0][yy].dirLen;

			// 1번 파이프
			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, 0.0635 + 0.025, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.050 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.050 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			}

			// 2번 파이프
			pipe2.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, -0.050, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			moveIn3D ('y', pipe2.radAng, 0.0635 + 0.025, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.300 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.300 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.200 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.200 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			}

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				// 1번 파이프
				//moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//pipe1.radAng += DegreeToRad (180.0);
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, 0.062 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, -0.062 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//pipe1.radAng -= DegreeToRad (180.0);
				// 2번 파이프
				//if ((abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng += DegreeToRad (180.0);
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, -0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, 0.062 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, 0.062 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, -0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng -= DegreeToRad (180.0);
				//}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (R) 배치 - 2단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > 0) && (placingZone->cellsAtRSide [2][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [2][yy].dirLen;

			// 1번 파이프
			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, 0.0635 + 0.025, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.050 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.050 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			}

			// 2번 파이프
			pipe2.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);
			moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, -0.050, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			moveIn3D ('y', pipe2.radAng, 0.0635 + 0.025, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.300 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.300 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.200 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.200 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			}

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				// 1번 파이프
				//moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//pipe1.radAng += DegreeToRad (180.0);
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, 0.062 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, -0.062 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//pipe1.radAng -= DegreeToRad (180.0);
				// 2번 파이프
				//if ((abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng += DegreeToRad (180.0);
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, -0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, 0.062 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, 0.062 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, -0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng -= DegreeToRad (180.0);
				//}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (하부-L) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('z', pipe1.radAng, (-0.0635 - 0.025) * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('x', pipe1.radAng, -(-0.0635 - 0.025) * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (하부-R) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('z', pipe1.radAng, (-0.0635 - 0.025) * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('x', pipe1.radAng, -(-0.0635 - 0.025) * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, placingZone->cellsAtBottom [0][xx].perLen + placingZone->cellsAtBottom [1][xx].perLen + placingZone->cellsAtBottom [2][xx].perLen, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				pipe1.radAng += DegreeToRad (180.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				pipe1.radAng -= DegreeToRad (180.0);

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (하부-센터) 배치
	if (((abs (placingZone->cellsAtBottom [0][0].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtBottom [0][0].perLen - 0.500) < EPS)) || (placingZone->cellsAtBottom [2][0].objType == EUROFORM)) {
		bShow = false;
		bBeginFound = false;
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
				// 연속적인 인덱스 범위 찾기
				if (bBeginFound == false) {
					beginIndex = xx;
					bBeginFound = true;
					bShow = true;
				}
				endIndex = xx;
			}

			if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
				// 원장 사이즈 단위로 끊어서 배치하기
				remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

				pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('z', pipe1.radAng, (-0.0635 - 0.025) * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -(-0.0635 - 0.025) * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				if (placingZone->cellsAtBottom [2][0].objType == EUROFORM)
					moveIn3D ('y', pipe1.radAng, placingZone->cellsAtBottom [0][0].perLen, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				else {
					if (abs (placingZone->cellsAtBottom [0][0].perLen - 0.600) < EPS)
						moveIn3D ('y', pipe1.radAng, placingZone->cellsAtBottom [0][0].perLen / 2, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
					else if (abs (placingZone->cellsAtBottom [0][0].perLen - 0.500) < EPS)
						moveIn3D ('y', pipe1.radAng, 0.200, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				}

				while (remainLengthDouble > EPS) {
					if (remainLengthDouble > 6.000)
						lengthDouble = 6.000;
					else
						lengthDouble = remainLengthDouble;

					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "측면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

					remainLengthDouble -= 6.000;
				}

				bBeginFound = false;
			}
		}
	}

	// 핀볼트 (L) 배치 - 1단 유로폼
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);
		pinbolt2.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);

		// 1번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt1.radAng, -0.1635, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.300) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.200) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.050 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.050 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		}

		// 2번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt2.radAng, -0.1635, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.300) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.300 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.300 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.200) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.200 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.200 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		}

		// 현재 셀이 유로폼, 다음 셀이 유로폼일 경우
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > EPS) && (placingZone->cellsAtLSide [0][xx].dirLen > EPS) && (placingZone->cellsAtLSide [0][xx+1].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx+1].perLen > EPS) && (placingZone->cellsAtLSide [0][xx+1].dirLen > EPS)) {
			// 1번 파이프에 체결할 핀볼트
			//moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			//pinbolt1.radAng += DegreeToRad (90.0);
			//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
			//pinbolt1.radAng -= DegreeToRad (90.0);

			// 2번 파이프에 체결할 핀볼트
			//if ((abs (placingZone->cellsAtLSide [0][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			//}
		}
	}

	// 핀볼트 (L) 배치 - 2단 유로폼
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [2][xx].leftBottomX, placingZone->cellsAtLSide [2][xx].leftBottomY, placingZone->cellsAtLSide [2][xx].leftBottomZ, placingZone->cellsAtLSide [2][xx].ang);
		pinbolt2.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [2][xx].leftBottomX, placingZone->cellsAtLSide [2][xx].leftBottomY, placingZone->cellsAtLSide [2][xx].leftBottomZ, placingZone->cellsAtLSide [2][xx].ang);

		// 1번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt1.radAng, -0.1635, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.300) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.200) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.050 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.050 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		}

		// 2번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt2.radAng, -0.1635, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.300) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.300 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.300 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.200) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.200 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.200 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		}

		// 현재 셀이 유로폼, 다음 셀이 유로폼일 경우
		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > EPS) && (placingZone->cellsAtLSide [2][xx].dirLen > EPS) && (placingZone->cellsAtLSide [2][xx+1].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx+1].perLen > EPS) && (placingZone->cellsAtLSide [2][xx+1].dirLen > EPS)) {
			// 1번 파이프에 체결할 핀볼트
			//moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			//pinbolt1.radAng += DegreeToRad (90.0);
			//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
			//pinbolt1.radAng -= DegreeToRad (90.0);

			// 2번 파이프에 체결할 핀볼트
			//if ((abs (placingZone->cellsAtLSide [2][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [2][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			//}
		}
	}

	// 핀볼트 (R) 배치 - 1단 유로폼
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);
		pinbolt2.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);

		// 1번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt1.radAng, 0.1635, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.300) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.200) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.050 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.050 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		}

		// 2번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt2.radAng, 0.1635, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.300) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.300 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.300 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.200) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.200 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.200 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		}

		// 현재 셀이 유로폼, 다음 셀이 유로폼일 경우
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > EPS) && (placingZone->cellsAtRSide [0][xx].dirLen > EPS) && (placingZone->cellsAtRSide [0][xx+1].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx+1].perLen > EPS) && (placingZone->cellsAtRSide [0][xx+1].dirLen > EPS)) {
			// 1번 파이프에 체결할 핀볼트
			//moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			//pinbolt1.radAng += DegreeToRad (90.0);
			//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
			//pinbolt1.radAng -= DegreeToRad (90.0);

			// 2번 파이프에 체결할 핀볼트
			//if ((abs (placingZone->cellsAtRSide [0][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [0][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			//}
		}
	}

	// 핀볼트 (R) 배치 - 2단 유로폼
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [2][xx].leftBottomX, placingZone->cellsAtRSide [2][xx].leftBottomY, placingZone->cellsAtRSide [2][xx].leftBottomZ, placingZone->cellsAtRSide [2][xx].ang);
		pinbolt2.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [2][xx].leftBottomX, placingZone->cellsAtRSide [2][xx].leftBottomY, placingZone->cellsAtRSide [2][xx].leftBottomZ, placingZone->cellsAtRSide [2][xx].ang);

		// 1번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt1.radAng, 0.1635, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.300) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.200) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.050 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.050 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		}

		// 2번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt2.radAng, 0.1635, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.300) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.300 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.300 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.200) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.200 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.200 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		}

		// 현재 셀이 유로폼, 다음 셀이 유로폼일 경우
		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > EPS) && (placingZone->cellsAtRSide [2][xx].dirLen > EPS) && (placingZone->cellsAtRSide [2][xx+1].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx+1].perLen > EPS) && (placingZone->cellsAtRSide [2][xx+1].dirLen > EPS)) {
			// 1번 파이프에 체결할 핀볼트
			//moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [2][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			//pinbolt1.radAng += DegreeToRad (90.0);
			//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
			//pinbolt1.radAng -= DegreeToRad (90.0);

			// 2번 파이프에 체결할 핀볼트
			//if ((abs (placingZone->cellsAtRSide [2][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [2][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			//}
		}
	}

	// 핀볼트 (하부-센터) 배치
	if (((abs (placingZone->cellsAtBottom [0][0].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtBottom [0][0].perLen - 0.500) < EPS)) || (placingZone->cellsAtBottom [2][0].objType == EUROFORM)) {
		for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
			pinbolt1.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);

			moveIn3D ('z', pinbolt1.radAng, -0.1635 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, 0.1635 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);

			if (placingZone->cellsAtBottom [2][0].objType == EUROFORM)
				moveIn3D ('y', pinbolt1.radAng, placingZone->cellsAtBottom [0][0].perLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			else {
				if (abs (placingZone->cellsAtBottom [0][0].perLen - 0.600) < EPS)
					moveIn3D ('y', pinbolt1.radAng, placingZone->cellsAtBottom [0][0].perLen / 2, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
				else
					moveIn3D ('y', pinbolt1.radAng, 0.200, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			}

			// 현재 셀이 유로폼, 다음 셀이 유로폼일 경우
			if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > EPS) && (placingZone->cellsAtBottom [0][xx].dirLen > EPS) && (placingZone->cellsAtBottom [0][xx+1].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx+1].perLen > EPS) && (placingZone->cellsAtBottom [0][xx+1].dirLen > EPS)) {
				moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtBottom [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
				pinbolt1.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
				pinbolt1.radAng -= DegreeToRad (90.0);
			}
		}
	}

	// 각파이프행거 (하부-L) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			hanger.init (L("각파이프행거.gsm"), layerInd_RectpipeHanger, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3D ('z', hanger.radAng, -0.0635 * cos (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3D ('x', hanger.radAng, 0.0635 * sin (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "각파이프행거", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (90.0);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, -0.150 + remainLengthDouble - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "각파이프행거", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (90.0);
			tempLengthDouble = 0.0;
			for (zz = beginIndex ; zz <= beginIndex + round ((endIndex - beginIndex) / 2, 0) ; ++zz) {
				tempLengthDouble += placingZone->cellsAtBottom [0][zz].dirLen;
			}
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150 - remainLengthDouble + tempLengthDouble - 0.450, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "각파이프행거", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (90.0);

			bBeginFound = false;
		}
	}

	// 각파이프행거 (하부-R) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			hanger.init (L("각파이프행거.gsm"), layerInd_RectpipeHanger, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3D ('z', hanger.radAng, -0.0635 * cos (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3D ('x', hanger.radAng, 0.0635 * sin (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3D ('y', hanger.radAng, placingZone->cellsAtBottom [0][xx].perLen + placingZone->cellsAtBottom [1][xx].perLen + placingZone->cellsAtBottom [2][xx].perLen, &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (270.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "각파이프행거", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (270.0);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, -0.150 + remainLengthDouble - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (270.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "각파이프행거", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (270.0);
			tempLengthDouble = 0.0;
			for (zz = beginIndex ; zz <= beginIndex + round ((endIndex - beginIndex) / 2, 0) ; ++zz) {
				tempLengthDouble += placingZone->cellsAtBottom [0][zz].dirLen;
			}
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150 - remainLengthDouble + tempLengthDouble - 0.450, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (270.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "각파이프행거", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (270.0);

			bBeginFound = false;
		}
	}

	// 블루클램프 (L) - 1단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > 0) && (placingZone->cellsAtLSide [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [0][yy].dirLen;

			// 시작 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// 끝 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// 블루클램프 (L) - 2단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > 0) && (placingZone->cellsAtLSide [2][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [2][yy].dirLen;

			// 시작 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// 끝 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// 블루클램프 (R) - 1단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > 0) && (placingZone->cellsAtRSide [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [0][yy].dirLen;

			// 시작 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, 0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// 끝 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, 0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// 블루클램프 (R) - 2단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > 0) && (placingZone->cellsAtRSide [2][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [2][yy].dirLen;

			// 시작 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, 0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// 끝 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, 0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// 블루클램프 (하부-L)
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			// 시작 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			
			if ( !((abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// 끝 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if ( !((abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// 블루클램프 (하부-R)
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [2][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [2][xx].perLen > 0) && (placingZone->cellsAtBottom [2][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [2][yy].dirLen;

			// 시작 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [2][beginIndex].leftBottomX, placingZone->cellsAtBottom [2][beginIndex].leftBottomY, placingZone->cellsAtBottom [2][beginIndex].leftBottomZ, placingZone->cellsAtBottom [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if ( !((abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// 끝 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [2][beginIndex].leftBottomX, placingZone->cellsAtBottom [2][beginIndex].leftBottomY, placingZone->cellsAtBottom [2][beginIndex].leftBottomZ, placingZone->cellsAtBottom [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if ( !((abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// 블루목심 (상부-L): 측면 아래에서 4번째 셀이 각재일 경우
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [3][xx].objType == TIMBER) || (placingZone->cellsAtLSide [3][xx].objType == PLYWOOD)) {
			blueTimberRail.init (L("블루목심v1.0.gsm"), layerInd_BlueTimberRail, infoBeam.floorInd, placingZone->cellsAtLSide [3][xx].leftBottomX, placingZone->cellsAtLSide [3][xx].leftBottomY, placingZone->cellsAtLSide [3][xx].leftBottomZ, placingZone->cellsAtLSide [3][xx].ang);

			if (placingZone->cellsAtLSide [3][xx].perLen < 0.040 - EPS) {
				// 앞으로 설치된 50*80 각재
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.050 - EPS)) {
				// 50*40 각재
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.080 - EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*50 각재
				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.080 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.100 + EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*80 각재
				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if (placingZone->cellsAtLSide [3][xx].perLen >= 0.100 - EPS) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.064, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 합판 및 제작틀
				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			}
		}
	}

	// 블루목심 (상부-R): 측면 아래에서 4번째 셀이 각재일 경우
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [3][xx].objType == TIMBER) || (placingZone->cellsAtRSide [3][xx].objType == PLYWOOD)) {
			blueTimberRail.init (L("블루목심v1.0.gsm"), layerInd_BlueTimberRail, infoBeam.floorInd, placingZone->cellsAtRSide [3][xx].leftBottomX, placingZone->cellsAtRSide [3][xx].leftBottomY, placingZone->cellsAtRSide [3][xx].leftBottomZ, placingZone->cellsAtRSide [3][xx].ang);

			if (placingZone->cellsAtRSide [3][xx].perLen < 0.040 - EPS) {
				// 앞으로 설치된 50*80 각재
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.050 - EPS)) {
				// 50*40 각재
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.080 - EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*50 각재
				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.080 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.100 + EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*80 각재
				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if (placingZone->cellsAtRSide [3][xx].perLen >= 0.100 - EPS) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.064, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 합판 및 제작틀
				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			}
		}
	}

	return	err;
}

// 유로폼/휠러/합판/각재를 채운 후 부자재 설치 (아웃코너앵글, 비계파이프, 핀볼트, 각파이프행거, 블루클램프, 블루목심)
GSErrCode	BeamTableformPlacingZone::placeAuxObjectsB (BeamTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx, yy, zz;

	// 원장 사이즈 길이 계산
	bool	bShow;
	bool	bBeginFound;
	short	beginIndex, endIndex;
	double	remainLengthDouble;
	double	lengthDouble;
	double	tempLengthDouble;

	// 회전 각도가 양수, 음수에 따라 파라미터에 전달할 경사 각도 변수
	double	slantAngle = (placingZone->slantAngle >= EPS) ? (placingZone->slantAngle) : (DegreeToRad (360.0) + placingZone->slantAngle);
	double	minusSlantAngle = (placingZone->slantAngle >= EPS) ? (DegreeToRad (360.0) - placingZone->slantAngle) : (-placingZone->slantAngle);

	EasyObjectPlacement outangle, hanger, blueClamp, blueTimberRail;
	EasyObjectPlacement pipe1, pipe2;
	EasyObjectPlacement pinbolt1, pinbolt2;

	// 아웃코너앵글 (L) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			outangle.init (L("아웃코너앵글v1.0.gsm"), layerInd_OutcornerAngle, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 2.400)
					lengthDouble = 2.400;
				else
					lengthDouble = remainLengthDouble;

				moveIn3DSlope ('x', outangle.radAng, placingZone->slantAngle, lengthDouble, &outangle.posX, &outangle.posY, &outangle.posZ);
				outangle.radAng += DegreeToRad (180.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (outangle.placeObject (2, "a_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "a_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
				outangle.radAng -= DegreeToRad (180.0);

				remainLengthDouble -= 2.400;
			}

			bBeginFound = false;
		}
	}

	// 아웃코너앵글 (R) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			outangle.init (L("아웃코너앵글v1.0.gsm"), layerInd_OutcornerAngle, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3D ('y', outangle.radAng, placingZone->cellsAtBottom [0][xx].perLen + placingZone->cellsAtBottom [1][xx].perLen + placingZone->cellsAtBottom [2][xx].perLen, &outangle.posX, &outangle.posY, &outangle.posZ);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 2.400)
					lengthDouble = 2.400;
				else
					lengthDouble = remainLengthDouble;

				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (outangle.placeObject (2, "a_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "a_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
				moveIn3DSlope ('x', outangle.radAng, placingZone->slantAngle, lengthDouble, &outangle.posX, &outangle.posY, &outangle.posZ);

				remainLengthDouble -= 2.400;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (L) 배치 - 1단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > 0) && (placingZone->cellsAtLSide [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [0][yy].dirLen;

			// 1번 파이프
			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, -0.0635 - 0.025, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.050 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.050 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			}

			// 2번 파이프
			pipe2.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, -0.050, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			moveIn3D ('y', pipe2.radAng, -0.0635 - 0.025, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			}

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				// 1번 파이프
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				// 2번 파이프
				if ((abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS)) {
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (L) 배치 - 2단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > 0) && (placingZone->cellsAtLSide [2][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [2][yy].dirLen;

			// 1번 파이프
			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, -0.0635 - 0.025, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.050 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.050 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			}

			// 2번 파이프
			pipe2.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);
			moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, -0.050, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			moveIn3D ('y', pipe2.radAng, -0.0635 - 0.025, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			}

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				// 1번 파이프
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				// 2번 파이프
				if ((abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS)) {
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (R) 배치 - 1단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > 0) && (placingZone->cellsAtRSide [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [0][yy].dirLen;

			// 1번 파이프
			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, 0.0635 + 0.025, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.050 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.050 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			}

			// 2번 파이프
			pipe2.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, -0.050, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			moveIn3D ('y', pipe2.radAng, 0.0635 + 0.025, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			}

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				// 1번 파이프
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				pipe1.radAng += DegreeToRad (180.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				pipe1.radAng -= DegreeToRad (180.0);
				// 2번 파이프
				if ((abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng += DegreeToRad (180.0);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					pipe2.radAng -= DegreeToRad (180.0);
				}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (R) 배치 - 2단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > 0) && (placingZone->cellsAtRSide [2][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [2][yy].dirLen;

			// 1번 파이프
			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, 0.0635 + 0.025, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.150 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.150 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', pipe1.radAng, 0.050 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -0.050 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			}

			// 2번 파이프
			pipe2.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);
			moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, -0.050, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			moveIn3D ('y', pipe2.radAng, 0.0635 + 0.025, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', pipe2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				moveIn3D ('x', pipe2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			}

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				// 1번 파이프
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				pipe1.radAng += DegreeToRad (180.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				pipe1.radAng -= DegreeToRad (180.0);
				// 2번 파이프
				if ((abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng += DegreeToRad (180.0);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					pipe2.radAng -= DegreeToRad (180.0);
				}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (하부-L) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('z', pipe1.radAng, (-0.0635 - 0.025) * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('x', pipe1.radAng, -(-0.0635 - 0.025) * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (하부-R) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('z', pipe1.radAng, (-0.0635 - 0.025) * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('x', pipe1.radAng, -(-0.0635 - 0.025) * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, placingZone->cellsAtBottom [0][xx].perLen + placingZone->cellsAtBottom [1][xx].perLen + placingZone->cellsAtBottom [2][xx].perLen, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				pipe1.radAng += DegreeToRad (180.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				pipe1.radAng -= DegreeToRad (180.0);

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (하부-센터) 배치
	if (((abs (placingZone->cellsAtBottom [0][0].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtBottom [0][0].perLen - 0.500) < EPS)) || (placingZone->cellsAtBottom [2][0].objType == EUROFORM)) {
		bShow = false;
		bBeginFound = false;
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
				// 연속적인 인덱스 범위 찾기
				if (bBeginFound == false) {
					beginIndex = xx;
					bBeginFound = true;
					bShow = true;
				}
				endIndex = xx;
			}

			if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
				// 원장 사이즈 단위로 끊어서 배치하기
				remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

				pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('z', pipe1.radAng, (-0.0635 - 0.025) * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				moveIn3D ('x', pipe1.radAng, -(-0.0635 - 0.025) * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				if (placingZone->cellsAtBottom [2][0].objType == EUROFORM)
					moveIn3D ('y', pipe1.radAng, placingZone->cellsAtBottom [0][0].perLen, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				else {
					if (abs (placingZone->cellsAtBottom [0][0].perLen - 0.600) < EPS)
						moveIn3D ('y', pipe1.radAng, placingZone->cellsAtBottom [0][0].perLen / 2, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
					else if (abs (placingZone->cellsAtBottom [0][0].perLen - 0.500) < EPS)
						moveIn3D ('y', pipe1.radAng, 0.200, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				}

				while (remainLengthDouble > EPS) {
					if (remainLengthDouble > 6.000)
						lengthDouble = 6.000;
					else
						lengthDouble = remainLengthDouble;

					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "측면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

					remainLengthDouble -= 6.000;
				}

				bBeginFound = false;
			}
		}
	}

	// 핀볼트 (L) 배치 - 1단 유로폼
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);
		pinbolt2.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);

		// 1번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt1.radAng, -0.1635, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.300) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.200) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.050 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.050 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		}

		// 2번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt2.radAng, -0.1635, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		}

		// 현재 셀이 유로폼, 다음 셀이 유로폼일 경우
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > EPS) && (placingZone->cellsAtLSide [0][xx].dirLen > EPS) && (placingZone->cellsAtLSide [0][xx+1].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx+1].perLen > EPS) && (placingZone->cellsAtLSide [0][xx+1].dirLen > EPS)) {
			// 1번 파이프에 체결할 핀볼트
			moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			pinbolt1.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
			pinbolt1.radAng -= DegreeToRad (90.0);

			// 2번 파이프에 체결할 핀볼트
			if ((abs (placingZone->cellsAtLSide [0][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			}
		}
	}

	// 핀볼트 (L) 배치 - 2단 유로폼
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [2][xx].leftBottomX, placingZone->cellsAtLSide [2][xx].leftBottomY, placingZone->cellsAtLSide [2][xx].leftBottomZ, placingZone->cellsAtLSide [2][xx].ang);
		pinbolt2.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [2][xx].leftBottomX, placingZone->cellsAtLSide [2][xx].leftBottomY, placingZone->cellsAtLSide [2][xx].leftBottomZ, placingZone->cellsAtLSide [2][xx].ang);

		// 1번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt1.radAng, -0.1635, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.300) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.200) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.050 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.050 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		}

		// 2번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt2.radAng, -0.1635, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		}

		// 현재 셀이 유로폼, 다음 셀이 유로폼일 경우
		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > EPS) && (placingZone->cellsAtLSide [2][xx].dirLen > EPS) && (placingZone->cellsAtLSide [2][xx+1].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx+1].perLen > EPS) && (placingZone->cellsAtLSide [2][xx+1].dirLen > EPS)) {
			// 1번 파이프에 체결할 핀볼트
			moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			pinbolt1.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
			pinbolt1.radAng -= DegreeToRad (90.0);

			// 2번 파이프에 체결할 핀볼트
			if ((abs (placingZone->cellsAtLSide [2][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [2][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			}
		}
	}

	// 핀볼트 (R) 배치 - 1단 유로폼
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);
		pinbolt2.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);

		// 1번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt1.radAng, 0.1635, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.300) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.200) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.050 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.050 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		}

		// 2번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt2.radAng, 0.1635, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		}

		// 현재 셀이 유로폼, 다음 셀이 유로폼일 경우
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > EPS) && (placingZone->cellsAtRSide [0][xx].dirLen > EPS) && (placingZone->cellsAtRSide [0][xx+1].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx+1].perLen > EPS) && (placingZone->cellsAtRSide [0][xx+1].dirLen > EPS)) {
			// 1번 파이프에 체결할 핀볼트
			moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			pinbolt1.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
			pinbolt1.radAng -= DegreeToRad (90.0);

			// 2번 파이프에 체결할 핀볼트
			if ((abs (placingZone->cellsAtRSide [0][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [0][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			}
		}
	}

	// 핀볼트 (R) 배치 - 2단 유로폼
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [2][xx].leftBottomX, placingZone->cellsAtRSide [2][xx].leftBottomY, placingZone->cellsAtRSide [2][xx].leftBottomZ, placingZone->cellsAtRSide [2][xx].ang);
		pinbolt2.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [2][xx].leftBottomX, placingZone->cellsAtRSide [2][xx].leftBottomY, placingZone->cellsAtRSide [2][xx].leftBottomZ, placingZone->cellsAtRSide [2][xx].ang);

		// 1번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt1.radAng, 0.1635, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.300) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.150 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.150 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.200) < EPS) {
			moveIn3D ('z', pinbolt1.radAng, 0.050 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, -0.050 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
		}

		// 2번 파이프에 체결할 핀볼트
		moveIn3D ('y', pinbolt2.radAng, 0.1635, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.600) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.600 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.600 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.500) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.500 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.500 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.450) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.450 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.450 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		} else if (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.400) < EPS) {
			moveIn3D ('z', pinbolt2.radAng, (0.400 - 0.150) * cos (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
			moveIn3D ('x', pinbolt2.radAng, -(0.400 - 0.150) * sin (placingZone->slantAngle), &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
		}

		// 현재 셀이 유로폼, 다음 셀이 유로폼일 경우
		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > EPS) && (placingZone->cellsAtRSide [2][xx].dirLen > EPS) && (placingZone->cellsAtRSide [2][xx+1].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx+1].perLen > EPS) && (placingZone->cellsAtRSide [2][xx+1].dirLen > EPS)) {
			// 1번 파이프에 체결할 핀볼트
			moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [2][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			pinbolt1.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
			pinbolt1.radAng -= DegreeToRad (90.0);

			// 2번 파이프에 체결할 핀볼트
			if ((abs (placingZone->cellsAtRSide [2][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [2][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			}
		}
	}

	// 핀볼트 (하부-센터) 배치
	if (((abs (placingZone->cellsAtBottom [0][0].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtBottom [0][0].perLen - 0.500) < EPS)) || (placingZone->cellsAtBottom [2][0].objType == EUROFORM)) {
		for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
			pinbolt1.init (L("핀볼트세트v1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);

			moveIn3D ('z', pinbolt1.radAng, -0.1635 * cos (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			moveIn3D ('x', pinbolt1.radAng, 0.1635 * sin (placingZone->slantAngle), &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);

			if (placingZone->cellsAtBottom [2][0].objType == EUROFORM)
				moveIn3D ('y', pinbolt1.radAng, placingZone->cellsAtBottom [0][0].perLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			else {
				if (abs (placingZone->cellsAtBottom [0][0].perLen - 0.600) < EPS)
					moveIn3D ('y', pinbolt1.radAng, placingZone->cellsAtBottom [0][0].perLen / 2, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
				else
					moveIn3D ('y', pinbolt1.radAng, 0.200, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			}

			// 현재 셀이 유로폼, 다음 셀이 유로폼일 경우
			if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > EPS) && (placingZone->cellsAtBottom [0][xx].dirLen > EPS) && (placingZone->cellsAtBottom [0][xx+1].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx+1].perLen > EPS) && (placingZone->cellsAtBottom [0][xx+1].dirLen > EPS)) {
				moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtBottom [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
				pinbolt1.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
				pinbolt1.radAng -= DegreeToRad (90.0);
			}
		}
	}

	// 각파이프행거 (하부-L) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			hanger.init (L("각파이프행거.gsm"), layerInd_RectpipeHanger, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);
			moveIn3D ('z', hanger.radAng, -0.0635 * cos (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3D ('x', hanger.radAng, 0.0635 * sin (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "각파이프행거", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (90.0);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, -0.150 + remainLengthDouble - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "각파이프행거", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (90.0);
			tempLengthDouble = 0.0;
			for (zz = beginIndex ; zz <= beginIndex + round ((endIndex - beginIndex) / 2, 0) ; ++zz) {
				tempLengthDouble += placingZone->cellsAtBottom [0][zz].dirLen;
			}
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150 - remainLengthDouble + tempLengthDouble - 0.450, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "각파이프행거", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (90.0);

			bBeginFound = false;
		}
	}

	// 각파이프행거 (하부-R) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			hanger.init (L("각파이프행거.gsm"), layerInd_RectpipeHanger, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3D ('z', hanger.radAng, -0.0635 * cos (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3D ('x', hanger.radAng, 0.0635 * sin (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3D ('y', hanger.radAng, placingZone->cellsAtBottom [0][xx].perLen + placingZone->cellsAtBottom [1][xx].perLen + placingZone->cellsAtBottom [2][xx].perLen, &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (270.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "각파이프행거", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (270.0);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, -0.150 + remainLengthDouble - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (270.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "각파이프행거", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (270.0);
			tempLengthDouble = 0.0;
			for (zz = beginIndex ; zz <= beginIndex + round ((endIndex - beginIndex) / 2, 0) ; ++zz) {
				tempLengthDouble += placingZone->cellsAtBottom [0][zz].dirLen;
			}
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150 - remainLengthDouble + tempLengthDouble - 0.450, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (270.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "각파이프행거", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (270.0);

			bBeginFound = false;
		}
	}

	// 블루클램프 (L) - 1단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > 0) && (placingZone->cellsAtLSide [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [0][yy].dirLen;

			// 시작 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// 끝 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// 블루클램프 (L) - 2단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > 0) && (placingZone->cellsAtLSide [2][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [2][yy].dirLen;

			// 시작 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// 끝 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// 블루클램프 (R) - 1단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > 0) && (placingZone->cellsAtRSide [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [0][yy].dirLen;

			// 시작 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, 0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// 끝 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, 0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// 블루클램프 (R) - 2단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > 0) && (placingZone->cellsAtRSide [2][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [2][yy].dirLen;

			// 시작 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, 0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// 끝 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, 0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.050 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.050 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.500 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.500 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.400 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.400 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.350 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.350 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.300 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.300 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.300) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.200 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.200 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			} else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) < EPS) {
				moveIn3D ('z', blueClamp.radAng, 0.150 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				moveIn3D ('x', blueClamp.radAng, -0.150 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			}

			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) > EPS)
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// 블루클램프 (하부-L)
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			// 시작 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			
			if ( !((abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// 끝 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if ( !((abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// 블루클램프 (하부-R)
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [2][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [2][xx].perLen > 0) && (placingZone->cellsAtBottom [2][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [2][yy].dirLen;

			// 시작 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [2][beginIndex].leftBottomX, placingZone->cellsAtBottom [2][beginIndex].leftBottomY, placingZone->cellsAtBottom [2][beginIndex].leftBottomZ, placingZone->cellsAtBottom [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if ( !((abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// 끝 부분
			blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [2][beginIndex].leftBottomX, placingZone->cellsAtBottom [2][beginIndex].leftBottomY, placingZone->cellsAtBottom [2][beginIndex].leftBottomZ, placingZone->cellsAtBottom [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if ( !((abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "유로목재클램프(제작품v1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// 블루목심 (상부-L): 측면 아래에서 4번째 셀이 각재일 경우
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [3][xx].objType == TIMBER) || (placingZone->cellsAtLSide [3][xx].objType == PLYWOOD)) {
			blueTimberRail.init (L("블루목심v1.0.gsm"), layerInd_BlueTimberRail, infoBeam.floorInd, placingZone->cellsAtLSide [3][xx].leftBottomX, placingZone->cellsAtLSide [3][xx].leftBottomY, placingZone->cellsAtLSide [3][xx].leftBottomZ, placingZone->cellsAtLSide [3][xx].ang);

			if (placingZone->cellsAtLSide [3][xx].perLen < 0.040 - EPS) {
				// 앞으로 설치된 50*80 각재
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.050 - EPS)) {
				// 50*40 각재
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.080 - EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*50 각재
				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.080 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.100 + EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*80 각재
				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if (placingZone->cellsAtLSide [3][xx].perLen >= 0.100 - EPS) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.064, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 합판 및 제작틀
				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			}
		}
	}

	// 블루목심 (상부-R): 측면 아래에서 4번째 셀이 각재일 경우
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [3][xx].objType == TIMBER) || (placingZone->cellsAtRSide [3][xx].objType == PLYWOOD)) {
			blueTimberRail.init (L("블루목심v1.0.gsm"), layerInd_BlueTimberRail, infoBeam.floorInd, placingZone->cellsAtRSide [3][xx].leftBottomX, placingZone->cellsAtRSide [3][xx].leftBottomY, placingZone->cellsAtRSide [3][xx].leftBottomZ, placingZone->cellsAtRSide [3][xx].ang);

			if (placingZone->cellsAtRSide [3][xx].perLen < 0.040 - EPS) {
				// 앞으로 설치된 50*80 각재
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.050 - EPS)) {
				// 50*40 각재
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.080 - EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*50 각재
				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.080 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.100 + EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*80 각재
				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if (placingZone->cellsAtRSide [3][xx].perLen >= 0.100 - EPS) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.064, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 합판 및 제작틀
				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			}
		}
	}

	return	err;
}

// 벽과 테이블폼 사이에 단열재를 배치함 (보 측면)
GSErrCode	BeamTableformPlacingZone::placeInsulationsSide (BeamTableformPlacingZone* placingZone, InfoBeam* infoBeam, insulElemForBeamTableform* insulElem, bool bMoreWider)
{
	GSErrCode	err = NoError;

	// 회전 각도가 양수, 음수에 따라 파라미터에 전달할 경사 각도 변수
	double	minusSlantAngle = (placingZone->slantAngle >= EPS) ? DegreeToRad (360.0) - placingZone->slantAngle : -placingZone->slantAngle;

	short	xx, yy;
	short	totalXX, totalYY;
	double	horLen, verLen;
	double	remainHorLen, remainVerLen;

	EasyObjectPlacement insul;

	if (insulElem->bLimitSize == true) {
		// 가로/세로 크기 제한이 true일 때

		if (placingZone->gapSideLeft > EPS) {
			// 높은쪽
			// bMoreWider가 true이면 측면 단열재가 하부 단열재를 덮고, false이면 하부 단열재가 측면 단열재를 떠받침
			if (bMoreWider == true)
				remainHorLen = placingZone->areaHeight_Left + placingZone->gapBottom;
			else
				remainHorLen = placingZone->areaHeight_Left;
			remainVerLen = placingZone->beamLength;
			totalXX = (short)floor (remainHorLen / insulElem->maxHorLen);
			totalYY = (short)floor (remainVerLen / insulElem->maxVerLen);

			insul.init (L("단열재v1.0.gsm"), insulElem->layerInd, infoBeam->floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
			moveIn3D ('y', insul.radAng, placingZone->gapSideLeft, &insul.posX, &insul.posY, &insul.posZ);
			moveIn3D ('z', insul.radAng, (placingZone->areaHeight_Left + placingZone->gapBottom) * cos (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);
			moveIn3D ('x', insul.radAng, -(placingZone->areaHeight_Left + placingZone->gapBottom) * sin (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);

			for (xx = 0 ; xx < totalXX+1 ; ++xx) {
				for (yy = 0 ; yy < totalYY+1 ; ++yy) {
					(remainHorLen > insulElem->maxHorLen) ? horLen = insulElem->maxHorLen : horLen = remainHorLen;
					(remainVerLen > insulElem->maxVerLen) ? verLen = insulElem->maxVerLen : verLen = remainVerLen;

					elemList_Insulation.Push (insul.placeObject (10,
						"A", APIParT_Length, format_string ("%f", horLen).c_str(),
						"B", APIParT_Length, format_string ("%f", verLen).c_str(),
						"ZZYZX", APIParT_Length, format_string ("%f", insulElem->thk).c_str(),
						"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(),
						"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0) + minusSlantAngle).c_str(),
						"bRestrictSize", APIParT_Boolean, (insulElem->bLimitSize ? "1.0" : "0.0"),
						"maxHorLen", APIParT_Length, format_string ("%f", insulElem->maxHorLen).c_str(),
						"maxVerLen", APIParT_Length, format_string ("%f", insulElem->maxVerLen).c_str(),
						"bLShape", APIParT_Boolean, "0.0",
						"bVerticalCut", APIParT_Boolean, "0.0"));

					remainVerLen -= insulElem->maxVerLen;
					moveIn3DSlope ('x', insul.radAng, placingZone->slantAngle, verLen, &insul.posX, &insul.posY, &insul.posZ);
				}
				remainHorLen -= insulElem->maxHorLen;
				remainVerLen = placingZone->beamLength;
				moveIn3DSlope ('x', insul.radAng, placingZone->slantAngle, -placingZone->beamLength, &insul.posX, &insul.posY, &insul.posZ);
				moveIn3D ('z', insul.radAng, -horLen * cos (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);
				moveIn3D ('x', insul.radAng, horLen * sin (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);
			}
		}

		if (placingZone->gapSideRight > EPS) {
			// 낮은쪽
			if (bMoreWider == true)
				remainHorLen = placingZone->areaHeight_Left + placingZone->gapBottom;
			else
				remainHorLen = placingZone->areaHeight_Left;
			remainVerLen = placingZone->beamLength;
			totalXX = (short)floor (remainHorLen / insulElem->maxHorLen);
			totalYY = (short)floor (remainVerLen / insulElem->maxVerLen);

			insul.init (L("단열재v1.0.gsm"), insulElem->layerInd, infoBeam->floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
			moveIn3D ('y', insul.radAng, placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight), &insul.posX, &insul.posY, &insul.posZ);
			moveIn3D ('z', insul.radAng, (placingZone->areaHeight_Left + placingZone->gapBottom) * cos (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);
			moveIn3D ('x', insul.radAng, -(placingZone->areaHeight_Left + placingZone->gapBottom) * sin (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);

			for (xx = 0 ; xx < totalXX+1 ; ++xx) {
				for (yy = 0 ; yy < totalYY+1 ; ++yy) {
					(remainHorLen > insulElem->maxHorLen) ? horLen = insulElem->maxHorLen : horLen = remainHorLen;
					(remainVerLen > insulElem->maxVerLen) ? verLen = insulElem->maxVerLen : verLen = remainVerLen;

					elemList_Insulation.Push (insul.placeObject (10,
						"A", APIParT_Length, format_string ("%f", horLen).c_str(),
						"B", APIParT_Length, format_string ("%f", verLen).c_str(),
						"ZZYZX", APIParT_Length, format_string ("%f", insulElem->thk).c_str(),
						"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(),
						"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0) + minusSlantAngle).c_str(),
						"bRestrictSize", APIParT_Boolean, (insulElem->bLimitSize ? "1.0" : "0.0"),
						"maxHorLen", APIParT_Length, format_string ("%f", insulElem->maxHorLen).c_str(),
						"maxVerLen", APIParT_Length, format_string ("%f", insulElem->maxVerLen).c_str(),
						"bLShape", APIParT_Boolean, "0.0",
						"bVerticalCut", APIParT_Boolean, "0.0"));

					remainVerLen -= insulElem->maxVerLen;
					moveIn3DSlope ('x', insul.radAng, placingZone->slantAngle, verLen, &insul.posX, &insul.posY, &insul.posZ);
				}
				remainHorLen -= insulElem->maxHorLen;
				remainVerLen = placingZone->beamLength;
				moveIn3DSlope ('x', insul.radAng, placingZone->slantAngle, -placingZone->beamLength, &insul.posX, &insul.posY, &insul.posZ);
				moveIn3D ('z', insul.radAng, -horLen * cos (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);
				moveIn3D ('x', insul.radAng, horLen * sin (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);
			}
		}
	} else {
		// 가로/세로 크기 제한이 false일 때

		if (placingZone->gapSideLeft > EPS) {
			// 높은쪽
			// bMoreWider가 true이면 측면 단열재가 하부 단열재를 덮고, false이면 하부 단열재가 측면 단열재를 떠받침
			if (bMoreWider == true)
				horLen = placingZone->areaHeight_Left + placingZone->gapBottom;
			else
				horLen = placingZone->areaHeight_Left;
			verLen = placingZone->beamLength;

			insul.init (L("단열재v1.0.gsm"), insulElem->layerInd, infoBeam->floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
			moveIn3D ('y', insul.radAng, placingZone->gapSideLeft, &insul.posX, &insul.posY, &insul.posZ);
			moveIn3D ('z', insul.radAng, (placingZone->areaHeight_Left + placingZone->gapBottom) * cos (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);
			moveIn3D ('x', insul.radAng, -(placingZone->areaHeight_Left + placingZone->gapBottom) * sin (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);

			elemList_Insulation.Push (insul.placeObject (10,
				"A", APIParT_Length, format_string ("%f", horLen).c_str(),
				"B", APIParT_Length, format_string ("%f", verLen).c_str(),
				"ZZYZX", APIParT_Length, format_string ("%f", insulElem->thk).c_str(),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0) + minusSlantAngle).c_str(),
				"bRestrictSize", APIParT_Boolean, (insulElem->bLimitSize ? "1.0" : "0.0"),
				"maxHorLen", APIParT_Length, format_string ("%f", insulElem->maxHorLen).c_str(),
				"maxVerLen", APIParT_Length, format_string ("%f", insulElem->maxVerLen).c_str(),
				"bLShape", APIParT_Boolean, "0.0",
				"bVerticalCut", APIParT_Boolean, "0.0"));
		}

		if (placingZone->gapSideRight > EPS) {
			// 낮은쪽
			if (bMoreWider == true)
				horLen = placingZone->areaHeight_Left + placingZone->gapBottom;
			else
				horLen = placingZone->areaHeight_Left;
			verLen = placingZone->beamLength;

			insul.init (L("단열재v1.0.gsm"), insulElem->layerInd, infoBeam->floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
			moveIn3D ('y', insul.radAng, placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight), &insul.posX, &insul.posY, &insul.posZ);
			moveIn3D ('z', insul.radAng, (placingZone->areaHeight_Left + placingZone->gapBottom) * cos (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);
			moveIn3D ('x', insul.radAng, -(placingZone->areaHeight_Left + placingZone->gapBottom) * sin (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);

			elemList_Insulation.Push (insul.placeObject (10,
				"A", APIParT_Length, format_string ("%f", horLen).c_str(),
				"B", APIParT_Length, format_string ("%f", verLen).c_str(),
				"ZZYZX", APIParT_Length, format_string ("%f", insulElem->thk).c_str(),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0) + minusSlantAngle).c_str(),
				"bRestrictSize", APIParT_Boolean, (insulElem->bLimitSize ? "1.0" : "0.0"),
				"maxHorLen", APIParT_Length, format_string ("%f", insulElem->maxHorLen).c_str(),
				"maxVerLen", APIParT_Length, format_string ("%f", insulElem->maxVerLen).c_str(),
				"bLShape", APIParT_Boolean, "0.0",
				"bVerticalCut", APIParT_Boolean, "0.0"));
		}
	}

	return	err;
}

// 벽과 테이블폼 사이에 단열재를 배치함 (보 하부)
GSErrCode	BeamTableformPlacingZone::placeInsulationsBottom (BeamTableformPlacingZone* placingZone, InfoBeam* infoBeam, insulElemForBeamTableform* insulElem, bool bMoreWider)
{
	GSErrCode	err = NoError;

	// 회전 각도가 양수, 음수에 따라 파라미터에 전달할 경사 각도 변수
	double	minusSlantAngle = (placingZone->slantAngle >= EPS) ? DegreeToRad (360.0) - placingZone->slantAngle : -placingZone->slantAngle;

	short	xx, yy;
	short	totalXX, totalYY;
	double	horLen, verLen;
	double	remainHorLen, remainVerLen;

	EasyObjectPlacement insul;

	if (insulElem->bLimitSize == true) {
		// 가로/세로 크기 제한이 true일 때
		if (bMoreWider == true)
			remainHorLen = placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight);
		else
			remainHorLen = placingZone->areaWidth_Bottom;
		remainVerLen = placingZone->beamLength;
		totalXX = (short)floor (remainHorLen / insulElem->maxHorLen);
		totalYY = (short)floor (remainVerLen / insulElem->maxVerLen);

		insul.init (L("단열재v1.0.gsm"), insulElem->layerInd, infoBeam->floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
		if (bMoreWider == false)
			moveIn3D ('y', insul.radAng, placingZone->gapSideLeft, &insul.posX, &insul.posY, &insul.posZ);
		moveIn3D ('z', insul.radAng, placingZone->gapBottom * cos (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);
		moveIn3D ('x', insul.radAng, -placingZone->gapBottom * sin (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);

		for (xx = 0 ; xx < totalXX+1 ; ++xx) {
			for (yy = 0 ; yy < totalYY+1 ; ++yy) {
				(remainHorLen > insulElem->maxHorLen) ? horLen = insulElem->maxHorLen : horLen = remainHorLen;
				(remainVerLen > insulElem->maxVerLen) ? verLen = insulElem->maxVerLen : verLen = remainVerLen;

				insul.radAng += DegreeToRad (90.0);
				elemList_Insulation.Push (insul.placeObject (10,
					"A", APIParT_Length, format_string ("%f", horLen).c_str(),
					"B", APIParT_Length, format_string ("%f", verLen).c_str(),
					"ZZYZX", APIParT_Length, format_string ("%f", insulElem->thk).c_str(),
					"angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(),
					"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(),
					"bRestrictSize", APIParT_Boolean, (insulElem->bLimitSize ? "1.0" : "0.0"),
					"maxHorLen", APIParT_Length, format_string ("%f", insulElem->maxHorLen).c_str(),
					"maxVerLen", APIParT_Length, format_string ("%f", insulElem->maxVerLen).c_str(),
					"bLShape", APIParT_Boolean, "0.0",
					"bVerticalCut", APIParT_Boolean, "0.0"));
				insul.radAng -= DegreeToRad (90.0);

				remainVerLen -= insulElem->maxVerLen;
				moveIn3DSlope ('x', insul.radAng, placingZone->slantAngle, verLen, &insul.posX, &insul.posY, &insul.posZ);
			}
			remainHorLen -= insulElem->maxHorLen;
			remainVerLen = placingZone->beamLength;
			moveIn3DSlope ('x', insul.radAng, placingZone->slantAngle, -placingZone->beamLength, &insul.posX, &insul.posY, &insul.posZ);
			moveIn3D ('y', insul.radAng, horLen, &insul.posX, &insul.posY, &insul.posZ);
		}
	} else {
		// 가로/세로 크기 제한이 false일 때
		if (bMoreWider == true)
			horLen = placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight);
		else
			horLen = placingZone->areaWidth_Bottom;
		verLen = placingZone->beamLength;

		insul.init (L("단열재v1.0.gsm"), insulElem->layerInd, infoBeam->floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
		if (bMoreWider == false)
			moveIn3D ('y', insul.radAng, placingZone->gapSideLeft, &insul.posX, &insul.posY, &insul.posZ);
		moveIn3D ('z', insul.radAng, placingZone->gapBottom * cos (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);
		moveIn3D ('x', insul.radAng, -placingZone->gapBottom * sin (placingZone->slantAngle), &insul.posX, &insul.posY, &insul.posZ);

		insul.radAng += DegreeToRad (90.0);
		elemList_Insulation.Push (insul.placeObject (10,
			"A", APIParT_Length, format_string ("%f", horLen).c_str(),
			"B", APIParT_Length, format_string ("%f", verLen).c_str(),
			"ZZYZX", APIParT_Length, format_string ("%f", insulElem->thk).c_str(),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(),
			"bRestrictSize", APIParT_Boolean, (insulElem->bLimitSize ? "1.0" : "0.0"),
			"maxHorLen", APIParT_Length, format_string ("%f", insulElem->maxHorLen).c_str(),
			"maxVerLen", APIParT_Length, format_string ("%f", insulElem->maxVerLen).c_str(),
			"bLShape", APIParT_Boolean, "0.0",
			"bVerticalCut", APIParT_Boolean, "0.0"));
		insul.radAng -= DegreeToRad (90.0);
	}

	return	err;
}

// 동바리/멍에제 프리셋을 배치함
GSErrCode	BeamTableformPlacingZone::placeSupportingPostPreset (BeamTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx;
	double	distance;

	double	heightGapBeamBtwPost;
	char	postType [16];

	// 보 하부면의 고도와 동바리 상부면 고도와의 차이
	if (placingZone->typeOfSupportingPost == 1)
		heightGapBeamBtwPost = 0.3565;	// PERI 동바리 + GT24 거더의 경우
	else if (placingZone->typeOfSupportingPost == 2)
		heightGapBeamBtwPost = 0.2185;	// PERI 동바리 + 보 멍에제

	// 수직재 규격 결정하기
	strcpy (postType, format_string ("%s", "MP 350").c_str());

	EasyObjectPlacement	verticalPost, horizontalPost, girder, beamBracket, yoke, timber, jackSupport;

	// PERI 동바리 + GT24 거더
	if (placingZone->typeOfSupportingPost == 1) {
		// 배치: PERI동바리 수직재
		verticalPost.init (L("PERI동바리 수직재 v0.1.gsm"), layerInd_VerticalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
		moveIn3D ('z', verticalPost.radAng, -0.003 - 0.1135 - 0.240 - placingZone->gapBottom, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		moveIn3D ('x', verticalPost.radAng, placingZone->postStartOffset, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		moveIn3D ('y', verticalPost.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - placingZone->gapSideLeft, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
		moveIn3D ('y', verticalPost.radAng, placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
		moveIn3D ('x', verticalPost.radAng, placingZone->postGapLength, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
		moveIn3D ('y', verticalPost.radAng, -placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));

		// 배치: GT24 거더
		girder.init (L("GT24 거더 v1.0.gsm"), layerInd_Girder, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
		moveIn3D ('z', girder.radAng, -0.1135 - 0.240 - placingZone->gapBottom, &girder.posX, &girder.posY, &girder.posZ);
		moveIn3D ('x', girder.radAng, placingZone->postStartOffset, &girder.posX, &girder.posY, &girder.posZ);
		moveIn3D ('y', girder.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - 0.160 - placingZone->gapSideLeft, &girder.posX, &girder.posY, &girder.posZ);
		girder.radAng += DegreeToRad (90.0);
		if (abs (placingZone->postGapWidth - 0.900) < EPS) {
			elemList_SupportingPost.Push (girder.placeObject (2, "type", APIParT_CString, "1200", "length", APIParT_Length, format_string ("%f", 1.214).c_str()));
		} else if (abs (placingZone->postGapWidth - 1.200) < EPS) {
			elemList_SupportingPost.Push (girder.placeObject (2, "type", APIParT_CString, "1500", "length", APIParT_Length, format_string ("%f", 1.510).c_str()));
		} else if (abs (placingZone->postGapWidth - 1.500) < EPS) {
			elemList_SupportingPost.Push (girder.placeObject (2, "type", APIParT_CString, "1800", "length", APIParT_Length, format_string ("%f", 1.806).c_str()));
		}
		girder.radAng -= DegreeToRad (90.0);
		moveIn3D ('x', girder.radAng, placingZone->postGapLength, &girder.posX, &girder.posY, &girder.posZ);
		girder.radAng += DegreeToRad (90.0);
		if (abs (placingZone->postGapWidth - 0.900) < EPS) {
			elemList_SupportingPost.Push (girder.placeObject (2, "type", APIParT_CString, "1200", "length", APIParT_Length, format_string ("%f", 1.214).c_str()));
		} else if (abs (placingZone->postGapWidth - 1.200) < EPS) {
			elemList_SupportingPost.Push (girder.placeObject (2, "type", APIParT_CString, "1500", "length", APIParT_Length, format_string ("%f", 1.510).c_str()));
		} else if (abs (placingZone->postGapWidth - 1.500) < EPS) {
			elemList_SupportingPost.Push (girder.placeObject (2, "type", APIParT_CString, "1800", "length", APIParT_Length, format_string ("%f", 1.806).c_str()));
		}
		girder.radAng -= DegreeToRad (90.0);

		// 배치: 보 브라켓
		beamBracket.init (L("블루 보 브라켓 v1.0.gsm"), layerInd_BeamBracket, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
		moveIn3D ('x', beamBracket.radAng, placingZone->postStartOffset, &beamBracket.posX, &beamBracket.posY, &beamBracket.posZ);
		moveIn3D ('y', beamBracket.radAng, -0.1135 - placingZone->gapSideLeft, &beamBracket.posX, &beamBracket.posY, &beamBracket.posZ);
		moveIn3D ('z', beamBracket.radAng, -0.1135 - placingZone->gapBottom, &beamBracket.posX, &beamBracket.posY, &beamBracket.posZ);
		beamBracket.radAng += DegreeToRad (270.0);
		elemList_SupportingPost.Push (beamBracket.placeObject (2, "type", APIParT_CString, "730", "verticalHeight", APIParT_Length, format_string ("%f", 0.500).c_str()));
		beamBracket.radAng -= DegreeToRad (270.0);
		moveIn3D ('x', beamBracket.radAng, placingZone->postGapLength, &beamBracket.posX, &beamBracket.posY, &beamBracket.posZ);
		beamBracket.radAng += DegreeToRad (270.0);
		elemList_SupportingPost.Push (beamBracket.placeObject (2, "type", APIParT_CString, "730", "verticalHeight", APIParT_Length, format_string ("%f", 0.500).c_str()));
		beamBracket.radAng -= DegreeToRad (270.0);
		moveIn3D ('y', beamBracket.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight)) + (0.1135 * 2), &beamBracket.posX, &beamBracket.posY, &beamBracket.posZ);
		beamBracket.radAng += DegreeToRad (90.0);
		elemList_SupportingPost.Push (beamBracket.placeObject (2, "type", APIParT_CString, "730", "verticalHeight", APIParT_Length, format_string ("%f", 0.500).c_str()));
		beamBracket.radAng -= DegreeToRad (90.0);
		moveIn3D ('x', beamBracket.radAng, -placingZone->postGapLength, &beamBracket.posX, &beamBracket.posY, &beamBracket.posZ);
		beamBracket.radAng += DegreeToRad (90.0);
		elemList_SupportingPost.Push (beamBracket.placeObject (2, "type", APIParT_CString, "730", "verticalHeight", APIParT_Length, format_string ("%f", 0.500).c_str()));
		beamBracket.radAng -= DegreeToRad (90.0);

	// PERI 동바리 + 보 멍에제
	} else if (placingZone->typeOfSupportingPost == 2) {
		// 배치: PERI동바리 수직재
		verticalPost.init (L("PERI동바리 수직재 v0.1.gsm"), layerInd_VerticalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
		moveIn3D ('z', verticalPost.radAng, -0.1135 - 0.240 + 0.135 - placingZone->gapBottom, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		moveIn3D ('x', verticalPost.radAng, placingZone->postStartOffset, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		moveIn3D ('y', verticalPost.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - placingZone->gapSideLeft, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
		moveIn3D ('y', verticalPost.radAng, placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
		moveIn3D ('x', verticalPost.radAng, placingZone->postGapLength, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
		moveIn3D ('y', verticalPost.radAng, -placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));

		// 배치: 보 멍에제
		if (abs (placingZone->postGapWidth - 0.900) < EPS)
			distance = 0.0015 + (placingZone->gapSideLeft + placingZone->gapSideRight);
		else if (abs (placingZone->postGapWidth - 1.200) < EPS)
			distance = 0.0015 + (placingZone->gapSideLeft + placingZone->gapSideRight);
		else if (abs (placingZone->postGapWidth - 1.500) < EPS)
			distance = 0.1515 + (placingZone->gapSideLeft + placingZone->gapSideRight);

		yoke.init (L("보 멍에제v1.0.gsm"), layerInd_Yoke, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
		moveIn3D ('x', yoke.radAng, placingZone->postStartOffset + 0.075, &yoke.posX, &yoke.posY, &yoke.posZ);
		moveIn3D ('y', yoke.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - 0.260 - placingZone->gapSideLeft, &yoke.posX, &yoke.posY, &yoke.posZ);
		moveIn3D ('z', yoke.radAng, -0.2635 - placingZone->gapBottom, &yoke.posX, &yoke.posY, &yoke.posZ);
		yoke.radAng += DegreeToRad (90.0);
		if (abs (placingZone->postGapWidth - 0.900) < EPS) {
			moveIn3D ('y', yoke.radAng - DegreeToRad (90.0), -0.150, &yoke.posX, &yoke.posY, &yoke.posZ);
			elemList_SupportingPost.Push (yoke.placeObject (5, "beamLength", APIParT_Length, format_string ("%f", 1.500).c_str(), "verticalGap", APIParT_Length, format_string ("%f", placingZone->postGapWidth).c_str(), "innerVerticalLen", APIParT_Length, "0.700", "LbarHdist", APIParT_Length, format_string ("%f", distance).c_str(), "RbarHdist", APIParT_Length, format_string ("%f", distance).c_str()));
		} else {
			elemList_SupportingPost.Push (yoke.placeObject (5, "beamLength", APIParT_Length, format_string ("%f", placingZone->postGapWidth + 0.300).c_str(), "verticalGap", APIParT_Length, format_string ("%f", placingZone->postGapWidth).c_str(), "innerVerticalLen", APIParT_Length, "0.700", "LbarHdist", APIParT_Length, format_string ("%f", distance).c_str(), "RbarHdist", APIParT_Length, format_string ("%f", distance).c_str()));
		}
		yoke.radAng -= DegreeToRad (90.0);
		moveIn3D ('x', yoke.radAng, placingZone->postGapLength, &yoke.posX, &yoke.posY, &yoke.posZ);
		yoke.radAng += DegreeToRad (90.0);
		if (abs (placingZone->postGapWidth - 0.900) < EPS) {
			elemList_SupportingPost.Push (yoke.placeObject (5, "beamLength", APIParT_Length, format_string ("%f", 1.500).c_str(), "verticalGap", APIParT_Length, format_string ("%f", placingZone->postGapWidth).c_str(), "innerVerticalLen", APIParT_Length, "0.700", "LbarHdist", APIParT_Length, format_string ("%f", distance).c_str(), "RbarHdist", APIParT_Length, format_string ("%f", distance).c_str()));
		} else {
			elemList_SupportingPost.Push (yoke.placeObject (5, "beamLength", APIParT_Length, format_string ("%f", placingZone->postGapWidth + 0.300).c_str(), "verticalGap", APIParT_Length, format_string ("%f", placingZone->postGapWidth).c_str(), "innerVerticalLen", APIParT_Length, "0.700", "LbarHdist", APIParT_Length, format_string ("%f", distance).c_str(), "RbarHdist", APIParT_Length, format_string ("%f", distance).c_str()));
		}
		yoke.radAng -= DegreeToRad (90.0);
	}

	// 배치: PERI동바리 수평재 (너비 방향)
	if (placingZone->typeOfSupportingPost == 1)
		distance = -0.003 - 0.1135 - 0.240;
	else if (placingZone->typeOfSupportingPost == 2)
		distance = -0.1135 - 0.240 + 0.135;

	distance -= 1.500;

	horizontalPost.init (L("PERI동바리 수평재 v0.2.gsm"), layerInd_HorizontalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
	moveIn3D ('x', horizontalPost.radAng, placingZone->postStartOffset, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
	moveIn3D ('y', horizontalPost.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) + placingZone->postGapWidth) / 2 - 0.050 - placingZone->gapSideLeft, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
	moveIn3D ('z', horizontalPost.radAng, distance - placingZone->gapBottom, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
	if (abs (placingZone->postGapWidth - 0.900) < EPS) {
		horizontalPost.radAng += DegreeToRad (270.0);
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "90 cm", "lenFrame", APIParT_Length, "0.800"));
		horizontalPost.radAng -= DegreeToRad (270.0);
	} else if (abs (placingZone->postGapWidth - 1.200) < EPS) {
		horizontalPost.radAng += DegreeToRad (270.0);
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "120 cm", "lenFrame", APIParT_Length, "1.100"));
		horizontalPost.radAng -= DegreeToRad (270.0);
	} else if (abs (placingZone->postGapWidth - 1.500) < EPS) {
		horizontalPost.radAng += DegreeToRad (270.0);
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "150 cm", "lenFrame", APIParT_Length, "1.400"));
		horizontalPost.radAng -= DegreeToRad (270.0);
	}
	moveIn3D ('x', horizontalPost.radAng, placingZone->postGapLength, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
	moveIn3D ('y', horizontalPost.radAng, -placingZone->postGapWidth + 0.100, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
	if (abs (placingZone->postGapWidth - 0.900) < EPS) {
		horizontalPost.radAng += DegreeToRad (90.0);
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "90 cm", "lenFrame", APIParT_Length, "0.800"));
		horizontalPost.radAng -= DegreeToRad (90.0);
	} else if (abs (placingZone->postGapWidth - 1.200) < EPS) {
		horizontalPost.radAng += DegreeToRad (90.0);
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "120 cm", "lenFrame", APIParT_Length, "1.100"));
		horizontalPost.radAng -= DegreeToRad (90.0);
	} else if (abs (placingZone->postGapWidth - 1.500) < EPS) {
		horizontalPost.radAng += DegreeToRad (90.0);
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "150 cm", "lenFrame", APIParT_Length, "1.400"));
		horizontalPost.radAng -= DegreeToRad (90.0);
	}

	// 배치: PERI동바리 수평재 (길이 방향)
	horizontalPost.init (L("PERI동바리 수평재 v0.2.gsm"), layerInd_HorizontalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
	moveIn3D ('x', horizontalPost.radAng, placingZone->postStartOffset + 0.050, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
	moveIn3D ('y', horizontalPost.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - placingZone->gapSideLeft, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
	moveIn3D ('z', horizontalPost.radAng, distance - placingZone->gapBottom, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
	if (abs (placingZone->postGapLength - 1.200) < EPS) {
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "120 cm", "lenFrame", APIParT_Length, "1.100"));
	} else if (abs (placingZone->postGapLength - 1.500) < EPS) {
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "150 cm", "lenFrame", APIParT_Length, "1.400"));
	} else if (abs (placingZone->postGapLength - 2.015) < EPS) {
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "201.5 cm", "lenFrame", APIParT_Length, "1.915"));
	} else if (abs (placingZone->postGapLength - 2.300) < EPS) {
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "230 cm", "lenFrame", APIParT_Length, "2.200"));
	}
	moveIn3D ('x', horizontalPost.radAng, placingZone->postGapLength - 0.100, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
	moveIn3D ('y', horizontalPost.radAng, placingZone->postGapWidth, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
	if (abs (placingZone->postGapLength - 1.200) < EPS) {
		horizontalPost.radAng += DegreeToRad (180.0);
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "120 cm", "lenFrame", APIParT_Length, "1.100"));
		horizontalPost.radAng -= DegreeToRad (180.0);
	} else if (abs (placingZone->postGapLength - 1.500) < EPS) {
		horizontalPost.radAng += DegreeToRad (180.0);
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "150 cm", "lenFrame", APIParT_Length, "1.400"));
		horizontalPost.radAng -= DegreeToRad (180.0);
	} else if (abs (placingZone->postGapLength - 2.015) < EPS) {
		horizontalPost.radAng += DegreeToRad (180.0);
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "201.5 cm", "lenFrame", APIParT_Length, "1.915"));
		horizontalPost.radAng -= DegreeToRad (180.0);
	} else if (abs (placingZone->postGapLength - 2.300) < EPS) {
		horizontalPost.radAng += DegreeToRad (180.0);
		elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "230 cm", "lenFrame", APIParT_Length, "2.200"));
		horizontalPost.radAng -= DegreeToRad (180.0);
	}

	if (placingZone->numOfSupportingPostSet == 2) {
		// PERI 동바리 + GT24 거더
		if (placingZone->typeOfSupportingPost == 1) {
			// 배치: PERI동바리 수직재
			verticalPost.init (L("PERI동바리 수직재 v0.1.gsm"), layerInd_VerticalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
			moveIn3D ('z', verticalPost.radAng, -0.003 - 0.1135 - 0.240 - placingZone->gapBottom, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			moveIn3D ('x', verticalPost.radAng, placingZone->beamLength - placingZone->postGapLength - placingZone->postStartOffset, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			moveIn3D ('y', verticalPost.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - placingZone->gapSideLeft, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
			moveIn3D ('y', verticalPost.radAng, placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
			moveIn3D ('x', verticalPost.radAng, placingZone->postGapLength, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
			moveIn3D ('y', verticalPost.radAng, -placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));

			// 배치: GT24 거더
			girder.init (L("GT24 거더 v1.0.gsm"), layerInd_Girder, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
			moveIn3D ('z', girder.radAng, -0.1135 - 0.240 - placingZone->gapBottom, &girder.posX, &girder.posY, &girder.posZ);
			moveIn3D ('x', girder.radAng, placingZone->beamLength - placingZone->postGapLength - placingZone->postStartOffset, &girder.posX, &girder.posY, &girder.posZ);
			moveIn3D ('y', girder.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - 0.160 - placingZone->gapSideLeft, &girder.posX, &girder.posY, &girder.posZ);
			girder.radAng += DegreeToRad (90.0);
			if (abs (placingZone->postGapWidth - 0.900) < EPS) {
				elemList_SupportingPost.Push (girder.placeObject (2, "type", APIParT_CString, "1200", "length", APIParT_Length, format_string ("%f", 1.214).c_str()));
			} else if (abs (placingZone->postGapWidth - 1.200) < EPS) {
				elemList_SupportingPost.Push (girder.placeObject (2, "type", APIParT_CString, "1500", "length", APIParT_Length, format_string ("%f", 1.510).c_str()));
			} else if (abs (placingZone->postGapWidth - 1.500) < EPS) {
				elemList_SupportingPost.Push (girder.placeObject (2, "type", APIParT_CString, "1800", "length", APIParT_Length, format_string ("%f", 1.806).c_str()));
			}
			girder.radAng -= DegreeToRad (90.0);
			moveIn3D ('x', girder.radAng, placingZone->postGapLength, &girder.posX, &girder.posY, &girder.posZ);
			girder.radAng += DegreeToRad (90.0);
			if (abs (placingZone->postGapWidth - 0.900) < EPS) {
				elemList_SupportingPost.Push (girder.placeObject (2, "type", APIParT_CString, "1200", "length", APIParT_Length, format_string ("%f", 1.214).c_str()));
			} else if (abs (placingZone->postGapWidth - 1.200) < EPS) {
				elemList_SupportingPost.Push (girder.placeObject (2, "type", APIParT_CString, "1500", "length", APIParT_Length, format_string ("%f", 1.510).c_str()));
			} else if (abs (placingZone->postGapWidth - 1.500) < EPS) {
				elemList_SupportingPost.Push (girder.placeObject (2, "type", APIParT_CString, "1800", "length", APIParT_Length, format_string ("%f", 1.806).c_str()));
			}
			girder.radAng -= DegreeToRad (90.0);

			// 배치: 보 브라켓
			beamBracket.init (L("블루 보 브라켓 v1.0.gsm"), layerInd_BeamBracket, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
			moveIn3D ('x', beamBracket.radAng, placingZone->beamLength - placingZone->postGapLength - placingZone->postStartOffset, &beamBracket.posX, &beamBracket.posY, &beamBracket.posZ);
			moveIn3D ('y', beamBracket.radAng, -0.1135 - placingZone->gapSideLeft, &beamBracket.posX, &beamBracket.posY, &beamBracket.posZ);
			moveIn3D ('z', beamBracket.radAng, -0.1135 - placingZone->gapBottom, &beamBracket.posX, &beamBracket.posY, &beamBracket.posZ);
			beamBracket.radAng += DegreeToRad (270.0);
			elemList_SupportingPost.Push (beamBracket.placeObject (2, "type", APIParT_CString, "730", "verticalHeight", APIParT_Length, format_string ("%f", 0.500).c_str()));
			beamBracket.radAng -= DegreeToRad (270.0);
			moveIn3D ('x', beamBracket.radAng, placingZone->postGapLength, &beamBracket.posX, &beamBracket.posY, &beamBracket.posZ);
			beamBracket.radAng += DegreeToRad (270.0);
			elemList_SupportingPost.Push (beamBracket.placeObject (2, "type", APIParT_CString, "730", "verticalHeight", APIParT_Length, format_string ("%f", 0.500).c_str()));
			beamBracket.radAng -= DegreeToRad (270.0);
			moveIn3D ('y', beamBracket.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight)) + (0.1135 * 2), &beamBracket.posX, &beamBracket.posY, &beamBracket.posZ);
			beamBracket.radAng += DegreeToRad (90.0);
			elemList_SupportingPost.Push (beamBracket.placeObject (2, "type", APIParT_CString, "730", "verticalHeight", APIParT_Length, format_string ("%f", 0.500).c_str()));
			beamBracket.radAng -= DegreeToRad (90.0);
			moveIn3D ('x', beamBracket.radAng, -placingZone->postGapLength, &beamBracket.posX, &beamBracket.posY, &beamBracket.posZ);
			beamBracket.radAng += DegreeToRad (90.0);
			elemList_SupportingPost.Push (beamBracket.placeObject (2, "type", APIParT_CString, "730", "verticalHeight", APIParT_Length, format_string ("%f", 0.500).c_str()));
			beamBracket.radAng -= DegreeToRad (90.0);

		// PERI 동바리 + 보 멍에제
		} else if (placingZone->typeOfSupportingPost == 2) {
			// 배치: PERI동바리 수직재
			verticalPost.init (L("PERI동바리 수직재 v0.1.gsm"), layerInd_VerticalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
			moveIn3D ('z', verticalPost.radAng, -0.1135 - 0.240 + 0.135 - placingZone->gapBottom, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			moveIn3D ('x', verticalPost.radAng, placingZone->beamLength - placingZone->postGapLength - placingZone->postStartOffset, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			moveIn3D ('y', verticalPost.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - placingZone->gapSideLeft, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
			moveIn3D ('y', verticalPost.radAng, placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
			moveIn3D ('x', verticalPost.radAng, placingZone->postGapLength, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
			moveIn3D ('y', verticalPost.radAng, -placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));

			// 배치: 보 멍에제
			if (abs (placingZone->postGapWidth - 0.900) < EPS)
				distance = 0.0015 + (placingZone->gapSideLeft + placingZone->gapSideRight);
			else if (abs (placingZone->postGapWidth - 1.200) < EPS)
				distance = 0.0015 + (placingZone->gapSideLeft + placingZone->gapSideRight);
			else if (abs (placingZone->postGapWidth - 1.500) < EPS)
				distance = 0.1515 + (placingZone->gapSideLeft + placingZone->gapSideRight);

			yoke.init (L("보 멍에제v1.0.gsm"), layerInd_Yoke, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
			moveIn3D ('x', yoke.radAng, placingZone->beamLength - placingZone->postGapLength - placingZone->postStartOffset + 0.075, &yoke.posX, &yoke.posY, &yoke.posZ);
			moveIn3D ('y', yoke.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - 0.260 - placingZone->gapSideLeft, &yoke.posX, &yoke.posY, &yoke.posZ);
			moveIn3D ('z', yoke.radAng, -0.2635 - placingZone->gapBottom, &yoke.posX, &yoke.posY, &yoke.posZ);
			yoke.radAng += DegreeToRad (90.0);
			if (abs (placingZone->postGapWidth - 0.900) < EPS) {
				moveIn3D ('y', yoke.radAng - DegreeToRad (90.0), -0.150, &yoke.posX, &yoke.posY, &yoke.posZ);
				elemList_SupportingPost.Push (yoke.placeObject (5, "beamLength", APIParT_Length, format_string ("%f", 1.500).c_str(), "verticalGap", APIParT_Length, format_string ("%f", placingZone->postGapWidth).c_str(), "innerVerticalLen", APIParT_Length, "0.700", "LbarHdist", APIParT_Length, format_string ("%f", distance).c_str(), "RbarHdist", APIParT_Length, format_string ("%f", distance).c_str()));
			} else {
				elemList_SupportingPost.Push (yoke.placeObject (5, "beamLength", APIParT_Length, format_string ("%f", placingZone->postGapWidth + 0.300).c_str(), "verticalGap", APIParT_Length, format_string ("%f", placingZone->postGapWidth).c_str(), "innerVerticalLen", APIParT_Length, "0.700", "LbarHdist", APIParT_Length, format_string ("%f", distance).c_str(), "RbarHdist", APIParT_Length, format_string ("%f", distance).c_str()));
			}
			yoke.radAng -= DegreeToRad (90.0);
			moveIn3D ('x', yoke.radAng, placingZone->postGapLength, &yoke.posX, &yoke.posY, &yoke.posZ);
			yoke.radAng += DegreeToRad (90.0);
			if (abs (placingZone->postGapWidth - 0.900) < EPS) {
				elemList_SupportingPost.Push (yoke.placeObject (5, "beamLength", APIParT_Length, format_string ("%f", 1.500).c_str(), "verticalGap", APIParT_Length, format_string ("%f", placingZone->postGapWidth).c_str(), "innerVerticalLen", APIParT_Length, "0.700", "LbarHdist", APIParT_Length, format_string ("%f", distance).c_str(), "RbarHdist", APIParT_Length, format_string ("%f", distance).c_str()));
			} else {
				elemList_SupportingPost.Push (yoke.placeObject (5, "beamLength", APIParT_Length, format_string ("%f", placingZone->postGapWidth + 0.300).c_str(), "verticalGap", APIParT_Length, format_string ("%f", placingZone->postGapWidth).c_str(), "innerVerticalLen", APIParT_Length, "0.700", "LbarHdist", APIParT_Length, format_string ("%f", distance).c_str(), "RbarHdist", APIParT_Length, format_string ("%f", distance).c_str()));
			}
			yoke.radAng -= DegreeToRad (90.0);
		}

		// 배치: PERI동바리 수평재 (너비 방향)
		if (placingZone->typeOfSupportingPost == 1)
			distance = -0.003 - 0.1135 - 0.240;
		else if (placingZone->typeOfSupportingPost == 2)
			distance = -0.1135 - 0.240 + 0.135;

		distance -= 1.500;

		horizontalPost.init (L("PERI동바리 수평재 v0.2.gsm"), layerInd_HorizontalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
		moveIn3D ('x', horizontalPost.radAng, placingZone->beamLength - placingZone->postGapLength - placingZone->postStartOffset, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
		moveIn3D ('y', horizontalPost.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) + placingZone->postGapWidth) / 2 - 0.050 - placingZone->gapSideLeft, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
		moveIn3D ('z', horizontalPost.radAng, distance - placingZone->gapBottom, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
		if (abs (placingZone->postGapWidth - 0.900) < EPS) {
			horizontalPost.radAng += DegreeToRad (270.0);
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "90 cm", "lenFrame", APIParT_Length, "0.800"));
			horizontalPost.radAng -= DegreeToRad (270.0);
		} else if (abs (placingZone->postGapWidth - 1.200) < EPS) {
			horizontalPost.radAng += DegreeToRad (270.0);
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "120 cm", "lenFrame", APIParT_Length, "1.100"));
			horizontalPost.radAng -= DegreeToRad (270.0);
		} else if (abs (placingZone->postGapWidth - 1.500) < EPS) {
			horizontalPost.radAng += DegreeToRad (270.0);
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "150 cm", "lenFrame", APIParT_Length, "1.400"));
			horizontalPost.radAng -= DegreeToRad (270.0);
		}
		moveIn3D ('x', horizontalPost.radAng, placingZone->postGapLength, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
		moveIn3D ('y', horizontalPost.radAng, -placingZone->postGapWidth + 0.100, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
		if (abs (placingZone->postGapWidth - 0.900) < EPS) {
			horizontalPost.radAng += DegreeToRad (90.0);
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "90 cm", "lenFrame", APIParT_Length, "0.800"));
			horizontalPost.radAng -= DegreeToRad (90.0);
		} else if (abs (placingZone->postGapWidth - 1.200) < EPS) {
			horizontalPost.radAng += DegreeToRad (90.0);
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "120 cm", "lenFrame", APIParT_Length, "1.100"));
			horizontalPost.radAng -= DegreeToRad (90.0);
		} else if (abs (placingZone->postGapWidth - 1.500) < EPS) {
			horizontalPost.radAng += DegreeToRad (90.0);
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "150 cm", "lenFrame", APIParT_Length, "1.400"));
			horizontalPost.radAng -= DegreeToRad (90.0);
		}

		// 배치: PERI동바리 수평재 (길이 방향)
		horizontalPost.init (L("PERI동바리 수평재 v0.2.gsm"), layerInd_HorizontalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
		moveIn3D ('x', horizontalPost.radAng, placingZone->beamLength - placingZone->postGapLength - placingZone->postStartOffset + 0.050, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
		moveIn3D ('y', horizontalPost.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - placingZone->gapSideLeft, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
		moveIn3D ('z', horizontalPost.radAng, distance - placingZone->gapBottom, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
		if (abs (placingZone->postGapLength - 1.200) < EPS) {
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "120 cm", "lenFrame", APIParT_Length, "1.100"));
		} else if (abs (placingZone->postGapLength - 1.500) < EPS) {
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "150 cm", "lenFrame", APIParT_Length, "1.400"));
		} else if (abs (placingZone->postGapLength - 2.015) < EPS) {
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "201.5 cm", "lenFrame", APIParT_Length, "1.915"));
		} else if (abs (placingZone->postGapLength - 2.300) < EPS) {
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "230 cm", "lenFrame", APIParT_Length, "2.200"));
		}
		moveIn3D ('x', horizontalPost.radAng, placingZone->postGapLength - 0.100, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
		moveIn3D ('y', horizontalPost.radAng, placingZone->postGapWidth, &horizontalPost.posX, &horizontalPost.posY, &horizontalPost.posZ);
		if (abs (placingZone->postGapLength - 1.200) < EPS) {
			horizontalPost.radAng += DegreeToRad (180.0);
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "120 cm", "lenFrame", APIParT_Length, "1.100"));
			horizontalPost.radAng -= DegreeToRad (180.0);
		} else if (abs (placingZone->postGapLength - 1.500) < EPS) {
			horizontalPost.radAng += DegreeToRad (180.0);
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "150 cm", "lenFrame", APIParT_Length, "1.400"));
			horizontalPost.radAng -= DegreeToRad (180.0);
		} else if (abs (placingZone->postGapLength - 2.015) < EPS) {
			horizontalPost.radAng += DegreeToRad (180.0);
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "201.5 cm", "lenFrame", APIParT_Length, "1.915"));
			horizontalPost.radAng -= DegreeToRad (180.0);
		} else if (abs (placingZone->postGapLength - 2.300) < EPS) {
			horizontalPost.radAng += DegreeToRad (180.0);
			elemList_SupportingPost.Push (horizontalPost.placeObject (2, "stType", APIParT_CString, "230 cm", "lenFrame", APIParT_Length, "2.200"));
			horizontalPost.radAng -= DegreeToRad (180.0);
		}
	}

	// 잭서포트와 각재 설치
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if (placingZone->cellsAtBottom [0][xx].objType == PLYWOOD) {
			// 배치: 각재
			timber.init (L("목재v1.0.gsm"), layerInd_Timber, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);

			moveIn3D ('x', timber.radAng, placingZone->cellsAtBottom [0][xx].dirLen / 2 - 0.075, &timber.posX, &timber.posY, &timber.posZ);
			moveIn3D ('y', timber.radAng, -0.0215, &timber.posX, &timber.posY, &timber.posZ);		
			moveIn3D ('z', timber.radAng, -0.0115, &timber.posX, &timber.posY, &timber.posZ);

			timber.radAng += DegreeToRad (90.0);
			elemList_Plywood [getAreaSeqNumOfCell (placingZone, true, false, xx)].Push (timber.placeObject (6, "w_ins", APIParT_CString, "바닥눕히기", "w_w", APIParT_Length, format_string ("%f", 0.080).c_str(), "w_h", APIParT_Length, format_string ("%f", 0.050).c_str(), "w_leng", APIParT_Length, format_string ("%f", placingZone->cellsAtBottom [0][xx].perLen + (0.0615 - 0.040)*2).c_str(), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0).c_str()));
			timber.radAng -= DegreeToRad (90.0);

			moveIn3D ('x', timber.radAng, 0.100, &timber.posX, &timber.posY, &timber.posZ);

			timber.radAng += DegreeToRad (90.0);
			elemList_Plywood [getAreaSeqNumOfCell (placingZone, true, false, xx)].Push (timber.placeObject (6, "w_ins", APIParT_CString, "바닥눕히기", "w_w", APIParT_Length, format_string ("%f", 0.080).c_str(), "w_h", APIParT_Length, format_string ("%f", 0.050).c_str(), "w_leng", APIParT_Length, format_string ("%f", placingZone->cellsAtBottom [0][xx].perLen + (0.0615 - 0.040)*2).c_str(), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0).c_str()));
			timber.radAng -= DegreeToRad (90.0);

			// 배치: 잭 서포트
			jackSupport.init (L("잭서포트v1.0.gsm"), layerInd_JackSupport, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);
			
			moveIn3D ('x', jackSupport.radAng, placingZone->cellsAtBottom [0][xx].dirLen / 2, &jackSupport.posX, &jackSupport.posY, &jackSupport.posZ);
			moveIn3D ('y', jackSupport.radAng, placingZone->cellsAtBottom [0][xx].perLen / 2, &jackSupport.posX, &jackSupport.posY, &jackSupport.posZ);
			moveIn3D ('z', jackSupport.radAng, -0.0115 - 0.080 - (2.000 - placingZone->gapBottom - 0.0115 - 0.080), &jackSupport.posX, &jackSupport.posY, &jackSupport.posZ);
		

			elemList_Plywood [getAreaSeqNumOfCell (placingZone, true, false, xx)].Push (jackSupport.placeObject (3, "j_comp", APIParT_CString, "잭서포트", "j_stan", APIParT_CString, "Free", "j_leng", APIParT_Length, format_string ("%f", 2.000 - placingZone->gapBottom - 0.0115 - 0.080).c_str()));
		}
	}

	return	err;
}

// 왼쪽 혹은 오른쪽 면의 idx 번째 셀에 배치되는 객체의 타입을 조사함
short BeamTableformPlacingZone::getObjectType (BeamTableformPlacingZone* placingZone, bool bLeft, short idx)
{
	short objType = NONE;

	for (short xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if (bLeft == true) {
			if (placingZone->cellsAtLSide [idx][xx].objType != NONE)
				objType = placingZone->cellsAtLSide [idx][xx].objType;
		} else {
			if (placingZone->cellsAtRSide [idx][xx].objType != NONE)
				objType = placingZone->cellsAtRSide [idx][xx].objType;
		}
	}

	return objType;
}

// idx 번째 셀은 몇 번째 연속적인 테이블폼 혹은 합판 영역인가?
short BeamTableformPlacingZone::getAreaSeqNumOfCell (BeamTableformPlacingZone* placingZone, bool bLeft, bool bTableform, short idx)
{
	short areaSeqNum = 0;
	short findObjType = (short)((bTableform == true) ? EUROFORM : PLYWOOD);
	short prevCellObjType;
	short curCellObjType;

	for (short xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if (bLeft == true)
			curCellObjType = placingZone->cellsAtLSide [0][xx].objType;
		else
			curCellObjType = placingZone->cellsAtRSide [0][xx].objType;

		if (xx == 0) {
			if (curCellObjType == findObjType)
				++ areaSeqNum;
		} else {
			if ((curCellObjType != prevCellObjType) && (curCellObjType == findObjType))
				++ areaSeqNum;
		}

		prevCellObjType = curCellObjType;

		if (xx == idx)	break;
	}

	return areaSeqNum;
}

// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK beamTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	short		xx;
	double		h1, h2, h3, h4, hRest_Left = 0.0, hRest_Right = 0.0, hRest_Bottom;		// 나머지 높이 계산을 위한 변수
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, L"보에 배치 - 보 단면");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 확인 버튼
			DGSetItemText(dialogID, DG_OK, L"확인");

			// 취소 버튼
			DGSetItemText(dialogID, DG_CANCEL, L"취소");

			// 단면 이미지 표현
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_NO_SLAB_NO_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_NO_SLAB_WITH_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_SAME_SLAB_NO_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_SAME_SLAB_WITH_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THK_RIGHT_THIN_SLAB_NO_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THK_RIGHT_THIN_SLAB_WITH_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THIN_RIGHT_THK_SLAB_NO_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THIN_RIGHT_THK_SLAB_WITH_INSUL);

			if (abs(infoBeam.height - placingZone.areaHeight_Left) < EPS && abs(infoBeam.height - placingZone.areaHeight_Right) < EPS && abs(placingZone.areaHeight_Left - placingZone.areaHeight_Right) < EPS) {
				DGShowItem(dialogID, IMAGE_BEAM_SECTION_NO_SLAB_NO_INSUL);
			}
			else if ((infoBeam.height - placingZone.areaHeight_Left > EPS) && (infoBeam.height - placingZone.areaHeight_Right > EPS) && abs(placingZone.areaHeight_Left - placingZone.areaHeight_Right) < EPS) {
				DGShowItem(dialogID, IMAGE_BEAM_SECTION_SAME_SLAB_NO_INSUL);
			}
			else if ((infoBeam.height - placingZone.areaHeight_Right > EPS) && (placingZone.areaHeight_Right - placingZone.areaHeight_Left > EPS)) {
				DGShowItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THK_RIGHT_THIN_SLAB_NO_INSUL);
			}
			else if ((infoBeam.height - placingZone.areaHeight_Left > EPS) && (placingZone.areaHeight_Left - placingZone.areaHeight_Right > EPS)) {
				DGShowItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THIN_RIGHT_THK_SLAB_NO_INSUL);
			}

			//////////////////////////////////////////////////////////// 아이템 배치 (유로폼)
			// 라벨 및 체크박스
			DGSetItemText(dialogID, LABEL_BEAM_HEIGHT, L"보 높이 [B.H]");
			DGSetItemText(dialogID, LABEL_BEAM_WIDTH, L"보 너비 [B.W]");
			DGSetItemText(dialogID, LABEL_MORPH_HEIGHT_LEFT, L"모프 높이 [M.L]");
			DGSetItemText(dialogID, LABEL_MORPH_HEIGHT_RIGHT, L"모프 높이 [M.R]");
			DGSetItemText(dialogID, LABEL_SLAB_THK_LEFT, L"슬래브 두께 [S.L]");
			DGSetItemText(dialogID, LABEL_SLAB_THK_RIGHT, L"슬래브 두께 [S.R]");
			DGSetItemText(dialogID, LABEL_INSUL_THK_LEFT, L"단열재 두께 [G.L]");
			DGSetItemText(dialogID, LABEL_INSUL_THK_BOTTOM, L"단열재 두께 [G.B]");
			DGSetItemText(dialogID, LABEL_INSUL_THK_RIGHT, L"단열재 두께 [G.R]");

			DGSetItemText(dialogID, LABEL_REST_LEFT, L"나머지");
			DGSetItemText(dialogID, CHECKBOX_TIMBER_LEFT, L"합판/각재");
			DGSetItemText(dialogID, CHECKBOX_T_FORM_LEFT, L"유로폼");
			DGSetItemText(dialogID, CHECKBOX_FILLER_LEFT, L"휠러");
			DGSetItemText(dialogID, CHECKBOX_B_FORM_LEFT, L"유로폼");

			DGSetItemText(dialogID, LABEL_REST_RIGHT, L"나머지");
			DGSetItemText(dialogID, CHECKBOX_TIMBER_RIGHT, L"합판/각재");
			DGSetItemText(dialogID, CHECKBOX_T_FORM_RIGHT, L"유로폼");
			DGSetItemText(dialogID, CHECKBOX_FILLER_RIGHT, L"휠러");
			DGSetItemText(dialogID, CHECKBOX_B_FORM_RIGHT, L"유로폼");

			DGSetItemText(dialogID, CHECKBOX_L_FORM_BOTTOM, L"유로폼");
			DGSetItemText(dialogID, CHECKBOX_FILLER_BOTTOM, L"휠러");
			DGSetItemText(dialogID, CHECKBOX_R_FORM_BOTTOM, L"유로폼");

			DGSetItemText(dialogID, BUTTON_COPY_TO_RIGHT, L"복사\n→");

			// 라벨: 레이어 설정
			DGSetItemText(dialogID, LABEL_LAYER_SETTINGS, L"부재별 레이어 설정");
			DGSetItemText(dialogID, LABEL_LAYER_EUROFORM, L"유로폼");
			DGSetItemText(dialogID, LABEL_LAYER_PLYWOOD, L"합판");
			DGSetItemText(dialogID, LABEL_LAYER_TIMBER, L"각재");
			DGSetItemText(dialogID, LABEL_LAYER_OUTCORNER_ANGLE, L"아웃코너앵글");
			DGSetItemText(dialogID, LABEL_LAYER_FILLERSPACER, L"휠러스페이서");
			DGSetItemText(dialogID, LABEL_LAYER_RECTPIPE, L"비계파이프");
			DGSetItemText(dialogID, LABEL_LAYER_RECTPIPE_HANGER, L"각파이프행거");
			DGSetItemText(dialogID, LABEL_LAYER_PINBOLT, L"핀볼트세트");
			DGSetItemText(dialogID, LABEL_LAYER_EUROFORM_HOOK, L"유로폼 후크");
			DGSetItemText(dialogID, LABEL_LAYER_BLUE_CLAMP, L"블루클램프");
			DGSetItemText(dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL, L"블루목심");

			// 체크박스: 레이어 묶음
			DGSetItemText(dialogID, CHECKBOX_LAYER_COUPLING, L"레이어 묶음");
			DGSetItemValLong(dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			DGSetItemText(dialogID, BUTTON_AUTOSET, L"레이어 자동 설정");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_TIMBER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_FILLERSPACER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE_HANGER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM_HOOK;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, 1);

			ucb.itemID	 = USERCONTROL_LAYER_BLUE_CLAMP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, 1);

			ucb.itemID	 = USERCONTROL_LAYER_BLUE_TIMBER_RAIL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, 1);

			// 기본 유로폼은 초기값 On
			DGSetItemValLong (dialogID, CHECKBOX_B_FORM_LEFT, TRUE);
			DGSetItemValLong (dialogID, CHECKBOX_B_FORM_RIGHT, TRUE);
			DGSetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM, TRUE);

			// 치수 정보는 기본적으로 비활성화
			DGDisableItem(dialogID, EDITCONTROL_BEAM_HEIGHT);
			DGDisableItem(dialogID, EDITCONTROL_BEAM_WIDTH);
			DGDisableItem(dialogID, EDITCONTROL_SLAB_THK_LEFT);
			DGDisableItem(dialogID, EDITCONTROL_SLAB_THK_RIGHT);
			DGDisableItem(dialogID, EDITCONTROL_MORPH_HEIGHT_LEFT);
			DGDisableItem(dialogID, EDITCONTROL_MORPH_HEIGHT_RIGHT);
			DGDisableItem(dialogID, EDITCONTROL_INSUL_THK_LEFT);
			DGDisableItem(dialogID, EDITCONTROL_INSUL_THK_BOTTOM);
			DGDisableItem(dialogID, EDITCONTROL_INSUL_THK_RIGHT);

			// 보 높이/너비
			DGSetItemValDouble(dialogID, EDITCONTROL_BEAM_HEIGHT, infoBeam.height);
			DGSetItemValDouble(dialogID, EDITCONTROL_BEAM_WIDTH, infoBeam.width);

			// 모프 높이
			DGSetItemValDouble(dialogID, EDITCONTROL_MORPH_HEIGHT_LEFT, placingZone.areaHeight_Left);
			DGSetItemValDouble(dialogID, EDITCONTROL_MORPH_HEIGHT_RIGHT, placingZone.areaHeight_Right);

			// 슬래브 두께
			DGSetItemValDouble(dialogID, EDITCONTROL_SLAB_THK_LEFT, infoBeam.height - placingZone.areaHeight_Left);
			DGSetItemValDouble(dialogID, EDITCONTROL_SLAB_THK_RIGHT, infoBeam.height - placingZone.areaHeight_Right);

			// 부재별 체크박스-규격 설정
			if (DGGetItemValLong(dialogID, CHECKBOX_TIMBER_LEFT) == TRUE) {
				DGEnableItem(dialogID, EDITCONTROL_TIMBER_LEFT);
				DGShowItem(dialogID, EDITCONTROL_TIMBER_LEFT);
			}
			else {
				DGDisableItem(dialogID, EDITCONTROL_TIMBER_LEFT);
				DGHideItem(dialogID, EDITCONTROL_TIMBER_LEFT);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_T_FORM_LEFT) == TRUE) {
				DGEnableItem(dialogID, POPUP_T_FORM_LEFT);
				DGShowItem(dialogID, POPUP_T_FORM_LEFT);
			}
			else {
				DGDisableItem(dialogID, POPUP_T_FORM_LEFT);
				DGHideItem(dialogID, POPUP_T_FORM_LEFT);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_LEFT) == TRUE) {
				DGEnableItem(dialogID, EDITCONTROL_FILLER_LEFT);
				DGShowItem(dialogID, EDITCONTROL_FILLER_LEFT);
			}
			else {
				DGDisableItem(dialogID, EDITCONTROL_FILLER_LEFT);
				DGHideItem(dialogID, EDITCONTROL_FILLER_LEFT);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) {
				DGEnableItem(dialogID, EDITCONTROL_FILLER_BOTTOM);
				DGShowItem(dialogID, EDITCONTROL_FILLER_BOTTOM);
			}
			else {
				DGDisableItem(dialogID, EDITCONTROL_FILLER_BOTTOM);
				DGHideItem(dialogID, EDITCONTROL_FILLER_BOTTOM);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) {
				DGEnableItem(dialogID, POPUP_R_FORM_BOTTOM);
				DGShowItem(dialogID, POPUP_R_FORM_BOTTOM);
			}
			else {
				DGDisableItem(dialogID, POPUP_R_FORM_BOTTOM);
				DGHideItem(dialogID, POPUP_R_FORM_BOTTOM);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_TIMBER_RIGHT) == TRUE) {
				DGEnableItem(dialogID, EDITCONTROL_TIMBER_RIGHT);
				DGShowItem(dialogID, EDITCONTROL_TIMBER_RIGHT);
			}
			else {
				DGDisableItem(dialogID, EDITCONTROL_TIMBER_RIGHT);
				DGHideItem(dialogID, EDITCONTROL_TIMBER_RIGHT);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_T_FORM_RIGHT) == TRUE) {
				DGEnableItem(dialogID, POPUP_T_FORM_RIGHT);
				DGShowItem(dialogID, POPUP_T_FORM_RIGHT);
			}
			else {
				DGDisableItem(dialogID, POPUP_T_FORM_RIGHT);
				DGHideItem(dialogID, POPUP_T_FORM_RIGHT);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_RIGHT) == TRUE) {
				DGEnableItem(dialogID, EDITCONTROL_FILLER_RIGHT);
				DGShowItem(dialogID, EDITCONTROL_FILLER_RIGHT);
			}
			else {
				DGDisableItem(dialogID, EDITCONTROL_FILLER_RIGHT);
				DGHideItem(dialogID, EDITCONTROL_FILLER_RIGHT);
			}

			// 나머지 값 계산
			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LEFT) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LEFT);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LEFT) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LEFT, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LEFT)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LEFT) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LEFT);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LEFT) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LEFT, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LEFT)).ToCStr ()) / 1000.0;
			hRest_Left = placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_INSUL_THK_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_LEFT, hRest_Left);

			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RIGHT) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RIGHT);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RIGHT) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RIGHT, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RIGHT)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RIGHT) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RIGHT);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RIGHT) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RIGHT, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RIGHT)).ToCStr ()) / 1000.0;
			hRest_Right = placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_INSUL_THK_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_RIGHT, hRest_Right);

			h1 = 0;
			h2 = 0;
			h3 = 0;
			if (DGGetItemValLong(dialogID, CHECKBOX_L_FORM_BOTTOM) == TRUE)		h1 = atof(DGPopUpGetItemText(dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected(dialogID, POPUP_L_FORM_BOTTOM)).ToCStr()) / 1000.0;
			if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE)		h2 = DGGetItemValDouble(dialogID, EDITCONTROL_FILLER_BOTTOM);
			if (DGGetItemValLong(dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE)		h3 = atof(DGPopUpGetItemText(dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected(dialogID, POPUP_R_FORM_BOTTOM)).ToCStr()) / 1000.0;
			hRest_Bottom = infoBeam.width + DGGetItemValDouble(dialogID, EDITCONTROL_INSUL_THK_LEFT) + DGGetItemValDouble(dialogID, EDITCONTROL_INSUL_THK_RIGHT) - (h1 + h2 + h3);

			// 직접 변경해서는 안 되는 항목 잠그기
			DGDisableItem (dialogID, EDITCONTROL_REST_LEFT);
			DGDisableItem (dialogID, EDITCONTROL_REST_RIGHT);

			// 레이어 옵션 활성화/비활성화
			if ((DGGetItemValLong (dialogID, CHECKBOX_FILLER_LEFT) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RIGHT) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE)) {
				DGEnableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			}

			// 유로폼 후크는 사용하지 않음
			DGDisableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
			DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);

			break;
		
		case DG_MSG_CHANGE:
			// 단면 이미지 표현
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_NO_SLAB_NO_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_NO_SLAB_WITH_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_SAME_SLAB_NO_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_SAME_SLAB_WITH_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THK_RIGHT_THIN_SLAB_NO_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THK_RIGHT_THIN_SLAB_WITH_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THIN_RIGHT_THK_SLAB_NO_INSUL);
			DGHideItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THIN_RIGHT_THK_SLAB_WITH_INSUL);

			if (abs(infoBeam.height - placingZone.areaHeight_Left) < EPS && abs(infoBeam.height - placingZone.areaHeight_Right) < EPS && abs(placingZone.areaHeight_Left - placingZone.areaHeight_Right) < EPS) {
				if (DGGetItemValLong(dialogID, CHECKBOX_INSULATION) == TRUE)
					DGShowItem(dialogID, IMAGE_BEAM_SECTION_NO_SLAB_WITH_INSUL);
				else
					DGShowItem(dialogID, IMAGE_BEAM_SECTION_NO_SLAB_NO_INSUL);
			}
			else if ((infoBeam.height - placingZone.areaHeight_Left > EPS) && (infoBeam.height - placingZone.areaHeight_Right > EPS) && abs(placingZone.areaHeight_Left - placingZone.areaHeight_Right) < EPS) {
				if (DGGetItemValLong(dialogID, CHECKBOX_INSULATION) == TRUE)
					DGShowItem(dialogID, IMAGE_BEAM_SECTION_SAME_SLAB_WITH_INSUL);
				else
					DGShowItem(dialogID, IMAGE_BEAM_SECTION_SAME_SLAB_NO_INSUL);
			}
			else if ((infoBeam.height - placingZone.areaHeight_Right > EPS) && (placingZone.areaHeight_Right - placingZone.areaHeight_Left > EPS)) {
				if (DGGetItemValLong(dialogID, CHECKBOX_INSULATION) == TRUE)
					DGShowItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THK_RIGHT_THIN_SLAB_WITH_INSUL);
				else
					DGShowItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THK_RIGHT_THIN_SLAB_NO_INSUL);
			}
			else if ((infoBeam.height - placingZone.areaHeight_Left > EPS) && (placingZone.areaHeight_Left - placingZone.areaHeight_Right > EPS)) {
				if (DGGetItemValLong(dialogID, CHECKBOX_INSULATION) == TRUE)
					DGShowItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THIN_RIGHT_THK_SLAB_WITH_INSUL);
				else
					DGShowItem(dialogID, IMAGE_BEAM_SECTION_LEFT_THIN_RIGHT_THK_SLAB_NO_INSUL);
			}

			// 단열재 체크박스
			if (DGGetItemValLong(dialogID, CHECKBOX_INSULATION) == TRUE) {
				DGEnableItem(dialogID, EDITCONTROL_INSUL_THK_LEFT);
				DGEnableItem(dialogID, EDITCONTROL_INSUL_THK_BOTTOM);
				DGEnableItem(dialogID, EDITCONTROL_INSUL_THK_RIGHT);
			}
			else {
				DGDisableItem(dialogID, EDITCONTROL_INSUL_THK_LEFT);
				DGDisableItem(dialogID, EDITCONTROL_INSUL_THK_BOTTOM);
				DGDisableItem(dialogID, EDITCONTROL_INSUL_THK_RIGHT);
			}

			// 기본 유로폼까지 꺼버리면 나머지 요소들도 꺼짐
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LEFT) == FALSE) {
				DGSetItemValLong (dialogID, CHECKBOX_TIMBER_LEFT, FALSE);
				DGSetItemValLong (dialogID, CHECKBOX_T_FORM_LEFT, FALSE);
				DGSetItemValLong (dialogID, CHECKBOX_FILLER_LEFT, FALSE);
			}
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RIGHT) == FALSE) {
				DGSetItemValLong (dialogID, CHECKBOX_TIMBER_RIGHT, FALSE);
				DGSetItemValLong (dialogID, CHECKBOX_T_FORM_RIGHT, FALSE);
				DGSetItemValLong (dialogID, CHECKBOX_FILLER_RIGHT, FALSE);
			}
			if (DGGetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM) == FALSE) {
				DGSetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM, FALSE);
				DGSetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM, FALSE);
			}

			// 부재별 체크박스-규격 설정
			if (DGGetItemValLong(dialogID, CHECKBOX_TIMBER_LEFT) == TRUE) {
				DGEnableItem(dialogID, EDITCONTROL_TIMBER_LEFT);
				DGShowItem(dialogID, EDITCONTROL_TIMBER_LEFT);
			}
			else {
				DGDisableItem(dialogID, EDITCONTROL_TIMBER_LEFT);
				DGHideItem(dialogID, EDITCONTROL_TIMBER_LEFT);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_T_FORM_LEFT) == TRUE) {
				DGEnableItem(dialogID, POPUP_T_FORM_LEFT);
				DGShowItem(dialogID, POPUP_T_FORM_LEFT);
			}
			else {
				DGDisableItem(dialogID, POPUP_T_FORM_LEFT);
				DGHideItem(dialogID, POPUP_T_FORM_LEFT);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_LEFT) == TRUE) {
				DGEnableItem(dialogID, EDITCONTROL_FILLER_LEFT);
				DGShowItem(dialogID, EDITCONTROL_FILLER_LEFT);
			}
			else {
				DGDisableItem(dialogID, EDITCONTROL_FILLER_LEFT);
				DGHideItem(dialogID, EDITCONTROL_FILLER_LEFT);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) {
				DGEnableItem(dialogID, EDITCONTROL_FILLER_BOTTOM);
				DGShowItem(dialogID, EDITCONTROL_FILLER_BOTTOM);
			}
			else {
				DGDisableItem(dialogID, EDITCONTROL_FILLER_BOTTOM);
				DGHideItem(dialogID, EDITCONTROL_FILLER_BOTTOM);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) {
				DGEnableItem(dialogID, POPUP_R_FORM_BOTTOM);
				DGShowItem(dialogID, POPUP_R_FORM_BOTTOM);
			}
			else {
				DGDisableItem(dialogID, POPUP_R_FORM_BOTTOM);
				DGHideItem(dialogID, POPUP_R_FORM_BOTTOM);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_TIMBER_RIGHT) == TRUE) {
				DGEnableItem(dialogID, EDITCONTROL_TIMBER_RIGHT);
				DGShowItem(dialogID, EDITCONTROL_TIMBER_RIGHT);
			}
			else {
				DGDisableItem(dialogID, EDITCONTROL_TIMBER_RIGHT);
				DGHideItem(dialogID, EDITCONTROL_TIMBER_RIGHT);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_T_FORM_RIGHT) == TRUE) {
				DGEnableItem(dialogID, POPUP_T_FORM_RIGHT);
				DGShowItem(dialogID, POPUP_T_FORM_RIGHT);
			}
			else {
				DGDisableItem(dialogID, POPUP_T_FORM_RIGHT);
				DGHideItem(dialogID, POPUP_T_FORM_RIGHT);
			}

			if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_RIGHT) == TRUE) {
				DGEnableItem(dialogID, EDITCONTROL_FILLER_RIGHT);
				DGShowItem(dialogID, EDITCONTROL_FILLER_RIGHT);
			}
			else {
				DGDisableItem(dialogID, EDITCONTROL_FILLER_RIGHT);
				DGHideItem(dialogID, EDITCONTROL_FILLER_RIGHT);
			}

			// 나머지 값 계산
			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong(dialogID, CHECKBOX_TIMBER_LEFT) == TRUE)		h1 = DGGetItemValDouble(dialogID, EDITCONTROL_TIMBER_LEFT);
			if (DGGetItemValLong(dialogID, CHECKBOX_T_FORM_LEFT) == TRUE)		h2 = atof(DGPopUpGetItemText(dialogID, POPUP_T_FORM_LEFT, DGPopUpGetSelected(dialogID, POPUP_T_FORM_LEFT)).ToCStr()) / 1000.0;
			if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_LEFT) == TRUE)		h3 = DGGetItemValDouble(dialogID, EDITCONTROL_FILLER_LEFT);
			if (DGGetItemValLong(dialogID, CHECKBOX_B_FORM_LEFT) == TRUE)		h4 = atof(DGPopUpGetItemText(dialogID, POPUP_B_FORM_LEFT, DGPopUpGetSelected(dialogID, POPUP_B_FORM_LEFT)).ToCStr()) / 1000.0;
			hRest_Left = placingZone.areaHeight_Left + DGGetItemValDouble(dialogID, EDITCONTROL_INSUL_THK_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble(dialogID, EDITCONTROL_REST_LEFT, hRest_Left);

			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong(dialogID, CHECKBOX_TIMBER_RIGHT) == TRUE)		h1 = DGGetItemValDouble(dialogID, EDITCONTROL_TIMBER_RIGHT);
			if (DGGetItemValLong(dialogID, CHECKBOX_T_FORM_RIGHT) == TRUE)		h2 = atof(DGPopUpGetItemText(dialogID, POPUP_T_FORM_RIGHT, DGPopUpGetSelected(dialogID, POPUP_T_FORM_RIGHT)).ToCStr()) / 1000.0;
			if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_RIGHT) == TRUE)		h3 = DGGetItemValDouble(dialogID, EDITCONTROL_FILLER_RIGHT);
			if (DGGetItemValLong(dialogID, CHECKBOX_B_FORM_RIGHT) == TRUE)		h4 = atof(DGPopUpGetItemText(dialogID, POPUP_B_FORM_RIGHT, DGPopUpGetSelected(dialogID, POPUP_B_FORM_RIGHT)).ToCStr()) / 1000.0;
			hRest_Right = placingZone.areaHeight_Right + DGGetItemValDouble(dialogID, EDITCONTROL_INSUL_THK_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble(dialogID, EDITCONTROL_REST_RIGHT, hRest_Right);

			h1 = 0;
			h2 = 0;
			h3 = 0;
			if (DGGetItemValLong(dialogID, CHECKBOX_L_FORM_BOTTOM) == TRUE)		h1 = atof(DGPopUpGetItemText(dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected(dialogID, POPUP_L_FORM_BOTTOM)).ToCStr()) / 1000.0;
			if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE)		h2 = DGGetItemValDouble(dialogID, EDITCONTROL_FILLER_BOTTOM);
			if (DGGetItemValLong(dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE)		h3 = atof(DGPopUpGetItemText(dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected(dialogID, POPUP_R_FORM_BOTTOM)).ToCStr()) / 1000.0;
			hRest_Bottom = infoBeam.width + DGGetItemValDouble(dialogID, EDITCONTROL_INSUL_THK_LEFT) + DGGetItemValDouble(dialogID, EDITCONTROL_INSUL_THK_RIGHT) - (h1 + h2 + h3);

			// 레이어 같이 바뀜
			if ((item >= USERCONTROL_LAYER_EUROFORM) && (item <= USERCONTROL_LAYER_BLUE_TIMBER_RAIL)) {
				if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
					long selectedLayer;

					selectedLayer = DGGetItemValLong (dialogID, item);

					for (xx = USERCONTROL_LAYER_EUROFORM ; xx <= USERCONTROL_LAYER_BLUE_TIMBER_RAIL ; ++xx)
						DGSetItemValLong (dialogID, xx, selectedLayer);
				}
			}

			// 레이어 옵션 활성화/비활성화
			if ((DGGetItemValLong(dialogID, CHECKBOX_FILLER_LEFT) == TRUE) || (DGGetItemValLong(dialogID, CHECKBOX_FILLER_RIGHT) == TRUE) || (DGGetItemValLong(dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE)) {
				DGEnableItem(dialogID, LABEL_LAYER_FILLERSPACER);
				DGEnableItem(dialogID, USERCONTROL_LAYER_FILLERSPACER);
			}
			else {
				DGDisableItem(dialogID, LABEL_LAYER_FILLERSPACER);
				DGDisableItem(dialogID, USERCONTROL_LAYER_FILLERSPACER);
			}

			// 나머지 값이 음수가 되면 경고함
			if ((hRest_Left < -EPS) || (hRest_Right < -EPS))
				DGAlert(DG_WARNING, L"경고", L"좌측면 또는 우측면 부재 총 너비가 영역 너비를 초과했습니다.", "", L"확인", "", "");
			if (hRest_Bottom < 0)
				DGAlert(DG_WARNING, L"경고", L"하부면 부재 총 너비가 영역 너비를 초과했습니다.", "", L"확인", "", "");

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					/////////////////////////////////////////////////////////////////// 왼쪽 측면 (높은쪽)
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LEFT) == TRUE) {
						placingZone.bFillBottomFormAtLSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtLSide [0][xx].objType = EUROFORM;
							placingZone.cellsAtLSide [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LEFT, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LEFT)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LEFT) == TRUE) {
						placingZone.bFillFillerAtLSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtLSide [1][xx].objType = FILLERSP;
							placingZone.cellsAtLSide [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LEFT);
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LEFT) == TRUE) {
						placingZone.bFillTopFormAtLSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtLSide [2][xx].objType = EUROFORM;
							placingZone.cellsAtLSide [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LEFT, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LEFT)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LEFT) == TRUE) {
						placingZone.bFillWoodAtLSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							if (DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LEFT) >= 0.100 + EPS)
								placingZone.cellsAtLSide [3][xx].objType = PLYWOOD;
							else
								placingZone.cellsAtLSide [3][xx].objType = TIMBER;
							placingZone.cellsAtLSide [3][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LEFT);
						}
					}

					/////////////////////////////////////////////////////////////////// 오른쪽 측면 (낮은쪽)
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RIGHT) == TRUE) {
						placingZone.bFillBottomFormAtRSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtRSide [0][xx].objType = EUROFORM;
							placingZone.cellsAtRSide [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RIGHT, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RIGHT)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RIGHT) == TRUE) {
						placingZone.bFillFillerAtRSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtRSide [1][xx].objType = FILLERSP;
							placingZone.cellsAtRSide [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RIGHT);
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RIGHT) == TRUE) {
						placingZone.bFillTopFormAtRSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtRSide [2][xx].objType = EUROFORM;
							placingZone.cellsAtRSide [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RIGHT, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RIGHT)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RIGHT) == TRUE) {
						placingZone.bFillWoodAtRSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							if (DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RIGHT) >= 0.100 + EPS)
								placingZone.cellsAtRSide [3][xx].objType = PLYWOOD;
							else
								placingZone.cellsAtRSide [3][xx].objType = TIMBER;
							placingZone.cellsAtRSide [3][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RIGHT);
						}
					}

					/////////////////////////////////////////////////////////////////// 하부
					if (DGGetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM) == TRUE) {
						placingZone.bFillLeftFormAtBottom = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtBottom [0][xx].objType = EUROFORM;
							placingZone.cellsAtBottom [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_L_FORM_BOTTOM)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) {
						placingZone.bFillFillerAtBottom = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtBottom [1][xx].objType = FILLERSP;
							placingZone.cellsAtBottom [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_BOTTOM);
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) {
						placingZone.bFillRightFormAtBottom = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtBottom [2][xx].objType = EUROFORM;
							placingZone.cellsAtBottom [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_R_FORM_BOTTOM)).ToCStr ()) / 1000.0;
						}
					}

					// 나머지 값 계산
					h1 = 0;
					h2 = 0;
					h3 = 0;
					h4 = 0;
					if (DGGetItemValLong(dialogID, CHECKBOX_TIMBER_LEFT) == TRUE)		h1 = DGGetItemValDouble(dialogID, EDITCONTROL_TIMBER_LEFT);
					if (DGGetItemValLong(dialogID, CHECKBOX_T_FORM_LEFT) == TRUE)		h2 = atof(DGPopUpGetItemText(dialogID, POPUP_T_FORM_LEFT, DGPopUpGetSelected(dialogID, POPUP_T_FORM_LEFT)).ToCStr()) / 1000.0;
					if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_LEFT) == TRUE)		h3 = DGGetItemValDouble(dialogID, EDITCONTROL_FILLER_LEFT);
					if (DGGetItemValLong(dialogID, CHECKBOX_B_FORM_LEFT) == TRUE)		h4 = atof(DGPopUpGetItemText(dialogID, POPUP_B_FORM_LEFT, DGPopUpGetSelected(dialogID, POPUP_B_FORM_LEFT)).ToCStr()) / 1000.0;
					hRest_Left = placingZone.areaHeight_Left + DGGetItemValDouble(dialogID, EDITCONTROL_INSUL_THK_BOTTOM) - (h1 + h2 + h3 + h4);
					DGSetItemValDouble(dialogID, EDITCONTROL_REST_LEFT, hRest_Left);

					h1 = 0;
					h2 = 0;
					h3 = 0;
					h4 = 0;
					if (DGGetItemValLong(dialogID, CHECKBOX_TIMBER_RIGHT) == TRUE)		h1 = DGGetItemValDouble(dialogID, EDITCONTROL_TIMBER_RIGHT);
					if (DGGetItemValLong(dialogID, CHECKBOX_T_FORM_RIGHT) == TRUE)		h2 = atof(DGPopUpGetItemText(dialogID, POPUP_T_FORM_RIGHT, DGPopUpGetSelected(dialogID, POPUP_T_FORM_RIGHT)).ToCStr()) / 1000.0;
					if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_RIGHT) == TRUE)		h3 = DGGetItemValDouble(dialogID, EDITCONTROL_FILLER_RIGHT);
					if (DGGetItemValLong(dialogID, CHECKBOX_B_FORM_RIGHT) == TRUE)		h4 = atof(DGPopUpGetItemText(dialogID, POPUP_B_FORM_RIGHT, DGPopUpGetSelected(dialogID, POPUP_B_FORM_RIGHT)).ToCStr()) / 1000.0;
					hRest_Right = placingZone.areaHeight_Right + DGGetItemValDouble(dialogID, EDITCONTROL_INSUL_THK_BOTTOM) - (h1 + h2 + h3 + h4);
					DGSetItemValDouble(dialogID, EDITCONTROL_REST_RIGHT, hRest_Right);

					// 나머지 길이
					placingZone.hRest_Left = hRest_Left;
					placingZone.hRest_Right = hRest_Right;

					// 보와의 간격
					placingZone.gapSideLeft = DGGetItemValDouble (dialogID, EDITCONTROL_INSUL_THK_LEFT);
					placingZone.gapSideRight = DGGetItemValDouble (dialogID, EDITCONTROL_INSUL_THK_RIGHT);
					placingZone.gapBottom = DGGetItemValDouble (dialogID, EDITCONTROL_INSUL_THK_BOTTOM);

					/////////////////////////////////////////////////////////////////// 보 양끝
					placingZone.beginCellAtLSide.perLen = placingZone.areaHeight_Left + placingZone.gapBottom;
					placingZone.beginCellAtRSide.perLen = placingZone.areaHeight_Right + placingZone.gapBottom;
					placingZone.beginCellAtBottom.perLen = placingZone.areaWidth_Bottom + placingZone.gapSideLeft + placingZone.gapSideRight;

					placingZone.endCellAtLSide.perLen = placingZone.areaHeight_Left + placingZone.gapBottom;
					placingZone.endCellAtRSide.perLen = placingZone.areaHeight_Right + placingZone.gapBottom;
					placingZone.endCellAtBottom.perLen = placingZone.areaWidth_Bottom + placingZone.gapSideLeft + placingZone.gapSideRight;

					// 레이어 번호 저장
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_Timber			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER);
					layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
					layerInd_Fillerspacer	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER);
					layerInd_Rectpipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					layerInd_RectpipeHanger	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
					layerInd_Pinbolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					layerInd_EuroformHook	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
					layerInd_BlueClamp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
					layerInd_BlueTimberRail	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);

					break;

				case BUTTON_AUTOSET:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

					layerInd_Euroform			= makeTemporaryLayer (structuralObject_forTableformBeam, "UFOM", NULL);
					layerInd_Plywood			= makeTemporaryLayer (structuralObject_forTableformBeam, "PLYW", NULL);
					layerInd_Timber				= makeTemporaryLayer (structuralObject_forTableformBeam, "TIMB", NULL);
					layerInd_OutcornerAngle		= makeTemporaryLayer (structuralObject_forTableformBeam, "OUTA", NULL);
					layerInd_Fillerspacer		= makeTemporaryLayer (structuralObject_forTableformBeam, "FISP", NULL);
					layerInd_Rectpipe			= makeTemporaryLayer (structuralObject_forTableformBeam, "SPIP", NULL);
					layerInd_RectpipeHanger		= makeTemporaryLayer (structuralObject_forTableformBeam, "JOIB", NULL);
					layerInd_Pinbolt			= makeTemporaryLayer (structuralObject_forTableformBeam, "PINB", NULL);
					//layerInd_EuroformHook		= makeTemporaryLayer (structuralObject_forTableformBeam, "HOOK", NULL);
					layerInd_BlueClamp			= makeTemporaryLayer (structuralObject_forTableformBeam, "UFCL", NULL);
					layerInd_BlueTimberRail		= makeTemporaryLayer (structuralObject_forTableformBeam, "RAIL", NULL);

					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, layerInd_OutcornerAngle);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER, layerInd_Fillerspacer);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_Rectpipe);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, layerInd_RectpipeHanger);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_Pinbolt);
					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, layerInd_EuroformHook);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);

					break;

				case DG_CANCEL:
					break;

				case BUTTON_COPY_TO_RIGHT:
					item = 0;

					// 체크박스 상태 복사
					DGSetItemValLong (dialogID, CHECKBOX_B_FORM_RIGHT, DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LEFT));
					DGSetItemValLong (dialogID, CHECKBOX_FILLER_RIGHT, DGGetItemValLong (dialogID, CHECKBOX_FILLER_LEFT));
					DGSetItemValLong (dialogID, CHECKBOX_T_FORM_RIGHT, DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LEFT));
					DGSetItemValLong (dialogID, CHECKBOX_TIMBER_RIGHT, DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LEFT));

					// 팝업 컨트롤 및 Edit컨트롤 값 복사
					DGPopUpSelectItem (dialogID, POPUP_B_FORM_RIGHT, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LEFT));
					DGSetItemValDouble (dialogID, EDITCONTROL_FILLER_RIGHT, DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LEFT));
					DGPopUpSelectItem (dialogID, POPUP_T_FORM_RIGHT, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LEFT));
					DGSetItemValDouble (dialogID, EDITCONTROL_TIMBER_RIGHT, DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LEFT));

					// 부재별 체크박스-규격 설정
					if (DGGetItemValLong(dialogID, CHECKBOX_TIMBER_LEFT) == TRUE) {
						DGEnableItem(dialogID, EDITCONTROL_TIMBER_LEFT);
						DGShowItem(dialogID, EDITCONTROL_TIMBER_LEFT);
					}
					else {
						DGDisableItem(dialogID, EDITCONTROL_TIMBER_LEFT);
						DGHideItem(dialogID, EDITCONTROL_TIMBER_LEFT);
					}

					if (DGGetItemValLong(dialogID, CHECKBOX_T_FORM_LEFT) == TRUE) {
						DGEnableItem(dialogID, POPUP_T_FORM_LEFT);
						DGShowItem(dialogID, POPUP_T_FORM_LEFT);
					}
					else {
						DGDisableItem(dialogID, POPUP_T_FORM_LEFT);
						DGHideItem(dialogID, POPUP_T_FORM_LEFT);
					}

					if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_LEFT) == TRUE) {
						DGEnableItem(dialogID, EDITCONTROL_FILLER_LEFT);
						DGShowItem(dialogID, EDITCONTROL_FILLER_LEFT);
					}
					else {
						DGDisableItem(dialogID, EDITCONTROL_FILLER_LEFT);
						DGHideItem(dialogID, EDITCONTROL_FILLER_LEFT);
					}

					if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) {
						DGEnableItem(dialogID, EDITCONTROL_FILLER_BOTTOM);
						DGShowItem(dialogID, EDITCONTROL_FILLER_BOTTOM);
					}
					else {
						DGDisableItem(dialogID, EDITCONTROL_FILLER_BOTTOM);
						DGHideItem(dialogID, EDITCONTROL_FILLER_BOTTOM);
					}

					if (DGGetItemValLong(dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) {
						DGEnableItem(dialogID, POPUP_R_FORM_BOTTOM);
						DGShowItem(dialogID, POPUP_R_FORM_BOTTOM);
					}
					else {
						DGDisableItem(dialogID, POPUP_R_FORM_BOTTOM);
						DGHideItem(dialogID, POPUP_R_FORM_BOTTOM);
					}

					if (DGGetItemValLong(dialogID, CHECKBOX_TIMBER_RIGHT) == TRUE) {
						DGEnableItem(dialogID, EDITCONTROL_TIMBER_RIGHT);
						DGShowItem(dialogID, EDITCONTROL_TIMBER_RIGHT);
					}
					else {
						DGDisableItem(dialogID, EDITCONTROL_TIMBER_RIGHT);
						DGHideItem(dialogID, EDITCONTROL_TIMBER_RIGHT);
					}

					if (DGGetItemValLong(dialogID, CHECKBOX_T_FORM_RIGHT) == TRUE) {
						DGEnableItem(dialogID, POPUP_T_FORM_RIGHT);
						DGShowItem(dialogID, POPUP_T_FORM_RIGHT);
					}
					else {
						DGDisableItem(dialogID, POPUP_T_FORM_RIGHT);
						DGHideItem(dialogID, POPUP_T_FORM_RIGHT);
					}

					if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_RIGHT) == TRUE) {
						DGEnableItem(dialogID, EDITCONTROL_FILLER_RIGHT);
						DGShowItem(dialogID, EDITCONTROL_FILLER_RIGHT);
					}
					else {
						DGDisableItem(dialogID, EDITCONTROL_FILLER_RIGHT);
						DGHideItem(dialogID, EDITCONTROL_FILLER_RIGHT);
					}

					// 나머지 값 계산
					h1 = 0;
					h2 = 0;
					h3 = 0;
					h4 = 0;
					if (DGGetItemValLong(dialogID, CHECKBOX_TIMBER_LEFT) == TRUE)		h1 = DGGetItemValDouble(dialogID, EDITCONTROL_TIMBER_LEFT);
					if (DGGetItemValLong(dialogID, CHECKBOX_T_FORM_LEFT) == TRUE)		h2 = atof(DGPopUpGetItemText(dialogID, POPUP_T_FORM_LEFT, DGPopUpGetSelected(dialogID, POPUP_T_FORM_LEFT)).ToCStr()) / 1000.0;
					if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_LEFT) == TRUE)		h3 = DGGetItemValDouble(dialogID, EDITCONTROL_FILLER_LEFT);
					if (DGGetItemValLong(dialogID, CHECKBOX_B_FORM_LEFT) == TRUE)		h4 = atof(DGPopUpGetItemText(dialogID, POPUP_B_FORM_LEFT, DGPopUpGetSelected(dialogID, POPUP_B_FORM_LEFT)).ToCStr()) / 1000.0;
					hRest_Left = placingZone.areaHeight_Left + DGGetItemValDouble(dialogID, EDITCONTROL_INSUL_THK_BOTTOM) - (h1 + h2 + h3 + h4);
					DGSetItemValDouble(dialogID, EDITCONTROL_REST_LEFT, hRest_Left);

					h1 = 0;
					h2 = 0;
					h3 = 0;
					h4 = 0;
					if (DGGetItemValLong(dialogID, CHECKBOX_TIMBER_RIGHT) == TRUE)		h1 = DGGetItemValDouble(dialogID, EDITCONTROL_TIMBER_RIGHT);
					if (DGGetItemValLong(dialogID, CHECKBOX_T_FORM_RIGHT) == TRUE)		h2 = atof(DGPopUpGetItemText(dialogID, POPUP_T_FORM_RIGHT, DGPopUpGetSelected(dialogID, POPUP_T_FORM_RIGHT)).ToCStr()) / 1000.0;
					if (DGGetItemValLong(dialogID, CHECKBOX_FILLER_RIGHT) == TRUE)		h3 = DGGetItemValDouble(dialogID, EDITCONTROL_FILLER_RIGHT);
					if (DGGetItemValLong(dialogID, CHECKBOX_B_FORM_RIGHT) == TRUE)		h4 = atof(DGPopUpGetItemText(dialogID, POPUP_B_FORM_RIGHT, DGPopUpGetSelected(dialogID, POPUP_B_FORM_RIGHT)).ToCStr()) / 1000.0;
					hRest_Right = placingZone.areaHeight_Right + DGGetItemValDouble(dialogID, EDITCONTROL_INSUL_THK_BOTTOM) - (h1 + h2 + h3 + h4);
					DGSetItemValDouble(dialogID, EDITCONTROL_REST_RIGHT, hRest_Right);

					break;
			}
		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK beamTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	xx;
	short	itmPosX, itmPosY;
	double	lengthDouble;
	const short		maxCol = 50;		// 열 최대 개수
	static short	dialogSizeX = 630;	// 현재 다이얼로그 크기 X
	static short	dialogSizeY = 360;	// 현재 다이얼로그 크기 Y

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, L"보에 배치 - 보 측면");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 230, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"확인");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 270, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"취소");
			DGShowItem (dialogID, DG_CANCEL);

			// 이전 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 310, 70, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, L"이전");
			DGShowItem (dialogID, DG_PREV);

			// 라벨: 보 측면
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 10, 100, 23);
			DGSetItemFont (dialogID, LABEL_BEAM_SIDE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_BEAM_SIDE, L"보 측면");
			DGShowItem (dialogID, LABEL_BEAM_SIDE);

			// 라벨: 총 길이
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 12, 60, 23);
			DGSetItemFont (dialogID, LABEL_TOTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_TOTAL_LENGTH, L"총 길이");
			DGShowItem (dialogID, LABEL_TOTAL_LENGTH);

			// Edit컨트롤: 총 길이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 165, 5, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_TOTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_TOTAL_LENGTH);

			// 라벨: 남은 길이
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 230, 12, 60, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_REMAIN_LENGTH, L"남은 길이");
			DGShowItem (dialogID, LABEL_REMAIN_LENGTH);

			// Edit컨트롤: 남은 길이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 300, 5, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_LENGTH);

			// 라벨: 테이블폼 타입
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 370, 12, 80, 23);
			DGSetItemFont (dialogID, LABEL_TABLEFORM_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_TABLEFORM_TYPE, L"테이블폼 타입");
			DGShowItem (dialogID, LABEL_TABLEFORM_TYPE);

			// 팝업컨트롤: 테이블폼 타입
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 455, 7, 70, 23);
			DGSetItemFont (dialogID, POPUP_TABLEFORM_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, L"타입A");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, L"타입B");
			DGPopUpSelectItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_TOP);
			DGShowItem (dialogID, POPUP_TABLEFORM_TYPE);

			// 라벨: 보 브라켓 타입
			DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 545, 12, 80, 23);
			DGSetItemFont(dialogID, LABEL_BEAM_BRACKET_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, LABEL_BEAM_BRACKET_TYPE, L"보 브라켓 타입");
			DGShowItem(dialogID, LABEL_BEAM_BRACKET_TYPE);

			// 팝업컨트롤: 보 브라켓 타입
			DGAppendDialogItem(dialogID, DG_ITM_POPUPCONTROL, 50, 1, 630, 7, 130, 23);
			DGSetItemFont(dialogID, POPUP_BEAM_BRACKET_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem(dialogID, POPUP_BEAM_BRACKET_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText(dialogID, POPUP_BEAM_BRACKET_TYPE, DG_POPUP_BOTTOM, L"보 브라켓 (최홍승)");
			DGPopUpInsertItem(dialogID, POPUP_BEAM_BRACKET_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText(dialogID, POPUP_BEAM_BRACKET_TYPE, DG_POPUP_BOTTOM, L"블루 보 브라켓");
			DGPopUpSelectItem(dialogID, POPUP_BEAM_BRACKET_TYPE, DG_POPUP_TOP);
			DGShowItem(dialogID, POPUP_BEAM_BRACKET_TYPE);

			// 버튼: 추가
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 70, 70, 25);
			DGSetItemFont (dialogID, BUTTON_ADD_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ADD_COL, L"추가");
			DGShowItem (dialogID, BUTTON_ADD_COL);

			// 버튼: 삭제
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 105, 70, 25);
			DGSetItemFont (dialogID, BUTTON_DEL_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DEL_COL, L"삭제");
			DGShowItem (dialogID, BUTTON_DEL_COL);

			// 버튼: 자동배치
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 140, 70, 25);
			DGSetItemFont(dialogID, BUTTON_AUTO_ARRAY, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, BUTTON_AUTO_ARRAY, L"자동배치");
			DGShowItem(dialogID, BUTTON_AUTO_ARRAY);

			// 자동배치 변수 초기화
			autoArrayInfo.centerFiller_objType = NONE;
			autoArrayInfo.centerFiller_length = 1.200;
			autoArrayInfo.bUseEuroform_1200 = true;
			autoArrayInfo.bUseEuroform_1050 = false;
			autoArrayInfo.bUseEuroform_0900 = true;
			autoArrayInfo.bUseEuroform_0750 = false;
			autoArrayInfo.bUseEuroform_0600 = true;
			autoArrayInfo.bUseEuroform_etc = false;

			// 왼쪽 끝 여백 채우기 여부 (체크박스)
			placingZone.CHECKBOX_MARGIN_LEFT_END = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 120, 70, 70, 70);
			DGSetItemFont (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, L"합판\n채우기");
			DGShowItem (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END);
			DGSetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, TRUE);
			// 왼쪽 끝 여백 길이 (Edit컨트롤)
			placingZone.EDITCONTROL_MARGIN_LEFT_END = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 120, 140, 70, 25);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
			DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END, 0.090);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END, 2.440);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END, 0.150);

			// 일반 셀: 기본값은 유로폼
			itmPosX = 120+70;
			itmPosY = 72;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				// 버튼
				placingZone.BUTTON_OBJ [xx] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
				DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], L"유로폼");
				DGShowItem (dialogID, placingZone.BUTTON_OBJ [xx]);
				DGDisableItem (dialogID, placingZone.BUTTON_OBJ [xx]);

				// 객체 타입 (팝업컨트롤)
				placingZone.POPUP_OBJ_TYPE [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_IS_EXTRASMALL | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, L"없음");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, L"유로폼");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, L"합판");
				DGPopUpInsertItem(dialogID, placingZone.POPUP_OBJ_TYPE[xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText(dialogID, placingZone.POPUP_OBJ_TYPE[xx], DG_POPUP_BOTTOM, L"휠러");
				DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_TOP+1);
				DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

				// 너비 (팝업컨트롤)
				placingZone.POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1200");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM, "1050");
				DGPopUpInsertItem(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "900");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM, "750");
				DGPopUpInsertItem(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "600");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, L"이형폼");
				DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
				DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);

				placingZone.EDITCONTROL_WIDTH_EUROFORM[xx] = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68 + 25, 70, 23);
				DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
				DGDisableItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
				DGSetItemMinDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx], 0.050);
				DGSetItemMaxDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx], 1.500);
				DGSetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx], 0.150);

				// 너비 (Edit컨트롤컨트롤) - 처음에는 숨김
				placingZone.EDITCONTROL_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
				DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
				DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.010);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 2.440);
				if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == PLYWOOD + 1)
					DGSetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx], 0.150);
				else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == FILLERSP + 1)
					DGSetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx], 0.010);

				itmPosX += 70;
			}

			// 오른쪽 끝 여백 채우기 여부 (체크박스)
			placingZone.CHECKBOX_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 70, 70, 70);
			DGSetItemFont (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, L"합판\n채우기");
			DGShowItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
			DGSetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, TRUE);
			// 오른쪽 끝 여백 길이 (Edit컨트롤)
			placingZone.EDITCONTROL_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 140, 70, 25);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
			DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.090);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 2.440);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.150);

			// 총 길이, 남은 길이 표시
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LENGTH, placingZone.beamLength);
			lengthDouble = placingZone.beamLength;
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1)
					lengthDouble -= atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000.0;
				else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == PLYWOOD + 1)
					lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
				else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == FILLERSP + 1)
					lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, lengthDouble);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_LENGTH);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_LENGTH);

			// 다이얼로그 크기 설정
			dialogSizeX = 630;
			dialogSizeY = 360;
			if (placingZone.nCells >= 4) {
				DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCells - 3), dialogSizeY, DG_TOPLEFT, true);
			}

			break;

		case DG_MSG_CHANGE:
			// 여백 채우기 버튼 체크 유무
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)
				DGEnableItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
			else
				DGDisableItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);

			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)
				DGEnableItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
			else
				DGDisableItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);

			// 객체 타입 변경시
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				// 해당 버튼의 이름 변경
				DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], DGPopUpGetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx])));

				// 없음이면 모두 숨김, 유로폼이면 팝업 표시, 합판이면 Edit컨트롤 표시
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE + 1) {
					DGHideItem(dialogID, placingZone.POPUP_WIDTH [xx]);
					DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
					DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1) {
					DGShowItem(dialogID, placingZone.POPUP_WIDTH [xx]);
					DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
					if (DGPopUpGetSelected(dialogID, placingZone.POPUP_WIDTH[xx]) == 6)
						DGShowItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
					else
						DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD + 1) {
					DGHideItem(dialogID, placingZone.POPUP_WIDTH [xx]);
					DGShowItem(dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
					DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
				}
				else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == FILLERSP + 1) {
					DGHideItem(dialogID, placingZone.POPUP_WIDTH[xx]);
					DGShowItem(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
					DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
				}
			}

			// 유로폼의 너비 항목 변경시
			for (xx = 0; xx < placingZone.nCells; ++xx) {
				if (DGPopUpGetSelected(dialogID, placingZone.POPUP_WIDTH[xx]) == 6)
					DGEnableItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
				else
					DGDisableItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
			}

			// 남은 길이 표시
			lengthDouble = placingZone.beamLength;
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == EUROFORM + 1) {
					if (DGPopUpGetSelected(dialogID, placingZone.POPUP_WIDTH[xx]) == 6)
						lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
					else
						lengthDouble -= atof(DGPopUpGetItemText(dialogID, placingZone.POPUP_WIDTH[xx], DGPopUpGetSelected(dialogID, placingZone.POPUP_WIDTH[xx])).ToCStr().Get()) / 1000.0;
				}
				else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == PLYWOOD + 1)
					lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
				else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == FILLERSP + 1)
					lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, lengthDouble);

			// 나머지 값이 음수가 되면 경고함
			if (lengthDouble < -EPS)
				DGAlert(DG_WARNING, L"경고", L"부재 총 길이가 영역 길이를 초과했습니다.", "", L"확인", "", "");

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_PREV:
					clickedPrevButton = true;
					break;

				case DG_OK:
					clickedOKButton = true;

					// 테이블폼 타입
					placingZone.tableformType = DGPopUpGetSelected(dialogID, POPUP_TABLEFORM_TYPE);

					// 보 브라켓 타입
					placingZone.beamBracketType = DGPopUpGetSelected(dialogID, POPUP_BEAM_BRACKET_TYPE);

					// 합판 채우기 정보 저장
					if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE) {
						placingZone.bFillMarginBegin = true;
						placingZone.marginBegin = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
					} else {
						placingZone.bFillMarginBegin = false;
						placingZone.marginBegin = 0.0;
					}

					if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE) {
						placingZone.bFillMarginEnd = true;
						placingZone.marginEnd = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
					} else {
						placingZone.bFillMarginEnd = false;
						placingZone.marginEnd = 0.0;
					}

					placingZone.beginCellAtLSide.dirLen = placingZone.marginBegin;
					placingZone.beginCellAtRSide.dirLen = placingZone.marginBegin;
					placingZone.beginCellAtBottom.dirLen = placingZone.marginBegin;

					placingZone.endCellAtLSide.dirLen = placingZone.marginEnd;
					placingZone.endCellAtRSide.dirLen = placingZone.marginEnd;
					placingZone.endCellAtBottom.dirLen = placingZone.marginEnd;

					// 셀 정보 저장
					for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE + 1) {
							placingZone.cellsAtLSide [0][xx].objType = NONE;
							placingZone.cellsAtLSide [0][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [0][xx].perLen = 0.0;
							placingZone.cellsAtLSide [1][xx].objType = NONE;
							placingZone.cellsAtLSide [1][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [1][xx].perLen = 0.0;
							placingZone.cellsAtLSide [2][xx].objType = NONE;
							placingZone.cellsAtLSide [2][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [2][xx].perLen = 0.0;
							placingZone.cellsAtLSide [3][xx].objType = NONE;
							placingZone.cellsAtLSide [3][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [3][xx].perLen = 0.0;

							placingZone.cellsAtRSide [0][xx].objType = NONE;
							placingZone.cellsAtRSide [0][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [0][xx].perLen = 0.0;
							placingZone.cellsAtRSide [1][xx].objType = NONE;
							placingZone.cellsAtRSide [1][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [1][xx].perLen = 0.0;
							placingZone.cellsAtRSide [2][xx].objType = NONE;
							placingZone.cellsAtRSide [2][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [2][xx].perLen = 0.0;
							placingZone.cellsAtRSide [3][xx].objType = NONE;
							placingZone.cellsAtRSide [3][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [3][xx].perLen = 0.0;

							placingZone.cellsAtBottom [0][xx].objType = NONE;
							placingZone.cellsAtBottom [0][xx].dirLen = 0.0;
							placingZone.cellsAtBottom [0][xx].perLen = 0.0;
							placingZone.cellsAtBottom [1][xx].objType = NONE;
							placingZone.cellsAtBottom [1][xx].dirLen = 0.0;
							placingZone.cellsAtBottom [1][xx].perLen = 0.0;
							placingZone.cellsAtBottom [2][xx].objType = NONE;
							placingZone.cellsAtBottom [2][xx].dirLen = 0.0;
							placingZone.cellsAtBottom [2][xx].perLen = 0.0;

						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1) {
							if (DGPopUpGetSelected(dialogID, placingZone.POPUP_WIDTH[xx]) == 6)
								lengthDouble = DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
							else
								lengthDouble = atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000.0;

							placingZone.cellsAtLSide [0][xx].dirLen = lengthDouble;
							placingZone.cellsAtLSide [1][xx].dirLen = lengthDouble;
							placingZone.cellsAtLSide [2][xx].dirLen = lengthDouble;
							placingZone.cellsAtLSide [3][xx].dirLen = lengthDouble;

							placingZone.cellsAtRSide [0][xx].dirLen = lengthDouble;
							placingZone.cellsAtRSide [1][xx].dirLen = lengthDouble;
							placingZone.cellsAtRSide [2][xx].dirLen = lengthDouble;
							placingZone.cellsAtRSide [3][xx].dirLen = lengthDouble;

							placingZone.cellsAtBottom [0][xx].dirLen = lengthDouble;
							placingZone.cellsAtBottom [1][xx].dirLen = lengthDouble;
							placingZone.cellsAtBottom [2][xx].dirLen = lengthDouble;

						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD + 1) {
							lengthDouble = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);

							placingZone.cellsAtLSide [0][xx].objType = PLYWOOD;
							placingZone.cellsAtLSide [0][xx].dirLen = lengthDouble;
							placingZone.cellsAtLSide [0][xx].perLen = placingZone.areaHeight_Left + placingZone.gapBottom;
							placingZone.cellsAtLSide [1][xx].objType = NONE;
							placingZone.cellsAtLSide [1][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [1][xx].perLen = 0.0;
							placingZone.cellsAtLSide [2][xx].objType = NONE;
							placingZone.cellsAtLSide [2][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [2][xx].perLen = 0.0;
							placingZone.cellsAtLSide [3][xx].objType = NONE;
							placingZone.cellsAtLSide [3][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [3][xx].perLen = 0.0;

							placingZone.cellsAtRSide [0][xx].objType = PLYWOOD;
							placingZone.cellsAtRSide [0][xx].dirLen = lengthDouble;
							placingZone.cellsAtRSide [0][xx].perLen = placingZone.areaHeight_Right + placingZone.gapBottom;
							placingZone.cellsAtRSide [1][xx].objType = NONE;
							placingZone.cellsAtRSide [1][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [1][xx].perLen = 0.0;
							placingZone.cellsAtRSide [2][xx].objType = NONE;
							placingZone.cellsAtRSide [2][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [2][xx].perLen = 0.0;
							placingZone.cellsAtRSide [3][xx].objType = NONE;
							placingZone.cellsAtRSide [3][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [3][xx].perLen = 0.0;

							placingZone.cellsAtBottom [0][xx].objType = PLYWOOD;
							placingZone.cellsAtBottom [0][xx].dirLen = lengthDouble;
							placingZone.cellsAtBottom [0][xx].perLen = placingZone.areaWidth_Bottom + placingZone.gapSideLeft + placingZone.gapSideRight;
							placingZone.cellsAtBottom [1][xx].objType = NONE;
							placingZone.cellsAtBottom [1][xx].dirLen = 0.0;
							placingZone.cellsAtBottom [1][xx].perLen = 0.0;
							placingZone.cellsAtBottom [2][xx].objType = NONE;
							placingZone.cellsAtBottom [2][xx].dirLen = 0.0;
							placingZone.cellsAtBottom [2][xx].perLen = 0.0;
						}
						else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == FILLERSP + 1) {
							lengthDouble = DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);

							placingZone.cellsAtLSide[0][xx].objType = FILLERSP;
							placingZone.cellsAtLSide[0][xx].dirLen = lengthDouble;
							placingZone.cellsAtLSide[0][xx].perLen = placingZone.areaHeight_Left + placingZone.gapBottom;
							placingZone.cellsAtLSide[1][xx].objType = NONE;
							placingZone.cellsAtLSide[1][xx].dirLen = 0.0;
							placingZone.cellsAtLSide[1][xx].perLen = 0.0;
							placingZone.cellsAtLSide[2][xx].objType = NONE;
							placingZone.cellsAtLSide[2][xx].dirLen = 0.0;
							placingZone.cellsAtLSide[2][xx].perLen = 0.0;
							placingZone.cellsAtLSide[3][xx].objType = NONE;
							placingZone.cellsAtLSide[3][xx].dirLen = 0.0;
							placingZone.cellsAtLSide[3][xx].perLen = 0.0;

							placingZone.cellsAtRSide[0][xx].objType = FILLERSP;
							placingZone.cellsAtRSide[0][xx].dirLen = lengthDouble;
							placingZone.cellsAtRSide[0][xx].perLen = placingZone.areaHeight_Right + placingZone.gapBottom;
							placingZone.cellsAtRSide[1][xx].objType = NONE;
							placingZone.cellsAtRSide[1][xx].dirLen = 0.0;
							placingZone.cellsAtRSide[1][xx].perLen = 0.0;
							placingZone.cellsAtRSide[2][xx].objType = NONE;
							placingZone.cellsAtRSide[2][xx].dirLen = 0.0;
							placingZone.cellsAtRSide[2][xx].perLen = 0.0;
							placingZone.cellsAtRSide[3][xx].objType = NONE;
							placingZone.cellsAtRSide[3][xx].dirLen = 0.0;
							placingZone.cellsAtRSide[3][xx].perLen = 0.0;

							placingZone.cellsAtBottom[0][xx].objType = FILLERSP;
							placingZone.cellsAtBottom[0][xx].dirLen = lengthDouble;
							placingZone.cellsAtBottom[0][xx].perLen = placingZone.areaWidth_Bottom + placingZone.gapSideLeft + placingZone.gapSideRight;
							placingZone.cellsAtBottom[1][xx].objType = NONE;
							placingZone.cellsAtBottom[1][xx].dirLen = 0.0;
							placingZone.cellsAtBottom[1][xx].perLen = 0.0;
							placingZone.cellsAtBottom[2][xx].objType = NONE;
							placingZone.cellsAtBottom[2][xx].dirLen = 0.0;
							placingZone.cellsAtBottom[2][xx].perLen = 0.0;
						}
					}

					break;

				case DG_CANCEL:
					break;

				case BUTTON_ADD_COL:
					item = 0;

					if (placingZone.nCells < maxCol) {
						// 오른쪽 끝 여백 채우기 버튼을 지우고
						DGRemoveDialogItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
						DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);

						// 마지막 셀 버튼 오른쪽에 새로운 셀 버튼을 추가하고
						itmPosX = 120+70 + (70 * placingZone.nCells);
						itmPosY = 72;
						// 버튼
						placingZone.BUTTON_OBJ [placingZone.nCells] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
						DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells], DG_IS_LARGE | DG_IS_PLAIN);
						DGSetItemText (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells], L"유로폼");
						DGShowItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells]);
						DGDisableItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells]);

						// 객체 타입 (팝업컨트롤)
						placingZone.POPUP_OBJ_TYPE [placingZone.nCells] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
						DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_IS_EXTRASMALL | DG_IS_PLAIN);
						DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM, L"없음");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM, L"유로폼");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM, L"합판");
						DGPopUpInsertItem(dialogID, placingZone.POPUP_OBJ_TYPE[placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText(dialogID, placingZone.POPUP_OBJ_TYPE[placingZone.nCells], DG_POPUP_BOTTOM, L"휠러");
						DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_TOP+1);
						DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells]);

						// 너비 (팝업컨트롤)
						placingZone.POPUP_WIDTH [placingZone.nCells] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
						DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_IS_LARGE | DG_IS_PLAIN);
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM, "1200");
						DGPopUpInsertItem(dialogID, placingZone.POPUP_WIDTH[placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText(dialogID, placingZone.POPUP_WIDTH[placingZone.nCells], DG_POPUP_BOTTOM, "1050");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM, "900");
						DGPopUpInsertItem(dialogID, placingZone.POPUP_WIDTH[placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText(dialogID, placingZone.POPUP_WIDTH[placingZone.nCells], DG_POPUP_BOTTOM, "750");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM, "600");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM, L"이형폼");
						DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_TOP);
						DGSetItemMinDouble (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], 0.090);
						DGSetItemMaxDouble (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], 2.440);

						placingZone.EDITCONTROL_WIDTH_EUROFORM[placingZone.nCells] = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68 + 25, 70, 23);
						DGDisableItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[placingZone.nCells]);
						DGSetItemMinDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[placingZone.nCells], 0.050);
						DGSetItemMaxDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[placingZone.nCells], 1.500);
						DGSetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[placingZone.nCells], 0.150);

						// 너비 (Edit컨트롤) - 처음에는 숨김
						placingZone.EDITCONTROL_WIDTH [placingZone.nCells] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
						if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[placingZone.nCells]) == PLYWOOD + 1)
							DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH[placingZone.nCells], 0.150);
						else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[placingZone.nCells]) == FILLERSP + 1)
							DGSetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[placingZone.nCells], 0.010);
						DGSetItemMinDouble(dialogID, placingZone.EDITCONTROL_WIDTH[placingZone.nCells], 0.010);
						DGSetItemMaxDouble(dialogID, placingZone.EDITCONTROL_WIDTH[placingZone.nCells], 2.440);

						itmPosX += 70;

						// 오른쪽 끝 여백 채우기 버튼을 오른쪽 끝에 붙임
						// 오른쪽 끝 여백 채우기 여부 (체크버튼)
						placingZone.CHECKBOX_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 70, 70, 70);
						DGSetItemFont (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, DG_IS_LARGE | DG_IS_PLAIN);
						DGSetItemText (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, L"합판\n채우기");
						DGShowItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
						DGSetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, TRUE);
						// 오른쪽 끝 여백 길이 (Edit컨트롤)
						placingZone.EDITCONTROL_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 140, 70, 25);
						DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.090);
						DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 2.440);
						DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.150);

						// 없음이면 모두 숨김, 유로폼이면 팝업 표시, 합판이면 Edit컨트롤 표시
						if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[placingZone.nCells]) == NONE + 1) {
							DGHideItem(dialogID, placingZone.POPUP_WIDTH[placingZone.nCells]);
							DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH[placingZone.nCells]);
							DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[placingZone.nCells]);
						}
						else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[placingZone.nCells]) == EUROFORM + 1) {
							DGShowItem(dialogID, placingZone.POPUP_WIDTH[placingZone.nCells]);
							DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH[placingZone.nCells]);
							if (DGPopUpGetSelected(dialogID, placingZone.POPUP_WIDTH[placingZone.nCells]) == 6)
								DGShowItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[placingZone.nCells]);
							else
								DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[placingZone.nCells]);
						}
						else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[placingZone.nCells]) == PLYWOOD + 1) {
							DGHideItem(dialogID, placingZone.POPUP_WIDTH[placingZone.nCells]);
							DGShowItem(dialogID, placingZone.EDITCONTROL_WIDTH[placingZone.nCells]);
							DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[placingZone.nCells]);
						}
						else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[placingZone.nCells]) == FILLERSP + 1) {
							DGHideItem(dialogID, placingZone.POPUP_WIDTH[placingZone.nCells]);
							DGShowItem(dialogID, placingZone.EDITCONTROL_WIDTH[placingZone.nCells]);
							DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[placingZone.nCells]);
						}

						// 남은 길이 표시
						lengthDouble = placingZone.beamLength;
						if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
						if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
							if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1)
								if (DGPopUpGetSelected(dialogID, placingZone.POPUP_WIDTH[xx]) == 6)
									lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
								else
									lengthDouble -= atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000.0;
							else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == PLYWOOD + 1)
								lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
							else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == FILLERSP + 1)
								lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
						}
						DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, lengthDouble);

						++placingZone.nCells;

						// 다이얼로그 크기 설정
						dialogSizeX = 630;
						dialogSizeY = 360;
						if (placingZone.nCells >= 4) {
							DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCells - 4), dialogSizeY, DG_TOPLEFT, true);
						}
					}

					break;

				case BUTTON_DEL_COL:
					item = 0;

					if (placingZone.nCells > 1) {
						// 오른쪽 끝 여백 채우기 버튼을 지우고
						DGRemoveDialogItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
						DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);

						// 마지막 셀 버튼을 지우고
						DGRemoveDialogItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells - 1]);
						DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells - 1]);
						DGRemoveDialogItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells - 1]);
						DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH [placingZone.nCells - 1]);
						DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM [placingZone.nCells - 1]);

						// 오른쪽 끝 여백 채우기 버튼을 오른쪽 끝에 붙임
						itmPosX = 120+70 + (70 * (placingZone.nCells - 1));
						itmPosY = 72;
						// 오른쪽 끝 여백 채우기 여부 (체크버튼)
						placingZone.CHECKBOX_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 70, 70, 70);
						DGSetItemFont (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, DG_IS_LARGE | DG_IS_PLAIN);
						DGSetItemText (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, L"합판\n채우기");
						DGShowItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
						DGSetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, TRUE);
						// 오른쪽 끝 여백 길이 (Edit컨트롤)
						placingZone.EDITCONTROL_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 140, 70, 25);
						DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.150);

						// 남은 길이 표시
						lengthDouble = placingZone.beamLength;
						if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
						if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
							if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == EUROFORM + 1)
								if (DGPopUpGetSelected(dialogID, placingZone.POPUP_WIDTH[xx]) == 6)
									lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
								else
									lengthDouble -= atof(DGPopUpGetItemText(dialogID, placingZone.POPUP_WIDTH[xx], DGPopUpGetSelected(dialogID, placingZone.POPUP_WIDTH[xx])).ToCStr().Get()) / 1000.0;
							else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == PLYWOOD + 1)
								lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
							else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == FILLERSP + 1)
								lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
						}
						DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, lengthDouble);

						--placingZone.nCells;

						// 다이얼로그 크기 설정
						dialogSizeX = 630;
						dialogSizeY = 360;
						if (placingZone.nCells >= 4) {
							DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCells - 4), dialogSizeY, DG_TOPLEFT, true);
						}
					}

					break;

				case BUTTON_AUTO_ARRAY:
					item = 0;
					result = DGBlankModalDialog(380, 250, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, beamTableformPlacerHandler2_AutoArray, (DGUserData)&autoArrayInfo);

					if (result == DG_OK) {
						double	remainFullLength = 0.0;
						double	remainHalfLength = 0.0;

						int nEuroform_1200 = 0;		int nEuroform_1200_init = 0;
						int nEuroform_1050 = 0;		int nEuroform_1050_init = 0;
						int nEuroform_0900 = 0;		int nEuroform_0900_init = 0;
						int nEuroform_0750 = 0;		int nEuroform_0750_init = 0;
						int nEuroform_0600 = 0;		int nEuroform_0600_init = 0;
						int nEuroform_etc = 0;		int nEuroform_etc_init = 0;
						int nPlywood_etc = 0;		int nPlywood_etc_init = 0;
						double etc_length = 0.0;

						struct TempCell
						{
							short	objType;
							double	dirLen;
						};
						TempCell	cells[50];
						int			nCells;		// 셀 개수 (시작/끝 셀 제외)
						TempCell	beginCell;	// 시작 셀 (합판)
						TempCell	endCell;	// 끝 셀 (합판)

						// 기존 배치도 제거하고
						DGRemoveDialogItems(dialogID, AFTER_ALL);

						// 영역을 유로폼/합판으로 채우기 (양끝 합판의 길이는 기본 150, 100~250까지 가능)
						beginCell.objType = PLYWOOD;
						endCell.objType = PLYWOOD;
						beginCell.dirLen = 0.150;
						endCell.dirLen = 0.150;

						// 센터필러가 없을 경우, 전체에 해당하는 길이를 할당함
						if (autoArrayInfo.centerFiller_objType == NONE) {
							remainFullLength = placingZone.beamLength;
							remainHalfLength = remainFullLength / 2;

							while (remainFullLength > EPS) {
								// 남은 길이가 1000 이상일 경우
								if (remainFullLength > 1.000 - EPS) {
									// 유로폼 1200 계수
									if ((autoArrayInfo.bUseEuroform_1200 == true) && (remainFullLength > 1.200 - EPS)) {
										remainFullLength -= 1.200;
										nEuroform_1200++;
									}
									// 유로폼 1050 계수
									else if ((autoArrayInfo.bUseEuroform_1050 == true) && (remainFullLength > 1.050 - EPS)) {
										remainFullLength -= 1.050;
										nEuroform_1050++;
									}
									// 유로폼 900 계수
									else if ((autoArrayInfo.bUseEuroform_0900 == true) && (remainFullLength > 0.900 - EPS)) {
										remainFullLength -= 0.900;
										nEuroform_0900++;
									}
									// 유로폼 750 계수
									else if ((autoArrayInfo.bUseEuroform_0750 == true) && (remainFullLength > 0.750 - EPS)) {
										remainFullLength -= 0.750;
										nEuroform_0750++;
									}
									// 유로폼 600 계수
									else if ((autoArrayInfo.bUseEuroform_0600 == true) && (remainFullLength > 0.600 - EPS)) {
										remainFullLength -= 0.600;
										nEuroform_0600++;
									}
								}
								// 남은 길이가 1000 미만 700 초과일 경우
								else if ((remainFullLength < 1.000) && (remainFullLength > 0.700)) {
									remainFullLength -= 0.600;
									nEuroform_0600++;

									beginCell.dirLen = remainFullLength / 2;	// 끝의 합판 길이를 새로 지정함
									endCell.dirLen = remainFullLength / 2;		// 끝의 합판 길이를 새로 지정함
									remainFullLength = 0.0;
								}
								// 남은 길이가 700 미만 500 초과일 경우
								else if ((remainFullLength < 0.700) && (remainFullLength > 0.500)) {
									// 이형유로폼 사용할 경우
									if (autoArrayInfo.bUseEuroform_etc == true) {
										nEuroform_etc++;
										etc_length = remainFullLength - 0.300;	// 양끝의 합판 길이를 제외한 나머지 길이
									}
									// 합판 사용할 경우
									else {
										nPlywood_etc++;
										etc_length = remainFullLength - 0.300;	// 양끝의 합판 길이를 제외한 나머지 길이
									}
									remainFullLength = 0.0;
								}
								// 남은 길이가 500 이하일 경우
								else {
									beginCell.dirLen = remainFullLength / 2;	// 끝의 합판 길이를 새로 지정함
									endCell.dirLen = remainFullLength / 2;		// 끝의 합판 길이를 새로 지정함
									remainFullLength = 0.0;
								}
							}
						}
						// 센터필러가 있을 경우, 반쪽에 해당하는 길이를 할당함
						else {
							remainHalfLength = (placingZone.beamLength - autoArrayInfo.centerFiller_length) / 2;

							while (remainHalfLength > EPS) {
								// 남은 길이가 700 이상일 경우
								if (remainHalfLength > 0.700 - EPS) {
									// 유로폼 1200 계수
									if ((autoArrayInfo.bUseEuroform_1200 == true) && (remainHalfLength > 1.200 - EPS)) {
										remainHalfLength -= 1.200;
										nEuroform_1200++;
									}
									// 유로폼 1050 계수
									else if ((autoArrayInfo.bUseEuroform_1050 == true) && (remainHalfLength > 1.050 - EPS)) {
										remainHalfLength -= 1.050;
										nEuroform_1050++;
									}
									// 유로폼 900 계수
									else if ((autoArrayInfo.bUseEuroform_0900 == true) && (remainHalfLength > 0.900 - EPS)) {
										remainHalfLength -= 0.900;
										nEuroform_0900++;
									}
									// 유로폼 750 계수
									else if ((autoArrayInfo.bUseEuroform_0750 == true) && (remainHalfLength > 0.750 - EPS)) {
										remainHalfLength -= 0.750;
										nEuroform_0750++;
									}
									else // 유로폼 600 계수
									if ((autoArrayInfo.bUseEuroform_0600 == true) && (remainHalfLength > 0.600 - EPS)) {
										remainHalfLength -= 0.600;
										nEuroform_0600++;
									}
								}
								// 남은 길이가 700 미만 250 초과일 경우
								else if ((remainHalfLength < 0.700) && (remainHalfLength > 0.250)) {
									// 이형유로폼 사용할 경우
									if (autoArrayInfo.bUseEuroform_etc == true) {
										nEuroform_etc++;
										etc_length = remainHalfLength - 0.150;	// 끝의 합판 길이를 제외한 나머지 길이
									}
									// 합판 사용할 경우
									else {
										nPlywood_etc++;
										etc_length = remainHalfLength - 0.150;	// 끝의 합판 길이를 제외한 나머지 길이
									}
									remainHalfLength = 0.0;
								}
								// 남은 길이가 250 이하일 경우
								else {
									beginCell.dirLen = remainHalfLength;	// 끝의 합판 길이를 새로 지정함
									endCell.dirLen = remainHalfLength;		// 끝의 합판 길이를 새로 지정함
									remainHalfLength = 0.0;
								}
							}

							remainFullLength = remainHalfLength * 2;
						}

						// 셀 정보 업데이트
						nCells = 0;

						// 센터필러가 없을 경우
						if (autoArrayInfo.centerFiller_objType == NONE) {
							// 셀 정보 설정
							while (nEuroform_1200 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 1.200;
								nEuroform_1200--;
								nCells++;
							}
							while (nEuroform_1050 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 1.050;
								nEuroform_1050--;
								nCells++;
							}
							while (nEuroform_0900 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 0.900;
								nEuroform_0900--;
								nCells++;
							}
							while (nEuroform_0750 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 0.750;
								nEuroform_0750--;
								nCells++;
							}
							while (nEuroform_0600 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 0.600;
								nEuroform_0600--;
								nCells++;
							}
							while (nEuroform_etc > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = etc_length;
								nEuroform_etc--;
								nCells++;
							}
							while (nPlywood_etc > 0) {
								cells[nCells].objType = PLYWOOD;
								cells[nCells].dirLen = etc_length;
								nPlywood_etc--;
								nCells++;
							}
						}
						// 센터필러가 있을 경우
						else {
							// 반대쪽에도 배치하기 위해 수량 정보를 미리 저장해 둘 것
							nEuroform_1200_init = nEuroform_1200;
							nEuroform_1050_init = nEuroform_1050;
							nEuroform_0900_init = nEuroform_0900;
							nEuroform_0750_init = nEuroform_0750;
							nEuroform_0600_init = nEuroform_0600;
							nEuroform_etc_init = nEuroform_etc;
							nPlywood_etc_init = nPlywood_etc;

							// 셀 정보 설정
							while (nEuroform_1200 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 1.200;
								nEuroform_1200--;
								nCells++;
							}
							while (nEuroform_1050 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 1.050;
								nEuroform_1050--;
								nCells++;
							}
							while (nEuroform_0900 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 0.900;
								nEuroform_0900--;
								nCells++;
							}
							while (nEuroform_0750 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 0.750;
								nEuroform_0750--;
								nCells++;
							}
							while (nEuroform_0600 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 0.600;
								nEuroform_0600--;
								nCells++;
							}
							while (nEuroform_etc > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = etc_length;
								nEuroform_etc--;
								nCells++;
							}
							while (nPlywood_etc > 0) {
								cells[nCells].objType = PLYWOOD;
								cells[nCells].dirLen = etc_length;
								nPlywood_etc--;
								nCells++;
							}

							// 센터필러 정보 저장함
							cells[nCells].objType = autoArrayInfo.centerFiller_objType;
							cells[nCells].dirLen = autoArrayInfo.centerFiller_length;
							nCells++;
							
							// 반대쪽에도 배치하기 위해 수량 정보 다시 로드함
							nEuroform_1200 = nEuroform_1200_init;
							nEuroform_1050 = nEuroform_1050_init;
							nEuroform_0900 = nEuroform_0900_init;
							nEuroform_0750 = nEuroform_0750_init;
							nEuroform_0600 = nEuroform_0600_init;
							nEuroform_etc = nEuroform_etc_init;
							nPlywood_etc = nPlywood_etc_init;

							// 셀 정보 설정
							while (nPlywood_etc > 0) {
								cells[nCells].objType = PLYWOOD;
								cells[nCells].dirLen = etc_length;
								nPlywood_etc--;
								nCells++;
							}
							while (nEuroform_etc > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = etc_length;
								nEuroform_etc--;
								nCells++;
							}
							while (nEuroform_0600 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 0.600;
								nEuroform_0600--;
								nCells++;
							}
							while (nEuroform_0750 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 0.750;
								nEuroform_0750--;
								nCells++;
							}
							while (nEuroform_0900 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 0.900;
								nEuroform_0900--;
								nCells++;
							}
							while (nEuroform_1050 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 1.050;
								nEuroform_1050--;
								nCells++;
							}
							while (nEuroform_1200 > 0) {
								cells[nCells].objType = EUROFORM;
								cells[nCells].dirLen = 1.200;
								nEuroform_1200--;
								nCells++;
							}
						}

						placingZone.nCells = nCells;
						
						// 왼쪽 끝 여백 채우기 여부 (체크박스)
						placingZone.CHECKBOX_MARGIN_LEFT_END = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 120, 70, 70, 70);
						DGSetItemFont(dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, DG_IS_LARGE | DG_IS_PLAIN);
						DGSetItemText(dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, L"합판\n채우기");
						DGShowItem(dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END);
						DGSetItemValLong(dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, TRUE);
						// 왼쪽 끝 여백 길이 (Edit컨트롤)
						placingZone.EDITCONTROL_MARGIN_LEFT_END = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 120, 140, 70, 25);
						DGShowItem(dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
						DGSetItemMinDouble(dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END, 0.090);
						DGSetItemMaxDouble(dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END, 2.440);
						DGSetItemValDouble(dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END, beginCell.dirLen);

						// 일반 셀: 기본값은 유로폼
						itmPosX = 120 + 70;
						itmPosY = 72;
						for (xx = 0; xx < placingZone.nCells; ++xx) {
							// 버튼
							placingZone.BUTTON_OBJ[xx] = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
							DGSetItemFont(dialogID, placingZone.BUTTON_OBJ[xx], DG_IS_LARGE | DG_IS_PLAIN);
							if (cells[xx].objType == NONE)
								DGSetItemText(dialogID, placingZone.BUTTON_OBJ[xx], L"없음");
							else if (cells[xx].objType == EUROFORM)
								DGSetItemText(dialogID, placingZone.BUTTON_OBJ[xx], L"유로폼");
							else if (cells[xx].objType == PLYWOOD)
								DGSetItemText(dialogID, placingZone.BUTTON_OBJ[xx], L"합판");
							else if (cells[xx].objType == FILLERSP)
								DGSetItemText(dialogID, placingZone.BUTTON_OBJ[xx], L"휠러");
							DGShowItem(dialogID, placingZone.BUTTON_OBJ[xx]);
							DGDisableItem(dialogID, placingZone.BUTTON_OBJ[xx]);

							// 객체 타입 (팝업컨트롤)
							placingZone.POPUP_OBJ_TYPE[xx] = itmIdx = DGAppendDialogItem(dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
							DGSetItemFont(dialogID, placingZone.POPUP_OBJ_TYPE[xx], DG_IS_EXTRASMALL | DG_IS_PLAIN);
							DGPopUpInsertItem(dialogID, placingZone.POPUP_OBJ_TYPE[xx], DG_POPUP_BOTTOM);
							DGPopUpSetItemText(dialogID, placingZone.POPUP_OBJ_TYPE[xx], DG_POPUP_BOTTOM, L"없음");
							DGPopUpInsertItem(dialogID, placingZone.POPUP_OBJ_TYPE[xx], DG_POPUP_BOTTOM);
							DGPopUpSetItemText(dialogID, placingZone.POPUP_OBJ_TYPE[xx], DG_POPUP_BOTTOM, L"유로폼");
							DGPopUpInsertItem(dialogID, placingZone.POPUP_OBJ_TYPE[xx], DG_POPUP_BOTTOM);
							DGPopUpSetItemText(dialogID, placingZone.POPUP_OBJ_TYPE[xx], DG_POPUP_BOTTOM, L"합판");
							DGPopUpInsertItem(dialogID, placingZone.POPUP_OBJ_TYPE[xx], DG_POPUP_BOTTOM);
							DGPopUpSetItemText(dialogID, placingZone.POPUP_OBJ_TYPE[xx], DG_POPUP_BOTTOM, L"휠러");
							if (cells[xx].objType == NONE)
								DGPopUpSelectItem(dialogID, placingZone.POPUP_OBJ_TYPE[xx], 1);
							else if (cells[xx].objType == EUROFORM)
								DGPopUpSelectItem(dialogID, placingZone.POPUP_OBJ_TYPE[xx], 2);
							else if (cells[xx].objType == PLYWOOD)
								DGPopUpSelectItem(dialogID, placingZone.POPUP_OBJ_TYPE[xx], 3);
							else if (cells[xx].objType == FILLERSP)
								DGPopUpSelectItem(dialogID, placingZone.POPUP_OBJ_TYPE[xx], 4);
							DGShowItem(dialogID, placingZone.POPUP_OBJ_TYPE[xx]);

							// 너비 (팝업컨트롤)
							placingZone.POPUP_WIDTH[xx] = DGAppendDialogItem(dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
							DGSetItemFont(dialogID, placingZone.POPUP_WIDTH[xx], DG_IS_LARGE | DG_IS_PLAIN);
							DGPopUpInsertItem(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM);
							DGPopUpSetItemText(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM, "1200");
							DGPopUpInsertItem(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM);
							DGPopUpSetItemText(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM, "1050");
							DGPopUpInsertItem(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM);
							DGPopUpSetItemText(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM, "900");
							DGPopUpInsertItem(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM);
							DGPopUpSetItemText(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM, "750");
							DGPopUpInsertItem(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM);
							DGPopUpSetItemText(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM, "600");
							DGPopUpInsertItem(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM);
							DGPopUpSetItemText(dialogID, placingZone.POPUP_WIDTH[xx], DG_POPUP_BOTTOM, L"이형폼");
							if ((cells[xx].objType == EUROFORM) && (abs(cells[xx].dirLen - 1.200) < EPS))
								DGPopUpSelectItem(dialogID, placingZone.POPUP_WIDTH[xx], 1);
							else if ((cells[xx].objType == EUROFORM) && (abs(cells[xx].dirLen - 1.050) < EPS))
								DGPopUpSelectItem(dialogID, placingZone.POPUP_WIDTH[xx], 2);
							else if ((cells[xx].objType == EUROFORM) && (abs(cells[xx].dirLen - 0.900) < EPS))
								DGPopUpSelectItem(dialogID, placingZone.POPUP_WIDTH[xx], 3);
							else if ((cells[xx].objType == EUROFORM) && (abs(cells[xx].dirLen - 0.750) < EPS))
								DGPopUpSelectItem(dialogID, placingZone.POPUP_WIDTH[xx], 4);
							else if ((cells[xx].objType == EUROFORM) && (abs(cells[xx].dirLen - 0.600) < EPS))
								DGPopUpSelectItem(dialogID, placingZone.POPUP_WIDTH[xx], 5);
							else
								DGPopUpSelectItem(dialogID, placingZone.POPUP_WIDTH[xx], 6);
							if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == EUROFORM + 1)
								DGEnableItem(dialogID, placingZone.POPUP_WIDTH[xx]);
							else
								DGDisableItem(dialogID, placingZone.POPUP_WIDTH[xx]);

							placingZone.EDITCONTROL_WIDTH_EUROFORM[xx] = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68 + 25, 70, 23);
							DGShowItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
							if ((cells[xx].objType == EUROFORM) && (DGPopUpGetSelected(dialogID, placingZone.POPUP_WIDTH[xx]) == 6)) {
								DGEnableItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
							}
							else {
								DGDisableItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
							}
							DGSetItemMinDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx], 0.050);
							DGSetItemMaxDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx], 1.500);
							if (DGPopUpGetSelected(dialogID, placingZone.POPUP_WIDTH[xx]) == 6)
								DGSetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx], cells[xx].dirLen);
							else
								DGSetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx], 0.150);

							// 너비 (Edit컨트롤컨트롤) - 처음에는 숨김
							placingZone.EDITCONTROL_WIDTH[xx] = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
							DGSetItemMinDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx], 0.010);
							DGSetItemMaxDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx], 2.440);
							if ((DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == PLYWOOD + 1) || (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == FILLERSP + 1))
								DGSetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx], cells[xx].dirLen);
							else
								DGSetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx], 0.150);

							// 없음이면 모두 숨김, 유로폼이면 팝업 표시, 합판이면 Edit컨트롤 표시
							if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == NONE + 1) {
								DGHideItem(dialogID, placingZone.POPUP_WIDTH[xx]);
								DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
								DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
							}
							else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == EUROFORM + 1) {
								DGShowItem(dialogID, placingZone.POPUP_WIDTH[xx]);
								DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
								if (DGPopUpGetSelected(dialogID, placingZone.POPUP_WIDTH[xx]) == 6)
									DGShowItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
								else
									DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
							}
							else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == PLYWOOD + 1) {
								DGHideItem(dialogID, placingZone.POPUP_WIDTH[xx]);
								DGShowItem(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
								DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
							}
							else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == FILLERSP + 1) {
								DGHideItem(dialogID, placingZone.POPUP_WIDTH[xx]);
								DGShowItem(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
								DGHideItem(dialogID, placingZone.EDITCONTROL_WIDTH_EUROFORM[xx]);
							}

							itmPosX += 70;
						}

						// 오른쪽 끝 여백 채우기 여부 (체크박스)
						placingZone.CHECKBOX_MARGIN_RIGHT_END = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 70, 70, 70);
						DGSetItemFont(dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, DG_IS_LARGE | DG_IS_PLAIN);
						DGSetItemText(dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, L"합판\n채우기");
						DGShowItem(dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
						DGSetItemValLong(dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, TRUE);
						// 오른쪽 끝 여백 길이 (Edit컨트롤)
						placingZone.EDITCONTROL_MARGIN_RIGHT_END = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 140, 70, 25);
						DGShowItem(dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						DGSetItemMinDouble(dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.090);
						DGSetItemMaxDouble(dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 2.440);
						DGSetItemValDouble(dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, endCell.dirLen);

						// 총 길이, 남은 길이 표시
						DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_LENGTH, placingZone.beamLength);
						lengthDouble = placingZone.beamLength;
						if (DGGetItemValLong(dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)	lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
						if (DGGetItemValLong(dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)	lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						for (xx = 0; xx < placingZone.nCells; ++xx) {
							if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == EUROFORM + 1)
								lengthDouble -= atof(DGPopUpGetItemText(dialogID, placingZone.POPUP_WIDTH[xx], DGPopUpGetSelected(dialogID, placingZone.POPUP_WIDTH[xx])).ToCStr().Get()) / 1000.0;
							else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == PLYWOOD + 1)
								lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
							else if (DGPopUpGetSelected(dialogID, placingZone.POPUP_OBJ_TYPE[xx]) == FILLERSP + 1)
								lengthDouble -= DGGetItemValDouble(dialogID, placingZone.EDITCONTROL_WIDTH[xx]);
						}
						DGSetItemValDouble(dialogID, EDITCONTROL_REMAIN_LENGTH, lengthDouble);
						DGDisableItem(dialogID, EDITCONTROL_TOTAL_LENGTH);
						DGDisableItem(dialogID, EDITCONTROL_REMAIN_LENGTH);

						// 다이얼로그 크기 설정
						dialogSizeX = 630;
						dialogSizeY = 360;
						if (placingZone.nCells >= 4) {
							DGSetDialogSize(dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCells - 4), dialogSizeY, DG_TOPLEFT, true);
						}
					}

					break;
			}

			break;

		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// 2차 다이얼로그에서 자동배치 버튼을 누를 때 배치 조건을 물어보는 다이얼로그
short DGCALLBACK beamTableformPlacerHandler2_AutoArray(short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	AutoArray* autoArrayInfo = (AutoArray*)userData;
	short	result;

	switch (message) {
	case DG_MSG_INIT:
		// 다이얼로그 타이틀
		DGSetDialogTitle(dialogID, L"자동배치 옵션 설정");

		// 확인 버튼
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 130, 210, 70, 25);
		DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_OK, L"확인");
		DGShowItem(dialogID, DG_OK);

		// 취소 버튼
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 210, 210, 70, 25);
		DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_CANCEL, L"취소");
		DGShowItem(dialogID, DG_CANCEL);

		// 라벨: 센터필러 타입
		DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 20, 90, 23);
		DGSetItemFont(dialogID, LABEL_CENTER_FILLER, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, LABEL_CENTER_FILLER, L"센터필러 타입");
		DGShowItem(dialogID, LABEL_CENTER_FILLER);

		// 센터필러 타입 (팝업컨트롤)
		DGAppendDialogItem(dialogID, DG_ITM_POPUPCONTROL, 50, 1, 115, 15, 80, 25);
		DGSetItemFont(dialogID, POPUP_CENTER_FILLER_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
		DGPopUpInsertItem(dialogID, POPUP_CENTER_FILLER_TYPE, DG_POPUP_BOTTOM);
		DGPopUpSetItemText(dialogID, POPUP_CENTER_FILLER_TYPE, DG_POPUP_BOTTOM, L"없음");
		DGPopUpInsertItem(dialogID, POPUP_CENTER_FILLER_TYPE, DG_POPUP_BOTTOM);
		DGPopUpSetItemText(dialogID, POPUP_CENTER_FILLER_TYPE, DG_POPUP_BOTTOM, L"유로폼");
		DGPopUpInsertItem(dialogID, POPUP_CENTER_FILLER_TYPE, DG_POPUP_BOTTOM);
		DGPopUpSetItemText(dialogID, POPUP_CENTER_FILLER_TYPE, DG_POPUP_BOTTOM, L"합판");
		DGPopUpInsertItem(dialogID, POPUP_CENTER_FILLER_TYPE, DG_POPUP_BOTTOM);
		DGPopUpSetItemText(dialogID, POPUP_CENTER_FILLER_TYPE, DG_POPUP_BOTTOM, L"휠러");
		DGShowItem(dialogID, POPUP_CENTER_FILLER_TYPE);
		DGPopUpSelectItem(dialogID, POPUP_CENTER_FILLER_TYPE, autoArrayInfo->centerFiller_objType + 1);

		// 센터필러 길이 (팝업컨트롤)
		DGAppendDialogItem(dialogID, DG_ITM_POPUPCONTROL, 50, 1, 200, 15, 80, 25);
		DGSetItemFont(dialogID, POPUP_CENTER_FILLER_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
		DGPopUpInsertItem(dialogID, POPUP_CENTER_FILLER_LENGTH, DG_POPUP_BOTTOM);
		DGPopUpSetItemText(dialogID, POPUP_CENTER_FILLER_LENGTH, DG_POPUP_BOTTOM, "1200");
		DGPopUpInsertItem(dialogID, POPUP_CENTER_FILLER_LENGTH, DG_POPUP_BOTTOM);
		DGPopUpSetItemText(dialogID, POPUP_CENTER_FILLER_LENGTH, DG_POPUP_BOTTOM, "1050");
		DGPopUpInsertItem(dialogID, POPUP_CENTER_FILLER_LENGTH, DG_POPUP_BOTTOM);
		DGPopUpSetItemText(dialogID, POPUP_CENTER_FILLER_LENGTH, DG_POPUP_BOTTOM, "900");
		DGPopUpInsertItem(dialogID, POPUP_CENTER_FILLER_LENGTH, DG_POPUP_BOTTOM);
		DGPopUpSetItemText(dialogID, POPUP_CENTER_FILLER_LENGTH, DG_POPUP_BOTTOM, "750");
		DGPopUpInsertItem(dialogID, POPUP_CENTER_FILLER_LENGTH, DG_POPUP_BOTTOM);
		DGPopUpSetItemText(dialogID, POPUP_CENTER_FILLER_LENGTH, DG_POPUP_BOTTOM, "600");
		DGPopUpInsertItem(dialogID, POPUP_CENTER_FILLER_LENGTH, DG_POPUP_BOTTOM);
		DGPopUpSetItemText(dialogID, POPUP_CENTER_FILLER_LENGTH, DG_POPUP_BOTTOM, L"이형폼");
		DGShowItem(dialogID, POPUP_CENTER_FILLER_LENGTH);
		DGDisableItem(dialogID, POPUP_CENTER_FILLER_LENGTH);

		// 센터필러 길이 (Edit컨트롤): 유로폼은 50~1500, 합판은 ~2400
		DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 280, 15, 70, 23);
		DGShowItem(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
		DGDisableItem(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
		DGSetItemMinDouble(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH, 0.050);
		DGSetItemMaxDouble(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH, 1.500);
		DGSetItemValDouble(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH, autoArrayInfo->centerFiller_length);

		// 그룹박스: 유로폼 사용여부
		DGAppendDialogItem(dialogID, DG_ITM_GROUPBOX, DG_GT_PRIMARY, 0, 115, 60, 170, 125);
		DGSetItemFont(dialogID, GROUPBOX_USE_EUROFORM, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, GROUPBOX_USE_EUROFORM, L"배치할 유로폼 길이 타입");
		DGShowItem(dialogID, GROUPBOX_USE_EUROFORM);

		// 유로폼 사용여부 (체크박스)
		DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 20 + 105, 90, 70, 25);
		DGSetItemFont(dialogID, CHECKBOX_USE_EUROFORM_1200, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, CHECKBOX_USE_EUROFORM_1200, "1200");
		DGShowItem(dialogID, CHECKBOX_USE_EUROFORM_1200);
		DGSetItemValLong(dialogID, CHECKBOX_USE_EUROFORM_1200, autoArrayInfo->bUseEuroform_1200);

		DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 20 + 105, 120, 70, 25);
		DGSetItemFont(dialogID, CHECKBOX_USE_EUROFORM_1050, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, CHECKBOX_USE_EUROFORM_1050, "1050");
		DGShowItem(dialogID, CHECKBOX_USE_EUROFORM_1050);
		DGSetItemValLong(dialogID, CHECKBOX_USE_EUROFORM_1050, autoArrayInfo->bUseEuroform_1050);

		DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 20 + 105, 150, 70, 25);
		DGSetItemFont(dialogID, CHECKBOX_USE_EUROFORM_0900, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, CHECKBOX_USE_EUROFORM_0900, "900");
		DGShowItem(dialogID, CHECKBOX_USE_EUROFORM_0900);
		DGSetItemValLong(dialogID, CHECKBOX_USE_EUROFORM_0900, autoArrayInfo->bUseEuroform_0900);

		DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 100 + 105, 90, 70, 25);
		DGSetItemFont(dialogID, CHECKBOX_USE_EUROFORM_0750, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, CHECKBOX_USE_EUROFORM_0750, "750");
		DGShowItem(dialogID, CHECKBOX_USE_EUROFORM_0750);
		DGSetItemValLong(dialogID, CHECKBOX_USE_EUROFORM_0750, autoArrayInfo->bUseEuroform_0750);

		DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 100 + 105, 120, 70, 25);
		DGSetItemFont(dialogID, CHECKBOX_USE_EUROFORM_0600, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, CHECKBOX_USE_EUROFORM_0600, "600");
		DGShowItem(dialogID, CHECKBOX_USE_EUROFORM_0600);
		DGSetItemValLong(dialogID, CHECKBOX_USE_EUROFORM_0600, autoArrayInfo->bUseEuroform_0600);

		DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 100 + 105, 150, 70, 25);
		DGSetItemFont(dialogID, CHECKBOX_USE_EUROFORM_ETC, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, CHECKBOX_USE_EUROFORM_ETC, L"이형폼");
		DGShowItem(dialogID, CHECKBOX_USE_EUROFORM_ETC);
		DGSetItemValLong(dialogID, CHECKBOX_USE_EUROFORM_ETC, autoArrayInfo->bUseEuroform_etc);

		// 센터필러 타입에 따라 팝업컨트롤/Edit컨트롤 사용 여부 결정
		// 센터필러 유로폼의 타입에 따라 Edit컨트롤 활성화 여부가 달라짐
		if (DGPopUpGetSelected(dialogID, POPUP_CENTER_FILLER_TYPE) == NONE + 1) {
			DGDisableItem(dialogID, POPUP_CENTER_FILLER_LENGTH);
			DGDisableItem(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
		}
		else if (DGPopUpGetSelected(dialogID, POPUP_CENTER_FILLER_TYPE) == EUROFORM + 1) {
			DGEnableItem(dialogID, POPUP_CENTER_FILLER_LENGTH);
			if (DGPopUpGetSelected(dialogID, POPUP_CENTER_FILLER_LENGTH) == 6)
				DGEnableItem(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
			else
				DGDisableItem(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
		}
		else if (DGPopUpGetSelected(dialogID, POPUP_CENTER_FILLER_TYPE) == PLYWOOD + 1) {
			DGDisableItem(dialogID, POPUP_CENTER_FILLER_LENGTH);
			DGEnableItem(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
		}
		else if (DGPopUpGetSelected(dialogID, POPUP_CENTER_FILLER_TYPE) == FILLERSP + 1) {
			DGDisableItem(dialogID, POPUP_CENTER_FILLER_LENGTH);
			DGEnableItem(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
		}

		break;

	case DG_MSG_CHANGE:
		// 센터필러 타입에 따라 팝업컨트롤/Edit컨트롤 사용 여부 결정
		// 센터필러 유로폼의 타입에 따라 Edit컨트롤 활성화 여부가 달라짐
		if (DGPopUpGetSelected(dialogID, POPUP_CENTER_FILLER_TYPE) == NONE + 1) {
			DGDisableItem(dialogID, POPUP_CENTER_FILLER_LENGTH);
			DGDisableItem(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
		}
		else if (DGPopUpGetSelected(dialogID, POPUP_CENTER_FILLER_TYPE) == EUROFORM + 1) {
			DGEnableItem(dialogID, POPUP_CENTER_FILLER_LENGTH);
			if (DGPopUpGetSelected(dialogID, POPUP_CENTER_FILLER_LENGTH) == 6)
				DGEnableItem(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
			else
				DGDisableItem(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
		}
		else if (DGPopUpGetSelected(dialogID, POPUP_CENTER_FILLER_TYPE) == PLYWOOD + 1) {
			DGDisableItem(dialogID, POPUP_CENTER_FILLER_LENGTH);
			DGEnableItem(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
		}
		else if (DGPopUpGetSelected(dialogID, POPUP_CENTER_FILLER_TYPE) == FILLERSP + 1) {
			DGDisableItem(dialogID, POPUP_CENTER_FILLER_LENGTH);
			DGEnableItem(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
		}

		break;

	case DG_MSG_CLICK:
		switch (item) {
		case DG_OK:
			autoArrayInfo->centerFiller_objType = DGPopUpGetSelected(dialogID, POPUP_CENTER_FILLER_TYPE) - 1;
			
			// 만약 NONE이면 0
			if (autoArrayInfo->centerFiller_objType == NONE) {
				autoArrayInfo->centerFiller_length = 0.0;
			}
			// EUROFORM이면 이형 사이즈인지 아닌지 여부에 따라 달라짐
			else if (autoArrayInfo->centerFiller_objType == EUROFORM) {
				if (DGPopUpGetSelected(dialogID, POPUP_CENTER_FILLER_LENGTH) == 6)
					autoArrayInfo->centerFiller_length = DGGetItemValDouble(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
				else
					autoArrayInfo->centerFiller_length = atof(DGPopUpGetItemText(dialogID, POPUP_CENTER_FILLER_LENGTH, DGPopUpGetSelected(dialogID, POPUP_CENTER_FILLER_LENGTH)).ToCStr().Get()) / 1000.0;
			}
			// PLYWOOD이면 Edit컨트롤의 값을 가져옴
			else if (autoArrayInfo->centerFiller_objType == PLYWOOD) {
				autoArrayInfo->centerFiller_length = DGGetItemValDouble(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
			}
			// FILLERSP이면 Edit컨트롤의 값을 가져옴
			else if (autoArrayInfo->centerFiller_objType == FILLERSP) {
				autoArrayInfo->centerFiller_length = DGGetItemValDouble(dialogID, EDITCONTROL_CENTER_FILLER_LENGTH);
			}

			autoArrayInfo->bUseEuroform_1200 = DGGetItemValLong(dialogID, CHECKBOX_USE_EUROFORM_1200);
			autoArrayInfo->bUseEuroform_1050 = DGGetItemValLong(dialogID, CHECKBOX_USE_EUROFORM_1050);
			autoArrayInfo->bUseEuroform_0900 = DGGetItemValLong(dialogID, CHECKBOX_USE_EUROFORM_0900);
			autoArrayInfo->bUseEuroform_0750 = DGGetItemValLong(dialogID, CHECKBOX_USE_EUROFORM_0750);
			autoArrayInfo->bUseEuroform_0600 = DGGetItemValLong(dialogID, CHECKBOX_USE_EUROFORM_0600);
			autoArrayInfo->bUseEuroform_etc = DGGetItemValLong(dialogID, CHECKBOX_USE_EUROFORM_ETC);

			break;

		case DG_CANCEL:
			break;
		}
		break;

	case DG_MSG_CLOSE:
		switch (item) {
		case DG_CLOSEBOX:
			break;
		}
	}

	result = item;

	return	result;
}

// 동바리/멍에제 프리셋을 배치할지 여부를 물어봄
short DGCALLBACK beamTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	short		xx;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, L"동바리/멍에제 프리셋 배치");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 확인 버튼
			DGSetItemText (dialogID, DG_OK, L"확 인");

			// 취소 버튼
			DGSetItemText (dialogID, DG_CANCEL, L"취 소");

			// 이전 버튼
			DGSetItemText (dialogID, DG_PREV_PERI, L"이 전");

			// 라벨
			DGSetItemText (dialogID, LABEL_TYPE_SUPPORTING_POST_PERI, L"타입");
			DGSetItemText (dialogID, LABEL_NUM_OF_POST_SET_PERI, L"동바리 세트 개수");
			DGSetItemText (dialogID, LABEL_PLAN_VIEW_PERI, L"평면도");
			DGSetItemText (dialogID, LABEL_BEAM_WIDTH_PERI, L"보 너비");
			DGSetItemText (dialogID, LABEL_BEAM_LENGTH_PERI, L"보 길이");
			DGSetItemText (dialogID, LABEL_OFFSET_1_PERI, L"시작 위치");
			DGSetItemText (dialogID, LABEL_GAP_WIDTH_DIRECTION_1_PERI, L"너비");
			DGSetItemText (dialogID, LABEL_GAP_LENGTH_DIRECTION_1_PERI, L"길이");
			DGSetItemText (dialogID, LABEL_OFFSET_2_PERI, L"시작 위치");
			DGSetItemText (dialogID, LABEL_GAP_WIDTH_DIRECTION_2_PERI, L"너비");
			DGSetItemText (dialogID, LABEL_GAP_LENGTH_DIRECTION_2_PERI, L"길이");

			// 라벨: 레이어 설정
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS_PERI, L"부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_VERTICAL_POST_PERI, L"PERI 수직재");
			DGSetItemText (dialogID, LABEL_LAYER_HORIZONTAL_POST_PERI, L"PERI 수평재");
			DGSetItemText (dialogID, LABEL_LAYER_GIRDER_PERI, L"GT24 거더");
			DGSetItemText (dialogID, LABEL_LAYER_BEAM_BRACKET_PERI, L"보 브라켓");
			DGSetItemText (dialogID, LABEL_LAYER_YOKE_PERI, L"보 멍에제");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER_PERI, L"각재");
			DGSetItemText (dialogID, LABEL_LAYER_JACK_SUPPORT_PERI, L"잭 서포트");

			// 체크박스: 레이어 묶음
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING_PERI, L"레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING_PERI, TRUE);

			DGSetItemText (dialogID, BUTTON_AUTOSET_PERI, L"레이어 자동 설정");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_VERTICAL_POST_PERI;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_VERTICAL_POST_PERI, 1);

			ucb.itemID	 = USERCONTROL_LAYER_HORIZONTAL_POST_PERI;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HORIZONTAL_POST_PERI, 1);

			ucb.itemID	 = USERCONTROL_LAYER_GIRDER_PERI;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_GIRDER_PERI, 1);

			ucb.itemID	 = USERCONTROL_LAYER_BEAM_BRACKET_PERI;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BEAM_BRACKET_PERI, 1);

			ucb.itemID	 = USERCONTROL_LAYER_YOKE_PERI;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_YOKE_PERI, 1);

			ucb.itemID	 = USERCONTROL_LAYER_TIMBER_PERI;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER_PERI, 1);

			ucb.itemID	 = USERCONTROL_LAYER_JACK_SUPPORT_PERI;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_JACK_SUPPORT_PERI, 1);

			// 팝업 컨트롤 항목 추가
			DGPopUpInsertItem (dialogID, POPUP_TYPE_SUPPORTING_POST_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE_SUPPORTING_POST_PERI, DG_POPUP_BOTTOM, L"PERI 동바리 + GT24 거더");
			DGPopUpInsertItem (dialogID, POPUP_TYPE_SUPPORTING_POST_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE_SUPPORTING_POST_PERI, DG_POPUP_BOTTOM, L"PERI 동바리 + 보 멍에제");
			DGPopUpSelectItem (dialogID, POPUP_TYPE_SUPPORTING_POST_PERI, DG_POPUP_TOP);

			DGPopUpInsertItem (dialogID, POPUP_NUM_OF_POST_SET_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_NUM_OF_POST_SET_PERI, DG_POPUP_BOTTOM, "1");
			DGPopUpInsertItem (dialogID, POPUP_NUM_OF_POST_SET_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_NUM_OF_POST_SET_PERI, DG_POPUP_BOTTOM, "2");
			DGPopUpSelectItem (dialogID, POPUP_NUM_OF_POST_SET_PERI, DG_POPUP_BOTTOM);

			DGPopUpInsertItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_WIDTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_WIDTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_WIDTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM, "1500");
			DGPopUpSelectItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_1_PERI, DG_POPUP_TOP);

			DGPopUpInsertItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM, "1500");
			DGPopUpSelectItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI, DG_POPUP_TOP);

			DGPopUpInsertItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI, DG_POPUP_BOTTOM, "2300");
			DGPopUpSelectItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI, DG_POPUP_TOP);

			DGPopUpInsertItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI, DG_POPUP_BOTTOM, "2300");
			DGPopUpSelectItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI, DG_POPUP_TOP);

			// 보 너비/길이 계산
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH_PERI, placingZone.areaWidth_Bottom + placingZone.gapSideLeft + placingZone.gapSideRight);	// 보 너비
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_LENGTH_PERI, placingZone.beamLength);															// 보 길이

			// 직접 변경해서는 안 되는 항목 잠그기
			DGDisableItem (dialogID, EDITCONTROL_BEAM_WIDTH_PERI);
			DGDisableItem (dialogID, EDITCONTROL_BEAM_LENGTH_PERI);
			DGDisableItem (dialogID, EDITCONTROL_OFFSET_2_PERI);
			DGDisableItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI);
			DGDisableItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI);

			// 레이어 옵션 활성화/비활성화
			if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SUPPORTING_POST_PERI) == 1) {
				DGEnableItem (dialogID, LABEL_LAYER_GIRDER_PERI);
				DGEnableItem (dialogID, USERCONTROL_LAYER_GIRDER_PERI);
				DGEnableItem (dialogID, LABEL_LAYER_BEAM_BRACKET_PERI);
				DGEnableItem (dialogID, USERCONTROL_LAYER_BEAM_BRACKET_PERI);
				DGDisableItem (dialogID, LABEL_LAYER_YOKE_PERI);
				DGDisableItem (dialogID, USERCONTROL_LAYER_YOKE_PERI);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_GIRDER_PERI);
				DGDisableItem (dialogID, USERCONTROL_LAYER_GIRDER_PERI);
				DGDisableItem (dialogID, LABEL_LAYER_BEAM_BRACKET_PERI);
				DGDisableItem (dialogID, USERCONTROL_LAYER_BEAM_BRACKET_PERI);
				DGEnableItem (dialogID, LABEL_LAYER_YOKE_PERI);
				DGEnableItem (dialogID, USERCONTROL_LAYER_YOKE_PERI);
			}

			break;

		case DG_MSG_CHANGE:
			// 레이어 옵션 활성화/비활성화
			if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SUPPORTING_POST_PERI) == 1) {
				DGEnableItem (dialogID, LABEL_LAYER_GIRDER_PERI);
				DGEnableItem (dialogID, USERCONTROL_LAYER_GIRDER_PERI);
				DGEnableItem (dialogID, LABEL_LAYER_BEAM_BRACKET_PERI);
				DGEnableItem (dialogID, USERCONTROL_LAYER_BEAM_BRACKET_PERI);
				DGDisableItem (dialogID, LABEL_LAYER_YOKE_PERI);
				DGDisableItem (dialogID, USERCONTROL_LAYER_YOKE_PERI);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_GIRDER_PERI);
				DGDisableItem (dialogID, USERCONTROL_LAYER_GIRDER_PERI);
				DGDisableItem (dialogID, LABEL_LAYER_BEAM_BRACKET_PERI);
				DGDisableItem (dialogID, USERCONTROL_LAYER_BEAM_BRACKET_PERI);
				DGEnableItem (dialogID, LABEL_LAYER_YOKE_PERI);
				DGEnableItem (dialogID, USERCONTROL_LAYER_YOKE_PERI);
			}

			// 시작 위치
			DGSetItemValDouble (dialogID, EDITCONTROL_OFFSET_2_PERI, DGGetItemValDouble (dialogID, EDITCONTROL_OFFSET_1_PERI));

			// 너비
			DGPopUpSelectItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI, DGPopUpGetSelected (dialogID, POPUP_GAP_WIDTH_DIRECTION_1_PERI));

			// 길이
			DGPopUpSelectItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI, DGPopUpGetSelected (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI));

			// 항목 보이기/숨기기
			if (DGPopUpGetSelected (dialogID, POPUP_NUM_OF_POST_SET_PERI) == 1) {
				// 동바리 세트 개수가 1개일 경우
				DGHideItem (dialogID, SEPARATOR_SUPPORTING_POST2_PERI);
				DGHideItem (dialogID, LABEL_OFFSET_2_PERI);
				DGHideItem (dialogID, EDITCONTROL_OFFSET_2_PERI);
				DGHideItem (dialogID, LABEL_GAP_WIDTH_DIRECTION_2_PERI);
				DGHideItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI);
				DGHideItem (dialogID, LABEL_GAP_LENGTH_DIRECTION_2_PERI);
				DGHideItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI);
			} else {
				// 동바리 세트 개수가 2개일 경우
				DGShowItem (dialogID, SEPARATOR_SUPPORTING_POST2_PERI);
				DGShowItem (dialogID, LABEL_OFFSET_2_PERI);
				DGShowItem (dialogID, EDITCONTROL_OFFSET_2_PERI);
				DGShowItem (dialogID, LABEL_GAP_WIDTH_DIRECTION_2_PERI);
				DGShowItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI);
				DGShowItem (dialogID, LABEL_GAP_LENGTH_DIRECTION_2_PERI);
				DGShowItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI);
			}

			// 레이어 같이 바뀜
			if ((item >= USERCONTROL_LAYER_VERTICAL_POST_PERI) && (item <= USERCONTROL_LAYER_JACK_SUPPORT_PERI)) {
				if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING_PERI) == 1) {
					long selectedLayer;

					selectedLayer = DGGetItemValLong (dialogID, item);

					for (xx = USERCONTROL_LAYER_VERTICAL_POST_PERI ; xx <= USERCONTROL_LAYER_JACK_SUPPORT_PERI ; ++xx)
						DGSetItemValLong (dialogID, xx, selectedLayer);
				}
			}

			break;
		
		case DG_MSG_CLICK:
			switch (item) {
				case DG_PREV_PERI:
					clickedPrevButton = true;
					break;

				case DG_OK:
					clickedOKButton = true;

					placingZone.typeOfSupportingPost = DGPopUpGetSelected (dialogID, POPUP_TYPE_SUPPORTING_POST_PERI);		// 타입
					placingZone.numOfSupportingPostSet = (short)atoi (DGPopUpGetItemText (dialogID, POPUP_NUM_OF_POST_SET_PERI, DGPopUpGetSelected (dialogID, POPUP_NUM_OF_POST_SET_PERI)).ToCStr ());	// 동바리 세트 개수

					placingZone.postStartOffset = DGGetItemValDouble (dialogID, EDITCONTROL_OFFSET_1_PERI);																										// 시작 위치
					placingZone.postGapWidth = atof (DGPopUpGetItemText (dialogID, POPUP_GAP_WIDTH_DIRECTION_1_PERI, DGPopUpGetSelected (dialogID, POPUP_GAP_WIDTH_DIRECTION_1_PERI)).ToCStr ()) / 1000.0;		// 너비
					placingZone.postGapLength = atof (DGPopUpGetItemText (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI, DGPopUpGetSelected (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI)).ToCStr ()) / 1000.0;	// 길이

					// 레이어 번호 저장
					layerInd_VerticalPost	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_VERTICAL_POST_PERI);
					layerInd_HorizontalPost	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HORIZONTAL_POST_PERI);
					layerInd_Girder			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_GIRDER_PERI);
					layerInd_BeamBracket	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BEAM_BRACKET_PERI);
					layerInd_Yoke			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_YOKE_PERI);
					layerInd_Timber			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER_PERI);
					layerInd_JackSupport	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JACK_SUPPORT_PERI);

					break;

				case BUTTON_AUTOSET_PERI:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING_PERI, FALSE);

					layerInd_VerticalPost	= makeTemporaryLayer (structuralObject_forTableformBeam, "MULT", NULL);
					layerInd_HorizontalPost	= makeTemporaryLayer (structuralObject_forTableformBeam, "TRUS", NULL);
					layerInd_Girder			= makeTemporaryLayer (structuralObject_forTableformBeam, "GIDR", NULL);
					layerInd_BeamBracket	= makeTemporaryLayer (structuralObject_forTableformBeam, "BBRK", NULL);
					layerInd_Yoke			= makeTemporaryLayer (structuralObject_forTableformBeam, "YOKE", NULL);
					layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformBeam, "TIMB", NULL);
					layerInd_JackSupport	= makeTemporaryLayer (structuralObject_forTableformBeam, "JACK", NULL);

					DGSetItemValLong (dialogID, USERCONTROL_LAYER_VERTICAL_POST_PERI, layerInd_VerticalPost);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HORIZONTAL_POST_PERI, layerInd_HorizontalPost);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_GIRDER_PERI, layerInd_Girder);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_BEAM_BRACKET_PERI, layerInd_BeamBracket);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_YOKE_PERI, layerInd_Yoke);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER_PERI, layerInd_Timber);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JACK_SUPPORT_PERI, layerInd_JackSupport);

					break;

				case DG_CANCEL:
					break;
			}
		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// 보과 테이블폼 사이에 단열재를 넣을지 여부를 물어보는 다이얼로그
short DGCALLBACK beamTableformPlacerHandler4_Insulation (short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	std::wstring*	title = (std::wstring*) userData;
	short	result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 타이틀
			DGSetDialogTitle (dialogID, title->c_str ());

			// 라벨
			DGSetItemText (dialogID, LABEL_EXPLANATION_INS, L"단열재 규격을 입력하십시오.");
			DGSetItemText (dialogID, LABEL_INSULATION_THK, L"두께");
			DGSetItemText (dialogID, LABEL_INS_HORLEN, L"가로");
			DGSetItemText (dialogID, LABEL_INS_VERLEN, L"세로");

			// 체크박스
			DGSetItemText (dialogID, CHECKBOX_INS_LIMIT_SIZE, L"가로/세로 크기 제한");
			DGSetItemValLong (dialogID, CHECKBOX_INS_LIMIT_SIZE, TRUE);

			// Edit 컨트롤
			DGSetItemValDouble (dialogID, EDITCONTROL_INS_HORLEN, 0.900);
			DGSetItemValDouble (dialogID, EDITCONTROL_INS_VERLEN, 1.800);

			// 레이어
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_INSULATION_LAYER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_INSULATION_LAYER, 1);

			// 버튼
			DGSetItemText (dialogID, DG_OK, L"확인");
			DGSetItemText (dialogID, DG_CANCEL, L"취소");

			// 두께는 자동
			if (title->compare (L"단열재 배치 (보 좌측면)") == 0) {
				DGSetItemValDouble (dialogID, EDITCONTROL_INSULATION_THK, placingZone.gapSideLeft);
			} else if (title->compare (L"단열재 배치 (보 우측면)") == 0) {
				DGSetItemValDouble (dialogID, EDITCONTROL_INSULATION_THK, placingZone.gapSideRight);
			} else {
				DGSetItemValDouble (dialogID, EDITCONTROL_INSULATION_THK, placingZone.gapBottom);
			}
			DGDisableItem (dialogID, EDITCONTROL_INSULATION_THK);
 
			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case CHECKBOX_INS_LIMIT_SIZE:
					if (DGGetItemValLong (dialogID, CHECKBOX_INS_LIMIT_SIZE) == TRUE) {
						DGEnableItem (dialogID, EDITCONTROL_INS_HORLEN);
						DGEnableItem (dialogID, EDITCONTROL_INS_VERLEN);
					} else {
						DGDisableItem (dialogID, EDITCONTROL_INS_HORLEN);
						DGDisableItem (dialogID, EDITCONTROL_INS_VERLEN);
					}
					break;
			}
 
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 레이어 정보 저장
					insulElem.layerInd = (short)DGGetItemValLong (dialogID, USERCONTROL_INSULATION_LAYER);

					// 두께, 가로, 세로 저장
					insulElem.thk = DGGetItemValDouble (dialogID, EDITCONTROL_INSULATION_THK);
					insulElem.maxHorLen = DGGetItemValDouble (dialogID, EDITCONTROL_INS_HORLEN);
					insulElem.maxVerLen = DGGetItemValDouble (dialogID, EDITCONTROL_INS_VERLEN);
					if (DGGetItemValLong (dialogID, CHECKBOX_INS_LIMIT_SIZE) == TRUE)
						insulElem.bLimitSize = true;
					else
						insulElem.bLimitSize = false;

					break;
				case DG_CANCEL:
					break;
			}
			break;

		case DG_MSG_CLOSE:
			break;
	}

	result = item;

	return	result;
}