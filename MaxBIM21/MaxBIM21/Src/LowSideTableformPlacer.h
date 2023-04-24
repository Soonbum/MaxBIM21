#ifndef	__LOW_SIDE_TABLEFORM_PLACER__
#define __LOW_SIDE_TABLEFORM_PLACER__

#include "MaxBIM21.h"

namespace lowSideTableformPlacerDG {
	// ���̾�α� �׸� �ε���
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
		NONE = 1,	// ����
		TABLEFORM,	// ���̺���
		EUROFORM,	// ������
		FILLERSP,	// �ٷ������̼�
		PLYWOOD,	// ����
		TIMBER		// ����
	};

	enum	objCornerType_forLowSideTableformPlacer {
		NOCORNER = 1,
		INCORNER_PANEL,
		OUTCORNER_PANEL,
		OUTCORNER_ANGLE
	};
}

// ���� ���� ����
struct InfoMorphForLowSideTableform
{
	API_Guid	guid;		// ������ GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	rightTopX;		// ���� ��ǥ X
	double	rightTopY;		// ���� ��ǥ Y
	double	rightTopZ;		// ���� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Degree, ȸ����: Z��)
};

// �׸��� �� �� ����
struct CellForLowSideTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	short	objType;		// ��ü Ÿ��: ����, ���̺���, ������, �ٷ������̼�, ����, ����
	
	int		horLen;			// ���� ����
	//int		verLen;			// ���� ���� (������� ����)

	// ���̺��� �� ������ ����
	int		tableInHor [10];	// ���� ����
};

// ���� ���� ���� ����
class LowSideTableformPlacingZone
{
public:
	double	leftBottomX;		// ���ϴ� ��ǥ X
	double	leftBottomY;		// ���ϴ� ��ǥ Y
	double	leftBottomZ;		// ���ϴ� ��ǥ Z

	double	horLen;				// ���� ����
	double	verLen;				// ���� ����
	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)

	bool	bVertical;			// ����: ���ι���(true), ���ι���(false)

	short	typeLcorner;		// ���� �ڳ� Ÿ��	: (1) ���� (2) ���ڳ��ǳ� (3) �ƿ��ڳ��ǳ� (4) �ƿ��ڳʾޱ�
	short	typeRcorner;		// ������ �ڳ� Ÿ��	: (1) ���� (2) ���ڳ��ǳ� (3) �ƿ��ڳ��ǳ� (4) �ƿ��ڳʾޱ�
	double	lenLcorner;			// ���� ���ڳ��ǳ�/�ƿ��ڳ��ǳ� ����
	double	lenRcorner;			// ������ ���ڳ��ǳ�/�ƿ��ڳ��ǳ� ����

	short	tableformType;		// ���̺��� Ÿ��: Ÿ��A (1), Ÿ��B (2), Ÿ��C (3)

	short	nCellsInHor;		// ���� ���� �� ����

	CellForLowSideTableform		cells [50];				// �� �迭 (���ڳ� ����)

	double	pipeVerticalLength;	// ���� ������ ����

public:
	// ���� ���� ���̺����� �ʺ� ���� (3600 ... 200)
	int	presetWidthVertical_tableform[65] = { 3600, 3500, 3450, 3400, 3350, 3300, 3250, 3200, 3150, 3100, 3050, 3000, 2950, 2900, 2850, 2800, 2750, 2700, 2650, 2600, 2550, 2500,
											  2450, 2400, 2350, 2300, 2250, 2200, 2150, 2100, 2050, 2000, 1950, 1900, 1850, 1800, 1750, 1700, 1650, 1600, 1550, 1500, 1450, 1400,
											  1350, 1300, 1250, 1200, 1150, 1100, 1050, 1000, 950, 900, 850, 800, 750, 700, 650, 600, 500, 450, 400, 300, 200 };
	// ���� ���� ���̺����� �ʺ� ���� (3600 ... 600)
	int	presetWidthHorizontal_tableform[11] = { 3600, 3300, 3000, 2700, 2400, 2100, 1800, 1500, 1200, 900, 600 };

	// ���� ���� �������� �ʺ� ���� (600 .. 200)
	int	presetWidthVertical_euroform[7] = { 600, 500, 450, 400, 300, 200, 0 };
	// ���� ���� �������� �ʺ� ���� (1200 ... 600)
	int	presetHeightHorizontal_euroform[4] = { 1200, 900, 600, 0 };

	// ���� ���� ���̺��� �� �������� �迭 ����
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
	// ���� ���� ���̺��� �� �������� �迭 ����
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
	void	initCells (LowSideTableformPlacingZone* placingZone, bool bVertical);					// �� ���� �ʱ�ȭ
	double	getCellPositionLeftBottomX (LowSideTableformPlacingZone* placingZone, short idx);		// ��(0-��� �ε��� ��ȣ)�� ���ϴ� �� ��ġ X ��ǥ�� ����
	void	adjustCellsPosition (LowSideTableformPlacingZone* placingZone);							// �� ��ġ�� �ٸ��� ������
	GSErrCode	placeObjects (LowSideTableformPlacingZone* placingZone);							// �� ������ ������� ��ü���� ��ġ��

	void	placeEuroformsOfTableform (LowSideTableformPlacingZone* placingZone, short idxCell);	// ���̺��� �� ������ ��ġ (����)
	void	placeTableformA (LowSideTableformPlacingZone* placingZone, short idxCell);				// ���̺��� Ÿ��A ��ġ (������ ����) - �������� 2��

public:
	// ���̾�α� ���� ��� �ε��� ��ȣ ����
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

GSErrCode	placeTableformOnLowSide (void);				// ���� ������ ���鿡 ���̺����� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK lowSideTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// ���̺���/������/�ٷ������̼�/����/���� ��ġ�� ���� ���̾�α� (���̺��� ����, ��� ����, ���� �� ����)
short DGCALLBACK lowSideTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// ��ü�� ���̾ �����ϱ� ���� ���̾�α�
short DGCALLBACK lowSideTableformPlacerHandler3_Vertical (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// ���̺��� ���ι��⿡ ���Ͽ� �������� ���� �迭�� �����ϱ� ���� ���̾�α�
short DGCALLBACK lowSideTableformPlacerHandler3_Horizontal (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// ���̺��� ���ι��⿡ ���Ͽ� �������� ���� �迭�� �����ϱ� ���� ���̾�α�

#endif