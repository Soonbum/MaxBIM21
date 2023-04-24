#ifndef	__WALL_TABLEFORM_PLACER__
#define __WALL_TABLEFORM_PLACER__

#include "MaxBIM21.h"

namespace wallTableformPlacerDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_1_forWallTableform {
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

	enum	idxItems_2_forWallTableform {
		LABEL_DESC1_TOPREST = 3,
		LABEL_HEIGHT_TOPREST,
		EDITCONTROL_HEIGHT_TOPREST,
		LABEL_DESC2_TOPREST,
		LABEL_UP_TOPREST,
		LABEL_ARROWUP_TOPREST,
		LABEL_DOWN_TOPREST,
		CHECKBOX_FORM_ONOFF_1_TOPREST,
		CHECKBOX_FORM_ONOFF_2_TOPREST,
		LABEL_PLYWOOD_TOPREST,
		CHECKBOX_SET_STANDARD_1_TOPREST,
		CHECKBOX_SET_STANDARD_2_TOPREST,
		POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST,
		POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST,
		EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST,
		EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST,
		EDITCONTROL_PLYWOOD_TOPREST
	};

	enum	tableformOrientation_forWallTableformPlacer {
		VERTICAL = 1,
		HORIZONTAL
	};

	enum	objType_forWallTableformPlacer {
		NONE = 1,	// ����
		TABLEFORM,	// ���̺���
		EUROFORM,	// ������
		FILLERSP,	// �ٷ������̼�
		PLYWOOD,	// ����
		TIMBER		// ����
	};

	enum	objCornerType_forWallTableformPlacer {
		NOCORNER = 1,
		INCORNER_PANEL,
		OUTCORNER_PANEL,
		OUTCORNER_ANGLE
	};

	enum	PushPullProps_InstallationType_forWallTableformPlacer {
		PUSHPULLPROPS_NONE = 1,
		PUSHPULLPROPS_INNER,
		PUSHPULLPROPS_OUTER,
		PUSHPULLPROPS_INOUT
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
}

// �ܿ���
struct insulElemForWallTableform
{
	short	layerInd;		// ���̾� �ε���
	double	thk;			// �β�
	bool	bLimitSize;		// ����/���� ũ�� ����
	double	maxHorLen;		// ���� �ִ� ����
	double	maxVerLen;		// ���� �ִ� ����
};

// ���� ���� ����
struct InfoMorphForWallTableform
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
struct CellForWallTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	short	objType;		// ��ü Ÿ��: ����, ���̺���, ������, �ٷ������̼�, ����, ����
	
	int		horLen;			// ���� ����
	int		verLenBasic;	// ���� ���� (������)
	int		verLenExtra;	// ���� ���� (������)

	// ���̺��� �� ������ ����
	int		tableInHor [10];		// ���� ����
	int		tableInVerBasic [10];	// ���� ���� (������)
	int		tableInVerExtra [10];	// ���� ���� (������)
};

// �׸��� �� ��/�ϴ� �� ����
struct MarginCellForWallTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	bool	bFill;			// ä��� ����

	bool	bEuroform1;				// 1�� ������ ����
	bool	bEuroformStandard1;		// 1�� �������� �԰������� ����
	double	formWidth1;				// 1�� �������� ��
	bool	bEuroform2;				// 2�� ������ ����
	bool	bEuroformStandard2;		// 2�� �������� �԰������� ����
	double	formWidth2;				// 2�� �������� ��
};

// ���� ���� ���� (����)
class WallTableformPlacingZone
{
public:
	double	leftBottomX;		// ���ϴ� ��ǥ X
	double	leftBottomY;		// ���ϴ� ��ǥ Y
	double	leftBottomZ;		// ���ϴ� ��ǥ Z

	double	horLen;				// ���� ����
	double	verLenBasic;		// ���� ���� (������)
	double	verLenExtra;		// ���� ���� (������)
	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)

	bool	bVertical;			// ����: ���ι���(true), ���ι���(false)

	double	gap;				// ������ ����

	bool	bSingleSide;		// �ܸ��ΰ�?

	bool	bExtra;				// ������ ������ �ִ°�?

	short	typeLcorner;		// ���� �ڳ� Ÿ��	: (1) ���� (2) ���ڳ��ǳ� (3) �ƿ��ڳ��ǳ� (4) �ƿ��ڳʾޱ�
	short	typeRcorner;		// ������ �ڳ� Ÿ��	: (1) ���� (2) ���ڳ��ǳ� (3) �ƿ��ڳ��ǳ� (4) �ƿ��ڳʾޱ�
	double	lenLcorner;			// ���� ���ڳ��ǳ�/�ƿ��ڳ��ǳ� ����
	double	lenRcorner;			// ������ ���ڳ��ǳ�/�ƿ��ڳ��ǳ� ����

	short	tableformType;		// ���̺��� Ÿ��: Ÿ��A (1), Ÿ��B (2), Ÿ��C (3)
	short	propsInstallType;	// Push-Pull Props ��ġ Ÿ��: ���� (0), ���� (1), �ٱ��� (2), ���� (3)

	short	nCellsInHor;		// ���� ���� ��(������) ����
	short	nCellsInVerBasic;	// ���� ���� ��(������) ���� (������)
	short	nCellsInVerExtra;	// ���� ���� ��(������) ���� (������)

	CellForWallTableform		cells [50];				// �� �迭 (���ڳ� ����)
	MarginCellForWallTableform	marginCellsBasic [50];	// �� �迭 (������)
	MarginCellForWallTableform	marginCellsExtra [50];	// �� �迭 (������)

	double	marginTopBasic;		// ��� ���� (������)
	double	marginTopExtra;		// ��� ���� (������)

public:
	// ���� ���� ���̺����� �ʺ� ���� (2300 ... 200)
	int	presetWidth_tableform[40] = { 2300, 2250, 2200, 2150, 2100, 2050, 2000, 1950, 1900, 1850, 1800, 1750, 1700, 1650, 1600, 1550, 1500, 1450, 1400, 1350, 1300, 1250, 1200, 1150, 1100, 1050, 1000, 950, 900, 850, 800, 750, 700, 650, 600, 500, 4500, 400, 300, 200 };
	// ���� ���� ���̺����� ���� ���� (6000 ... 1500)
	int	presetHeight_tableform[16] = { 6000, 5700, 5400, 5100, 4800, 4500, 4200, 3900, 3600, 3300, 3000, 2700, 2400, 2100, 1800, 1500 };

	// ���� ���� �������� �ʺ� ���� (600 ... 200, 0)
	int	presetWidth_euroform[7] = { 600, 500, 450, 400, 300, 200, 0 };
	// ���� ���� �������� ���� ���� (1200 ... 600, 0)
	int	presetHeight_euroform[4] = { 1200, 900, 600, 0 };

	// ���� ���� ���̺��� �� �������� �迭 ����
	int	presetWidth_config_vertical[40][5] =
		{
			4, 600, 600, 500, 600,		// 2300
			4, 600, 600, 450, 600,		// 2250
			4, 600, 600, 400, 600,		// 2200
			4, 600, 500, 450, 600,		// 2150
			4, 600, 600, 300, 600,		// 2100
			4, 600, 450, 400, 600,		// 2050
			4, 600, 600, 200, 600,		// 2000
			4, 600, 450, 300, 600,		// 1950
			4, 600, 500, 200, 600,		// 1900
			4, 600, 450, 200, 600,		// 1850
			3, 600, 600, 600, 0,		// 1800
			4, 600, 200, 450, 500,		// 1750
			3, 600, 500, 600, 0,		// 1700
			3, 600, 450, 600, 0,		// 1650
			3, 600, 400, 600, 0,		// 1600
			3, 600, 450, 500, 0,		// 1550
			3, 600, 300, 600, 0,		// 1500
			3, 500, 450, 500, 0,		// 1450
			3, 500, 400, 500, 0,		// 1400
			3, 600, 300, 450, 0,		// 1350
			3, 600, 200, 500, 0,		// 1300
			3, 600, 200, 450, 0,		// 1250
			2, 600, 600, 0, 0,			// 1200
			3, 450, 300, 400, 0,		// 1150
			3, 400, 300, 400, 0,		// 1100
			3, 450, 300, 300, 0,		// 1050
			2, 600, 400, 0, 0,			// 1000
			2, 450, 500, 0, 0,			// 950
			2, 600, 300, 0, 0,			// 900
			2, 400, 450, 0, 0,			// 850
			2, 400, 400, 0, 0,			// 800
			2, 450, 300, 0, 0,			// 750
			2, 400, 300, 0, 0,			// 700
			2, 450, 200, 0, 0,			// 650
			1, 600, 0, 0, 0,			// 600
			1, 500, 0, 0, 0,			// 500
			1, 450, 0, 0, 0,			// 450
			1, 400, 0, 0, 0,			// 400
			1, 300, 0, 0, 0,			// 300
			1, 200, 0, 0, 0				// 200
		};
	// ���� ���� ���̺��� �� �������� �迭 ����
	int	presetHeight_config_vertical[16][6] =
		{
			5, 1200, 1200, 1200, 1200, 1200,	// 6000
			5, 1200, 1200, 1200, 1200, 900,		// 5700
			5, 1200, 1200, 1200, 900, 900,		// 5400
			5, 1200, 1200, 1200, 900, 600,		// 5100
			4, 1200, 1200, 1200, 1200, 0,		// 4800
			4, 1200, 1200, 1200, 900, 0,		// 4500
			4, 1200, 1200, 900, 900, 0,			// 4200
			4, 1200, 1200, 900, 600, 0,			// 3900
			3, 1200, 1200, 1200, 0, 0,			// 3600
			3, 1200, 1200, 900, 0, 0,			// 3300
			3, 1200, 1200, 600, 0, 0,			// 3000
			3, 1200, 900, 600, 0, 0,			// 2700
			2, 1200, 1200, 0, 0, 0,				// 2400
			2, 1200, 900, 0, 0, 0,				// 2100
			2, 900, 900, 0, 0, 0,				// 1800
			2, 900, 600, 0, 0, 0				// 1500
		};
	// ���� ���� ���̺��� �� �������� �迭 ����
	int	presetWidth_config_horizontal[16][6] =
		{
			5, 1200, 1200, 1200, 1200, 1200,	// 6000
			5, 1200, 1200, 1200, 1200, 900,		// 5700
			5, 1200, 1200, 1200, 900, 900,		// 5400
			5, 1200, 1200, 1200, 900, 600,		// 5100
			4, 1200, 1200, 1200, 1200, 0,		// 4800
			4, 1200, 1200, 1200, 900, 0,		// 4500
			4, 1200, 1200, 900, 900, 0,			// 4200
			4, 1200, 1200, 900, 600, 0,			// 3900
			3, 1200, 1200, 1200, 0, 0,			// 3600
			3, 1200, 1200, 900, 0, 0,			// 3300
			3, 1200, 1200, 600, 0, 0,			// 3000
			3, 1200, 900, 600, 0, 0,			// 2700
			2, 1200, 1200, 0, 0, 0,				// 2400
			2, 1200, 900, 0, 0, 0,				// 2100
			2, 900, 900, 0, 0, 0,				// 1800
			2, 900, 600, 0, 0, 0				// 1500
		};
	// ���� ���� ���̺��� �� �������� �迭 ����
	int	presetHeight_config_horizontal[40][5] =
		{
			4, 600, 600, 600, 500,		// 2300
			4, 600, 600, 450, 600,		// 2250
			4, 600, 600, 600, 400,		// 2200
			4, 600, 450, 600, 500,		// 2150
			4, 600, 300, 600, 600,		// 2100
			4, 600, 600, 450, 400,		// 2050
			4, 600, 600, 600, 200,		// 2000
			4, 600, 300, 450, 600,		// 1950
			4, 600, 600, 200, 500,		// 1900
			4, 600, 600, 450, 200,		// 1850
			3, 600, 600, 600, 0,		// 1800
			4, 600, 450, 200, 500,		// 1750
			3, 600, 600, 500, 0,		// 1700
			3, 600, 450, 600, 0,		// 1650
			3, 600, 600, 400, 0,		// 1600
			3, 600, 450, 500, 0,		// 1550
			3, 600, 300, 600, 0,		// 1500
			3, 600, 450, 400, 0,		// 1450
			3, 600, 300, 500, 0,		// 1400
			3, 450, 300, 600, 0,		// 1350
			3, 600, 200, 500, 0,		// 1300
			3, 450, 600, 200, 0,		// 1250
			2, 600, 600, 0, 0,			// 1200
			3, 450, 300, 400, 0,		// 1150
			2, 600, 500, 0, 0,			// 1100
			3, 300, 300, 450, 0,		// 1050
			2, 600, 400, 0, 0,			// 1000
			2, 450, 500, 0, 0,			// 950
			2, 600, 300, 0, 0,			// 900
			2, 450, 400, 0, 0,			// 850
			2, 400, 400, 0, 0,			// 800
			2, 300, 450, 0, 0,			// 750
			2, 300, 400, 0, 0,			// 700
			2, 450, 200, 0, 0,			// 650
			1, 600, 0, 0, 0,			// 600
			1, 500, 0, 0, 0,			// 500
			1, 450, 0, 0, 0,			// 450
			1, 400, 0, 0, 0,			// 400
			1, 300, 0, 0, 0,			// 300
			1, 200, 0, 0, 0				// 200
		};

public:
	void	initCells (WallTableformPlacingZone* placingZone, bool bVertical);				// �� ���� �ʱ�ȭ
	double	getCellPositionLeftBottomX (WallTableformPlacingZone* placingZone, short idx);	// ��(0-��� �ε��� ��ȣ)�� ���ϴ� �� ��ġ X ��ǥ�� ����
	void	adjustCellsPosition (WallTableformPlacingZone* placingZone);					// �� ��ġ�� �ٸ��� ������
	void	adjustMarginCellsPosition (WallTableformPlacingZone* placingZone);				// ��� ���� �� ��ġ�� �ٸ��� ������
	GSErrCode	placeObjects (WallTableformPlacingZone* placingZone);						// �� ������ ������� ��ü���� ��ġ��
	GSErrCode	fillRestAreas (WallTableformPlacingZone* placingZone, short idxCell);		// ��� ������ ������ �Ǵ� ����, ���� ������ ä��
	GSErrCode	placeInsulations (WallTableformPlacingZone* placingZone, InfoWall* infoWall, insulElemForWallTableform* insulElem);		// ���� ���̺��� ���̿� �ܿ��縦 ��ġ��

	void	placeEuroformsOfTableform (WallTableformPlacingZone* placingZone, short idxCell);	// ���̺��� �� ������ ��ġ (����)
	void	placeTableformA (WallTableformPlacingZone* placingZone, short idxCell);				// ���̺��� Ÿ��A ��ġ (������ ����) - �������� 2��
	void	placeTableformB (WallTableformPlacingZone* placingZone, short idxCell);				// ���̺��� Ÿ��B ��ġ (������ ����) - �������� 1��
	void	placeTableformC (WallTableformPlacingZone* placingZone, short idxCell);				// ���̺��� Ÿ��C ��ġ (������ ����) - �������� 1��, ���� ����Ʈ �� Ȱ��

public:
	// ���̾�α� ���� ��� �ε��� ��ȣ ����
	short	EDITCONTROL_GAP;
	short	CHECKBOX_SINGLESIDE;
	short	POPUP_DIRECTION;
	short	POPUP_TABLEFORM_TYPE;
	short	POPUP_PROPS_INSTALL;
	short	EDITCONTROL_REMAIN_WIDTH;
	short	EDITCONTROL_REMAIN_HEIGHT_BASIC;
	short	EDITCONTROL_REMAIN_HEIGHT_EXTRA;
	short	BUTTON_ADD_HOR;
	short	BUTTON_DEL_HOR;
	short	BUTTON_LCORNER;
	short	POPUP_OBJ_TYPE_LCORNER;
	short	EDITCONTROL_WIDTH_LCORNER;
	short	BUTTON_RCORNER;
	short	POPUP_OBJ_TYPE_RCORNER;
	short	EDITCONTROL_WIDTH_RCORNER;
	short	BUTTON_ADD_VER_BASIC;
	short	BUTTON_DEL_VER_BASIC;
	short	BUTTON_ADD_VER_EXTRA;
	short	BUTTON_DEL_VER_EXTRA;

	short	BUTTON_OBJ [50];
	short	POPUP_OBJ_TYPE [50];
	short	POPUP_WIDTH [50];
	short	EDITCONTROL_WIDTH [50];
	short	POPUP_HEIGHT_PRESET;
	short	POPUP_HEIGHT_BASIC [10];
	short	POPUP_HEIGHT_EXTRA [10];

	short	LABEL_TOTAL_WIDTH;
	short	POPUP_WIDTH_IN_TABLE [10];
};

GSErrCode	placeTableformOnWall (void);				// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ - ��ü ����
short DGCALLBACK wallTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// ���̺���/������/�ٷ������̼�/����/���� ��ġ�� ���� ���̾�α� (���̺��� ����, ��� ����, ���� �� ����)
short DGCALLBACK wallTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// ��ü�� ���̾ �����ϱ� ���� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler3_Vertical (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// ���̺��� ���ι��⿡ ���Ͽ� �������� ���� �迭�� �����ϱ� ���� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler3_Horizontal (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ���̺��� ���ι��⿡ ���Ͽ� �������� ���� �迭�� �����ϱ� ���� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler4 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// �� ����� ������ ������ ������ �Ǵ� ����/����� ä���� ����� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler5_Insulation (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ���� ���̺��� ���̿� �ܿ��縦 ������ ���θ� ����� ���̾�α�

#endif