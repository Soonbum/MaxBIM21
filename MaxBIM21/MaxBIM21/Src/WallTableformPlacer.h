#ifndef	__WALL_TABLEFORM_PLACER__
#define __WALL_TABLEFORM_PLACER__

#include "MaxBIM21.h"

namespace namespaceWallTableform {
	enum objType {
		OBJ_NONE,				// 없음 (표시하지 않음)
		OBJ_EUROFORM,			// 유로폼
		OBJ_PLYWOOD,			// 합판
		OBJ_TIMBER,				// 각재
		OBJ_FILLERSPACER,		// 휠러스페이서
		OBJ_WALL_TABLEFORM_A,	// 테이블폼A
		OBJ_WALL_TABLEFORM_B,	// 테이블폼B
		OBJ_WALL_TABLEFORM_C,	// 테이블폼C
		OBJ_INCORNER_PANEL_L,	// 인코너판넬(L)
		OBJ_OUTCORNER_PANEL_L,	// 아웃코너판넬(L)
		OBJ_OUTCORNER_ANGLE_L,	// 아웃코너앵글(L)
		OBJ_INCORNER_PANEL_R,	// 인코너판넬(R)
		OBJ_OUTCORNER_PANEL_R,	// 아웃코너판넬(R)
		OBJ_OUTCORNER_ANGLE_R	// 아웃코너앵글(R)
	};
	GS::UniString objTypeStr[14] = { L"없음", L"유로폼", L"합판", L"각재", L"휠러스페이서", L"테이블폼A", L"테이블폼B", L"테이블폼C", L"인코너판넬(L)", L"아웃코너판넬(L)", L"아웃코너앵글(L)", L"인코너판넬(R)", L"아웃코너판넬(R)", L"아웃코너앵글(R)" };

	enum DG1_itemIndex {
		LABEL_SIDE_SETTINGS = 3,	// 라벨: 배치면
		RADIOBUTTON_DUAL_SIDE,		// 양면
		RADIOBUTTON_SINGLE_SIDE,	// 단면
		LABEL_TABLEFORM_DIRECTION,	// 라벨: 테이블폼 방향
		RADIOBUTTON_VERTICAL,		// 세로 
		RADIOBUTTON_HORIZONTAL,		// 가로
		LABEL_GAP,					// 라벨: 벽과의 간격
		EDITCONTROL_GAP,			// 벽과의 간격
		LABEL_PROPS,				// 라벨: Push-Pull Props
		RADIOBUTTON_PROPS_ON,		// 설치
		RADIOBUTTON_PROPS_OFF,		// 미설치
		LABEL_SHOW_SIDE,			// 라벨: 표시면 선택
		RADIOBUTTON_SHOW_FRONT,		// 앞면
		RADIOBUTTON_SHOW_BACK,		// 뒷면
		PUSHBUTTON_INFO_COPY,		// 셀정보 복사
		LABEL_GUIDE					// 라벨: 안내 텍스트
	};

	enum DG1_branch_itemIndex {
		BRANCH_LABEL_OBJ_TYPE = 3,		// 라벨: 객체 종류
		BRANCH_POPUP_OBJ_TYPE,			// 객체 종류
		BRANCH_LABEL_TOTAL_WIDTH,		// 전체 너비
		BRANCH_EDITCONTROL_TOTAL_WIDTH,	// 전체 너비
		BRANCH_LABEL_GUIDE				// 라벨: 안내 텍스트
	};

	enum DG2_itemIndex {
		DG2_BUTTON_PREV = 3,			// 이전 버튼
		DG2_LABEL_TOTAL_HEIGHT,			// 라벨: 전체 높이
		DG2_EDITCONTROL_TOTAL_HEIGHT,	// 전체 높이
		DG2_LABEL_REMAIN_HEIGHT,		// 라벨: 남은 높이
		DG2_EDITCONTROL_REMAIN_HEIGHT,	// 남은 높이
		DG2_LABEL_GUIDE					// 라벨: 안내 텍스트
	};

	enum layer_itemIndex {
		CHECKBOX_LAYER_COUPLING = 3,

		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_TIMBER,
		LABEL_LAYER_FILLERSPACER,
		LABEL_LAYER_INCORNER_PANEL,
		LABEL_LAYER_OUTCORNER_PANEL,
		LABEL_LAYER_OUTCORNER_ANGLE,
		LABEL_LAYER_BLUE_CLAMP,
		LABEL_LAYER_BLUE_TIMBER_RAIL,
		LABEL_LAYER_RECTPIPE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_HEADPIECE,
		LABEL_LAYER_PROPS,
		LABEL_LAYER_JOIN,
		LABEL_LAYER_CROSS_JOINT_BAR,
		LABEL_LAYER_16,
		LABEL_LAYER_17,
		LABEL_LAYER_18,
		LABEL_LAYER_19,
		LABEL_LAYER_20,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_TIMBER,
		USERCONTROL_LAYER_FILLERSPACER,
		USERCONTROL_LAYER_INCORNER_PANEL,
		USERCONTROL_LAYER_OUTCORNER_PANEL,
		USERCONTROL_LAYER_OUTCORNER_ANGLE,
		USERCONTROL_LAYER_BLUE_CLAMP,
		USERCONTROL_LAYER_BLUE_TIMBER_RAIL,
		USERCONTROL_LAYER_RECTPIPE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_HEADPIECE,
		USERCONTROL_LAYER_PROPS,
		USERCONTROL_LAYER_JOIN,
		USERCONTROL_LAYER_CROSS_JOINT_BAR,
		USERCONTROL_LAYER_16,
		USERCONTROL_LAYER_17,
		USERCONTROL_LAYER_18,
		USERCONTROL_LAYER_19,
		USERCONTROL_LAYER_20,

		BUTTON_AUTOSET
	};

	enum insulationDialog {
		LABEL_EXPLANATION_INS = 3,
		USERCONTROL_INSULATION_LAYER,
		LABEL_INSULATION_THK,
		EDITCONTROL_INSULATION_THK,
		CHECKBOX_INS_LIMIT_SIZE,
		LABEL_INS_HORLEN,
		EDITCONTROL_INS_HORLEN,
		LABEL_INS_VERLEN,
		EDITCONTROL_INS_VERLEN,
	};

	double DEFAULT_TABLEFORM_WIDTH = 2.250;
	double DEFAULT_EUROFORM_HEIGHT = 1.200;

	int MAX_ROW = 10;
	int MAX_COL = 50;

	struct Cell
	{
		double	leftBottomX;	// 좌하단 좌표 X
		double	leftBottomY;	// 좌하단 좌표 Y
		double	leftBottomZ;	// 좌하단 좌표 Z
		double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

		short	objType;		// 객체 타입

		double	horLen;			// 가로 길이
		double	verLen;			// 세로 길이

		// 테이블폼 변수
		int		nCellsHor;		// 수평 방향 셀 개수
		double	cellHorLen[5];	// 셀 길이 (가로)
	};

	struct MarginCell
	{
		double	leftBottomX;	// 좌하단 좌표 X
		double	leftBottomY;	// 좌하단 좌표 Y
		double	leftBottomZ;	// 좌하단 좌표 Z
		double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

		short	objType;		// 객체 타입

		double	horLen;			// 가로 길이
		double	verLen;			// 세로 길이
	};

	class PlacingZone
	{
	public:
		double	leftBottomX;	// 좌하단 좌표 X
		double	leftBottomY;	// 좌하단 좌표 Y
		double	leftBottomZ;	// 좌하단 좌표 Z
		double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

		double	wallThk;		// 벽 두께
		short	floorInd;		// 벽 레이어 인덱스

		double	horLen;			// 가로 길이
		double	verLenBasic;	// 세로 길이 (낮은쪽)
		double	verLenExtra;	// 세로 길이 (높은쪽)

		double	gap;			// 벽과의 간격

		bool	bVertical;		// 폼 방향: 세로방향(true), 가로방향(false)
		bool	bSingleSide;	// 폼 설치면: 단면(true), 양면(false)
		bool	bExtra;			// 추가 영역: 높은쪽 영역 있음(true), 높은쪽 영역 없음(false)
		bool	bInstallProps;	// Push-Pull Props 설치 여부: 설치(true), 미설치(false)

		int		nCellsHor;		// 수평 방향 셀 개수
		int		nCellsVerBasic;	// 수직 방향 셀 개수 (낮은쪽)
		int		nCellsVerExtra;	// 수직 방향 셀 개수 (높은쪽)

		int		nMarginCellsVerBasic;	// 수직 방향 여백 셀 개수 (낮은쪽)
		int		nMarginCellsVerExtra;	// 수직 방향 여백 셀 개수 (높은쪽)

		Cell	cellsBasic[10][50];				// 셀 배열 (낮은쪽)
		Cell	cellsExtra[10][50];				// 셀 배열 (높은쪽)
		MarginCell	marginCellsBasic[10][50];	// 여백 셀 배열 (낮은쪽)
		MarginCell	marginCellsExtra[10][50];	// 여백 셀 배열 (높은쪽)

		double	marginTopBasic;	// 상단 여백 (낮은쪽)
		double	marginTopExtra;	// 상단 여백 (높은쪽)

		// 테이블폼 세로 방향 너비 모음
		int		tableformPresetVertical[40] = { 2300, 2250, 2200, 2150, 2100, 2050, 2000, 1950, 1900, 1850, 1800, 1750, 1700, 1650, 1600, 1550, \
												1500, 1450, 1400, 1350, 1300, 1250, 1200, 1150, 1100, 1050, 1000, 950, 900, 850, 800, 750, \
												700, 650, 600, 500, 450, 400, 300, 200 };
		// 테이블폼 가로 방향 너비 모음
		int		tableformPresetHorizontal[16] = { 6000, 5700, 5400, 5100, 4800, 4500, 4200, 3900, 3600, 3300, 3000, 2700, 2400, 2100, 1800, 1500 };
		// 유로폼 세로 방향 너비 모음
		int		euroformPresetVertical[6] = { 600, 500, 450, 400, 300, 200 };
		// 유로폼 가로 방향 너비 모음
		int		euroformPresetHorizontal[3] = { 1200, 900, 600 };
		
		// 테이블폼 세로 방향 너비 구성
		int		tableformPresetVerticalConfig[40][6] = {
			{ 2300, 4, 600, 600, 500, 600 },
			{ 2250, 4, 600, 600, 450, 600 },
			{ 2200, 4, 600, 600, 400, 600 },
			{ 2150, 4, 600, 500, 450, 600 },
			{ 2100, 4, 600, 600, 300, 600 },
			{ 2050, 4, 600, 450, 400, 600 },
			{ 2000, 4, 600, 600, 200, 600 },
			{ 1950, 4, 600, 450, 300, 600 },
			{ 1900, 4, 600, 500, 200, 600 },
			{ 1850, 4, 600, 450, 200, 600 },
			{ 1800, 3, 600, 600, 600,   0 },
			{ 1750, 4, 600, 200, 450, 500 },
			{ 1700, 3, 600, 500, 600,   0 },
			{ 1650, 3, 600, 450, 600,   0 },
			{ 1600, 3, 600, 400, 600,   0 },
			{ 1550, 3, 600, 450, 500,   0 },
			{ 1500, 3, 600, 300, 600,   0 },
			{ 1450, 3, 500, 450, 500,   0 },
			{ 1400, 3, 500, 400, 500,   0 },
			{ 1350, 3, 600, 300, 450,   0 },
			{ 1300, 3, 600, 200, 500,   0 },
			{ 1250, 3, 600, 200, 450,   0 },
			{ 1200, 2, 600, 600,   0,   0 },
			{ 1150, 3, 450, 300, 400,   0 },
			{ 1100, 3, 400, 300, 400,   0 },
			{ 1050, 3, 450, 300, 300,   0 },
			{ 1000, 2, 600, 400,   0,   0 },
			{  950, 2, 450, 500,   0,   0 },
			{  900, 2, 600, 300,   0,   0 },
			{  850, 2, 400, 450,   0,   0 },
			{  800, 2, 400, 400,   0,   0 },
			{  750, 2, 450, 300,   0,   0 },
			{  700, 2, 400, 300,   0,   0 },
			{  650, 2, 450, 200,   0,   0 },
			{  600, 1, 600,   0,   0,   0 },
			{  500, 1, 500,   0,   0,   0 },
			{  450, 1, 450,   0,   0,   0 },
			{  400, 1, 400,   0,   0,   0 },
			{  300, 1, 300,   0,   0,   0 },
			{  200, 1, 200,   0,   0,   0 }
		};
		// 테이블폼 가로 방향 너비 구성
		int		tableformPresetHorizontalConfig[16][7] = {
			{ 6000, 5, 1200, 1200, 1200, 1200, 1200 },
			{ 5700, 5, 1200, 1200, 1200, 1200,  900 },
			{ 5400, 5, 1200, 1200, 1200,  900,  900 },
			{ 5100, 5, 1200, 1200, 1200,  900,  600 },
			{ 4800, 4, 1200, 1200, 1200, 1200,    0 },
			{ 4500, 4, 1200, 1200, 1200,  900,    0 },
			{ 4200, 4, 1200, 1200,  900,  900,    0 },
			{ 3900, 4, 1200, 1200,  900,  600,    0 },
			{ 3600, 3, 1200, 1200, 1200,    0,    0 },
			{ 3300, 3, 1200, 1200,  900,    0,    0 },
			{ 3000, 3, 1200, 1200,  600,    0,    0 },
			{ 2700, 3, 1200,  900,  600,    0,    0 },
			{ 2400, 2, 1200, 1200,    0,    0,    0 },
			{ 2100, 2, 1200,  900,    0,    0,    0 },
			{ 1800, 2,  900,  900,    0,    0,    0 },
			{ 1500, 2,  900,  600,    0,    0,    0 }
		};

		// 레이어 지정 변수
		short	layerInd_Euroform;			// 유로폼
		short	layerInd_Plywood;			// 합판
		short	layerInd_Timber;			// 각재
		short	layerInd_Fillerspacer;		// 휠러스페이서
		short	layerInd_IncornerPanel;		// 인코너판넬
		short	layerInd_OutcornerPanel;	// 아웃코너판넬
		short	layerInd_OutcornerAngle;	// 아웃코너앵글
		short	layerInd_BlueClamp;			// 블루클램프
		short	layerInd_BlueTimberRail;	// 블루목심
		short	layerInd_Rectpipe;			// 각파이프
		short	layerInd_Pinbolt;			// 핀볼트
		short	layerInd_Headpiece;			// 헤드피스
		short	layerInd_Props;				// 푸시풀프롭스
		short	layerInd_Join;				// 결합철물
		short	layerInd_CrossJointBar;		// 십자조인트바

		bool	bLayerInd_Euroform;			// 유로폼
		bool	bLayerInd_Plywood;			// 합판
		bool	bLayerInd_Timber;			// 각재
		bool	bLayerInd_Fillerspacer;		// 휠러스페이서
		bool	bLayerInd_IncornerPanel;	// 인코너판넬
		bool	bLayerInd_OutcornerPanel;	// 아웃코너판넬
		bool	bLayerInd_OutcornerAngle;	// 아웃코너앵글
		bool	bLayerInd_BlueClamp;		// 블루클램프
		bool	bLayerInd_BlueTimberRail;	// 블루목심
		bool	bLayerInd_Rectpipe;			// 각파이프
		bool	bLayerInd_Pinbolt;			// 핀볼트
		bool	bLayerInd_Headpiece;		// 헤드피스
		bool	bLayerInd_Props;			// 푸시풀프롭스
		bool	bLayerInd_Join;				// 결합철물
		bool	bLayerInd_CrossJointBar;	// 십자조인트바
		
		// 생성자
		PlacingZone()
		{
			this->leftBottomX = 0.0;
			this->leftBottomY = 0.0;
			this->leftBottomZ = 0.0;
			this->ang = 0.0;

			this->wallThk = 0.0;
			this->floorInd = 0;

			this->horLen = 0.0;
			this->verLenBasic = 0.0;
			this->verLenExtra = 0.0;

			this->gap = 0.0;

			this->bVertical = true;
			this->bSingleSide = false;
			this->bExtra = false;
			this->bInstallProps = true;

			this->nCellsHor = 0;
			this->nCellsVerBasic = 0;
			this->nCellsVerExtra = 0;

			this->nMarginCellsVerBasic = 0;
			this->nMarginCellsVerExtra = 0;

			this->marginTopBasic = 0.0;
			this->marginTopExtra = 0.0;

			for (int i = 0; i < 10; i++) {
				for (int j = 0; j < 50; j++) {
					this->cellsBasic[i][j].leftBottomX = 0.0;
					this->cellsBasic[i][j].leftBottomY = 0.0;
					this->cellsBasic[i][j].leftBottomZ = 0.0;
					this->cellsBasic[i][j].ang = 0.0;
					this->cellsBasic[i][j].horLen = 0.0;
					this->cellsBasic[i][j].verLen = 0.0;
					this->cellsBasic[i][j].objType = OBJ_NONE;
					this->cellsBasic[i][j].nCellsHor = 0;
					for (int m = 0; m < 10; m++) {
						this->cellsBasic[i][j].cellHorLen[m] = 0.0;
					}

					this->cellsExtra[i][j].leftBottomX = 0.0;
					this->cellsExtra[i][j].leftBottomY = 0.0;
					this->cellsExtra[i][j].leftBottomZ = 0.0;
					this->cellsExtra[i][j].ang = 0.0;
					this->cellsExtra[i][j].horLen = 0.0;
					this->cellsExtra[i][j].verLen = 0.0;
					this->cellsExtra[i][j].objType = OBJ_NONE;
					this->cellsExtra[i][j].nCellsHor = 0;
					for (int m = 0; m < 10; m++) {
						this->cellsExtra[i][j].cellHorLen[m] = 0.0;
					}

					this->marginCellsBasic[i][j].leftBottomX = 0.0;
					this->marginCellsBasic[i][j].leftBottomY = 0.0;
					this->marginCellsBasic[i][j].leftBottomZ = 0.0;
					this->marginCellsBasic[i][j].ang = 0.0;
					this->marginCellsBasic[i][j].horLen = 0.0;
					this->marginCellsBasic[i][j].verLen = 0.0;
					this->marginCellsBasic[i][j].objType = OBJ_NONE;

					this->marginCellsExtra[i][j].leftBottomX = 0.0;
					this->marginCellsExtra[i][j].leftBottomY = 0.0;
					this->marginCellsExtra[i][j].leftBottomZ = 0.0;
					this->marginCellsExtra[i][j].ang = 0.0;
					this->marginCellsExtra[i][j].horLen = 0.0;
					this->marginCellsExtra[i][j].verLen = 0.0;
					this->marginCellsExtra[i][j].objType = OBJ_NONE;
				}
			}

			this->bLayerInd_Euroform = false;
			this->bLayerInd_Plywood = false;
			this->bLayerInd_Timber = false;
			this->bLayerInd_Fillerspacer = false;
			this->bLayerInd_IncornerPanel = false;
			this->bLayerInd_OutcornerPanel = false;
			this->bLayerInd_OutcornerAngle = false;
			this->bLayerInd_BlueClamp = false;
			this->bLayerInd_BlueTimberRail = false;
			this->bLayerInd_Rectpipe = false;
			this->bLayerInd_Pinbolt = false;
			this->bLayerInd_Headpiece = false;
			this->bLayerInd_Props = false;
			this->bLayerInd_Join = false;
			this->bLayerInd_CrossJointBar = false;

			this->layerInd_Euroform = 1;
			this->layerInd_Plywood = 1;
			this->layerInd_Timber = 1;
			this->layerInd_Fillerspacer = 1;
			this->layerInd_IncornerPanel = 1;
			this->layerInd_OutcornerPanel = 1;
			this->layerInd_OutcornerAngle = 1;
			this->layerInd_BlueClamp = 1;
			this->layerInd_BlueTimberRail = 1;
			this->layerInd_Rectpipe = 1;
			this->layerInd_Pinbolt = 1;
			this->layerInd_Headpiece = 1;
			this->layerInd_Props = 1;
			this->layerInd_Join = 1;
			this->layerInd_CrossJointBar = 1;
		}

		// 배치 정보 구축
		void initCells()
		{
			// 모든 셀을 테이블폼A로 초기화
			for (int i = 0; i < 10; i++) {
				for (int j = 0; j < 50; j++) {
					// 앞면
					this->cellsBasic[i][j].horLen = DEFAULT_TABLEFORM_WIDTH;
					this->cellsBasic[i][j].verLen = DEFAULT_EUROFORM_HEIGHT;
					this->cellsBasic[i][j].objType = OBJ_WALL_TABLEFORM_A;
					this->cellsBasic[i][j].nCellsHor = 4;

					this->cellsBasic[i][j].cellHorLen[0] = 0.600;
					this->cellsBasic[i][j].cellHorLen[1] = 0.600;
					this->cellsBasic[i][j].cellHorLen[2] = 0.450;
					this->cellsBasic[i][j].cellHorLen[3] = 0.600;

					// 뒷면
					this->cellsExtra[i][j].horLen = DEFAULT_TABLEFORM_WIDTH;
					this->cellsExtra[i][j].verLen = DEFAULT_EUROFORM_HEIGHT;
					this->cellsExtra[i][j].objType = OBJ_WALL_TABLEFORM_A;
					this->cellsExtra[i][j].nCellsHor = 4;

					this->cellsExtra[i][j].cellHorLen[0] = 0.600;
					this->cellsExtra[i][j].cellHorLen[1] = 0.600;
					this->cellsExtra[i][j].cellHorLen[2] = 0.450;
					this->cellsExtra[i][j].cellHorLen[3] = 0.600;

					// 앞면 여백
					this->marginCellsBasic[i][j].objType = OBJ_EUROFORM;
					this->marginCellsBasic[i][j].horLen = DEFAULT_TABLEFORM_WIDTH;
					this->marginCellsBasic[i][j].verLen = 0.600;

					// 뒷면 여백
					this->marginCellsExtra[i][j].objType = OBJ_EUROFORM;
					this->marginCellsExtra[i][j].horLen = DEFAULT_TABLEFORM_WIDTH;
					this->marginCellsExtra[i][j].verLen = 0.600;
				}
			}
		}

		// 특정 셀의 원점 X 좌표를 알아냄
		double getCellPositionLeftBottomX(int index) {
			double distance = 0.0;

			for (int i = 0; i < index; i++)
				distance += this->cellsBasic[0][i].horLen;

			return distance;
		}

		// 배치 정보 위치 정렬
		void alignPositions() {
			moveIn2D('y', this->ang, -this->gap, &this->leftBottomX, &this->leftBottomY);

			for (int i = 0; i < this->nCellsVerBasic; i++) {
				for (int j = 0; j < this->nCellsHor; j++) {
					this->cellsBasic[i][j].ang = this->ang;
					this->cellsBasic[i][j].leftBottomX = this->leftBottomX + (this->gap * sin(this->ang)) + (this->getCellPositionLeftBottomX(j) * cos(this->ang));
					this->cellsBasic[i][j].leftBottomY = this->leftBottomY - (this->gap * cos(this->ang)) + (this->getCellPositionLeftBottomX(j) * sin(this->ang));
					if (i > 0) {
						this->cellsBasic[i][j].leftBottomZ = this->cellsBasic[i - 1][j].leftBottomZ + this->cellsBasic[i - 1][j].verLen;
					}
					else {
						this->cellsBasic[i][j].leftBottomZ = this->leftBottomZ;
					}
				}
			}

			for (int i = 0; i < this->nCellsVerExtra; i++) {
				for (int j = 0; j < this->nCellsHor; j++) {
					this->cellsExtra[i][j].ang = this->ang;
					this->cellsExtra[i][j].leftBottomX = this->leftBottomX + (this->gap * sin(this->ang)) + (this->getCellPositionLeftBottomX(j) * cos(this->ang));
					this->cellsExtra[i][j].leftBottomY = this->leftBottomY - (this->gap * cos(this->ang)) + (this->getCellPositionLeftBottomX(j) * sin(this->ang));
					if (i > 0) {
						this->cellsExtra[i][j].leftBottomZ = this->cellsExtra[i - 1][j].leftBottomZ + this->cellsExtra[i - 1][j].verLen;
					}
					else {
						this->cellsExtra[i][j].leftBottomZ = this->leftBottomZ;
					}
					moveIn2D('x', this->ang, this->cellsExtra[i][j].leftBottomX, &this->cellsExtra[i][j].leftBottomX, &this->cellsExtra[i][j].leftBottomY);
					moveIn2D('y', this->ang, this->wallThk + this->gap * 2, &this->cellsExtra[i][j].leftBottomX, &this->cellsExtra[i][j].leftBottomY);
				}
			}

			for (int i = 0; i < this->nMarginCellsVerBasic; i++) {
				for (int j = 0; j < this->nCellsHor; j++) {
					this->marginCellsBasic[i][j].ang = this->ang;
					this->marginCellsBasic[i][j].leftBottomX = this->leftBottomX + (this->gap * sin(this->ang)) + (this->getCellPositionLeftBottomX(j) * cos(this->ang));
					this->marginCellsBasic[i][j].leftBottomY = this->leftBottomY - (this->gap * cos(this->ang)) + (this->getCellPositionLeftBottomX(j) * sin(this->ang));
					if (i > 0) {
						this->marginCellsBasic[i][j].leftBottomZ = this->marginCellsBasic[i - 1][j].leftBottomZ + this->marginCellsBasic[i - 1][j].verLen;
					}
					else {
						this->marginCellsBasic[i][j].leftBottomZ = this->cellsBasic[nCellsVerBasic - 1][j].leftBottomZ + this->cellsBasic[nCellsVerBasic - 1][j].verLen;
					}
				}
			}

			for (int i = 0; i < this->nMarginCellsVerExtra; i++) {
				for (int j = 0; j < this->nCellsHor; j++) {
					this->marginCellsExtra[i][j].ang = this->ang;
					this->marginCellsExtra[i][j].leftBottomX = this->leftBottomX + (this->gap * sin(this->ang)) + (this->getCellPositionLeftBottomX(j) * cos(this->ang));
					this->marginCellsExtra[i][j].leftBottomY = this->leftBottomY - (this->gap * cos(this->ang)) + (this->getCellPositionLeftBottomX(j) * sin(this->ang));
					if (i > 0) {
						this->marginCellsExtra[i][j].leftBottomZ = this->marginCellsExtra[i - 1][j].leftBottomZ + this->marginCellsExtra[i - 1][j].verLen;
					}
					else {
						this->marginCellsExtra[i][j].leftBottomZ = this->cellsExtra[nCellsVerExtra - 1][j].leftBottomZ + this->cellsExtra[nCellsVerExtra - 1][j].verLen;
					}
					moveIn2D('x', this->ang, this->marginCellsExtra[i][j].leftBottomX, &this->marginCellsExtra[i][j].leftBottomX, &this->marginCellsExtra[i][j].leftBottomY);
					moveIn2D('y', this->ang, this->wallThk + this->gap * 2, &this->marginCellsExtra[i][j].leftBottomX, &this->marginCellsExtra[i][j].leftBottomY);
				}
			}
		}

		// 유로폼 배치
		API_Guid placeEuroform(double leftBottomX, double leftBottomY, double leftBottomZ, double radAng, double horLen, double verLen, bool bVertical, bool bForCeil) {
			API_Guid guid;
			EasyObjectPlacement euroform;

			euroform.init(L"유로폼v2.0.gsm", layerInd_Euroform, this->floorInd, leftBottomX, leftBottomY, leftBottomZ, radAng);

			// 규격폼 여부 확인
			bool	bStandard = false;
			if (((abs(horLen - 0.600) < EPS) || (abs(horLen - 0.500) < EPS) || (abs(horLen - 0.450) < EPS) || (abs(horLen - 0.400) < EPS) || (abs(horLen - 0.300) < EPS) || (abs(horLen - 0.200) < EPS)) &&
				((abs(verLen - 1.200) < EPS) || (abs(verLen - 0.900) < EPS) || (abs(verLen - 0.600) < EPS)))
				bStandard = true;

			// 유로폼이 세로 방향인지, 가로 방향인지?
			char insDir[20];
			double xOffset = 0.0;

			if (bVertical == true) {
				strcpy(insDir, "벽세우기");
				xOffset = 0.0;
			}
			else {
				strcpy(insDir, "벽눕히기");
				xOffset = verLen;
			}
			
			// 벽에 붙이나? 천장에 붙이나?
			double angX;
			double yOffset = 0.0;

			if (bForCeil == true) {
				angX = 0.0;
				if (bVertical == true)
					yOffset = verLen;
				else
					yOffset = horLen;
			}
			else {
				angX = DegreeToRad(90.0);
				yOffset = 0.0;
			}

			if (bStandard == true) {
				moveIn3D('x', euroform.radAng, xOffset, &euroform.posX, &euroform.posY, &euroform.posZ);
				moveIn3D('y', euroform.radAng, yOffset, &euroform.posX, &euroform.posY, &euroform.posZ);
				guid = euroform.placeObject(6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string("%d", (int)(horLen * 1000)).c_str(),
					"eu_hei", APIParT_CString, format_string("%d", (int)(verLen * 1000)).c_str(),
					"u_ins", APIParT_CString, insDir,
					"ang_x", APIParT_Angle, format_string("%f", angX).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0)).c_str());
			}
			else {
				moveIn3D('x', euroform.radAng, xOffset, &euroform.posX, &euroform.posY, &euroform.posZ);
				moveIn3D('y', euroform.radAng, yOffset, &euroform.posX, &euroform.posY, &euroform.posZ);
				guid = euroform.placeObject(6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string("%f", horLen).c_str(),
					"eu_hei2", APIParT_Length, format_string("%f", verLen).c_str(),
					"u_ins", APIParT_CString, insDir,
					"ang_x", APIParT_Angle, format_string("%f", angX).c_str(),
					"ang_y", APIParT_Angle, format_string("%f", DegreeToRad(90.0)).c_str());
			}

			return guid;
		}

		// 합판 배치
		API_Guid placePlywood(double leftBottomX, double leftBottomY, double leftBottomZ, double radAng, double horLen, double verLen, bool bVertical, bool bForCeil) {
			API_Guid guid;
			EasyObjectPlacement plywood;

			plywood.init(L"합판v1.0.gsm", layerInd_Plywood, this->floorInd, leftBottomX, leftBottomY, leftBottomZ, radAng);

			char insDir[20];

			if (bVertical == true) {
				strcpy(insDir, "벽세우기");
			}
			else {
				strcpy(insDir, "벽눕히기");
			}

			if (bForCeil == true) {
				if (bVertical == true) {
					strcpy(insDir, "바닥깔기");
					plywood.radAng += DegreeToRad(90.0);
					moveIn3D('y', plywood.radAng, -horLen, &plywood.posX, &plywood.posY, &plywood.posZ);
				}
				else {
					strcpy(insDir, "바닥깔기");
				}
			}

			guid = plywood.placeObject(23,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, insDir,
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string("%f", horLen).c_str(),
				"p_leng", APIParT_Length, format_string("%f", verLen).c_str(),
				"p_ang", APIParT_Angle, format_string("%f", 0.0).c_str(),
				"p_ang_alter", APIParT_Angle, format_string("%f", 0.0).c_str(),
				"p_rot", APIParT_Angle, format_string("%f", 0.0).c_str(),
				"bCornerCutLT", APIParT_Boolean, "0.0",
				"bCornerCutLB", APIParT_Boolean, "0.0",
				"bCornerCutRT", APIParT_Boolean, "0.0",
				"bCornerCutRB", APIParT_Boolean, "0.0",
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string("%f", 0.0).c_str(),
				"gap_b", APIParT_Length, format_string("%f", 0.0).c_str(),
				"gap_c", APIParT_Length, format_string("%f", 0.0).c_str(),
				"gap_d", APIParT_Length, format_string("%f", 0.0).c_str(),
				"bTimber_a", APIParT_Boolean, "1.0",
				"bTimber_b", APIParT_Boolean, "1.0",
				"bTimber_c", APIParT_Boolean, "1.0",
				"bTimber_d", APIParT_Boolean, "1.0");

			return guid;
		}

		// 각재 배치
		API_Guid placeTimber(double leftBottomX, double leftBottomY, double leftBottomZ, double radAng, double horLen, double verLen, bool bVertical, bool bForCeil) {
			API_Guid guid;
			EasyObjectPlacement timber;

			timber.init(L"목재v1.0.gsm", layerInd_Timber, this->floorInd, leftBottomX, leftBottomY, leftBottomZ, radAng);

			char insDir[20];
			double w_ang;

			if (bForCeil == true) {
				if (bVertical == true) {
					strcpy(insDir, "바닥눕히기");
					w_ang = DegreeToRad(0.0);
					timber.radAng += DegreeToRad(90.0);
				}
				else {
					strcpy(insDir, "바닥눕히기");
					w_ang = DegreeToRad(0.0);
					moveIn3D('y', timber.radAng, horLen, &timber.posX, &timber.posY, &timber.posZ);
				}
			}
			else {
				if (bVertical == true) {
					strcpy(insDir, "벽세우기");
					w_ang = DegreeToRad(90.0);
					moveIn3D('x', timber.radAng, horLen, &timber.posX, &timber.posY, &timber.posZ);
				}
				else {
					strcpy(insDir, "벽세우기");
					w_ang = DegreeToRad(0.0);
				}
			}

			guid = timber.placeObject(6,
				"w_ins", APIParT_CString, insDir,
				"w_w", APIParT_Length, format_string("%f", 0.050).c_str(),
				"w_h", APIParT_Length, format_string("%f", horLen).c_str(),
				"w_leng", APIParT_Length, format_string("%f", verLen).c_str(),
				"w_ang", APIParT_Angle, format_string("%f", w_ang).c_str(),
				"torsion_ang", APIParT_Angle, format_string("%f", 0.0).c_str());

			return guid;
		}

		// 휠러스페이서 배치
		API_Guid placeFillerspacer(double leftBottomX, double leftBottomY, double leftBottomZ, double radAng, double horLen, double verLen, bool bVertical, bool bForCeil) {
			API_Guid guid;
			EasyObjectPlacement fillersp;

			fillersp.init(L"휠러스페이서v1.0.gsm", layerInd_Fillerspacer, this->floorInd, leftBottomX, leftBottomY, leftBottomZ, radAng);

			double f_ang;
			double f_rota;

			if (bForCeil == true) {
				if (bVertical == true) {
					fillersp.radAng += DegreeToRad(90.0);
					f_ang = DegreeToRad(0.0);
					f_rota = DegreeToRad(90.0);
				}
				else {
					moveIn3D('y', fillersp.radAng, horLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
					f_ang = DegreeToRad(0.0);
					f_rota = DegreeToRad(90.0);
				}
			}
			else {
				if (bVertical == true) {
					moveIn3D('x', fillersp.radAng, horLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
					f_ang = DegreeToRad(90.0);
					f_rota = DegreeToRad(0.0);
				}
				else {
					f_ang = DegreeToRad(0.0);
					f_rota = DegreeToRad(0.0);
				}
			}

			guid = fillersp.placeObject(4,
				"f_thk", APIParT_Length, format_string("%f", horLen).c_str(),
				"f_leng", APIParT_Length, format_string("%f", verLen).c_str(),
				"f_ang", APIParT_Angle, format_string("%f", f_ang).c_str(),
				"f_rota", APIParT_Angle, format_string("%f", f_rota).c_str());

			return guid;
		}

		// 인코너판넬(L) 배치
		API_Guid placeIncornerPanel_L(double leftBottomX, double leftBottomY, double leftBottomZ, double radAng, double horLen, double horLen2, double verLen) {
			API_Guid guid;
			EasyObjectPlacement incornerPanel;

			incornerPanel.init(L"인코너판넬v1.0.gsm", layerInd_IncornerPanel, this->floorInd, leftBottomX, leftBottomY, leftBottomZ, radAng);

			incornerPanel.radAng += DegreeToRad(270.0);

			guid = incornerPanel.placeObject(5,
				"in_comp", APIParT_CString, "인코너판넬",
				"wid_s", APIParT_Length, format_string("%f", horLen2).c_str(),
				"leng_s", APIParT_Length, format_string("%f", horLen).c_str(),
				"hei_s", APIParT_Length, format_string("%f", verLen).c_str(),
				"dir_s", APIParT_CString, "세우기");

			return guid;
		}

		// 아웃코너판넬(L) 배치
		API_Guid placeOutcornerPanel_L(double leftBottomX, double leftBottomY, double leftBottomZ, double radAng, double horLen, double horLen2, double verLen) {
			API_Guid guid;
			EasyObjectPlacement outcornerPanel;

			outcornerPanel.init(L"아웃코너판넬v1.0.gsm", layerInd_OutcornerPanel, this->floorInd, leftBottomX, leftBottomY, leftBottomZ, radAng);

			guid = outcornerPanel.placeObject(4,
				"wid_s", APIParT_Length, format_string("%f", horLen).c_str(),
				"leng_s", APIParT_Length, format_string("%f", horLen2).c_str(),
				"hei_s", APIParT_Length, format_string("%f", verLen).c_str(),
				"dir_s", APIParT_CString, "세우기");

			return guid;
		}

		// 아웃코너앵글(L) 배치
		API_Guid placeOutcornerAngle_L(double leftBottomX, double leftBottomY, double leftBottomZ, double radAng, double verLen) {
			API_Guid guid;
			EasyObjectPlacement outcornerAngle;

			outcornerAngle.init(L"아웃코너앵글v1.0.gsm", layerInd_OutcornerAngle, this->floorInd, leftBottomX, leftBottomY, leftBottomZ, radAng);

			outcornerAngle.radAng += DegreeToRad(180.0);
			guid = outcornerAngle.placeObject(2,
				"a_leng", APIParT_Length, format_string("%f", verLen).c_str(),
				"a_ang", APIParT_Angle, format_string("%f", DegreeToRad(90.0)).c_str());

			return guid;
		}

		// 인코너판넬(R) 배치
		API_Guid placeIncornerPanel_R(double leftBottomX, double leftBottomY, double leftBottomZ, double radAng, double horLen, double horLen2, double verLen) {
			API_Guid guid;
			EasyObjectPlacement incornerPanel;

			incornerPanel.init(L"인코너판넬v1.0.gsm", layerInd_IncornerPanel, this->floorInd, leftBottomX, leftBottomY, leftBottomZ, radAng);

			moveIn3D('x', incornerPanel.radAng, horLen, &incornerPanel.posX, &incornerPanel.posY, &incornerPanel.posZ);
			incornerPanel.radAng += DegreeToRad(180.0);

			guid = incornerPanel.placeObject(5,
				"in_comp", APIParT_CString, "인코너판넬",
				"wid_s", APIParT_Length, format_string("%f", horLen).c_str(),
				"leng_s", APIParT_Length, format_string("%f", horLen2).c_str(),
				"hei_s", APIParT_Length, format_string("%f", verLen).c_str(),
				"dir_s", APIParT_CString, "세우기");

			return guid;
		}

		// 아웃코너판넬(R) 배치
		API_Guid placeOutcornerPanel_R(double leftBottomX, double leftBottomY, double leftBottomZ, double radAng, double horLen2, double verLen) {
			API_Guid guid;
			EasyObjectPlacement outcornerPanel;

			outcornerPanel.init(L"아웃코너판넬v1.0.gsm", layerInd_OutcornerPanel, this->floorInd, leftBottomX, leftBottomY, leftBottomZ, radAng);

			moveIn3D('x', outcornerPanel.radAng, horLen, &outcornerPanel.posX, &outcornerPanel.posY, &outcornerPanel.posZ);
			outcornerPanel.radAng += DegreeToRad(90.0);

			guid = outcornerPanel.placeObject(4,
				"wid_s", APIParT_Length, format_string("%f", horLen2).c_str(),
				"leng_s", APIParT_Length, format_string("%f", horLen).c_str(),
				"hei_s", APIParT_Length, format_string("%f", verLen).c_str(),
				"dir_s", APIParT_CString, "세우기");

			return guid;
		}

		// 아웃코너앵글(R) 배치
		API_Guid placeOutcornerAngle_R(double leftBottomX, double leftBottomY, double leftBottomZ, double radAng, double verLen) {
			API_Guid guid;
			EasyObjectPlacement outcornerAngle;

			outcornerAngle.init(L"아웃코너앵글v1.0.gsm", layerInd_OutcornerAngle, this->floorInd, leftBottomX, leftBottomY, leftBottomZ, radAng);

			outcornerAngle.radAng += DegreeToRad(270.0);
			guid = outcornerAngle.placeObject(2,
				"a_leng", APIParT_Length, format_string("%f", verLen).c_str(),
				"a_ang", APIParT_Angle, format_string("%f", DegreeToRad(90.0)).c_str());

			return guid;
		}

		// 테이블폼 타입A 배치 (각파이프 2줄)
		GS::Array<API_Guid> placeWallTableformA(double leftBottomX, double leftBottomY, double leftBottomZ, double radAng, double horLen, double verLen, bool bVertical, bool bFront) {
			// 유로폼 배치

			// 각파이프(수평) 배치

			// 각파이프(수직) 배치

			// 핀볼트 배치

			// 결합철물 배치

			// 헤드피스 배치

			// Push-Pull Props 배치
		}

		// 테이블폼 타입B 배치 (각파이프 1줄)
		GS::Array<API_Guid> placeWallTableformB(double leftBottomX, double leftBottomY, double leftBottomZ, double radAng, double horLen, double verLen, bool bVertical, bool bFront) {
			// 유로폼 배치

			// 각파이프(수평) 배치

			// 각파이프(수직) 배치

			// 핀볼트 배치

			// 결합철물 배치

			// 헤드피스 배치

			// Push-Pull Props 배치
		}

		// 테이블폼 타입C 배치 (각파이프 1줄, 십자 조인트 바 활용)
		GS::Array<API_Guid> placeWallTableformC(double leftBottomX, double leftBottomY, double leftBottomZ, double radAng, double horLen, double verLen, bool bVertical, bool bFront) {
			// 유로폼 배치

			// 십자 조인트 바 배치

			// 각파이프(수평) 배치

			// 각파이프(수직) 배치

			// 핀볼트 배치

			// 결합철물 배치

			// 헤드피스 배치

			// Push-Pull Props 배치
		}

		// 기본 셀 배치하기
		void placeCells() {
			// !!! 모든 셀 순회하기 (양면이면 앞뒤로 순회, 단면이면 앞면만 순회)
			placeOutcornerAngle_R(this->leftBottomX, this->leftBottomY, this->leftBottomZ, this->ang, 1.200);

			for (int i = 0; i < this->nCellsHor; i++) {
				// ... 테이블폼은 위의 셀까지 한꺼번에 설치해야 함
			}

			if (this->bSingleSide == false) {
				for (int i = 0; i < this->nCellsHor; i++) {
					// ... 테이블폼은 위의 셀까지 한꺼번에 설치해야 함
				}
			}
		}

		// 여백 셀 배치하기
		void placeMarginCells() {
			// !!! 모든 여백 셀 순회하기 (양면이면 앞뒤로 순회, 단면이면 앞면만 순회)
		}
	};

	// 단열재
	struct insulationElement
	{
		short	layerInd;		// 레이어 인덱스
		double	thk;			// 두께
		bool	bLimitSize;		// 가로/세로 크기 제한
		double	maxHorLen;		// 가로 최대 길이
		double	maxVerLen;		// 세로 최대 길이
	};

	PlacingZone placingZone;		// 배치 정보
	InfoWall infoWall;				// 벽 객체 정보
	insulationElement insulElem;	// 단열재 정보
	API_Guid structuralObject;		// 구조 객체의 GUID

	GS::Array<API_Guid> elemList_Front;
	GS::Array<API_Guid> elemList_Back;
	GS::Array<API_Guid> elemList_Margin_Front;
	GS::Array<API_Guid> elemList_Margin_Back;

	// 항목 인덱스 (1차 다이얼로그)
	short GRID_START_INDEX;						// 그리드
	short BUTTON_OBJ_TYPE_START_INDEX;			// 객체 타입 선택 버튼
	short EDITCONTROL_OBJ_WIDTH_START_INDEX;	// 객체 너비
	short BUTTON_ADD_COLUMN;					// 열 방향 객체 추가 버튼
	short BUTTON_DEL_COLUMN;					// 열 방향 객체 삭제 버튼
	short EDITCONTROL_OBJ_HEIGHT_START_INDEX;	// 객체 높이
	short BUTTON_ADD_ROW;						// 행 방향 객체 추가 버튼
	short BUTTON_DEL_ROW;						// 행 방향 객체 삭제 버튼
	
	short EDITCONTROL_TOTAL_HEIGHT;				// Edit컨트롤: 전체 높이
	short EDITCONTROL_REMAIN_HEIGHT;			// Edit컨트롤: 남은 높이
	
	short EDITCONTROL_TOTAL_WIDTH;				// Edit컨트롤: 전체 너비
	short EDITCONTROL_REMAIN_WIDTH;				// Edit컨트롤: 남은 너비

	// 항목 인덱스 (1차 세부 다이얼로그)
	short BRANCH_GRID_START_INDEX;					// 그리드
	short BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX;	// 객체 너비
	short BRANCH_BUTTON_ADD_COLUMN;					// 열 방향 객체 추가 버튼
	short BRANCH_BUTTON_DEL_COLUMN;					// 열 방향 객체 삭제 버튼

	// 항목 인덱스 (2차 다이얼로그)
	short DG2_GRID_START_INDEX;						// 그리드
	short DG2_BUTTON_OBJ_TYPE_START_INDEX;			// 객체 타입 선택 버튼
	short DG2_EDITCONTROL_OBJ_HEIGHT_START_INDEX;	// 객체 높이
	short DG2_BUTTON_ADD_ROW;						// 행 방향 객체 추가 버튼
	short DG2_BUTTON_DEL_ROW;						// 행 방향 객체 삭제 버튼

	short DGCALLBACK handler1_branch(short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */) {
		int* x = (int *)userData;	// 객체 버튼의 순번

		short	result;
		short	itemIndex;
		short	posX, posY;
		short	sizeX, sizeY;
		double	totalWidth;

		switch (message) {
		case DG_MSG_INIT:
			DGSetDialogTitle(dialogID, L"벽에 테이블폼 배치 - 셀 설정");

			// 확인 버튼
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 15, 70, 25);
			DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_OK, L"확인");
			DGShowItem(dialogID, DG_OK);

			// 취소
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 70, 25);
			DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_CANCEL, L"취소");
			DGShowItem(dialogID, DG_CANCEL);
			
			// 객체 종류
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 20, 55, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"객체 종류");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_POPUPCONTROL, 200, 0, 160, 15, 150, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			for (int i = 1; i < 14; i++) {
				DGPopUpInsertItem(dialogID, itemIndex, DG_POPUP_BOTTOM);
				DGPopUpSetItemText(dialogID, itemIndex, DG_POPUP_BOTTOM, objTypeStr[i]);
			}
			DGShowItem(dialogID, itemIndex);

			// 전체 너비
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 50, 55, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"전체 너비");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 160, 45, 85, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem(dialogID, itemIndex);
			DGDisableItem(dialogID, itemIndex);

			// 안내 텍스트
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 80, 300, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"안내 텍스트입니다.");
			DGShowItem(dialogID, itemIndex);

			// 셀 그리드 표시
			posX = 100; posY = 115;
			sizeX = 100; sizeY = 100;
			for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (i * sizeX), posY, sizeX, sizeY);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					BRANCH_GRID_START_INDEX = itemIndex;
			}

			// 셀 너비 (Edit컨트롤)
			for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX + (i * sizeX), posY + 100, sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX = itemIndex;
			}

			// 열 방향 객체 추가 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX), posY + 100, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			BRANCH_BUTTON_ADD_COLUMN = itemIndex;

			// 열 방향 객체 삭제 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX) + 25, posY + 100, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			BRANCH_BUTTON_DEL_COLUMN = itemIndex;

			// 다이얼로그 창 크기 조절
			DGSetDialogSize(dialogID, DG_CLIENT, 205 + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX), 300, DG_TOPLEFT, true);

			// 기본값 설정
			DGPopUpSelectItem(dialogID, BRANCH_POPUP_OBJ_TYPE, placingZone.cellsBasic[0][*x].objType);				// 팝업컨트롤
			DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH, placingZone.cellsBasic[0][*x].horLen);		// 전체 너비
			for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {										// 셀 너비
				DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + i, placingZone.cellsBasic[0][*x].cellHorLen[i]);
			}
				
			// 테이블폼의 경우
			if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_WALL_TABLEFORM_A) ||
				(DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_WALL_TABLEFORM_B) ||
				(DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_WALL_TABLEFORM_C)) {

				totalWidth = 0.0;
				for (int i = BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX; i < (BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + placingZone.cellsBasic[0][*x].nCellsHor); i++) {
					DGEnableItem(dialogID, i);
					totalWidth += DGGetItemValDouble(dialogID, i);
				}
				DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH, totalWidth);
				DGDisableItem(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);
			}
			// 그 외의 경우
			else {
				for (int i = BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX; i < (BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + placingZone.cellsBasic[0][*x].nCellsHor); i++) {
					DGDisableItem(dialogID, i);
				}
				DGEnableItem(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);
			}

			break;

		case DG_MSG_FOCUS:
			if (item == BRANCH_EDITCONTROL_TOTAL_WIDTH) {
				if (placingZone.bVertical == true) {
					if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_EUROFORM) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 600, 500, 450, 400, 300, 200");
					}
					else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_PLYWOOD) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 100 ~ 1220");
					}
					else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_TIMBER) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 5 ~ 1000");
					}
					else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_FILLERSPACER) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 10 ~ 50");
					}
					else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_INCORNER_PANEL_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_INCORNER_PANEL_R)) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 80 ~ 500");
					}
					else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_PANEL_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_PANEL_R)) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 80 ~ 500");
					}
					else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_ANGLE_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_ANGLE_R)) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 0");
					}
				}
				else {
					if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_EUROFORM) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 1200, 900, 600");
					}
					else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_PLYWOOD) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 100 ~ 2440");
					}
					else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_TIMBER) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 5 ~ 1000");
					}
					else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_FILLERSPACER) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 10 ~ 50");
					}
					else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_INCORNER_PANEL_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_INCORNER_PANEL_R)) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 80 ~ 500");
					}
					else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_PANEL_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_PANEL_R)) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 80 ~ 500");
					}
					else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_ANGLE_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_ANGLE_R)) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 0");
					}
				}
			}

			for (int i = BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX; i < (BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + placingZone.cellsBasic[0][*x].nCellsHor); i++) {
				if (item == i) {
					if (placingZone.bVertical == true) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 600, 500, 450, 400, 300, 200");
					}
					else {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 1200, 900, 600");
					}
				}
			}

			break;

		case DG_MSG_CHANGE:
			// 테이블폼의 경우
			if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_WALL_TABLEFORM_A) ||
				(DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_WALL_TABLEFORM_B) ||
				(DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_WALL_TABLEFORM_C)) {

				totalWidth = 0.0;
				for (int i = BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX; i < (BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + placingZone.cellsBasic[0][*x].nCellsHor); i++) {
					DGEnableItem(dialogID, i);
					totalWidth += DGGetItemValDouble(dialogID, i);
				}
				DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH, totalWidth);
				DGDisableItem(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);
			}
			// 그 외의 경우
			else {
				for (int i = BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX; i < (BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + placingZone.cellsBasic[0][*x].nCellsHor); i++) {
					DGDisableItem(dialogID, i);
				}
				DGEnableItem(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);
			}

			break;

		case DG_MSG_CLICK:
			if (item == DG_OK) {
				for (int i = 0; i < MAX_ROW; i++) {
					// 셀 너비
					for (int j = 0; j < placingZone.cellsBasic[i][*x].nCellsHor; j++) {
						placingZone.cellsBasic[i][*x].cellHorLen[j] = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + j);
						placingZone.cellsExtra[i][*x].cellHorLen[j] = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + j);
					}

					// 전체 너비
					placingZone.cellsBasic[i][*x].horLen = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);
					placingZone.cellsExtra[i][*x].horLen = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);

					placingZone.marginCellsBasic[i][*x].horLen = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);
					placingZone.marginCellsExtra[i][*x].horLen = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);

					// 객체 타입
					placingZone.cellsBasic[i][*x].objType = DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE);
					placingZone.cellsExtra[i][*x].objType = DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE);
				}
				break;
			}

			// 열 추가 버튼
			if (item == BRANCH_BUTTON_ADD_COLUMN) {
				item = 0;
				for (int i = 0; i < MAX_ROW; i++) {
					placingZone.cellsBasic[i][*x].nCellsHor++;
					placingZone.cellsExtra[i][*x].nCellsHor++;
				}
				break;
			}

			// 열 삭제 버튼
			if (item == BRANCH_BUTTON_DEL_COLUMN) {
				item = 0;
				for (int i = 0; i < MAX_ROW; i++) {
					placingZone.cellsBasic[i][*x].nCellsHor--;
					placingZone.cellsExtra[i][*x].nCellsHor--;
				}
				break;
			}

			// ---------- 동적 요소 새로 그리기
			// 동적 요소 모두 제거
			DGRemoveDialogItems(dialogID, BRANCH_GRID_START_INDEX);

			// 셀 그리드 표시
			posX = 100; posY = 115;
			sizeX = 100; sizeY = 100;
			for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (i * sizeX), posY, sizeX, sizeY);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					BRANCH_GRID_START_INDEX = itemIndex;
			}

			// 셀 너비 (Edit컨트롤)
			for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX + (i * sizeX), posY + 100, sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX = itemIndex;
			}

			// 열 방향 객체 추가 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX), posY + 100, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			BRANCH_BUTTON_ADD_COLUMN = itemIndex;

			// 열 방향 객체 삭제 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX) + 25, posY + 100, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			BRANCH_BUTTON_DEL_COLUMN = itemIndex;

			// 다이얼로그 창 크기 조절
			DGSetDialogSize(dialogID, DG_CLIENT, 205 + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX), 300, DG_TOPLEFT, true);

			// 기본값 설정
			DGPopUpSelectItem(dialogID, BRANCH_POPUP_OBJ_TYPE, placingZone.cellsBasic[0][*x].objType);				// 팝업컨트롤
			DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH, placingZone.cellsBasic[0][*x].horLen);		// 전체 너비
			for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {										// 셀 너비
				DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + i, placingZone.cellsBasic[0][*x].cellHorLen[i]);
			}
			// ---------- 동적 요소 새로 그리기

			break;

		case DG_MSG_CLOSE:
			break;
		}

		result = item;

		return result;
	}

	short DGCALLBACK handler1(short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */) {
		short	result;
		short	itemIndex;
		short	posX, posY;
		short	sizeX, sizeY;

		double	remainHeight, remainWidth;
		GS::UniString	buttonName;

		int		nCellsVer;
		
		switch (message) {
		case DG_MSG_INIT:
			DGSetDialogTitle(dialogID, L"벽에 테이블폼 배치");

			// 다음 버튼
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 15, 70, 25);
			DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_OK, L"다음");
			DGShowItem(dialogID, DG_OK);

			// 취소
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 70, 25);
			DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_CANCEL, L"취소");
			DGShowItem(dialogID, DG_CANCEL);

			// 배치면 선택 (양면/단면)
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 20, 50, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"배치면");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 1, 155, 15, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"양면");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 1, 210, 15, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"단면");
			DGShowItem(dialogID, itemIndex);

			// 테이블폼 방향 (세로/가로)
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 300, 20, 70, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"테이블폼 방향");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 2, 395, 15, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"세로");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 2, 450, 15, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"가로");
			DGShowItem(dialogID, itemIndex);

			// 벽과의 간격
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 50, 70, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"벽과의 간격");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 175, 45, 85, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem(dialogID, itemIndex);

			// Push-Pull Props 설치 유무
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 300, 50, 90, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"Push-Pull Props");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 3, 395, 45, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"설치");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 3, 450, 45, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"미설치");
			DGShowItem(dialogID, itemIndex);

			// 앞면/뒷면 전환
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 90, 70, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"표시면 선택");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 4, 180, 85, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"앞면");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 4, 235, 85, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"뒷면");
			DGShowItem(dialogID, itemIndex);

			// 셀 정보 복사 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 300, 85, 200, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, L"앞면->뒷면으로 셀정보 복사");
			DGShowItem(dialogID, itemIndex);

			// 안내 텍스트
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 120, 130, 380, 70);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"안내 텍스트입니다.");
			DGShowItem(dialogID, itemIndex);

			// 전체 높이
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 10, 130, 80, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"전체 높이");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 10, 155, 80, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGDisableItem(dialogID, itemIndex);
			DGShowItem(dialogID, itemIndex);
			EDITCONTROL_TOTAL_HEIGHT = itemIndex;

			// 남은 높이
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 10, 190, 80, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"남은 높이");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 10, 215, 80, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGDisableItem(dialogID, itemIndex);
			DGShowItem(dialogID, itemIndex);
			EDITCONTROL_REMAIN_HEIGHT = itemIndex;

			// 전체 너비
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 10, 290, 80, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"전체 너비");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 10, 315, 80, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGDisableItem(dialogID, itemIndex);
			DGShowItem(dialogID, itemIndex);
			EDITCONTROL_TOTAL_WIDTH = itemIndex;

			// 남은 너비
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 10, 350, 80, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"남은 너비");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 10, 375, 80, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGDisableItem(dialogID, itemIndex);
			DGShowItem(dialogID, itemIndex);
			EDITCONTROL_REMAIN_WIDTH = itemIndex;

			nCellsVer = placingZone.nCellsVerBasic;

			// 셀 그리드 표시
			posX = 205; posY = 230;
			sizeX = 100; sizeY = 100;
			for (int i = 0; i < nCellsVer; i++) {
				for (short j = 0; j < placingZone.nCellsHor; j++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (j * sizeX), posY + (i * sizeY), sizeX, sizeY);
					DGShowItem(dialogID, itemIndex);
					if (i == 0 && j == 0)
						GRID_START_INDEX = itemIndex;
				}
			}

			// 객체 타입 선택 버튼 표시
			posX = 205; posY = 230 + (nCellsVer * sizeY) + 5;
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (i * sizeX), posY, sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText(dialogID, itemIndex, objTypeStr[placingZone.cellsBasic[0][i].objType]);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					BUTTON_OBJ_TYPE_START_INDEX = itemIndex;
			}

			// 객체 너비 (Edit컨트롤)
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX + (i * sizeX), posY + 25, sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					EDITCONTROL_OBJ_WIDTH_START_INDEX = itemIndex;
			}

			// 열 방향 객체 추가 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX), posY, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			BUTTON_ADD_COLUMN = itemIndex;

			// 열 방향 객체 삭제 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX) + 25, posY, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			BUTTON_DEL_COLUMN = itemIndex;

			// 객체 높이
			posX = 100; posY = 230 + (nCellsVer * sizeY) - 63;
			for (int i = 0; i < nCellsVer; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX, posY - (i * sizeY), sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					EDITCONTROL_OBJ_HEIGHT_START_INDEX = itemIndex;
			}

			// 행 방향 객체 추가 버튼
			posX = 100; posY = 230;
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 25, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			BUTTON_ADD_ROW = itemIndex;

			// 행 방향 객체 삭제 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 50, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			BUTTON_DEL_ROW = itemIndex;

			// 다이얼로그 창 크기 조절
			DGSetDialogSize(dialogID, DG_CLIENT, 205 + (placingZone.nCellsHor * sizeX) + 100, 230 + (nCellsVer * sizeY) + 100, DG_TOPLEFT, true);

			// 기본값 설정: 라디오 버튼
			DGSetItemValLong(dialogID, RADIOBUTTON_DUAL_SIDE, true);	// 배치면: 양면
			DGSetItemValLong(dialogID, RADIOBUTTON_VERTICAL, true);		// 테이블폼 방향: 세로
			DGSetItemValLong(dialogID, RADIOBUTTON_PROPS_ON, true);		// Push-Pull Props: 설치
			DGSetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT, true);	// 표시면 선택: 앞면

			// 기본값 설정: 셀 크기
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				DGSetItemValDouble(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX + i, placingZone.cellsBasic[0][i].horLen);
			}

			for (int i = 0; i < placingZone.nCellsVerBasic; i++) {
				DGSetItemValDouble(dialogID, EDITCONTROL_OBJ_HEIGHT_START_INDEX + i, placingZone.cellsBasic[i][0].verLen);
			}

			// 너비 관련 값 설정
			DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.horLen);
			remainWidth = placingZone.horLen;
			for (int i = 0; i < placingZone.nCellsHor; i++)
				remainWidth -= DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX + i);
			DGSetItemValDouble(dialogID, EDITCONTROL_REMAIN_WIDTH, remainWidth);

			// 높이 관련 값 설정
			if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1) {
				DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_HEIGHT, placingZone.verLenBasic);
				remainHeight = placingZone.verLenBasic;
			}
			else {
				DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_HEIGHT, placingZone.verLenExtra);
				remainHeight = placingZone.verLenExtra;
			}
			for (int i = 0; i < nCellsVer; i++)
				remainHeight -= DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_HEIGHT_START_INDEX + i);
			DGSetItemValDouble(dialogID, EDITCONTROL_REMAIN_HEIGHT, remainHeight);

			break;

		case DG_MSG_FOCUS:
			if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1)
				nCellsVer = placingZone.nCellsVerBasic;
			else
				nCellsVer = placingZone.nCellsVerExtra;

			for (int i = 0; i < placingZone.nCellsHor; i++) {
				if (item == EDITCONTROL_OBJ_WIDTH_START_INDEX + i) {
					if (DGGetItemValLong(dialogID, RADIOBUTTON_VERTICAL)) {
						if (placingZone.cellsBasic[0][i].objType == OBJ_EUROFORM) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 600, 500, 450, 400, 300, 200");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_WALL_TABLEFORM_A) || (placingZone.cellsBasic[0][i].objType == OBJ_WALL_TABLEFORM_B) || (placingZone.cellsBasic[0][i].objType == OBJ_WALL_TABLEFORM_C)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 2300, 2250, 2200, 2150, 2100, 2050, 2000, 1950, 1900, \n1850, 1800, 1750, 1700, 1650, 1600, 1550, 1500, 1450, 1400, 1350, \n1300, 1250, 1200, 1150, 1100, 1050, 1000, 950, 900, 850, \n800, 750, 700, 650, 600, 500, 450, 400, 300, 200");
						}
						else if (placingZone.cellsBasic[0][i].objType == OBJ_PLYWOOD) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 100 ~ 1220");
						}
						else if (placingZone.cellsBasic[0][i].objType == OBJ_TIMBER) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 5 ~ 1000");
						}
						else if (placingZone.cellsBasic[0][i].objType == OBJ_FILLERSPACER) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 10 ~ 50");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_INCORNER_PANEL_L) || (placingZone.cellsBasic[0][i].objType == OBJ_INCORNER_PANEL_R)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 80 ~ 500");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_PANEL_L) || (placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_PANEL_R)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 80 ~ 500");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_ANGLE_L) || (placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_ANGLE_R)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 0");
						}
					}
					else {
						if (placingZone.cellsBasic[0][i].objType == OBJ_EUROFORM) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 1200, 900, 600");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_WALL_TABLEFORM_A) || (placingZone.cellsBasic[0][i].objType == OBJ_WALL_TABLEFORM_B) || (placingZone.cellsBasic[0][i].objType == OBJ_WALL_TABLEFORM_C)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 6000, 5700, 5400, 5100, 4800, 4500, 4200, \n3900, 3600, 3300, 3000, 2700, 2400, 2100, 1800, 1500");
						}
						else if (placingZone.cellsBasic[0][i].objType == OBJ_PLYWOOD) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 100 ~ 2440");
						}
						else if (placingZone.cellsBasic[0][i].objType == OBJ_TIMBER) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 5 ~ 1000");
						}
						else if (placingZone.cellsBasic[0][i].objType == OBJ_FILLERSPACER) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 10 ~ 50");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_INCORNER_PANEL_L) || (placingZone.cellsBasic[0][i].objType == OBJ_INCORNER_PANEL_R)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 80 ~ 500");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_PANEL_L) || (placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_PANEL_R)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 80 ~ 500");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_ANGLE_L) || (placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_ANGLE_R)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 0");
						}
					}
				}
			}

			for (int i = 0; i < nCellsVer; i++) {
				if (item == EDITCONTROL_OBJ_HEIGHT_START_INDEX + i) {
					if (DGGetItemValLong(dialogID, RADIOBUTTON_VERTICAL))
						DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 1200, 900, 600");
					else
						DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 600, 500, 450, 400, 300, 200");
				}
			}

			break;

		case DG_MSG_CHANGE:
			if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1)
				nCellsVer = placingZone.nCellsVerBasic;
			else
				nCellsVer = placingZone.nCellsVerExtra;

			// 테이블폼의 경우, 전체 너비 변경시 내부 셀 정보도 자동 변경됨
			for (int i = EDITCONTROL_OBJ_WIDTH_START_INDEX; i < EDITCONTROL_OBJ_WIDTH_START_INDEX + placingZone.nCellsHor; i++) {
				if (item == i) {
					int clickedIndex = i - EDITCONTROL_OBJ_WIDTH_START_INDEX;
					if ((placingZone.cellsBasic[0][clickedIndex].objType == OBJ_WALL_TABLEFORM_A) ||
						(placingZone.cellsBasic[0][clickedIndex].objType == OBJ_WALL_TABLEFORM_B) ||
						(placingZone.cellsBasic[0][clickedIndex].objType == OBJ_WALL_TABLEFORM_C)) {

						int INPUT_VALUE = (int)floor(DGGetItemValDouble(dialogID, i) * 1000);

						if (DGGetItemValLong(dialogID, RADIOBUTTON_VERTICAL) == 1) {
							for (int k = 0; k < sizeof(placingZone.tableformPresetVertical) / sizeof(int); k++) {
								if (INPUT_VALUE == placingZone.tableformPresetVertical[k]) {
									for (int m = 0; m < MAX_ROW; m++) {
										placingZone.cellsBasic[m][clickedIndex].nCellsHor = placingZone.tableformPresetVerticalConfig[k][1];
										placingZone.cellsExtra[m][clickedIndex].nCellsHor = placingZone.tableformPresetVerticalConfig[k][1];

										for (int n = 0; n < placingZone.cellsBasic[m][clickedIndex].nCellsHor; n++) {
											placingZone.cellsBasic[m][clickedIndex].cellHorLen[n] = (double)placingZone.tableformPresetVerticalConfig[k][n + 2] / 1000.0;
											placingZone.cellsExtra[m][clickedIndex].cellHorLen[n] = (double)placingZone.tableformPresetVerticalConfig[k][n + 2] / 1000.0;
										}
									}
								}
							}
						}
						else {
							for (int k = 0; k < sizeof(placingZone.tableformPresetHorizontal) / sizeof(int); k++) {
								if (INPUT_VALUE == placingZone.tableformPresetHorizontal[k]) {
									for (int m = 0; m < MAX_ROW; m++) {
										placingZone.cellsBasic[m][clickedIndex].nCellsHor = placingZone.tableformPresetHorizontalConfig[k][1];
										placingZone.cellsExtra[m][clickedIndex].nCellsHor = placingZone.tableformPresetHorizontalConfig[k][1];

										for (int n = 0; n < placingZone.cellsBasic[m][clickedIndex].nCellsHor; n++) {
											placingZone.cellsBasic[m][clickedIndex].cellHorLen[n] = (double)placingZone.tableformPresetHorizontalConfig[k][n + 2] / 1000.0;
											placingZone.cellsExtra[m][clickedIndex].cellHorLen[n] = (double)placingZone.tableformPresetHorizontalConfig[k][n + 2] / 1000.0;
										}
									}
								}
							}
						}
					}
				}
			}

			// 배치 정보 저장
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				for (int j = 0; j < MAX_ROW; j++) {
					placingZone.cellsBasic[j][i].horLen = DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX + i);
				}
			}

			if (item == RADIOBUTTON_SHOW_FRONT || item == RADIOBUTTON_SHOW_BACK) {
				// 앞면/뒷면을 선택하는 순간
				if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1) {
					for (int i = 0; i < placingZone.nCellsVerExtra; i++) {
						for (int j = 0; j < MAX_COL; j++) {
							placingZone.cellsExtra[i][j].verLen = DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_HEIGHT_START_INDEX + i);
						}
					}
				}
				else {
					for (int i = 0; i < placingZone.nCellsVerBasic; i++) {
						for (int j = 0; j < MAX_COL; j++) {
							placingZone.cellsBasic[i][j].verLen = DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_HEIGHT_START_INDEX + i);
						}
					}
				}
			}
			else {
				// 그 외의 경우
				if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1) {
					for (int i = 0; i < placingZone.nCellsVerBasic; i++) {
						for (int j = 0; j < MAX_COL; j++) {
							placingZone.cellsBasic[i][j].verLen = DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_HEIGHT_START_INDEX + i);
						}
					}
				}
				else {
					for (int i = 0; i < placingZone.nCellsVerExtra; i++) {
						for (int j = 0; j < MAX_COL; j++) {
							placingZone.cellsExtra[i][j].verLen = DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_HEIGHT_START_INDEX + i);
						}
					}
				}
			}

			// 배치면 저장
			if (DGGetItemValLong(dialogID, RADIOBUTTON_SINGLE_SIDE) == 1)
				placingZone.bSingleSide = true;
			else
				placingZone.bSingleSide = false;

			// 테이블폼 방향 저장
			if (DGGetItemValLong(dialogID, RADIOBUTTON_VERTICAL) == 1)
				placingZone.bVertical = true;
			else
				placingZone.bVertical = false;

			// 벽과의 간격 저장
			placingZone.gap = DGGetItemValDouble(dialogID, EDITCONTROL_GAP);

			// Push-Pull Props 저장
			if (DGGetItemValLong(dialogID, RADIOBUTTON_PROPS_ON) == 1)
				placingZone.bInstallProps = true;
			else
				placingZone.bInstallProps = false;

			// 앞면/뒷면 선택시
			if (item == RADIOBUTTON_SHOW_FRONT || item == RADIOBUTTON_SHOW_BACK) {
				// 동적 요소 모두 제거
				DGRemoveDialogItems(dialogID, GRID_START_INDEX);

				// 셀 그리드 표시
				posX = 205; posY = 230;
				sizeX = 100; sizeY = 100;
				for (int i = 0; i < nCellsVer; i++) {
					for (short j = 0; j < placingZone.nCellsHor; j++) {
						itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (j * sizeX), posY + (i * sizeY), sizeX, sizeY);
						DGShowItem(dialogID, itemIndex);
						if (i == 0 && j == 0)
							GRID_START_INDEX = itemIndex;
					}
				}

				// 객체 타입 선택 버튼 표시
				posX = 205; posY = 230 + (nCellsVer * sizeY) + 5;
				for (int i = 0; i < placingZone.nCellsHor; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (i * sizeX), posY, sizeX, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText(dialogID, itemIndex, objTypeStr[placingZone.cellsBasic[0][i].objType]);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						BUTTON_OBJ_TYPE_START_INDEX = itemIndex;
				}

				// 객체 너비 (Edit컨트롤)
				for (int i = 0; i < placingZone.nCellsHor; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX + (i * sizeX), posY + 25, sizeX, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						EDITCONTROL_OBJ_WIDTH_START_INDEX = itemIndex;
				}

				// 열 방향 객체 추가 버튼
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX), posY, 25, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText(dialogID, itemIndex, "+");
				DGShowItem(dialogID, itemIndex);
				BUTTON_ADD_COLUMN = itemIndex;

				// 열 방향 객체 삭제 버튼
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX) + 25, posY, 25, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText(dialogID, itemIndex, "-");
				DGShowItem(dialogID, itemIndex);
				BUTTON_DEL_COLUMN = itemIndex;

				// 객체 높이
				posX = 100; posY = 230 + (nCellsVer * sizeY) - 63;
				for (int i = 0; i < nCellsVer; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX, posY - (i * sizeY), sizeX, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						EDITCONTROL_OBJ_HEIGHT_START_INDEX = itemIndex;
				}

				// 행 방향 객체 추가 버튼
				posX = 100; posY = 230;
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 25, posY - 25, 25, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText(dialogID, itemIndex, "+");
				DGShowItem(dialogID, itemIndex);
				BUTTON_ADD_ROW = itemIndex;

				// 행 방향 객체 삭제 버튼
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 50, posY - 25, 25, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText(dialogID, itemIndex, "-");
				DGShowItem(dialogID, itemIndex);
				BUTTON_DEL_ROW = itemIndex;

				// 다이얼로그 창 크기 조절
				DGSetDialogSize(dialogID, DG_CLIENT, 205 + (placingZone.nCellsHor * sizeX) + 100, 230 + (nCellsVer * sizeY) + 100, DG_TOPLEFT, true);
			}

			// 기본값 설정: 셀 크기
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				DGSetItemValDouble(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX + i, placingZone.cellsBasic[0][i].horLen);
			}

			if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1) {
				for (int i = 0; i < placingZone.nCellsVerBasic; i++) {
					DGSetItemValDouble(dialogID, EDITCONTROL_OBJ_HEIGHT_START_INDEX + i, placingZone.cellsBasic[i][0].verLen);
				}
			}
			else {
				for (int i = 0; i < placingZone.nCellsVerExtra; i++) {
					DGSetItemValDouble(dialogID, EDITCONTROL_OBJ_HEIGHT_START_INDEX + i, placingZone.cellsBasic[i][0].verLen);
				}
			}

			// 너비 관련 값 설정
			DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.horLen);
			remainWidth = placingZone.horLen;
			for (int i = 0; i < placingZone.nCellsHor; i++)
				remainWidth -= DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX + i);
			DGSetItemValDouble(dialogID, EDITCONTROL_REMAIN_WIDTH, remainWidth);

			// 높이 관련 값 설정
			if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1) {
				DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_HEIGHT, placingZone.verLenBasic);
				remainHeight = placingZone.verLenBasic;
			}
			else {
				DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_HEIGHT, placingZone.verLenExtra);
				remainHeight = placingZone.verLenExtra;
			}
			for (int i = 0; i < nCellsVer; i++)
				remainHeight -= DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_HEIGHT_START_INDEX + i);
			DGSetItemValDouble(dialogID, EDITCONTROL_REMAIN_HEIGHT, remainHeight);

			break;

		case DG_MSG_CLICK:
			if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1)
				nCellsVer = placingZone.nCellsVerBasic;
			else
				nCellsVer = placingZone.nCellsVerExtra;

			if (item == DG_OK) {
				// 배치면
				if (DGGetItemValLong(dialogID, RADIOBUTTON_SINGLE_SIDE) == 1)
					placingZone.bSingleSide = true;
				else
					placingZone.bSingleSide = false;

				// 테이블폼 방향
				if (DGGetItemValLong(dialogID, RADIOBUTTON_VERTICAL) == 1)
					placingZone.bVertical = true;
				else
					placingZone.bVertical = false;

				// 벽과의 간격
				placingZone.gap = DGGetItemValDouble(dialogID, EDITCONTROL_GAP);

				// Push-Pull Props
				if (DGGetItemValLong(dialogID, RADIOBUTTON_PROPS_ON) == 1)
					placingZone.bInstallProps = true;
				else
					placingZone.bInstallProps = false;

				// 상단 여백
				double remainHeight;

				remainHeight = placingZone.verLenBasic;
				for (int i = 0; i < placingZone.nCellsVerBasic; i++)
					remainHeight -= placingZone.cellsBasic[i][0].verLen;
				placingZone.marginTopBasic = remainHeight;

				remainHeight = placingZone.verLenExtra;
				for (int i = 0; i < placingZone.nCellsVerExtra; i++)
					remainHeight -= placingZone.cellsExtra[i][0].verLen;
				placingZone.marginTopExtra = remainHeight;

				break;
			}
									
			// 셀 정보 복사 버튼
			if (item == PUSHBUTTON_INFO_COPY) {
				item = 0;
				placingZone.nCellsVerExtra = placingZone.nCellsVerBasic;
				for (int i = 0; i < MAX_ROW; i++) {
					for (int j = 0; j < MAX_COL; j++) {
						placingZone.cellsExtra[i][j] = placingZone.cellsBasic[i][j];
					}
				}

				break;
			}
			
			// 열 추가 버튼
			if (item == BUTTON_ADD_COLUMN) {
				item = 0;
				placingZone.nCellsHor++;
			}
			
			// 열 삭제 버튼
			if (item == BUTTON_DEL_COLUMN) {
				item = 0;
				placingZone.nCellsHor--;
			}
			
			// 행 추가 버튼
			if (item == BUTTON_ADD_ROW) {
				item = 0;

				if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1)
					placingZone.nCellsVerBasic++;
				else
					placingZone.nCellsVerExtra++;

				nCellsVer++;
			}
			
			// 행 삭제 버튼
			if (item == BUTTON_DEL_ROW) {
				item = 0;

				if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1)
					placingZone.nCellsVerBasic--;
				else
					placingZone.nCellsVerExtra--;

				nCellsVer--;
			}
			
			// 객체 타입 버튼 (UI 창 표시)
			for (int i = BUTTON_OBJ_TYPE_START_INDEX; i < BUTTON_OBJ_TYPE_START_INDEX + placingZone.nCellsHor; i++) {
				if (item == i) {
					item = 0;
					int clickedIndex = i - BUTTON_OBJ_TYPE_START_INDEX;
					result = DGBlankModalDialog(500, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, handler1_branch, (DGUserData)&clickedIndex);
				}
			}
			
			// ---------- 동적 요소 새로 그리기
			// 동적 요소 모두 제거
			DGRemoveDialogItems(dialogID, GRID_START_INDEX);

			// 셀 그리드 표시
			posX = 205; posY = 230;
			sizeX = 100; sizeY = 100;
			for (int i = 0; i < nCellsVer; i++) {
				for (short j = 0; j < placingZone.nCellsHor; j++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (j * sizeX), posY + (i * sizeY), sizeX, sizeY);
					DGShowItem(dialogID, itemIndex);
					if (i == 0 && j == 0)
						GRID_START_INDEX = itemIndex;
				}
			}

			// 객체 타입 선택 버튼 표시
			posX = 205; posY = 230 + (nCellsVer * sizeY) + 5;
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (i * sizeX), posY, sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText(dialogID, itemIndex, objTypeStr[placingZone.cellsBasic[0][i].objType]);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					BUTTON_OBJ_TYPE_START_INDEX = itemIndex;
			}

			// 객체 너비 (Edit컨트롤)
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX + (i * sizeX), posY + 25, sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					EDITCONTROL_OBJ_WIDTH_START_INDEX = itemIndex;
			}

			// 열 방향 객체 추가 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX), posY, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			BUTTON_ADD_COLUMN = itemIndex;

			// 열 방향 객체 삭제 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX) + 25, posY, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			BUTTON_DEL_COLUMN = itemIndex;

			// 객체 높이
			posX = 100; posY = 230 + (nCellsVer * sizeY) - 63;
			for (int i = 0; i < nCellsVer; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX, posY - (i * sizeY), sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					EDITCONTROL_OBJ_HEIGHT_START_INDEX = itemIndex;
			}

			// 행 방향 객체 추가 버튼
			posX = 100; posY = 230;
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 25, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			BUTTON_ADD_ROW = itemIndex;

			// 행 방향 객체 삭제 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 50, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			BUTTON_DEL_ROW = itemIndex;

			// 다이얼로그 창 크기 조절
			DGSetDialogSize(dialogID, DG_CLIENT, 205 + (placingZone.nCellsHor * sizeX) + 100, 230 + (nCellsVer * sizeY) + 100, DG_TOPLEFT, true);
			// ---------- 동적 요소 새로 그리기
			
			// 기본값 설정: 셀 크기
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				DGSetItemValDouble(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX + i, placingZone.cellsBasic[0][i].horLen);
			}

			if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1) {
				for (int i = 0; i < placingZone.nCellsVerBasic; i++) {
					DGSetItemValDouble(dialogID, EDITCONTROL_OBJ_HEIGHT_START_INDEX + i, placingZone.cellsBasic[i][0].verLen);
				}
			}
			else {
				for (int i = 0; i < placingZone.nCellsVerExtra; i++) {
					DGSetItemValDouble(dialogID, EDITCONTROL_OBJ_HEIGHT_START_INDEX + i, placingZone.cellsBasic[i][0].verLen);
				}
			}

			// 너비 관련 값 설정
			DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.horLen);
			remainWidth = placingZone.horLen;
			for (int i = 0; i < placingZone.nCellsHor; i++)
				remainWidth -= DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX + i);
			DGSetItemValDouble(dialogID, EDITCONTROL_REMAIN_WIDTH, remainWidth);

			// 높이 관련 값 설정
			DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_HEIGHT, placingZone.verLenBasic);
			if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1)
				remainHeight = placingZone.verLenBasic;
			else
				remainHeight = placingZone.verLenExtra;
			for (int i = 0; i < nCellsVer; i++)
				remainHeight -= DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_HEIGHT_START_INDEX + i);
			DGSetItemValDouble(dialogID, EDITCONTROL_REMAIN_HEIGHT, remainHeight);

			break;

		case DG_MSG_CLOSE:
			break;
		}

		result = item;

		return result;
	}

	short DGCALLBACK handler2_branch(short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */) {
		int* x = (int*)userData;	// 객체 버튼의 순번

		short	result;
		short	itemIndex;

		switch (message) {
		case DG_MSG_INIT:
			DGSetDialogTitle(dialogID, L"벽에 테이블폼 배치 - 여백 셀 설정");

			// 확인 버튼
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 15, 70, 25);
			DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_OK, L"확인");
			DGShowItem(dialogID, DG_OK);

			// 취소
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 70, 25);
			DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_CANCEL, L"취소");
			DGShowItem(dialogID, DG_CANCEL);

			// 객체 종류
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 20, 55, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"객체 종류");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_POPUPCONTROL, 200, 0, 160, 15, 150, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			for (int i = 1; i <= 2; i++) {
				DGPopUpInsertItem(dialogID, itemIndex, DG_POPUP_BOTTOM);
				DGPopUpSetItemText(dialogID, itemIndex, DG_POPUP_BOTTOM, objTypeStr[i]);
			}
			DGShowItem(dialogID, itemIndex);

			// 기본값 설정
			DGPopUpSelectItem(dialogID, BRANCH_POPUP_OBJ_TYPE, placingZone.marginCellsBasic[*x][0].objType);	// 팝업컨트롤

			break;

		case DG_MSG_CLICK:
			if (item == DG_OK) {
				for (int i = 0; i < MAX_COL; i++) {
					// 객체 타입
					placingZone.marginCellsBasic[*x][i].objType = DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE);
					placingZone.marginCellsBasic[*x][i].objType = DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE);
				}
				break;
			}

			break;

		case DG_MSG_CLOSE:
			break;
		}

		result = item;

		return result;
	}

	short DGCALLBACK handler2(short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */) {
		bool* bFront = (bool*)userData;

		short	result;
		short	itemIndex;
		short	posX, posY;
		short	sizeX, sizeY;

		short focusedItemIndex;
		short buttonItemIndex;
		double remainHeight;

		switch (message) {
		case DG_MSG_INIT:
			// 타이틀
			if (*bFront == true)
				DGSetDialogTitle(dialogID, L"상단 여백 채우기 (낮은쪽)");
			else
				DGSetDialogTitle(dialogID, L"상단 여백 채우기 (높은쪽)");

			// 다음 버튼
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 15, 70, 25);
			DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_OK, L"다음");
			DGShowItem(dialogID, DG_OK);

			// 통과 버튼
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 75, 70, 25);
			DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_CANCEL, L"통과");
			DGShowItem(dialogID, DG_CANCEL);

			// 이전 버튼
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 70, 25);
			DGSetItemFont(dialogID, DG2_BUTTON_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG2_BUTTON_PREV, L"이전");
			DGShowItem(dialogID, DG2_BUTTON_PREV);

			// 전체 높이
			DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 20, 55, 23);
			DGSetItemFont(dialogID, DG2_LABEL_TOTAL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG2_LABEL_TOTAL_HEIGHT, L"전체 높이");
			DGShowItem(dialogID, DG2_LABEL_TOTAL_HEIGHT);

			DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 160, 15, 85, 25);
			DGSetItemFont(dialogID, DG2_EDITCONTROL_TOTAL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			if (*bFront == true)
				DGSetItemValDouble(dialogID, DG2_EDITCONTROL_TOTAL_HEIGHT, placingZone.marginTopBasic);
			else
				DGSetItemValDouble(dialogID, DG2_EDITCONTROL_TOTAL_HEIGHT, placingZone.marginTopExtra);
			DGShowItem(dialogID, DG2_EDITCONTROL_TOTAL_HEIGHT);
			DGDisableItem(dialogID, DG2_EDITCONTROL_TOTAL_HEIGHT);

			// 남은 높이
			DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 270, 20, 55, 23);
			DGSetItemFont(dialogID, DG2_LABEL_REMAIN_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG2_LABEL_REMAIN_HEIGHT, L"남은 높이");
			DGShowItem(dialogID, DG2_LABEL_REMAIN_HEIGHT);

			DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 330, 15, 85, 25);
			DGSetItemFont(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			if (*bFront == true) {
				remainHeight = placingZone.marginTopBasic;
				for (int i = 0; i < placingZone.nMarginCellsVerBasic; i++) {
					remainHeight -= placingZone.marginCellsBasic[i][0].verLen;
				}
				DGSetItemValDouble(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT, remainHeight);
			}
			else {
				remainHeight = placingZone.marginTopExtra;
				for (int i = 0; i < placingZone.nMarginCellsVerExtra; i++) {
					remainHeight -= placingZone.marginCellsExtra[i][0].verLen;
				}
				DGSetItemValDouble(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT, remainHeight);
			}
			DGShowItem(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT);
			DGDisableItem(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT);

			// 안내 텍스트
			DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 50, 300, 23);
			DGSetItemFont(dialogID, DG2_LABEL_GUIDE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG2_LABEL_GUIDE, L"안내 텍스트입니다.");
			DGShowItem(dialogID, DG2_LABEL_GUIDE);

			// 셀 그리드 표시
			posX = 205; posY = 115;
			sizeX = 100; sizeY = 100;
			if (*bFront == true) {
				for (int i = 0; i < placingZone.nMarginCellsVerBasic; i++) {
					for (int j = 0; j < placingZone.nCellsHor; j++) {
						itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (j * sizeX), posY + (i * sizeY), sizeX, sizeY);
						DGShowItem(dialogID, itemIndex);
						if (i == 0 && j == 0)
							DG2_GRID_START_INDEX = itemIndex;
					}
				}
			}
			else {
				for (int i = 0; i < placingZone.nMarginCellsVerExtra; i++) {
					for (int j = 0; j < placingZone.nCellsHor; j++) {
						itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (j * sizeX), posY + (i * sizeY), sizeX, sizeY);
						DGShowItem(dialogID, itemIndex);
						if (i == 0 && j == 0)
							DG2_GRID_START_INDEX = itemIndex;
					}
				}
			}

			// 객체 타입 선택 버튼 표시
			if (*bFront == true) {
				for (int i = 0; i < placingZone.nMarginCellsVerBasic; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX - sizeX - 5, posY + 50 + (sizeY * i), 100, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText(dialogID, itemIndex, objTypeStr[placingZone.marginCellsBasic[i][0].objType]);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						DG2_BUTTON_OBJ_TYPE_START_INDEX = itemIndex;
				}
			}
			else {
				for (int i = 0; i < placingZone.nMarginCellsVerExtra; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX - sizeX - 5, posY + 50 + (sizeY * i), 100, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText(dialogID, itemIndex, objTypeStr[placingZone.marginCellsExtra[i][0].objType]);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						DG2_BUTTON_OBJ_TYPE_START_INDEX = itemIndex;
				}
			}

			// 셀 높이 (Edit컨트롤)
			if (*bFront == true) {
				for (int i = 0; i < placingZone.nMarginCellsVerBasic; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX - sizeX - 5, posY + 25 + (sizeY * i), 100, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemValDouble(dialogID, itemIndex, placingZone.marginCellsBasic[i][0].verLen);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						DG2_EDITCONTROL_OBJ_HEIGHT_START_INDEX = itemIndex;
				}
			}
			else {
				for (int i = 0; i < placingZone.nMarginCellsVerExtra; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX - sizeX - 5, posY + 25 + (sizeY * i), 100, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemValDouble(dialogID, itemIndex, placingZone.marginCellsExtra[i][0].verLen);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						DG2_EDITCONTROL_OBJ_HEIGHT_START_INDEX = itemIndex;
				}
			}

			// 행 방향 객체 추가 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX - sizeX + 25, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			DG2_BUTTON_ADD_ROW = itemIndex;

			// 행 방향 객체 삭제 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX - sizeX + 50, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			DG2_BUTTON_DEL_ROW = itemIndex;

			// 남은 높이 업데이트
			if (*bFront == true) {
				remainHeight = placingZone.marginTopBasic;
				for (int i = 0; i < placingZone.nMarginCellsVerBasic; i++) {
					remainHeight -= placingZone.marginCellsBasic[i][0].verLen;
				}
				DGSetItemValDouble(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT, remainHeight);
			}
			else {
				remainHeight = placingZone.marginTopExtra;
				for (int i = 0; i < placingZone.nMarginCellsVerExtra; i++) {
					remainHeight -= placingZone.marginCellsExtra[i][0].verLen;
				}
				DGSetItemValDouble(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT, remainHeight);
			}
			DGShowItem(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT);
			DGDisableItem(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT);

			// 다이얼로그 창 크기 조절
			DGSetDialogSize(dialogID, DG_CLIENT, 300 + (placingZone.nCellsHor * sizeX), 200 + (placingZone.nMarginCellsVerBasic * sizeY), DG_TOPLEFT, true);

			break;

		case DG_MSG_FOCUS:
			if (*bFront == true) {
				for (int i = 0; i < placingZone.nMarginCellsVerBasic; i++) {
					focusedItemIndex = DG2_EDITCONTROL_OBJ_HEIGHT_START_INDEX + i;
					buttonItemIndex = DG2_BUTTON_OBJ_TYPE_START_INDEX + i;
					
					if (item == focusedItemIndex) {
						if (placingZone.marginCellsBasic[i][0].objType == OBJ_EUROFORM) {
							DGSetItemText(dialogID, DG2_LABEL_GUIDE, L"입력 가능 치수: 600, 500, 450, 400, 300, 200");
						}
						else if (placingZone.marginCellsBasic[i][0].objType == OBJ_PLYWOOD) {
							DGSetItemText(dialogID, DG2_LABEL_GUIDE, L"입력 가능 치수: 100 ~ 1220");
						}
					}
				}
			}
			else {
				for (int i = 0; i < placingZone.nMarginCellsVerExtra; i++) {
					focusedItemIndex = DG2_EDITCONTROL_OBJ_HEIGHT_START_INDEX + i;
					buttonItemIndex = DG2_BUTTON_OBJ_TYPE_START_INDEX + i;

					if (item == focusedItemIndex) {
						if (placingZone.marginCellsExtra[i][0].objType == OBJ_EUROFORM) {
							DGSetItemText(dialogID, DG2_LABEL_GUIDE, L"입력 가능 치수: 600, 500, 450, 400, 300, 200");
						}
						else if (placingZone.marginCellsExtra[i][0].objType == OBJ_PLYWOOD) {
							DGSetItemText(dialogID, DG2_LABEL_GUIDE, L"입력 가능 치수: 100 ~ 1220");
						}
					}
				}
			}

			break;

		case DG_MSG_CHANGE:
			// 배치 정보 업데이트
			if (*bFront == true) {
				for (int i = 0; i < placingZone.nMarginCellsVerBasic; i++) {
					focusedItemIndex = DG2_EDITCONTROL_OBJ_HEIGHT_START_INDEX + i;
					for (int j = 0; j < placingZone.nCellsHor; j++) {
						placingZone.marginCellsBasic[i][0].verLen = DGGetItemValDouble(dialogID, focusedItemIndex);
					}
				}
			}
			else {
				for (int i = 0; i < placingZone.nMarginCellsVerExtra; i++) {
					focusedItemIndex = DG2_EDITCONTROL_OBJ_HEIGHT_START_INDEX + i;
					for (int j = 0; j < placingZone.nCellsHor; j++) {
						placingZone.marginCellsExtra[i][0].verLen = DGGetItemValDouble(dialogID, focusedItemIndex);
					}
				}
			}

			// 남은 높이 업데이트
			if (*bFront == true) {
				remainHeight = placingZone.marginTopBasic;
				for (int i = 0; i < placingZone.nMarginCellsVerBasic; i++) {
					remainHeight -= placingZone.marginCellsBasic[i][0].verLen;
				}
				DGSetItemValDouble(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT, remainHeight);
			}
			else {
				remainHeight = placingZone.marginTopExtra;
				for (int i = 0; i < placingZone.nMarginCellsVerExtra; i++) {
					remainHeight -= placingZone.marginCellsExtra[i][0].verLen;
				}
				DGSetItemValDouble(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT, remainHeight);
			}
			DGShowItem(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT);
			DGDisableItem(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT);

			break;

		case DG_MSG_CLICK:
			if (item == DG_OK) {
				for (int i = 0; i < MAX_ROW; i++) {
					for (int j = 0; j < placingZone.nMarginCellsVerBasic; j++) {
						// 셀 높이
						if (*bFront == true) {
							placingZone.marginCellsBasic[i][j].verLen = DGGetItemValDouble(dialogID, DG2_EDITCONTROL_OBJ_HEIGHT_START_INDEX + j);
						}
						else {
							placingZone.marginCellsExtra[i][j].verLen = DGGetItemValDouble(dialogID, DG2_EDITCONTROL_OBJ_HEIGHT_START_INDEX + j);
						}
					}
				}
				break;
			}

			// 행 추가 버튼
			if (item == DG2_BUTTON_ADD_ROW) {
				item = 0;
				if (*bFront == true) {
					placingZone.nMarginCellsVerBasic++;
				}
				else {
					placingZone.nMarginCellsVerExtra++;
				}
			}

			// 행 삭제 버튼
			if (item == DG2_BUTTON_DEL_ROW) {
				item = 0;
				if (*bFront == true) {
					placingZone.nMarginCellsVerBasic--;
				}
				else {
					placingZone.nMarginCellsVerExtra--;
				}
			}

			// 객체 타입 버튼 (UI 창 표시)
			if (*bFront == true) {
				for (int i = DG2_BUTTON_OBJ_TYPE_START_INDEX; i < DG2_BUTTON_OBJ_TYPE_START_INDEX + placingZone.nMarginCellsVerBasic; i++) {
					if (item == i) {
						item = 0;
						int clickedIndex = i - DG2_BUTTON_OBJ_TYPE_START_INDEX;
						result = DGBlankModalDialog(350, 100, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, handler2_branch, (DGUserData)&clickedIndex);
					}
				}
			}
			else {
				for (int i = DG2_BUTTON_OBJ_TYPE_START_INDEX; i < DG2_BUTTON_OBJ_TYPE_START_INDEX + placingZone.nMarginCellsVerExtra; i++) {
					if (item == i) {
						item = 0;
						int clickedIndex = i - DG2_BUTTON_OBJ_TYPE_START_INDEX;
						result = DGBlankModalDialog(350, 100, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, handler2_branch, (DGUserData)&clickedIndex);
					}
				}
			}

			// ---------- 동적 요소 새로 그리기
			// 동적 요소 모두 제거
			DGRemoveDialogItems(dialogID, DG2_GRID_START_INDEX);

			// 셀 그리드 표시
			posX = 205; posY = 115;
			sizeX = 100; sizeY = 100;
			if (*bFront == true) {
				for (int i = 0; i < placingZone.nMarginCellsVerBasic; i++) {
					for (int j = 0; j < placingZone.nCellsHor; j++) {
						itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (j * sizeX), posY + (i * sizeY), sizeX, sizeY);
						DGShowItem(dialogID, itemIndex);
						if (i == 0 && j == 0)
							DG2_GRID_START_INDEX = itemIndex;
					}
				}
			}
			else {
				for (int i = 0; i < placingZone.nMarginCellsVerExtra; i++) {
					for (int j = 0; j < placingZone.nCellsHor; j++) {
						itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (j * sizeX), posY + (i * sizeY), sizeX, sizeY);
						DGShowItem(dialogID, itemIndex);
						if (i == 0 && j == 0)
							DG2_GRID_START_INDEX = itemIndex;
					}
				}
			}

			// 객체 타입 선택 버튼 표시
			if (*bFront == true) {
				for (int i = 0; i < placingZone.nMarginCellsVerBasic; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX - sizeX - 5, posY + 50 + (sizeY * i), 100, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText(dialogID, itemIndex, objTypeStr[placingZone.marginCellsBasic[i][0].objType]);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						DG2_BUTTON_OBJ_TYPE_START_INDEX = itemIndex;
				}
			}
			else {
				for (int i = 0; i < placingZone.nMarginCellsVerExtra; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX - sizeX - 5, posY + 50 + (sizeY * i), 100, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText(dialogID, itemIndex, objTypeStr[placingZone.marginCellsExtra[i][0].objType]);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						DG2_BUTTON_OBJ_TYPE_START_INDEX = itemIndex;
				}
			}

			// 셀 높이 (Edit컨트롤)
			if (*bFront == true) {
				for (int i = 0; i < placingZone.nMarginCellsVerBasic; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX - sizeX - 5, posY + 25 + (sizeY * i), 100, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemValDouble(dialogID, itemIndex, placingZone.marginCellsBasic[i][0].verLen);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						DG2_EDITCONTROL_OBJ_HEIGHT_START_INDEX = itemIndex;
				}
			}
			else {
				for (int i = 0; i < placingZone.nMarginCellsVerExtra; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX - sizeX - 5, posY + 25 + (sizeY * i), 100, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemValDouble(dialogID, itemIndex, placingZone.marginCellsExtra[i][0].verLen);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						DG2_EDITCONTROL_OBJ_HEIGHT_START_INDEX = itemIndex;
				}
			}

			// 행 방향 객체 추가 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX - sizeX + 25, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			DG2_BUTTON_ADD_ROW = itemIndex;

			// 행 방향 객체 삭제 버튼
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX - sizeX + 50, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			DG2_BUTTON_DEL_ROW = itemIndex;

			// 다이얼로그 창 크기 조절
			DGSetDialogSize(dialogID, DG_CLIENT, 300 + (placingZone.nCellsHor * sizeX), 200 + (placingZone.nMarginCellsVerBasic * sizeY), DG_TOPLEFT, true);
			// ---------- 동적 요소 새로 그리기

			// 남은 높이 업데이트
			if (*bFront == true) {
				remainHeight = placingZone.marginTopBasic;
				for (int i = 0; i < placingZone.nMarginCellsVerBasic; i++) {
					remainHeight -= placingZone.marginCellsBasic[i][0].verLen;
				}
				DGSetItemValDouble(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT, remainHeight);
			}
			else {
				remainHeight = placingZone.marginTopExtra;
				for (int i = 0; i < placingZone.nMarginCellsVerExtra; i++) {
					remainHeight -= placingZone.marginCellsExtra[i][0].verLen;
				}
				DGSetItemValDouble(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT, remainHeight);
			}
			DGShowItem(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT);
			DGDisableItem(dialogID, DG2_EDITCONTROL_REMAIN_HEIGHT);

			break;

		case DG_MSG_CLOSE:
			break;
		}

		result = item;

		return result;
	}

	short DGCALLBACK layer_handler(short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */) {
		short	result;
		API_UCCallbackType	ucb;

		switch (message) {
		case DG_MSG_INIT:
			DGSetDialogTitle(dialogID, L"레이어 선택하기");

			DGSetItemText(dialogID, DG_OK, L"확인");
			DGSetItemText(dialogID, DG_CANCEL, L"취소");

			DGSetItemText(dialogID, CHECKBOX_LAYER_COUPLING, L"레이어 묶음");
			DGSetItemValLong(dialogID, CHECKBOX_LAYER_COUPLING, true);

			DGSetItemText(dialogID, LABEL_LAYER_EUROFORM, L"유로폼");
			DGSetItemText(dialogID, LABEL_LAYER_PLYWOOD, L"합판");
			DGSetItemText(dialogID, LABEL_LAYER_TIMBER, L"각재");
			DGSetItemText(dialogID, LABEL_LAYER_FILLERSPACER, L"휠러스페이서");
			DGSetItemText(dialogID, LABEL_LAYER_INCORNER_PANEL, L"인코너판넬");
			DGSetItemText(dialogID, LABEL_LAYER_OUTCORNER_PANEL, L"아웃코너판넬");
			DGSetItemText(dialogID, LABEL_LAYER_OUTCORNER_ANGLE, L"아웃코너앵글");
			DGSetItemText(dialogID, LABEL_LAYER_BLUE_CLAMP, L"블루클램프");
			DGSetItemText(dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL, L"블루목심");
			DGSetItemText(dialogID, LABEL_LAYER_RECTPIPE, L"각파이프");
			DGSetItemText(dialogID, LABEL_LAYER_PINBOLT, L"핀볼트");
			DGSetItemText(dialogID, LABEL_LAYER_HEADPIECE, L"헤드피스");
			DGSetItemText(dialogID, LABEL_LAYER_PROPS, L"푸시풀프롭스");
			DGSetItemText(dialogID, LABEL_LAYER_JOIN, L"결합철물");
			DGSetItemText(dialogID, LABEL_LAYER_CROSS_JOINT_BAR, L"십자조인트바");

			DGHideItem(dialogID, LABEL_LAYER_16);
			DGHideItem(dialogID, LABEL_LAYER_17);
			DGHideItem(dialogID, LABEL_LAYER_18);
			DGHideItem(dialogID, LABEL_LAYER_19);
			DGHideItem(dialogID, LABEL_LAYER_20);
			DGHideItem(dialogID, USERCONTROL_LAYER_16);
			DGHideItem(dialogID, USERCONTROL_LAYER_17);
			DGHideItem(dialogID, USERCONTROL_LAYER_18);
			DGHideItem(dialogID, USERCONTROL_LAYER_19);
			DGHideItem(dialogID, USERCONTROL_LAYER_20);

			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 540, 160, 25);
			DGSetItemFont(dialogID, BUTTON_AUTOSET, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, BUTTON_AUTOSET, L"레이어 자동 설정");
			DGShowItem(dialogID, BUTTON_AUTOSET);

			DGSetDialogSize(dialogID, DG_CLIENT, 300, 580, DG_TOPLEFT, true);

			// 객체 종류를 조사하여 필요한 레이어만 켜기
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				for (int j = 0; j < placingZone.nCellsVerBasic; j++) {
					if (placingZone.cellsBasic[j][i].objType == OBJ_EUROFORM)
						placingZone.bLayerInd_Euroform = true;
					if (placingZone.cellsBasic[j][i].objType == OBJ_FILLERSPACER)
						placingZone.bLayerInd_Fillerspacer = true;
					if ((placingZone.cellsBasic[j][i].objType == OBJ_INCORNER_PANEL_L) || (placingZone.cellsBasic[j][i].objType == OBJ_INCORNER_PANEL_R))
						placingZone.bLayerInd_IncornerPanel = true;
					if ((placingZone.cellsBasic[j][i].objType == OBJ_OUTCORNER_PANEL_L) || (placingZone.cellsBasic[j][i].objType == OBJ_OUTCORNER_PANEL_R))
						placingZone.bLayerInd_OutcornerPanel = true;
					if ((placingZone.cellsBasic[j][i].objType == OBJ_OUTCORNER_ANGLE_L) || (placingZone.cellsBasic[j][i].objType == OBJ_OUTCORNER_ANGLE_R))
						placingZone.bLayerInd_OutcornerAngle = true;
					if (placingZone.cellsBasic[j][i].objType == OBJ_WALL_TABLEFORM_A) {
						placingZone.bLayerInd_Euroform = true;
						placingZone.bLayerInd_Rectpipe = true;
						placingZone.bLayerInd_Pinbolt = true;
						placingZone.bLayerInd_Headpiece = true;
						placingZone.bLayerInd_Props = true;
						placingZone.bLayerInd_Join = true;
					}
					if (placingZone.cellsBasic[j][i].objType == OBJ_WALL_TABLEFORM_B) {
						placingZone.bLayerInd_Euroform = true;
						placingZone.bLayerInd_Rectpipe = true;
						placingZone.bLayerInd_Pinbolt = true;
						placingZone.bLayerInd_Headpiece = true;
						placingZone.bLayerInd_Props = true;
						placingZone.bLayerInd_Join = true;
					}
					if (placingZone.cellsBasic[j][i].objType == OBJ_WALL_TABLEFORM_C) {
						placingZone.bLayerInd_Euroform = true;
						placingZone.bLayerInd_Rectpipe = true;
						placingZone.bLayerInd_Pinbolt = true;
						placingZone.bLayerInd_Headpiece = true;
						placingZone.bLayerInd_Props = true;
						placingZone.bLayerInd_Join = true;
						placingZone.bLayerInd_CrossJointBar = true;
					}
				}

				for (int j = 0; j < placingZone.nCellsVerExtra; j++) {
					if (placingZone.cellsExtra[j][i].objType == OBJ_EUROFORM)
						placingZone.bLayerInd_Euroform = true;
					if (placingZone.cellsExtra[j][i].objType == OBJ_FILLERSPACER)
						placingZone.bLayerInd_Fillerspacer = true;
					if ((placingZone.cellsExtra[j][i].objType == OBJ_INCORNER_PANEL_L) || (placingZone.cellsExtra[j][i].objType == OBJ_INCORNER_PANEL_R))
						placingZone.bLayerInd_IncornerPanel = true;
					if ((placingZone.cellsExtra[j][i].objType == OBJ_OUTCORNER_PANEL_L) || (placingZone.cellsExtra[j][i].objType == OBJ_OUTCORNER_PANEL_R))
						placingZone.bLayerInd_OutcornerPanel = true;
					if ((placingZone.cellsExtra[j][i].objType == OBJ_OUTCORNER_ANGLE_L) || (placingZone.cellsExtra[j][i].objType == OBJ_OUTCORNER_ANGLE_R))
						placingZone.bLayerInd_OutcornerAngle = true;
					if (placingZone.cellsExtra[j][i].objType == OBJ_WALL_TABLEFORM_A) {
						placingZone.bLayerInd_Euroform = true;
						placingZone.bLayerInd_Rectpipe = true;
						placingZone.bLayerInd_Pinbolt = true;
						placingZone.bLayerInd_Headpiece = true;
						placingZone.bLayerInd_Props = true;
						placingZone.bLayerInd_Join = true;
					}
					if (placingZone.cellsExtra[j][i].objType == OBJ_WALL_TABLEFORM_B) {
						placingZone.bLayerInd_Euroform = true;
						placingZone.bLayerInd_Rectpipe = true;
						placingZone.bLayerInd_Pinbolt = true;
						placingZone.bLayerInd_Headpiece = true;
						placingZone.bLayerInd_Props = true;
						placingZone.bLayerInd_Join = true;
					}
					if (placingZone.cellsExtra[j][i].objType == OBJ_WALL_TABLEFORM_C) {
						placingZone.bLayerInd_Euroform = true;
						placingZone.bLayerInd_Rectpipe = true;
						placingZone.bLayerInd_Pinbolt = true;
						placingZone.bLayerInd_Headpiece = true;
						placingZone.bLayerInd_Props = true;
						placingZone.bLayerInd_Join = true;
						placingZone.bLayerInd_CrossJointBar = true;
					}
				}
			}

			placingZone.bLayerInd_Plywood = true;
			placingZone.bLayerInd_Timber = true;
			placingZone.bLayerInd_BlueClamp = true;
			placingZone.bLayerInd_BlueTimberRail = true;

			// 유저 컨트롤 초기화
			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_EUROFORM, true);
			if (placingZone.bLayerInd_Euroform == false) {
				DGDisableItem(dialogID, LABEL_LAYER_EUROFORM);
				DGDisableItem(dialogID, USERCONTROL_LAYER_EUROFORM);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_PLYWOOD, true);
			if (placingZone.bLayerInd_Plywood == false) {
				DGDisableItem(dialogID, LABEL_LAYER_PLYWOOD);
				DGDisableItem(dialogID, USERCONTROL_LAYER_PLYWOOD);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_TIMBER;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_TIMBER, true);
			if (placingZone.bLayerInd_Timber == false) {
				DGDisableItem(dialogID, LABEL_LAYER_TIMBER);
				DGDisableItem(dialogID, USERCONTROL_LAYER_TIMBER);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_FILLERSPACER;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_FILLERSPACER, true);
			if (placingZone.bLayerInd_Fillerspacer == false) {
				DGDisableItem(dialogID, LABEL_LAYER_FILLERSPACER);
				DGDisableItem(dialogID, USERCONTROL_LAYER_FILLERSPACER);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_INCORNER_PANEL;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_INCORNER_PANEL, true);
			if (placingZone.bLayerInd_IncornerPanel == false) {
				DGDisableItem(dialogID, LABEL_LAYER_INCORNER_PANEL);
				DGDisableItem(dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_OUTCORNER_PANEL;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, true);
			if (placingZone.bLayerInd_OutcornerPanel == false) {
				DGDisableItem(dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGDisableItem(dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, true);
			if (placingZone.bLayerInd_OutcornerAngle == false) {
				DGDisableItem(dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGDisableItem(dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_BLUE_CLAMP;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_BLUE_CLAMP, true);
			if (placingZone.bLayerInd_BlueClamp == false) {
				DGDisableItem(dialogID, LABEL_LAYER_BLUE_CLAMP);
				DGDisableItem(dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_BLUE_TIMBER_RAIL;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, true);
			if (placingZone.bLayerInd_BlueTimberRail == false) {
				DGDisableItem(dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL);
				DGDisableItem(dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_RECTPIPE;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_RECTPIPE, true);
			if (placingZone.bLayerInd_Rectpipe == false) {
				DGDisableItem(dialogID, LABEL_LAYER_RECTPIPE);
				DGDisableItem(dialogID, USERCONTROL_LAYER_RECTPIPE);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_PINBOLT, true);
			if (placingZone.bLayerInd_Pinbolt == false) {
				DGDisableItem(dialogID, LABEL_LAYER_PINBOLT);
				DGDisableItem(dialogID, USERCONTROL_LAYER_PINBOLT);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_HEADPIECE;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_HEADPIECE, true);
			if (placingZone.bLayerInd_Headpiece == false) {
				DGDisableItem(dialogID, LABEL_LAYER_HEADPIECE);
				DGDisableItem(dialogID, USERCONTROL_LAYER_HEADPIECE);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_PROPS;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_PROPS, true);
			if (placingZone.bLayerInd_Props == false) {
				DGDisableItem(dialogID, LABEL_LAYER_PROPS);
				DGDisableItem(dialogID, USERCONTROL_LAYER_PROPS);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_JOIN;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_JOIN, true);
			if (placingZone.bLayerInd_Join == false) {
				DGDisableItem(dialogID, LABEL_LAYER_JOIN);
				DGDisableItem(dialogID, USERCONTROL_LAYER_JOIN);
			}

			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_LAYER_CROSS_JOINT_BAR;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR, true);
			if (placingZone.bLayerInd_CrossJointBar == false) {
				DGDisableItem(dialogID, LABEL_LAYER_CROSS_JOINT_BAR);
				DGDisableItem(dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);
			}

			break;

		case DG_MSG_CHANGE:
			if ((bool)DGGetItemValLong(dialogID, CHECKBOX_LAYER_COUPLING) == true) {
				long selectedLayer = DGGetItemValLong(dialogID, item);

				for (short i = USERCONTROL_LAYER_EUROFORM; i <= USERCONTROL_LAYER_CROSS_JOINT_BAR; i++)
					DGSetItemValLong(dialogID, i, selectedLayer);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
			case DG_OK:
				if (placingZone.bLayerInd_Euroform == true)			placingZone.layerInd_Euroform			= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_EUROFORM);
				if (placingZone.bLayerInd_Plywood == true)			placingZone.layerInd_Plywood			= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_PLYWOOD);
				if (placingZone.bLayerInd_Timber == true)			placingZone.layerInd_Timber				= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_TIMBER);
				if (placingZone.bLayerInd_Fillerspacer == true)		placingZone.layerInd_Fillerspacer		= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_FILLERSPACER);
				if (placingZone.bLayerInd_IncornerPanel == true)	placingZone.layerInd_IncornerPanel		= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
				if (placingZone.bLayerInd_OutcornerPanel == true)	placingZone.layerInd_OutcornerPanel		= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
				if (placingZone.bLayerInd_OutcornerAngle == true)	placingZone.layerInd_OutcornerAngle		= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
				if (placingZone.bLayerInd_BlueClamp == true)		placingZone.layerInd_BlueClamp			= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
				if (placingZone.bLayerInd_BlueTimberRail == true)	placingZone.layerInd_BlueTimberRail		= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);
				if (placingZone.bLayerInd_Rectpipe == true)			placingZone.layerInd_Rectpipe			= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_RECTPIPE);
				if (placingZone.bLayerInd_Pinbolt == true)			placingZone.layerInd_Pinbolt			= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_PINBOLT);
				if (placingZone.bLayerInd_Headpiece == true)		placingZone.layerInd_Headpiece			= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_HEADPIECE);
				if (placingZone.bLayerInd_Props == true)			placingZone.layerInd_Props				= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_PROPS);
				if (placingZone.bLayerInd_Join == true)				placingZone.layerInd_Join				= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_JOIN);
				if (placingZone.bLayerInd_CrossJointBar == true)	placingZone.layerInd_CrossJointBar		= (short)DGGetItemValLong(dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);

				break;

			case BUTTON_AUTOSET:
				item = 0;

				DGSetItemValLong(dialogID, CHECKBOX_LAYER_COUPLING, false);

				placingZone.layerInd_Euroform			= makeTemporaryLayer(structuralObject, "UFOM", NULL);
				placingZone.layerInd_Plywood			= makeTemporaryLayer(structuralObject, "PLYW", NULL);
				placingZone.layerInd_Timber				= makeTemporaryLayer(structuralObject, "TIMB", NULL);
				placingZone.layerInd_Fillerspacer		= makeTemporaryLayer(structuralObject, "FISP", NULL);
				placingZone.layerInd_IncornerPanel		= makeTemporaryLayer(structuralObject, "INCO", NULL);
				placingZone.layerInd_OutcornerPanel		= makeTemporaryLayer(structuralObject, "OUTP", NULL);
				placingZone.layerInd_OutcornerAngle		= makeTemporaryLayer(structuralObject, "OUTA", NULL);
				placingZone.layerInd_BlueClamp			= makeTemporaryLayer(structuralObject, "UFCL", NULL);
				placingZone.layerInd_BlueTimberRail		= makeTemporaryLayer(structuralObject, "RAIL", NULL);
				placingZone.layerInd_Rectpipe			= makeTemporaryLayer(structuralObject, "SPIP", NULL);
				placingZone.layerInd_Pinbolt			= makeTemporaryLayer(structuralObject, "PINB", NULL);
				placingZone.layerInd_Headpiece			= makeTemporaryLayer(structuralObject, "HEAD", NULL);
				placingZone.layerInd_Props				= makeTemporaryLayer(structuralObject, "PUSH", NULL);
				placingZone.layerInd_Join				= makeTemporaryLayer(structuralObject, "CLAM", NULL);
				placingZone.layerInd_CrossJointBar		= makeTemporaryLayer(structuralObject, "CROS", NULL);

				DGSetItemValLong(dialogID, USERCONTROL_LAYER_EUROFORM, placingZone.layerInd_Euroform);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_PLYWOOD, placingZone.layerInd_Plywood);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_TIMBER, placingZone.layerInd_Timber);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_FILLERSPACER, placingZone.layerInd_Fillerspacer);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_INCORNER_PANEL, placingZone.layerInd_IncornerPanel);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, placingZone.layerInd_OutcornerPanel);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, placingZone.layerInd_OutcornerAngle);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_BLUE_CLAMP, placingZone.layerInd_BlueClamp);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, placingZone.layerInd_BlueTimberRail);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_RECTPIPE, placingZone.layerInd_Rectpipe);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_PINBOLT, placingZone.layerInd_Pinbolt);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_HEADPIECE, placingZone.layerInd_Headpiece);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_PROPS, placingZone.layerInd_Props);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_JOIN, placingZone.layerInd_Join);
				DGSetItemValLong(dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR, placingZone.layerInd_CrossJointBar);

				break;
			}
			break;

		case DG_MSG_CLOSE:
			break;
		}

		result = item;

		return result;
	}

	short DGCALLBACK insulation_handler(short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */) {
		short	result;
		API_UCCallbackType	ucb;

		switch (message) {
		case DG_MSG_INIT:
			// 타이틀
			DGSetDialogTitle(dialogID, L"단열재 배치");

			// 라벨
			DGSetItemText(dialogID, LABEL_EXPLANATION_INS, L"단열재 규격을 입력하십시오.");
			DGSetItemText(dialogID, LABEL_INSULATION_THK, L"두께");
			DGSetItemText(dialogID, LABEL_INS_HORLEN, L"가로");
			DGSetItemText(dialogID, LABEL_INS_VERLEN, L"세로");

			// 체크박스
			DGSetItemText(dialogID, CHECKBOX_INS_LIMIT_SIZE, L"가로/세로 크기 제한");
			DGSetItemValLong(dialogID, CHECKBOX_INS_LIMIT_SIZE, true);

			// Edit 컨트롤
			DGSetItemValDouble(dialogID, EDITCONTROL_INS_HORLEN, 0.900);
			DGSetItemValDouble(dialogID, EDITCONTROL_INS_VERLEN, 1.800);

			// 레이어
			BNZeroMemory(&ucb, sizeof(ucb));
			ucb.dialogID = dialogID;
			ucb.type = APIUserControlType_Layer;
			ucb.itemID = USERCONTROL_INSULATION_LAYER;
			ACAPI_Interface(APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong(dialogID, USERCONTROL_INSULATION_LAYER, 1);

			// 버튼
			DGSetItemText(dialogID, DG_OK, L"확인");
			DGSetItemText(dialogID, DG_CANCEL, L"취소");

			// 두께는 자동
			DGSetItemValDouble(dialogID, EDITCONTROL_INSULATION_THK, placingZone.gap);
			DGDisableItem(dialogID, EDITCONTROL_INSULATION_THK);

			break;

		case DG_MSG_CHANGE:
			switch (item) {
			case CHECKBOX_INS_LIMIT_SIZE:
				if (DGGetItemValLong(dialogID, CHECKBOX_INS_LIMIT_SIZE) == TRUE) {
					DGEnableItem(dialogID, EDITCONTROL_INS_HORLEN);
					DGEnableItem(dialogID, EDITCONTROL_INS_VERLEN);
				}
				else {
					DGDisableItem(dialogID, EDITCONTROL_INS_HORLEN);
					DGDisableItem(dialogID, EDITCONTROL_INS_VERLEN);
				}
				break;
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
			case DG_OK:
				// 레이어 정보 저장
				insulElem.layerInd = (short)DGGetItemValLong(dialogID, USERCONTROL_INSULATION_LAYER);

				// 두께, 가로, 세로 저장
				insulElem.thk = DGGetItemValDouble(dialogID, EDITCONTROL_INSULATION_THK);
				insulElem.maxHorLen = DGGetItemValDouble(dialogID, EDITCONTROL_INS_HORLEN);
				insulElem.maxVerLen = DGGetItemValDouble(dialogID, EDITCONTROL_INS_VERLEN);
				if (DGGetItemValLong(dialogID, CHECKBOX_INS_LIMIT_SIZE) == TRUE)
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
}

using namespace namespaceWallTableform;

// 벽에 테이블폼 배치
GSErrCode	placeWallTableform(void)
{
	GSErrCode	err = NoError;

	// 작업 층 높이
	double	workLevel_wall;

	// Selection Manager 관련 변수
	GS::Array<API_Guid>		walls;
	GS::Array<API_Guid>		morphs;
	long					nWalls = 0;
	long					nMorphs = 0;

	// 객체 정보 가져오기
	API_Element			elem;
	API_ElementMemo		memo;
	API_ElemInfo3D		info3D;

	// 모프 객체 정보
	InfoMorph			infoMorph[2];
	InfoMorph			infoMorph_Basic;
	InfoMorph			infoMorph_Extra;

	// 선택한 요소 가져오기 (벽 1개, 모프 1~2개)
	GSErrCode	err1 = getGuidsOfSelection(&walls, API_WallID, &nWalls);
	GSErrCode	err2 = getGuidsOfSelection(&morphs, API_MorphID, &nMorphs);
	if (err1 == APIERR_NOSEL || err2 == APIERR_NOSEL) {
		DGAlert(DG_ERROR, L"오류", L"아무것도 선택하지 않았습니다.", L"필수선택: 벽(1개), 벽을 덮는 모프(1개)\n추가선택: 벽을 덮는 모프(뒷면 - 1차 모프와 높이가 다름)(1개)", L"확인", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	if (nWalls != 1) {
		DGAlert(DG_ERROR, L"오류", L"벽을 1개만 선택해야 합니다.", "", L"확인", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	if (nMorphs < 1 || nMorphs > 2) {
		DGAlert(DG_ERROR, L"오류", L"벽을 덮는 모프를 1개 또는 2개 선택해야 합니다.", "", L"확인", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// 벽 정보를 가져옴
	BNZeroMemory(&elem, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	elem.header.guid = walls.Pop();
	structuralObject = elem.header.guid;
	ACAPI_Element_Get(&elem);
	ACAPI_Element_GetMemo(elem.header.guid, &memo);

	if(elem.wall.thickness != elem.wall.thickness1) {
		DGAlert(DG_ERROR, L"오류", L"벽의 두께는 균일해야 합니다.", "", L"확인", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	infoWall.wallThk = elem.wall.thickness;
	infoWall.floorInd = elem.header.floorInd;
	infoWall.bottomOffset = elem.wall.bottomOffset;
	infoWall.begX = elem.wall.begC.x;
	infoWall.begY = elem.wall.begC.y;
	infoWall.endX = elem.wall.endC.x;
	infoWall.endY = elem.wall.endC.y;

	ACAPI_DisposeElemMemoHdls(&memo);

	// 모프 정보를 가져옴
	for (int i = 0; i < nMorphs; i++) {
		BNZeroMemory(&elem, sizeof(API_Element));
		elem.header.guid = morphs.Pop();
		ACAPI_Element_Get(&elem);
		ACAPI_Element_Get3DInfo(elem.header, &info3D);

		// 만약 모프가 세워져 있지 않으면 중단
		if (abs(info3D.bounds.zMax - info3D.bounds.zMin) < EPS) {
			DGAlert(DG_ERROR, L"오류", L"모프는 세워져 있어야 합니다.", "", L"확인", "", "");
			err = APIERR_GENERAL;
			return err;
		}

		// 모프의 GUID, 층 인덱스, 고도, 높이 저장
		infoMorph[i].guid = elem.header.guid;
		infoMorph[i].floorInd = elem.header.floorInd;
		infoMorph[i].level = elem.morph.level;
		infoMorph[i].height = info3D.bounds.zMax - info3D.bounds.zMin;

		// 모프의 좌하단, 우상단 점 지정
		if (abs(elem.morph.tranmat.tmx[11] - info3D.bounds.zMin) < EPS) {
			// 좌하단 좌표 결정
			infoMorph[i].leftBottomX = elem.morph.tranmat.tmx[3];
			infoMorph[i].leftBottomY = elem.morph.tranmat.tmx[7];
			infoMorph[i].leftBottomZ = elem.morph.tranmat.tmx[11];

			// 우상단 좌표는?
			if (abs(infoMorph[i].leftBottomX - info3D.bounds.xMin) < EPS)
				infoMorph[i].rightTopX = info3D.bounds.xMax;
			else
				infoMorph[i].rightTopX = info3D.bounds.xMin;
			if (abs(infoMorph[i].leftBottomY - info3D.bounds.yMin) < EPS)
				infoMorph[i].rightTopY = info3D.bounds.yMax;
			else
				infoMorph[i].rightTopY = info3D.bounds.yMin;
			if (abs(infoMorph[i].leftBottomZ - info3D.bounds.zMin) < EPS)
				infoMorph[i].rightTopZ = info3D.bounds.zMax;
			else
				infoMorph[i].rightTopZ = info3D.bounds.zMin;
		}
		else {
			// 우상단 좌표 결정
			infoMorph[i].rightTopX = elem.morph.tranmat.tmx[3];
			infoMorph[i].rightTopY = elem.morph.tranmat.tmx[7];
			infoMorph[i].rightTopZ = elem.morph.tranmat.tmx[11];

			// 좌하단 좌표는?
			if (abs(infoMorph[i].rightTopX - info3D.bounds.xMin) < EPS)
				infoMorph[i].leftBottomX = info3D.bounds.xMax;
			else
				infoMorph[i].leftBottomX = info3D.bounds.xMin;
			if (abs(infoMorph[i].rightTopY - info3D.bounds.yMin) < EPS)
				infoMorph[i].leftBottomY = info3D.bounds.yMax;
			else
				infoMorph[i].leftBottomY = info3D.bounds.yMin;
			if (abs(infoMorph[i].rightTopZ - info3D.bounds.zMin) < EPS)
				infoMorph[i].leftBottomZ = info3D.bounds.zMax;
			else
				infoMorph[i].leftBottomZ = info3D.bounds.zMin;
		}

		// 모프의 Z축 회전 각도 (벽의 설치 각도)
		double dx = infoMorph[i].rightTopX - infoMorph[i].leftBottomX;
		double dy = infoMorph[i].rightTopY - infoMorph[i].leftBottomY;
		infoMorph[i].ang = RadToDegree(atan2(dy, dx));

		// 모프의 가로/세로 길이
		infoMorph[i].horLen = GetDistance(info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.xMax, info3D.bounds.yMax);
		infoMorph[i].verLen = abs(info3D.bounds.zMax - info3D.bounds.zMin);

		// 모프 제거
		GS::Array<API_Element>	elems;
		elems.Push(elem);
		deleteElements(elems);
	}

	// 모프가 1개 또는 2개일 때
	if (nMorphs == 1) {
		placingZone.bExtra = false;

		infoMorph_Basic = infoMorph[0];
		infoMorph_Extra = infoMorph[0];
	}
	else {
		placingZone.bExtra = true;

		if (abs(infoMorph[1].verLen - infoMorph[0].verLen) > EPS) {
			infoMorph_Basic = infoMorph[0];
			infoMorph_Extra = infoMorph[1];
		}
		else {
			infoMorph_Basic = infoMorph[1];
			infoMorph_Extra = infoMorph[0];
		}
	}

	// 벽면 모프를 통해 배치 정보 업데이트
	placingZone.leftBottomX = infoMorph_Basic.leftBottomX;
	placingZone.leftBottomY = infoMorph_Basic.leftBottomY;
	placingZone.leftBottomZ = infoMorph_Basic.leftBottomZ;
	placingZone.horLen = infoMorph_Basic.horLen;
	placingZone.verLenBasic = infoMorph_Basic.verLen;
	placingZone.verLenExtra = infoMorph_Extra.verLen;
	placingZone.ang = DegreeToRad(infoMorph_Basic.ang);
	placingZone.wallThk = infoWall.wallThk;
	placingZone.floorInd = infoWall.floorInd;

	// 작업 층 높이 가져오기
	workLevel_wall = getWorkLevel(infoWall.floorInd);

	// 배치 정보의 고도 정보를 수정
	placingZone.leftBottomZ = infoWall.bottomOffset;

	// 초기 셀 개수 계산
	placingZone.nCellsHor = (int)floor(placingZone.horLen / DEFAULT_TABLEFORM_WIDTH);
	placingZone.nCellsVerBasic = (int)floor(placingZone.verLenBasic / DEFAULT_EUROFORM_HEIGHT);
	placingZone.nCellsVerExtra = (int)floor(placingZone.verLenExtra / DEFAULT_EUROFORM_HEIGHT);
	placingZone.nMarginCellsVerBasic = 1;
	placingZone.nMarginCellsVerExtra = 1;

	// 배치 정보 구축
	placingZone.initCells();

	bool exitCondition = false;
	int nSteps = 1;
	short result;

	bool bFrontTopMarginFill = false;
	bool bBackTopMarginFill = false;
	bool bInsulationFill = false;

	while (exitCondition == false) {
		if (nSteps == 1) {
			// 1번 다이얼로그: 폼 설정
			result = DGBlankModalDialog(550, 500, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, handler1, 0);
			if (result == DG_OK) {
				nSteps = 2;
				exitCondition = false;
			}
			else {
				nSteps = 0;
				exitCondition = true;
			}
		}

		if (nSteps == 2) {
			// 2번 다이얼로그: 상단 여백 채우기 (낮은쪽)
			bool bFront = true;
			result = DGBlankModalDialog(550, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, handler2, (DGUserData)&bFront);
			
			if (result == DG_OK) {
				nSteps = 3;
				exitCondition = false;
				bFrontTopMarginFill = true;
			}
			else if (result == DG_CANCEL) {
				nSteps = 3;
				exitCondition = false;
				bFrontTopMarginFill = false;
			}
			else if (result == DG2_BUTTON_PREV) {
				nSteps = 1;
				exitCondition = false;
			}
		}

		if (nSteps == 3) {
			// 2번 다이얼로그: 상단 여백 채우기 (높은쪽)
			bool bFront = false;
			result = DGBlankModalDialog(550, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, handler2, (DGUserData)&bFront);

			if (result == DG_OK) {
				nSteps = 4;
				exitCondition = false;
				bBackTopMarginFill = true;
			}
			else if (result == DG_CANCEL) {
				nSteps = 4;
				exitCondition = false;
				bBackTopMarginFill = false;
			}
			else if (result == DG2_BUTTON_PREV) {
				nSteps = 1;
				exitCondition = false;
			}
		}

		if (nSteps == 4) {
			// 레이어 설정 다이얼로그
			result = DGModalDialog (ACAPI_GetOwnResModule (), 32501, ACAPI_GetOwnResModule (), layer_handler, 0);
			
			if (result == DG_OK) {
				nSteps = 5;
				exitCondition = false;
			}
			else if (result == DG_CANCEL) {
				nSteps = 5;
				exitCondition = false;
			}
		}

		if (nSteps == 5) {
			// 단열재 채우기 다이얼로그
			if (placingZone.gap > EPS) {
				result = DGModalDialog(ACAPI_GetOwnResModule(), 32502, ACAPI_GetOwnResModule(), insulation_handler, 0);

				if (result == DG_OK) {
					nSteps = 6;
					exitCondition = true;
					bInsulationFill = true;
				}
				else if (result == DG_CANCEL) {
					nSteps = 6;
					exitCondition = true;
					bInsulationFill = false;
				}
			}
			else {
				nSteps = 6;
				exitCondition = true;
			}
		}
	}

	// 배치 정보 위치 정렬
	placingZone.alignPositions();

	// !!!
	placingZone.placeCells();
	// 1단계 다이얼로그 기반으로 객체 배치
	// 상단 여백 채우기 (낮은쪽) bFrontTopMarginFill
	// 상단 여백 채우기 (높은쪽) bBackTopMarginFill
	// 단열재 채우기 bInsulationFill

	return err;
}

#endif