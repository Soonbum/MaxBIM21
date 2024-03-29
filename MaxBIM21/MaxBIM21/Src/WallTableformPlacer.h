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
		BRANCH_LABEL_OBJ_TYPE = 3,			// 라벨: 객체 종류
		BRANCH_POPUP_OBJ_TYPE,				// 객체 종류
		BRANCH_LABEL_TOTAL_WIDTH,			// 전체 너비
		BRANCH_EDITCONTROL_TOTAL_WIDTH,		// 전체 너비
		BRANCH_LABEL_GUIDE					// 라벨: 안내 텍스트
	};

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

		// 생성자
		PlacingZone()
		{
			this->leftBottomX = 0.0;
			this->leftBottomY = 0.0;
			this->leftBottomZ = 0.0;
			this->ang = 0.0;

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
		}

		// 배치 정보 구축
		void initCells()
		{
			// 모든 셀을 테이블폼A로 초기화
			for (int i = 0; i < 10; i++) {
				for (int j = 0; j < 50; j++) {
					// 앞면
					this->cellsBasic[i][j].horLen = 2.250;
					this->cellsBasic[i][j].verLen = 1.200;
					this->cellsBasic[i][j].objType = OBJ_WALL_TABLEFORM_A;
					this->cellsBasic[i][j].nCellsHor = 4;

					this->cellsBasic[i][j].cellHorLen[0] = 0.600;
					this->cellsBasic[i][j].cellHorLen[1] = 0.600;
					this->cellsBasic[i][j].cellHorLen[2] = 0.450;
					this->cellsBasic[i][j].cellHorLen[3] = 0.600;

					// 뒷면
					this->cellsExtra[i][j].horLen = 2.250;
					this->cellsExtra[i][j].verLen = 1.200;
					this->cellsExtra[i][j].objType = OBJ_WALL_TABLEFORM_A;
					this->cellsExtra[i][j].nCellsHor = 4;

					this->cellsExtra[i][j].cellHorLen[0] = 0.600;
					this->cellsExtra[i][j].cellHorLen[1] = 0.600;
					this->cellsExtra[i][j].cellHorLen[2] = 0.450;
					this->cellsExtra[i][j].cellHorLen[3] = 0.600;
				}
			}
		}
	};

	PlacingZone	placingZone;			// 배치 정보
	InfoWall	infoWall;				// 벽 객체 정보
	API_Guid	structuralObject;		// 구조 객체의 GUID

	double DEFAULT_TABLEFORM_WIDTH = 2.250;
	double DEFAULT_EUROFORM_HEIGHT = 1.200;

	int MAX_ROW = 10;
	int MAX_COL = 50;

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

	// 항목 인덱스 (2차 다이얼로그)
	short BRANCH_GRID_START_INDEX;					// 그리드
	short BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX;	// 객체 너비
	short BRANCH_BUTTON_ADD_COLUMN;					// 열 방향 객체 추가 버튼
	short BRANCH_BUTTON_DEL_COLUMN;					// 열 방향 객체 삭제 버튼

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
			if (placingZone.cellsBasic[0][*x].nCellsHor > 3)
				DGSetDialogSize(dialogID, DG_CLIENT, 500 + ((placingZone.cellsBasic[0][*x].nCellsHor - 3) * sizeX), 300, DG_TOPLEFT, true);

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
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 63.5");
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
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 63.5");
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
			if (placingZone.cellsBasic[0][*x].nCellsHor > 3)
				DGSetDialogSize(dialogID, DG_CLIENT, 500 + ((placingZone.cellsBasic[0][*x].nCellsHor - 3) * sizeX), 300, DG_TOPLEFT, true);

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
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 63.5");
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
							DGSetItemText(dialogID, LABEL_GUIDE, L"입력 가능 치수: 63.5");
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
			DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_HEIGHT, placingZone.verLenBasic);
			if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1)
				remainHeight = placingZone.verLenBasic;
			else
				remainHeight = placingZone.verLenExtra;
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

	short DGCALLBACK handler2(short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */) {
		//int* x = (int*)userData;

		short	result;
		short	itemIndex;
		short	posX, posY;
		short	sizeX, sizeY;
		double	totalWidth;

		switch (message) {
		case DG_MSG_INIT:
			//DGSetDialogTitle(dialogID, L"벽에 테이블폼 배치 - 셀 설정");

			//// 확인 버튼
			//DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 15, 70, 25);
			//DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText(dialogID, DG_OK, L"확인");
			//DGShowItem(dialogID, DG_OK);

			//// 취소
			//DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 70, 25);
			//DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText(dialogID, DG_CANCEL, L"취소");
			//DGShowItem(dialogID, DG_CANCEL);

			//// 객체 종류
			//itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 20, 55, 23);
			//DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText(dialogID, itemIndex, L"객체 종류");
			//DGShowItem(dialogID, itemIndex);

			//itemIndex = DGAppendDialogItem(dialogID, DG_ITM_POPUPCONTROL, 200, 0, 160, 15, 150, 25);
			//DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			//for (int i = 1; i < 14; i++) {
			//	DGPopUpInsertItem(dialogID, itemIndex, DG_POPUP_BOTTOM);
			//	DGPopUpSetItemText(dialogID, itemIndex, DG_POPUP_BOTTOM, objTypeStr[i]);
			//}
			//DGShowItem(dialogID, itemIndex);

			//// 전체 너비
			//itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 50, 55, 23);
			//DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText(dialogID, itemIndex, L"전체 너비");
			//DGShowItem(dialogID, itemIndex);

			//itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 160, 45, 85, 25);
			//DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			//DGShowItem(dialogID, itemIndex);
			//DGDisableItem(dialogID, itemIndex);

			//// 안내 텍스트
			//itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 80, 300, 23);
			//DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText(dialogID, itemIndex, L"안내 텍스트입니다.");
			//DGShowItem(dialogID, itemIndex);

			//// 셀 그리드 표시
			//posX = 100; posY = 115;
			//sizeX = 100; sizeY = 100;
			//for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {
			//	itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (i * sizeX), posY, sizeX, sizeY);
			//	DGShowItem(dialogID, itemIndex);
			//	if (i == 0)
			//		BRANCH_GRID_START_INDEX = itemIndex;
			//}

			//// 셀 너비 (Edit컨트롤)
			//for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {
			//	itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX + (i * sizeX), posY + 100, sizeX, 25);
			//	DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGShowItem(dialogID, itemIndex);
			//	if (i == 0)
			//		BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX = itemIndex;
			//}

			//// 열 방향 객체 추가 버튼
			//itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX), posY + 100, 25, 25);
			//DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			//DGSetItemText(dialogID, itemIndex, "+");
			//DGShowItem(dialogID, itemIndex);
			//BRANCH_BUTTON_ADD_COLUMN = itemIndex;

			//// 열 방향 객체 삭제 버튼
			//itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX) + 25, posY + 100, 25, 25);
			//DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			//DGSetItemText(dialogID, itemIndex, "-");
			//DGShowItem(dialogID, itemIndex);
			//BRANCH_BUTTON_DEL_COLUMN = itemIndex;

			//// 다이얼로그 창 크기 조절
			//if (placingZone.cellsBasic[0][*x].nCellsHor > 3)
			//	DGSetDialogSize(dialogID, DG_CLIENT, 500 + ((placingZone.cellsBasic[0][*x].nCellsHor - 3) * sizeX), 300, DG_TOPLEFT, true);

			//// 기본값 설정
			//DGPopUpSelectItem(dialogID, BRANCH_POPUP_OBJ_TYPE, placingZone.cellsBasic[0][*x].objType);				// 팝업컨트롤
			//DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH, placingZone.cellsBasic[0][*x].horLen);		// 전체 너비
			//for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {										// 셀 너비
			//	DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + i, placingZone.cellsBasic[0][*x].cellHorLen[i]);
			//}

			//// 테이블폼의 경우
			//if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_WALL_TABLEFORM_A) ||
			//	(DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_WALL_TABLEFORM_B) ||
			//	(DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_WALL_TABLEFORM_C)) {

			//	totalWidth = 0.0;
			//	for (int i = BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX; i < (BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + placingZone.cellsBasic[0][*x].nCellsHor); i++) {
			//		DGEnableItem(dialogID, i);
			//		totalWidth += DGGetItemValDouble(dialogID, i);
			//	}
			//	DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH, totalWidth);
			//	DGDisableItem(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);
			//}
			//// 그 외의 경우
			//else {
			//	for (int i = BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX; i < (BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + placingZone.cellsBasic[0][*x].nCellsHor); i++) {
			//		DGDisableItem(dialogID, i);
			//	}
			//	DGEnableItem(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);
			//}

			break;

		case DG_MSG_FOCUS:
			//if (item == BRANCH_EDITCONTROL_TOTAL_WIDTH) {
			//	if (placingZone.bVertical == true) {
			//		if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_EUROFORM) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 600, 500, 450, 400, 300, 200");
			//		}
			//		else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_PLYWOOD) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 100 ~ 1220");
			//		}
			//		else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_TIMBER) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 5 ~ 1000");
			//		}
			//		else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_FILLERSPACER) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 10 ~ 50");
			//		}
			//		else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_INCORNER_PANEL_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_INCORNER_PANEL_R)) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 80 ~ 500");
			//		}
			//		else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_PANEL_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_PANEL_R)) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 80 ~ 500");
			//		}
			//		else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_ANGLE_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_ANGLE_R)) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 63.5");
			//		}
			//	}
			//	else {
			//		if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_EUROFORM) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 1200, 900, 600");
			//		}
			//		else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_PLYWOOD) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 100 ~ 2440");
			//		}
			//		else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_TIMBER) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 5 ~ 1000");
			//		}
			//		else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_FILLERSPACER) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 10 ~ 50");
			//		}
			//		else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_INCORNER_PANEL_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_INCORNER_PANEL_R)) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 80 ~ 500");
			//		}
			//		else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_PANEL_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_PANEL_R)) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 80 ~ 500");
			//		}
			//		else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_ANGLE_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_ANGLE_R)) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 63.5");
			//		}
			//	}
			//}

			//for (int i = BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX; i < (BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + placingZone.cellsBasic[0][*x].nCellsHor); i++) {
			//	if (item == i) {
			//		if (placingZone.bVertical == true) {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 600, 500, 450, 400, 300, 200");
			//		}
			//		else {
			//			DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"입력 가능 치수: 1200, 900, 600");
			//		}
			//	}
			//}

			break;

		case DG_MSG_CHANGE:
			//// 테이블폼의 경우
			//if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_WALL_TABLEFORM_A) ||
			//	(DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_WALL_TABLEFORM_B) ||
			//	(DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_WALL_TABLEFORM_C)) {

			//	totalWidth = 0.0;
			//	for (int i = BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX; i < (BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + placingZone.cellsBasic[0][*x].nCellsHor); i++) {
			//		DGEnableItem(dialogID, i);
			//		totalWidth += DGGetItemValDouble(dialogID, i);
			//	}
			//	DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH, totalWidth);
			//	DGDisableItem(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);
			//}
			//// 그 외의 경우
			//else {
			//	for (int i = BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX; i < (BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + placingZone.cellsBasic[0][*x].nCellsHor); i++) {
			//		DGDisableItem(dialogID, i);
			//	}
			//	DGEnableItem(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);
			//}

			break;

		case DG_MSG_CLICK:
			//if (item == DG_OK) {
			//	for (int i = 0; i < MAX_ROW; i++) {
			//		// 셀 너비
			//		for (int j = 0; j < placingZone.cellsBasic[i][*x].nCellsHor; j++) {
			//			placingZone.cellsBasic[i][*x].cellHorLen[j] = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + j);
			//			placingZone.cellsExtra[i][*x].cellHorLen[j] = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + j);
			//		}

			//		// 전체 너비
			//		placingZone.cellsBasic[i][*x].horLen = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);
			//		placingZone.cellsExtra[i][*x].horLen = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);

			//		// 객체 타입
			//		placingZone.cellsBasic[i][*x].objType = DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE);
			//		placingZone.cellsExtra[i][*x].objType = DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE);
			//	}
			//	break;
			//}

			//// 열 추가 버튼
			//if (item == BRANCH_BUTTON_ADD_COLUMN) {
			//	item = 0;
			//	for (int i = 0; i < MAX_ROW; i++) {
			//		placingZone.cellsBasic[i][*x].nCellsHor++;
			//		placingZone.cellsExtra[i][*x].nCellsHor++;
			//	}
			//	break;
			//}

			//// 열 삭제 버튼
			//if (item == BRANCH_BUTTON_DEL_COLUMN) {
			//	item = 0;
			//	for (int i = 0; i < MAX_ROW; i++) {
			//		placingZone.cellsBasic[i][*x].nCellsHor--;
			//		placingZone.cellsExtra[i][*x].nCellsHor--;
			//	}
			//	break;
			//}

			//// ---------- 동적 요소 새로 그리기
			//// 동적 요소 모두 제거
			//DGRemoveDialogItems(dialogID, BRANCH_GRID_START_INDEX);

			//// 셀 그리드 표시
			//posX = 100; posY = 115;
			//sizeX = 100; sizeY = 100;
			//for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {
			//	itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (i * sizeX), posY, sizeX, sizeY);
			//	DGShowItem(dialogID, itemIndex);
			//	if (i == 0)
			//		BRANCH_GRID_START_INDEX = itemIndex;
			//}

			//// 셀 너비 (Edit컨트롤)
			//for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {
			//	itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX + (i * sizeX), posY + 100, sizeX, 25);
			//	DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGShowItem(dialogID, itemIndex);
			//	if (i == 0)
			//		BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX = itemIndex;
			//}

			//// 열 방향 객체 추가 버튼
			//itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX), posY + 100, 25, 25);
			//DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			//DGSetItemText(dialogID, itemIndex, "+");
			//DGShowItem(dialogID, itemIndex);
			//BRANCH_BUTTON_ADD_COLUMN = itemIndex;

			//// 열 방향 객체 삭제 버튼
			//itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX) + 25, posY + 100, 25, 25);
			//DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			//DGSetItemText(dialogID, itemIndex, "-");
			//DGShowItem(dialogID, itemIndex);
			//BRANCH_BUTTON_DEL_COLUMN = itemIndex;

			//// 다이얼로그 창 크기 조절
			//if (placingZone.cellsBasic[0][*x].nCellsHor > 3)
			//	DGSetDialogSize(dialogID, DG_CLIENT, 500 + ((placingZone.cellsBasic[0][*x].nCellsHor - 3) * sizeX), 300, DG_TOPLEFT, true);

			//// 기본값 설정
			//DGPopUpSelectItem(dialogID, BRANCH_POPUP_OBJ_TYPE, placingZone.cellsBasic[0][*x].objType);				// 팝업컨트롤
			//DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH, placingZone.cellsBasic[0][*x].horLen);		// 전체 너비
			//for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {										// 셀 너비
			//	DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + i, placingZone.cellsBasic[0][*x].cellHorLen[i]);
			//}
			//// ---------- 동적 요소 새로 그리기

			break;

		case DG_MSG_CLOSE:
			break;
		}

		result = item;

		return result;
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

	// 작업 층 높이 가져오기
	workLevel_wall = getWorkLevel(infoWall.floorInd);

	// 배치 정보의 고도 정보를 수정
	placingZone.leftBottomZ = infoWall.bottomOffset;

	// 초기 셀 개수 계산
	placingZone.nCellsHor = (int)floor(placingZone.horLen / DEFAULT_TABLEFORM_WIDTH);
	placingZone.nCellsVerBasic = (int)floor(placingZone.verLenBasic / DEFAULT_EUROFORM_HEIGHT);
	placingZone.nCellsVerExtra = (int)floor(placingZone.verLenExtra / DEFAULT_EUROFORM_HEIGHT);

	// 배치 정보 구축
	placingZone.initCells();

	bool exitCondition = false;
	int nSteps = 1;
	short result;

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
			//result = DGBlankModalDialog(500, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, handler2, 0);
			
			// -> DG_OK이면 nSteps = 3, exitCondition = false
			// -> DG_CANCEL이면 nSteps = 3, exitCondition = false
			// -> 이전 버튼을 누르면 nSteps = 1, exitCondition = false
			exitCondition = true;
		}

		if (nSteps == 3) {
			// 3번 다이얼로그: 상단 여백 채우기 (높은쪽) [bExtra == true일때]
			// -> DG_OK이면 nSteps = 4, exitCondition = false
			// -> DG_CANCEL이면 nSteps = 4, exitCondition = false
			// -> 이전 버튼을 누르면 nSteps = 1, exitCondition = false
		}

		if (nSteps == 4) {
			// 4번 다이얼로그: 단열재 채우기 [gap > EPS일때]
			// -> DG_OK이면 nSteps = 5, exitCondition = false
			// -> DG_CANCEL이면 nSteps = 5, exitCondition = false
			// -> 이전 버튼을 누르면 nSteps = 2, exitCondition = false
		}

		if (nSteps == 5) {
			// 5번 다이얼로그: 레이어 설정
			// -> DG_OK이면 nSteps = 0, exitCondition = true
			// -> DG_CANCEL이면 err = NoError, return err
			// -> 이전 버튼을 누르면 nSteps  = 4, exitCondition = false
		}
	}

	return err;
}

#endif