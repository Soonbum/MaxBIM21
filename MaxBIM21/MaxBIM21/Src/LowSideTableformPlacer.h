#ifndef	__LOW_SIDE_TABLEFORM_PLACER__
#define __LOW_SIDE_TABLEFORM_PLACER__

#include "MaxBIM21.h"

namespace lowSideTableformPlacerDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_1_forLowSideTableformPlacer {
		CHECKBOX_LAYER_COUPLING = 3,

		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_RECTPIPE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_JOIN,
		LABEL_LAYER_HEADPIECE,
		LABEL_LAYER_PROPS,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_TIMBER,
		LABEL_LAYER_FILLERSP,
		LABEL_LAYER_OUTCORNER_ANGLE,
		LABEL_LAYER_OUTCORNER_PANEL,
		LABEL_LAYER_INCORNER_PANEL,
		LABEL_LAYER_RECTPIPE_HANGER,
		LABEL_LAYER_EUROFORM_HOOK,
		LABEL_LAYER_CROSS_JOINT_BAR,
		LABEL_LAYER_BLUE_CLAMP,
		LABEL_LAYER_BLUE_TIMBER_RAIL,
		LABEL_LAYER_18,
		LABEL_LAYER_19,
		LABEL_LAYER_20,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_RECTPIPE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_JOIN,
		USERCONTROL_LAYER_HEADPIECE,
		USERCONTROL_LAYER_PROPS,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_TIMBER,
		USERCONTROL_LAYER_FILLERSP,
		USERCONTROL_LAYER_OUTCORNER_ANGLE,
		USERCONTROL_LAYER_OUTCORNER_PANEL,
		USERCONTROL_LAYER_INCORNER_PANEL,
		USERCONTROL_LAYER_RECTPIPE_HANGER,
		USERCONTROL_LAYER_EUROFORM_HOOK,
		USERCONTROL_LAYER_CROSS_JOINT_BAR,
		USERCONTROL_LAYER_BLUE_CLAMP,
		USERCONTROL_LAYER_BLUE_TIMBER_RAIL,
		USERCONTROL_LAYER_18,
		USERCONTROL_LAYER_19,
		USERCONTROL_LAYER_20,

		BUTTON_AUTOSET
	};

	enum	tableformOrientation_forLowSideTableformPlacer {
		VERTICAL = 1,
		HORIZONTAL
	};

	enum	objCellType_forLowSideTableformPlacer {
		NONE = 1,	// 없음
		TABLEFORM,	// 테이블폼
		EUROFORM,	// 유로폼
		FILLERSP,	// 휠러스페이서
		PLYWOOD,	// 합판
		TIMBER		// 각재
	};

	enum	objCornerType_forLowSideTableformPlacer {
		NOCORNER = 1,
		INCORNER_PANEL,
		OUTCORNER_PANEL,
		OUTCORNER_ANGLE
	};
}

// 모프 관련 정보
struct InfoMorphForLowSideTableform
{
	API_Guid	guid;		// 모프의 GUID

	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	rightTopX;		// 우상단 좌표 X
	double	rightTopY;		// 우상단 좌표 Y
	double	rightTopZ;		// 우상단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Degree, 회전축: Z축)
};

// 그리드 각 셀 정보
struct CellForLowSideTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	short	objType;		// 객체 타입: 없음, 테이블폼, 유로폼, 휠러스페이서, 합판, 각재
	
	int		horLen;			// 가로 길이
	//int		verLen;			// 세로 길이 (사용하지 않음)

	// 테이블폼 내 유로폼 길이
	int		tableInHor [10];	// 가로 방향
};

// 낮은 측면 영역 정보
class LowSideTableformPlacingZone
{
public:
	double	leftBottomX;		// 좌하단 좌표 X
	double	leftBottomY;		// 좌하단 좌표 Y
	double	leftBottomZ;		// 좌하단 좌표 Z

	double	horLen;				// 가로 길이
	double	verLen;				// 세로 길이
	double	ang;				// 회전 각도 (단위: Radian, 회전축: Z축)

	bool	bVertical;			// 방향: 세로방향(true), 가로방향(false)

	short	typeLcorner;		// 왼쪽 코너 타입	: (1) 없음 (2) 인코너판넬 (3) 아웃코너판넬 (4) 아웃코너앵글
	short	typeRcorner;		// 오른쪽 코너 타입	: (1) 없음 (2) 인코너판넬 (3) 아웃코너판넬 (4) 아웃코너앵글
	double	lenLcorner;			// 왼쪽 인코너판넬/아웃코너판넬 길이
	double	lenRcorner;			// 오른쪽 인코너판넬/아웃코너판넬 길이

	short	tableformType;		// 테이블폼 타입: 타입A (1), 타입B (2), 타입C (3)

	short	nCellsInHor;		// 수평 방향 셀 개수

	CellForLowSideTableform		cells [50];				// 셀 배열 (인코너 제외)

	double	pipeVerticalLength;	// 수직 파이프 길이

public:
	// 세로 방향 테이블폼의 너비 모음 (3600 ... 200)
	int	presetWidthVertical_tableform[65] = { 3600, 3500, 3450, 3400, 3350, 3300, 3250, 3200, 3150, 3100, 3050, 3000, 2950, 2900, 2850, 2800, 2750, 2700, 2650, 2600, 2550, 2500,
											  2450, 2400, 2350, 2300, 2250, 2200, 2150, 2100, 2050, 2000, 1950, 1900, 1850, 1800, 1750, 1700, 1650, 1600, 1550, 1500, 1450, 1400,
											  1350, 1300, 1250, 1200, 1150, 1100, 1050, 1000, 950, 900, 850, 800, 750, 700, 650, 600, 500, 450, 400, 300, 200 };
	// 가로 방향 테이블폼의 너비 모음 (3600 ... 600)
	int	presetWidthHorizontal_tableform[11] = { 3600, 3300, 3000, 2700, 2400, 2100, 1800, 1500, 1200, 900, 600 };

	// 세로 방향 유로폼의 너비 모음 (600 .. 200)
	int	presetWidthVertical_euroform[7] = { 600, 500, 450, 400, 300, 200, 0 };
	// 가로 방향 유로폼의 너비 모음 (1200 ... 600)
	int	presetHeightHorizontal_euroform[4] = { 1200, 900, 600, 0 };

	// 세로 방향 테이블폼 내 유로폼의 배열 순서
	int	presetWidth_config_vertical[65][7] =
		{
			6, 600, 600, 600, 600, 600, 600,		// 3600
			6, 600, 600, 600, 600, 500, 600,		// 3500
			6, 600, 600, 600, 600, 450, 600,		// 3450
			6, 600, 600, 600, 600, 400, 600,		// 3400
			6, 600, 600, 600, 500, 450, 600,		// 3350
			6, 600, 600, 600, 600, 300, 600,		// 3300
			6, 600, 600, 600, 450, 400, 600,		// 3250
			6, 600, 600, 600, 600, 200, 600,		// 3200
			6, 600, 600, 600, 450, 300, 600,		// 3150
			6, 600, 600, 600, 400, 300, 600,		// 3100
			6, 600, 600, 600, 450, 200, 600,		// 3050
			5, 600, 600, 600, 600, 600, 0,			// 3000
			6, 600, 600, 600, 400, 300, 450,		// 2950
			6, 600, 600, 600, 450, 200, 450,		// 2900
			5, 600, 600, 600, 600, 450, 0,			// 2850
			5, 600, 600, 600, 600, 400, 0,			// 2800
			6, 600, 600, 600, 450, 200, 300,		// 2750
			5, 600, 600, 600, 600, 300, 0,			// 2700
			5, 600, 600, 600, 400, 450, 0,			// 2650
			5, 600, 600, 600, 500, 300, 0,			// 2600
			5, 600, 600, 600, 450, 300, 0,			// 2550
			5, 600, 600, 600, 400, 300, 0,			// 2500
			5, 600, 600, 600, 200, 450, 0,			// 2450
			4, 600, 600, 600, 600, 0, 0,			// 2400
			5, 600, 600, 450, 400, 300, 0,			// 2350
			4, 600, 600, 500, 600, 0, 0,			// 2300
			4, 600, 600, 450, 600, 0, 0,			// 2250
			4, 600, 600, 400, 600, 0, 0,			// 2200
			4, 600, 500, 450, 600, 0, 0,			// 2150
			4, 600, 600, 300, 600, 0, 0,			// 2100
			4, 600, 450, 400, 600, 0, 0,			// 2050
			4, 600, 600, 200, 600, 0, 0,			// 2000
			4, 600, 450, 300, 600, 0, 0,			// 1950
			4, 600, 400, 300, 600, 0, 0,			// 1900
			4, 600, 450, 200, 600, 0, 0,			// 1850
			3, 600, 600, 600, 0, 0, 0,				// 1800
			4, 600, 400, 450, 300, 0, 0,			// 1750
			3, 600, 500, 600, 0, 0, 0,				// 1700
			3, 600, 450, 600, 0, 0, 0,				// 1650
			3, 600, 400, 600, 0, 0, 0,				// 1600
			4, 600, 450, 200, 300, 0, 0,			// 1550
			3, 600, 300, 600, 0, 0, 0,				// 1500
			3, 600, 400, 450, 0, 0, 0,				// 1450
			3, 600, 200, 600, 0, 0, 0,				// 1400
			3, 600, 300, 450, 0, 0, 0,				// 1350
			3, 600, 400, 300, 0, 0, 0,				// 1300
			3, 600, 200, 450, 0, 0, 0,				// 1250
			2, 600, 600, 0, 0, 0, 0,				// 1200
			3, 450, 400, 300, 0, 0, 0,				// 1150
			3, 400, 400, 300, 0, 0, 0,				// 1100
			3, 450, 300, 300, 0, 0, 0,				// 1050
			2, 600, 400, 0, 0, 0, 0,				// 1000
			2, 500, 450, 0, 0, 0, 0,				// 950
			2, 600, 300, 0, 0, 0, 0,				// 900
			2, 400, 450, 0, 0, 0, 0,				// 850
			2, 400, 400, 0, 0, 0, 0,				// 800
			2, 450, 300, 0, 0, 0, 0,				// 750
			2, 400, 300, 0, 0, 0, 0,				// 700
			2, 450, 200, 0, 0, 0, 0,				// 650
			1, 600, 0, 0, 0, 0, 0,					// 600
			1, 500, 0, 0, 0, 0, 0,					// 500
			1, 450, 0, 0, 0, 0, 0,					// 450
			1, 400, 0, 0, 0, 0, 0,					// 400
			1, 300, 0, 0, 0, 0, 0,					// 300
			1, 200, 0, 0, 0, 0, 0					// 200
		};
	// 가로 방향 테이블폼 내 유로폼의 배열 순서
	int	presetWidth_config_horizontal[11][4] =
		{
			3, 1200, 1200, 1200,	// 3600
			3, 1200, 1200, 900,		// 3300
			3, 1200, 1200, 600,		// 3000
			3, 1200, 900, 600,		// 2700
			2, 1200, 1200, 0,		// 2400
			2, 1200, 900, 0,		// 2100
			2, 900, 900, 0,			// 1800
			2, 900, 600, 0,			// 1500
			1, 1200, 0, 0,			// 1200
			1, 900, 0, 0,			// 900
			1, 600, 0, 0			// 600
		};

public:
	void	initCells (LowSideTableformPlacingZone* placingZone, bool bVertical);					// 셀 정보 초기화
	double	getCellPositionLeftBottomX (LowSideTableformPlacingZone* placingZone, short idx);		// 셀(0-기반 인덱스 번호)의 좌하단 점 위치 X 좌표를 구함
	void	adjustCellsPosition (LowSideTableformPlacingZone* placingZone);							// 셀 위치를 바르게 교정함
	GSErrCode	placeObjects (LowSideTableformPlacingZone* placingZone);							// 셀 정보를 기반으로 객체들을 배치함

	void	placeEuroformsOfTableform (LowSideTableformPlacingZone* placingZone, short idxCell);	// 테이블폼 내 유로폼 배치 (공통)
	void	placeTableformA (LowSideTableformPlacingZone* placingZone, short idxCell);				// 테이블폼 타입A 배치 (유로폼 제외) - 각파이프 2줄

public:
	// 다이얼로그 동적 요소 인덱스 번호 저장
	short	POPUP_DIRECTION;
	short	POPUP_TABLEFORM_TYPE;
	short	POPUP_TABLEFORM_HEIGHT;
	short	BUTTON_ADD_HOR;
	short	BUTTON_DEL_HOR;
	short	EDITCONTROL_REMAIN_WIDTH;
	short	EDITCONTROL_CURRENT_HEIGHT;
	short	BUTTON_LCORNER;
	short	POPUP_OBJ_TYPE_LCORNER;
	short	EDITCONTROL_WIDTH_LCORNER;
	short	BUTTON_RCORNER;
	short	POPUP_OBJ_TYPE_RCORNER;
	short	EDITCONTROL_WIDTH_RCORNER;
	short	BUTTON_OBJ [50];
	short	POPUP_OBJ_TYPE [50];
	short	POPUP_WIDTH [50];
	short	EDITCONTROL_WIDTH [50];
	short	EDITCONTROL_VPIPE_LENGTH;

	short	LABEL_TOTAL_WIDTH;
	short	POPUP_WIDTH_IN_TABLE [10];
};

GSErrCode	placeTableformOnLowSide (void);				// 낮은 슬래브 측면에 테이블폼을 배치하는 통합 루틴
short DGCALLBACK lowSideTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// 테이블폼/유로폼/휠러스페이서/합판/목재 배치를 위한 다이얼로그 (테이블폼 구성, 요소 방향, 개수 및 길이)
short DGCALLBACK lowSideTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// 객체의 레이어를 선택하기 위한 다이얼로그
short DGCALLBACK lowSideTableformPlacerHandler3_Vertical (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 테이블폼 세로방향에 대하여 유로폼의 수평 배열을 변경하기 위한 다이얼로그
short DGCALLBACK lowSideTableformPlacerHandler3_Horizontal (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 테이블폼 가로방향에 대하여 유로폼의 수평 배열을 변경하기 위한 다이얼로그

#endif