#ifndef	__EXPORT__
#define __EXPORT__

#include "MaxBIM21.h"

using namespace std;

namespace exportDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_1_exportDG {
		LABEL_DIST_BTW_COLUMN		= 3,
		EDITCONTROL_DIST_BTW_COLUMN
	};

	enum	layerTypeEnum {
		UNDEFINED,	// ���ǵ��� ����
		WALL,	// �� -> ���� �յ޸� ĸ���� ��
		SLAB,	// ������ -> ���� �Ʒ��� ĸ���� ��
		COLU,	// ��� -> ������ ĸ���� ��
		BEAM,	// �� -> ����, �Ϻ� ĸ���� ��
		WLBM	// ���纸 -> ����, �Ϻ� ĸ���� ��
	};

	enum	filterSelectionDG {
		BUTTON_ALL_SEL = 3,
		BUTTON_ALL_UNSEL,
		CHECKBOX_INCLUDE_UNKNOWN_OBJECT
	};

	enum	BEAMTABLEFORM_DIRECTION {
		HORIZONTAL_DIRECTION,
		VERTICAL_DIRECTION
	};

	enum	BEAMTABLEFORM_OBJECT_TYPE {
		NONE,
		EUROFORM,
		PLYWOOD,
		PLYWOOD_POLY
	};

	enum	ATTACH_POSITION {
		NO_POSITION,
		LEFT_SIDE,
		RIGHT_SIDE,
		BOTTOM_SIDE
	};
}

// ������ ������� ��� ����
class SummaryOfObjectInfo
{
public:
	SummaryOfObjectInfo ();		// ������
	void clear ();				// ���ڵ� ���� �����
	int	quantityPlus1 (vector<string> record);	// ��ü�� ���ڵ� ���� 1 ���� (������ ����, ������ �ű� �߰�)

public:
	// objectInfo.csv ���� ����
	vector<string>				keyName;	// ��ü�� ������ �� �ִ� ��(���ڿ�)�� ��� �ִ� ���� �̸� (��: u_comp)
	vector<string>				keyDesc;	// ��ü�� �̸� (��: ������)
	vector<int>					nInfo;		// ǥ���� ���� �ʵ� ����
	vector<vector<string>>		varName;	// ������ �̸� (��: eu_stan_onoff)		-- ���ο� �� �ٸ� vector<string>�� ���ԵǸ�, �װ��� ���̴� �ش� ��ü�� nInfo�� ����
	vector<vector<string>>		varDesc;	// ������ �̸��� ���� ���� (��: �԰���)	-- ���ο� �� �ٸ� vector<string>�� ���ԵǸ�, �װ��� ���̴� �ش� ��ü�� nInfo�� ����

	// ��ü�� ���� ������ ���պ� ����
	vector<vector<string>>		records;	// ��ü�� �̸�, ���� ������ ����, ���տ� �ش��ϴ� ��ü ������ ��� ���� (�ʵ� 1���� ����: ���ڳ��ǳ� | 100 | 100 | 1200 | 3)

	// ��Ÿ
	int nKnownObjects;						// ������ ��ü�� ����
	int nUnknownObjects;					// �������� ���� ��ü�� ����

	// ��ü�� ���� (Beam Ÿ��)
	vector<int>		beamLength;				// �� ����
	vector<int>		beamCount;				// �ش� �� ���̿� ���� ����
	int				nCountsBeam;			// �� ������ ����
};

// ���̴� ���̾� ���� ��ü�� ��Ī, ���� ����, ���̱� ����
struct VisibleObjectInfo
{
	// Object Ÿ��
	short	nKinds;				// ��ü ���� ����
	char	varName [100][64];	// 1��: ������
	char	objName [100][128];	// 2��: ��ü��
	bool	bExist [100];		// ���� ����
	bool	bShow [100];		// ǥ�� ����
	short	itmIdx [100];		// ���̾�α� �� �ε���

	// �˷����� ���� Object Ÿ���� ��ü
	bool	bShow_Unknown;
	long	nUnknownObjects;

	// ������ Ÿ��
	bool	bExist_Walls;
	bool	bShow_Walls;
	short	itmIdx_Walls;

	bool	bExist_Columns;
	bool	bShow_Columns;
	short	itmIdx_Columns;

	bool	bExist_Beams;
	bool	bShow_Beams;
	short	itmIdx_Beams;

	bool	bExist_Slabs;
	bool	bShow_Slabs;
	short	itmIdx_Slabs;

	bool	bExist_Roofs;
	bool	bShow_Roofs;
	short	itmIdx_Roofs;

	bool	bExist_Meshes;
	bool	bShow_Meshes;
	short	itmIdx_Meshes;

	bool	bExist_Morphs;
	bool	bShow_Morphs;
	short	itmIdx_Morphs;

	bool	bExist_Shells;
	bool	bShow_Shells;
	short	itmIdx_Shells;

	// �����ϴ� �׸� ��ü ����
	short	nItems;
};

// �� ���̺��� �� ����鿡 ���� ����
struct objectInBeamTableform
{
	short	objType;			// NONE, EUROFORM, PLYWOOD
	short	attachPosition;		// LEFT_SIDE, RIGHT_SIDE, BOTTOM_SIDE
	API_Coord3D	origin;			// ���� ��ǥ
	API_Coord3D	minPos, maxPos;	// �ּ���, �ִ��� ��ġ
	double	width;				// ��ü �ʺ�
	double	length;				// ��ü ����

	double	beginPos;			// ������ ��ġ
	double	endPos;				// ���� ��ġ
};

// �� ���̺��� ����ǥ �ۼ��� ���� ����ü�� ���Ǵ� �� �迭 ����
class BeamTableformCellArray
{
public:
	short	iBeamDirection;			// HORIZONTAL_DIRECTION, VERTICAL_DIRECTION
	double	cellElev [2];			// [n]: n+1�� ������
	double	cellPos [2];			// [n]: n+1�� X,Y��
	short	nCells_Left;			// ����ϴ� �� ����
	short	nCells_Right;			// ����ϴ� �� ����
	short	nCells_Bottom;			// ����ϴ� �� ����
	objectInBeamTableform	cells_Left [30][2];		// �� ���̺��� ���� �迭 (2�ܱ��� ����)
	objectInBeamTableform	cells_Right [30][2];	// �� ���̺��� ���� �迭 (2�ܱ��� ����)
	objectInBeamTableform	cells_Bottom [30][2];	// �� ���̺��� ���� �迭 (2�ܱ��� ����)

public:
	BeamTableformCellArray ();
	void	init ();
};

// ���̾� �̸� �� �ش� ���̾��� �ε���
class LayerList
{
public:
	short	layerInd;
	string	layerName;
};

void		initArray (double arr [], short arrSize);											// �迭 �ʱ�ȭ �Լ�
int			compare (const void* first, const void* second);									// ������������ ������ �� ����ϴ� ���Լ� (����Ʈ)
bool		comparePosX (const objectInBeamTableform& a, const objectInBeamTableform& b);		// vector �� ���� ���� ����ü ������ ���� �� �Լ� (��ǥ�� X ����)
bool		comparePosY (const objectInBeamTableform& a, const objectInBeamTableform& b);		// vector �� ���� ���� ����ü ������ ���� �� �Լ� (��ǥ�� Y ����)
bool		compareLayerName (const LayerList& a, const LayerList& b);							// vector �� ���̾� ���� ����ü ������ ���� �� �Լ� (���̾� �̸� ����)
bool		compareVectorString (const vector<string>& a, const vector<string>& b);				// vector �� ���ڵ� �� �ʵ带 �������� �������� ������ ���� �� �Լ�

GSErrCode	exportSelectedElementInfo (void);													// ������ ���� ���� �������� (Single ���)
GSErrCode	exportElementInfoOnVisibleLayers (void);											// ������ ���� ���� �������� (Multi ���)

int			quantityPlusN (vector<vector<string>> *db, vector<string> record, int n);			// ��ü�� ���ڵ� ���� n ����

GSErrCode	filterSelection (void);																// ���纰 ���� �� �����ֱ�
short		DGCALLBACK filterSelectionHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α�] ���̾�α׿��� ���̴� ���̾� �� �ִ� ��ü���� ������ �����ְ�, üũ�� ������ ��ü�鸸 ���� �� ������

GSErrCode	exportBeamTableformInformation (void);												// �� ���̺��� ���� ���� ��������

GSErrCode	calcTableformArea (void);															// ���̺��� ���� ���

GSErrCode	exportAllElevationsToPDFSingleMode (void);											// ��� �Ը鵵 PDF�� �������� (Single ���)
GSErrCode	exportAllElevationsToPDFMultiMode (void);											// ��� �Ը鵵 PDF�� �������� (Multi ���)

short DGCALLBACK scaleQuestionHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);			// [���̾�α�] ����ڰ� ��ô ���� ���� �Է��� �� �ֵ��� ��
short DGCALLBACK filenameQuestionHandler(short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α�] ���ϸ��� �Է��� �� �ֵ��� ��

#endif