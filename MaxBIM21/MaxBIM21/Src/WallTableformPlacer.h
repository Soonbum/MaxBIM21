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
				}
			}
		}
	};

	PlacingZone	placingZone;			// ��ġ ����
	InfoWall	infoWall;				// �� ��ü ����
	API_Guid	structuralObject;		// ���� ��ü�� GUID

	double DEFAULT_TABLEFORM_WIDTH = 2.250;
	double DEFAULT_EUROFORM_HEIGHT = 1.200;

	// �׸� �ε���
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

	short DGCALLBACK handler1(short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */) {
		short result;
		short itemIndex;
		short posX, posY;
		short sizeX, sizeY;

		char numStr[32];
		double remainHeight, remainWidth;
		GS::UniString buttonName;
		short targetIndex;
		
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

			// �ȳ� �ؽ�Ʈ
			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 120, 130, 380, 80);
			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itemIndex, L"�ȳ� �ؽ�Ʈ�Դϴ�.");
			DGShowItem(dialogID, itemIndex);

			// �� �׸��� ǥ��
			posX = 205; posY = 230;
			sizeX = 100; sizeY = 100;
			for (int i = 0; i < placingZone.nCellsVerBasic; i++) {
				for (short j = 0; j < placingZone.nCellsHor; j++) {
					itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX+(j*sizeX), posY+(i*sizeY), sizeX, sizeY);
					DGShowItem(dialogID, itemIndex);
					if (i == 0 && j == 0)
						GRID_START_INDEX = itemIndex;
				}
			}

			// ��ü Ÿ�� ���� ��ư ǥ��
			posX = 205; posY = 230 + (placingZone.nCellsVerBasic * sizeY) + 5;
			for (int i = 0; i < placingZone.nCellsHor; i++) {
				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX+(i*sizeX), posY, sizeX, 25);
				DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText(dialogID, itemIndex, objTypeStr[OBJ_WALL_TABLEFORM_A]);
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

			// ��ü ���� (�˾���Ʈ��)
			posX = 100; posY = 230 + (placingZone.nCellsVerBasic * sizeY) - 63;
			for (int i = 0; i < placingZone.nCellsVerBasic; i++) {
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
			DGSetDialogSize(dialogID, DG_CLIENT, 205 + (placingZone.nCellsHor * sizeX) + 100, 230 + (placingZone.nCellsVerBasic * sizeY) + 100, DG_TOPLEFT, true);

			// �⺻�� ����: ����
			DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_HEIGHT, placingZone.verLenBasic);
			remainHeight = placingZone.verLenBasic;
			for (int i = 0; i < placingZone.nCellsVerBasic; i++)
				remainHeight -= placingZone.cellsBasic[i][0].verLen;
			DGSetItemValDouble(dialogID, EDITCONTROL_REMAIN_HEIGHT, remainHeight);

			// �⺻�� ����: �ʺ�
			DGSetItemValDouble(dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.horLen);
			remainWidth = placingZone.horLen;
			for (int i = 0; i < placingZone.nCellsHor; i++)
				remainWidth -= placingZone.cellsBasic[0][i].horLen;
			DGSetItemValDouble(dialogID, EDITCONTROL_REMAIN_WIDTH, remainWidth);

			// �⺻�� ����: ���� ��ư
			DGSetItemValLong(dialogID, RADIOBUTTON_DUAL_SIDE, true);	// ��ġ��: ���
			DGSetItemValLong(dialogID, RADIOBUTTON_VERTICAL, true);		// ���̺��� ����: ����
			DGSetItemValLong(dialogID, RADIOBUTTON_PROPS_ON, true);		// Push-Pull Props: ��ġ
			DGSetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT, true);	// ǥ�ø� ����: �ո�

			break;

		case DG_MSG_CHANGE:
			//// ���̺��� ���� ���� ��ư �����
			//if (item == RADIOBUTTON_VERTICAL || item == RADIOBUTTON_HORIZONTAL) {
			//	// �� ���� �ʱ�ȭ
			//	placingZone.initCells();

			//	// ���� �����̸�,
			//	if (DGGetItemValLong(dialogID, RADIOBUTTON_VERTICAL) == 1) {
			//		// �ʺ� �˾���Ʈ�� ������ �����ٰ� �ٽ� ä��
			//		for (int i = BUTTON_OBJ_TYPE_START_INDEX; i <= BUTTON_OBJ_TYPE_START_INDEX + placingZone.nCellsHor; i++) {
			//			buttonName = DGGetItemText(dialogID, i);

			//			// ���̺���A, ���̺���B, ���̺���C�� ���,
			//			if ((buttonName.Compare(L"���̺���A") == 1) || (buttonName.Compare(L"���̺���B") == 1) || (buttonName.Compare(L"���̺���C") == 1)) {
			//				targetIndex = POPUP_OBJ_WIDTH_START_INDEX + (i - BUTTON_OBJ_TYPE_START_INDEX);
			//				DGPopUpDeleteItem(dialogID, targetIndex, DG_ALL_ITEMS);
			//				for (int j = 0; j < sizeof(placingZone.tableformPresetVertical) / sizeof(int); j++) {
			//					DGPopUpInsertItem(dialogID, targetIndex, DG_POPUP_BOTTOM);
			//					_itoa(placingZone.tableformPresetVertical[j], numStr, 10);
			//					DGPopUpSetItemText(dialogID, targetIndex, DG_POPUP_BOTTOM, numStr);
			//				}
			//				DGPopUpSelectItem(dialogID, targetIndex, DG_POPUP_TOP);
			//			}
			//			// �������� ���,
			//			else if (buttonName.Compare(L"������") == 1) {
			//				targetIndex = POPUP_OBJ_WIDTH_START_INDEX + (i - BUTTON_OBJ_TYPE_START_INDEX);
			//				DGPopUpDeleteItem(dialogID, targetIndex, DG_ALL_ITEMS);
			//				for (int j = 0; j < sizeof(placingZone.euroformPresetVertical) / sizeof(int); j++) {
			//					DGPopUpInsertItem(dialogID, targetIndex, DG_POPUP_BOTTOM);
			//					_itoa(placingZone.euroformPresetVertical[j], numStr, 10);
			//					DGPopUpSetItemText(dialogID, targetIndex, DG_POPUP_BOTTOM, numStr);
			//				}
			//				DGPopUpSelectItem(dialogID, targetIndex, DG_POPUP_TOP);
			//			}
			//		}

			//		// ���� �˾���Ʈ�� ������ �����ٰ� �ٽ� ä�� (������)
			//		if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1) {
			//			for (int i = POPUP_OBJ_HEIGHT_START_INDEX; i <= POPUP_OBJ_HEIGHT_START_INDEX + placingZone.nCellsVerBasic; i++) {
			//				DGPopUpDeleteItem(dialogID, i, DG_ALL_ITEMS);
			//				for (int j = 0; j < sizeof(placingZone.euroformPresetHorizontal) / sizeof(int); j++) {
			//					DGPopUpInsertItem(dialogID, i, DG_POPUP_BOTTOM);
			//					_itoa(placingZone.euroformPresetHorizontal[j], numStr, 10);
			//					DGPopUpSetItemText(dialogID, i, DG_POPUP_BOTTOM, numStr);
			//				}
			//				DGPopUpSelectItem(dialogID, i, DG_POPUP_TOP);
			//			}
			//		}
			//		// ���� �˾���Ʈ�� ������ �����ٰ� �ٽ� ä�� (������)
			//		else {
			//			for (int i = POPUP_OBJ_HEIGHT_START_INDEX; i <= POPUP_OBJ_HEIGHT_START_INDEX + placingZone.nCellsVerExtra; i++) {
			//				for (int i = POPUP_OBJ_HEIGHT_START_INDEX; i <= POPUP_OBJ_HEIGHT_START_INDEX + placingZone.nCellsVerExtra; i++) {
			//					DGPopUpDeleteItem(dialogID, i, DG_ALL_ITEMS);
			//					for (int j = 0; j < sizeof(placingZone.euroformPresetHorizontal) / sizeof(int); j++) {
			//						DGPopUpInsertItem(dialogID, i, DG_POPUP_BOTTOM);
			//						_itoa(placingZone.euroformPresetHorizontal[j], numStr, 10);
			//						DGPopUpSetItemText(dialogID, i, DG_POPUP_BOTTOM, numStr);
			//					}
			//					DGPopUpSelectItem(dialogID, i, DG_POPUP_TOP);
			//				}
			//			}
			//		}
			//	}
			//	// ���� �����̸�,
			//	else {
			//		// �ʺ� �˾���Ʈ�� ������ �����ٰ� �ٽ� ä��
			//		for (int i = BUTTON_OBJ_TYPE_START_INDEX; i <= BUTTON_OBJ_TYPE_START_INDEX + placingZone.nCellsHor; i++) {
			//			buttonName = DGGetItemText(dialogID, i);

			//			// ���̺���A, ���̺���B, ���̺���C�� ���,
			//			if ((buttonName.Compare(L"���̺���A") == 1) || (buttonName.Compare(L"���̺���B") == 1) || (buttonName.Compare(L"���̺���C") == 1)) {
			//				targetIndex = POPUP_OBJ_WIDTH_START_INDEX + (i - BUTTON_OBJ_TYPE_START_INDEX);
			//				DGPopUpDeleteItem(dialogID, targetIndex, DG_ALL_ITEMS);
			//				for (int j = 0; j < sizeof(placingZone.tableformPresetHorizontal) / sizeof(int); j++) {
			//					DGPopUpInsertItem(dialogID, targetIndex, DG_POPUP_BOTTOM);
			//					_itoa(placingZone.tableformPresetHorizontal[j], numStr, 10);
			//					DGPopUpSetItemText(dialogID, targetIndex, DG_POPUP_BOTTOM, numStr);
			//				}
			//				DGPopUpSelectItem(dialogID, targetIndex, DG_POPUP_TOP);
			//			}
			//			// �������� ���,
			//			else if (buttonName.Compare(L"������") == 1) {
			//				targetIndex = POPUP_OBJ_WIDTH_START_INDEX + (i - BUTTON_OBJ_TYPE_START_INDEX);
			//				DGPopUpDeleteItem(dialogID, targetIndex, DG_ALL_ITEMS);
			//				for (int j = 0; j < sizeof(placingZone.euroformPresetHorizontal) / sizeof(int); j++) {
			//					DGPopUpInsertItem(dialogID, targetIndex, DG_POPUP_BOTTOM);
			//					_itoa(placingZone.euroformPresetHorizontal[j], numStr, 10);
			//					DGPopUpSetItemText(dialogID, targetIndex, DG_POPUP_BOTTOM, numStr);
			//				}
			//				DGPopUpSelectItem(dialogID, targetIndex, DG_POPUP_TOP);
			//			}
			//		}

			//		// ���� �˾���Ʈ�� ������ �����ٰ� �ٽ� ä�� (������)
			//		if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1) {
			//			for (int i = POPUP_OBJ_HEIGHT_START_INDEX; i <= POPUP_OBJ_HEIGHT_START_INDEX + placingZone.nCellsVerBasic; i++) {
			//				DGPopUpDeleteItem(dialogID, i, DG_ALL_ITEMS);
			//				for (int j = 0; j < sizeof(placingZone.euroformPresetVertical) / sizeof(int); j++) {
			//					DGPopUpInsertItem(dialogID, i, DG_POPUP_BOTTOM);
			//					_itoa(placingZone.euroformPresetVertical[j], numStr, 10);
			//					DGPopUpSetItemText(dialogID, i, DG_POPUP_BOTTOM, numStr);
			//				}
			//				DGPopUpSelectItem(dialogID, i, DG_POPUP_TOP);
			//			}
			//		}
			//		// ���� �˾���Ʈ�� ������ �����ٰ� �ٽ� ä�� (������)
			//		else {
			//			for (int i = POPUP_OBJ_HEIGHT_START_INDEX; i <= POPUP_OBJ_HEIGHT_START_INDEX + placingZone.nCellsVerExtra; i++) {
			//				for (int i = POPUP_OBJ_HEIGHT_START_INDEX; i <= POPUP_OBJ_HEIGHT_START_INDEX + placingZone.nCellsVerExtra; i++) {
			//					DGPopUpDeleteItem(dialogID, i, DG_ALL_ITEMS);
			//					for (int j = 0; j < sizeof(placingZone.euroformPresetVertical) / sizeof(int); j++) {
			//						DGPopUpInsertItem(dialogID, i, DG_POPUP_BOTTOM);
			//						_itoa(placingZone.euroformPresetVertical[j], numStr, 10);
			//						DGPopUpSetItemText(dialogID, i, DG_POPUP_BOTTOM, numStr);
			//					}
			//					DGPopUpSelectItem(dialogID, i, DG_POPUP_TOP);
			//				}
			//			}
			//		}
			//	}
			//}

			//// �ո�/�޸� ���ý�
			//if (item == RADIOBUTTON_SHOW_FRONT || item == RADIOBUTTON_SHOW_BACK) {
			//	// �ո����� ��ȯ�ϸ�,
			//	if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1) {
			//		// ���� ��� ��� ����
			//		DGRemoveDialogItems(dialogID, GRID_START_INDEX);

			//		// �� �׸��� ǥ��
			//		posX = 205; posY = 230;
			//		sizeX = 100; sizeY = 100;
			//		for (int i = 0; i < placingZone.nCellsVerBasic; i++) {
			//			for (short j = 0; j < placingZone.nCellsHor; j++) {
			//				itemIndex = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, posX + (j * sizeX), posY + (i * sizeY), sizeX, sizeY);
			//				DGShowItem(dialogID, itemIndex);
			//				if (i == 0 && j == 0)
			//					GRID_START_INDEX = itemIndex;
			//			}
			//		}

			//		// ��ü Ÿ�� ���� ��ư ǥ��
			//		posX = 205; posY = 230 + (placingZone.nCellsVerBasic * sizeY) + 5;
			//		for (int i = 0; i < placingZone.nCellsHor; i++) {
			//			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (i * sizeX), posY, sizeX, 25);
			//			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			//			DGSetItemText(dialogID, itemIndex, convertStr(objTypeStr[OBJ_WALL_TABLEFORM_A]));
			//			DGShowItem(dialogID, itemIndex);
			//			if (i == 0)
			//				BUTTON_OBJ_TYPE_START_INDEX = itemIndex;
			//		}

			//		// ��ü �ʺ� (�˾���Ʈ��)
			//		posX = 205; posY = 230 + (placingZone.nCellsVerBasic * sizeY) + 35;
			//		for (int i = 0; i < placingZone.nCellsHor; i++) {
			//			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_POPUPCONTROL, 200, 0, posX + (i * sizeX), posY, sizeX, 25);
			//			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			//			//DGShowItem(dialogID, itemIndex);
			//			if (i == 0)
			//				POPUP_OBJ_WIDTH_START_INDEX = itemIndex;
			//		}

			//		// ��ü �ʺ� (Edit��Ʈ��)
			//		for (int i = 0; i < placingZone.nCellsHor; i++) {
			//			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, posX + (i * sizeX), posY, sizeX, 25);
			//			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			//			//DGShowItem(dialogID, itemIndex);
			//			if (i == 0)
			//				EDITCONTROL_OBJ_WIDTH_START_INDEX = itemIndex;
			//		}

			//		// �� ���� ��ü �߰� ��ư
			//		itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX), posY, 25, 25);
			//		DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			//		DGSetItemText(dialogID, itemIndex, "+");
			//		DGShowItem(dialogID, itemIndex);
			//		BUTTON_ADD_COLUMN = itemIndex;

			//		// �� ���� ��ü ���� ��ư
			//		itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + (placingZone.nCellsHor * sizeX) + 25, posY, 25, 25);
			//		DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			//		DGSetItemText(dialogID, itemIndex, "-");
			//		DGShowItem(dialogID, itemIndex);
			//		BUTTON_DEL_COLUMN = itemIndex;

			//		// ��ü ���� (�˾���Ʈ��)
			//		posX = 100; posY = 230 + (placingZone.nCellsVerBasic * sizeY) - 63;
			//		for (int i = 0; i < placingZone.nCellsVerBasic; i++) {
			//			itemIndex = DGAppendDialogItem(dialogID, DG_ITM_POPUPCONTROL, 200, 0, posX, posY - (i * sizeY), sizeX, 25);
			//			DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			//			DGShowItem(dialogID, itemIndex);
			//			if (i == 0)
			//				POPUP_OBJ_HEIGHT_START_INDEX = itemIndex;
			//		}

			//		// �� ���� ��ü �߰� ��ư
			//		posX = 100; posY = 230;
			//		itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 25, posY - 25, 25, 25);
			//		DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			//		DGSetItemText(dialogID, itemIndex, "+");
			//		DGShowItem(dialogID, itemIndex);
			//		BUTTON_ADD_ROW = itemIndex;

			//		// �� ���� ��ü ���� ��ư
			//		itemIndex = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, posX + 50, posY - 25, 25, 25);
			//		DGSetItemFont(dialogID, itemIndex, DG_IS_LARGE | DG_IS_BOLD);
			//		DGSetItemText(dialogID, itemIndex, "-");
			//		DGShowItem(dialogID, itemIndex);
			//		BUTTON_DEL_ROW = itemIndex;

			//		// ���̾�α� â ũ�� ����
			//		DGSetDialogSize(dialogID, DG_CLIENT, 205 + (placingZone.nCellsHor * sizeX) + 100, 230 + (placingZone.nCellsVerBasic * sizeY) + 100, DG_TOPLEFT, true);

			//		// ��ü Ÿ�� ���� ��ư ����
			//		for (int i = 0; i < placingZone.nCellsHor; i++) {
			//			DGSetItemText(dialogID, BUTTON_OBJ_TYPE_START_INDEX + i, convertStr(objTypeStr[placingZone.cellsBasic[0][i].objType]));
			//		}

			//		// ���� �����̸�,
			//		if (DGGetItemValLong(dialogID, RADIOBUTTON_VERTICAL) == 1) {
			//			// �ʺ� �˾���Ʈ�� ������ �����ٰ� �ٽ� ä��
			//			for (int i = BUTTON_OBJ_TYPE_START_INDEX; i <= BUTTON_OBJ_TYPE_START_INDEX + placingZone.nCellsHor; i++) {
			//				buttonName = DGGetItemText(dialogID, i);

			//				// ���̺���A, ���̺���B, ���̺���C�� ���,
			//				if ((buttonName.Compare(L"���̺���A") == 1) || (buttonName.Compare(L"���̺���B") == 1) || (buttonName.Compare(L"���̺���C") == 1)) {
			//					targetIndex = POPUP_OBJ_WIDTH_START_INDEX + (i - BUTTON_OBJ_TYPE_START_INDEX);
			//					DGPopUpDeleteItem(dialogID, targetIndex, DG_ALL_ITEMS);
			//					for (int j = 0; j < sizeof(placingZone.tableformPresetVertical) / sizeof(int); j++) {
			//						DGPopUpInsertItem(dialogID, targetIndex, DG_POPUP_BOTTOM);
			//						_itoa(placingZone.tableformPresetVertical[j], numStr, 10);
			//						DGPopUpSetItemText(dialogID, targetIndex, DG_POPUP_BOTTOM, numStr);
			//					}
			//					DGPopUpSelectItem(dialogID, targetIndex, DG_POPUP_TOP);
			//				}
			//				// �������� ���,
			//				else if (buttonName.Compare(L"������") == 1) {
			//					targetIndex = POPUP_OBJ_WIDTH_START_INDEX + (i - BUTTON_OBJ_TYPE_START_INDEX);
			//					DGPopUpDeleteItem(dialogID, targetIndex, DG_ALL_ITEMS);
			//					for (int j = 0; j < sizeof(placingZone.euroformPresetVertical) / sizeof(int); j++) {
			//						DGPopUpInsertItem(dialogID, targetIndex, DG_POPUP_BOTTOM);
			//						_itoa(placingZone.euroformPresetVertical[j], numStr, 10);
			//						DGPopUpSetItemText(dialogID, targetIndex, DG_POPUP_BOTTOM, numStr);
			//					}
			//					DGPopUpSelectItem(dialogID, targetIndex, DG_POPUP_TOP);
			//				}
			//			}

			//			// ���� �˾���Ʈ�� ������ �����ٰ� �ٽ� ä�� (������)
			//			for (int i = POPUP_OBJ_HEIGHT_START_INDEX; i <= POPUP_OBJ_HEIGHT_START_INDEX + placingZone.nCellsVerBasic; i++) {
			//				DGPopUpDeleteItem(dialogID, i, DG_ALL_ITEMS);
			//				for (int j = 0; j < sizeof(placingZone.euroformPresetHorizontal) / sizeof(int); j++) {
			//					DGPopUpInsertItem(dialogID, i, DG_POPUP_BOTTOM);
			//					_itoa(placingZone.euroformPresetHorizontal[j], numStr, 10);
			//					DGPopUpSetItemText(dialogID, i, DG_POPUP_BOTTOM, numStr);
			//				}
			//				DGPopUpSelectItem(dialogID, i, DG_POPUP_TOP);
			//			}
			//		}
			//		// ���� �����̸�,
			//		else {
			//			// �ʺ� �˾���Ʈ�� ������ �����ٰ� �ٽ� ä��
			//			for (int i = BUTTON_OBJ_TYPE_START_INDEX; i <= BUTTON_OBJ_TYPE_START_INDEX + placingZone.nCellsHor; i++) {
			//				buttonName = DGGetItemText(dialogID, i);

			//				// ���̺���A, ���̺���B, ���̺���C�� ���,
			//				if ((buttonName.Compare(L"���̺���A") == 1) || (buttonName.Compare(L"���̺���B") == 1) || (buttonName.Compare(L"���̺���C") == 1)) {
			//					targetIndex = POPUP_OBJ_WIDTH_START_INDEX + (i - BUTTON_OBJ_TYPE_START_INDEX);
			//					DGPopUpDeleteItem(dialogID, targetIndex, DG_ALL_ITEMS);
			//					for (int j = 0; j < sizeof(placingZone.tableformPresetHorizontal) / sizeof(int); j++) {
			//						DGPopUpInsertItem(dialogID, targetIndex, DG_POPUP_BOTTOM);
			//						_itoa(placingZone.tableformPresetHorizontal[j], numStr, 10);
			//						DGPopUpSetItemText(dialogID, targetIndex, DG_POPUP_BOTTOM, numStr);
			//					}
			//					DGPopUpSelectItem(dialogID, targetIndex, DG_POPUP_TOP);
			//				}
			//				// �������� ���,
			//				else if (buttonName.Compare(L"������") == 1) {
			//					targetIndex = POPUP_OBJ_WIDTH_START_INDEX + (i - BUTTON_OBJ_TYPE_START_INDEX);
			//					DGPopUpDeleteItem(dialogID, targetIndex, DG_ALL_ITEMS);
			//					for (int j = 0; j < sizeof(placingZone.euroformPresetHorizontal) / sizeof(int); j++) {
			//						DGPopUpInsertItem(dialogID, targetIndex, DG_POPUP_BOTTOM);
			//						_itoa(placingZone.euroformPresetHorizontal[j], numStr, 10);
			//						DGPopUpSetItemText(dialogID, targetIndex, DG_POPUP_BOTTOM, numStr);
			//					}
			//					DGPopUpSelectItem(dialogID, targetIndex, DG_POPUP_TOP);
			//				}
			//			}

			//			// ���� �˾���Ʈ�� ������ �����ٰ� �ٽ� ä�� (������)
			//			for (int i = POPUP_OBJ_HEIGHT_START_INDEX; i <= POPUP_OBJ_HEIGHT_START_INDEX + placingZone.nCellsVerBasic; i++) {
			//				DGPopUpDeleteItem(dialogID, i, DG_ALL_ITEMS);
			//				for (int j = 0; j < sizeof(placingZone.euroformPresetVertical) / sizeof(int); j++) {
			//					DGPopUpInsertItem(dialogID, i, DG_POPUP_BOTTOM);
			//					_itoa(placingZone.euroformPresetVertical[j], numStr, 10);
			//					DGPopUpSetItemText(dialogID, i, DG_POPUP_BOTTOM, numStr);
			//				}
			//				DGPopUpSelectItem(dialogID, i, DG_POPUP_TOP);
			//			}
			//		}

			//		// �ʺ� ���� �˾���Ʈ��, Ȥ�� Edit��Ʈ�� ǥ�� ���� �� �� ����
			//		for (int i = 0; i < placingZone.nCellsHor; i++) {
			//			buttonName = DGGetItemText(dialogID, BUTTON_OBJ_TYPE_START_INDEX + i);

			//			// ���̺���A, ���̺���B, ���̺���C�� ���,
			//			if ((buttonName.Compare(L"���̺���A") == 1) || (buttonName.Compare(L"���̺���B") == 1) || (buttonName.Compare(L"���̺���C") == 1)) {
			//				for (int j = 0; j < sizeof(placingZone.tableformPresetVertical) / sizeof(int); j++) {
			//					if ((int)floor(placingZone.cellsBasic[0][i].horLen * 1000) == placingZone.tableformPresetVertical[j]) {
			//						DGPopUpSelectItem(dialogID, POPUP_OBJ_WIDTH_START_INDEX + i, j + 1);
			//						DGShowItem(dialogID, POPUP_OBJ_WIDTH_START_INDEX + i);
			//					}
			//				}
			//			}
			//			// �������� ���,
			//			else if (buttonName.Compare(L"������") == 1) {
			//				for (int j = 0; j < sizeof(placingZone.euroformPresetVertical) / sizeof(int); j++) {
			//					if ((int)floor(placingZone.cellsBasic[0][i].horLen * 1000) == placingZone.euroformPresetVertical[j]) {
			//						DGPopUpSelectItem(dialogID, POPUP_OBJ_WIDTH_START_INDEX + i, j + 1);
			//						DGShowItem(dialogID, POPUP_OBJ_WIDTH_START_INDEX + i);
			//					}
			//				}
			//			}
			//			// ������ ��ü�� ���,
			//			else {
			//				DGSetItemValDouble(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX, placingZone.cellsBasic[0][i].horLen);
			//				DGShowItem(dialogID, EDITCONTROL_OBJ_WIDTH_START_INDEX + i);
			//			}
			//		}

			//		// ���̿� ���� �˾���Ʈ��, Ȥ�� Edit��Ʈ�� ǥ�� ���� �� �� ����
			//		for (int i = 0; i < placingZone.nCellsVerBasic; i++) {
			//			// ... 
			//		}
			//	}
			//	// �޸����� ��ȯ�ϸ�,
			//	else {
			//		// ...
			//	}
			//}
			//
			//// ... �ʺ� �˾���Ʈ��
			//// ... �ʺ� Edit��Ʈ��
			//	// ... ���� �ʺ� ������Ʈ
			//
			//// ... ���� �˾���Ʈ��
			//// ... ���� Edit��Ʈ��
			//	// ... ���� ���� ������Ʈ

			//// ... ��ư �̸��� ���� �˾���Ʈ�� �Ǵ� Edit��Ʈ���� ǥ����

			//// ... ��ġ ���� ���� (������ ��ġ�� ������ ������)
			//for (int i = 0; i < placingZone.nCellsHor; i++) {
			//	buttonName = DGGetItemText(dialogID, BUTTON_OBJ_TYPE_START_INDEX + i);

			//	// ... ��ư �̸��� ���� Ÿ�� ����
			//	//for (int j = 0; j < sizeof(objTypeStr[j]) / sizeof(char) * 32; j++)
			//	//	if (buttonName.Compare(convertStr(objTypeStr[j])) == 1)
			//	//		;
			//	
			//	// ... ���̺���/�������̸� �˾���Ʈ�� ������� �ʺ� �� ����, �ƴϸ� Edit��Ʈ�ѷ� �ʺ� �� ����
			//	// ���̺���A, ���̺���B, ���̺���C�� ���,
			//	if ((buttonName.Compare(L"���̺���A") == 1) || (buttonName.Compare(L"���̺���B") == 1) || (buttonName.Compare(L"���̺���C") == 1)) {
			//		//
			//	}
			//	// �������� ���,
			//	else if (buttonName.Compare(L"������") == 1) {
			//		//
			//	}
			//}

			//if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_FRONT) == 1) {
			//	// ... �˾���Ʈ�� ������� ���� �� ���� (������)
			//	for (int i = 0; i < placingZone.nCellsVerBasic; i++) {
			//	}
			//}
			//else if (DGGetItemValLong(dialogID, RADIOBUTTON_SHOW_BACK) == 1) {
			//	// ... �˾���Ʈ�� ������� ���� �� ���� (������)
			//	for (int i = 0; i < placingZone.nCellsVerExtra; i++) {
			//	}
			//}

			break;

		case DG_MSG_CLICK:
			// ... OK ��ư
			
			// ... Cancel ��ư
			
			// ... �ո� ��ư
			
			// ... �޸� ��ư
			
			// ... �� ���� ���� ��ư
			
			// ... �� �߰� ��ư
			
			// ... �� ���� ��ư
			
			// ... �� �߰� ��ư
			
			// ... �� ���� ��ư
			
			// ... ��ü Ÿ�� ��ư

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

	// ��ġ ���� ����
	placingZone.initCells();

	bool exitCondition = false;
	int nSteps = 1;
	short result;

	while (exitCondition == false) {
		if (nSteps == 1) {
			// 1�� ���̾�α�: �� ����
			result = DGBlankModalDialog(550, 500, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, handler1, 0);
			// -> DG_OK�̸� nSteps = 2
			// -> DG_CANCEL�̸� nSteps = 0, exitCondition = true
			exitCondition = true;
		}

		if (nSteps == 2) {
			// 2�� ���̾�α�: ��� ���� ä��� (������)
			// -> DG_OK�̸� nSteps = 3, exitCondition = false
			// -> DG_CANCEL�̸� nSteps = 3, exitCondition = false
			// -> ���� ��ư�� ������ nSteps = 1, exitCondition = false
		}

		if (nSteps == 3) {
			// 3�� ���̾�α�: ��� ���� ä��� (������) [bExtra == true�϶�]
			// -> DG_OK�̸� nSteps = 4, exitCondition = false
			// -> DG_CANCEL�̸� nSteps = 4, exitCondition = false
			// -> ���� ��ư�� ������ nSteps = 1, exitCondition = false
		}

		if (nSteps == 4) {
			// 4�� ���̾�α�: �ܿ��� ä��� [gap > EPS�϶�]
			// -> DG_OK�̸� nSteps = 5, exitCondition = false
			// -> DG_CANCEL�̸� nSteps = 5, exitCondition = false
			// -> ���� ��ư�� ������ nSteps = 2, exitCondition = false
		}

		if (nSteps == 5) {
			// 5�� ���̾�α�: ���̾� ����
			// -> DG_OK�̸� nSteps = 0, exitCondition = true
			// -> DG_CANCEL�̸� err = NoError, return err
			// -> ���� ��ư�� ������ nSteps  = 4, exitCondition = false
		}
	}

	return err;
}

#endif