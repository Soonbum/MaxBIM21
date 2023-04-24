#include <cstdio>
#include <cstdlib>
#include "MaxBIM21.h"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.h"
#include "LowSideTableformPlacer.h"

namespace lowSideTableformPlacerDG {
	LowSideTableformPlacingZone	placingZone;		// 낮은 슬래브 측면 영역 정보

	InfoWall						infoWall;			// 벽 객체 정보
	InfoBeam						infoBeam;			// 보 객체 정보
	InfoSlab						infoSlab;			// 슬래브 객체 정보
	InfoMesh						infoMesh;			// 메시 객체 정보
	short floorInd;

	API_Guid		structuralObject_forTableformLowSide;	// 구조 객체의 GUID

	short	layerInd_Euroform;			// 레이어 번호: 유로폼 (공통)
	short	layerInd_RectPipe;			// 레이어 번호: 비계 파이프 (공통)
	short	layerInd_PinBolt;			// 레이어 번호: 핀볼트 세트
	short	layerInd_HeadPiece;			// 레이어 번호: 헤드피스
	short	layerInd_Props;				// 레이어 번호: Push-Pull Props
	short	layerInd_Join;				// 레이어 번호: 결합철물
	short	layerInd_Plywood;			// 레이어 번호: 합판 (공통)
	short	layerInd_Timber;			// 레이어 번호: 각재 (공통)
	short	layerInd_EuroformHook;		// 레이어 번호: 유로폼 후크
	short	layerInd_CrossJointBar;		// 레이어 번호: 십자 조인트 바
	short	layerInd_BlueClamp;			// 레이어 번호: 블루 클램프
	short	layerInd_BlueTimberRail;	// 레이어 번호: 블루 목심

	short	layerInd_Fillersp;			// 레이어 번호: 휠러스페이서
	short	layerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
	short	layerInd_OutcornerPanel;	// 레이어 번호: 아웃코너판넬
	short	layerInd_IncornerPanel;		// 레이어 번호: 인코너판넬
	short	layerInd_RectpipeHanger;	// 레이어 번호: 각파이프 행거

	bool		bLayerInd_Euroform;			// 레이어 번호: 유로폼
	bool		bLayerInd_RectPipe;			// 레이어 번호: 비계 파이프
	bool		bLayerInd_PinBolt;			// 레이어 번호: 핀볼트 세트
	bool		bLayerInd_WallTie;			// 레이어 번호: 벽체 타이
	bool		bLayerInd_HeadPiece;		// 레이어 번호: 헤드피스
	bool		bLayerInd_Props;			// 레이어 번호: Push-Pull Props
	bool		bLayerInd_Join;				// 레이어 번호: 결합철물
	bool		bLayerInd_Plywood;			// 레이어 번호: 합판
	bool		bLayerInd_Timber;			// 레이어 번호: 각재
	bool		bLayerInd_EuroformHook;		// 레이어 번호: 유로폼 후크
	bool		bLayerInd_CrossJointBar;	// 레이어 번호: 십자 조인트 바
	bool		bLayerInd_BlueClamp;		// 레이어 번호: 블루 클램프
	bool		bLayerInd_BlueTimberRail;	// 레이어 번호: 블루 목심

	bool		bLayerInd_Fillersp;			// 레이어 번호: 휠러스페이서
	bool		bLayerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
	bool		bLayerInd_OutcornerPanel;	// 레이어 번호: 아웃코너판넬
	bool		bLayerInd_IncornerPanel;	// 레이어 번호: 인코너판넬
	bool		bLayerInd_RectpipeHanger;	// 레이어 번호: 각파이프 행거

	GS::Array<API_Guid>	elemList;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함

	int	clickedIndex;	// 클릭한 버튼의 인덱스
}

using namespace lowSideTableformPlacerDG;

// 낮은 슬래브 측면에 테이블폼을 배치하는 통합 루틴
GSErrCode	placeTableformOnLowSide (void)
{
	GSErrCode	err = NoError;
	short		result;
	double		dx, dy;
	
	GS::Array<API_Guid>		wall;
	GS::Array<API_Guid>		beam;
	GS::Array<API_Guid>		slab;
	GS::Array<API_Guid>		mesh;
	GS::Array<API_Guid>		morph;

	long	nWall;
	long	nBeam;
	long	nSlab;
	long	nMesh;
	long	nMorph;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElemInfo3D			info3D;

	// 모프 객체 정보
	InfoMorphForLowSideTableform	infoMorph;

	// 작업 층 정보
	double			workLevel_structural;	// 구조 요소의 작업 층 높이

	// 선택한 요소 가져오기 (구조 요소 1개, 영역 모프 1개 선택해야 함)
	err = getGuidsOfSelection (&wall, API_WallID, &nWall);
	err = getGuidsOfSelection (&beam, API_BeamID, &nBeam);
	err = getGuidsOfSelection (&slab, API_SlabID, &nSlab);
	err = getGuidsOfSelection (&mesh, API_MeshID, &nMesh);
	err = getGuidsOfSelection (&morph, API_MorphID, &nMorph);
	if (err == APIERR_NOPLAN) {
		DGAlert (DG_ERROR, L"오류", L"열린 프로젝트 창이 없습니다.", "", L"확인", "", "");
	}
	if (err == APIERR_NOSEL) {
		DGAlert (DG_ERROR, L"오류", L"아무 것도 선택하지 않았습니다.\n필수 선택: 구조 요소 (벽, 보, 슬래브, 메시 중 1개만), 구조 요소 측면을 덮는 모프 (1개)", "", L"확인", "", "");
	}

	// 구조 요소가 1개인가?
	if (nWall + nBeam + nSlab + nMesh != 1) {
		DGAlert (DG_ERROR, L"오류", L"구조 요소(벽, 보, 슬래브, 메시)를 1개 선택해야 합니다.", "", L"확인", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개인가?
	if (nMorph != 1) {
		DGAlert (DG_ERROR, L"오류", L"구조 요소의 측면을 덮는 모프를 1개 선택해야 합니다.", "", L"확인", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// 벽 정보를 가져옴
	if (nWall == 1) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = wall.Pop ();
		structuralObject_forTableformLowSide = elem.header.guid;
		err = ACAPI_Element_Get (&elem);

		infoWall.guid			= elem.header.guid;
		infoWall.floorInd		= elem.header.floorInd;
		infoWall.wallThk		= elem.wall.thickness;
		infoWall.bottomOffset	= elem.wall.bottomOffset;
		infoWall.begX			= elem.wall.begC.x;
		infoWall.begY			= elem.wall.begC.y;
		infoWall.endX			= elem.wall.endC.x;
		infoWall.endY			= elem.wall.endC.y;
	}

	// 보 정보를 가져옴
	else if (nBeam == 1) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = beam.Pop ();
		structuralObject_forTableformLowSide = elem.header.guid;
		err = ACAPI_Element_Get (&elem);

		infoBeam.guid			= elem.header.guid;
		infoBeam.floorInd		= elem.header.floorInd;
		infoBeam.begC			= elem.beam.begC;
		infoBeam.endC			= elem.beam.endC;
		infoBeam.height			= elem.beam.height;		// elem.beamSegment.assemblySegmentData.nominalHeight;
		infoBeam.level			= elem.beam.level;
		infoBeam.offset			= elem.beam.offset;
		infoBeam.width			= elem.beam.width;		// elem.beamSegment.assemblySegmentData.nominalWidth;
	}

	// 슬래브 정보를 가져옴
	else if (nSlab == 1) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = slab.Pop ();
		structuralObject_forTableformLowSide = elem.header.guid;
		err = ACAPI_Element_Get (&elem);

		infoSlab.guid			= elem.header.guid;
		infoSlab.floorInd		= elem.header.floorInd;
		infoSlab.level			= elem.slab.level;
		infoSlab.offsetFromTop	= elem.slab.offsetFromTop;
		infoSlab.thickness		= elem.slab.thickness;
	}

	// 메시 정보를 가져옴
	else if (nMesh == 1) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = mesh.Pop ();
		structuralObject_forTableformLowSide = elem.header.guid;
		err = ACAPI_Element_Get (&elem);

		infoMesh.guid			= elem.header.guid;
		infoMesh.floorInd		= elem.header.floorInd;
		infoMesh.level			= elem.mesh.level;
		infoMesh.skirtLevel		= elem.mesh.skirtLevel;
	}

	// 모프 정보를 가져옴
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morph.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	if (abs (info3D.bounds.zMax - info3D.bounds.zMin) < EPS) {
		DGAlert (DG_ERROR, L"오류", L"모프가 세워져 있지 않습니다.", "", L"확인", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	infoMorph.guid = elem.header.guid;

	// 모프의 좌하단, 우상단 점 지정
	if (abs (elem.morph.tranmat.tmx [11] - info3D.bounds.zMin) < EPS) {
		// 좌하단 좌표 결정
		infoMorph.leftBottomX = elem.morph.tranmat.tmx [3];
		infoMorph.leftBottomY = elem.morph.tranmat.tmx [7];
		infoMorph.leftBottomZ = elem.morph.tranmat.tmx [11];

		// 우상단 좌표는?
		if (abs (infoMorph.leftBottomX - info3D.bounds.xMin) < EPS)
			infoMorph.rightTopX = info3D.bounds.xMax;
		else
			infoMorph.rightTopX = info3D.bounds.xMin;
		if (abs (infoMorph.leftBottomY - info3D.bounds.yMin) < EPS)
			infoMorph.rightTopY = info3D.bounds.yMax;
		else
			infoMorph.rightTopY = info3D.bounds.yMin;
		if (abs (infoMorph.leftBottomZ - info3D.bounds.zMin) < EPS)
			infoMorph.rightTopZ = info3D.bounds.zMax;
		else
			infoMorph.rightTopZ = info3D.bounds.zMin;
	} else {
		// 우상단 좌표 결정
		infoMorph.rightTopX = elem.morph.tranmat.tmx [3];
		infoMorph.rightTopY = elem.morph.tranmat.tmx [7];
		infoMorph.rightTopZ = elem.morph.tranmat.tmx [11];

		// 좌하단 좌표는?
		if (abs (infoMorph.rightTopX - info3D.bounds.xMin) < EPS)
			infoMorph.leftBottomX = info3D.bounds.xMax;
		else
			infoMorph.leftBottomX = info3D.bounds.xMin;
		if (abs (infoMorph.rightTopY - info3D.bounds.yMin) < EPS)
			infoMorph.leftBottomY = info3D.bounds.yMax;
		else
			infoMorph.leftBottomY = info3D.bounds.yMin;
		if (abs (infoMorph.rightTopZ - info3D.bounds.zMin) < EPS)
			infoMorph.leftBottomZ = info3D.bounds.zMax;
		else
			infoMorph.leftBottomZ = info3D.bounds.zMin;
	}

	// 모프의 Z축 회전 각도 (벽의 설치 각도)
	dx = infoMorph.rightTopX - infoMorph.leftBottomX;
	dy = infoMorph.rightTopY - infoMorph.leftBottomY;
	infoMorph.ang = RadToDegree (atan2 (dy, dx));

	// 모프의 가로 길이
	infoMorph.horLen = GetDistance (info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.xMax, info3D.bounds.yMax);
	
	// 모프의 세로 길이
	infoMorph.verLen = abs (info3D.bounds.zMax - info3D.bounds.zMin);

	// 영역 모프 제거
	GS::Array<API_Element>	elems;
	elems.Push (elem);
	deleteElements (elems);

	// 작업 층 높이 반영 -- 모프
	if (nWall == 1)			floorInd = infoWall.floorInd;
	else if (nBeam == 1)	floorInd = infoBeam.floorInd;
	else if (nSlab == 1)	floorInd = infoSlab.floorInd;
	else if (nMesh == 1)	floorInd = infoMesh.floorInd;

	workLevel_structural = getWorkLevel (floorInd);

	// 영역 정보를 업데이트
	placingZone.leftBottomX = infoMorph.leftBottomX;
	placingZone.leftBottomY = infoMorph.leftBottomY;
	placingZone.leftBottomZ = infoMorph.leftBottomZ;
	placingZone.horLen = infoMorph.horLen;
	placingZone.verLen = infoMorph.verLen;
	placingZone.ang = DegreeToRad (infoMorph.ang);

	if (nWall == 1)			placingZone.leftBottomZ = infoWall.bottomOffset;
	else if (nBeam == 1)	placingZone.leftBottomZ = infoBeam.level - infoBeam.height;
	else if (nSlab == 1)	placingZone.leftBottomZ = infoSlab.level + infoSlab.offsetFromTop - infoSlab.thickness;
	else if (nMesh == 1)	placingZone.leftBottomZ = infoMesh.level - infoMesh.skirtLevel;

	// 테이블폼 방향 결정 (600mm 미만이면 가로, 이상이면 세로)
	if (placingZone.verLen < 0.600 - EPS) {
		placingZone.bVertical = false;
	} else {
		placingZone.bVertical = true;
	}

	// 수평 방향 셀 개수
	placingZone.nCellsInHor = (short)(floor (placingZone.horLen / 3.600));

FIRST:

	// [DIALOG] 1번째 다이얼로그에서 인코너판넬/아웃코너판넬/아웃코너앵글 유무 및 길이, 테이블폼의 방향과 가로/세로 방향 유로폼의 개수와 각각의 길이를 선택함
	result = DGBlankModalDialog (600, 350, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, lowSideTableformPlacerHandler1, 0);

	if (result != DG_OK)
		return err;

	// [DIALOG] 2번째 다이얼로그에서 부재별 레이어를 지정함
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32601, ACAPI_GetOwnResModule (), lowSideTableformPlacerHandler2, 0);

	if (result != DG_OK)
		goto FIRST;

	// 객체 배치하기
	placingZone.placeObjects (&placingZone);

	return	err;
}

// 셀 정보 초기화
void	LowSideTableformPlacingZone::initCells (LowSideTableformPlacingZone* placingZone, bool bVertical)
{
	short	xx;

	// 셀 정보 채우기 (셀 너비 정보만 미리 채움)
	// 세로방향이면
	if (bVertical == true) {
		for (xx = 0 ; xx < sizeof (placingZone->cells) / sizeof (CellForLowSideTableform) ; ++xx) {
			placingZone->cells [xx].objType = TABLEFORM;
			placingZone->cells [xx].horLen = 3600;
			placingZone->cells [xx].tableInHor [0] = 600;
			placingZone->cells [xx].tableInHor [1] = 600;
			placingZone->cells [xx].tableInHor [2] = 600;
			placingZone->cells [xx].tableInHor [3] = 600;
			placingZone->cells [xx].tableInHor [4] = 600;
			placingZone->cells [xx].tableInHor [5] = 600;
			placingZone->cells [xx].tableInHor [6] = 0;
			placingZone->cells [xx].tableInHor [7] = 0;
			placingZone->cells [xx].tableInHor [8] = 0;
			placingZone->cells [xx].tableInHor [9] = 0;
		}

	// 가로방향이면
	} else {
		for (xx = 0 ; xx < sizeof (placingZone->cells) / sizeof (CellForLowSideTableform) ; ++xx) {
			placingZone->cells [xx].objType = TABLEFORM;
			placingZone->cells [xx].horLen = 3600;
			placingZone->cells [xx].tableInHor [0] = 1200;
			placingZone->cells [xx].tableInHor [1] = 1200;
			placingZone->cells [xx].tableInHor [2] = 1200;
			placingZone->cells [xx].tableInHor [3] = 0;
			placingZone->cells [xx].tableInHor [4] = 0;
			placingZone->cells [xx].tableInHor [5] = 0;
			placingZone->cells [xx].tableInHor [6] = 0;
			placingZone->cells [xx].tableInHor [7] = 0;
			placingZone->cells [xx].tableInHor [8] = 0;
			placingZone->cells [xx].tableInHor [9] = 0;
		}
	}
}

// 셀(0-기반 인덱스 번호)의 좌하단 점 위치 X 좌표를 구함
double	LowSideTableformPlacingZone::getCellPositionLeftBottomX (LowSideTableformPlacingZone* placingZone, short idx)
{
	double	distance = 0.0;

	if ((placingZone->typeLcorner == INCORNER_PANEL) || (placingZone->typeLcorner == OUTCORNER_PANEL))
		distance = placingZone->lenLcorner;

	for (short xx = 0 ; xx < idx ; ++xx)
		distance += (double)placingZone->cells [xx].horLen / 1000.0;

	return distance;
}

// 셀 위치를 바르게 교정함
void	LowSideTableformPlacingZone::adjustCellsPosition (LowSideTableformPlacingZone* placingZone)
{
	for (short xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		placingZone->cells [xx].ang = placingZone->ang;
		placingZone->cells [xx].leftBottomX = placingZone->leftBottomX + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cells [xx].leftBottomY = placingZone->leftBottomY + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cells [xx].leftBottomZ = placingZone->leftBottomZ;
	}
}

// 셀 정보를 기반으로 객체들을 배치함
GSErrCode	LowSideTableformPlacingZone::placeObjects (LowSideTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx;

	// ================================================== 인코너 배치
	// 좌측 인코너/아웃코너 배치
	if (placingZone->typeLcorner == INCORNER_PANEL) {
		EasyObjectPlacement incornerPanel;
		incornerPanel.init (L("인코너판넬v1.0.gsm"), layerInd_IncornerPanel, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang - DegreeToRad (90.0));
		incornerPanel.placeObject (5,
			"in_comp", APIParT_CString, "인코너판넬",
			"wid_s", APIParT_Length, format_string ("%f", 0.100).c_str(),
			"leng_s", APIParT_Length, format_string ("%f", placingZone->lenLcorner).c_str(),
			"hei_s", APIParT_Length, format_string ("%f", placingZone->verLen).c_str(),
			"dir_s", APIParT_CString, "세우기");
	} else if (placingZone->typeLcorner == OUTCORNER_PANEL) {
		EasyObjectPlacement outcornerPanel;
		outcornerPanel.init (L("아웃코너판넬v1.0.gsm"), layerInd_OutcornerPanel, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);
		outcornerPanel.placeObject (4,
			"wid_s", APIParT_Length, format_string ("%f", placingZone->lenLcorner).c_str(),
			"leng_s", APIParT_Length, format_string ("%f", 0.100).c_str(),
			"hei_s", APIParT_Length, format_string ("%f", placingZone->verLen).c_str(),
			"dir_s", APIParT_CString, "세우기");
	} else if (placingZone->typeLcorner == OUTCORNER_ANGLE) {
		EasyObjectPlacement outcornerAngle;
		outcornerAngle.init (L("아웃코너앵글v1.0.gsm"), layerInd_OutcornerAngle, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));
		outcornerAngle.placeObject (2,
			"a_leng", APIParT_Length, format_string ("%f", placingZone->verLen).c_str(),
			"a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str());
	}

	// 우측 인코너/아웃코너 배치
	if (placingZone->typeRcorner == INCORNER_PANEL) {
		EasyObjectPlacement incornerPanel;
		incornerPanel.init (L("인코너판넬v1.0.gsm"), layerInd_IncornerPanel, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang - DegreeToRad (180.0));
		moveIn3D ('x', incornerPanel.radAng + DegreeToRad (180.0), placingZone->horLen, &incornerPanel.posX, &incornerPanel.posY, &incornerPanel.posZ);
		incornerPanel.placeObject (5,
			"in_comp", APIParT_CString, "인코너판넬",
			"wid_s", APIParT_Length, format_string ("%f", placingZone->lenRcorner).c_str(),
			"leng_s", APIParT_Length, format_string ("%f", 0.100).c_str(),
			"hei_s", APIParT_Length, format_string ("%f", placingZone->verLen).c_str(),
			"dir_s", APIParT_CString, "세우기");
	} else if (placingZone->typeRcorner == OUTCORNER_PANEL) {
		EasyObjectPlacement outcornerPanel;
		outcornerPanel.init (L("아웃코너판넬v1.0.gsm"), layerInd_OutcornerPanel, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (90.0));
		moveIn3D ('x', outcornerPanel.radAng - DegreeToRad (90.0), placingZone->horLen, &outcornerPanel.posX, &outcornerPanel.posY, &outcornerPanel.posZ);
		outcornerPanel.placeObject (4,
			"wid_s", APIParT_Length, format_string ("%f", 0.100).c_str(),
			"leng_s", APIParT_Length, format_string ("%f", placingZone->lenRcorner).c_str(),
			"hei_s", APIParT_Length, format_string ("%f", placingZone->verLen).c_str(),
			"dir_s", APIParT_CString, "세우기");
	} else if (placingZone->typeRcorner == OUTCORNER_ANGLE) {
		EasyObjectPlacement outcornerAngle;
		outcornerAngle.init (L("아웃코너앵글v1.0.gsm"), layerInd_OutcornerAngle, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (270.0));
		moveIn3D ('x', outcornerAngle.radAng - DegreeToRad (270.0), placingZone->horLen, &outcornerAngle.posX, &outcornerAngle.posY, &outcornerAngle.posZ);
		outcornerAngle.placeObject (2,
			"a_leng", APIParT_Length, format_string ("%f", placingZone->verLen).c_str(),
			"a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str());
	}

	// ================================================== 유로폼 배치
	EasyObjectPlacement euroform;
	euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	moveIn3D ('x', euroform.radAng, placingZone->lenLcorner, &euroform.posX, &euroform.posY, &euroform.posZ);		// 좌측 코너 길이만큼 x 이동

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == EUROFORM) {
			if (placingZone->bVertical == true) {
				// 세로방향
				elemList.Push (euroform.placeObject (5,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen).c_str(),
					"eu_hei", APIParT_CString, format_string ("%d", (int)(placingZone->verLen * 1000)).c_str(),
					"u_ins", APIParT_CString, "벽세우기",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
			} else {
				// 가로방향
				moveIn3D ('x', euroform.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
				elemList.Push (euroform.placeObject (5,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%d", (int)(placingZone->verLen * 1000)).c_str(),
					"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen).c_str(),
					"u_ins", APIParT_CString, "벽눕히기",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
				moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}

			// 결과물 전체 그룹화
			groupElements (elemList);
			elemList.Clear ();
		}

		// 무조건 가로 방향으로 이동
		moveIn3D ('x', euroform.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	}

	// ================================================== 휠러스페이서 배치 (항상 세로방향)
	EasyObjectPlacement fillersp;
	fillersp.init (L("휠러스페이서v1.0.gsm"), layerInd_Fillersp, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	moveIn3D ('x', fillersp.radAng, placingZone->lenLcorner, &fillersp.posX, &fillersp.posY, &fillersp.posZ);		// 좌측 코너 길이만큼 x 이동

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == FILLERSP) {
			moveIn3D ('x', fillersp.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
			elemList.Push (fillersp.placeObject (4,
				"f_thk", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0).c_str(),
				"f_leng", APIParT_Length, format_string ("%f", placingZone->verLen).c_str(),
				"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(),
				"f_rota", APIParT_Angle, format_string ("%f", 0.0).c_str()));

			moveIn3D ('x', fillersp.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

			// 결과물 전체 그룹화
			groupElements (elemList);
			elemList.Clear ();
		}

		// 무조건 가로 방향으로 이동
		moveIn3D ('x', fillersp.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
	}

	// ================================================== 합판 배치 (항상 세로방향)
	EasyObjectPlacement plywood;
	plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	moveIn3D ('x', plywood.radAng, placingZone->lenLcorner, &plywood.posX, &plywood.posY, &plywood.posZ);		// 좌측 코너 길이만큼 x 이동

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == PLYWOOD) {
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "벽세우기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->verLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));

			// 결과물 전체 그룹화
			groupElements (elemList);
			elemList.Clear ();
		}

		// 무조건 가로 방향으로 이동
		moveIn3D ('x', plywood.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
	}

	// ================================================== 각재 배치 (항상 세로방향)
	EasyObjectPlacement timber;
	timber.init (L("목재v1.0.gsm"), layerInd_Timber, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	moveIn3D ('x', timber.radAng, placingZone->lenLcorner, &timber.posX, &timber.posY, &timber.posZ);		// 좌측 코너 길이만큼 x 이동

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == TIMBER) {
			moveIn3D ('x', timber.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
			elemList.Push (timber.placeObject (6,
				"w_ins", APIParT_CString, "벽세우기",
				"w_w", APIParT_Length, format_string ("%f", 0.050).c_str(),
				"w_h", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0).c_str(),
				"w_leng", APIParT_Length, format_string ("%f", placingZone->verLen).c_str(),
				"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(),
				"torsion_ang", APIParT_Angle, format_string ("%f", 0.0).c_str()));
			moveIn3D ('x', timber.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
			
			// 결과물 전체 그룹화
			groupElements (elemList);
			elemList.Clear ();
		}

		// 무조건 가로 방향으로 이동
		moveIn3D ('x', timber.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
	}

	// ================================================== 테이블폼 배치
	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		placingZone->placeEuroformsOfTableform (this, xx);		// 테이블폼의 유로폼 배치 (타입 불문 공통)
		
		if (placingZone->tableformType == 1)		placingZone->placeTableformA (this, xx);

		// 결과물 전체 그룹화
		groupElements (elemList);
		elemList.Clear ();
	}

	return err;
}

// 테이블폼 내 유로폼 배치 (공통)
void	LowSideTableformPlacingZone::placeEuroformsOfTableform (LowSideTableformPlacingZone* placingZone, short idxCell)
{
	short	xx;

	EasyObjectPlacement euroform;

	if (placingZone->cells [idxCell].objType == TABLEFORM) {
		if (placingZone->bVertical == true) {
			// 세로방향
			euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
				if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (placingZone->verLen > EPS)) {
					elemList.Push (euroform.placeObject (5,
						"eu_stan_onoff", APIParT_Boolean, "1.0",
						"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]).c_str(),
						"eu_hei", APIParT_CString, format_string ("%d", (int)(placingZone->verLen * 1000)).c_str(),
						"u_ins", APIParT_CString, "벽세우기",
						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
				}
				moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}
		} else {
			// 가로방향
			euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
				if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (placingZone->verLen > EPS)) {
					moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					elemList.Push (euroform.placeObject (5,
						"eu_stan_onoff", APIParT_Boolean, "1.0",
						"eu_wid", APIParT_CString, format_string ("%d", (int)(placingZone->verLen * 1000)).c_str(),
						"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]).c_str(),
						"u_ins", APIParT_CString, "벽눕히기",
						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
					moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
				}
				moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}
		}
	}
}

// 테이블폼 타입A 배치 (유로폼 제외) - 각파이프 2줄
void	LowSideTableformPlacingZone::placeTableformA (LowSideTableformPlacingZone* placingZone, short idxCell)
{
	short	xx;
	int		pipeLength;
	double	sideMargin;
	
	int		firstWidth, lastWidth;
	bool	bFoundFirstWidth;
	short	realWidthCount, count;

	// 테이블폼 내 1번째와 마지막 유로폼의 각각의 너비를 가져옴 (길이가 0인 유로폼은 통과) - 세로방향
	realWidthCount = 0;
	bFoundFirstWidth = false;
	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
		if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
			if (bFoundFirstWidth == false) {
				firstWidth = placingZone->cells [idxCell].tableInHor [xx];
				bFoundFirstWidth = true;
			}
			lastWidth = placingZone->cells [idxCell].tableInHor [xx];
			++realWidthCount;
		}
	}

	if (placingZone->cells [idxCell].objType != TABLEFORM)
		return;

	if (placingZone->cells [idxCell].horLen == 0)
		return;

	if (placingZone->bVertical == true) {
		// ================================================== 세로방향
		// 수평 파이프 배치
	//	if (placingZone->cells [idxCell].horLen % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].horLen - 100;
			sideMargin = 0.050;
	//	} else {
	//		pipeLength = placingZone->cells [idxCell].horLen - 50;
	//		sideMargin = 0.025;
	//	}

		EasyObjectPlacement rectPipe;

		// 하부
		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng, 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		// 상부
		if (placingZone->bVertical == true) {
			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('z', rectPipe.radAng, placingZone->verLen - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('z', rectPipe.radAng, -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		}

		// 수직 파이프 배치
		if (placingZone->pipeVerticalLength > EPS) {
		//	if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
				//pipeLength = (int)(placingZone->verLen * 1000) - 100;
				pipeLength = (int)(placingZone->pipeVerticalLength * 1000);
				sideMargin = 0.050;
		//	} else {
		//		pipeLength = placingZone->verLen - 50;
		//		sideMargin = 0.025;
		//	}

			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			moveIn3D ('x', rectPipe.radAng, (double)firstWidth / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 왼쪽
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng, -0.035 + 0.150 - (double)firstWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('x', rectPipe.radAng, (double)(placingZone->cells [idxCell].horLen - lastWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 오른쪽
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
		}

		// 핀볼트 세트
		EasyObjectPlacement pinbolt;
		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realWidthCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 하부
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100).c_str(), "bolt_dia", APIParT_Length, format_string ("%f", 0.010).c_str(), "washer_pos", APIParT_Length, format_string ("%f", 0.050).c_str(), "washer_size", APIParT_Length, format_string ("%f", 0.100).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('z', pinbolt.radAng, placingZone->verLen - 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realWidthCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 상부
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100).c_str(), "bolt_dia", APIParT_Length, format_string ("%f", 0.010).c_str(), "washer_pos", APIParT_Length, format_string ("%f", 0.050).c_str(), "washer_size", APIParT_Length, format_string ("%f", 0.100).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		// 결합철물
		if (placingZone->pipeVerticalLength > EPS) {
			EasyObjectPlacement join;
			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			moveIn3D ('x', join.radAng, (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('z', join.radAng, 0.150, &join.posX, &join.posY, &join.posZ);

			elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150).c_str(), "bolt_dia", APIParT_Length, format_string ("%f", 0.012).c_str(), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000).c_str(), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108).c_str(), "washer_size", APIParT_Length, format_string ("%f", 0.100).c_str(), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			moveIn3D ('x', join.radAng, (double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150).c_str(), "bolt_dia", APIParT_Length, format_string ("%f", 0.012).c_str(), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000).c_str(), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108).c_str(), "washer_size", APIParT_Length, format_string ("%f", 0.100).c_str(), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));

			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			moveIn3D ('x', join.radAng, (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('z', join.radAng, placingZone->verLen - 0.300, &join.posX, &join.posY, &join.posZ);

			elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150).c_str(), "bolt_dia", APIParT_Length, format_string ("%f", 0.012).c_str(), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000).c_str(), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108).c_str(), "washer_size", APIParT_Length, format_string ("%f", 0.100).c_str(), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			moveIn3D ('x', join.radAng, (double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150).c_str(), "bolt_dia", APIParT_Length, format_string ("%f", 0.012).c_str(), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000).c_str(), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108).c_str(), "washer_size", APIParT_Length, format_string ("%f", 0.100).c_str(), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
		}

		// 헤드피스
		if (placingZone->pipeVerticalLength > EPS) {
			EasyObjectPlacement headpiece;
			headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			moveIn3D ('x', headpiece.radAng, (double)(firstWidth - 150 - 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng, -0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('z', headpiece.radAng, 0.210, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			moveIn3D ('z', headpiece.radAng, (-0.210 + placingZone->verLen - 0.240), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			moveIn3D ('z', headpiece.radAng, -(-0.210 + placingZone->verLen - 0.240), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('x', headpiece.radAng, (double)(-(firstWidth - 150 - 100) + placingZone->cells [idxCell].horLen + (-lastWidth + 150 - 100)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			moveIn3D ('z', headpiece.radAng, (-0.210 + placingZone->verLen - 0.240), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
		} else {
			EasyObjectPlacement headpiece;
			headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			moveIn3D ('x', headpiece.radAng, (double)(firstWidth - 150 - 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng, -0.1725 + 0.050, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('z', headpiece.radAng, 0.210 + 0.040 + 0.150, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
			moveIn3D ('z', headpiece.radAng, (-0.210 + placingZone->verLen - 0.240), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
			moveIn3D ('z', headpiece.radAng, -(-0.210 + placingZone->verLen - 0.240), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('x', headpiece.radAng, (double)(-(firstWidth - 150 - 100) + placingZone->cells [idxCell].horLen + (-lastWidth + 150 - 100)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
			moveIn3D ('z', headpiece.radAng, (-0.210 + placingZone->verLen - 0.240), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
		}
	} else {
	//	// ================================================== 가로방향
	//	// 수평 파이프 배치
		EasyObjectPlacement rectPipe;
		if (placingZone->pipeVerticalLength > EPS) {
		//	if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
				//pipeLength = (int)(placingZone->verLen * 1000) - 100;
				pipeLength = (int)(placingZone->pipeVerticalLength * 1000);
				sideMargin = 0.050;
		//	} else {
		//		pipeLength = placingZone->verLen - 50;
		//		sideMargin = 0.025;
		//	}
			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025 + 0.050), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('x', rectPipe.radAng, 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 왼쪽
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng, -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {												// 중간
				if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
					moveIn3D ('x', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

					moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
					elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('x', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
					elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				}
			}
			moveIn3D ('x', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInHor [realWidthCount - 1] / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 오른쪽
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		}

		// 수직 파이프 배치
	//	if (placingZone->cells [idxCell].horLen % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].horLen - 100;
			sideMargin = 0.050;
	//	} else {
	//		pipeLength = placingZone->cells [idxCell].horLen - 50;
	//		sideMargin = 0.025;
	//	}

		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', rectPipe.radAng, 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, 0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 아래쪽
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, -0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, 0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		// 가로 방향일 때 테이블폼의 높이가 600 이상일 경우에는 파이프 추가 배치
		if (placingZone->verLen > 0.600 - EPS) {
			moveIn3D ('z', rectPipe.radAng, -0.150*2 + placingZone->verLen, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('z', rectPipe.radAng, 0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 아래쪽
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('z', rectPipe.radAng, -0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0).c_str(), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('z', rectPipe.radAng, 0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		}

		// 핀볼트 세트
		EasyObjectPlacement pinbolt;
		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

				pinbolt.radAng += DegreeToRad (90.0);
				elemList.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100).c_str(), "bolt_dia", APIParT_Length, format_string ("%f", 0.010).c_str(), "washer_pos", APIParT_Length, format_string ("%f", 0.050).c_str(), "washer_size", APIParT_Length, format_string ("%f", 0.100).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
				pinbolt.radAng -= DegreeToRad (90.0);
			}
		}

		// 가로 방향일 때 테이블폼의 높이가 600 이상일 경우에는 핀볼트 세트 추가 배치
		if (placingZone->verLen > 0.600 - EPS) {
			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->verLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {
				if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
					moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

					pinbolt.radAng += DegreeToRad (90.0);
					elemList.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100).c_str(), "bolt_dia", APIParT_Length, format_string ("%f", 0.010).c_str(), "washer_pos", APIParT_Length, format_string ("%f", 0.050).c_str(), "washer_size", APIParT_Length, format_string ("%f", 0.100).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
					pinbolt.radAng -= DegreeToRad (90.0);
				}
			}
		}

		// 결합철물
		if (placingZone->pipeVerticalLength > EPS) {
			EasyObjectPlacement join;
			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			moveIn3D ('z', join.radAng, 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('x', join.radAng, 0.150, &join.posX, &join.posY, &join.posZ);

			elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150).c_str(), "bolt_dia", APIParT_Length, format_string ("%f", 0.012).c_str(), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000).c_str(), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108).c_str(), "washer_size", APIParT_Length, format_string ("%f", 0.100).c_str(), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			moveIn3D ('x', join.radAng, (double)(-150 + placingZone->cells [idxCell].horLen - 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150).c_str(), "bolt_dia", APIParT_Length, format_string ("%f", 0.012).c_str(), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000).c_str(), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108).c_str(), "washer_size", APIParT_Length, format_string ("%f", 0.100).c_str(), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));

			// 가로 방향일 때 테이블폼의 높이가 600 이상일 경우에는 결합철물 추가 배치
			if (placingZone->verLen > 0.600 - EPS) {
				join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

				moveIn3D ('z', join.radAng, -0.150 + placingZone->verLen, &join.posX, &join.posY, &join.posZ);
				moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
				moveIn3D ('x', join.radAng, 0.150, &join.posX, &join.posY, &join.posZ);

				elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150).c_str(), "bolt_dia", APIParT_Length, format_string ("%f", 0.012).c_str(), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000).c_str(), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108).c_str(), "washer_size", APIParT_Length, format_string ("%f", 0.100).c_str(), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
				moveIn3D ('x', join.radAng, (double)(-150 + placingZone->cells [idxCell].horLen - 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
				elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150).c_str(), "bolt_dia", APIParT_Length, format_string ("%f", 0.012).c_str(), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000).c_str(), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108).c_str(), "washer_size", APIParT_Length, format_string ("%f", 0.100).c_str(), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			}
		}

		// 헤드피스
		if (placingZone->pipeVerticalLength > EPS) {
			EasyObjectPlacement headpiece;
			headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			moveIn3D ('z', headpiece.radAng, 0.210, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng, -0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('x', headpiece.radAng, 0.050, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));

			moveIn3D ('x', headpiece.radAng, (double)(-50 + placingZone->cells [idxCell].horLen - 50 - 200) / 1000, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));

			// 수직 파이프 길이가 600 이상일 경우 상단 헤드피스도 설치할 것
			if (placingZone->pipeVerticalLength >= 0.600) {
				headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

				moveIn3D ('z', headpiece.radAng, placingZone->verLen - 0.090, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
				moveIn3D ('y', headpiece.radAng, -0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
				moveIn3D ('x', headpiece.radAng, 0.050, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

				elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));

				moveIn3D ('x', headpiece.radAng, (double)(-50 + placingZone->cells [idxCell].horLen - 50 - 200) / 1000, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

				elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			}
		} else {
			EasyObjectPlacement headpiece;
			headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			// 하단 헤드피스
			moveIn3D ('z', headpiece.radAng, 0.210 + 0.040, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng, -0.1725 + 0.050, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('x', headpiece.radAng, 0.050, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));

			moveIn3D ('x', headpiece.radAng, (double)(-50 + placingZone->cells [idxCell].horLen - 50 - 200) / 1000, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));

			// 가로 방향일 때 테이블폼의 높이가 600 이상일 경우에는 상단 헤드피스 배치
			if (placingZone->verLen > 0.600 - EPS) {
				headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

				moveIn3D ('z', headpiece.radAng, placingZone->verLen - 0.090 + 0.040, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
				moveIn3D ('y', headpiece.radAng, -0.1725 + 0.050, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
				moveIn3D ('x', headpiece.radAng, 0.050, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

				elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));

				moveIn3D ('x', headpiece.radAng, (double)(-50 + placingZone->cells [idxCell].horLen - 50 - 200) / 1000, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

				elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009).c_str(), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
			}
		}

		// 결과물 전체 그룹화
		groupElements (elemList);
		elemList.Clear ();
	}
}

// 테이블폼/유로폼/휠러스페이서/합판/목재 배치를 위한 다이얼로그 (테이블폼 구성, 요소 방향, 개수 및 길이)
short DGCALLBACK lowSideTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy, zz;
	char	numbuf [32];
	int		cellWidthValue, accumLength;
	const short		maxCol = 50;		// 열 최대 개수
	double			totalWidth;
	static short	dialogSizeX = 600;			// 현재 다이얼로그 크기 X
	static short	dialogSizeY = 350;			// 현재 다이얼로그 크기 Y

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, L"테이블폼/인코너/유로폼/휠러스페이서/합판/목재 구성");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 225, 300, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"확 인");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 325, 300, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"취 소");
			DGShowItem (dialogID, DG_CANCEL);

			// 라벨: 테이블폼 방향
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 20, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"테이블폼 방향");
			DGShowItem (dialogID, itmIdx);

			// 팝업컨트롤: 테이블폼 방향
			placingZone.POPUP_DIRECTION = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 105, 15, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_DIRECTION, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM, L"세로");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM, L"가로");
			if (placingZone.bVertical == true)
				DGPopUpSelectItem (dialogID, placingZone.POPUP_DIRECTION, 1);
			else
				DGPopUpSelectItem (dialogID, placingZone.POPUP_DIRECTION, 2);
			DGShowItem (dialogID, placingZone.POPUP_DIRECTION);

			// 라벨: 테이블폼 타입
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 220, 20, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"테이블폼 타입");
			DGShowItem (dialogID, itmIdx);

			// 팝업컨트롤: 테이블폼 타입
			placingZone.POPUP_TABLEFORM_TYPE = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 305, 15, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, L"타입A");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE);

			// 라벨: 테이블폼 높이
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 400, 20, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"테이블폼 높이");
			DGShowItem (dialogID, itmIdx);

			// 팝업컨트롤: 테이블폼 높이
			placingZone.POPUP_TABLEFORM_HEIGHT = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 480, 15, 80, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			if (placingZone.bVertical == true) {
				DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "1200");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "900");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "600");
			} else {
				DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "600");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "500");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "450");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "400");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "300");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "200");
			}
			DGPopUpSelectItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT);

			// 버튼: 추가
			placingZone.BUTTON_ADD_HOR = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 55, 70, 25);
			DGSetItemFont (dialogID, placingZone.BUTTON_ADD_HOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_ADD_HOR, L"추가");
			DGShowItem (dialogID, placingZone.BUTTON_ADD_HOR);

			// 버튼: 삭제
			placingZone.BUTTON_DEL_HOR = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 100, 55, 70, 25);
			DGSetItemFont (dialogID, placingZone.BUTTON_DEL_HOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_DEL_HOR, L"삭제");
			DGShowItem (dialogID, placingZone.BUTTON_DEL_HOR);

			// 라벨: 남은 너비
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 220, 60, 70, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"남은 너비");
			DGShowItem (dialogID, itmIdx);

			// Edit컨트롤: 남은 너비
			placingZone.EDITCONTROL_REMAIN_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 300, 55, 80, 25);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH);
			DGShowItem (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH);

			// 라벨: 현재 높이
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 400, 60, 70, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"현재 높이");
			DGShowItem (dialogID, itmIdx);

			// Edit컨트롤: 현재 높이
			placingZone.EDITCONTROL_CURRENT_HEIGHT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 480, 55, 80, 25);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_CURRENT_HEIGHT);
			DGShowItem (dialogID, placingZone.EDITCONTROL_CURRENT_HEIGHT);

			// 라벨: 수직 파이프 길이
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 260, 100, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"수직 파이프 길이");
			DGShowItem (dialogID, itmIdx);

			// Edit컨트롤: 수직 파이프 길이
			placingZone.EDITCONTROL_VPIPE_LENGTH = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 140, 255, 80, 25);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_VPIPE_LENGTH, 0.0);
			DGShowItem (dialogID, placingZone.EDITCONTROL_VPIPE_LENGTH);

			//////////////////////////////////////////////////////////// 셀 정보 초기화
			placingZone.initCells (&placingZone, placingZone.bVertical);

			//////////////////////////////////////////////////////////// 아이템 배치 (정면 관련 버튼)
			// 왼쪽 인코너판넬/아웃코너판넬/아웃코너앵글
			// 버튼
			placingZone.BUTTON_LCORNER = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 137, 71, 66);
			DGSetItemFont (dialogID, placingZone.BUTTON_LCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, L"없음");
			DGShowItem (dialogID, placingZone.BUTTON_LCORNER);
			DGDisableItem (dialogID, placingZone.BUTTON_LCORNER);
			// 객체 타입 (팝업컨트롤)
			placingZone.POPUP_OBJ_TYPE_LCORNER = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 20, 137 - 25, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_IS_EXTRASMALL | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM, L"없음");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM, L"인코너판넬");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM, L"아웃코너판넬");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM, L"아웃코너앵글");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER);
			// 너비 (Edit컨트롤)
			placingZone.EDITCONTROL_WIDTH_LCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 20, 137 + 68, 70, 23);
			DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);

			// 일반 셀: 기본값은 테이블폼
			itmPosX = 90;
			itmPosY = 137;
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				// 버튼
				placingZone.BUTTON_OBJ [xx] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
				DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], L"테이블폼");
				DGShowItem (dialogID, placingZone.BUTTON_OBJ [xx]);

				// 객체 타입 (팝업컨트롤)
				placingZone.POPUP_OBJ_TYPE [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_IS_EXTRASMALL | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, L"없음");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, L"테이블폼");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, L"유로폼");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, L"휠러스페이서");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, L"합판");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, L"각재");
				DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_TOP+1);
				DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

				// 너비 (팝업컨트롤)
				placingZone.POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
				if (placingZone.bVertical == true) {
					for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_tableform) / sizeof (int) ; ++yy) {
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
						_itoa (placingZone.presetWidthVertical_tableform [yy], numbuf, 10);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
					}
				} else {
					for (yy = 0 ; yy < sizeof (placingZone.presetWidthHorizontal_tableform) / sizeof (int) ; ++yy) {
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
						_itoa (placingZone.presetWidthHorizontal_tableform [yy], numbuf, 10);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
					}
				}
				DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
				DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);

				// 너비 (팝업컨트롤) - 처음에는 숨김
				placingZone.EDITCONTROL_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
				DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);

				itmPosX += 70;
			}

			// 오른쪽 인코너판넬/아웃코너판넬/아웃코너앵글
			// 버튼
			placingZone.BUTTON_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, 137, 71, 66);
			DGSetItemFont (dialogID, placingZone.BUTTON_RCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, L"없음");
			DGShowItem (dialogID, placingZone.BUTTON_RCORNER);
			DGDisableItem (dialogID, placingZone.BUTTON_RCORNER);
			// 객체 타입 (팝업컨트롤)
			placingZone.POPUP_OBJ_TYPE_RCORNER = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, 137 - 25, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_IS_EXTRASMALL | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, L"없음");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, L"인코너판넬");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, L"아웃코너판넬");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, L"아웃코너앵글");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
			// 너비 (Edit컨트롤)
			placingZone.EDITCONTROL_WIDTH_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 137 + 68, 70, 23);
			DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

			//////////////////////////////////////////////////////////// 다이얼로그 크기 변경, 남은 너비 및 높이 계산
			// 다이얼로그 크기 설정
			dialogSizeX = 600;
			dialogSizeY = 350;
			if (placingZone.nCellsInHor >= 5) {
				DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCellsInHor - 5), dialogSizeY, DG_TOPLEFT, true);
			}

			// 남은 너비 계산
			totalWidth = 0.0;
			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
					totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
				else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
					totalWidth += 0.0;
				else
					totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

			// 현재 높이 계산
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_CURRENT_HEIGHT, placingZone.verLen);

			break;

		case DG_MSG_CHANGE:
			// 가로/세로 변경할 때
			if (item == placingZone.POPUP_DIRECTION) {
				// 가로일 경우
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_DIRECTION) == HORIZONTAL) {
					// 셀 정보 초기화
					placingZone.initCells (&placingZone, false);
					placingZone.bVertical = false;

					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthHorizontal_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidthHorizontal_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetHeightHorizontal_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeightHorizontal_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					}

				// 세로일 경우
				} else {
					// 셀 정보 초기화
					placingZone.initCells (&placingZone, true);
					placingZone.bVertical = true;

					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidthVertical_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidthVertical_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					}
				}

				// 가로, 세로 변경시 테이블폼 높이 팝업컨트롤 내용 변경
				DGPopUpDeleteItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_ALL_ITEMS);
				if (placingZone.bVertical == true) {
					DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "1200");
					DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "900");
					DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "600");
				} else {
					DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "600");
					DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "500");
					DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "450");
					DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "400");
					DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "300");
					DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "200");
				}
				DGPopUpSelectItem (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DG_POPUP_TOP);
			}

			// 인코너판넬/아웃코너판넬/아웃코너앵글 변경시
			if (item == placingZone.POPUP_OBJ_TYPE_LCORNER) {
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) == NOCORNER) {
					DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, L"없음");
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) == INCORNER_PANEL) {
					DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, L"인코너판넬");
					DGEnableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.080);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.500);
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.100);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) == OUTCORNER_PANEL) {
					DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, L"아웃코너판넬");
					DGEnableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.080);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.500);
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.100);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) == OUTCORNER_ANGLE) {
					DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, L"아웃코너앵글");
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
				}
			}

			if (item == placingZone.POPUP_OBJ_TYPE_RCORNER) {
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) == NOCORNER) {
					DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, L"없음");
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) == INCORNER_PANEL) {
					DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, L"인코너판넬");
					DGEnableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.080);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.500);
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.100);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) == OUTCORNER_PANEL) {
					DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, L"아웃코너판넬");
					DGEnableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.080);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.500);
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.100);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) == OUTCORNER_ANGLE) {
					DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, L"아웃코너앵글");
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
				}
			}

			// 객체 타입 변경할 때
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if (item == placingZone.POPUP_OBJ_TYPE [xx]) {
					// 해당 버튼의 이름 변경
					DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], DGPopUpGetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx])));
					
					// 가로/세로 방향 여부에 따라 팝업컨트롤의 내용물이 바뀜
					if (placingZone.bVertical == false) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthHorizontal_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidthHorizontal_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetHeightHorizontal_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeightHorizontal_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					} else {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidthVertical_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidthVertical_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					}

					// 테이블폼 타입이 아니면 버튼을 잠금
					if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) != TABLEFORM)
						DGDisableItem (dialogID, placingZone.BUTTON_OBJ [xx]);
					else
						DGEnableItem (dialogID, placingZone.BUTTON_OBJ [xx]);

					// 테이블폼/유로폼이면 너비가 팝업컨트롤, 그 외에는 Edit컨트롤, 없으면 모두 숨김
					if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM)) {
						if (!DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);
						if (DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))		DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
					} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE) {
						if (DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))		DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
						if (DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);
					} else {
						if (!DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))	DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
						if (DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);

						// 휠러스페이서, 합판, 각재의 가로 방향 길이의 최소, 최대값
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == FILLERSP) {
							DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.010);
							DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.050);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD) {
							DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.090);
							DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 1.220);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TIMBER) {
							DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.005);
							DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 1.000);
						}
					}
				}
			}

			// 테이블폼 너비를 변경할 때
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if (item == placingZone.POPUP_WIDTH [xx]) {
					if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
						cellWidthValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], (DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx]))).ToCStr ().Get ());
						if (placingZone.bVertical == true) {
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_tableform) / sizeof (int) ; ++yy) {
								if (cellWidthValue == placingZone.presetWidthVertical_tableform [yy]) {
									for (zz = 0 ; zz < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++zz) {
										if ((zz >= 0) && (zz < placingZone.presetWidth_config_vertical [yy][0]))
											placingZone.cells [xx].tableInHor [zz] = placingZone.presetWidth_config_vertical [yy][zz+1];
										else
											placingZone.cells [xx].tableInHor [zz] = 0;
									}
								}
							}
						} else {
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthHorizontal_tableform) / sizeof (int) ; ++yy) {
								if (cellWidthValue == placingZone.presetWidthHorizontal_tableform [yy]) {
									for (zz = 0 ; zz < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++zz) {
										if ((zz >= 0) && (zz < placingZone.presetWidth_config_horizontal [yy][0]))
											placingZone.cells [xx].tableInHor [zz] = placingZone.presetWidth_config_horizontal [yy][zz+1];
										else
											placingZone.cells [xx].tableInHor [zz] = 0;
									}
								}
							}
						}
						placingZone.cells [xx].horLen = cellWidthValue;
					}
				}
			}

			// 남은 너비 계산
			totalWidth = 0.0;
			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
					totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
				else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
					totalWidth += 0.0;
				else
					totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

			break;

		case DG_MSG_CLICK:
			// 확인 버튼
			if (item == DG_OK) {
				// 테이블폼 방향
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_DIRECTION) == VERTICAL)
					placingZone.bVertical = true;
				else
					placingZone.bVertical = false;

				// 테이블폼 타입
				placingZone.tableformType = DGPopUpGetSelected (dialogID, placingZone.POPUP_TABLEFORM_TYPE);

				// 인코너판넬/아웃코너판넬/아웃코너앵글 유무 및 길이
				placingZone.typeLcorner = DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER);
				placingZone.lenLcorner = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
				placingZone.typeRcorner = DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
				placingZone.lenRcorner = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

				// 셀 정보 업데이트
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					// 객체 타입
					placingZone.cells [xx].objType = DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

					// 가로 방향 길이 지정
					if (placingZone.cells [xx].objType == NONE) {
						// 가로 길이 0
						placingZone.cells [xx].horLen = 0;

						// 테이블 내 가로 길이 모두 0
						for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
							placingZone.cells [xx].tableInHor [yy] = 0;

					} else if (placingZone.cells [xx].objType == EUROFORM) {
						// 유로폼 너비
						placingZone.cells [xx].horLen = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ());

						// 테이블 내 가로 길이 모두 0
						for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
							placingZone.cells [xx].tableInHor [yy] = 0;

					} else if ((placingZone.cells [xx].objType == FILLERSP) || (placingZone.cells [xx].objType == PLYWOOD) || (placingZone.cells [xx].objType == TIMBER)) {
						// 휠러스페이서, 합판, 목재의 너비
						placingZone.cells [xx].horLen = (int)(DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]) * 1000);

						// 테이블 내 가로 길이 모두 0
						for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
							placingZone.cells [xx].tableInHor [yy] = 0;
					}
				}

				// 유로폼 높이 지정
				placingZone.verLen = atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT, DGPopUpGetSelected (dialogID, placingZone.POPUP_TABLEFORM_HEIGHT)).ToCStr ().Get ()) / 1000;

				// 수직 파이프 길이
				placingZone.pipeVerticalLength = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_VPIPE_LENGTH);

				// 레이어 설정
				bLayerInd_Euroform = true;			// 유로폼 항상 On
				bLayerInd_RectPipe = false;
				bLayerInd_PinBolt = false;
				bLayerInd_WallTie = false;
				bLayerInd_HeadPiece = false;
				bLayerInd_Props = false;
				bLayerInd_Join = false;

				bLayerInd_Plywood = false;
				bLayerInd_Timber = false;
				bLayerInd_IncornerPanel = false;
				bLayerInd_OutcornerAngle = false;
				bLayerInd_OutcornerPanel = false;
				bLayerInd_RectpipeHanger = false;
				bLayerInd_EuroformHook = false;
				bLayerInd_CrossJointBar = false;
				bLayerInd_Fillersp = false;
				bLayerInd_BlueClamp = false;
				bLayerInd_BlueTimberRail = false;

				// 휠러스페이서, 합판, 각재
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					if (placingZone.cells [xx].objType == FILLERSP)	bLayerInd_Fillersp = true;
					if (placingZone.cells [xx].objType == PLYWOOD)	bLayerInd_Plywood = true;
					if (placingZone.cells [xx].objType == TIMBER)	bLayerInd_Timber = true;
				}

				// 인코너판넬
				if ((placingZone.typeLcorner == INCORNER_PANEL) || (placingZone.typeRcorner == INCORNER_PANEL))
					bLayerInd_IncornerPanel = true;

				// 아웃코너판넬
				if ((placingZone.typeLcorner == OUTCORNER_PANEL) || (placingZone.typeRcorner == OUTCORNER_PANEL))
					bLayerInd_OutcornerPanel = true;

				// 아웃코너앵글
				if ((placingZone.typeLcorner == OUTCORNER_ANGLE) || (placingZone.typeRcorner == OUTCORNER_ANGLE))
					bLayerInd_OutcornerAngle = true;

				if (placingZone.tableformType == 1) {
					bLayerInd_Euroform = true;
					bLayerInd_RectPipe = true;
					bLayerInd_PinBolt = true;
					bLayerInd_HeadPiece = true;
					bLayerInd_Props = true;
					bLayerInd_Join = true;
				}

				placingZone.adjustCellsPosition (&placingZone);		// 셀의 위치 교정

			} else if (item == DG_CANCEL) {
				// 아무 작업도 하지 않음
			} else {
				if ((item == placingZone.BUTTON_ADD_HOR) || (item == placingZone.BUTTON_DEL_HOR)) {
					// 추가 버튼 클릭
					if (item == placingZone.BUTTON_ADD_HOR) {
						if (placingZone.nCellsInHor < maxCol) {
							// 우측 인코너 버튼을 지우고
							DGRemoveDialogItem (dialogID, placingZone.BUTTON_RCORNER);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
							DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

							// 마지막 셀 버튼 오른쪽에 새로운 셀 버튼을 추가하고
							itmPosX = 90 + (70 * placingZone.nCellsInHor);
							itmPosY = 137;
							// 버튼
							placingZone.BUTTON_OBJ [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
							DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor], DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor], L"테이블폼");
							DGShowItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor]);

							// 객체 타입 (팝업컨트롤)
							placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_IS_EXTRASMALL | DG_IS_PLAIN);
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, L"없음");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, L"테이블폼");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, L"유로폼");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, L"휠러스페이서");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, L"합판");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, L"각재");
							DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_TOP+1);
							DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor]);

							// 너비 (팝업컨트롤)
							placingZone.POPUP_WIDTH [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_IS_LARGE | DG_IS_PLAIN);
							if (placingZone.bVertical == true) {
								for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_tableform) / sizeof (int) ; ++yy) {
									DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
									_itoa (placingZone.presetWidthVertical_tableform [yy], numbuf, 10);
									DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM, numbuf);
								}
							} else {
								for (yy = 0 ; yy < sizeof (placingZone.presetWidthHorizontal_tableform) / sizeof (int) ; ++yy) {
									DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
									_itoa (placingZone.presetWidthHorizontal_tableform [yy], numbuf, 10);
									DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM, numbuf);
								}
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_TOP);
							DGShowItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor]);

							// 너비 (팝업컨트롤) - 처음에는 숨김
							placingZone.EDITCONTROL_WIDTH [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);

							itmPosX += 70;

							// 우측 인코너 버튼을 오른쪽 끝에 붙임
							// 버튼
							placingZone.BUTTON_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, 137, 71, 66);
							DGSetItemFont (dialogID, placingZone.BUTTON_RCORNER, DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, L"없음");
							DGShowItem (dialogID, placingZone.BUTTON_RCORNER);
							DGDisableItem (dialogID, placingZone.BUTTON_RCORNER);
							// 객체 타입 (팝업컨트롤)
							placingZone.POPUP_OBJ_TYPE_RCORNER = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, 137 - 25, 70, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_IS_EXTRASMALL | DG_IS_PLAIN);
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, L"없음");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, L"인코너판넬");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, L"아웃코너판넬");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, L"아웃코너앵글");
							DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_TOP);
							DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
							// 너비 (Edit컨트롤)
							placingZone.EDITCONTROL_WIDTH_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 137 + 68, 70, 23);
							DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
							DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

							++placingZone.nCellsInHor;
						}
					}

					// 삭제 버튼 클릭
					else if (item == placingZone.BUTTON_DEL_HOR) {
						if (placingZone.nCellsInHor > 1) {
							// 우측 인코너 버튼을 지우고
							DGRemoveDialogItem (dialogID, placingZone.BUTTON_RCORNER);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
							DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

							// 마지막 셀 버튼을 지우고
							DGRemoveDialogItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH [placingZone.nCellsInHor - 1]);

							// 우측 인코너 버튼을 오른쪽 끝에 붙임
							itmPosX = 90 + (70 * (placingZone.nCellsInHor - 1));
							itmPosY = 137;
							// 버튼
							placingZone.BUTTON_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, 137, 71, 66);
							DGSetItemFont (dialogID, placingZone.BUTTON_RCORNER, DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, L"없음");
							DGShowItem (dialogID, placingZone.BUTTON_RCORNER);
							DGDisableItem (dialogID, placingZone.BUTTON_RCORNER);
							// 객체 타입 (팝업컨트롤)
							placingZone.POPUP_OBJ_TYPE_RCORNER = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, 137 - 25, 70, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_IS_EXTRASMALL | DG_IS_PLAIN);
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, L"없음");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, L"인코너판넬");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, L"아웃코너판넬");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, L"아웃코너앵글");
							DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_TOP);
							DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
							// 너비 (Edit컨트롤)
							placingZone.EDITCONTROL_WIDTH_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 137 + 68, 70, 23);
							DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
							DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

							--placingZone.nCellsInHor;
						}
					}

				} else {
					// 객체 버튼 클릭 (테이블폼인 경우에만 유효함)
					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if ((item == placingZone.BUTTON_OBJ [xx]) && (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM)) {
							clickedIndex = xx;
							if (placingZone.bVertical == true) {
								// 테이블폼 타입 (세로 방향)일 경우, 3번째 다이얼로그(세로방향) 열기
								result = DGBlankModalDialog (770, 180, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, lowSideTableformPlacerHandler3_Vertical, (short) 0);
							} else {
								// 테이블폼 타입 (가로 방향)일 경우, 3번째 다이얼로그(가로방향) 열기
								result = DGBlankModalDialog (770, 180, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, lowSideTableformPlacerHandler3_Horizontal, (short) 0);
							}

							// 콤보박스의 전체 너비 값 변경
							accumLength = 0;

							for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
								accumLength += placingZone.cells [xx].tableInHor [yy];

							bool bFoundWidth = false;

							for (yy = 1 ; yy <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_WIDTH [xx]) ; ++yy) {
								if (accumLength == atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], yy).ToCStr ().Get ())) {
									DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], yy);
									bFoundWidth = true;
									break;
								}
							}

							if (bFoundWidth == false) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (accumLength, numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
								DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
							}
						}
					}
				}

				// 다이얼로그 크기 설정
				dialogSizeX = 600;
				dialogSizeY = 350;
				if (placingZone.nCellsInHor >= 5) {
					DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCellsInHor - 5), dialogSizeY, DG_TOPLEFT, true);
				}

				// 남은 너비 계산
				totalWidth = 0.0;
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
						totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
					else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
						totalWidth += 0.0;
					else
						totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
				}
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

				item = 0;
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

// 객체의 레이어를 선택하기 위한 다이얼로그
short DGCALLBACK lowSideTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	xx;
	short	result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, L"가설재 레이어 선택하기");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGSetItemText (dialogID, DG_OK, L"확 인");

			// 종료 버튼
			DGSetItemText (dialogID, DG_CANCEL, L"취 소");

			// 체크박스: 레이어 묶음
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, L"레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			// 레이어 관련 라벨
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, L"유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, L"비계 파이프");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, L"핀볼트 세트");
			DGSetItemText (dialogID, LABEL_LAYER_JOIN, L"결합철물");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, L"헤드피스");
			DGSetItemText (dialogID, LABEL_LAYER_PROPS, L"푸시풀프롭스");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, L"합판");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER, L"각재");
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSP, L"휠러스페이서");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, L"아웃코너앵글");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_PANEL, L"아웃코너판넬");
			DGSetItemText (dialogID, LABEL_LAYER_INCORNER_PANEL, L"인코너판넬");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER, L"각파이프행거");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK, L"유로폼 후크");
			DGSetItemText (dialogID, LABEL_LAYER_CROSS_JOINT_BAR, L"십자조인트바");
			DGSetItemText (dialogID, LABEL_LAYER_BLUE_CLAMP, L"블루클램프");
			DGSetItemText (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL, L"블루목심");

			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 610, 160, 25);
			DGSetItemFont (dialogID, BUTTON_AUTOSET, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_AUTOSET, L"레이어 자동 설정");
			DGShowItem (dialogID, BUTTON_AUTOSET);

			// 불필요한 항목 숨김
			DGHideItem(dialogID, LABEL_LAYER_18);
			DGHideItem(dialogID, LABEL_LAYER_19);
			DGHideItem(dialogID, LABEL_LAYER_20);
			DGHideItem(dialogID, USERCONTROL_LAYER_18);
			DGHideItem(dialogID, USERCONTROL_LAYER_19);
			DGHideItem(dialogID, USERCONTROL_LAYER_20);

			// 다이얼로그 크기 조절
			DGSetDialogSize(dialogID, DG_CLIENT, 300, 660, DG_TOPLEFT, false);

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);
			if (bLayerInd_Euroform == true) {
				DGEnableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			}

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, 1);
			if (bLayerInd_RectPipe == true) {
				DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);
			if (bLayerInd_PinBolt == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			}

			ucb.itemID	 = USERCONTROL_LAYER_JOIN;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, 1);
			if (bLayerInd_Join == true) {
				DGEnableItem (dialogID, LABEL_LAYER_JOIN);
				DGEnableItem (dialogID, USERCONTROL_LAYER_JOIN);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_JOIN);
				DGDisableItem (dialogID, USERCONTROL_LAYER_JOIN);
			}

			ucb.itemID	 = USERCONTROL_LAYER_HEADPIECE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, 1);
			if (bLayerInd_HeadPiece == true) {
				DGEnableItem (dialogID, LABEL_LAYER_HEADPIECE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_HEADPIECE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_HEADPIECE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_HEADPIECE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PROPS;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROPS, 1);
			if (bLayerInd_Props == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PROPS);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PROPS);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PROPS);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PROPS);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);
			if (bLayerInd_Plywood == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
			}

			ucb.itemID	 = USERCONTROL_LAYER_TIMBER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, 1);
			if (bLayerInd_Timber == true) {
				DGEnableItem (dialogID, LABEL_LAYER_TIMBER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_TIMBER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			}

			ucb.itemID	 = USERCONTROL_LAYER_FILLERSP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, 1);
			if (bLayerInd_Fillersp == true) {
				DGEnableItem (dialogID, LABEL_LAYER_FILLERSP);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSP);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FILLERSP);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSP);
			}

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);
			if (bLayerInd_OutcornerAngle == true) {
				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_PANEL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, 1);
			if (bLayerInd_OutcornerPanel == true) {
				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
			}

			ucb.itemID	 = USERCONTROL_LAYER_INCORNER_PANEL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, 1);
			if (bLayerInd_IncornerPanel == true) {
				DGEnableItem (dialogID, LABEL_LAYER_INCORNER_PANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_INCORNER_PANEL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
			}

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE_HANGER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, 1);
			if (bLayerInd_RectpipeHanger == true) {
				DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
			}

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM_HOOK;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, 1);
			if (bLayerInd_EuroformHook == true) {
				DGEnableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
				DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
			}

			ucb.itemID	 = USERCONTROL_LAYER_CROSS_JOINT_BAR;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR, 1);
			if (bLayerInd_CrossJointBar == true) {
				DGEnableItem (dialogID, LABEL_LAYER_CROSS_JOINT_BAR);
				DGEnableItem (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_CROSS_JOINT_BAR);
				DGDisableItem (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);
			}

			ucb.itemID	 = USERCONTROL_LAYER_BLUE_CLAMP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, 1);
			if (bLayerInd_BlueClamp == true) {
				DGEnableItem (dialogID, LABEL_LAYER_BLUE_CLAMP);
				DGEnableItem (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_BLUE_CLAMP);
				DGDisableItem (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
			}

			ucb.itemID	 = USERCONTROL_LAYER_BLUE_TIMBER_RAIL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, 1);
			if (bLayerInd_BlueTimberRail == true) {
				DGEnableItem (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);
			}

			break;

		case DG_MSG_CHANGE:
			// 레이어 같이 바뀜
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
				long selectedLayer;

				selectedLayer = DGGetItemValLong (dialogID, item);

				for (xx = USERCONTROL_LAYER_EUROFORM; xx <= USERCONTROL_LAYER_BLUE_TIMBER_RAIL ; ++xx)
					DGSetItemValLong (dialogID, xx, selectedLayer);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 레이어 번호 저장
					if (bLayerInd_Euroform == true)			layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					if (bLayerInd_RectPipe == true)			layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					if (bLayerInd_PinBolt == true)			layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					if (bLayerInd_Join == true)				layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
					if (bLayerInd_HeadPiece == true)		layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
					if (bLayerInd_Props == true)			layerInd_Props			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROPS);
					if (bLayerInd_Plywood == true)			layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					if (bLayerInd_Timber == true)			layerInd_Timber			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER);
					if (bLayerInd_Fillersp == true)			layerInd_Fillersp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP);
					if (bLayerInd_OutcornerAngle == true)	layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
					if (bLayerInd_OutcornerPanel == true)	layerInd_OutcornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
					if (bLayerInd_IncornerPanel == true)	layerInd_IncornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
					if (bLayerInd_RectpipeHanger == true)	layerInd_RectpipeHanger	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
					if (bLayerInd_EuroformHook == true)		layerInd_EuroformHook	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
					if (bLayerInd_CrossJointBar == true)	layerInd_CrossJointBar	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);
					if (bLayerInd_BlueClamp == true)		layerInd_BlueClamp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
					if (bLayerInd_BlueTimberRail == true)	layerInd_BlueTimberRail	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);

					break;

				case BUTTON_AUTOSET:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

					if (placingZone.tableformType == 1) {
						layerInd_Euroform		= makeTemporaryLayer (structuralObject_forTableformLowSide, "UFOM", NULL);
						layerInd_RectPipe		= makeTemporaryLayer (structuralObject_forTableformLowSide, "SPIP", NULL);
						layerInd_PinBolt		= makeTemporaryLayer (structuralObject_forTableformLowSide, "PINB", NULL);
						layerInd_Join			= makeTemporaryLayer (structuralObject_forTableformLowSide, "CLAM", NULL);
						layerInd_HeadPiece		= makeTemporaryLayer (structuralObject_forTableformLowSide, "HEAD", NULL);
						layerInd_Props			= makeTemporaryLayer (structuralObject_forTableformLowSide, "PUSH", NULL);
						layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformLowSide, "PLYW", NULL);
						layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformLowSide, "TIMB", NULL);
						layerInd_BlueClamp		= makeTemporaryLayer (structuralObject_forTableformLowSide, "UFCL", NULL);
						layerInd_BlueTimberRail	= makeTemporaryLayer (structuralObject_forTableformLowSide, "RAIL", NULL);

						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_RectPipe);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_PinBolt);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, layerInd_Join);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_HeadPiece);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);
					}

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

// 테이블폼 세로방향에 대하여 유로폼의 수평 배열을 변경하기 위한 다이얼로그
short DGCALLBACK lowSideTableformPlacerHandler3_Vertical (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	// clickedIndex: 이전 다이얼로그에서 눌린 버튼의 0-기반 인덱스 번호 (BUTTON_OBJ [xx])

	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy;
	int		accumLength;
	wchar_t	buffer [256];
	char	numbuf [32];

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, L"테이블폼 (세로방향) 배열 설정");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 140, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"확 인");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 390, 140, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"취 소");
			DGShowItem (dialogID, DG_CANCEL);

			// 기존 너비 (라벨)
			accumLength = 0;
			for (xx = 0 ; xx < sizeof (placingZone.cells [clickedIndex].tableInHor) / sizeof (int) ; ++xx)
				accumLength += placingZone.cells [clickedIndex].tableInHor [xx];
			swprintf (buffer, L"기존 너비: %d", accumLength);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 270, 20, 100, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, buffer);
			DGShowItem (dialogID, itmIdx);

			// 변경된 너비 (라벨)
			swprintf (buffer, L"변경된 너비: %d", 0);
			placingZone.LABEL_TOTAL_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 400, 20, 100, 23);
			DGSetItemFont (dialogID, placingZone.LABEL_TOTAL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);
			DGShowItem (dialogID, placingZone.LABEL_TOTAL_WIDTH);

			itmPosX = 35;
			itmPosY = 55;

			for (xx = 0 ; xx < 10 ; ++xx) {
				// 구분자
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 70);
				DGShowItem (dialogID, itmIdx);

				// 텍스트(유로폼)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 10, itmPosY + 10, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"유로폼");
				DGShowItem (dialogID, itmIdx);

				// 콤보박스
				placingZone.POPUP_WIDTH_IN_TABLE [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 5, itmPosY + 40, 60, 25);
				DGSetItemFont (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_IS_LARGE | DG_IS_PLAIN);
				for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_euroform) / sizeof (int) ; ++yy) {
					DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM);
					_itoa (placingZone.presetWidthVertical_euroform [yy], numbuf, 10);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM, numbuf);
				}
				for (yy = 1 ; yy <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]) ; ++yy) {
					if (placingZone.cells [clickedIndex].tableInHor [xx] == atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy).ToCStr ().Get ())) {
						DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy);
						break;
					}
				}
				DGShowItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]);

				itmPosX += 70;
			}

			// 변경된 너비 (라벨) 업데이트
			accumLength = 0;
			for (xx = 0 ; xx < 10 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			swprintf (buffer, L"변경된 너비: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CHANGE:

			// 변경된 너비 (라벨) 업데이트
			accumLength = 0;
			for (xx = 0 ; xx < 10 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			swprintf (buffer, L"변경된 너비: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 선택한 콤보박스들의 값을 기반으로 구조체 값을 갱신함
					for (xx = 0 ; xx < 10 ; ++xx) {
						placingZone.cells [clickedIndex].tableInHor [xx] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}

					accumLength = 0;
					for (xx = 0 ; xx < 10 ; ++xx) {
						accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}
					placingZone.cells [clickedIndex].horLen = accumLength;
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

// 테이블폼 가로방향에 대하여 유로폼의 수평 배열을 변경하기 위한 다이얼로그
short DGCALLBACK lowSideTableformPlacerHandler3_Horizontal (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	// clickedIndex: 이전 다이얼로그에서 눌린 버튼의 0-기반 인덱스 번호 (BUTTON_OBJ [xx])

	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy;
	int		accumLength;
	wchar_t	buffer [256];
	char	numbuf [32];

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, L"테이블폼 (가로방향) 배열 설정");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 140, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"확 인");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 390, 140, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"취 소");
			DGShowItem (dialogID, DG_CANCEL);

			// 기존 너비 (라벨)
			accumLength = 0;
			for (xx = 0 ; xx < sizeof (placingZone.cells [clickedIndex].tableInHor) / sizeof (int) ; ++xx)
				accumLength += placingZone.cells [clickedIndex].tableInHor [xx];
			swprintf (buffer, L"기존 너비: %d", accumLength);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 270, 20, 100, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, buffer);
			DGShowItem (dialogID, itmIdx);
			
			// 변경된 너비 (라벨)
			swprintf (buffer, L"변경된 너비: %d", 0);
			placingZone.LABEL_TOTAL_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 400, 20, 100, 23);
			DGSetItemFont (dialogID, placingZone.LABEL_TOTAL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);
			DGShowItem (dialogID, placingZone.LABEL_TOTAL_WIDTH);

			itmPosX = 35;
			itmPosY = 55;

			for (xx = 0 ; xx < 10 ; ++xx) {
				// 구분자
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 70);
				DGShowItem (dialogID, itmIdx);

				// 텍스트(유로폼)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 10, itmPosY + 10, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"유로폼");
				DGShowItem (dialogID, itmIdx);

				// 콤보박스
				placingZone.POPUP_WIDTH_IN_TABLE [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 5, itmPosY + 40, 60, 25);
				DGSetItemFont (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_IS_LARGE | DG_IS_PLAIN);
				for (yy = 0 ; yy < sizeof (placingZone.presetHeightHorizontal_euroform) / sizeof (int) ; ++yy) {
					DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM);
					_itoa (placingZone.presetHeightHorizontal_euroform [yy], numbuf, 10);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM, numbuf);
				}
				for (yy = 1 ; yy <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]) ; ++yy) {
					if (placingZone.cells [clickedIndex].tableInHor [xx] == atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy).ToCStr ().Get ())) {
						DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy);
						break;
					}
				}
				DGShowItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]);

				itmPosX += 70;
			}

			// 변경된 너비 (라벨) 업데이트
			accumLength = 0;
			for (xx = 0 ; xx < 10 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			swprintf (buffer, L"변경된 너비: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CHANGE:

			// 변경된 너비 (라벨) 업데이트
			accumLength = 0;
			for (xx = 0 ; xx < 10 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			swprintf (buffer, L"변경된 너비: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 선택한 콤보박스들의 값을 기반으로 구조체 값을 갱신함
					for (xx = 0 ; xx < 10 ; ++xx) {
						placingZone.cells [clickedIndex].tableInHor [xx] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}

					accumLength = 0;
					for (xx = 0 ; xx < 10 ; ++xx) {
						accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}
					placingZone.cells [clickedIndex].horLen = accumLength;
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
