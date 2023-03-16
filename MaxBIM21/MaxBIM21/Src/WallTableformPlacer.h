#ifndef	__WALL_TABLEFORM_PLACER__
#define __WALL_TABLEFORM_PLACER__

#include "MaxBIM21.h"

namespace namespaceWallTableform {
	enum objType {
		OBJ_NONE,				// ���� (ǥ������ ����)
		OBJ_EUROFORM,			// ������
		OBJ_PLYWOOD,			// ����
		OBJ_TIMBER,				// ����
		OBJ_FILLERSPACER,		// �ٷ������̼�
		OBJ_WALL_TABLEFORM_A,	// ���̺���A
		OBJ_WALL_TABLEFORM_B,	// ���̺���B
		OBJ_WALL_TABLEFORM_C,	// ���̺���C
		OBJ_INCORNER_PANEL_L,	// ���ڳ��ǳ�(L)
		OBJ_OUTCORNER_PANEL_L,	// �ƿ��ڳ��ǳ�(L)
		OBJ_OUTCORNER_ANGLE_L,	// �ƿ��ڳʾޱ�(L)
		OBJ_INCORNER_PANEL_R,	// ���ڳ��ǳ�(R)
		OBJ_OUTCORNER_PANEL_R,	// �ƿ��ڳ��ǳ�(R)
		OBJ_OUTCORNER_ANGLE_R	// �ƿ��ڳʾޱ�(R)
	};
	GS::UniString objTypeStr[14] = { L"����", L"������", L"����", L"����", L"�ٷ������̼�", L"���̺���A", L"���̺���B", L"���̺���C", L"���ڳ��ǳ�(L)", L"�ƿ��ڳ��ǳ�(L)", L"�ƿ��ڳʾޱ�(L)", L"���ڳ��ǳ�(R)", L"�ƿ��ڳ��ǳ�(R)", L"�ƿ��ڳʾޱ�(R)" };

	enum DG1_itemIndex {
		LABEL_SIDE_SETTINGS = 3,	// ��: ��ġ��
		RADIOBUTTON_DUAL_SIDE,		// ���
		RADIOBUTTON_SINGLE_SIDE,	// �ܸ�
		LABEL_TABLEFORM_DIRECTION,	// ��: ���̺��� ����
		RADIOBUTTON_VERTICAL,		// ���� 
		RADIOBUTTON_HORIZONTAL,		// ����
		LABEL_GAP,					// ��: ������ ����
		EDITCONTROL_GAP,			// ������ ����
		LABEL_PROPS,				// ��: Push-Pull Props
		RADIOBUTTON_PROPS_ON,		// ��ġ
		RADIOBUTTON_PROPS_OFF,		// �̼�ġ
		LABEL_SHOW_SIDE,			// ��: ǥ�ø� ����
		RADIOBUTTON_SHOW_FRONT,		// �ո�
		RADIOBUTTON_SHOW_BACK,		// �޸�
		PUSHBUTTON_INFO_COPY,		// ������ ����
		LABEL_GUIDE					// ��: �ȳ� �ؽ�Ʈ
	};

	enum DG1_branch_itemIndex {
		BRANCH_LABEL_OBJ_TYPE = 3,			// ��: ��ü ����
		BRANCH_POPUP_OBJ_TYPE,				// ��ü ����
		BRANCH_LABEL_TOTAL_WIDTH,			// ��ü �ʺ�
		BRANCH_EDITCONTROL_TOTAL_WIDTH,		// ��ü �ʺ�
		BRANCH_LABEL_GUIDE					// ��: �ȳ� �ؽ�Ʈ
	};

	enum DG2_itemIndex {
		DG2_BUTTON_PREV = 3,			// ���� ��ư
		DG2_LABEL_TOTAL_HEIGHT,			// ��: ��ü ����
		DG2_EDITCONTROL_TOTAL_HEIGHT,	// ��ü ����
		DG2_LABEL_REMAIN_HEIGHT,		// ��: ���� ����
		DG2_EDITCONTROL_REMAIN_HEIGHT,	// ���� ����
		DG2_LABEL_GUIDE					// ��: �ȳ� �ؽ�Ʈ
	};

	struct Cell
	{
		double	leftBottomX;	// ���ϴ� ��ǥ X
		double	leftBottomY;	// ���ϴ� ��ǥ Y
		double	leftBottomZ;	// ���ϴ� ��ǥ Z
		double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

		short	objType;		// ��ü Ÿ��

		double	horLen;			// ���� ����
		double	verLen;			// ���� ����

		// ���̺��� ����
		int		nCellsHor;		// ���� ���� �� ����
		double	cellHorLen[5];	// �� ���� (����)
	};

	struct MarginCell
	{
		double	leftBottomX;	// ���ϴ� ��ǥ X
		double	leftBottomY;	// ���ϴ� ��ǥ Y
		double	leftBottomZ;	// ���ϴ� ��ǥ Z
		double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

		short	objType;		// ��ü Ÿ��

		double	horLen;			// ���� ����
		double	verLen;			// ���� ����
	};

	class PlacingZone
	{
	public:
		double	leftBottomX;	// ���ϴ� ��ǥ X
		double	leftBottomY;	// ���ϴ� ��ǥ Y
		double	leftBottomZ;	// ���ϴ� ��ǥ Z
		double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

		double	horLen;			// ���� ����
		double	verLenBasic;	// ���� ���� (������)
		double	verLenExtra;	// ���� ���� (������)

		double	gap;			// ������ ����

		bool	bVertical;		// �� ����: ���ι���(true), ���ι���(false)
		bool	bSingleSide;	// �� ��ġ��: �ܸ�(true), ���(false)
		bool	bExtra;			// �߰� ����: ������ ���� ����(true), ������ ���� ����(false)
		bool	bInstallProps;	// Push-Pull Props ��ġ ����: ��ġ(true), �̼�ġ(false)

		int		nCellsHor;		// ���� ���� �� ����
		int		nCellsVerBasic;	// ���� ���� �� ���� (������)
		int		nCellsVerExtra;	// ���� ���� �� ���� (������)

		int		nMarginCellsVerBasic;	// ���� ���� ���� �� ���� (������)
		int		nMarginCellsVerExtra;	// ���� ���� ���� �� ���� (������)

		Cell	cellsBasic[10][50];				// �� �迭 (������)
		Cell	cellsExtra[10][50];				// �� �迭 (������)
		MarginCell	marginCellsBasic[10][50];	// ���� �� �迭 (������)
		MarginCell	marginCellsExtra[10][50];	// ���� �� �迭 (������)

		double	marginTopBasic;	// ��� ���� (������)
		double	marginTopExtra;	// ��� ���� (������)

		// ���̺��� ���� ���� �ʺ� ����
		int		tableformPresetVertical[40] = { 2300, 2250, 2200, 2150, 2100, 2050, 2000, 1950, 1900, 1850, 1800, 1750, 1700, 1650, 1600, 1550, \
												1500, 1450, 1400, 1350, 1300, 1250, 1200, 1150, 1100, 1050, 1000, 950, 900, 850, 800, 750, \
												700, 650, 600, 500, 450, 400, 300, 200 };
		// ���̺��� ���� ���� �ʺ� ����
		int		tableformPresetHorizontal[16] = { 6000, 5700, 5400, 5100, 4800, 4500, 4200, 3900, 3600, 3300, 3000, 2700, 2400, 2100, 1800, 1500 };
		// ������ ���� ���� �ʺ� ����
		int		euroformPresetVertical[6] = { 600, 500, 450, 400, 300, 200 };
		// ������ ���� ���� �ʺ� ����
		int		euroformPresetHorizontal[3] = { 1200, 900, 600 };
		
		// ���̺��� ���� ���� �ʺ� ����
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
		// ���̺��� ���� ���� �ʺ� ����
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

		// ������
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
		}

		// ��ġ ���� ����
		void initCells()
		{
			// ��� ���� ���̺���A�� �ʱ�ȭ
			for (int i = 0; i < 10; i++) {
				for (int j = 0; j < 50; j++) {
					// �ո�
					this->cellsBasic[i][j].horLen = 2.250;
					this->cellsBasic[i][j].verLen = 1.200;
					this->cellsBasic[i][j].objType = OBJ_WALL_TABLEFORM_A;
					this->cellsBasic[i][j].nCellsHor = 4;

					this->cellsBasic[i][j].cellHorLen[0] = 0.600;
					this->cellsBasic[i][j].cellHorLen[1] = 0.600;
					this->cellsBasic[i][j].cellHorLen[2] = 0.450;
					this->cellsBasic[i][j].cellHorLen[3] = 0.600;

					// �޸�
					this->cellsExtra[i][j].horLen = 2.250;
					this->cellsExtra[i][j].verLen = 1.200;
					this->cellsExtra[i][j].objType = OBJ_WALL_TABLEFORM_A;
					this->cellsExtra[i][j].nCellsHor = 4;

					this->cellsExtra[i][j].cellHorLen[0] = 0.600;
					this->cellsExtra[i][j].cellHorLen[1] = 0.600;
					this->cellsExtra[i][j].cellHorLen[2] = 0.450;
					this->cellsExtra[i][j].cellHorLen[3] = 0.600;

					// �ո� ����
					this->marginCellsBasic[i][j].objType = OBJ_EUROFORM;
					this->marginCellsBasic[i][j].horLen = 1.200;
					this->marginCellsBasic[i][j].verLen = 0.600;

					// �޸� ����
					this->marginCellsExtra[i][j].objType = OBJ_EUROFORM;
					this->marginCellsExtra[i][j].horLen = 1.200;
					this->marginCellsExtra[i][j].verLen = 0.600;
				}
			}
		}
	};

	PlacingZone	placingZone;			// ��ġ ����
	InfoWall	infoWall;				// �� ��ü ����
	API_Guid	structuralObject;		// ���� ��ü�� GUID

	double DEFAULT_TABLEFORM_WIDTH = 2.250;
	double DEFAULT_EUROFORM_HEIGHT = 1.200;

	int MAX_ROW = 10;
	int MAX_COL = 50;

	// �׸� �ε��� (1�� ���̾�α�)
	short GRID_START_INDEX;						// �׸���
	short BUTTON_OBJ_TYPE_START_INDEX;			// ��ü Ÿ�� ���� ��ư
	short EDITCONTROL_OBJ_WIDTH_START_INDEX;	// ��ü �ʺ�
	short BUTTON_ADD_COLUMN;					// �� ���� ��ü �߰� ��ư
	short BUTTON_DEL_COLUMN;					// �� ���� ��ü ���� ��ư
	short EDITCONTROL_OBJ_HEIGHT_START_INDEX;	// ��ü ����
	short BUTTON_ADD_ROW;						// �� ���� ��ü �߰� ��ư
	short BUTTON_DEL_ROW;						// �� ���� ��ü ���� ��ư
	
	short EDITCONTROL_TOTAL_HEIGHT;				// Edit��Ʈ��: ��ü ����
	short EDITCONTROL_REMAIN_HEIGHT;			// Edit��Ʈ��: ���� ����
	
	short EDITCONTROL_TOTAL_WIDTH;				// Edit��Ʈ��: ��ü �ʺ�
	short EDITCONTROL_REMAIN_WIDTH;				// Edit��Ʈ��: ���� �ʺ�

	// �׸� �ε��� (1�� ���� ���̾�α�)
	short BRANCH_GRID_START_INDEX;					// �׸���
	short BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX;	// ��ü �ʺ�
	short BRANCH_BUTTON_ADD_COLUMN;					// �� ���� ��ü �߰� ��ư
	short BRANCH_BUTTON_DEL_COLUMN;					// �� ���� ��ü ���� ��ư

	// �׸� �ε��� (2�� ���̾�α�)
	short DG2_GRID_START_INDEX;						// �׸���
	short DG2_BUTTON_OBJ_TYPE_START_INDEX;			// ��ü Ÿ�� ���� ��ư
	short DG2_EDITCONTROL_OBJ_HEIGHT_START_INDEX;	// ��ü ����
	short DG2_BUTTON_ADD_ROW;						// �� ���� ��ü �߰� ��ư
	short DG2_BUTTON_DEL_ROW;						// �� ���� ��ü ���� ��ư

	short DGCALLBACK handler1_branch(short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */) {
		int* x = (int *)userData;	// ��ü ��ư�� ����

		short	result;
		short	itemIndex;
		short	posX, posY;
		short	sizeX, sizeY;
		double	totalWidth;

		switch (message) {
		case DG_MSG_INIT:
			DGSetDialogTitle(dialogID, L"���� ���̺��� ��ġ - �� ����");

			// Ȯ�� ��ư
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 15, 70, 25);
			DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_OK, L"Ȯ��");
			DGShowItem(dialogID, DG_OK);

			// ���
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 70, 25);
			DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_CANCEL, L"���");
			DGShowItem(dialogID, DG_CANCEL);
			
			// ��ü ����
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 20, 55, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"��ü ����");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_POPUPCONTROL, 200, 0, 160, 15, 150, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			for (int i = 1; i < 14; i++) {
				DGPopUpInsertItem(dialogID, itemIndex, DG_POPUP_BOTTOM);
				DGPopUpSetItemText(dialogID, itemIndex, DG_POPUP_BOTTOM, objTypeStr[i]);
			}
			DGShowItem(dialogID, itemIndex);

			// ��ü �ʺ�
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 50, 55, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"��ü �ʺ�");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 160, 45, 85, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem(dialogID, itemIndex);
			DGDisableItem(dialogID, itemIndex);

			// �ȳ� �ؽ�Ʈ
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 80, 300, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"�ȳ� �ؽ�Ʈ�Դϴ�.");
			DGShowItem(dialogID, itemIndex);

			// �� �׸��� ǥ��
			posX = 100; posY = 115;
			sizeX = 100; sizeY = 100;
			for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (i * sizeX), posY, sizeX, sizeY);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					BRANCH_GRID_START_INDEX = itemIndex;
			}

			// �� �ʺ� (Edit��Ʈ��)
			for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX + (i * sizeX), posY + 100, sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX = itemIndex;
			}

			// �� ���� ��ü �߰� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX), posY + 100, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			BRANCH_BUTTON_ADD_COLUMN = itemIndex;

			// �� ���� ��ü ���� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX) + 25, posY + 100, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			BRANCH_BUTTON_DEL_COLUMN = itemIndex;

			// ���̾�α� â ũ�� ����
			if (placingZone.cellsBasic[0][*x].nCellsHor > 3)
				DGSetDialogSize(dialogID, DG_CLIENT, 500 + ((placingZone.cellsBasic[0][*x].nCellsHor - 3) * sizeX), 300, DG_TOPLEFT, true);

			// �⺻�� ����
			DGPopUpSelectItem(dialogID, BRANCH_POPUP_OBJ_TYPE, placingZone.cellsBasic[0][*x].objType);				// �˾���Ʈ��
			DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH, placingZone.cellsBasic[0][*x].horLen);		// ��ü �ʺ�
			for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {										// �� �ʺ�
				DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + i, placingZone.cellsBasic[0][*x].cellHorLen[i]);
			}
				
			// ���̺����� ���
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
			// �� ���� ���
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
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 600, 500, 450, 400, 300, 200");
					}
					else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_PLYWOOD) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 100 ~ 1220");
					}
					else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_TIMBER) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 5 ~ 1000");
					}
					else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_FILLERSPACER) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 10 ~ 50");
					}
					else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_INCORNER_PANEL_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_INCORNER_PANEL_R)) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 80 ~ 500");
					}
					else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_PANEL_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_PANEL_R)) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 80 ~ 500");
					}
					else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_ANGLE_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_ANGLE_R)) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 63.5");
					}
				}
				else {
					if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_EUROFORM) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 1200, 900, 600");
					}
					else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_PLYWOOD) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 100 ~ 2440");
					}
					else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_TIMBER) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 5 ~ 1000");
					}
					else if (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_FILLERSPACER) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 10 ~ 50");
					}
					else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_INCORNER_PANEL_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_INCORNER_PANEL_R)) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 80 ~ 500");
					}
					else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_PANEL_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_PANEL_R)) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 80 ~ 500");
					}
					else if ((DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_ANGLE_L) || (DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE) == OBJ_OUTCORNER_ANGLE_R)) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 63.5");
					}
				}
			}

			for (int i = BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX; i < (BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + placingZone.cellsBasic[0][*x].nCellsHor); i++) {
				if (item == i) {
					if (placingZone.bVertical == true) {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 600, 500, 450, 400, 300, 200");
					}
					else {
						DGSetItemText(dialogID, BRANCH_LABEL_GUIDE, L"�Է� ���� ġ��: 1200, 900, 600");
					}
				}
			}

			break;

		case DG_MSG_CHANGE:
			// ���̺����� ���
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
			// �� ���� ���
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
					// �� �ʺ�
					for (int j = 0; j < placingZone.cellsBasic[i][*x].nCellsHor; j++) {
						placingZone.cellsBasic[i][*x].cellHorLen[j] = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + j);
						placingZone.cellsExtra[i][*x].cellHorLen[j] = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + j);
					}

					// ��ü �ʺ�
					placingZone.cellsBasic[i][*x].horLen = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);
					placingZone.cellsExtra[i][*x].horLen = DGGetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH);

					// ��ü Ÿ��
					placingZone.cellsBasic[i][*x].objType = DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE);
					placingZone.cellsExtra[i][*x].objType = DGPopUpGetSelected(dialogID, BRANCH_POPUP_OBJ_TYPE);
				}
				break;
			}

			// �� �߰� ��ư
			if (item == BRANCH_BUTTON_ADD_COLUMN) {
				item = 0;
				for (int i = 0; i < MAX_ROW; i++) {
					placingZone.cellsBasic[i][*x].nCellsHor++;
					placingZone.cellsExtra[i][*x].nCellsHor++;
				}
				break;
			}

			// �� ���� ��ư
			if (item == BRANCH_BUTTON_DEL_COLUMN) {
				item = 0;
				for (int i = 0; i < MAX_ROW; i++) {
					placingZone.cellsBasic[i][*x].nCellsHor--;
					placingZone.cellsExtra[i][*x].nCellsHor--;
				}
				break;
			}

			// ---------- ���� ��� ���� �׸���
			// ���� ��� ��� ����
			DGRemoveDialogItems(dialogID, BRANCH_GRID_START_INDEX);

			// �� �׸��� ǥ��
			posX = 100; posY = 115;
			sizeX = 100; sizeY = 100;
			for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (i * sizeX), posY, sizeX, sizeY);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					BRANCH_GRID_START_INDEX = itemIndex;
			}

			// �� �ʺ� (Edit��Ʈ��)
			for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX + (i * sizeX), posY + 100, sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX = itemIndex;
			}

			// �� ���� ��ü �߰� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX), posY + 100, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			BRANCH_BUTTON_ADD_COLUMN = itemIndex;

			// �� ���� ��ü ���� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.cellsBasic[0][*x].nCellsHor * sizeX) + 25, posY + 100, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			BRANCH_BUTTON_DEL_COLUMN = itemIndex;

			// ���̾�α� â ũ�� ����
			if (placingZone.cellsBasic[0][*x].nCellsHor > 3)
				DGSetDialogSize(dialogID, DG_CLIENT, 500 + ((placingZone.cellsBasic[0][*x].nCellsHor - 3) * sizeX), 300, DG_TOPLEFT, true);

			// �⺻�� ����
			DGPopUpSelectItem(dialogID, BRANCH_POPUP_OBJ_TYPE, placingZone.cellsBasic[0][*x].objType);				// �˾���Ʈ��
			DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_TOTAL_WIDTH, placingZone.cellsBasic[0][*x].horLen);		// ��ü �ʺ�
			for (int i = 0; i < placingZone.cellsBasic[0][*x].nCellsHor; i++) {										// �� �ʺ�
				DGSetItemValDouble(dialogID, BRANCH_EDITCONTROL_OBJ_WIDTH_START_INDEX + i, placingZone.cellsBasic[0][*x].cellHorLen[i]);
			}
			// ---------- ���� ��� ���� �׸���

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
			DGSetDialogTitle(dialogID, L"���� ���̺��� ��ġ");

			// ���� ��ư
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 15, 70, 25);
			DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_OK, L"����");
			DGShowItem(dialogID, DG_OK);

			// ���
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 70, 25);
			DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_CANCEL, L"���");
			DGShowItem(dialogID, DG_CANCEL);

			// ��ġ�� ���� (���/�ܸ�)
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 20, 50, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"��ġ��");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 1, 155, 15, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"���");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 1, 210, 15, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"�ܸ�");
			DGShowItem(dialogID, itemIndex);

			// ���̺��� ���� (����/����)
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 300, 20, 70, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"���̺��� ����");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 2, 395, 15, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"����");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 2, 450, 15, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"����");
			DGShowItem(dialogID, itemIndex);

			// ������ ����
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 50, 70, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"������ ����");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 175, 45, 85, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem(dialogID, itemIndex);

			// Push-Pull Props ��ġ ����
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 300, 50, 90, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"Push-Pull Props");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 3, 395, 45, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"��ġ");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 3, 450, 45, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"�̼�ġ");
			DGShowItem(dialogID, itemIndex);

			// �ո�/�޸� ��ȯ
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 90, 70, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"ǥ�ø� ����");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 4, 180, 85, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"�ո�");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 4, 235, 85, 50, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"�޸�");
			DGShowItem(dialogID, itemIndex);

			// �� ���� ���� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 300, 85, 200, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, L"�ո�->�޸����� ������ ����");
			DGShowItem(dialogID, itemIndex);

			// �ȳ� �ؽ�Ʈ
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 120, 130, 380, 70);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"�ȳ� �ؽ�Ʈ�Դϴ�.");
			DGShowItem(dialogID, itemIndex);

			// ��ü ����
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 10, 130, 80, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"��ü ����");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 10, 155, 80, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGDisableItem(dialogID, itemIndex);
			DGShowItem(dialogID, itemIndex);
			EDITCONTROL_TOTAL_HEIGHT = itemIndex;

			// ���� ����
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 10, 190, 80, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"���� ����");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 10, 215, 80, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGDisableItem(dialogID, itemIndex);
			DGShowItem(dialogID, itemIndex);
			EDITCONTROL_REMAIN_HEIGHT = itemIndex;

			// ��ü �ʺ�
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 10, 290, 80, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"��ü �ʺ�");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 10, 315, 80, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGDisableItem(dialogID, itemIndex);
			DGShowItem(dialogID, itemIndex);
			EDITCONTROL_TOTAL_WIDTH = itemIndex;

			// ���� �ʺ�
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 10, 350, 80, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"���� �ʺ�");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 10, 375, 80, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGDisableItem(dialogID, itemIndex);
			DGShowItem(dialogID, itemIndex);
			EDITCONTROL_REMAIN_WIDTH = itemIndex;

			nCellsVer = placingZone.nCellsVerBasic;

			// �� �׸��� ǥ��
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

			// ��ü Ÿ�� ���� ��ư ǥ��
			posX = 205; posY = 230 + (nCellsVer * sizeY) + 5;
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (i * sizeX), posY, sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText(dialogID, itemIndex, objTypeStr[placingZone.cellsBasic[0][i].objType]);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					BUTTON_OBJ_TYPE_START_INDEX = itemIndex;
			}

			// ��ü �ʺ� (Edit��Ʈ��)
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX + (i * sizeX), posY + 25, sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					EDITCONTROL_OBJ_WIDTH_START_INDEX = itemIndex;
			}

			// �� ���� ��ü �߰� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX), posY, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			BUTTON_ADD_COLUMN = itemIndex;

			// �� ���� ��ü ���� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX) + 25, posY, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			BUTTON_DEL_COLUMN = itemIndex;

			// ��ü ����
			posX = 100; posY = 230 + (nCellsVer * sizeY) - 63;
			for (int i = 0; i < nCellsVer; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX, posY - (i * sizeY), sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					EDITCONTROL_OBJ_HEIGHT_START_INDEX = itemIndex;
			}

			// �� ���� ��ü �߰� ��ư
			posX = 100; posY = 230;
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 25, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			BUTTON_ADD_ROW = itemIndex;

			// �� ���� ��ü ���� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 50, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			BUTTON_DEL_ROW = itemIndex;

			// ���̾�α� â ũ�� ����
			DGSetDialogSize(dialogID, DG_CLIENT, 205 + (placingZone.nCellsHor * sizeX) + 100, 230 + (nCellsVer * sizeY) + 100, DG_TOPLEFT, true);

			// �⺻�� ����: ���� ��ư
			DGSetItemValLong(dialogID, RADIOBUTTON_DUAL_SIDE, true);	// ��ġ��: ���
			DGSetItemValLong(dialogID, RADIOBUTTON_VERTICAL, true);		// ���̺��� ����: ����
			DGSetItemValLong(dialogID, RADIOBUTTON_PROPS_ON, true);		// Push-Pull Props: ��ġ
			DGSetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT, true);	// ǥ�ø� ����: �ո�

			// �⺻�� ����: �� ũ��
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				DGSetItemValDouble(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX + i, placingZone.cellsBasic[0][i].horLen);
			}

			for (int i = 0; i < placingZone.nCellsVerBasic; i++) {
				DGSetItemValDouble(dialogID, EDITCONTROL_OBJ_HEIGHT_START_INDEX + i, placingZone.cellsBasic[i][0].verLen);
			}

			// �ʺ� ���� �� ����
			DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.horLen);
			remainWidth = placingZone.horLen;
			for (int i = 0; i < placingZone.nCellsHor; i++)
				remainWidth -= DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX + i);
			DGSetItemValDouble(dialogID, EDITCONTROL_REMAIN_WIDTH, remainWidth);

			// ���� ���� �� ����
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
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 600, 500, 450, 400, 300, 200");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_WALL_TABLEFORM_A) || (placingZone.cellsBasic[0][i].objType == OBJ_WALL_TABLEFORM_B) || (placingZone.cellsBasic[0][i].objType == OBJ_WALL_TABLEFORM_C)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 2300, 2250, 2200, 2150, 2100, 2050, 2000, 1950, 1900, \n1850, 1800, 1750, 1700, 1650, 1600, 1550, 1500, 1450, 1400, 1350, \n1300, 1250, 1200, 1150, 1100, 1050, 1000, 950, 900, 850, \n800, 750, 700, 650, 600, 500, 450, 400, 300, 200");
						}
						else if (placingZone.cellsBasic[0][i].objType == OBJ_PLYWOOD) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 100 ~ 1220");
						}
						else if (placingZone.cellsBasic[0][i].objType == OBJ_TIMBER) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 5 ~ 1000");
						}
						else if (placingZone.cellsBasic[0][i].objType == OBJ_FILLERSPACER) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 10 ~ 50");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_INCORNER_PANEL_L) || (placingZone.cellsBasic[0][i].objType == OBJ_INCORNER_PANEL_R)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 80 ~ 500");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_PANEL_L) || (placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_PANEL_R)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 80 ~ 500");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_ANGLE_L) || (placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_ANGLE_R)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 63.5");
						}
					}
					else {
						if (placingZone.cellsBasic[0][i].objType == OBJ_EUROFORM) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 1200, 900, 600");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_WALL_TABLEFORM_A) || (placingZone.cellsBasic[0][i].objType == OBJ_WALL_TABLEFORM_B) || (placingZone.cellsBasic[0][i].objType == OBJ_WALL_TABLEFORM_C)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 6000, 5700, 5400, 5100, 4800, 4500, 4200, \n3900, 3600, 3300, 3000, 2700, 2400, 2100, 1800, 1500");
						}
						else if (placingZone.cellsBasic[0][i].objType == OBJ_PLYWOOD) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 100 ~ 2440");
						}
						else if (placingZone.cellsBasic[0][i].objType == OBJ_TIMBER) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 5 ~ 1000");
						}
						else if (placingZone.cellsBasic[0][i].objType == OBJ_FILLERSPACER) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 10 ~ 50");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_INCORNER_PANEL_L) || (placingZone.cellsBasic[0][i].objType == OBJ_INCORNER_PANEL_R)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 80 ~ 500");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_PANEL_L) || (placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_PANEL_R)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 80 ~ 500");
						}
						else if ((placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_ANGLE_L) || (placingZone.cellsBasic[0][i].objType == OBJ_OUTCORNER_ANGLE_R)) {
							DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 63.5");
						}
					}
				}
			}

			for (int i = 0; i < nCellsVer; i++) {
				if (item == EDITCONTROL_OBJ_HEIGHT_START_INDEX + i) {
					if (DGGetItemValLong(dialogID, RADIOBUTTON_VERTICAL))
						DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 1200, 900, 600");
					else
						DGSetItemText(dialogID, LABEL_GUIDE, L"�Է� ���� ġ��: 600, 500, 450, 400, 300, 200");
				}
			}

			break;

		case DG_MSG_CHANGE:
			if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1)
				nCellsVer = placingZone.nCellsVerBasic;
			else
				nCellsVer = placingZone.nCellsVerExtra;

			// ���̺����� ���, ��ü �ʺ� ����� ���� �� ������ �ڵ� �����
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

			// ��ġ ���� ����
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				for (int j = 0; j < MAX_ROW; j++) {
					placingZone.cellsBasic[j][i].horLen = DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX + i);
				}
			}

			if (item == RADIOBUTTON_SHOW_FRONT || item == RADIOBUTTON_SHOW_BACK) {
				// �ո�/�޸��� �����ϴ� ����
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
				// �� ���� ���
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

			// ��ġ�� ����
			if (DGGetItemValLong(dialogID, RADIOBUTTON_SINGLE_SIDE) == 1)
				placingZone.bSingleSide = true;
			else
				placingZone.bSingleSide = false;

			// ���̺��� ���� ����
			if (DGGetItemValLong(dialogID, RADIOBUTTON_VERTICAL) == 1)
				placingZone.bVertical = true;
			else
				placingZone.bVertical = false;

			// ������ ���� ����
			placingZone.gap = DGGetItemValDouble(dialogID, EDITCONTROL_GAP);

			// Push-Pull Props ����
			if (DGGetItemValLong(dialogID, RADIOBUTTON_PROPS_ON) == 1)
				placingZone.bInstallProps = true;
			else
				placingZone.bInstallProps = false;

			// �ո�/�޸� ���ý�
			if (item == RADIOBUTTON_SHOW_FRONT || item == RADIOBUTTON_SHOW_BACK) {
				// ���� ��� ��� ����
				DGRemoveDialogItems(dialogID, GRID_START_INDEX);

				// �� �׸��� ǥ��
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

				// ��ü Ÿ�� ���� ��ư ǥ��
				posX = 205; posY = 230 + (nCellsVer * sizeY) + 5;
				for (int i = 0; i < placingZone.nCellsHor; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (i * sizeX), posY, sizeX, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText(dialogID, itemIndex, objTypeStr[placingZone.cellsBasic[0][i].objType]);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						BUTTON_OBJ_TYPE_START_INDEX = itemIndex;
				}

				// ��ü �ʺ� (Edit��Ʈ��)
				for (int i = 0; i < placingZone.nCellsHor; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX + (i * sizeX), posY + 25, sizeX, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						EDITCONTROL_OBJ_WIDTH_START_INDEX = itemIndex;
				}

				// �� ���� ��ü �߰� ��ư
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX), posY, 25, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText(dialogID, itemIndex, "+");
				DGShowItem(dialogID, itemIndex);
				BUTTON_ADD_COLUMN = itemIndex;

				// �� ���� ��ü ���� ��ư
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX) + 25, posY, 25, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText(dialogID, itemIndex, "-");
				DGShowItem(dialogID, itemIndex);
				BUTTON_DEL_COLUMN = itemIndex;

				// ��ü ����
				posX = 100; posY = 230 + (nCellsVer * sizeY) - 63;
				for (int i = 0; i < nCellsVer; i++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX, posY - (i * sizeY), sizeX, 25);
					DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem(dialogID, itemIndex);
					if (i == 0)
						EDITCONTROL_OBJ_HEIGHT_START_INDEX = itemIndex;
				}

				// �� ���� ��ü �߰� ��ư
				posX = 100; posY = 230;
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 25, posY - 25, 25, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText(dialogID, itemIndex, "+");
				DGShowItem(dialogID, itemIndex);
				BUTTON_ADD_ROW = itemIndex;

				// �� ���� ��ü ���� ��ư
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 50, posY - 25, 25, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText(dialogID, itemIndex, "-");
				DGShowItem(dialogID, itemIndex);
				BUTTON_DEL_ROW = itemIndex;

				// ���̾�α� â ũ�� ����
				DGSetDialogSize(dialogID, DG_CLIENT, 205 + (placingZone.nCellsHor * sizeX) + 100, 230 + (nCellsVer * sizeY) + 100, DG_TOPLEFT, true);
			}

			// �⺻�� ����: �� ũ��
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

			// �ʺ� ���� �� ����
			DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.horLen);
			remainWidth = placingZone.horLen;
			for (int i = 0; i < placingZone.nCellsHor; i++)
				remainWidth -= DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX + i);
			DGSetItemValDouble(dialogID, EDITCONTROL_REMAIN_WIDTH, remainWidth);

			// ���� ���� �� ����
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
				// ��ġ��
				if (DGGetItemValLong(dialogID, RADIOBUTTON_SINGLE_SIDE) == 1)
					placingZone.bSingleSide = true;
				else
					placingZone.bSingleSide = false;

				// ���̺��� ����
				if (DGGetItemValLong(dialogID, RADIOBUTTON_VERTICAL) == 1)
					placingZone.bVertical = true;
				else
					placingZone.bVertical = false;

				// ������ ����
				placingZone.gap = DGGetItemValDouble(dialogID, EDITCONTROL_GAP);

				// Push-Pull Props
				if (DGGetItemValLong(dialogID, RADIOBUTTON_PROPS_ON) == 1)
					placingZone.bInstallProps = true;
				else
					placingZone.bInstallProps = false;

				// ��� ����
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
									
			// �� ���� ���� ��ư
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
			
			// �� �߰� ��ư
			if (item == BUTTON_ADD_COLUMN) {
				item = 0;
				placingZone.nCellsHor++;
			}
			
			// �� ���� ��ư
			if (item == BUTTON_DEL_COLUMN) {
				item = 0;
				placingZone.nCellsHor--;
			}
			
			// �� �߰� ��ư
			if (item == BUTTON_ADD_ROW) {
				item = 0;

				if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1)
					placingZone.nCellsVerBasic++;
				else
					placingZone.nCellsVerExtra++;

				nCellsVer++;
			}
			
			// �� ���� ��ư
			if (item == BUTTON_DEL_ROW) {
				item = 0;

				if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1)
					placingZone.nCellsVerBasic--;
				else
					placingZone.nCellsVerExtra--;

				nCellsVer--;
			}
			
			// ��ü Ÿ�� ��ư (UI â ǥ��)
			for (int i = BUTTON_OBJ_TYPE_START_INDEX; i < BUTTON_OBJ_TYPE_START_INDEX + placingZone.nCellsHor; i++) {
				if (item == i) {
					item = 0;
					int clickedIndex = i - BUTTON_OBJ_TYPE_START_INDEX;
					result = DGBlankModalDialog(500, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, handler1_branch, (DGUserData)&clickedIndex);
				}
			}
			
			// ---------- ���� ��� ���� �׸���
			// ���� ��� ��� ����
			DGRemoveDialogItems(dialogID, GRID_START_INDEX);

			// �� �׸��� ǥ��
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

			// ��ü Ÿ�� ���� ��ư ǥ��
			posX = 205; posY = 230 + (nCellsVer * sizeY) + 5;
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (i * sizeX), posY, sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText(dialogID, itemIndex, objTypeStr[placingZone.cellsBasic[0][i].objType]);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					BUTTON_OBJ_TYPE_START_INDEX = itemIndex;
			}

			// ��ü �ʺ� (Edit��Ʈ��)
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX + (i * sizeX), posY + 25, sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					EDITCONTROL_OBJ_WIDTH_START_INDEX = itemIndex;
			}

			// �� ���� ��ü �߰� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX), posY, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			BUTTON_ADD_COLUMN = itemIndex;

			// �� ���� ��ü ���� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX) + 25, posY, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			BUTTON_DEL_COLUMN = itemIndex;

			// ��ü ����
			posX = 100; posY = 230 + (nCellsVer * sizeY) - 63;
			for (int i = 0; i < nCellsVer; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX, posY - (i * sizeY), sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem(dialogID, itemIndex);
				if (i == 0)
					EDITCONTROL_OBJ_HEIGHT_START_INDEX = itemIndex;
			}

			// �� ���� ��ü �߰� ��ư
			posX = 100; posY = 230;
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 25, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			BUTTON_ADD_ROW = itemIndex;

			// �� ���� ��ü ���� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 50, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			BUTTON_DEL_ROW = itemIndex;

			// ���̾�α� â ũ�� ����
			DGSetDialogSize(dialogID, DG_CLIENT, 205 + (placingZone.nCellsHor * sizeX) + 100, 230 + (nCellsVer * sizeY) + 100, DG_TOPLEFT, true);
			// ---------- ���� ��� ���� �׸���
			
			// �⺻�� ����: �� ũ��
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

			// �ʺ� ���� �� ����
			DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.horLen);
			remainWidth = placingZone.horLen;
			for (int i = 0; i < placingZone.nCellsHor; i++)
				remainWidth -= DGGetItemValDouble(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX + i);
			DGSetItemValDouble(dialogID, EDITCONTROL_REMAIN_WIDTH, remainWidth);

			// ���� ���� �� ����
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
		int* x = (int*)userData;	// ��ü ��ư�� ����

		short	result;
		short	itemIndex;

		switch (message) {
		case DG_MSG_INIT:
			DGSetDialogTitle(dialogID, L"���� ���̺��� ��ġ - ���� �� ����");

			// Ȯ�� ��ư
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 15, 70, 25);
			DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_OK, L"Ȯ��");
			DGShowItem(dialogID, DG_OK);

			// ���
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 70, 25);
			DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_CANCEL, L"���");
			DGShowItem(dialogID, DG_CANCEL);

			// ��ü ����
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 20, 55, 23);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"��ü ����");
			DGShowItem(dialogID, itemIndex);

			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_POPUPCONTROL, 200, 0, 160, 15, 150, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			for (int i = 1; i <= 2; i++) {
				DGPopUpInsertItem(dialogID, itemIndex, DG_POPUP_BOTTOM);
				DGPopUpSetItemText(dialogID, itemIndex, DG_POPUP_BOTTOM, objTypeStr[i]);
			}
			DGShowItem(dialogID, itemIndex);

			// �⺻�� ����
			DGPopUpSelectItem(dialogID, BRANCH_POPUP_OBJ_TYPE, placingZone.marginCellsBasic[*x][0].objType);	// �˾���Ʈ��

			break;

		case DG_MSG_CLICK:
			if (item == DG_OK) {
				for (int i = 0; i < MAX_COL; i++) {
					// ��ü Ÿ��
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
			// Ÿ��Ʋ
			if (*bFront == true)
				DGSetDialogTitle(dialogID, L"��� ���� ä��� (������)");
			else
				DGSetDialogTitle(dialogID, L"��� ���� ä��� (������)");

			// ���� ��ư
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 15, 70, 25);
			DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_OK, L"����");
			DGShowItem(dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 75, 70, 25);
			DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG_CANCEL, L"���");
			DGShowItem(dialogID, DG_CANCEL);

			// ���� ��ư
			DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 70, 25);
			DGSetItemFont(dialogID, DG2_BUTTON_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG2_BUTTON_PREV, L"����");
			DGShowItem(dialogID, DG2_BUTTON_PREV);

			// ��ü ����
			DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 20, 55, 23);
			DGSetItemFont(dialogID, DG2_LABEL_TOTAL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG2_LABEL_TOTAL_HEIGHT, L"��ü ����");
			DGShowItem(dialogID, DG2_LABEL_TOTAL_HEIGHT);

			DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 160, 15, 85, 25);
			DGSetItemFont(dialogID, DG2_EDITCONTROL_TOTAL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			if (*bFront == true)
				DGSetItemValDouble(dialogID, DG2_EDITCONTROL_TOTAL_HEIGHT, placingZone.marginTopBasic);
			else
				DGSetItemValDouble(dialogID, DG2_EDITCONTROL_TOTAL_HEIGHT, placingZone.marginTopExtra);
			DGShowItem(dialogID, DG2_EDITCONTROL_TOTAL_HEIGHT);
			DGDisableItem(dialogID, DG2_EDITCONTROL_TOTAL_HEIGHT);

			// ���� ����
			DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 270, 20, 55, 23);
			DGSetItemFont(dialogID, DG2_LABEL_REMAIN_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG2_LABEL_REMAIN_HEIGHT, L"���� ����");
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

			// �ȳ� �ؽ�Ʈ
			DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 50, 300, 23);
			DGSetItemFont(dialogID, DG2_LABEL_GUIDE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, DG2_LABEL_GUIDE, L"�ȳ� �ؽ�Ʈ�Դϴ�.");
			DGShowItem(dialogID, DG2_LABEL_GUIDE);

			// �� �׸��� ǥ��
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

			// ��ü Ÿ�� ���� ��ư ǥ��
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

			// �� ���� (Edit��Ʈ��)
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

			// �� ���� ��ü �߰� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX - sizeX + 25, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			DG2_BUTTON_ADD_ROW = itemIndex;

			// �� ���� ��ü ���� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX - sizeX + 50, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			DG2_BUTTON_DEL_ROW = itemIndex;

			// ���� ���� ������Ʈ
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

			// ���̾�α� â ũ�� ����
			DGSetDialogSize(dialogID, DG_CLIENT, 300 + (placingZone.nCellsHor * sizeX), 200 + (placingZone.nMarginCellsVerBasic * sizeY), DG_TOPLEFT, true);

			break;

		case DG_MSG_FOCUS:
			if (*bFront == true) {
				for (int i = 0; i < placingZone.nMarginCellsVerBasic; i++) {
					focusedItemIndex = DG2_EDITCONTROL_OBJ_HEIGHT_START_INDEX + i;
					buttonItemIndex = DG2_BUTTON_OBJ_TYPE_START_INDEX + i;
					
					if (item == focusedItemIndex) {
						if (placingZone.marginCellsBasic[i][0].objType == OBJ_EUROFORM) {
							DGSetItemText(dialogID, DG2_LABEL_GUIDE, L"�Է� ���� ġ��: 600, 500, 450, 400, 300, 200");
						}
						else if (placingZone.marginCellsBasic[i][0].objType == OBJ_PLYWOOD) {
							DGSetItemText(dialogID, DG2_LABEL_GUIDE, L"�Է� ���� ġ��: 100 ~ 1220");
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
							DGSetItemText(dialogID, DG2_LABEL_GUIDE, L"�Է� ���� ġ��: 600, 500, 450, 400, 300, 200");
						}
						else if (placingZone.marginCellsExtra[i][0].objType == OBJ_PLYWOOD) {
							DGSetItemText(dialogID, DG2_LABEL_GUIDE, L"�Է� ���� ġ��: 100 ~ 1220");
						}
					}
				}
			}

			break;

		case DG_MSG_CHANGE:
			// ��ġ ���� ������Ʈ
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

			// ���� ���� ������Ʈ
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
						// �� ����
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

			// �� �߰� ��ư
			if (item == DG2_BUTTON_ADD_ROW) {
				item = 0;
				if (*bFront == true) {
					placingZone.nMarginCellsVerBasic++;
				}
				else {
					placingZone.nMarginCellsVerExtra++;
				}
			}

			// �� ���� ��ư
			if (item == DG2_BUTTON_DEL_ROW) {
				item = 0;
				if (*bFront == true) {
					placingZone.nMarginCellsVerBasic--;
				}
				else {
					placingZone.nMarginCellsVerExtra--;
				}
			}

			// ��ü Ÿ�� ��ư (UI â ǥ��)
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

			// ---------- ���� ��� ���� �׸���
			// ���� ��� ��� ����
			DGRemoveDialogItems(dialogID, DG2_GRID_START_INDEX);

			// �� �׸��� ǥ��
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

			// ��ü Ÿ�� ���� ��ư ǥ��
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

			// �� ���� (Edit��Ʈ��)
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

			// �� ���� ��ü �߰� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX - sizeX + 25, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "+");
			DGShowItem(dialogID, itemIndex);
			DG2_BUTTON_ADD_ROW = itemIndex;

			// �� ���� ��ü ���� ��ư
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX - sizeX + 50, posY - 25, 25, 25);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itemIndex, "-");
			DGShowItem(dialogID, itemIndex);
			DG2_BUTTON_DEL_ROW = itemIndex;

			// ���̾�α� â ũ�� ����
			DGSetDialogSize(dialogID, DG_CLIENT, 300 + (placingZone.nCellsHor * sizeX), 200 + (placingZone.nMarginCellsVerBasic * sizeY), DG_TOPLEFT, true);
			// ---------- ���� ��� ���� �׸���

			// ���� ���� ������Ʈ
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

}

using namespace namespaceWallTableform;

// ���� ���̺��� ��ġ
GSErrCode	placeWallTableform(void)
{
	GSErrCode	err = NoError;

	// �۾� �� ����
	double	workLevel_wall;

	// Selection Manager ���� ����
	GS::Array<API_Guid>		walls;
	GS::Array<API_Guid>		morphs;
	long					nWalls = 0;
	long					nMorphs = 0;

	// ��ü ���� ��������
	API_Element			elem;
	API_ElementMemo		memo;
	API_ElemInfo3D		info3D;

	// ���� ��ü ����
	InfoMorph			infoMorph[2];
	InfoMorph			infoMorph_Basic;
	InfoMorph			infoMorph_Extra;

	// ������ ��� �������� (�� 1��, ���� 1~2��)
	GSErrCode	err1 = getGuidsOfSelection(&walls, API_WallID, &nWalls);
	GSErrCode	err2 = getGuidsOfSelection(&morphs, API_MorphID, &nMorphs);
	if (err1 == APIERR_NOSEL || err2 == APIERR_NOSEL) {
		DGAlert(DG_ERROR, L"����", L"�ƹ��͵� �������� �ʾҽ��ϴ�.", L"�ʼ�����: ��(1��), ���� ���� ����(1��)\n�߰�����: ���� ���� ����(�޸� - 1�� ������ ���̰� �ٸ�)(1��)", L"Ȯ��", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	if (nWalls != 1) {
		DGAlert(DG_ERROR, L"����", L"���� 1���� �����ؾ� �մϴ�.", "", L"Ȯ��", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	if (nMorphs < 1 || nMorphs > 2) {
		DGAlert(DG_ERROR, L"����", L"���� ���� ������ 1�� �Ǵ� 2�� �����ؾ� �մϴ�.", "", L"Ȯ��", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// �� ������ ������
	BNZeroMemory(&elem, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	elem.header.guid = walls.Pop();
	structuralObject = elem.header.guid;
	ACAPI_Element_Get(&elem);
	ACAPI_Element_GetMemo(elem.header.guid, &memo);

	if(elem.wall.thickness != elem.wall.thickness1) {
		DGAlert(DG_ERROR, L"����", L"���� �β��� �����ؾ� �մϴ�.", "", L"Ȯ��", "", "");
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

	// ���� ������ ������
	for (int i = 0; i < nMorphs; i++) {
		BNZeroMemory(&elem, sizeof(API_Element));
		elem.header.guid = morphs.Pop();
		ACAPI_Element_Get(&elem);
		ACAPI_Element_Get3DInfo(elem.header, &info3D);

		// ���� ������ ������ ���� ������ �ߴ�
		if (abs(info3D.bounds.zMax - info3D.bounds.zMin) < EPS) {
			DGAlert(DG_ERROR, L"����", L"������ ������ �־�� �մϴ�.", "", L"Ȯ��", "", "");
			err = APIERR_GENERAL;
			return err;
		}

		// ������ GUID, �� �ε���, ��, ���� ����
		infoMorph[i].guid = elem.header.guid;
		infoMorph[i].floorInd = elem.header.floorInd;
		infoMorph[i].level = elem.morph.level;
		infoMorph[i].height = info3D.bounds.zMax - info3D.bounds.zMin;

		// ������ ���ϴ�, ���� �� ����
		if (abs(elem.morph.tranmat.tmx[11] - info3D.bounds.zMin) < EPS) {
			// ���ϴ� ��ǥ ����
			infoMorph[i].leftBottomX = elem.morph.tranmat.tmx[3];
			infoMorph[i].leftBottomY = elem.morph.tranmat.tmx[7];
			infoMorph[i].leftBottomZ = elem.morph.tranmat.tmx[11];

			// ���� ��ǥ��?
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
			// ���� ��ǥ ����
			infoMorph[i].rightTopX = elem.morph.tranmat.tmx[3];
			infoMorph[i].rightTopY = elem.morph.tranmat.tmx[7];
			infoMorph[i].rightTopZ = elem.morph.tranmat.tmx[11];

			// ���ϴ� ��ǥ��?
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

		// ������ Z�� ȸ�� ���� (���� ��ġ ����)
		double dx = infoMorph[i].rightTopX - infoMorph[i].leftBottomX;
		double dy = infoMorph[i].rightTopY - infoMorph[i].leftBottomY;
		infoMorph[i].ang = RadToDegree(atan2(dy, dx));

		// ������ ����/���� ����
		infoMorph[i].horLen = GetDistance(info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.xMax, info3D.bounds.yMax);
		infoMorph[i].verLen = abs(info3D.bounds.zMax - info3D.bounds.zMin);

		// ���� ����
		GS::Array<API_Element>	elems;
		elems.Push(elem);
		deleteElements(elems);
	}

	// ������ 1�� �Ǵ� 2���� ��
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

	// ���� ������ ���� ��ġ ���� ������Ʈ
	placingZone.leftBottomX = infoMorph_Basic.leftBottomX;
	placingZone.leftBottomY = infoMorph_Basic.leftBottomY;
	placingZone.leftBottomZ = infoMorph_Basic.leftBottomZ;
	placingZone.horLen = infoMorph_Basic.horLen;
	placingZone.verLenBasic = infoMorph_Basic.verLen;
	placingZone.verLenExtra = infoMorph_Extra.verLen;
	placingZone.ang = DegreeToRad(infoMorph_Basic.ang);

	// �۾� �� ���� ��������
	workLevel_wall = getWorkLevel(infoWall.floorInd);

	// ��ġ ������ �� ������ ����
	placingZone.leftBottomZ = infoWall.bottomOffset;

	// �ʱ� �� ���� ���
	placingZone.nCellsHor = (int)floor(placingZone.horLen / DEFAULT_TABLEFORM_WIDTH);
	placingZone.nCellsVerBasic = (int)floor(placingZone.verLenBasic / DEFAULT_EUROFORM_HEIGHT);
	placingZone.nCellsVerExtra = (int)floor(placingZone.verLenExtra / DEFAULT_EUROFORM_HEIGHT);
	placingZone.nMarginCellsVerBasic = 1;
	placingZone.nMarginCellsVerExtra = 1;

	// ��ġ ���� ����
	placingZone.initCells();

	bool exitCondition = false;
	int nSteps = 1;
	short result;

	while (exitCondition == false) {
		if (nSteps == 1) {
			// 1�� ���̾�α�: �� ����
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
			// 2�� ���̾�α�: ��� ���� ä��� (������)
			bool bFront = true;
			result = DGBlankModalDialog(550, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, handler2, (DGUserData)&bFront);
			
			if (result == DG_OK) {
				nSteps = 3;
				exitCondition = false;
			}
			else if (result == DG_CANCEL) {
				nSteps = 3;
				exitCondition = false;
			}
			else if (result == DG2_BUTTON_PREV) {
				nSteps = 1;
				exitCondition = false;
			}
		}

		if (nSteps == 3) {
			// 2�� ���̾�α�: ��� ���� ä��� (������)
			bool bFront = false;
			result = DGBlankModalDialog(550, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, handler2, (DGUserData)&bFront);

			if (result == DG_OK) {
				nSteps = 4;
				exitCondition = false;
			}
			else if (result == DG_CANCEL) {
				nSteps = 4;
				exitCondition = false;
			}
			else if (result == DG2_BUTTON_PREV) {
				nSteps = 1;
				exitCondition = false;
			}
		}

		if (nSteps == 4) {
			// 3�� ���̾�α�: ���̾� ����
			//result = DGModalDialog (ACAPI_GetOwnResModule (), 32501, ACAPI_GetOwnResModule (), layer_handler, 0);
			// -> DG_OK�̸� nSteps = 5, exitCondition = false
			// -> DG_CANCEL�̸� nSteps = 5, exitCondition = false
			// -> ���� ��ư�� ������ nSteps  = 2, exitCondition = false
			
			exitCondition = true;
		}

		//if (nSteps == 5) {
		//	// 4�� ���̾�α�: �ܿ��� ä��� [gap > EPS�϶�]
		//	if (placingZone.gap > EPS) {
		//		result = ...

		//		if (result == DG_OK) {
		//			nSteps = 6;
		//			exitCondition = true;
		//		}
		//		else if (result == DG_CANCEL) {
		//			nSteps = 6;
		//			exitCondition = true;
		//		}
		//		else if (result == DG4_BUTTON_PREV) {
		//			nSteps = 4;
		//			exitCondition = false;
		//		}
		//	}
		//	else {
		//		nSteps = 6;
		//		exitCondition = true;
		//	}
		//}
	}

	return err;
}

#endif