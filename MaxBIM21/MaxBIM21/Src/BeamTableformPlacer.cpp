#include <cstdio>
#include <cstdlib>
#include "MaxBIM21.h"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.h"
#include "BeamTableformPlacer.h"

using namespace beamTableformPlacerDG;

static BeamTableformPlacingZone		placingZone;			// �⺻ �� ���� ����
static InfoBeam			infoBeam;							// �� ��ü ����
static insulElemForBeamTableform	insulElem;				// �ܿ��� ����
API_Guid				structuralObject_forTableformBeam;	// ���� ��ü�� GUID

static short	layerInd_Euroform;			// ���̾� ��ȣ: ������
static short	layerInd_Plywood;			// ���̾� ��ȣ: ����
static short	layerInd_Timber;			// ���̾� ��ȣ: ����
static short	layerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static short	layerInd_Fillerspacer;		// ���̾� ��ȣ: �ٷ������̼�
static short	layerInd_Rectpipe;			// ���̾� ��ȣ: ���������
static short	layerInd_RectpipeHanger;	// ���̾� ��ȣ: �����������
static short	layerInd_Pinbolt;			// ���̾� ��ȣ: �ɺ�Ʈ
static short	layerInd_EuroformHook;		// ���̾� ��ȣ: ������ ��ũ
static short	layerInd_BlueClamp;			// ���̾� ��ȣ: ����Ŭ����
static short	layerInd_BlueTimberRail;	// ���̾� ��ȣ: ������

static short	layerInd_VerticalPost;		// ���̾� ��ȣ: PERI���ٸ� ������
static short	layerInd_HorizontalPost;	// ���̾� ��ȣ: PERI���ٸ� ������
static short	layerInd_Girder;			// ���̾� ��ȣ: GT24 �Ŵ�
static short	layerInd_BeamBracket;		// ���̾� ��ȣ: �� �����
static short	layerInd_Yoke;				// ���̾� ��ȣ: �� �ۿ���
static short	layerInd_JackSupport;		// ���̾� ��ȣ: �� ����Ʈ

static short	clickedBtnItemIdx;			// �׸��� ��ư���� Ŭ���� ��ư�� �ε��� ��ȣ�� ����
static bool		clickedOKButton;			// OK ��ư�� �������ϱ�?
static bool		clickedPrevButton;			// ���� ��ư�� �������ϱ�?

static GS::Array<API_Guid>	elemList_LeftEnd;			// �׷�ȭ (���� ��)
static GS::Array<API_Guid>	elemList_RightEnd;			// �׷�ȭ (���� ��)
static GS::Array<API_Guid>	elemList_Plywood [10];		// �׷�ȭ (���� �� �� �� ����Ʈ)
static GS::Array<API_Guid>	elemList_Tableform [10];	// �׷�ȭ (���̺���)
static GS::Array<API_Guid>	elemList_SupportingPost;	// �׷�ȭ (���ٸ�/�ۿ���)
static GS::Array<API_Guid>	elemList_Insulation;		// �׷�ȭ (�ܿ���)


// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ
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

	// ��ü ���� ��������
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// ���� 3D ������� ��������
	GS::Array<API_Coord3D>	coords;
	long					nNodes;

	// ���� ��ü ����
	InfoMorphForBeamTableform	infoMorph [2];
	API_Coord3D					morph_p1 [2][2];	// ���� 2���� ����/�Ʒ��� ������
	API_Coord3D					morph_p2 [2][2];	// ���� 2���� ����/�Ʒ��� ����
	double						morph_height [2];	// ���� 2���� ����

	// ����Ʈ �ӽ� ����
	API_Coord3D	pT;

	// �۾� �� ����
	double					workLevel_beam;


	// ������ ��� �������� (�� 1��, ���� 1~2�� �����ؾ� ��)
	err = getGuidsOfSelection (&morphs, API_MorphID, &nMorphs);
	err = getGuidsOfSelection (&beams, API_BeamID, &nBeams);
	if (err == APIERR_NOPLAN) {
		DGAlert (DG_ERROR, L"����", L"���� ������Ʈ â�� �����ϴ�.", "", L"Ȯ��", "", "");
	}
	if (err == APIERR_NOSEL) {
		DGAlert (DG_ERROR, L"����", L"�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: �� (1��), �� ����(��ü/�Ϻ�)�� ���� ���� (1��)\n�ɼ� ���� (1): �� �ݴ��� ������ ���� ���� (1��)", "", L"Ȯ��", "", "");
	}

	// ���� 1���ΰ�?
	if (nBeams != 1) {
		DGAlert (DG_ERROR, L"����", L"���� 1�� �����ؾ� �մϴ�.", "", L"Ȯ��", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1~2���ΰ�?
	if ( !((nMorphs >= 1) && (nMorphs <= 2)) ) {
		DGAlert (DG_ERROR, L"����", L"�� ����(��ü/�Ϻ�)�� ���� ������ 1�� �����ϼž� �մϴ�.\n���� ���̰� ���Ī�̸� �� �ݴ��� ������ ���� ������ �־�� �մϴ�.", "", L"Ȯ��", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// �� ���� ����
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
		// ���� ������ ������
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = morphs.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

		// ������ ���� ����
		infoMorph [xx].guid		= elem.header.guid;
		infoMorph [xx].floorInd	= elem.header.floorInd;
		infoMorph [xx].level	= info3D.bounds.zMin;

		nNodes = getVerticesOfMorph (elem.header.guid, &coords);

		// ���� ������ ������/������ ���̸� �˾Ƴ�
		short ind = 0;
		morph_p1 [xx][0] = coords [0];		// 1��° ���� ������ ����

		for (yy = 1 ; yy < nNodes ; ++yy) {
			// x, y ��ǥ ���� ������ p1�� �����ϰ�, �ٸ��� p2�� ����
			if ( (abs (morph_p1 [xx][0].x - coords [yy].x) < EPS) && (abs (morph_p1 [xx][0].y - coords [yy].y) < EPS) )
				morph_p1 [xx][1] = coords [yy];
			else {
				if (ind < 2) {
					morph_p2 [xx][ind] = coords [yy];
					ind ++;
				}
			}
		}

		// �� ���� ����
		// morph_p1, morph_p2�� [xx][0]: ���� ��
		// morph_p1, morph_p2�� [xx][1]: ���� ��
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

		// ������ ���ϴ�/���� �� ��������
		if (abs (elem.morph.tranmat.tmx [11] - info3D.bounds.zMin) < EPS) {
			// ���ϴ� ��ǥ ����
			infoMorph [xx].leftBottomX = elem.morph.tranmat.tmx [3];
			infoMorph [xx].leftBottomY = elem.morph.tranmat.tmx [7];
			infoMorph [xx].leftBottomZ = elem.morph.tranmat.tmx [11];

			// ���� ��ǥ��?
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
			// ���� ��ǥ ����
			infoMorph [xx].rightTopX = elem.morph.tranmat.tmx [3];
			infoMorph [xx].rightTopY = elem.morph.tranmat.tmx [7];
			infoMorph [xx].rightTopZ = elem.morph.tranmat.tmx [11];

			// ���ϴ� ��ǥ��?
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

		// ������ Z�� ȸ�� ���� (���� ��ġ ����)
		dx = infoMorph [xx].rightTopX - infoMorph [xx].leftBottomX;
		dy = infoMorph [xx].rightTopY - infoMorph [xx].leftBottomY;
		infoMorph [xx].ang = RadToDegree (atan2 (dy, dx));

		// ������ ���ϴ� ���� ����� ���� ������, �� ���� �������� ������
		if ( GetDistance (infoMorph [xx].leftBottomX, infoMorph [xx].leftBottomY, infoMorph [xx].leftBottomZ, morph_p1 [xx][0].x, morph_p1 [xx][0].y, morph_p1 [xx][0].z)
		   > GetDistance (infoMorph [xx].leftBottomX, infoMorph [xx].leftBottomY, infoMorph [xx].leftBottomZ, morph_p2 [xx][0].x, morph_p2 [xx][0].y, morph_p2 [xx][0].z)) {

			   for (yy = 0 ; yy < 2 ; ++yy) {
				   pT = morph_p1 [xx][yy];
				   morph_p1 [xx][yy] = morph_p2 [xx][yy];
				   morph_p2 [xx][yy] = pT;
			   }
		}

		// ����� ��ǥ�� ������
		coords.Clear ();

		// ���� ���� ����
		GS::Array<API_Element>	elems;
		elems.Push (elem);
		deleteElements (elems);
	}

	// �Ʒ��� ���ϴ� ��ǥ�� ������, �Ʒ��� ������ �������� ����
	placingZone.begC = morph_p1 [0][0];
	placingZone.endC = morph_p2 [0][0];

	// ���� ȸ�� ������ ������
	placingZone.ang = DegreeToRad (infoMorph [0].ang);

	// ���� ���̸� ������
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

	// ��ġ ���� ������, ���� (���� ������ ���̰� �������� ������ ��)
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

	// �� ����
	API_Coord3D	p1 = (morph_p1 [0][0].z < morph_p1 [0][1].z) ? morph_p1 [0][0] : morph_p1 [0][1];
	API_Coord3D	p2 = (morph_p2 [0][0].z < morph_p2 [0][1].z) ? morph_p2 [0][0] : morph_p2 [0][1];
	placingZone.beamLength = GetDistance (p1, p2);

	// �� �ʺ�
	placingZone.areaWidth_Bottom = infoBeam.width;

	// �� ������
	placingZone.offset = infoBeam.offset;

	// �� ���� ����
	placingZone.level = infoBeam.level;

	// ��� ���� ����
	placingZone.slantAngle = infoBeam.slantAngle;
	if (morph_p1 [0][0].z > morph_p2 [0][0].z)
		placingZone.slantAngle = -placingZone.slantAngle;

	// ���� ���� ����
	placingZone.areaHeight_Left *= cos (placingZone.slantAngle);
	placingZone.areaHeight_Right *= cos (placingZone.slantAngle);

	// �۾� �� ���� �ݿ�
	workLevel_beam = getWorkLevel (infoBeam.floorInd);

	placingZone.begC.z -= workLevel_beam;
	placingZone.endC.z -= workLevel_beam;

FIRST:

	// placingZone�� Cell ���� �ʱ�ȭ
	placingZone.initCells (&placingZone);

	// [DIALOG] 1��° ���̾�α׿��� ������ ���� �Է� ����
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32506, ACAPI_GetOwnResModule (), beamTableformPlacerHandler1, 0);

	if (result == DG_CANCEL)
		return err;

	// �� ���̿��� ������ ���� ������ ����� �뷫���� �� ����(nCells) �ʱ� ����
	placingZone.nCells = (short)(floor (placingZone.beamLength / 1.200));

SECOND:

	// [DIALOG] 2��° ���̾�α׿��� ������ ��ġ�� �����մϴ�.
	clickedOKButton = false;
	clickedPrevButton = false;
	result = DGBlankModalDialog (500, 360, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, beamTableformPlacerHandler2, 0);
	
	// ���� ��ư�� ������ 1��° ���̾�α� �ٽ� ����
	if (clickedPrevButton == true)
		goto FIRST;

	// 2��° ���̾�α׿��� OK ��ư�� �����߸� ���� �ܰ�� �Ѿ
	if (clickedOKButton != true)
		return err;

	// ��ü ��ġ ������
	placingZone.alignPlacingZone (&placingZone);

	// [DIALOG] ���ٸ�/�ۿ��� �������� ��ġ���� ���θ� ���
	clickedOKButton = false;
	clickedPrevButton = false;
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32509, ACAPI_GetOwnResModule (), beamTableformPlacerHandler3, 0);

	// ���� ��ư�� ������ 2��° ���̾�α� �ٽ� ����
	if (clickedPrevButton == true)
		goto SECOND;

	// �⺻ ��ü ��ġ�ϱ�
	err = placingZone.placeBasicObjects (&placingZone);

	// ������ ��ġ�ϱ�
	if (placingZone.tableformType == 1)
		err = placingZone.placeAuxObjectsA (&placingZone);
	else if (placingZone.tableformType == 2)
		err = placingZone.placeAuxObjectsB (&placingZone);


	// ����� ��ü �׷�ȭ
	groupElements (elemList_LeftEnd);
	groupElements (elemList_RightEnd);

	for (xx = 0 ; xx < 10 ; ++xx) {
		groupElements (elemList_Tableform [xx]);
		groupElements (elemList_Plywood [xx]);
	}

	// 3��° ���̾�α׿��� OK ��ư�� �����߸� ���� �ܰ�� �Ѿ
	if (clickedOKButton == true) {
		err = placingZone.placeSupportingPostPreset (&placingZone);		// ���ٸ�/�ۿ��� �������� ��ġ��
		groupElements (elemList_SupportingPost);						// ���ٸ� �׷�ȭ
	}

	// �ܿ��� ä���
	std::wstring title;

	if (placingZone.gapSideLeft > EPS) {
		title = L"�ܿ��� ��ġ (�� ������)";
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32512, ACAPI_GetOwnResModule (), beamTableformPlacerHandler4_Insulation, (DGUserData) &title);
		
		if (result == DG_OK)
			placingZone.placeInsulationsSide (&placingZone, &infoBeam, &insulElem, true);
	}

	if (placingZone.gapSideRight > EPS) {
		title = L"�ܿ��� ��ġ (�� ������)";
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32512, ACAPI_GetOwnResModule (), beamTableformPlacerHandler4_Insulation, (DGUserData) &title);
		
		if (result == DG_OK)
			placingZone.placeInsulationsSide (&placingZone, &infoBeam, &insulElem, true);
	}

	if (placingZone.gapBottom > EPS) {
		title = L"�ܿ��� ��ġ (�� �Ϻ�)";
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32512, ACAPI_GetOwnResModule (), beamTableformPlacerHandler4_Insulation, (DGUserData) &title);
		
		if (result == DG_OK)
			placingZone.placeInsulationsBottom (&placingZone, &infoBeam, &insulElem, false);
	}

	return	err;
}

// Cell �迭�� �ʱ�ȭ��
void	BeamTableformPlacingZone::initCells (BeamTableformPlacingZone* placingZone)
{
	short xx, yy;

	// �� ����/�Ϻ� ��� ���� �ʱ�ȭ
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

	// ���� ������ ���� ä�� ���� �ʱ�ȭ
	placingZone->bFillMarginBegin = false;
	placingZone->bFillMarginEnd = false;

	// ���� ������ ���� ���� �ʱ�ȭ
	placingZone->marginBegin = 0.0;
	placingZone->marginEnd = 0.0;

	// �� �糡 ��
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

	// �� ���� �ʱ�ȭ
	placingZone->nCells = 0;

	// �� ���� �ʱ�ȭ
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

// ��(0-��� �ε��� ��ȣ)�� ���ϴ� �� ��ġ X ��ǥ�� ����
double	BeamTableformPlacingZone::getCellPositionLeftBottomX (BeamTableformPlacingZone* placingZone, short idx)
{
	double	distance = (placingZone->bFillMarginBegin == true) ? placingZone->marginBegin : 0;

	for (short xx = 0 ; xx < idx ; ++xx)
		distance += placingZone->cellsAtLSide [0][xx].dirLen;

	return distance;
}

// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
void	BeamTableformPlacingZone::alignPlacingZone (BeamTableformPlacingZone* placingZone)
{
	short	xx;

	// ���� ����
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

	// ���� ����
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
		// ���� (�Ʒ��� ������ ����)
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

		// ���� (�ٷ������̼� ����)
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

		// ���� (���� ������ ����)
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

		// ���� (���� ����)
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

		// �Ϻ� (������, �ٷ�, ������)
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

// ������/�ٷ�/����/���縦 ��ġ��
GSErrCode	BeamTableformPlacingZone::placeBasicObjects (BeamTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx, yy;

	// ���� ������ ���� ���
	bool	bShow;
	bool	bBeginFound;
	short	beginIndex, endIndex;
	double	remainLengthDouble;
	double	lengthDouble;

	// ���� �� ���� ��ġ�� ����
	double	horLen, verLen;
	bool	bTimberMove;
	double	moveZ;
	short	addedPlywood;

	// ȸ�� ������ ���, ������ ���� �Ķ���Ϳ� ������ ��� ���� ����
	double	slantAngle = (placingZone->slantAngle >= EPS) ? placingZone->slantAngle : DegreeToRad (360.0) + placingZone->slantAngle;
	double	minusSlantAngle = (placingZone->slantAngle >= EPS) ? DegreeToRad (360.0) - placingZone->slantAngle : -placingZone->slantAngle;

	EasyObjectPlacement euroform, fillersp, plywood, timber;
	EasyObjectPlacement plywood1, plywood2, plywood3;

	if (placingZone->bFillMarginBegin == true) {
		// ���� �κ� ����(L) ���� ��ġ
		if ((placingZone->bFillBottomFormAtLSide == true) && (placingZone->beginCellAtLSide.dirLen > EPS)) {
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
			moveIn3D ('z', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, 0.0615 * sin (placingZone->slantAngle), &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList_LeftEnd.Push (plywood.placeObject (14,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "��������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->beginCellAtLSide.perLen + 0.0615 - placingZone->hRest_Left).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->beginCellAtLSide.dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.070).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}

		// ���� �κ� ����(R) ���� ��ġ
		if ((placingZone->bFillBottomFormAtRSide == true) && (placingZone->beginCellAtRSide.dirLen > EPS)) {
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->beginCellAtRSide.leftBottomX, placingZone->beginCellAtRSide.leftBottomY, placingZone->beginCellAtRSide.leftBottomZ, placingZone->beginCellAtRSide.ang);
			moveIn3D ('z', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, 0.0615 * sin (placingZone->slantAngle), &plywood.posX, &plywood.posY, &plywood.posZ);
			plywood.radAng += DegreeToRad (180.0);
			elemList_LeftEnd.Push (plywood.placeObjectMirrored (14,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "��������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->beginCellAtRSide.perLen + 0.0615 - placingZone->hRest_Right).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->beginCellAtRSide.dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.070).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}

		// ���� �κ� �Ϻ� ���� ��ġ
		if (placingZone->beginCellAtBottom.dirLen > EPS) {
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->beginCellAtBottom.leftBottomX, placingZone->beginCellAtBottom.leftBottomY, placingZone->beginCellAtBottom.leftBottomZ, placingZone->beginCellAtBottom.ang);
			elemList_LeftEnd.Push (plywood.placeObject (14,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�ٴڱ��",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->beginCellAtBottom.perLen).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->beginCellAtBottom.dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.070).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}
	}

	if (placingZone->bFillMarginEnd == true) {
		// �� �κ� ����(L) ���� ��ġ
		if ((placingZone->bFillBottomFormAtLSide == true) && (placingZone->endCellAtLSide.dirLen > EPS)) {
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->endCellAtLSide.leftBottomX, placingZone->endCellAtLSide.leftBottomY, placingZone->endCellAtLSide.leftBottomZ, placingZone->endCellAtLSide.ang);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, -placingZone->endCellAtLSide.dirLen, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3D ('z', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, 0.0615 * sin (placingZone->slantAngle), &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList_RightEnd.Push (plywood.placeObject (14,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "��������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->endCellAtLSide.perLen + 0.0615 - placingZone->hRest_Left).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->endCellAtLSide.dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.070).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}

		// �� �κ� ����(R) ���� ��ġ
		if ((placingZone->bFillBottomFormAtRSide == true) && (placingZone->endCellAtRSide.dirLen > EPS)) {
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->endCellAtRSide.leftBottomX, placingZone->endCellAtRSide.leftBottomY, placingZone->endCellAtRSide.leftBottomZ, placingZone->endCellAtRSide.ang);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, -placingZone->endCellAtRSide.dirLen, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3D ('z', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, 0.0615 * sin (placingZone->slantAngle), &plywood.posX, &plywood.posY, &plywood.posZ);
			plywood.radAng += DegreeToRad (180.0);
			elemList_RightEnd.Push (plywood.placeObjectMirrored (14,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "��������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->endCellAtRSide.perLen + 0.0615 - placingZone->hRest_Right).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->endCellAtRSide.dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.070).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}

		// �� �κ� �Ϻ� ���� ��ġ
		if (placingZone->endCellAtBottom.dirLen > EPS) {
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->endCellAtBottom.leftBottomX, placingZone->endCellAtBottom.leftBottomY, placingZone->endCellAtBottom.leftBottomZ, placingZone->endCellAtBottom.ang);
			moveIn3DSlope ('x', plywood.radAng, placingZone->slantAngle, -placingZone->endCellAtBottom.dirLen, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList_RightEnd.Push (plywood.placeObject (14,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�ٴڱ��",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->endCellAtBottom.perLen).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->endCellAtBottom.dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.070).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}
	}

	// ����(L) ������ 1�� ��ġ
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);

		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > EPS) && (placingZone->cellsAtLSide [0][xx].dirLen > EPS)) {
			moveIn3DSlope ('x', euroform.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (euroform.placeObject (6,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsAtLSide [0][xx].perLen * 1000.0).c_str(),
				"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsAtLSide [0][xx].dirLen * 1000.0).c_str(),
				"u_ins", APIParT_CString, "��������",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(),
				"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0) + placingZone->slantAngle).c_str()));
		}
	}

	// ����(L) �ٷ������̼� ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [1][xx].objType == FILLERSP) && (placingZone->cellsAtLSide [1][xx].perLen > 0) && (placingZone->cellsAtLSide [1][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [1][xx].objType != FILLERSP) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [1][yy].dirLen;

			fillersp.init (L("�ٷ������̼�v1.0.gsm"), layerInd_Fillerspacer, infoBeam.floorInd, placingZone->cellsAtLSide [1][beginIndex].leftBottomX, placingZone->cellsAtLSide [1][beginIndex].leftBottomY, placingZone->cellsAtLSide [1][beginIndex].leftBottomZ, placingZone->cellsAtLSide [1][beginIndex].ang);

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

	// ����(L) ������ 2�� ��ġ
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtLSide [2][xx].leftBottomX, placingZone->cellsAtLSide [2][xx].leftBottomY, placingZone->cellsAtLSide [2][xx].leftBottomZ, placingZone->cellsAtLSide [2][xx].ang);

		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > EPS) && (placingZone->cellsAtLSide [2][xx].dirLen > EPS)) {
			moveIn3DSlope ('x', euroform.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [2][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (euroform.placeObject (6,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsAtLSide [2][xx].perLen * 1000.0).c_str(),
				"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsAtLSide [2][xx].dirLen * 1000.0).c_str(),
				"u_ins", APIParT_CString, "��������",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(),
				"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0) + placingZone->slantAngle).c_str()));
		}
	}

	// ����(L) ����/���� ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if (getObjectType (placingZone, true, 3) == TIMBER) {
			if ((placingZone->cellsAtLSide [3][xx].objType == TIMBER) && (placingZone->cellsAtLSide [3][xx].perLen > 0) && (placingZone->cellsAtLSide [3][xx].dirLen > 0)) {
				// �������� �ε��� ���� ã��
				if (bBeginFound == false) {
					beginIndex = xx;
					bBeginFound = true;
					bShow = true;

					bTimberMove = false;
					moveZ = 0.0;
					addedPlywood = 0;
					if (placingZone->cellsAtLSide [3][xx].perLen < 0.040 - EPS) {
						// 40mm �̸��̸� ��������(!) 50*80 ����
						horLen = 0.050;
						verLen = 0.080;
						bTimberMove = true;

						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.010) < EPS)	addedPlywood = 1;	// 10mm �̸� ���� 1�� ����
						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.020) < EPS)	addedPlywood = 2;	// 20mm �̸� ���� 2�� ����
						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.030) < EPS)	addedPlywood = 3;	// 30mm �̸� ���� 3�� ����
					} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.050 - EPS)) {
						// 40mm �̻� 50mm �̸��̸�, 50*40 ����
						horLen = 0.050;
						verLen = 0.040;
					} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.080 - EPS)) {
						// 50mm �̻� 80mm �̸��̸�, 80*50 ����
						horLen = 0.080;
						verLen = 0.050;
						moveZ = verLen;

						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.060) < EPS)	addedPlywood = 1;	// 60mm �̸� ���� 1�� ����
						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.070) < EPS)	addedPlywood = 2;	// 70mm �̸� ���� 2�� ����
					} else {
						// 80mm �̻� 90mm �̸��̸�, 80*80 ����
						horLen = 0.080;
						verLen = 0.080;
						moveZ = verLen;

						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.090) < EPS)	addedPlywood = 1;	// 90mm �̸� ���� 1�� ����
						if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.100) < EPS)	addedPlywood = 2;	// 100mm �̸� ���� 2�� ����
					}
				}
				endIndex = xx;
			}

			if (((placingZone->cellsAtLSide [3][xx].objType != TIMBER) || (xx == placingZone->nCells-1)) && (bShow == true)) {
				// ���� ������ ������ ��� ��ġ�ϱ� (����)
				remainLengthDouble = 0.0;
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtLSide [3][yy].dirLen;

				timber.init (L("����v1.0.gsm"), layerInd_Timber, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ, placingZone->cellsAtLSide [3][beginIndex].ang);

				while (remainLengthDouble > EPS) {
					if (remainLengthDouble > 3.600)
						lengthDouble = 3.600;
					else
						lengthDouble = remainLengthDouble;

					// ���� ��ġ
					if (bTimberMove == true) {
						moveIn3D ('y', timber.radAng, -0.067, &timber.posX, &timber.posY, &timber.posZ);
						moveIn3D ('z', timber.radAng, -0.080 * cos (placingZone->slantAngle), &timber.posX, &timber.posY, &timber.posZ);
						moveIn3D ('x', timber.radAng, 0.080 * sin (placingZone->slantAngle), &timber.posX, &timber.posY, &timber.posZ);
					}
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (timber.placeObject (6,
						"w_ins", APIParT_CString, "�������",
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

				// ���� ������ ������ ��� ��ġ�ϱ� (�߰� ����)
				remainLengthDouble = 0.0;
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtLSide [3][yy].dirLen;

				plywood1.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ, placingZone->cellsAtLSide [3][beginIndex].ang);
				plywood2.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ, placingZone->cellsAtLSide [3][beginIndex].ang);
				plywood3.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ, placingZone->cellsAtLSide [3][beginIndex].ang);

				while (remainLengthDouble > EPS) {
					if (remainLengthDouble > 2.400)
						lengthDouble = 2.400;
					else
						lengthDouble = remainLengthDouble;
						
					if (addedPlywood >= 1) {
						moveIn3D ('y', plywood1.radAng, -0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3D ('z', plywood1.radAng, moveZ * cos (placingZone->slantAngle), &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3D ('x', plywood1.radAng, -moveZ * sin (placingZone->slantAngle), &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood1.placeObject (8, "p_stan", APIParT_CString, "��԰�", "w_dir", APIParT_CString, "�ٴڵ���", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070).c_str(), "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(), "p_rot", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "sogak", APIParT_Boolean, "0.0"));
						moveIn3D ('y', plywood1.radAng, 0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3D ('z', plywood1.radAng, -moveZ * cos (placingZone->slantAngle), &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3D ('x', plywood1.radAng, moveZ * sin (placingZone->slantAngle), &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						moveIn3DSlope ('x', plywood1.radAng, placingZone->slantAngle, lengthDouble, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
					}
					if (addedPlywood >= 2) {
						moveIn3D ('y', plywood2.radAng, -0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3D ('z', plywood2.radAng, (moveZ + 0.0115) * cos (placingZone->slantAngle), &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3D ('x', plywood2.radAng, -(moveZ + 0.0115) * sin (placingZone->slantAngle), &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood2.placeObject (8, "p_stan", APIParT_CString, "��԰�", "w_dir", APIParT_CString, "�ٴڵ���", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070).c_str(), "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(), "p_rot", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "sogak", APIParT_Boolean, "0.0"));
						moveIn3D ('y', plywood2.radAng, 0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3D ('z', plywood2.radAng, -(moveZ + 0.0115) * cos (placingZone->slantAngle), &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3D ('x', plywood2.radAng, (moveZ + 0.0115) * sin (placingZone->slantAngle), &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						moveIn3DSlope ('x', plywood2.radAng, placingZone->slantAngle, lengthDouble, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
					}
					if (addedPlywood >= 3) {
						moveIn3D ('y', plywood3.radAng, -0.070, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						moveIn3D ('z', plywood3.radAng, (moveZ + 0.0115*2) * cos (placingZone->slantAngle), &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						moveIn3D ('x', plywood3.radAng, -(moveZ + 0.0115*2) * sin (placingZone->slantAngle), &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood3.placeObject (8, "p_stan", APIParT_CString, "��԰�", "w_dir", APIParT_CString, "�ٴڵ���", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070).c_str(), "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(), "p_rot", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "sogak", APIParT_Boolean, "0.0"));
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
			// 100mm �ʰ��̸�
			if ((placingZone->cellsAtLSide [3][xx].objType == PLYWOOD) && (placingZone->cellsAtLSide [3][xx].perLen > 0) && (placingZone->cellsAtLSide [3][xx].dirLen > 0)) {
				// �������� �ε��� ���� ã��
				if (bBeginFound == false) {
					beginIndex = xx;
					bBeginFound = true;
					bShow = true;
				}
				endIndex = xx;
			}

			if (((placingZone->cellsAtLSide [3][xx].objType != PLYWOOD) || (xx == placingZone->nCells-1)) && (bShow == true)) {
				// ���� ������ ������ ��� ��ġ�ϱ� (����)
				remainLengthDouble = 0.0;
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtLSide [3][yy].dirLen;

				plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ, placingZone->cellsAtLSide [3][beginIndex].ang);

				while (remainLengthDouble > EPS) {
					if (remainLengthDouble > 2.400)
						lengthDouble = 2.400;
					else
						lengthDouble = remainLengthDouble;

					// ���� ��ġ
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood.placeObject (14,
						"p_stan", APIParT_CString, "��԰�",
						"w_dir", APIParT_CString, "��������",
						"p_thk", APIParT_CString, "11.5T",
						"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtLSide [3][beginIndex].perLen).c_str(),
						"p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(),
						"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
						"sogak", APIParT_Boolean, "1.0",
						"bInverseSogak", APIParT_Boolean, "1.0",
						"prof", APIParT_CString, "�Ұ�",
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
	
	// ����(R) ������ 1�� ��ġ
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);

		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > EPS) && (placingZone->cellsAtRSide [0][xx].dirLen > EPS)) {
			moveIn3DSlope ('x', euroform.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [0][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			euroform.radAng += DegreeToRad (180.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (euroform.placeObjectMirrored (6,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsAtRSide [0][xx].perLen * 1000.0).c_str(),
				"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsAtRSide [0][xx].dirLen * 1000.0).c_str(),
				"u_ins", APIParT_CString, "��������",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(),
				"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0) + placingZone->slantAngle).c_str()));
		}
	}

	// ����(R) �ٷ������̼� ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [1][xx].objType == FILLERSP) && (placingZone->cellsAtRSide [1][xx].perLen > 0) && (placingZone->cellsAtRSide [1][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [1][xx].objType != FILLERSP) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [1][yy].dirLen;

			fillersp.init (L("�ٷ������̼�v1.0.gsm"), layerInd_Fillerspacer, infoBeam.floorInd, placingZone->cellsAtRSide [1][beginIndex].leftBottomX, placingZone->cellsAtRSide [1][beginIndex].leftBottomY, placingZone->cellsAtRSide [1][beginIndex].leftBottomZ, placingZone->cellsAtRSide [1][beginIndex].ang);

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

	// ����(R) ������ 2�� ��ġ
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtRSide [2][xx].leftBottomX, placingZone->cellsAtRSide [2][xx].leftBottomY, placingZone->cellsAtRSide [2][xx].leftBottomZ, placingZone->cellsAtRSide [2][xx].ang);

		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > EPS) && (placingZone->cellsAtRSide [2][xx].dirLen > EPS)) {
			moveIn3DSlope ('x', euroform.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [2][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			euroform.radAng += DegreeToRad (180.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (euroform.placeObjectMirrored (6,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsAtRSide [2][xx].perLen * 1000.0).c_str(),
				"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsAtRSide [2][xx].dirLen * 1000.0).c_str(),
				"u_ins", APIParT_CString, "��������",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(),
				"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0) + placingZone->slantAngle).c_str()));
		}
	}

	// ����(R) ����/���� ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if (getObjectType (placingZone, false, 3) == TIMBER) {
			if ((placingZone->cellsAtRSide [3][xx].objType == TIMBER) && (placingZone->cellsAtRSide [3][xx].perLen > 0) && (placingZone->cellsAtRSide [3][xx].dirLen > 0)) {
				// �������� �ε��� ���� ã��
				if (bBeginFound == false) {
					beginIndex = xx;
					bBeginFound = true;
					bShow = true;

					bTimberMove = false;
					moveZ = 0.0;
					addedPlywood = 0;
					if (placingZone->cellsAtRSide [3][xx].perLen < 0.040 - EPS) {
						// 40mm �̸��̸� ��������(!) 50*80 ����
						horLen = 0.050;
						verLen = 0.080;
						bTimberMove = true;

						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.010) < EPS)	addedPlywood = 1;	// 10mm �̸� ���� 1�� ����
						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.020) < EPS)	addedPlywood = 2;	// 20mm �̸� ���� 2�� ����
						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.030) < EPS)	addedPlywood = 3;	// 30mm �̸� ���� 3�� ����
					} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.050 - EPS)) {
						// 40mm �̻� 50mm �̸��̸�, 50*40 ����
						horLen = 0.050;
						verLen = 0.040;
					} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.080 - EPS)) {
						// 50mm �̻� 80mm �̸��̸�, 80*50 ����
						horLen = 0.080;
						verLen = 0.050;
						moveZ = verLen;

						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.060) < EPS)	addedPlywood = 1;	// 60mm �̸� ���� 1�� ����
						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.070) < EPS)	addedPlywood = 2;	// 70mm �̸� ���� 2�� ����
					} else {
						// 80mm �̻� 90mm �̸��̸�, 80*80 ����
						horLen = 0.080;
						verLen = 0.080;
						moveZ = verLen;

						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.090) < EPS)	addedPlywood = 1;	// 90mm �̸� ���� 1�� ����
						if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.100) < EPS)	addedPlywood = 2;	// 100mm �̸� ���� 2�� ����
					}
				}
				endIndex = xx;
			}

			if (((placingZone->cellsAtRSide [3][xx].objType != TIMBER) || (xx == placingZone->nCells-1)) && (bShow == true)) {
				// ���� ������ ������ ��� ��ġ�ϱ� (����)
				remainLengthDouble = 0.0;
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtRSide [3][yy].dirLen;

				timber.init (L("����v1.0.gsm"), layerInd_Timber, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ, placingZone->cellsAtRSide [3][beginIndex].ang);

				while (remainLengthDouble > EPS) {
					if (remainLengthDouble > 3.600)
						lengthDouble = 3.600;
					else
						lengthDouble = remainLengthDouble;

					// ���� ��ġ
					if (bTimberMove == true) {
						moveIn3D ('y', timber.radAng, 0.067, &timber.posX, &timber.posY, &timber.posZ);
						moveIn3D ('z', timber.radAng, -0.080 * cos (placingZone->slantAngle), &timber.posX, &timber.posY, &timber.posZ);
						moveIn3D ('x', timber.radAng, 0.080 * sin (placingZone->slantAngle), &timber.posX, &timber.posY, &timber.posZ);
					}
					timber.radAng += DegreeToRad (180.0);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (timber.placeObjectMirrored (6,
						"w_ins", APIParT_CString, "�������",
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

				// ���� ������ ������ ��� ��ġ�ϱ� (�߰� ����)
				remainLengthDouble = 0.0;
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtRSide [3][yy].dirLen;

				plywood1.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ, placingZone->cellsAtRSide [3][beginIndex].ang);
				plywood2.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ, placingZone->cellsAtRSide [3][beginIndex].ang);
				plywood3.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ, placingZone->cellsAtRSide [3][beginIndex].ang);

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
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood1.placeObjectMirrored (8, "p_stan", APIParT_CString, "��԰�", "w_dir", APIParT_CString, "�ٴڵ���", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070).c_str(), "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(), "p_rot", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "sogak", APIParT_Boolean, "0.0"));
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
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood2.placeObjectMirrored (8, "p_stan", APIParT_CString, "��԰�", "w_dir", APIParT_CString, "�ٴڵ���", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070).c_str(), "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(), "p_rot", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "sogak", APIParT_Boolean, "0.0"));
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
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood3.placeObjectMirrored (8, "p_stan", APIParT_CString, "��԰�", "w_dir", APIParT_CString, "�ٴڵ���", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070).c_str(), "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(), "p_rot", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "sogak", APIParT_Boolean, "0.0"));
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
			// 100mm �ʰ��̸�
			if ((placingZone->cellsAtRSide [3][xx].objType == PLYWOOD) && (placingZone->cellsAtRSide [3][xx].perLen > 0) && (placingZone->cellsAtRSide [3][xx].dirLen > 0)) {
				// �������� �ε��� ���� ã��
				if (bBeginFound == false) {
					beginIndex = xx;
					bBeginFound = true;
					bShow = true;
				}
				endIndex = xx;
			}

			if (((placingZone->cellsAtRSide [3][xx].objType != PLYWOOD) || (xx == placingZone->nCells-1)) && (bShow == true)) {
				// ���� ������ ������ ��� ��ġ�ϱ� (����)
				remainLengthDouble = 0.0;
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtRSide [3][yy].dirLen;

				plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ, placingZone->cellsAtRSide [3][beginIndex].ang);

				while (remainLengthDouble > EPS) {
					if (remainLengthDouble > 2.400)
						lengthDouble = 2.400;
					else
						lengthDouble = remainLengthDouble;

					// ���� ��ġ
					plywood.radAng += DegreeToRad (180.0);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (plywood.placeObjectMirrored (14,
						"p_stan", APIParT_CString, "��԰�",
						"w_dir", APIParT_CString, "��������",
						"p_thk", APIParT_CString, "11.5T",
						"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtRSide [3][beginIndex].perLen).c_str(),
						"p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(),
						"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (360.0) - placingZone->slantAngle).c_str(),
						"sogak", APIParT_Boolean, "1.0",
						"bInverseSogak", APIParT_Boolean, "1.0",
						"prof", APIParT_CString, "�Ұ�",
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

	// �Ϻ� ������(L) ��ġ
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);

		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > EPS) && (placingZone->cellsAtBottom [0][xx].dirLen > EPS)) {
			moveIn3DSlope ('x', euroform.radAng, placingZone->slantAngle, placingZone->cellsAtBottom [0][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			moveIn3D ('y', euroform.radAng, placingZone->cellsAtBottom [0][xx].perLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (euroform.placeObject (6,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsAtBottom [0][xx].perLen * 1000.0).c_str(),
				"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsAtBottom [0][xx].dirLen * 1000.0).c_str(),
				"u_ins", APIParT_CString, "��������",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(),
				"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0) + placingZone->slantAngle).c_str()));
		}
	}

	// �Ϻ� �ٷ������̼� ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [1][xx].objType == FILLERSP) && (placingZone->cellsAtBottom [1][xx].perLen > 0) && (placingZone->cellsAtBottom [1][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [1][xx].objType != FILLERSP) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [1][yy].dirLen;

			fillersp.init (L("�ٷ������̼�v1.0.gsm"), layerInd_Fillerspacer, infoBeam.floorInd, placingZone->cellsAtBottom [1][beginIndex].leftBottomX, placingZone->cellsAtBottom [1][beginIndex].leftBottomY, placingZone->cellsAtBottom [1][beginIndex].leftBottomZ, placingZone->cellsAtBottom [1][beginIndex].ang);

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

	// �Ϻ� ������(R) ��ġ
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtBottom [2][xx].leftBottomX, placingZone->cellsAtBottom [2][xx].leftBottomY, placingZone->cellsAtBottom [2][xx].leftBottomZ, placingZone->cellsAtBottom [2][xx].ang);

		if ((placingZone->cellsAtBottom [2][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [2][xx].perLen > EPS) && (placingZone->cellsAtBottom [2][xx].dirLen > EPS)) {
			moveIn3DSlope ('x', euroform.radAng, placingZone->slantAngle, placingZone->cellsAtBottom [2][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			moveIn3D ('y', euroform.radAng, placingZone->cellsAtBottom [2][xx].perLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (euroform.placeObject (6,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsAtBottom [2][xx].perLen * 1000.0).c_str(),
				"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsAtBottom [2][xx].dirLen * 1000.0).c_str(),
				"u_ins", APIParT_CString, "��������",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(),
				"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0) + placingZone->slantAngle).c_str()));
		}
	}

	// ���� �� ��ġ (���� �信�� �������� ������ ���)
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [0][xx].objType == PLYWOOD) && (placingZone->cellsAtLSide [0][xx].perLen > EPS) && (placingZone->cellsAtLSide [0][xx].dirLen > EPS)) {
			// ����(L) ���� ��ġ
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);
			elemList_Plywood [getAreaSeqNumOfCell (placingZone, true, false, xx)].Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "��������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtLSide [0][xx].perLen - placingZone->hRest_Left).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->cellsAtLSide [0][xx].dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));

			// ����(R) ���� ��ġ
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);
			plywood.radAng += DegreeToRad (180.0);
			elemList_Plywood [getAreaSeqNumOfCell (placingZone, true, false, xx)].Push (plywood.placeObjectMirrored (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "��������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtRSide [0][xx].perLen - placingZone->hRest_Right).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->cellsAtRSide [0][xx].dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));

			// �Ϻ� ���� ��ġ
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);
			moveIn3D ('y', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList_Plywood [getAreaSeqNumOfCell (placingZone, true, false, xx)].Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�ٴڱ��",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtBottom [0][xx].perLen + 0.0615*2).c_str(),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->cellsAtBottom [0][xx].dirLen).c_str(),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0).c_str(),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string ("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string ("%f", 0.0).c_str()));
		}
	}

	return	err;
}

// ������/�ٷ�/����/���縦 ä�� �� ������ ��ġ (�ƿ��ڳʾޱ�, ���������, �ɺ�Ʈ, �����������, ����Ŭ����, ������)
GSErrCode	BeamTableformPlacingZone::placeAuxObjectsA (BeamTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx, yy, zz;

	// ���� ������ ���� ���
	bool	bShow;
	bool	bBeginFound;
	short	beginIndex, endIndex;
	double	remainLengthDouble;
	double	lengthDouble;
	double	tempLengthDouble;

	// ȸ�� ������ ���, ������ ���� �Ķ���Ϳ� ������ ��� ���� ����
	double	slantAngle = (placingZone->slantAngle >= EPS) ? (placingZone->slantAngle) : (DegreeToRad (360.0) + placingZone->slantAngle);
	double	minusSlantAngle = (placingZone->slantAngle >= EPS) ? (DegreeToRad (360.0) - placingZone->slantAngle) : (-placingZone->slantAngle);

	EasyObjectPlacement outangle, hanger, blueClamp, blueTimberRail;
	EasyObjectPlacement pipe1, pipe2;
	EasyObjectPlacement pinbolt1, pinbolt2;

	// �ƿ��ڳʾޱ� (L) ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			outangle.init (L("�ƿ��ڳʾޱ�v1.0.gsm"), layerInd_OutcornerAngle, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);

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

	// �ƿ��ڳʾޱ� (R) ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			outangle.init (L("�ƿ��ڳʾޱ�v1.0.gsm"), layerInd_OutcornerAngle, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
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

	// ��������� (L) ��ġ - 1�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > 0) && (placingZone->cellsAtLSide [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [0][yy].dirLen;

			// 1�� ������
			pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);
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

			// 2�� ������
			pipe2.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);
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

				// 1�� ������
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, 0.062 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, -0.062 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				// 2�� ������
				//if ((abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, 0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, 0.062 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, -0.062 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, 0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				//}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// ��������� (L) ��ġ - 2�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > 0) && (placingZone->cellsAtLSide [2][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [2][yy].dirLen;

			// 1�� ������
			pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);
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

			// 2�� ������
			pipe2.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);
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

				// 1�� ������
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, 0.062 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, -0.062 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				// 2�� ������
				//if ((abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, 0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, 0.062 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, -0.062 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, 0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				//}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// ��������� (R) ��ġ - 1�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > 0) && (placingZone->cellsAtRSide [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [0][yy].dirLen;

			// 1�� ������
			pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);
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

			// 2�� ������
			pipe2.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);
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

				// 1�� ������
				//moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//pipe1.radAng += DegreeToRad (180.0);
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, 0.062 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, -0.062 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//pipe1.radAng -= DegreeToRad (180.0);
				// 2�� ������
				//if ((abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng += DegreeToRad (180.0);
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, -0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, 0.062 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, 0.062 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, -0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng -= DegreeToRad (180.0);
				//}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// ��������� (R) ��ġ - 2�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > 0) && (placingZone->cellsAtRSide [2][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [2][yy].dirLen;

			// 1�� ������
			pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);
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

			// 2�� ������
			pipe2.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);
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

				// 1�� ������
				//moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//pipe1.radAng += DegreeToRad (180.0);
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, 0.062 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, -0.062 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
				//moveIn3D ('z', pipe1.radAng, -0.031 * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//moveIn3D ('x', pipe1.radAng, 0.031 * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				//pipe1.radAng -= DegreeToRad (180.0);
				// 2�� ������
				//if ((abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng += DegreeToRad (180.0);
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, -0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, 0.062 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, 0.062 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', pipe2.radAng, -0.031 * cos (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					moveIn3D ('x', pipe2.radAng, -0.031 * sin (placingZone->slantAngle), &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng -= DegreeToRad (180.0);
				//}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// ��������� (�Ϻ�-L) ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('z', pipe1.radAng, (-0.0635 - 0.025) * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('x', pipe1.radAng, -(-0.0635 - 0.025) * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// ��������� (�Ϻ�-R) ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				pipe1.radAng -= DegreeToRad (180.0);

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// ��������� (�Ϻ�-����) ��ġ
	if (((abs (placingZone->cellsAtBottom [0][0].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtBottom [0][0].perLen - 0.500) < EPS)) || (placingZone->cellsAtBottom [2][0].objType == EUROFORM)) {
		bShow = false;
		bBeginFound = false;
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
				// �������� �ε��� ���� ã��
				if (bBeginFound == false) {
					beginIndex = xx;
					bBeginFound = true;
					bShow = true;
				}
				endIndex = xx;
			}

			if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
				// ���� ������ ������ ��� ��ġ�ϱ�
				remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

				pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
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

					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

					remainLengthDouble -= 6.000;
				}

				bBeginFound = false;
			}
		}
	}

	// �ɺ�Ʈ (L) ��ġ - 1�� ������
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);
		pinbolt2.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);

		// 1�� �������� ü���� �ɺ�Ʈ
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

		// 2�� �������� ü���� �ɺ�Ʈ
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

		// ���� ���� ������, ���� ���� �������� ���
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > EPS) && (placingZone->cellsAtLSide [0][xx].dirLen > EPS) && (placingZone->cellsAtLSide [0][xx+1].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx+1].perLen > EPS) && (placingZone->cellsAtLSide [0][xx+1].dirLen > EPS)) {
			// 1�� �������� ü���� �ɺ�Ʈ
			//moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			//pinbolt1.radAng += DegreeToRad (90.0);
			//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
			//pinbolt1.radAng -= DegreeToRad (90.0);

			// 2�� �������� ü���� �ɺ�Ʈ
			//if ((abs (placingZone->cellsAtLSide [0][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			//}
		}
	}

	// �ɺ�Ʈ (L) ��ġ - 2�� ������
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [2][xx].leftBottomX, placingZone->cellsAtLSide [2][xx].leftBottomY, placingZone->cellsAtLSide [2][xx].leftBottomZ, placingZone->cellsAtLSide [2][xx].ang);
		pinbolt2.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [2][xx].leftBottomX, placingZone->cellsAtLSide [2][xx].leftBottomY, placingZone->cellsAtLSide [2][xx].leftBottomZ, placingZone->cellsAtLSide [2][xx].ang);

		// 1�� �������� ü���� �ɺ�Ʈ
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

		// 2�� �������� ü���� �ɺ�Ʈ
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

		// ���� ���� ������, ���� ���� �������� ���
		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > EPS) && (placingZone->cellsAtLSide [2][xx].dirLen > EPS) && (placingZone->cellsAtLSide [2][xx+1].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx+1].perLen > EPS) && (placingZone->cellsAtLSide [2][xx+1].dirLen > EPS)) {
			// 1�� �������� ü���� �ɺ�Ʈ
			//moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			//pinbolt1.radAng += DegreeToRad (90.0);
			//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
			//pinbolt1.radAng -= DegreeToRad (90.0);

			// 2�� �������� ü���� �ɺ�Ʈ
			//if ((abs (placingZone->cellsAtLSide [2][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [2][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			//}
		}
	}

	// �ɺ�Ʈ (R) ��ġ - 1�� ������
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);
		pinbolt2.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);

		// 1�� �������� ü���� �ɺ�Ʈ
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

		// 2�� �������� ü���� �ɺ�Ʈ
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

		// ���� ���� ������, ���� ���� �������� ���
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > EPS) && (placingZone->cellsAtRSide [0][xx].dirLen > EPS) && (placingZone->cellsAtRSide [0][xx+1].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx+1].perLen > EPS) && (placingZone->cellsAtRSide [0][xx+1].dirLen > EPS)) {
			// 1�� �������� ü���� �ɺ�Ʈ
			//moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			//pinbolt1.radAng += DegreeToRad (90.0);
			//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
			//pinbolt1.radAng -= DegreeToRad (90.0);

			// 2�� �������� ü���� �ɺ�Ʈ
			//if ((abs (placingZone->cellsAtRSide [0][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [0][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			//}
		}
	}

	// �ɺ�Ʈ (R) ��ġ - 2�� ������
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [2][xx].leftBottomX, placingZone->cellsAtRSide [2][xx].leftBottomY, placingZone->cellsAtRSide [2][xx].leftBottomZ, placingZone->cellsAtRSide [2][xx].ang);
		pinbolt2.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [2][xx].leftBottomX, placingZone->cellsAtRSide [2][xx].leftBottomY, placingZone->cellsAtRSide [2][xx].leftBottomZ, placingZone->cellsAtRSide [2][xx].ang);

		// 1�� �������� ü���� �ɺ�Ʈ
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

		// 2�� �������� ü���� �ɺ�Ʈ
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

		// ���� ���� ������, ���� ���� �������� ���
		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > EPS) && (placingZone->cellsAtRSide [2][xx].dirLen > EPS) && (placingZone->cellsAtRSide [2][xx+1].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx+1].perLen > EPS) && (placingZone->cellsAtRSide [2][xx+1].dirLen > EPS)) {
			// 1�� �������� ü���� �ɺ�Ʈ
			//moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [2][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			//pinbolt1.radAng += DegreeToRad (90.0);
			//elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
			//pinbolt1.radAng -= DegreeToRad (90.0);

			// 2�� �������� ü���� �ɺ�Ʈ
			//if ((abs (placingZone->cellsAtRSide [2][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [2][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			//}
		}
	}

	// �ɺ�Ʈ (�Ϻ�-����) ��ġ
	if (((abs (placingZone->cellsAtBottom [0][0].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtBottom [0][0].perLen - 0.500) < EPS)) || (placingZone->cellsAtBottom [2][0].objType == EUROFORM)) {
		for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
			pinbolt1.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);

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

			// ���� ���� ������, ���� ���� �������� ���
			if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > EPS) && (placingZone->cellsAtBottom [0][xx].dirLen > EPS) && (placingZone->cellsAtBottom [0][xx+1].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx+1].perLen > EPS) && (placingZone->cellsAtBottom [0][xx+1].dirLen > EPS)) {
				moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtBottom [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
				pinbolt1.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
				pinbolt1.radAng -= DegreeToRad (90.0);
			}
		}
	}

	// ����������� (�Ϻ�-L) ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			hanger.init (L("�����������.gsm"), layerInd_RectpipeHanger, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3D ('z', hanger.radAng, -0.0635 * cos (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3D ('x', hanger.radAng, 0.0635 * sin (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "�����������", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (90.0);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, -0.150 + remainLengthDouble - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "�����������", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (90.0);
			tempLengthDouble = 0.0;
			for (zz = beginIndex ; zz <= beginIndex + round ((endIndex - beginIndex) / 2, 0) ; ++zz) {
				tempLengthDouble += placingZone->cellsAtBottom [0][zz].dirLen;
			}
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150 - remainLengthDouble + tempLengthDouble - 0.450, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "�����������", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (90.0);

			bBeginFound = false;
		}
	}

	// ����������� (�Ϻ�-R) ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			hanger.init (L("�����������.gsm"), layerInd_RectpipeHanger, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3D ('z', hanger.radAng, -0.0635 * cos (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3D ('x', hanger.radAng, 0.0635 * sin (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3D ('y', hanger.radAng, placingZone->cellsAtBottom [0][xx].perLen + placingZone->cellsAtBottom [1][xx].perLen + placingZone->cellsAtBottom [2][xx].perLen, &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (270.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "�����������", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (270.0);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, -0.150 + remainLengthDouble - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (270.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "�����������", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (270.0);
			tempLengthDouble = 0.0;
			for (zz = beginIndex ; zz <= beginIndex + round ((endIndex - beginIndex) / 2, 0) ; ++zz) {
				tempLengthDouble += placingZone->cellsAtBottom [0][zz].dirLen;
			}
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150 - remainLengthDouble + tempLengthDouble - 0.450, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (270.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "�����������", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (270.0);

			bBeginFound = false;
		}
	}

	// ����Ŭ���� (L) - 1�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > 0) && (placingZone->cellsAtLSide [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [0][yy].dirLen;

			// ���� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// �� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// ����Ŭ���� (L) - 2�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > 0) && (placingZone->cellsAtLSide [2][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [2][yy].dirLen;

			// ���� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// �� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// ����Ŭ���� (R) - 1�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > 0) && (placingZone->cellsAtRSide [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [0][yy].dirLen;

			// ���� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// �� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// ����Ŭ���� (R) - 2�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > 0) && (placingZone->cellsAtRSide [2][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [2][yy].dirLen;

			// ���� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// �� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// ����Ŭ���� (�Ϻ�-L)
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			// ���� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			
			if ( !((abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// �� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if ( !((abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// ����Ŭ���� (�Ϻ�-R)
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [2][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [2][xx].perLen > 0) && (placingZone->cellsAtBottom [2][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [2][yy].dirLen;

			// ���� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [2][beginIndex].leftBottomX, placingZone->cellsAtBottom [2][beginIndex].leftBottomY, placingZone->cellsAtBottom [2][beginIndex].leftBottomZ, placingZone->cellsAtBottom [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if ( !((abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// �� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [2][beginIndex].leftBottomX, placingZone->cellsAtBottom [2][beginIndex].leftBottomY, placingZone->cellsAtBottom [2][beginIndex].leftBottomZ, placingZone->cellsAtBottom [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if ( !((abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// ������ (���-L): ���� �Ʒ����� 4��° ���� ������ ���
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [3][xx].objType == TIMBER) || (placingZone->cellsAtLSide [3][xx].objType == PLYWOOD)) {
			blueTimberRail.init (L("������v1.0.gsm"), layerInd_BlueTimberRail, infoBeam.floorInd, placingZone->cellsAtLSide [3][xx].leftBottomX, placingZone->cellsAtLSide [3][xx].leftBottomY, placingZone->cellsAtLSide [3][xx].leftBottomZ, placingZone->cellsAtLSide [3][xx].ang);

			if (placingZone->cellsAtLSide [3][xx].perLen < 0.040 - EPS) {
				// ������ ��ġ�� 50*80 ����
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.050 - EPS)) {
				// 50*40 ����
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.080 - EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*50 ����
				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.080 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.100 + EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*80 ����
				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if (placingZone->cellsAtLSide [3][xx].perLen >= 0.100 - EPS) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.064, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// ���� �� ����Ʋ
				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			}
		}
	}

	// ������ (���-R): ���� �Ʒ����� 4��° ���� ������ ���
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [3][xx].objType == TIMBER) || (placingZone->cellsAtRSide [3][xx].objType == PLYWOOD)) {
			blueTimberRail.init (L("������v1.0.gsm"), layerInd_BlueTimberRail, infoBeam.floorInd, placingZone->cellsAtRSide [3][xx].leftBottomX, placingZone->cellsAtRSide [3][xx].leftBottomY, placingZone->cellsAtRSide [3][xx].leftBottomZ, placingZone->cellsAtRSide [3][xx].ang);

			if (placingZone->cellsAtRSide [3][xx].perLen < 0.040 - EPS) {
				// ������ ��ġ�� 50*80 ����
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.050 - EPS)) {
				// 50*40 ����
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.080 - EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*50 ����
				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.080 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.100 + EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*80 ����
				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if (placingZone->cellsAtRSide [3][xx].perLen >= 0.100 - EPS) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.064, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// ���� �� ����Ʋ
				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			}
		}
	}

	return	err;
}

// ������/�ٷ�/����/���縦 ä�� �� ������ ��ġ (�ƿ��ڳʾޱ�, ���������, �ɺ�Ʈ, �����������, ����Ŭ����, ������)
GSErrCode	BeamTableformPlacingZone::placeAuxObjectsB (BeamTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx, yy, zz;

	// ���� ������ ���� ���
	bool	bShow;
	bool	bBeginFound;
	short	beginIndex, endIndex;
	double	remainLengthDouble;
	double	lengthDouble;
	double	tempLengthDouble;

	// ȸ�� ������ ���, ������ ���� �Ķ���Ϳ� ������ ��� ���� ����
	double	slantAngle = (placingZone->slantAngle >= EPS) ? (placingZone->slantAngle) : (DegreeToRad (360.0) + placingZone->slantAngle);
	double	minusSlantAngle = (placingZone->slantAngle >= EPS) ? (DegreeToRad (360.0) - placingZone->slantAngle) : (-placingZone->slantAngle);

	EasyObjectPlacement outangle, hanger, blueClamp, blueTimberRail;
	EasyObjectPlacement pipe1, pipe2;
	EasyObjectPlacement pinbolt1, pinbolt2;

	// �ƿ��ڳʾޱ� (L) ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			outangle.init (L("�ƿ��ڳʾޱ�v1.0.gsm"), layerInd_OutcornerAngle, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);

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

	// �ƿ��ڳʾޱ� (R) ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			outangle.init (L("�ƿ��ڳʾޱ�v1.0.gsm"), layerInd_OutcornerAngle, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
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

	// ��������� (L) ��ġ - 1�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > 0) && (placingZone->cellsAtLSide [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [0][yy].dirLen;

			// 1�� ������
			pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);
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

			// 2�� ������
			pipe2.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);
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

				// 1�� ������
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				// 2�� ������
				if ((abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS)) {
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// ��������� (L) ��ġ - 2�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > 0) && (placingZone->cellsAtLSide [2][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [2][yy].dirLen;

			// 1�� ������
			pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);
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

			// 2�� ������
			pipe2.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);
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

				// 1�� ������
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				// 2�� ������
				if ((abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS)) {
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// ��������� (R) ��ġ - 1�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > 0) && (placingZone->cellsAtRSide [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [0][yy].dirLen;

			// 1�� ������
			pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);
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

			// 2�� ������
			pipe2.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);
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

				// 1�� ������
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				pipe1.radAng += DegreeToRad (180.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				pipe1.radAng -= DegreeToRad (180.0);
				// 2�� ������
				if ((abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng += DegreeToRad (180.0);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					pipe2.radAng -= DegreeToRad (180.0);
				}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// ��������� (R) ��ġ - 2�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > 0) && (placingZone->cellsAtRSide [2][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [2][yy].dirLen;

			// 1�� ������
			pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);
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

			// 2�� ������
			pipe2.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);
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

				// 1�� ������
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				pipe1.radAng += DegreeToRad (180.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				pipe1.radAng -= DegreeToRad (180.0);
				// 2�� ������
				if ((abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3DSlope ('x', pipe2.radAng, placingZone->slantAngle, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng += DegreeToRad (180.0);
					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe2.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					pipe2.radAng -= DegreeToRad (180.0);
				}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// ��������� (�Ϻ�-L) ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('z', pipe1.radAng, (-0.0635 - 0.025) * cos (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('x', pipe1.radAng, -(-0.0635 - 0.025) * sin (placingZone->slantAngle), &pipe1.posX, &pipe1.posY, &pipe1.posZ);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// ��������� (�Ϻ�-R) ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "bPunching", APIParT_Boolean, "0.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				pipe1.radAng -= DegreeToRad (180.0);

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// ��������� (�Ϻ�-����) ��ġ
	if (((abs (placingZone->cellsAtBottom [0][0].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtBottom [0][0].perLen - 0.500) < EPS)) || (placingZone->cellsAtBottom [2][0].objType == EUROFORM)) {
		bShow = false;
		bBeginFound = false;
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
				// �������� �ε��� ���� ã��
				if (bBeginFound == false) {
					beginIndex = xx;
					bBeginFound = true;
					bShow = true;
				}
				endIndex = xx;
			}

			if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
				// ���� ������ ������ ��� ��ġ�ϱ�
				remainLengthDouble = 0.100;		// �������� 50mm Ƣ����Ƿ� ���� �̸� �߰���
				for (yy = beginIndex ; yy <= endIndex ; ++yy)
					remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

				pipe1.init (L("���������v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
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

					elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", lengthDouble).c_str(), "p_ang", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "����", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					moveIn3DSlope ('x', pipe1.radAng, placingZone->slantAngle, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

					remainLengthDouble -= 6.000;
				}

				bBeginFound = false;
			}
		}
	}

	// �ɺ�Ʈ (L) ��ġ - 1�� ������
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);
		pinbolt2.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);

		// 1�� �������� ü���� �ɺ�Ʈ
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

		// 2�� �������� ü���� �ɺ�Ʈ
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

		// ���� ���� ������, ���� ���� �������� ���
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > EPS) && (placingZone->cellsAtLSide [0][xx].dirLen > EPS) && (placingZone->cellsAtLSide [0][xx+1].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx+1].perLen > EPS) && (placingZone->cellsAtLSide [0][xx+1].dirLen > EPS)) {
			// 1�� �������� ü���� �ɺ�Ʈ
			moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			pinbolt1.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
			pinbolt1.radAng -= DegreeToRad (90.0);

			// 2�� �������� ü���� �ɺ�Ʈ
			if ((abs (placingZone->cellsAtLSide [0][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [0][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			}
		}
	}

	// �ɺ�Ʈ (L) ��ġ - 2�� ������
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [2][xx].leftBottomX, placingZone->cellsAtLSide [2][xx].leftBottomY, placingZone->cellsAtLSide [2][xx].leftBottomZ, placingZone->cellsAtLSide [2][xx].ang);
		pinbolt2.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtLSide [2][xx].leftBottomX, placingZone->cellsAtLSide [2][xx].leftBottomY, placingZone->cellsAtLSide [2][xx].leftBottomZ, placingZone->cellsAtLSide [2][xx].ang);

		// 1�� �������� ü���� �ɺ�Ʈ
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

		// 2�� �������� ü���� �ɺ�Ʈ
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

		// ���� ���� ������, ���� ���� �������� ���
		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > EPS) && (placingZone->cellsAtLSide [2][xx].dirLen > EPS) && (placingZone->cellsAtLSide [2][xx+1].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx+1].perLen > EPS) && (placingZone->cellsAtLSide [2][xx+1].dirLen > EPS)) {
			// 1�� �������� ü���� �ɺ�Ʈ
			moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			pinbolt1.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
			pinbolt1.radAng -= DegreeToRad (90.0);

			// 2�� �������� ü���� �ɺ�Ʈ
			if ((abs (placingZone->cellsAtLSide [2][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [2][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtLSide [2][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			}
		}
	}

	// �ɺ�Ʈ (R) ��ġ - 1�� ������
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);
		pinbolt2.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);

		// 1�� �������� ü���� �ɺ�Ʈ
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

		// 2�� �������� ü���� �ɺ�Ʈ
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

		// ���� ���� ������, ���� ���� �������� ���
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > EPS) && (placingZone->cellsAtRSide [0][xx].dirLen > EPS) && (placingZone->cellsAtRSide [0][xx+1].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx+1].perLen > EPS) && (placingZone->cellsAtRSide [0][xx+1].dirLen > EPS)) {
			// 1�� �������� ü���� �ɺ�Ʈ
			moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			pinbolt1.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
			pinbolt1.radAng -= DegreeToRad (90.0);

			// 2�� �������� ü���� �ɺ�Ʈ
			if ((abs (placingZone->cellsAtRSide [0][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [0][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [0][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			}
		}
	}

	// �ɺ�Ʈ (R) ��ġ - 2�� ������
	for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
		pinbolt1.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [2][xx].leftBottomX, placingZone->cellsAtRSide [2][xx].leftBottomY, placingZone->cellsAtRSide [2][xx].leftBottomZ, placingZone->cellsAtRSide [2][xx].ang);
		pinbolt2.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtRSide [2][xx].leftBottomX, placingZone->cellsAtRSide [2][xx].leftBottomY, placingZone->cellsAtRSide [2][xx].leftBottomZ, placingZone->cellsAtRSide [2][xx].ang);

		// 1�� �������� ü���� �ɺ�Ʈ
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

		// 2�� �������� ü���� �ɺ�Ʈ
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

		// ���� ���� ������, ���� ���� �������� ���
		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > EPS) && (placingZone->cellsAtRSide [2][xx].dirLen > EPS) && (placingZone->cellsAtRSide [2][xx+1].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx+1].perLen > EPS) && (placingZone->cellsAtRSide [2][xx+1].dirLen > EPS)) {
			// 1�� �������� ü���� �ɺ�Ʈ
			moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [2][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
			pinbolt1.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
			pinbolt1.radAng -= DegreeToRad (90.0);

			// 2�� �������� ü���� �ɺ�Ʈ
			if ((abs (placingZone->cellsAtRSide [2][xx].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [2][xx].perLen - 0.400) < EPS)) {
				moveIn3DSlope ('x', pinbolt2.radAng, placingZone->slantAngle, placingZone->cellsAtRSide [2][xx].dirLen, &pinbolt2.posX, &pinbolt2.posY, &pinbolt2.posZ);
				pinbolt2.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt2.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str()));
				pinbolt2.radAng -= DegreeToRad (90.0);
			}
		}
	}

	// �ɺ�Ʈ (�Ϻ�-����) ��ġ
	if (((abs (placingZone->cellsAtBottom [0][0].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtBottom [0][0].perLen - 0.500) < EPS)) || (placingZone->cellsAtBottom [2][0].objType == EUROFORM)) {
		for (xx = 0 ; xx < placingZone->nCells - 1 ; ++xx) {
			pinbolt1.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);

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

			// ���� ���� ������, ���� ���� �������� ���
			if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > EPS) && (placingZone->cellsAtBottom [0][xx].dirLen > EPS) && (placingZone->cellsAtBottom [0][xx+1].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx+1].perLen > EPS) && (placingZone->cellsAtBottom [0][xx+1].dirLen > EPS)) {
				moveIn3DSlope ('x', pinbolt1.radAng, placingZone->slantAngle, placingZone->cellsAtBottom [0][xx].dirLen, &pinbolt1.posX, &pinbolt1.posY, &pinbolt1.posZ);
				pinbolt1.radAng += DegreeToRad (90.0);
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (pinbolt1.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.100", "bolt_dia", APIParT_Length, "0.010", "washer_pos", APIParT_Length, "0.050", "washer_size", APIParT_Length, "0.100", "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
				pinbolt1.radAng -= DegreeToRad (90.0);
			}
		}
	}

	// ����������� (�Ϻ�-L) ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			hanger.init (L("�����������.gsm"), layerInd_RectpipeHanger, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);
			moveIn3D ('z', hanger.radAng, -0.0635 * cos (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3D ('x', hanger.radAng, 0.0635 * sin (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "�����������", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (90.0);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, -0.150 + remainLengthDouble - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "�����������", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (90.0);
			tempLengthDouble = 0.0;
			for (zz = beginIndex ; zz <= beginIndex + round ((endIndex - beginIndex) / 2, 0) ; ++zz) {
				tempLengthDouble += placingZone->cellsAtBottom [0][zz].dirLen;
			}
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150 - remainLengthDouble + tempLengthDouble - 0.450, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "�����������", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (90.0);

			bBeginFound = false;
		}
	}

	// ����������� (�Ϻ�-R) ��ġ
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			hanger.init (L("�����������.gsm"), layerInd_RectpipeHanger, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);
			moveIn3D ('z', hanger.radAng, -0.0635 * cos (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3D ('x', hanger.radAng, 0.0635 * sin (placingZone->slantAngle), &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3D ('y', hanger.radAng, placingZone->cellsAtBottom [0][xx].perLen + placingZone->cellsAtBottom [1][xx].perLen + placingZone->cellsAtBottom [2][xx].perLen, &hanger.posX, &hanger.posY, &hanger.posZ);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (270.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "�����������", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (270.0);
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, -0.150 + remainLengthDouble - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (270.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "�����������", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (270.0);
			tempLengthDouble = 0.0;
			for (zz = beginIndex ; zz <= beginIndex + round ((endIndex - beginIndex) / 2, 0) ; ++zz) {
				tempLengthDouble += placingZone->cellsAtBottom [0][zz].dirLen;
			}
			moveIn3DSlope ('x', hanger.radAng, placingZone->slantAngle, 0.150 - remainLengthDouble + tempLengthDouble - 0.450, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (270.0);
			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (hanger.placeObject (4, "m_type", APIParT_CString, "�����������", "c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angX", APIParT_Angle, format_string ("%f", slantAngle).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str()));
			hanger.radAng -= DegreeToRad (270.0);

			bBeginFound = false;
		}
	}

	// ����Ŭ���� (L) - 1�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > 0) && (placingZone->cellsAtLSide [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [0][yy].dirLen;

			// ���� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// �� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// ����Ŭ���� (L) - 2�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > 0) && (placingZone->cellsAtLSide [2][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [2][yy].dirLen;

			// ���� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// �� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// ����Ŭ���� (R) - 1�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > 0) && (placingZone->cellsAtRSide [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [0][yy].dirLen;

			// ���� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// �� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// ����Ŭ���� (R) - 2�� ������
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > 0) && (placingZone->cellsAtRSide [2][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [2][yy].dirLen;

			// ���� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// �� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);

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

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

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
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// ����Ŭ���� (�Ϻ�-L)
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > 0) && (placingZone->cellsAtBottom [0][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [0][yy].dirLen;

			// ���� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			
			if ( !((abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// �� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [0][beginIndex].leftBottomX, placingZone->cellsAtBottom [0][beginIndex].leftBottomY, placingZone->cellsAtBottom [0][beginIndex].leftBottomZ, placingZone->cellsAtBottom [0][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if ( !((abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [0][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// ����Ŭ���� (�Ϻ�-R)
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [2][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [2][xx].perLen > 0) && (placingZone->cellsAtBottom [2][xx].dirLen > 0)) {
			// �������� �ε��� ���� ã��
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// ���� ������ ������ ��� ��ġ�ϱ�
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [2][yy].dirLen;

			// ���� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [2][beginIndex].leftBottomX, placingZone->cellsAtBottom [2][beginIndex].leftBottomY, placingZone->cellsAtBottom [2][beginIndex].leftBottomZ, placingZone->cellsAtBottom [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, -0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if ( !((abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			// �� �κ�
			blueClamp.init (L("����Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoBeam.floorInd, placingZone->cellsAtBottom [2][beginIndex].leftBottomX, placingZone->cellsAtBottom [2][beginIndex].leftBottomY, placingZone->cellsAtBottom [2][beginIndex].leftBottomZ, placingZone->cellsAtBottom [2][beginIndex].ang);

			moveIn3DSlope ('x', blueClamp.radAng, placingZone->slantAngle, remainLengthDouble + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('z', blueClamp.radAng, -0.0659 * cos (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.0659 * sin (placingZone->slantAngle), &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			moveIn3D ('y', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			moveIn3D ('y', blueClamp.radAng, placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

			if ( !((abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.300) < EPS) || (abs (placingZone->cellsAtBottom [2][beginIndex].perLen - 0.200) < EPS)) )
				elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueClamp.placeObject (5, "type", APIParT_CString, "���θ���Ŭ����(����ǰv1)", "openingWidth", APIParT_Length, "0.047", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0) + minusSlantAngle).c_str(), "bReverseRotation", APIParT_Boolean, "1.0"));

			bBeginFound = false;
		}
	}

	// ������ (���-L): ���� �Ʒ����� 4��° ���� ������ ���
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [3][xx].objType == TIMBER) || (placingZone->cellsAtLSide [3][xx].objType == PLYWOOD)) {
			blueTimberRail.init (L("������v1.0.gsm"), layerInd_BlueTimberRail, infoBeam.floorInd, placingZone->cellsAtLSide [3][xx].leftBottomX, placingZone->cellsAtLSide [3][xx].leftBottomY, placingZone->cellsAtLSide [3][xx].leftBottomZ, placingZone->cellsAtLSide [3][xx].ang);

			if (placingZone->cellsAtLSide [3][xx].perLen < 0.040 - EPS) {
				// ������ ��ġ�� 50*80 ����
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.050 - EPS)) {
				// 50*40 ����
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.080 - EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*50 ����
				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.080 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.100 + EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*80 ����
				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if (placingZone->cellsAtLSide [3][xx].perLen >= 0.100 - EPS) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, -0.064, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// ���� �� ����Ʋ
				if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtLSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", minusSlantAngle).c_str()));
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			}
		}
	}

	// ������ (���-R): ���� �Ʒ����� 4��° ���� ������ ���
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [3][xx].objType == TIMBER) || (placingZone->cellsAtRSide [3][xx].objType == PLYWOOD)) {
			blueTimberRail.init (L("������v1.0.gsm"), layerInd_BlueTimberRail, infoBeam.floorInd, placingZone->cellsAtRSide [3][xx].leftBottomX, placingZone->cellsAtRSide [3][xx].leftBottomY, placingZone->cellsAtRSide [3][xx].leftBottomZ, placingZone->cellsAtRSide [3][xx].ang);

			if (placingZone->cellsAtRSide [3][xx].perLen < 0.040 - EPS) {
				// ������ ��ġ�� 50*80 ����
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.050 - EPS)) {
				// 50*40 ����
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.080 - EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*50 ����
				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.080 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.100 + EPS)) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// 80*80 ����
				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			} else if (placingZone->cellsAtRSide [3][xx].perLen >= 0.100 - EPS) {
				moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, -0.023 + 0.194, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng, 0.064, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng, -0.003 * cos (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('x', blueTimberRail.radAng, 0.003 * sin (placingZone->slantAngle), &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);

				// ���� �� ����Ʋ
				if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 1.200) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.900) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				} else if (abs (placingZone->cellsAtRSide [3][xx].dirLen - 0.600) < EPS) {
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						blueTimberRail.radAng += DegreeToRad (180.0);
						elemList_Tableform [getAreaSeqNumOfCell (placingZone, true, true, xx)].Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "������ 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", slantAngle).c_str()));
						blueTimberRail.radAng -= DegreeToRad (180.0);
					moveIn3DSlope ('x', blueTimberRail.radAng, placingZone->slantAngle, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				}
			}
		}
	}

	return	err;
}

// ���� ���̺��� ���̿� �ܿ��縦 ��ġ�� (�� ����)
GSErrCode	BeamTableformPlacingZone::placeInsulationsSide (BeamTableformPlacingZone* placingZone, InfoBeam* infoBeam, insulElemForBeamTableform* insulElem, bool bMoreWider)
{
	GSErrCode	err = NoError;

	// ȸ�� ������ ���, ������ ���� �Ķ���Ϳ� ������ ��� ���� ����
	double	minusSlantAngle = (placingZone->slantAngle >= EPS) ? DegreeToRad (360.0) - placingZone->slantAngle : -placingZone->slantAngle;

	short	xx, yy;
	short	totalXX, totalYY;
	double	horLen, verLen;
	double	remainHorLen, remainVerLen;

	EasyObjectPlacement insul;

	if (insulElem->bLimitSize == true) {
		// ����/���� ũ�� ������ true�� ��

		if (placingZone->gapSideLeft > EPS) {
			// ������
			// bMoreWider�� true�̸� ���� �ܿ��簡 �Ϻ� �ܿ��縦 ����, false�̸� �Ϻ� �ܿ��簡 ���� �ܿ��縦 ����ħ
			if (bMoreWider == true)
				remainHorLen = placingZone->areaHeight_Left + placingZone->gapBottom;
			else
				remainHorLen = placingZone->areaHeight_Left;
			remainVerLen = placingZone->beamLength;
			totalXX = (short)floor (remainHorLen / insulElem->maxHorLen);
			totalYY = (short)floor (remainVerLen / insulElem->maxVerLen);

			insul.init (L("�ܿ���v1.0.gsm"), insulElem->layerInd, infoBeam->floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
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
			// ������
			if (bMoreWider == true)
				remainHorLen = placingZone->areaHeight_Left + placingZone->gapBottom;
			else
				remainHorLen = placingZone->areaHeight_Left;
			remainVerLen = placingZone->beamLength;
			totalXX = (short)floor (remainHorLen / insulElem->maxHorLen);
			totalYY = (short)floor (remainVerLen / insulElem->maxVerLen);

			insul.init (L("�ܿ���v1.0.gsm"), insulElem->layerInd, infoBeam->floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
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
		// ����/���� ũ�� ������ false�� ��

		if (placingZone->gapSideLeft > EPS) {
			// ������
			// bMoreWider�� true�̸� ���� �ܿ��簡 �Ϻ� �ܿ��縦 ����, false�̸� �Ϻ� �ܿ��簡 ���� �ܿ��縦 ����ħ
			if (bMoreWider == true)
				horLen = placingZone->areaHeight_Left + placingZone->gapBottom;
			else
				horLen = placingZone->areaHeight_Left;
			verLen = placingZone->beamLength;

			insul.init (L("�ܿ���v1.0.gsm"), insulElem->layerInd, infoBeam->floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
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
			// ������
			if (bMoreWider == true)
				horLen = placingZone->areaHeight_Left + placingZone->gapBottom;
			else
				horLen = placingZone->areaHeight_Left;
			verLen = placingZone->beamLength;

			insul.init (L("�ܿ���v1.0.gsm"), insulElem->layerInd, infoBeam->floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
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

// ���� ���̺��� ���̿� �ܿ��縦 ��ġ�� (�� �Ϻ�)
GSErrCode	BeamTableformPlacingZone::placeInsulationsBottom (BeamTableformPlacingZone* placingZone, InfoBeam* infoBeam, insulElemForBeamTableform* insulElem, bool bMoreWider)
{
	GSErrCode	err = NoError;

	// ȸ�� ������ ���, ������ ���� �Ķ���Ϳ� ������ ��� ���� ����
	double	minusSlantAngle = (placingZone->slantAngle >= EPS) ? DegreeToRad (360.0) - placingZone->slantAngle : -placingZone->slantAngle;

	short	xx, yy;
	short	totalXX, totalYY;
	double	horLen, verLen;
	double	remainHorLen, remainVerLen;

	EasyObjectPlacement insul;

	if (insulElem->bLimitSize == true) {
		// ����/���� ũ�� ������ true�� ��
		if (bMoreWider == true)
			remainHorLen = placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight);
		else
			remainHorLen = placingZone->areaWidth_Bottom;
		remainVerLen = placingZone->beamLength;
		totalXX = (short)floor (remainHorLen / insulElem->maxHorLen);
		totalYY = (short)floor (remainVerLen / insulElem->maxVerLen);

		insul.init (L("�ܿ���v1.0.gsm"), insulElem->layerInd, infoBeam->floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
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
		// ����/���� ũ�� ������ false�� ��
		if (bMoreWider == true)
			horLen = placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight);
		else
			horLen = placingZone->areaWidth_Bottom;
		verLen = placingZone->beamLength;

		insul.init (L("�ܿ���v1.0.gsm"), insulElem->layerInd, infoBeam->floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
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

// ���ٸ�/�ۿ��� �������� ��ġ��
GSErrCode	BeamTableformPlacingZone::placeSupportingPostPreset (BeamTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx;
	double	distance;

	double	heightGapBeamBtwPost;
	char	postType [16];

	// �� �Ϻθ��� ������ ���ٸ� ��θ� �������� ����
	if (placingZone->typeOfSupportingPost == 1)
		heightGapBeamBtwPost = 0.3565;	// PERI ���ٸ� + GT24 �Ŵ��� ���
	else if (placingZone->typeOfSupportingPost == 2)
		heightGapBeamBtwPost = 0.2185;	// PERI ���ٸ� + �� �ۿ���

	// ������ �԰� �����ϱ�
	strcpy (postType, format_string ("%s", "MP 350").c_str());

	EasyObjectPlacement	verticalPost, horizontalPost, girder, beamBracket, yoke, timber, jackSupport;

	// PERI ���ٸ� + GT24 �Ŵ�
	if (placingZone->typeOfSupportingPost == 1) {
		// ��ġ: PERI���ٸ� ������
		verticalPost.init (L("PERI���ٸ� ������ v0.1.gsm"), layerInd_VerticalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
		moveIn3D ('z', verticalPost.radAng, -0.003 - 0.1135 - 0.240 - placingZone->gapBottom, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		moveIn3D ('x', verticalPost.radAng, placingZone->postStartOffset, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		moveIn3D ('y', verticalPost.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - placingZone->gapSideLeft, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
		moveIn3D ('y', verticalPost.radAng, placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
		moveIn3D ('x', verticalPost.radAng, placingZone->postGapLength, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
		moveIn3D ('y', verticalPost.radAng, -placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));

		// ��ġ: GT24 �Ŵ�
		girder.init (L("GT24 �Ŵ� v1.0.gsm"), layerInd_Girder, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
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

		// ��ġ: �� �����
		beamBracket.init (L("���� �� ����� v1.0.gsm"), layerInd_BeamBracket, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
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

	// PERI ���ٸ� + �� �ۿ���
	} else if (placingZone->typeOfSupportingPost == 2) {
		// ��ġ: PERI���ٸ� ������
		verticalPost.init (L("PERI���ٸ� ������ v0.1.gsm"), layerInd_VerticalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
		moveIn3D ('z', verticalPost.radAng, -0.1135 - 0.240 + 0.135 - placingZone->gapBottom, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		moveIn3D ('x', verticalPost.radAng, placingZone->postStartOffset, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		moveIn3D ('y', verticalPost.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - placingZone->gapSideLeft, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
		moveIn3D ('y', verticalPost.radAng, placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
		moveIn3D ('x', verticalPost.radAng, placingZone->postGapLength, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
		moveIn3D ('y', verticalPost.radAng, -placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
		elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));

		// ��ġ: �� �ۿ���
		if (abs (placingZone->postGapWidth - 0.900) < EPS)
			distance = 0.0015 + (placingZone->gapSideLeft + placingZone->gapSideRight);
		else if (abs (placingZone->postGapWidth - 1.200) < EPS)
			distance = 0.0015 + (placingZone->gapSideLeft + placingZone->gapSideRight);
		else if (abs (placingZone->postGapWidth - 1.500) < EPS)
			distance = 0.1515 + (placingZone->gapSideLeft + placingZone->gapSideRight);

		yoke.init (L("�� �ۿ���v1.0.gsm"), layerInd_Yoke, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
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

	// ��ġ: PERI���ٸ� ������ (�ʺ� ����)
	if (placingZone->typeOfSupportingPost == 1)
		distance = -0.003 - 0.1135 - 0.240;
	else if (placingZone->typeOfSupportingPost == 2)
		distance = -0.1135 - 0.240 + 0.135;

	distance -= 1.500;

	horizontalPost.init (L("PERI���ٸ� ������ v0.2.gsm"), layerInd_HorizontalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
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

	// ��ġ: PERI���ٸ� ������ (���� ����)
	horizontalPost.init (L("PERI���ٸ� ������ v0.2.gsm"), layerInd_HorizontalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
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
		// PERI ���ٸ� + GT24 �Ŵ�
		if (placingZone->typeOfSupportingPost == 1) {
			// ��ġ: PERI���ٸ� ������
			verticalPost.init (L("PERI���ٸ� ������ v0.1.gsm"), layerInd_VerticalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
			moveIn3D ('z', verticalPost.radAng, -0.003 - 0.1135 - 0.240 - placingZone->gapBottom, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			moveIn3D ('x', verticalPost.radAng, placingZone->beamLength - placingZone->postGapLength - placingZone->postStartOffset, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			moveIn3D ('y', verticalPost.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - placingZone->gapSideLeft, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
			moveIn3D ('y', verticalPost.radAng, placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
			moveIn3D ('x', verticalPost.radAng, placingZone->postGapLength, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
			moveIn3D ('y', verticalPost.radAng, -placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));

			// ��ġ: GT24 �Ŵ�
			girder.init (L("GT24 �Ŵ� v1.0.gsm"), layerInd_Girder, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
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

			// ��ġ: �� �����
			beamBracket.init (L("���� �� ����� v1.0.gsm"), layerInd_BeamBracket, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
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

		// PERI ���ٸ� + �� �ۿ���
		} else if (placingZone->typeOfSupportingPost == 2) {
			// ��ġ: PERI���ٸ� ������
			verticalPost.init (L("PERI���ٸ� ������ v0.1.gsm"), layerInd_VerticalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
			moveIn3D ('z', verticalPost.radAng, -0.1135 - 0.240 + 0.135 - placingZone->gapBottom, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			moveIn3D ('x', verticalPost.radAng, placingZone->beamLength - placingZone->postGapLength - placingZone->postStartOffset, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			moveIn3D ('y', verticalPost.radAng, (placingZone->areaWidth_Bottom + (placingZone->gapSideLeft + placingZone->gapSideRight) - placingZone->postGapWidth) / 2 - placingZone->gapSideLeft, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
			moveIn3D ('y', verticalPost.radAng, placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
			moveIn3D ('x', verticalPost.radAng, placingZone->postGapLength, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));
			moveIn3D ('y', verticalPost.radAng, -placingZone->postGapWidth, &verticalPost.posX, &verticalPost.posY, &verticalPost.posZ);
			elemList_SupportingPost.Push (verticalPost.placeObject (7, "stType", APIParT_CString, postType, "len_current", APIParT_Length, format_string ("%f", 1.950).c_str(), "bCrosshead", APIParT_Boolean, "0.0", "posCrosshead", APIParT_CString, "�ϴ�", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)).c_str()));

			// ��ġ: �� �ۿ���
			if (abs (placingZone->postGapWidth - 0.900) < EPS)
				distance = 0.0015 + (placingZone->gapSideLeft + placingZone->gapSideRight);
			else if (abs (placingZone->postGapWidth - 1.200) < EPS)
				distance = 0.0015 + (placingZone->gapSideLeft + placingZone->gapSideRight);
			else if (abs (placingZone->postGapWidth - 1.500) < EPS)
				distance = 0.1515 + (placingZone->gapSideLeft + placingZone->gapSideRight);

			yoke.init (L("�� �ۿ���v1.0.gsm"), layerInd_Yoke, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
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

		// ��ġ: PERI���ٸ� ������ (�ʺ� ����)
		if (placingZone->typeOfSupportingPost == 1)
			distance = -0.003 - 0.1135 - 0.240;
		else if (placingZone->typeOfSupportingPost == 2)
			distance = -0.1135 - 0.240 + 0.135;

		distance -= 1.500;

		horizontalPost.init (L("PERI���ٸ� ������ v0.2.gsm"), layerInd_HorizontalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
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

		// ��ġ: PERI���ٸ� ������ (���� ����)
		horizontalPost.init (L("PERI���ٸ� ������ v0.2.gsm"), layerInd_HorizontalPost, infoBeam.floorInd, placingZone->begC.x, placingZone->begC.y, placingZone->begC.z, placingZone->ang);
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

	// �輭��Ʈ�� ���� ��ġ
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if (placingZone->cellsAtBottom [0][xx].objType == PLYWOOD) {
			// ��ġ: ����
			timber.init (L("����v1.0.gsm"), layerInd_Timber, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);

			moveIn3D ('x', timber.radAng, placingZone->cellsAtBottom [0][xx].dirLen / 2 - 0.075, &timber.posX, &timber.posY, &timber.posZ);
			moveIn3D ('y', timber.radAng, -0.0215, &timber.posX, &timber.posY, &timber.posZ);		
			moveIn3D ('z', timber.radAng, -0.0115, &timber.posX, &timber.posY, &timber.posZ);

			timber.radAng += DegreeToRad (90.0);
			elemList_Plywood [getAreaSeqNumOfCell (placingZone, true, false, xx)].Push (timber.placeObject (6, "w_ins", APIParT_CString, "�ٴڴ�����", "w_w", APIParT_Length, format_string ("%f", 0.080).c_str(), "w_h", APIParT_Length, format_string ("%f", 0.050).c_str(), "w_leng", APIParT_Length, format_string ("%f", placingZone->cellsAtBottom [0][xx].perLen + (0.0615 - 0.040)*2).c_str(), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0).c_str()));
			timber.radAng -= DegreeToRad (90.0);

			moveIn3D ('x', timber.radAng, 0.100, &timber.posX, &timber.posY, &timber.posZ);

			timber.radAng += DegreeToRad (90.0);
			elemList_Plywood [getAreaSeqNumOfCell (placingZone, true, false, xx)].Push (timber.placeObject (6, "w_ins", APIParT_CString, "�ٴڴ�����", "w_w", APIParT_Length, format_string ("%f", 0.080).c_str(), "w_h", APIParT_Length, format_string ("%f", 0.050).c_str(), "w_leng", APIParT_Length, format_string ("%f", placingZone->cellsAtBottom [0][xx].perLen + (0.0615 - 0.040)*2).c_str(), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str(), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0).c_str()));
			timber.radAng -= DegreeToRad (90.0);

			// ��ġ: �� ����Ʈ
			jackSupport.init (L("�輭��Ʈv1.0.gsm"), layerInd_JackSupport, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);
			
			moveIn3D ('x', jackSupport.radAng, placingZone->cellsAtBottom [0][xx].dirLen / 2, &jackSupport.posX, &jackSupport.posY, &jackSupport.posZ);
			moveIn3D ('y', jackSupport.radAng, placingZone->cellsAtBottom [0][xx].perLen / 2, &jackSupport.posX, &jackSupport.posY, &jackSupport.posZ);
			moveIn3D ('z', jackSupport.radAng, -0.0115 - 0.080 - (2.000 - placingZone->gapBottom - 0.0115 - 0.080), &jackSupport.posX, &jackSupport.posY, &jackSupport.posZ);
		

			elemList_Plywood [getAreaSeqNumOfCell (placingZone, true, false, xx)].Push (jackSupport.placeObject (3, "j_comp", APIParT_CString, "�輭��Ʈ", "j_stan", APIParT_CString, "Free", "j_leng", APIParT_Length, format_string ("%f", 2.000 - placingZone->gapBottom - 0.0115 - 0.080).c_str()));
		}
	}

	return	err;
}

// ���� Ȥ�� ������ ���� idx ��° ���� ��ġ�Ǵ� ��ü�� Ÿ���� ������
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

// idx ��° ���� �� ��° �������� ���̺��� Ȥ�� ���� �����ΰ�?
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

// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK beamTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	short		xx;
	double		h1, h2, h3, h4, hRest_Left = 0.0, hRest_Right = 0.0;	// ������ ���� ����� ���� ����
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"���� ��ġ - �� �ܸ�");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// Ȯ�� ��ư
			DGSetItemText (dialogID, DG_OK, L"Ȯ ��");

			// ��� ��ư
			DGSetItemText (dialogID, DG_CANCEL, L"�� ��");

			//////////////////////////////////////////////////////////// ������ ��ġ (������)
			// �� �� üũ�ڽ�
			DGSetItemText (dialogID, LABEL_BEAM_SECTION, L"�� �ܸ�");
			DGSetItemText (dialogID, LABEL_BEAM_LEFT_HEIGHT, L"�� ���� (L)");
			DGSetItemText (dialogID, LABEL_BEAM_RIGHT_HEIGHT, L"�� ���� (R)");
			DGSetItemText (dialogID, LABEL_BEAM_WIDTH, L"�� �ʺ�");
			DGSetItemText (dialogID, LABEL_TOTAL_LEFT_HEIGHT, L"�� ���� (L)");
			DGSetItemText (dialogID, LABEL_TOTAL_RIGHT_HEIGHT, L"�� ���� (R)");
			DGSetItemText (dialogID, LABEL_TOTAL_WIDTH, L"�� �ʺ�");

			DGSetItemText (dialogID, LABEL_REST_LSIDE, L"������");
			DGSetItemText (dialogID, CHECKBOX_TIMBER_LSIDE, L"����/����");
			DGSetItemText (dialogID, CHECKBOX_T_FORM_LSIDE, L"������");
			DGSetItemText (dialogID, CHECKBOX_FILLER_LSIDE, L"�ٷ�");
			DGSetItemText (dialogID, CHECKBOX_B_FORM_LSIDE, L"������");

			DGSetItemText (dialogID, LABEL_REST_RSIDE, L"������");
			DGSetItemText (dialogID, CHECKBOX_TIMBER_RSIDE, L"����/����");
			DGSetItemText (dialogID, CHECKBOX_T_FORM_RSIDE, L"������");
			DGSetItemText (dialogID, CHECKBOX_FILLER_RSIDE, L"�ٷ�");
			DGSetItemText (dialogID, CHECKBOX_B_FORM_RSIDE, L"������");

			DGSetItemText (dialogID, CHECKBOX_L_FORM_BOTTOM, L"������");
			DGSetItemText (dialogID, CHECKBOX_FILLER_BOTTOM, L"�ٷ�");
			DGSetItemText (dialogID, CHECKBOX_R_FORM_BOTTOM, L"������");

			DGSetItemText (dialogID, BUTTON_COPY_TO_RIGHT, L"����\n��");

			// ��: ���̾� ����
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, L"���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, L"������");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, L"����");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER, L"����");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, L"�ƿ��ڳʾޱ�");
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSPACER, L"�ٷ������̼�");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, L"���������");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER, L"�����������");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, L"�ɺ�Ʈ��Ʈ");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK, L"������ ��ũ");
			DGSetItemText (dialogID, LABEL_LAYER_BLUE_CLAMP, L"����Ŭ����");
			DGSetItemText (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL, L"������");

			// üũ�ڽ�: ���̾� ����
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, L"���̾� ����");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			DGSetItemText (dialogID, BUTTON_AUTOSET, L"���̾� �ڵ� ����");

			// ���� ��Ʈ�� �ʱ�ȭ
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

			// �⺻ �������� �ʱⰪ On
			DGSetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE, TRUE);
			DGSetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE, TRUE);
			DGSetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM, TRUE);

			// �� ����/�ʺ� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_LEFT_HEIGHT, placingZone.areaHeight_Left);		// �� ���� (L)
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_RIGHT_HEIGHT, placingZone.areaHeight_Right);		// �� ���� (R)
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH, placingZone.areaWidth_Bottom);			// �� �ʺ�

			// �� ����/�ʺ� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LEFT_HEIGHT, placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));		// �� ���� (L)
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.areaWidth_Bottom + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1) + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2));		// �� �ʺ�
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_RIGHT_HEIGHT, placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));	// �� ���� (R)

			// ���纰 üũ�ڽ�-�԰� ����
			(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_LSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_LSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_LSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_LSIDE);

			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_BOTTOM)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_BOTTOM);
			(DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_R_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_R_FORM_BOTTOM);

			(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_RSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_RSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_RSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_RSIDE);

			// ������ �� ���
			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE)).ToCStr ()) / 1000.0;
			hRest_Left = placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_LSIDE, hRest_Left);

			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RSIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RSIDE)).ToCStr ()) / 1000.0;
			hRest_Right = placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_RSIDE, hRest_Right);

			// ���� �����ؼ��� �� �Ǵ� �׸� ��ױ�
			DGDisableItem (dialogID, EDITCONTROL_BEAM_LEFT_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_BEAM_RIGHT_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_BEAM_WIDTH);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_LEFT_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_RIGHT_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_WIDTH);
			DGDisableItem (dialogID, EDITCONTROL_REST_LSIDE);
			DGDisableItem (dialogID, EDITCONTROL_REST_RSIDE);

			// ���̾� �ɼ� Ȱ��ȭ/��Ȱ��ȭ
			if ((DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE)) {
				DGEnableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			}

			// ������ ��ũ�� ������� ����
			DGDisableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
			DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);

			break;
		
		case DG_MSG_CHANGE:
			// �⺻ ���������� �������� ������ ��ҵ鵵 ����
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == FALSE) {
				DGSetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE, FALSE);
				DGSetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE, FALSE);
				DGSetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE, FALSE);
			}
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == FALSE) {
				DGSetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE, FALSE);
				DGSetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE, FALSE);
				DGSetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE, FALSE);
			}
			if (DGGetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM) == FALSE) {
				DGSetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM, FALSE);
				DGSetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM, FALSE);
			}

			// �� ����/�ʺ� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_LEFT_HEIGHT, placingZone.areaHeight_Left);		// �� ���� (L)
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_RIGHT_HEIGHT, placingZone.areaHeight_Right);		// �� ���� (R)
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH, placingZone.areaWidth_Bottom);			// �� �ʺ�

			// �� ����/�ʺ� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LEFT_HEIGHT, placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));		// �� ���� (L)
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.areaWidth_Bottom + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1) + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2));		// �� �ʺ�
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_RIGHT_HEIGHT, placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));	// �� ���� (R)

			// ���纰 üũ�ڽ�-�԰� ����
			(DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_B_FORM_LSIDE)			:	DGDisableItem (dialogID, POPUP_B_FORM_LSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_LSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_LSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_LSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_LSIDE);

			(DGGetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_L_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_L_FORM_BOTTOM);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_BOTTOM)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_BOTTOM);
			(DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_R_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_R_FORM_BOTTOM);

			(DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_B_FORM_RSIDE)			:	DGDisableItem (dialogID, POPUP_B_FORM_RSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_RSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_RSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_RSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_RSIDE);

			// ������ �� ���
			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE)).ToCStr ()) / 1000.0;
			hRest_Left = placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_LSIDE, hRest_Left);

			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RSIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RSIDE)).ToCStr ()) / 1000.0;
			hRest_Right = placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_RSIDE, hRest_Right);

			// ���̾� ���� �ٲ�
			if ((item >= USERCONTROL_LAYER_EUROFORM) && (item <= USERCONTROL_LAYER_BLUE_TIMBER_RAIL)) {
				if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
					long selectedLayer;

					selectedLayer = DGGetItemValLong (dialogID, item);

					for (xx = USERCONTROL_LAYER_EUROFORM ; xx <= USERCONTROL_LAYER_BLUE_TIMBER_RAIL ; ++xx)
						DGSetItemValLong (dialogID, xx, selectedLayer);
				}
			}

			// ���̾� �ɼ� Ȱ��ȭ/��Ȱ��ȭ
			if ((DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE)) {
				DGEnableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					///////////////////////////////////////////////////////////////// ���� ���� (������)
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE) {
						placingZone.bFillBottomFormAtLSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtLSide [0][xx].objType = EUROFORM;
							placingZone.cellsAtLSide [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) {
						placingZone.bFillFillerAtLSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtLSide [1][xx].objType = FILLERSP;
							placingZone.cellsAtLSide [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE);
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE) {
						placingZone.bFillTopFormAtLSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtLSide [2][xx].objType = EUROFORM;
							placingZone.cellsAtLSide [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE) {
						placingZone.bFillWoodAtLSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							if (DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE) >= 0.100 + EPS)
								placingZone.cellsAtLSide [3][xx].objType = PLYWOOD;
							else
								placingZone.cellsAtLSide [3][xx].objType = TIMBER;
							placingZone.cellsAtLSide [3][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE);
						}
					}

					///////////////////////////////////////////////////////////////// ������ ���� (������)
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE) {
						placingZone.bFillBottomFormAtRSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtRSide [0][xx].objType = EUROFORM;
							placingZone.cellsAtRSide [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RSIDE)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) {
						placingZone.bFillFillerAtRSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtRSide [1][xx].objType = FILLERSP;
							placingZone.cellsAtRSide [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE);
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE) {
						placingZone.bFillTopFormAtRSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtRSide [2][xx].objType = EUROFORM;
							placingZone.cellsAtRSide [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RSIDE)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE) {
						placingZone.bFillWoodAtRSide = true;
						for (xx = 0 ; xx < 50 ; ++xx) {
							if (DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE) >= 0.100 + EPS)
								placingZone.cellsAtRSide [3][xx].objType = PLYWOOD;
							else
								placingZone.cellsAtRSide [3][xx].objType = TIMBER;
							placingZone.cellsAtRSide [3][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE);
						}
					}

					///////////////////////////////////////////////////////////////// �Ϻ�
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

					// ������ �� ���
					h1 = 0;
					h2 = 0;
					h3 = 0;
					h4 = 0;
					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE)).ToCStr ()) / 1000.0;
					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE)).ToCStr ()) / 1000.0;
					hRest_Left = placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
					DGSetItemValDouble (dialogID, EDITCONTROL_REST_LSIDE, hRest_Left);

					h1 = 0;
					h2 = 0;
					h3 = 0;
					h4 = 0;
					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RSIDE)).ToCStr ()) / 1000.0;
					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RSIDE)).ToCStr ()) / 1000.0;
					hRest_Right = placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
					DGSetItemValDouble (dialogID, EDITCONTROL_REST_RSIDE, hRest_Right);

					// ������ ����
					placingZone.hRest_Left = hRest_Left;
					placingZone.hRest_Right = hRest_Right;

					// ������ ����
					placingZone.gapSideLeft = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1);
					placingZone.gapSideRight = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2);
					placingZone.gapBottom = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM);

					///////////////////////////////////////////////////////////////// �� �糡
					placingZone.beginCellAtLSide.perLen = placingZone.areaHeight_Left + placingZone.gapBottom;
					placingZone.beginCellAtRSide.perLen = placingZone.areaHeight_Right + placingZone.gapBottom;
					placingZone.beginCellAtBottom.perLen = placingZone.areaWidth_Bottom + placingZone.gapSideLeft + placingZone.gapSideRight;

					placingZone.endCellAtLSide.perLen = placingZone.areaHeight_Left + placingZone.gapBottom;
					placingZone.endCellAtRSide.perLen = placingZone.areaHeight_Right + placingZone.gapBottom;
					placingZone.endCellAtBottom.perLen = placingZone.areaWidth_Bottom + placingZone.gapSideLeft + placingZone.gapSideRight;

					// ���̾� ��ȣ ����
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
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, layerInd_EuroformHook);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);

					break;

				case DG_CANCEL:
					break;

				case BUTTON_COPY_TO_RIGHT:
					item = 0;

					// üũ�ڽ� ���� ����
					DGSetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE, DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE));
					DGSetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE, DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE));
					DGSetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE, DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE));
					DGSetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE, DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE));

					// �˾� ��Ʈ�� �� Edit��Ʈ�� �� ����
					DGPopUpSelectItem (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE));
					DGSetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE, DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE));
					DGPopUpSelectItem (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE));
					DGSetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE, DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE));

					// �� ����/�ʺ� ���
					DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_LEFT_HEIGHT, placingZone.areaHeight_Left);		// �� ���� (L)
					DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_RIGHT_HEIGHT, placingZone.areaHeight_Right);		// �� ���� (R)
					DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH, placingZone.areaWidth_Bottom);			// �� �ʺ�

					// �� ����/�ʺ� ���
					DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LEFT_HEIGHT, placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));		// �� ���� (L)
					DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.areaWidth_Bottom + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1) + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2));		// �� �ʺ�
					DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_RIGHT_HEIGHT, placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));	// �� ���� (R)

					// ���纰 üũ�ڽ�-�԰� ����
					(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_LSIDE);
					(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_LSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_LSIDE);
					(DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_LSIDE);

					(DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_BOTTOM)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_BOTTOM);
					(DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_R_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_R_FORM_BOTTOM);

					(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_RSIDE);
					(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_RSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_RSIDE);
					(DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_RSIDE);

					// ������ �� ���
					h1 = 0;
					h2 = 0;
					h3 = 0;
					h4 = 0;
					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE)).ToCStr ()) / 1000.0;
					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE)).ToCStr ()) / 1000.0;
					hRest_Left = placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
					DGSetItemValDouble (dialogID, EDITCONTROL_REST_LSIDE, hRest_Left);

					h1 = 0;
					h2 = 0;
					h3 = 0;
					h4 = 0;
					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RSIDE)).ToCStr ()) / 1000.0;
					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RSIDE)).ToCStr ()) / 1000.0;
					hRest_Right = placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
					DGSetItemValDouble (dialogID, EDITCONTROL_REST_RSIDE, hRest_Right);

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

// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK beamTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	xx;
	short	itmPosX, itmPosY;
	double	lengthDouble;
	const short		maxCol = 50;		// �� �ִ� ����
	static short	dialogSizeX = 500;	// ���� ���̾�α� ũ�� X
	static short	dialogSizeY = 360;	// ���� ���̾�α� ũ�� Y

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"���� ��ġ - �� ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 230, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"Ȯ��");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 270, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"���");
			DGShowItem (dialogID, DG_CANCEL);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 310, 70, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, L"����");
			DGShowItem (dialogID, DG_PREV);

			// ��: �� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 10, 100, 23);
			DGSetItemFont (dialogID, LABEL_BEAM_SIDE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_BEAM_SIDE, L"�� ����");
			DGShowItem (dialogID, LABEL_BEAM_SIDE);

			// ��: �� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 12, 60, 23);
			DGSetItemFont (dialogID, LABEL_TOTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_TOTAL_LENGTH, L"�� ����");
			DGShowItem (dialogID, LABEL_TOTAL_LENGTH);

			// Edit��Ʈ��: �� ����
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 165, 5, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_TOTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_TOTAL_LENGTH);

			// ��: ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 230, 12, 60, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_REMAIN_LENGTH, L"���� ����");
			DGShowItem (dialogID, LABEL_REMAIN_LENGTH);

			// Edit��Ʈ��: ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 300, 5, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_LENGTH);

			// ��: ���̺��� Ÿ��
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 420, 12, 80, 23);
			DGSetItemFont (dialogID, LABEL_TABLEFORM_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_TABLEFORM_TYPE, L"���̺��� Ÿ��");
			DGShowItem (dialogID, LABEL_TABLEFORM_TYPE);

			// �˾���Ʈ��: ���̺��� Ÿ��
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 505, 7, 70, 23);
			DGSetItemFont (dialogID, POPUP_TABLEFORM_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, L"Ÿ��A");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, L"Ÿ��B");
			DGPopUpSelectItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_TOP);
			DGShowItem (dialogID, POPUP_TABLEFORM_TYPE);

			// ��ư: �߰�
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 70, 70, 25);
			DGSetItemFont (dialogID, BUTTON_ADD_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ADD_COL, L"�߰�");
			DGShowItem (dialogID, BUTTON_ADD_COL);

			// ��ư: ����
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 105, 70, 25);
			DGSetItemFont (dialogID, BUTTON_DEL_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DEL_COL, L"����");
			DGShowItem (dialogID, BUTTON_DEL_COL);

			// ���� �� ���� ä��� ���� (üũ�ڽ�)
			placingZone.CHECKBOX_MARGIN_LEFT_END = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 120, 70, 70, 70);
			DGSetItemFont (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, L"����\nä���");
			DGShowItem (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END);
			DGSetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, TRUE);
			// ���� �� ���� ���� (Edit��Ʈ��)
			placingZone.EDITCONTROL_MARGIN_LEFT_END = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 120, 140, 70, 25);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
			DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END, 0.090);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END, 2.440);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END, 0.200);

			// �Ϲ� ��: �⺻���� ������
			itmPosX = 120+70;
			itmPosY = 72;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				// ��ư
				placingZone.BUTTON_OBJ [xx] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
				DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], L"������");
				DGShowItem (dialogID, placingZone.BUTTON_OBJ [xx]);
				DGDisableItem (dialogID, placingZone.BUTTON_OBJ [xx]);

				// ��ü Ÿ�� (�˾���Ʈ��)
				placingZone.POPUP_OBJ_TYPE [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_IS_EXTRASMALL | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, L"����");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, L"������");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, L"����");
				DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_TOP+1);
				DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

				// �ʺ� (�˾���Ʈ��)
				placingZone.POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1200");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "900");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "600");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "0");
				DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
				DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);

				// �ʺ� (Edit��Ʈ����Ʈ��) - ó������ ����
				placingZone.EDITCONTROL_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
				DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
				DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.090);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 2.440);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.200);

				itmPosX += 70;
			}

			// ������ �� ���� ä��� ���� (üũ�ڽ�)
			placingZone.CHECKBOX_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 70, 70, 70);
			DGSetItemFont (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, L"����\nä���");
			DGShowItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
			DGSetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, TRUE);
			// ������ �� ���� ���� (Edit��Ʈ��)
			placingZone.EDITCONTROL_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 140, 70, 25);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
			DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.090);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 2.440);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.200);

			// �� ����, ���� ���� ǥ��
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LENGTH, placingZone.beamLength);
			lengthDouble = placingZone.beamLength;
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1)
					lengthDouble -= atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000.0;
				else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD + 1)
					lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, lengthDouble);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_LENGTH);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_LENGTH);

			// ���̾�α� ũ�� ����
			dialogSizeX = 500;
			dialogSizeY = 360;
			if (placingZone.nCells >= 4) {
				DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCells - 3), dialogSizeY, DG_TOPLEFT, true);
			}

			break;

		case DG_MSG_CHANGE:

			// ���� ä��� ��ư üũ ����
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)
				DGEnableItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
			else
				DGDisableItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);

			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)
				DGEnableItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
			else
				DGDisableItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);

			// ��ü Ÿ�� �����
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if (item == placingZone.POPUP_OBJ_TYPE [xx]) {
					// �ش� ��ư�� �̸� ����
					DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], DGPopUpGetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx])));

					// �����̸� ��� ����, �������̸� �˾� ǥ��, �����̸� Edit��Ʈ�� ǥ��
					if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE + 1) {
						DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);
						DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
					} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1) {
						DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);
						DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
					} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD + 1) {
						DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);
						DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
					}
				}
			}

			// ���� ���� ǥ��
			lengthDouble = placingZone.beamLength;
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1)
					lengthDouble -= atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000.0;
				else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD + 1)
					lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, lengthDouble);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_PREV:
					clickedPrevButton = true;
					break;

				case DG_OK:
					clickedOKButton = true;

					// ���̺��� Ÿ��
					placingZone.tableformType = DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_TYPE);

					// ���� ä��� ���� ����
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

					// �� ���� ����
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
					}

					break;

				case DG_CANCEL:
					break;

				case BUTTON_ADD_COL:
					item = 0;

					if (placingZone.nCells < maxCol) {
						// ������ �� ���� ä��� ��ư�� �����
						DGRemoveDialogItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
						DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);

						// ������ �� ��ư �����ʿ� ���ο� �� ��ư�� �߰��ϰ�
						itmPosX = 120+70 + (70 * placingZone.nCells);
						itmPosY = 72;
						// ��ư
						placingZone.BUTTON_OBJ [placingZone.nCells] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
						DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells], DG_IS_LARGE | DG_IS_PLAIN);
						DGSetItemText (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells], L"������");
						DGShowItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells]);
						DGDisableItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells]);

						// ��ü Ÿ�� (�˾���Ʈ��)
						placingZone.POPUP_OBJ_TYPE [placingZone.nCells] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
						DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_IS_EXTRASMALL | DG_IS_PLAIN);
						DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM, L"����");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM, L"������");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM, L"����");
						DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_TOP+1);
						DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells]);

						// �ʺ� (�˾���Ʈ��)
						placingZone.POPUP_WIDTH [placingZone.nCells] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
						DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_IS_LARGE | DG_IS_PLAIN);
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM, "1200");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM, "900");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM, "600");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM, "0");
						DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_TOP);
						DGShowItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells]);
						DGSetItemMinDouble (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], 0.090);
						DGSetItemMaxDouble (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], 2.440);

						// �ʺ� (Edit��Ʈ��) - ó������ ����
						placingZone.EDITCONTROL_WIDTH [placingZone.nCells] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
						DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [placingZone.nCells], 0.200);

						itmPosX += 70;

						// ������ �� ���� ä��� ��ư�� ������ ���� ����
						// ������ �� ���� ä��� ���� (üũ��ư)
						placingZone.CHECKBOX_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 70, 70, 70);
						DGSetItemFont (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, DG_IS_LARGE | DG_IS_PLAIN);
						DGSetItemText (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, L"����\nä���");
						DGShowItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
						DGSetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, TRUE);
						// ������ �� ���� ���� (Edit��Ʈ��)
						placingZone.EDITCONTROL_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 140, 70, 25);
						DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.090);
						DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 2.440);
						DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.200);

						++placingZone.nCells;
						
						// ���� ���� ǥ��
						lengthDouble = placingZone.beamLength;
						if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
						if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
							if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1)
								lengthDouble -= atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000.0;
							else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD + 1)
								lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
						}
						DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, lengthDouble);

						// ���̾�α� ũ�� ����
						dialogSizeX = 500;
						dialogSizeY = 360;
						if (placingZone.nCells >= 4) {
							DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCells - 3), dialogSizeY, DG_TOPLEFT, true);
						}
					}

					break;

				case BUTTON_DEL_COL:
					item = 0;

					if (placingZone.nCells > 1) {
						// ������ �� ���� ä��� ��ư�� �����
						DGRemoveDialogItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
						DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);

						// ������ �� ��ư�� �����
						DGRemoveDialogItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells - 1]);
						DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells - 1]);
						DGRemoveDialogItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells - 1]);
						DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH [placingZone.nCells - 1]);

						// ������ �� ���� ä��� ��ư�� ������ ���� ����
						itmPosX = 120+70 + (70 * (placingZone.nCells - 1));
						itmPosY = 72;
						// ������ �� ���� ä��� ���� (üũ��ư)
						placingZone.CHECKBOX_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 70, 70, 70);
						DGSetItemFont (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, DG_IS_LARGE | DG_IS_PLAIN);
						DGSetItemText (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, L"����\nä���");
						DGShowItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
						DGSetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, TRUE);
						// ������ �� ���� ���� (Edit��Ʈ��)
						placingZone.EDITCONTROL_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 140, 70, 25);
						DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.200);

						--placingZone.nCells;

						// ���� ���� ǥ��
						lengthDouble = placingZone.beamLength;
						if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
						if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
							if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1)
								lengthDouble -= atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000.0;
							else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD + 1)
								lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
						}
						DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, lengthDouble);

						// ���̾�α� ũ�� ����
						dialogSizeX = 500;
						dialogSizeY = 360;
						if (placingZone.nCells >= 4) {
							DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCells - 3), dialogSizeY, DG_TOPLEFT, true);
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

// ���ٸ�/�ۿ��� �������� ��ġ���� ���θ� ���
short DGCALLBACK beamTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	short		xx;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"���ٸ�/�ۿ��� ������ ��ġ");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// Ȯ�� ��ư
			DGSetItemText (dialogID, DG_OK, L"Ȯ ��");

			// ��� ��ư
			DGSetItemText (dialogID, DG_CANCEL, L"�� ��");

			// ���� ��ư
			DGSetItemText (dialogID, DG_PREV_PERI, L"�� ��");

			// ��
			DGSetItemText (dialogID, LABEL_TYPE_SUPPORTING_POST_PERI, L"Ÿ��");
			DGSetItemText (dialogID, LABEL_NUM_OF_POST_SET_PERI, L"���ٸ� ��Ʈ ����");
			DGSetItemText (dialogID, LABEL_PLAN_VIEW_PERI, L"��鵵");
			DGSetItemText (dialogID, LABEL_BEAM_WIDTH_PERI, L"�� �ʺ�");
			DGSetItemText (dialogID, LABEL_BEAM_LENGTH_PERI, L"�� ����");
			DGSetItemText (dialogID, LABEL_OFFSET_1_PERI, L"���� ��ġ");
			DGSetItemText (dialogID, LABEL_GAP_WIDTH_DIRECTION_1_PERI, L"�ʺ�");
			DGSetItemText (dialogID, LABEL_GAP_LENGTH_DIRECTION_1_PERI, L"����");
			DGSetItemText (dialogID, LABEL_OFFSET_2_PERI, L"���� ��ġ");
			DGSetItemText (dialogID, LABEL_GAP_WIDTH_DIRECTION_2_PERI, L"�ʺ�");
			DGSetItemText (dialogID, LABEL_GAP_LENGTH_DIRECTION_2_PERI, L"����");

			// ��: ���̾� ����
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS_PERI, L"���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_VERTICAL_POST_PERI, L"PERI ������");
			DGSetItemText (dialogID, LABEL_LAYER_HORIZONTAL_POST_PERI, L"PERI ������");
			DGSetItemText (dialogID, LABEL_LAYER_GIRDER_PERI, L"GT24 �Ŵ�");
			DGSetItemText (dialogID, LABEL_LAYER_BEAM_BRACKET_PERI, L"�� �����");
			DGSetItemText (dialogID, LABEL_LAYER_YOKE_PERI, L"�� �ۿ���");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER_PERI, L"����");
			DGSetItemText (dialogID, LABEL_LAYER_JACK_SUPPORT_PERI, L"�� ����Ʈ");

			// üũ�ڽ�: ���̾� ����
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING_PERI, L"���̾� ����");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING_PERI, TRUE);

			DGSetItemText (dialogID, BUTTON_AUTOSET_PERI, L"���̾� �ڵ� ����");

			// ���� ��Ʈ�� �ʱ�ȭ
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

			// �˾� ��Ʈ�� �׸� �߰�
			DGPopUpInsertItem (dialogID, POPUP_TYPE_SUPPORTING_POST_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE_SUPPORTING_POST_PERI, DG_POPUP_BOTTOM, L"PERI ���ٸ� + GT24 �Ŵ�");
			DGPopUpInsertItem (dialogID, POPUP_TYPE_SUPPORTING_POST_PERI, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE_SUPPORTING_POST_PERI, DG_POPUP_BOTTOM, L"PERI ���ٸ� + �� �ۿ���");
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

			// �� �ʺ�/���� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH_PERI, placingZone.areaWidth_Bottom + placingZone.gapSideLeft + placingZone.gapSideRight);	// �� �ʺ�
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_LENGTH_PERI, placingZone.beamLength);															// �� ����

			// ���� �����ؼ��� �� �Ǵ� �׸� ��ױ�
			DGDisableItem (dialogID, EDITCONTROL_BEAM_WIDTH_PERI);
			DGDisableItem (dialogID, EDITCONTROL_BEAM_LENGTH_PERI);
			DGDisableItem (dialogID, EDITCONTROL_OFFSET_2_PERI);
			DGDisableItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI);
			DGDisableItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI);

			// ���̾� �ɼ� Ȱ��ȭ/��Ȱ��ȭ
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
			// ���̾� �ɼ� Ȱ��ȭ/��Ȱ��ȭ
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

			// ���� ��ġ
			DGSetItemValDouble (dialogID, EDITCONTROL_OFFSET_2_PERI, DGGetItemValDouble (dialogID, EDITCONTROL_OFFSET_1_PERI));

			// �ʺ�
			DGPopUpSelectItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI, DGPopUpGetSelected (dialogID, POPUP_GAP_WIDTH_DIRECTION_1_PERI));

			// ����
			DGPopUpSelectItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI, DGPopUpGetSelected (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI));

			// �׸� ���̱�/�����
			if (DGPopUpGetSelected (dialogID, POPUP_NUM_OF_POST_SET_PERI) == 1) {
				// ���ٸ� ��Ʈ ������ 1���� ���
				DGHideItem (dialogID, SEPARATOR_SUPPORTING_POST2_PERI);
				DGHideItem (dialogID, LABEL_OFFSET_2_PERI);
				DGHideItem (dialogID, EDITCONTROL_OFFSET_2_PERI);
				DGHideItem (dialogID, LABEL_GAP_WIDTH_DIRECTION_2_PERI);
				DGHideItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI);
				DGHideItem (dialogID, LABEL_GAP_LENGTH_DIRECTION_2_PERI);
				DGHideItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI);
			} else {
				// ���ٸ� ��Ʈ ������ 2���� ���
				DGShowItem (dialogID, SEPARATOR_SUPPORTING_POST2_PERI);
				DGShowItem (dialogID, LABEL_OFFSET_2_PERI);
				DGShowItem (dialogID, EDITCONTROL_OFFSET_2_PERI);
				DGShowItem (dialogID, LABEL_GAP_WIDTH_DIRECTION_2_PERI);
				DGShowItem (dialogID, POPUP_GAP_WIDTH_DIRECTION_2_PERI);
				DGShowItem (dialogID, LABEL_GAP_LENGTH_DIRECTION_2_PERI);
				DGShowItem (dialogID, POPUP_GAP_LENGTH_DIRECTION_2_PERI);
			}

			// ���̾� ���� �ٲ�
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

					placingZone.typeOfSupportingPost = DGPopUpGetSelected (dialogID, POPUP_TYPE_SUPPORTING_POST_PERI);		// Ÿ��
					placingZone.numOfSupportingPostSet = (short)atoi (DGPopUpGetItemText (dialogID, POPUP_NUM_OF_POST_SET_PERI, DGPopUpGetSelected (dialogID, POPUP_NUM_OF_POST_SET_PERI)).ToCStr ());	// ���ٸ� ��Ʈ ����

					placingZone.postStartOffset = DGGetItemValDouble (dialogID, EDITCONTROL_OFFSET_1_PERI);																										// ���� ��ġ
					placingZone.postGapWidth = atof (DGPopUpGetItemText (dialogID, POPUP_GAP_WIDTH_DIRECTION_1_PERI, DGPopUpGetSelected (dialogID, POPUP_GAP_WIDTH_DIRECTION_1_PERI)).ToCStr ()) / 1000.0;		// �ʺ�
					placingZone.postGapLength = atof (DGPopUpGetItemText (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI, DGPopUpGetSelected (dialogID, POPUP_GAP_LENGTH_DIRECTION_1_PERI)).ToCStr ()) / 1000.0;	// ����

					// ���̾� ��ȣ ����
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

// ���� ���̺��� ���̿� �ܿ��縦 ������ ���θ� ����� ���̾�α�
short DGCALLBACK beamTableformPlacerHandler4_Insulation (short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	std::wstring*	title = (std::wstring*) userData;
	short	result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// Ÿ��Ʋ
			DGSetDialogTitle (dialogID, title->c_str ());

			// ��
			DGSetItemText (dialogID, LABEL_EXPLANATION_INS, L"�ܿ��� �԰��� �Է��Ͻʽÿ�.");
			DGSetItemText (dialogID, LABEL_INSULATION_THK, L"�β�");
			DGSetItemText (dialogID, LABEL_INS_HORLEN, L"����");
			DGSetItemText (dialogID, LABEL_INS_VERLEN, L"����");

			// üũ�ڽ�
			DGSetItemText (dialogID, CHECKBOX_INS_LIMIT_SIZE, L"����/���� ũ�� ����");
			DGSetItemValLong (dialogID, CHECKBOX_INS_LIMIT_SIZE, TRUE);

			// Edit ��Ʈ��
			DGSetItemValDouble (dialogID, EDITCONTROL_INS_HORLEN, 0.900);
			DGSetItemValDouble (dialogID, EDITCONTROL_INS_VERLEN, 1.800);

			// ���̾�
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_INSULATION_LAYER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_INSULATION_LAYER, 1);

			// ��ư
			DGSetItemText (dialogID, DG_OK, L"Ȯ��");
			DGSetItemText (dialogID, DG_CANCEL, L"���");

			// �β��� �ڵ�
			if (title->compare (L"�ܿ��� ��ġ (�� ������)") == 0) {
				DGSetItemValDouble (dialogID, EDITCONTROL_INSULATION_THK, placingZone.gapSideLeft);
			} else if (title->compare (L"�ܿ��� ��ġ (�� ������)") == 0) {
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
					// ���̾� ���� ����
					insulElem.layerInd = (short)DGGetItemValLong (dialogID, USERCONTROL_INSULATION_LAYER);

					// �β�, ����, ���� ����
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