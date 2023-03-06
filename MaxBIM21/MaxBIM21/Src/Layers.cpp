#include <cstdio>
#include <cstdlib>
#include "MaxBIM21.h"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.h"
#include "Layers.h"

using namespace	layersDG;

LayerNameSystem	layerInfo;
LayerNameSystem	selectedInfo;
StatusOfLayerNameSystem	selectedInfoSaved;	// ����� ��ư ���¸� ����� �ε���

short	changedBtnItemIdx;	// ���°� ����� ��ư�� �׸� �ε���
short	clickedBtnItemIdx;	// Ŭ���� ��ư�� �׸� �ε���

short	BUTTON_LOAD;
short	BUTTON_FILTER;
short	BUTTON_UNFILTER;
short	LABEL_CODE;
short	LABEL_LAYER_NAME;
short	SELECTALL_1_CONTYPE;
short	SELECTALL_2_DONG;
short	SELECTALL_3_FLOOR;
short	SELECTALL_4_CAST;
short	SELECTALL_5_CJ;
short	SELECTALL_6_ORDER;
short	SELECTALL_7_OBJ;
short	SELECTALL_8_PRODUCT_SITE;
short	SELECTALL_9_PRODUCT_NUM;

short	DONG_STRING_RADIOBUTTON;
short	DONG_STRING_EDITCONTROL;	// 0000,0101,0102 �� ���ڷ� �� �� ���ڿ��� ��� Edit ��Ʈ���� �ε��� 
short	DONG_REST_BUTTONS[100];	// ���ڰ� �ƴ� �� ���ڿ� üũ��ư�� �ε���
short	OBJ_BUTTONS[100];			// ���� ��ư�� �ε���
short	PRODUCT_NUM_STRING_RADIOBUTTON;
short	PRODUCT_NUM_STRING_EDITCONTROL;		// 001-100 �� ���ڷ� �� ���� ��ȣ ���ڿ��� ��� Edit ��Ʈ���� �ε���


// �޸� �Ҵ�
void		allocateMemory(LayerNameSystem* layerInfo)
{
	short	xx;

	// ���̾� ���� �޸� �Ҵ�
	layerInfo->code_state = new bool[layerInfo->code_name.size()];
	layerInfo->dong_state = new bool[layerInfo->dong_name.size()];
	layerInfo->floor_state = new bool[layerInfo->floor_name.size()];
	layerInfo->cast_state = new bool[layerInfo->cast_name.size()];
	layerInfo->CJ_state = new bool[layerInfo->CJ_name.size()];
	layerInfo->orderInCJ_state = new bool[layerInfo->orderInCJ_name.size()];
	layerInfo->obj_state = new bool[layerInfo->obj_name.size()];
	layerInfo->productSite_state = new bool[layerInfo->productSite_name.size()];
	layerInfo->productNum_state = new bool[layerInfo->productNum_name.size()];

	layerInfo->code_idx = new short[layerInfo->code_name.size()];
	layerInfo->dong_idx = new short[layerInfo->dong_name.size()];
	layerInfo->floor_idx = new short[layerInfo->floor_name.size()];
	layerInfo->cast_idx = new short[layerInfo->cast_name.size()];
	layerInfo->CJ_idx = new short[layerInfo->CJ_name.size()];
	layerInfo->orderInCJ_idx = new short[layerInfo->orderInCJ_name.size()];
	layerInfo->obj_idx = new short[layerInfo->obj_name.size()];
	layerInfo->productSite_idx = new short[layerInfo->productSite_name.size()];
	layerInfo->productNum_idx = new short[layerInfo->productNum_name.size()];

	// �迭 �ʱ�ȭ
	for (xx = 0; xx < layerInfo->code_name.size(); ++xx) {
		layerInfo->code_state[xx] = false;
		layerInfo->code_idx[xx] = 0;
	}
	for (xx = 0; xx < layerInfo->dong_name.size(); ++xx) {
		layerInfo->dong_state[xx] = false;
		layerInfo->dong_idx[xx] = 0;
	}
	for (xx = 0; xx < layerInfo->floor_name.size(); ++xx) {
		layerInfo->floor_state[xx] = false;
		layerInfo->floor_idx[xx] = 0;
	}
	for (xx = 0; xx < layerInfo->cast_name.size(); ++xx) {
		layerInfo->cast_state[xx] = false;
		layerInfo->cast_idx[xx] = 0;
	}
	for (xx = 0; xx < layerInfo->CJ_name.size(); ++xx) {
		layerInfo->CJ_state[xx] = false;
		layerInfo->CJ_idx[xx] = 0;
	}
	for (xx = 0; xx < layerInfo->orderInCJ_name.size(); ++xx) {
		layerInfo->orderInCJ_state[xx] = false;
		layerInfo->orderInCJ_idx[xx] = 0;
	}
	for (xx = 0; xx < layerInfo->obj_name.size(); ++xx) {
		layerInfo->obj_state[xx] = false;
		layerInfo->obj_idx[xx] = 0;
	}
	for (xx = 0; xx < layerInfo->productSite_name.size(); ++xx) {
		layerInfo->productSite_state[xx] = false;
		layerInfo->productSite_idx[xx] = 0;
	}
	for (xx = 0; xx < layerInfo->productNum_name.size(); ++xx) {
		layerInfo->productNum_state[xx] = false;
		layerInfo->productNum_idx[xx] = 0;
	}
}

// �޸� ����
void		deallocateMemory(LayerNameSystem* layerInfo)
{
	// ���̾� ���� �޸� ����
	delete[]	layerInfo->code_state;
	delete[]	layerInfo->dong_state;
	delete[]	layerInfo->floor_state;
	delete[]	layerInfo->cast_state;
	delete[]	layerInfo->CJ_state;
	delete[]	layerInfo->orderInCJ_state;
	delete[]	layerInfo->obj_state;
	delete[]	layerInfo->productSite_state;
	delete[]	layerInfo->productNum_state;

	delete[]	layerInfo->code_idx;
	delete[]	layerInfo->dong_idx;
	delete[]	layerInfo->floor_idx;
	delete[]	layerInfo->cast_idx;
	delete[]	layerInfo->CJ_idx;
	delete[]	layerInfo->orderInCJ_idx;
	delete[]	layerInfo->obj_idx;
	delete[]	layerInfo->productSite_idx;
	delete[]	layerInfo->productNum_idx;
}

// ���̾� �ʵ� �ڵ忡 ������ ���°�?
bool		isFullLayer(LayerNameSystem* layerInfo)
{
	short	xx;
	short	anyTrue = 0;
	short	totalTrue = 0;
	short	AddedTotalTrue = 0;

	// ���� ����
	anyTrue = 0;
	for (xx = 0; xx < layerInfo->code_name.size(); ++xx)
		if (layerInfo->code_state[xx] == true)	anyTrue++;
	if (anyTrue > 0) {
		totalTrue++;
	}

	// �� ����
	anyTrue = 0;
	for (xx = 0; xx < layerInfo->dong_name.size(); ++xx)
		if (layerInfo->dong_state[xx] == true)	anyTrue++;
	if (anyTrue > 0) {
		totalTrue++;
	}

	// �� ����
	anyTrue = 0;
	for (xx = 0; xx < layerInfo->floor_name.size(); ++xx)
		if (layerInfo->floor_state[xx] == true)	anyTrue++;
	if (anyTrue > 0) {
		totalTrue++;
	}

	// Ÿ�� ��ȣ
	anyTrue = 0;
	for (xx = 0; xx < layerInfo->cast_name.size(); ++xx)
		if (layerInfo->cast_state[xx] == true) anyTrue++;
	if (anyTrue > 0) {
		totalTrue++;
	}

	// CJ
	anyTrue = 0;
	for (xx = 0; xx < layerInfo->CJ_name.size(); ++xx)
		if (layerInfo->CJ_state[xx] == true)	anyTrue++;
	if (anyTrue > 0) {
		totalTrue++;
	}

	// �ð�����
	anyTrue = 0;
	for (xx = 0; xx < layerInfo->orderInCJ_name.size(); ++xx)
		if (layerInfo->orderInCJ_state[xx] == true)	anyTrue++;
	if (anyTrue > 0) {
		totalTrue++;
	}

	// ����
	anyTrue = 0;
	for (xx = 0; xx < layerInfo->obj_name.size(); ++xx)
		if (layerInfo->obj_state[xx] == true)	anyTrue++;
	if (anyTrue > 0) {
		totalTrue++;
	}

	// ����ó ����
	anyTrue = 0;
	for (xx = 0; xx < layerInfo->productSite_name.size(); ++xx)
		if (layerInfo->productSite_state[xx] == true)	anyTrue++;
	if (anyTrue > 0) {
		AddedTotalTrue++;
	}

	// ���� ��ȣ
	anyTrue = 0;
	for (xx = 0; xx < layerInfo->productNum_name.size(); ++xx)
		if (layerInfo->productNum_state[xx] == true)	anyTrue++;
	if (anyTrue > 0) {
		AddedTotalTrue++;
	}

	// �⺻��
	if (layerInfo->extendedLayer == false) {
		if (totalTrue == 7)
			return true;
		else
			return false;

		// Ȯ����
	}
	else {
		if ((totalTrue == 7) && (AddedTotalTrue == 2))
			return true;
		else
			return false;
	}
}

// ���̾� ���� ���� ��������
bool		importLayerInfo(LayerNameSystem* layerInfo)
{
	FILE* fp;			// ���� ������
	char	line[20480];	// ���Ͽ��� �о�� ���� �ϳ�
	string	insElem;		// ��ū�� string���� ��ȯ�ؼ� vector�� ����
	char* token;			// �о�� ���ڿ��� ��ū
	short	lineCount;		// �о�� ���� ��
	short	tokCount;		// �о�� ��ū ����

	char	tempStr[128];

	// ���̾� ���� ���� ��������
	fp = fopen("C:\\layer.csv", "r");

	for (lineCount = 1; lineCount <= 14; ++lineCount) {
		if (fp != NULL) {
			tokCount = 0;
			fscanf(fp, "%s\n", line);
			token = strtok(line, ",");
			tokCount++;
			while (token != NULL) {
				if (strlen(token) > 0) {
					if (tokCount > 1) {
						insElem = token;

						if (lineCount == 1)		layerInfo->code_name.push_back(insElem);		// ���籸��
						if (lineCount == 2)		layerInfo->code_desc.push_back(insElem);		// ���籸�� ����
						if (lineCount == 3) {
							if (isStringDouble(token) == TRUE) {
								// ������ ���
								sprintf(tempStr, "%04d", atoi(token));
							}
							else {
								// ���ڿ��� ���
								strcpy(tempStr, token);
							}
							insElem = tempStr;

							layerInfo->dong_name.push_back(insElem);							// ��
						}
						if (lineCount == 4)		layerInfo->dong_desc.push_back(insElem);		// �� ����
						if (lineCount == 5)		layerInfo->floor_name.push_back(insElem);		// ��
						if (lineCount == 6)		layerInfo->floor_desc.push_back(insElem);		// �� ����
						if (lineCount == 7) {
							if (isStringDouble(token) == TRUE) {
								// ������ ���
								sprintf(tempStr, "%02d", atoi(token));
							}
							else {
								// ���ڿ��� ���
								strcpy(tempStr, token);
							}
							insElem = tempStr;

							layerInfo->cast_name.push_back(insElem);		// Ÿ����ȣ
						}
						if (lineCount == 8) {
							if (isStringDouble(token) == TRUE) {
								// ������ ���
								sprintf(tempStr, "%02d", atoi(token));
							}
							else {
								// ���ڿ��� ���
								strcpy(tempStr, token);
							}
							insElem = tempStr;

							layerInfo->CJ_name.push_back(insElem);			// CJ
						}
						if (lineCount == 9) {
							if (isStringDouble(token) == TRUE) {
								// ������ ���
								sprintf(tempStr, "%02d", atoi(token));
							}
							else {
								// ���ڿ��� ���
								strcpy(tempStr, token);
							}
							insElem = tempStr;

							layerInfo->orderInCJ_name.push_back(insElem);	// CJ �� �ð�����
						}
						if (lineCount == 10)	layerInfo->obj_name.push_back(insElem);			// ����
						if (lineCount == 11)	layerInfo->obj_desc.push_back(insElem);			// ���� ����
						if (lineCount == 12)	layerInfo->obj_cat.push_back(insElem);				// ���簡 ���� ī�װ���(���籸��)
						if (lineCount == 13)	layerInfo->productSite_name.push_back(insElem);	// ����ó ����
						if (lineCount == 14) {
							if (isStringDouble(token) == TRUE) {
								// ������ ���
								sprintf(tempStr, "%03d", atoi(token));
							}
							else {
								// ���ڿ��� ���
								strcpy(tempStr, token);
							}
							insElem = tempStr;

							layerInfo->productNum_name.push_back(insElem);	// ���� ��ȣ
						}
					}
				}
				token = strtok(NULL, ",");
				tokCount++;
			}
		}
		else {
			DGAlert(DG_ERROR, L"����", L"layer.csv ������ C:\\�� �����Ͻʽÿ�.", "", L"Ȯ��", "", "");
			fclose(fp);
			return	false;
		}
	}

	fclose(fp);
	return true;
}

// ���̾� ���� �����ϱ� (���̾� ���� �����ֱ�)
GSErrCode	showLayersEasily(void)
{
	GSErrCode	err = NoError;

	string	insElem;		// ��ū�� string���� ��ȯ�ؼ� vector�� ����
	char* token;			// �о�� ���ڿ��� ��ū

	API_Attribute	attrib;
	short			xx, yy, i;
	short			nLayers;

	short	nBaseFields;
	short	nExtendFields;
	bool	success;
	bool	extSuccess;
	char	tempStr[128];
	char	tok1[32];
	char	tok2[32];
	char	tok3[32];
	char	tok4[32];
	char	tok5[32];
	char	tok6[32];
	char	tok7[32];
	char	tok8[32];
	char	tok9[32];
	char	tok10[32];
	char	constructionCode[8];

	short	result;


	// ���̾� ���� ���� ��������
	importLayerInfo(&layerInfo);

	// ����ü �ʱ�ȭ
	allocateMemory(&layerInfo);
	selectedInfo = layerInfo;	// selectedInfo���� vector�� ��� �����Ƿ� �ʱ�ȭ�� ���� ������ ��
	allocateMemory(&selectedInfo);

	// ������Ʈ �� ���̾� ������ �˾Ƴ�
	nLayers = getLayerCount();

	for (xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.header.typeID = API_LayerID;
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			strcpy(tok1, "");
			strcpy(tok2, "");
			strcpy(tok3, "");
			strcpy(tok4, "");
			strcpy(tok5, "");
			strcpy(tok6, "");
			strcpy(tok7, "");
			strcpy(tok8, "");
			strcpy(tok9, "");
			i = 1;
			success = false;
			extSuccess = false;
			nBaseFields = 0;
			nExtendFields = 0;
			// ����(�⺻): 05-T-0000-F01-01-01-01-WALL
			// ����(Ȯ��): 05-T-0000-F01-01-01-01-WALL-��������-001
			// ���̾� �̸��� "-" ���� �������� �ɰ���
			token = strtok(attrib.layer.head.name, "-");
			while (token != NULL) {
				// ���� �� ���� Ȯ��
				// 1�� (�Ϸù�ȣ) - �ʼ� (2����, ����)
				if (i == 1) {
					strcpy(tempStr, token);
					if (strlen(tempStr) == 2) {
						strcpy(tok1, tempStr);
						success = true;
						nBaseFields++;
					}
					else {
						i = 100;
						success = false;
					}
				}
				// 2�� (���� ����) - �ʼ� (1����, ����)
				else if (i == 2) {
					strcpy(tempStr, token);
					if (strlen(tempStr) == 1) {
						strcpy(tok2, tempStr);
						success = true;
						nBaseFields++;
					}
					else {
						i = 100;
						success = false;
					}
				}
				// 3�� (�� ����) - �ʼ� (4����)
				else if (i == 3) {
					strcpy(tempStr, token);
					if (isStringDouble(tempStr) == TRUE) {
						// ������ ���
						sprintf(tok3, "%04d", atoi(tempStr));
					}
					else {
						// ���ڿ��� ���
						strcpy(tok3, tempStr);
					}
					success = true;
					nBaseFields++;
				}
				// 4�� (�� ����) - �ʼ� (3����)
				else if (i == 4) {
					strcpy(tempStr, token);
					if (strlen(tempStr) == 3) {
						strcpy(tok4, tempStr);
						success = true;
						nBaseFields++;
					}
					else {
						i = 100;
						success = false;
					}
				}
				// 5�� (Ÿ����ȣ) - �ʼ� (2����, ����)
				else if (i == 5) {
					strcpy(tempStr, token);
					if (isStringDouble(tempStr) == TRUE) {
						// ������ ���
						sprintf(tok5, "%02d", atoi(tempStr));
					}
					else {
						// ���ڿ��� ���
						strcpy(tok5, tempStr);
					}
					success = true;
					nBaseFields++;
				}
				// 6�� (CJ ����) - �ʼ� (2����, ����)
				else if (i == 6) {
					strcpy(tempStr, token);
					if (isStringDouble(tempStr) == TRUE) {
						// ������ ���
						sprintf(tok6, "%02d", atoi(tempStr));
					}
					else {
						// ���ڿ��� ���
						strcpy(tok6, tempStr);
					}
					success = true;
					nBaseFields++;
				}
				// 7�� (CJ �� �ð�����) - �ʼ� (2����, ����)
				else if (i == 7) {
					strcpy(tempStr, token);
					if (isStringDouble(tempStr) == TRUE) {
						// ������ ���
						sprintf(tok7, "%02d", atoi(tempStr));
					}
					else {
						// ���ڿ��� ���
						strcpy(tok7, tempStr);
					}
					success = true;
					nBaseFields++;
				}
				// 8�� (���� ����) - �ʼ� (3���� �̻�)
				else if (i == 8) {
					strcpy(tempStr, token);
					if (strlen(tempStr) >= 3) {
						strcpy(tok8, tempStr);
						success = true;
						nBaseFields++;
					}
					else {
						i = 100;
						success = false;
					}
				}
				// 9�� (����ó ����) - ���� (�ѱ� 4����..)
				else if (i == 9) {
					strcpy(tempStr, token);
					if (strlen(tempStr) >= 4) {
						strcpy(tok9, tempStr);
						extSuccess = true;
						nExtendFields++;
					}
					else {
						i = 100;
						extSuccess = false;
					}
				}
				// 10�� (���� ��ȣ) - �ʼ� (3����, ����)
				else if (i == 10) {
					strcpy(tempStr, token);
					if (isStringDouble(tempStr) == TRUE) {
						// ������ ���
						sprintf(tok10, "%03d", atoi(tempStr));
					}
					else {
						// ���ڿ��� ���
						strcpy(tok10, tempStr);
					}
					extSuccess = true;
					nExtendFields++;
				}
				++i;
				token = strtok(NULL, "-");
			}

			// 8�ܰ� �Ǵ� 10�ܰ���� ���������� �Ϸ�Ǹ� ����ü�� ����
			if ((success == true) && (nBaseFields == 8)) {
				// �Ϸ� ��ȣ�� ���� ���� ���ڸ� ���� ��ħ
				sprintf(constructionCode, "%s-%s", tok1, tok2);

				// 1,2�ܰ�. ���� ���� Ȯ��
				for (yy = 0; yy < layerInfo.code_name.size(); ++yy) {
					if (strncmp(constructionCode, layerInfo.code_name[yy].c_str(), 4) == 0) {
						layerInfo.code_state[yy] = true;
					}
					layerInfo.bCodeAllShow = true;
				}

				// 3�ܰ�. �� ����
				for (yy = 0; yy < layerInfo.dong_name.size(); ++yy) {
					if (strncmp(tok3, layerInfo.dong_name[yy].c_str(), 4) == 0) {
						layerInfo.dong_state[yy] = true;
					}
					layerInfo.bDongAllShow = true;
				}

				// 4�ܰ�. �� ����
				for (yy = 0; yy < layerInfo.floor_name.size(); ++yy) {
					if (strncmp(tok4, layerInfo.floor_name[yy].c_str(), 3) == 0) {
						layerInfo.floor_state[yy] = true;
					}
					layerInfo.bFloorAllShow = true;
				}

				// 5�ܰ�. Ÿ����ȣ
				for (yy = 0; yy < layerInfo.cast_name.size(); ++yy) {
					if (my_strcmp(tok5, layerInfo.cast_name[yy].c_str()) == 0) {
						layerInfo.cast_state[yy] = true;
					}
					layerInfo.bCastAllShow = true;
				}

				// 6�ܰ�. CJ ����
				for (yy = 0; yy < layerInfo.CJ_name.size(); ++yy) {
					if (my_strcmp(tok6, layerInfo.CJ_name[yy].c_str()) == 0) {
						layerInfo.CJ_state[yy] = true;
					}
					layerInfo.bCJAllShow = true;
				}

				// 7�ܰ�. CJ �� �ð�����
				for (yy = 0; yy < layerInfo.orderInCJ_name.size(); ++yy) {
					if (my_strcmp(tok7, layerInfo.orderInCJ_name[yy].c_str()) == 0) {
						layerInfo.orderInCJ_state[yy] = true;
					}
					layerInfo.bOrderInCJAllShow = true;
				}

				// 8�ܰ�. ���� ����
				for (yy = 0; yy < layerInfo.obj_name.size(); ++yy) {
					if ((strncmp(constructionCode, layerInfo.obj_cat[yy].c_str(), 4) == 0) && (strncmp(tok8, layerInfo.obj_name[yy].c_str(), 5) == 0)) {
						layerInfo.obj_state[yy] = true;
					}
				}

				if ((extSuccess == true) && (nExtendFields == 2)) {
					// 9�ܰ�. ����ó ����
					for (yy = 0; yy < layerInfo.productSite_name.size(); ++yy) {
						if (my_strcmp(tok9, layerInfo.productSite_name[yy].c_str()) == 0) {
							layerInfo.productSite_state[yy] = true;
						}
					}

					// 10�ܰ�. ���� ��ȣ
					for (yy = 0; yy < layerInfo.productNum_name.size(); ++yy) {
						if (strncmp(tok10, layerInfo.productNum_name[yy].c_str(), 3) == 0) {
							layerInfo.productNum_state[yy] = true;
						}
						layerInfo.bProductNumAllShow = true;
					}
				}
			}
		}
		if (err == APIERR_DELETED)
			err = NoError;
	}

	// [���̾�α� �ڽ�] ���̾� ���� �����ϱ�
	result = DGBlankModalDialog(1500, 600, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerShowHandler, 0);

	// OK ��ư�� �ƴϸ� �޸� �����ϰ� ����
	if (result != DG_OK) {
		deallocateMemory(&layerInfo);
		deallocateMemory(&selectedInfo);
		return	err;
	}

	// ��� ���̾� �����
	BNZeroMemory(&attrib, sizeof(API_Attribute));
	attrib.header.typeID = API_LayerID;

	for (xx = 1; xx <= nLayers; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			//if (!(attrib.layer.head.flags & APILay_Hidden == true)) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
			//}
		}
	}

	short	z;
	char	code1[25][32];		// ���� �ڵ�
	short	LenCode1;
	char	code2[1600][32];	// �� �ڵ�
	short	LenCode2;
	char	code3[120][32];	// �� �ڵ�
	short	LenCode3;
	char	code4[120][32];	// Ÿ����ȣ �ڵ�
	short	LenCode4;
	char	code5[120][32];	// CJ �ڵ�
	short	LenCode5;
	char	code6[120][32];	// CJ �� �ð����� �ڵ�
	short	LenCode6;
	char	code7[120][32];	// ���� �ڵ�
	short	LenCode7;
	char	code8[10][32];		// ����ó �ڵ�
	short	LenCode8;
	char	code9[1000][32];	// ���۹�ȣ �ڵ�
	short	LenCode9;

	char	fullLayerName[128];
	short	x1, x2, x3, x4, x5, x6, x7, x8, x9;

	// 1. ���� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
		if (selectedInfo.code_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.code_name[xx].c_str());
			strcpy(code1[z++], tempStr);
		}
	}
	LenCode1 = z;

	// 2. �� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
		if (selectedInfo.dong_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.dong_name[xx].c_str());
			strcpy(code2[z++], tempStr);
		}
	}
	LenCode2 = z;

	// 3. �� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
		if (selectedInfo.floor_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.floor_name[xx].c_str());
			strcpy(code3[z++], tempStr);
		}
	}
	LenCode3 = z;

	// 4. Ÿ����ȣ �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
		if (selectedInfo.cast_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.cast_name[xx].c_str());
			strcpy(code4[z++], tempStr);
		}
	}
	LenCode4 = z;

	// 5. CJ �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
		if (selectedInfo.CJ_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.CJ_name[xx].c_str());
			strcpy(code5[z++], tempStr);
		}
	}
	LenCode5 = z;

	// 6. CJ �� �ð����� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
		if (selectedInfo.orderInCJ_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.orderInCJ_name[xx].c_str());
			strcpy(code6[z++], tempStr);
		}
	}
	LenCode6 = z;

	// 7. ���� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
		if (selectedInfo.obj_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.obj_name[xx].c_str());
			strcpy(code7[z++], tempStr);
		}
	}
	LenCode7 = z;

	// 8. ����ó ���� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
		if (selectedInfo.productSite_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.productSite_name[xx].c_str());
			strcpy(code8[z++], tempStr);
		}
	}
	LenCode8 = z;

	// 9. ���� ��ȣ ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
		if (selectedInfo.productNum_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.productNum_name[xx].c_str());
			strcpy(code9[z++], tempStr);
		}
	}
	LenCode9 = z;

	// ���̾� �̸� �����ϱ�
	if (layerInfo.extendedLayer == true) {
		for (x1 = 0; x1 < LenCode1; ++x1) {
			for (x2 = 0; x2 < LenCode2; ++x2) {
				for (x3 = 0; x3 < LenCode3; ++x3) {
					for (x4 = 0; x4 < LenCode4; ++x4) {
						for (x5 = 0; x5 < LenCode5; ++x5) {
							for (x6 = 0; x6 < LenCode6; ++x6) {
								for (x7 = 0; x7 < LenCode7; ++x7) {
									for (x8 = 0; x8 < LenCode8; ++x8) {
										for (x9 = 0; x9 < LenCode9; ++x9) {

											// ���� ����
											strcpy(fullLayerName, "");
											strcpy(fullLayerName, code1[x1]);

											// �� ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code2[x2]);

											// �� ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code3[x3]);

											// Ÿ����ȣ
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code4[x4]);

											// CJ ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code5[x5]);

											// CJ �� �ð�����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code6[x6]);

											// ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code7[x7]);

											// ����ó ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code8[x8]);

											// ���� ��ȣ
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code9[x9]);

											// ������ ���̾� �̸� �˻��ϱ�
											BNZeroMemory(&attrib, sizeof(API_Attribute));
											attrib.header.typeID = API_LayerID;
											CHCopyC(fullLayerName, attrib.header.name);
											err = ACAPI_Attribute_Get(&attrib);

											// �ش� ���̾� �����ֱ�
											if ((attrib.layer.head.flags & APILay_Hidden) == true) {
												attrib.layer.head.flags ^= APILay_Hidden;
												ACAPI_Attribute_Modify(&attrib, NULL);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else {
		for (x1 = 0; x1 < LenCode1; ++x1) {
			for (x2 = 0; x2 < LenCode2; ++x2) {
				for (x3 = 0; x3 < LenCode3; ++x3) {
					for (x4 = 0; x4 < LenCode4; ++x4) {
						for (x5 = 0; x5 < LenCode5; ++x5) {
							for (x6 = 0; x6 < LenCode6; ++x6) {
								for (x7 = 0; x7 < LenCode7; ++x7) {

									// ���� ����
									strcpy(fullLayerName, "");
									strcpy(fullLayerName, code1[x1]);

									// �� ����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code2[x2]);

									// �� ����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code3[x3]);

									// Ÿ����ȣ
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code4[x4]);

									// CJ ����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code5[x5]);

									// CJ �� �ð�����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code6[x6]);

									// ����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code7[x7]);

									// ������ ���̾� �̸� �˻��ϱ�
									BNZeroMemory(&attrib, sizeof(API_Attribute));
									attrib.header.typeID = API_LayerID;
									CHCopyC(fullLayerName, attrib.header.name);
									err = ACAPI_Attribute_Get(&attrib);

									// �ش� ���̾� �����ֱ�
									if ((attrib.layer.head.flags & APILay_Hidden) == true) {
										attrib.layer.head.flags ^= APILay_Hidden;
										ACAPI_Attribute_Modify(&attrib, NULL);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// �޸� ����
	deallocateMemory(&layerInfo);
	deallocateMemory(&selectedInfo);

	return err;
}

// [���̾�α� �ڽ�] ���̾� ���� �����ϱ�
short DGCALLBACK layerShowHandler(short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy;
	char	tempStr[128];
	short	dialogSizeX, dialogSizeY;
	short	borderX = 1400;

	GSErrCode err = NoError;
	API_ModulData  info;

	// ���̾�α׿��� ����ڰ� ���� ������ �ڵ� ��ư�� �̸��� ������
	vector<string>	selected_code_name;
	vector<string>	selected_dong_name;
	vector<string>	selected_floor_name;
	vector<string>	selected_cast_name;
	vector<string>	selected_CJ_name;
	vector<string>	selected_orderInCJ_name;
	vector<string>	selected_obj_name;
	vector<string>	selected_productSite_name;
	vector<string>	selected_productNum_name;

	// ����ڰ� ������ �� �ִ� �ڵ� ������ ������
	vector<string>	visible_code_name;
	vector<string>	visible_dong_name;
	vector<string>	visible_floor_name;
	vector<string>	visible_cast_name;
	vector<string>	visible_CJ_name;
	vector<string>	visible_orderInCJ_name;
	vector<string>	visible_obj_name;
	vector<string>	visible_productSite_name;
	vector<string>	visible_productNum_name;

	short			nLayers;
	API_Attribute	attrib;

	string	layerName;
	string	extractedStr;
	vector<string>	visibleLayerList;
	char* token;
	char	layerNameStr[256];

	switch (message) {
	case DG_MSG_INIT:
		// ���̾�α� Ÿ��Ʋ
		DGSetDialogTitle(dialogID, L"���̾� ���� �����ϱ�");

		// Ȯ�� ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 20, 50, 25);
		DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_OK, L"Ȯ��");
		DGShowItem(dialogID, DG_OK);

		// ��� ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 50, 50, 25);
		DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_CANCEL, L"���");
		DGShowItem(dialogID, DG_CANCEL);

		// �ε� ��ư
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 80, 50, 25);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"�ε�");
		DGShowItem(dialogID, itmIdx);
		BUTTON_LOAD = itmIdx;

		// ���� ��ư
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 110, 50, 25);
		DGSetItemFont(dialogID, itmIdx, DG_IS_EXTRASMALL | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"����");
		DGShowItem(dialogID, itmIdx);
		BUTTON_FILTER = itmIdx;

		// ������ ��ư
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 140, 50, 25);
		DGSetItemFont(dialogID, itmIdx, DG_IS_EXTRASMALL | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"������");
		DGShowItem(dialogID, itmIdx);
		BUTTON_UNFILTER = itmIdx;

		// ��: �ڵ� �����ֱ�
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 0, 180, 50, 23);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"CODE");
		DGShowItem(dialogID, itmIdx);
		LABEL_CODE = itmIdx;

		// ��: ���� ����
		itmPosX = 50;
		itmPosY = 25;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"���� ����");
		DGShowItem(dialogID, itmIdx);

		// üũ�ڽ�: ���� ���� ��ư
		itmPosX = 150;
		itmPosY = 20;
		for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
			if (layerInfo.code_state[xx] == true) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				sprintf(tempStr, "%s %s", layerInfo.code_name[xx].c_str(), layerInfo.code_desc[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				layerInfo.code_idx[xx] = itmIdx;

				itmPosX += 100;
				if (itmPosX >= borderX) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
		}
		// ��� ����
		if (layerInfo.bCastAllShow) {
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"��� ����");
			DGShowItem(dialogID, itmIdx);
			SELECTALL_1_CONTYPE = itmIdx;
			itmPosX += 100;
			if (itmPosX >= borderX) {
				itmPosX = 150;
				itmPosY += 30;
			}
		}

		itmPosY += 30;

		// ��: �� ����
		itmPosX = 50;
		itmPosY += 10;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"�� ����");
		DGShowItem(dialogID, itmIdx);

		// üũ�ڽ�: �� ���� ��ư
		itmPosX = 150;
		itmPosY -= 5;
		for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
			if (layerInfo.dong_state[xx] == true) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText(dialogID, itmIdx, charToWchar(layerInfo.dong_desc[xx].c_str()));
				DGShowItem(dialogID, itmIdx);
				layerInfo.dong_idx[xx] = itmIdx;

				itmPosX += 100;
				if (itmPosX >= borderX) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
		}
		// ��� ����
		if (layerInfo.bDongAllShow) {
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"��� ����");
			DGShowItem(dialogID, itmIdx);
			SELECTALL_2_DONG = itmIdx;
			itmPosX += 100;
			if (itmPosX >= borderX) {
				itmPosX = 150;
				itmPosY += 30;
			}
		}

		itmPosY += 30;

		// ��: �� ����
		itmPosX = 50;
		itmPosY += 10;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"�� ����");
		DGShowItem(dialogID, itmIdx);

		// üũ�ڽ�: �� ���� ��ư
		itmPosX = 150;
		itmPosY -= 5;
		for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
			if (layerInfo.floor_state[xx] == true) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText(dialogID, itmIdx, charToWchar(layerInfo.floor_desc[xx].c_str()));
				DGShowItem(dialogID, itmIdx);
				layerInfo.floor_idx[xx] = itmIdx;

				itmPosX += 100;
				if (itmPosX >= borderX) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
		}
		// ��� ����
		if (layerInfo.bFloorAllShow) {
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"��� ����");
			DGShowItem(dialogID, itmIdx);
			SELECTALL_3_FLOOR = itmIdx;
			itmPosX += 100;
			if (itmPosX >= borderX) {
				itmPosX = 150;
				itmPosY += 30;
			}
		}

		itmPosY += 30;

		// ��: Ÿ����ȣ
		itmPosX = 50;
		itmPosY += 10;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"Ÿ����ȣ");
		DGShowItem(dialogID, itmIdx);

		// üũ�ڽ�: Ÿ����ȣ ��ư
		itmPosX = 150;
		itmPosY -= 5;
		for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
			if (layerInfo.cast_state[xx] == true) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText(dialogID, itmIdx, charToWchar(layerInfo.cast_name[xx].c_str()));
				DGShowItem(dialogID, itmIdx);
				layerInfo.cast_idx[xx] = itmIdx;

				itmPosX += 100;
				if (itmPosX >= borderX) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
		}
		// ��� ����
		if (layerInfo.bCastAllShow) {
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"��� ����");
			DGShowItem(dialogID, itmIdx);
			SELECTALL_4_CAST = itmIdx;
			itmPosX += 100;
			if (itmPosX >= borderX) {
				itmPosX = 150;
				itmPosY += 30;
			}
		}

		itmPosY += 30;

		// ��: CJ ����
		itmPosX = 50;
		itmPosY += 10;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"CJ ����");
		DGShowItem(dialogID, itmIdx);

		// üũ�ڽ�: CJ ���� ��ư
		itmPosX = 150;
		itmPosY -= 5;
		for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
			if (layerInfo.CJ_state[xx] == true) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText(dialogID, itmIdx, charToWchar(layerInfo.CJ_name[xx].c_str()));
				DGShowItem(dialogID, itmIdx);
				layerInfo.CJ_idx[xx] = itmIdx;

				itmPosX += 100;
				if (itmPosX >= borderX) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
		}
		// ��� ����
		if (layerInfo.bCJAllShow) {
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"��� ����");
			DGShowItem(dialogID, itmIdx);
			SELECTALL_5_CJ = itmIdx;
			itmPosX += 100;
			if (itmPosX >= borderX) {
				itmPosX = 150;
				itmPosY += 30;
			}
		}

		itmPosY += 30;

		// ��: CJ �� �ð�����
		itmPosX = 50;
		itmPosY += 10;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"�ð�����");
		DGShowItem(dialogID, itmIdx);

		// üũ�ڽ�: CJ �� �ð����� ��ư
		itmPosX = 150;
		itmPosY -= 5;
		for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
			if (layerInfo.orderInCJ_state[xx] == true) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText(dialogID, itmIdx, charToWchar(layerInfo.orderInCJ_name[xx].c_str()));
				DGShowItem(dialogID, itmIdx);
				layerInfo.orderInCJ_idx[xx] = itmIdx;

				itmPosX += 100;
				if (itmPosX >= borderX) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
		}
		// ��� ����
		if (layerInfo.bOrderInCJAllShow) {
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"��� ����");
			DGShowItem(dialogID, itmIdx);
			SELECTALL_6_ORDER = itmIdx;
			itmPosX += 100;
			if (itmPosX >= borderX) {
				itmPosX = 150;
				itmPosY += 30;
			}
		}

		itmPosY += 30;

		// ��: ����(����)
		itmPosX = 50;
		itmPosY += 10;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"*����");
		DGShowItem(dialogID, itmIdx);

		// üũ�ڽ�: ����(����)
		itmPosX = 150;
		itmPosY -= 5;
		for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
			if (strncmp(layerInfo.obj_cat[xx].c_str(), "01-S", 4) == 0) {
				if (layerInfo.obj_state[xx] == true) {
					itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText(dialogID, itmIdx, charToWchar(layerInfo.obj_desc[xx].c_str()));
					DGShowItem(dialogID, itmIdx);
					layerInfo.obj_idx[xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= borderX) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
		}

		itmPosY += 30;

		// ��: ����(���ึ��)
		itmPosX = 50;
		itmPosY += 10;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"*���ึ��");
		DGShowItem(dialogID, itmIdx);

		// üũ�ڽ�: ����(���ึ��)
		itmPosX = 150;
		itmPosY -= 5;
		for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
			if (strncmp(layerInfo.obj_cat[xx].c_str(), "02-A", 4) == 0) {
				if (layerInfo.obj_state[xx] == true) {
					itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText(dialogID, itmIdx, charToWchar(layerInfo.obj_desc[xx].c_str()));
					DGShowItem(dialogID, itmIdx);
					layerInfo.obj_idx[xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= borderX) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
		}

		itmPosY += 30;

		// ��: ����(���ü�)
		itmPosX = 50;
		itmPosY += 10;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"*���ü�");
		DGShowItem(dialogID, itmIdx);

		// üũ�ڽ�: ����(���ü�)
		itmPosX = 150;
		itmPosY -= 5;
		for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
			if (strncmp(layerInfo.obj_cat[xx].c_str(), "06-F", 4) == 0) {
				if (layerInfo.obj_state[xx] == true) {
					itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText(dialogID, itmIdx, charToWchar(layerInfo.obj_desc[xx].c_str()));
					DGShowItem(dialogID, itmIdx);
					layerInfo.obj_idx[xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= borderX) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
		}

		itmPosY += 30;

		// ��: ����
		itmPosX = 50;
		itmPosY += 10;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"*����");
		DGShowItem(dialogID, itmIdx);

		// üũ�ڽ�: ����
		itmPosX = 150;
		itmPosY -= 5;
		for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
			if (strncmp(layerInfo.obj_cat[xx].c_str(), "50-", 3) == 0) {
				if (layerInfo.obj_state[xx] == true) {
					itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText(dialogID, itmIdx, charToWchar(layerInfo.obj_desc[xx].c_str()));
					DGShowItem(dialogID, itmIdx);
					layerInfo.obj_idx[xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= borderX) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
		}

		itmPosY += 30;

		// ��: ����ó ����
		itmPosX = 50;
		itmPosY += 10;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"����ó ����");
		DGShowItem(dialogID, itmIdx);

		// üũ�ڽ�: ����ó ���� ��ư
		itmPosX = 150;
		itmPosY -= 5;
		for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
			if (layerInfo.productSite_state[xx] == true) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				sprintf(tempStr, "%s", layerInfo.productSite_name[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				layerInfo.productSite_idx[xx] = itmIdx;

				itmPosX += 100;
				if (itmPosX >= borderX) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
		}
		// ��� ����
		if (layerInfo.bProductSiteAllShow) {
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"��� ����");
			DGShowItem(dialogID, itmIdx);
			SELECTALL_8_PRODUCT_SITE = itmIdx;
			itmPosX += 100;
			if (itmPosX >= borderX) {
				itmPosX = 150;
				itmPosY += 30;
			}
		}

		itmPosY += 30;

		// ��: ���� ��ȣ
		itmPosX = 50;
		itmPosY += 10;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"���� ��ȣ");
		DGShowItem(dialogID, itmIdx);

		// üũ�ڽ�: ���� ��ȣ ��ư
		itmPosX = 150;
		itmPosY -= 5;
		for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
			if (layerInfo.productNum_state[xx] == true) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 45, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText(dialogID, itmIdx, charToWchar(layerInfo.productNum_name[xx].c_str()));
				DGShowItem(dialogID, itmIdx);
				layerInfo.productNum_idx[xx] = itmIdx;

				itmPosX += 50;
				if (itmPosX >= borderX) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
		}
		// ��� ����
		if (layerInfo.bProductNumAllShow) {
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"��� ����");
			DGShowItem(dialogID, itmIdx);
			SELECTALL_9_PRODUCT_NUM = itmIdx;
			itmPosX += 100;
			if (itmPosX >= borderX) {
				itmPosX = 150;
				itmPosY += 30;
			}
		}

		dialogSizeX = 1500;
		dialogSizeY = itmPosY + 150;
		DGSetDialogSize(dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

		break;

	case DG_MSG_CHANGE:
		changedBtnItemIdx = item;

		if (changedBtnItemIdx == SELECTALL_1_CONTYPE) {
			if (DGGetItemValLong(dialogID, SELECTALL_1_CONTYPE) == TRUE) {
				// ��� ����
				for (xx = 0; xx < layerInfo.code_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.code_idx[xx], TRUE);
			}
			else {
				// ��� ����
				for (xx = 0; xx < layerInfo.code_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.code_idx[xx], FALSE);
			}
		}
		if (changedBtnItemIdx == SELECTALL_2_DONG) {
			if (DGGetItemValLong(dialogID, SELECTALL_2_DONG) == TRUE) {
				// ��� ����
				for (xx = 0; xx < layerInfo.dong_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.dong_idx[xx], TRUE);
			}
			else {
				// ��� ����
				for (xx = 0; xx < layerInfo.dong_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.dong_idx[xx], FALSE);
			}
		}
		if (changedBtnItemIdx == SELECTALL_3_FLOOR) {
			if (DGGetItemValLong(dialogID, SELECTALL_3_FLOOR) == TRUE) {
				// ��� ����
				for (xx = 0; xx < layerInfo.floor_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.floor_idx[xx], TRUE);
			}
			else {
				// ��� ����
				for (xx = 0; xx < layerInfo.floor_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.floor_idx[xx], FALSE);
			}
		}
		if (changedBtnItemIdx == SELECTALL_4_CAST) {
			if (DGGetItemValLong(dialogID, SELECTALL_4_CAST) == TRUE) {
				// ��� ����
				for (xx = 0; xx < layerInfo.cast_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.cast_idx[xx], TRUE);
			}
			else {
				// ��� ����
				for (xx = 0; xx < layerInfo.cast_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.cast_idx[xx], FALSE);
			}
		}
		if (changedBtnItemIdx == SELECTALL_5_CJ) {
			if (DGGetItemValLong(dialogID, SELECTALL_5_CJ) == TRUE) {
				// ��� ����
				for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.CJ_idx[xx], TRUE);
			}
			else {
				// ��� ����
				for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.CJ_idx[xx], FALSE);
			}
		}
		if (changedBtnItemIdx == SELECTALL_6_ORDER) {
			if (DGGetItemValLong(dialogID, SELECTALL_6_ORDER) == TRUE) {
				// ��� ����
				for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.orderInCJ_idx[xx], TRUE);
			}
			else {
				// ��� ����
				for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.orderInCJ_idx[xx], FALSE);
			}
		}
		if (changedBtnItemIdx == SELECTALL_8_PRODUCT_SITE) {
			if (DGGetItemValLong(dialogID, SELECTALL_8_PRODUCT_SITE) == TRUE) {
				// ��� ����
				for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.productSite_idx[xx], TRUE);
			}
			else {
				// ��� ����
				for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.productSite_idx[xx], FALSE);
			}
		}
		if (changedBtnItemIdx == SELECTALL_9_PRODUCT_NUM) {
			if (DGGetItemValLong(dialogID, SELECTALL_9_PRODUCT_NUM) == TRUE) {
				// ��� ����
				for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.productNum_idx[xx], TRUE);
			}
			else {
				// ��� ����
				for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx)
					DGSetItemValLong(dialogID, layerInfo.productNum_idx[xx], FALSE);
			}
		}

		// ��ư�� �̸� �����ֱ�
		for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
			if ((layerInfo.code_idx[xx] == changedBtnItemIdx) && (layerInfo.code_state[xx] == true)) {
				sprintf(tempStr, "%s", layerInfo.code_name[xx].c_str());
				DGSetItemText(dialogID, LABEL_CODE, charToWchar(tempStr));
			}
		}
		for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
			if ((layerInfo.dong_idx[xx] == changedBtnItemIdx) && (layerInfo.dong_state[xx] == true)) {
				sprintf(tempStr, "%s", layerInfo.dong_name[xx].c_str());
				DGSetItemText(dialogID, LABEL_CODE, charToWchar(tempStr));
			}
		}
		for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
			if ((layerInfo.floor_idx[xx] == changedBtnItemIdx) && (layerInfo.floor_state[xx] == true)) {
				sprintf(tempStr, "%s", layerInfo.floor_name[xx].c_str());
				DGSetItemText(dialogID, LABEL_CODE, charToWchar(tempStr));
			}
		}
		for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
			if ((layerInfo.cast_idx[xx] == changedBtnItemIdx) && (layerInfo.cast_state[xx] == true)) {
				sprintf(tempStr, "%s", layerInfo.cast_name[xx].c_str());
				DGSetItemText(dialogID, LABEL_CODE, charToWchar(tempStr));
			}
		}
		for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
			if ((layerInfo.CJ_idx[xx] == changedBtnItemIdx) && (layerInfo.CJ_state[xx] == true)) {
				sprintf(tempStr, "%s", layerInfo.CJ_name[xx].c_str());
				DGSetItemText(dialogID, LABEL_CODE, charToWchar(tempStr));
			}
		}
		for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
			if ((layerInfo.orderInCJ_idx[xx] == changedBtnItemIdx) && (layerInfo.orderInCJ_state[xx] == true)) {
				sprintf(tempStr, "%s", layerInfo.orderInCJ_name[xx].c_str());
				DGSetItemText(dialogID, LABEL_CODE, charToWchar(tempStr));
			}
		}
		for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
			if ((layerInfo.obj_idx[xx] == changedBtnItemIdx) && (layerInfo.obj_state[xx] == true)) {
				sprintf(tempStr, "%s", layerInfo.obj_name[xx].c_str());
				DGSetItemText(dialogID, LABEL_CODE, charToWchar(tempStr));
			}
		}
		for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
			if ((layerInfo.productSite_idx[xx] == changedBtnItemIdx) && (layerInfo.productSite_state[xx] == true)) {
				sprintf(tempStr, "%s", layerInfo.productSite_name[xx].c_str());
				DGSetItemText(dialogID, LABEL_CODE, charToWchar(tempStr));
			}
		}
		for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
			if ((layerInfo.productNum_idx[xx] == changedBtnItemIdx) && (layerInfo.productNum_state[xx] == true)) {
				sprintf(tempStr, "%s", layerInfo.productNum_name[xx].c_str());
				DGSetItemText(dialogID, LABEL_CODE, charToWchar(tempStr));
			}
		}

		break;

	case DG_MSG_CLICK:
		switch (item) {
		case DG_OK:
			// ���� ����
			for (xx = 0; xx < layerInfo.code_name.size(); ++xx)
				(DGGetItemValLong(dialogID, layerInfo.code_idx[xx]) == TRUE) ? selectedInfo.code_state[xx] = true : selectedInfo.code_state[xx] = false;

			// �� ����
			for (xx = 0; xx < layerInfo.dong_name.size(); ++xx)
				(DGGetItemValLong(dialogID, layerInfo.dong_idx[xx]) == TRUE) ? selectedInfo.dong_state[xx] = true : selectedInfo.dong_state[xx] = false;

			// �� ����
			for (xx = 0; xx < layerInfo.floor_name.size(); ++xx)
				(DGGetItemValLong(dialogID, layerInfo.floor_idx[xx]) == TRUE) ? selectedInfo.floor_state[xx] = true : selectedInfo.floor_state[xx] = false;

			// Ÿ����ȣ
			for (xx = 0; xx < layerInfo.cast_name.size(); ++xx)
				(DGGetItemValLong(dialogID, layerInfo.cast_idx[xx]) == TRUE) ? selectedInfo.cast_state[xx] = true : selectedInfo.cast_state[xx] = false;

			// CJ
			for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx)
				(DGGetItemValLong(dialogID, layerInfo.CJ_idx[xx]) == TRUE) ? selectedInfo.CJ_state[xx] = true : selectedInfo.CJ_state[xx] = false;

			// CJ �� �ð�����
			for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx)
				(DGGetItemValLong(dialogID, layerInfo.orderInCJ_idx[xx]) == TRUE) ? selectedInfo.orderInCJ_state[xx] = true : selectedInfo.orderInCJ_state[xx] = false;

			// ����
			for (xx = 0; xx < layerInfo.obj_name.size(); ++xx)
				(DGGetItemValLong(dialogID, layerInfo.obj_idx[xx]) == TRUE) ? selectedInfo.obj_state[xx] = true : selectedInfo.obj_state[xx] = false;

			layerInfo.extendedLayer = false;

			// ����ó ����
			for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx)
				if (DGGetItemValLong(dialogID, layerInfo.productSite_idx[xx]) == TRUE) {
					selectedInfo.productSite_state[xx] = true;
					layerInfo.extendedLayer = true;
				}
				else {
					selectedInfo.productSite_state[xx] = false;
				}

			// ���� ��ȣ
			for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx)
				if (DGGetItemValLong(dialogID, layerInfo.productNum_idx[xx]) == TRUE) {
					selectedInfo.productNum_state[xx] = true;
					layerInfo.extendedLayer = true;
				}
				else {
					selectedInfo.productNum_state[xx] = false;
				}

			// ��ư ���� ����
			saveButtonStatus_show();

			break;

		case DG_CANCEL:
			break;

		default:
			clickedBtnItemIdx = item;
			item = 0;

			// ����� ��ư ���¸� �ҷ���
			if (clickedBtnItemIdx == BUTTON_LOAD) {
				BNZeroMemory(&info, sizeof(API_ModulData));
				err = ACAPI_ModulData_Get(&info, "ButtonStatus_show");

				if (err == NoError && info.dataVersion == 1) {
					selectedInfoSaved = *(reinterpret_cast<StatusOfLayerNameSystem*> (*info.dataHdl));

					for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
						if (selectedInfoSaved.code_state[xx] == true)
							DGSetItemValLong(dialogID, layerInfo.code_idx[xx], TRUE);
					}
					for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
						if (selectedInfoSaved.dong_state[xx] == true)
							DGSetItemValLong(dialogID, layerInfo.dong_idx[xx], TRUE);
					}
					for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
						if (selectedInfoSaved.floor_state[xx] == true)
							DGSetItemValLong(dialogID, layerInfo.floor_idx[xx], TRUE);
					}
					for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
						if (selectedInfoSaved.cast_state[xx] == true)
							DGSetItemValLong(dialogID, layerInfo.cast_idx[xx], TRUE);
					}
					for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
						if (selectedInfoSaved.CJ_state[xx] == true)
							DGSetItemValLong(dialogID, layerInfo.CJ_idx[xx], TRUE);
					}
					for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
						if (selectedInfoSaved.orderInCJ_state[xx] == true)
							DGSetItemValLong(dialogID, layerInfo.orderInCJ_idx[xx], TRUE);
					}
					for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
						if (selectedInfoSaved.obj_state[xx] == true)
							DGSetItemValLong(dialogID, layerInfo.obj_idx[xx], TRUE);
					}
					for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
						if (selectedInfoSaved.productSite_state[xx] == true)
							DGSetItemValLong(dialogID, layerInfo.productSite_idx[xx], TRUE);
					}
					for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
						if (selectedInfoSaved.productNum_state[xx] == true)
							DGSetItemValLong(dialogID, layerInfo.productNum_idx[xx], TRUE);
					}
				}

				BMKillHandle(&info.dataHdl);
			}

			// Ŭ���� ��ư�� ���� ���, ������ �ڵ�� ���þ��� ��ư�� ��Ȱ��ȭ
			if (clickedBtnItemIdx == BUTTON_FILTER) {
				selected_code_name.clear();
				selected_dong_name.clear();
				selected_floor_name.clear();
				selected_cast_name.clear();
				selected_CJ_name.clear();
				selected_orderInCJ_name.clear();
				selected_obj_name.clear();
				selected_productSite_name.clear();
				selected_productNum_name.clear();

				// ������ �ڵ带 ������
				for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, layerInfo.code_idx[xx]) == TRUE)
						selected_code_name.push_back(layerInfo.code_name[xx].c_str());
				}
				for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, layerInfo.dong_idx[xx]) == TRUE)
						selected_dong_name.push_back(layerInfo.dong_name[xx].c_str());
				}
				for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, layerInfo.floor_idx[xx]) == TRUE)
						selected_floor_name.push_back(layerInfo.floor_name[xx].c_str());
				}
				for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, layerInfo.cast_idx[xx]) == TRUE)
						selected_cast_name.push_back(layerInfo.cast_name[xx].c_str());
				}
				for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, layerInfo.CJ_idx[xx]) == TRUE)
						selected_CJ_name.push_back(layerInfo.CJ_name[xx].c_str());
				}
				for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, layerInfo.orderInCJ_idx[xx]) == TRUE)
						selected_orderInCJ_name.push_back(layerInfo.orderInCJ_name[xx].c_str());
				}
				for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, layerInfo.obj_idx[xx]) == TRUE)
						selected_obj_name.push_back(layerInfo.obj_name[xx].c_str());
				}
				for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, layerInfo.productSite_idx[xx]) == TRUE)
						selected_productSite_name.push_back(layerInfo.productSite_name[xx].c_str());
				}
				for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, layerInfo.productNum_idx[xx]) == TRUE)
						selected_productNum_name.push_back(layerInfo.productNum_name[xx].c_str());
				}

				// �ڵ� ��ư ��ü ��Ȱ��ȭ
				for (xx = 0; xx < layerInfo.code_name.size(); ++xx)			if (layerInfo.code_state[xx] == true)			DGDisableItem(dialogID, layerInfo.code_idx[xx]);
				for (xx = 0; xx < layerInfo.dong_name.size(); ++xx)			if (layerInfo.dong_state[xx] == true)			DGDisableItem(dialogID, layerInfo.dong_idx[xx]);
				for (xx = 0; xx < layerInfo.floor_name.size(); ++xx)			if (layerInfo.floor_state[xx] == true)			DGDisableItem(dialogID, layerInfo.floor_idx[xx]);
				for (xx = 0; xx < layerInfo.cast_name.size(); ++xx)			if (layerInfo.cast_state[xx] == true)			DGDisableItem(dialogID, layerInfo.cast_idx[xx]);
				for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx)			if (layerInfo.CJ_state[xx] == true)			DGDisableItem(dialogID, layerInfo.CJ_idx[xx]);
				for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx)		if (layerInfo.orderInCJ_state[xx] == true)		DGDisableItem(dialogID, layerInfo.orderInCJ_idx[xx]);
				for (xx = 0; xx < layerInfo.obj_name.size(); ++xx)			if (layerInfo.obj_state[xx] == true)			DGDisableItem(dialogID, layerInfo.obj_idx[xx]);
				for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx)	if (layerInfo.productSite_state[xx] == true)	DGDisableItem(dialogID, layerInfo.productSite_idx[xx]);
				for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx)	if (layerInfo.productNum_state[xx] == true)	DGDisableItem(dialogID, layerInfo.productNum_idx[xx]);

				// ������ �ڵ忡 ���ؼ��� ��ư�� Ȱ��ȭ��
				visibleLayerList.clear();
				nLayers = getLayerCount();

				for (xx = 1; xx <= nLayers; ++xx) {
					// ���� ���̾� �̸����� ������ �ڵ� ��ư�� ���Ե� ���̾� �̸��� ������
					BNZeroMemory(&attrib, sizeof(API_Attribute));
					attrib.header.typeID = API_LayerID;
					attrib.header.index = xx;
					err = ACAPI_Attribute_Get(&attrib);
					if (err == NoError) {
						layerName = attrib.layer.head.name;

						// ���� ���� �ڵ� ��
						token = strtok(attrib.layer.head.name, "-");
						if (token == NULL)	continue;
						extractedStr = token;
						strcpy(tempStr, extractedStr.c_str());
						token = strtok(NULL, "-");
						if (token == NULL)	continue;
						extractedStr = token;
						strcat(tempStr, "-");
						strcat(tempStr, extractedStr.c_str());
						extractedStr = tempStr;
						for (yy = 0; yy < selected_code_name.size(); ++yy)		if (extractedStr.compare(selected_code_name[yy]) == 0)		visibleLayerList.push_back(layerName);

						// �� ���� �ڵ� ��
						token = strtok(NULL, "-");
						if (token == NULL)	continue;
						extractedStr = token;
						for (yy = 0; yy < selected_dong_name.size(); ++yy)		if (extractedStr.compare(selected_dong_name[yy]) == 0)		visibleLayerList.push_back(layerName);

						// �� ���� �ڵ� ��
						token = strtok(NULL, "-");
						if (token == NULL)	continue;
						extractedStr = token;
						for (yy = 0; yy < selected_floor_name.size(); ++yy)		if (extractedStr.compare(selected_floor_name[yy]) == 0)		visibleLayerList.push_back(layerName);

						// Ÿ����ȣ �ڵ� ��
						token = strtok(NULL, "-");
						if (token == NULL)	continue;
						extractedStr = token;
						for (yy = 0; yy < selected_cast_name.size(); ++yy)		if (extractedStr.compare(selected_cast_name[yy]) == 0)		visibleLayerList.push_back(layerName);

						// CJ ���� �ڵ� ��
						token = strtok(NULL, "-");
						if (token == NULL)	continue;
						extractedStr = token;
						for (yy = 0; yy < selected_CJ_name.size(); ++yy)			if (extractedStr.compare(selected_CJ_name[yy]) == 0)			visibleLayerList.push_back(layerName);

						// �ð����� �ڵ� ��
						token = strtok(NULL, "-");
						if (token == NULL)	continue;
						extractedStr = token;
						for (yy = 0; yy < selected_orderInCJ_name.size(); ++yy)	if (extractedStr.compare(selected_orderInCJ_name[yy]) == 0)	visibleLayerList.push_back(layerName);

						// ���� �ڵ� ��
						token = strtok(NULL, "-");
						if (token == NULL)	continue;
						extractedStr = token;
						for (yy = 0; yy < selected_obj_name.size(); ++yy)		if (extractedStr.compare(selected_obj_name[yy]) == 0)			visibleLayerList.push_back(layerName);

						// ����ó ���� �ڵ� ��
						token = strtok(NULL, "-");
						if (token == NULL)	continue;
						extractedStr = token;
						for (yy = 0; yy < selected_productSite_name.size(); ++yy)	if (extractedStr.compare(selected_productSite_name[yy]) == 0)		visibleLayerList.push_back(layerName);

						// ���� ��ȣ �ڵ� ��
						token = strtok(NULL, "-");
						if (token == NULL)	continue;
						extractedStr = token;
						for (yy = 0; yy < selected_productNum_name.size(); ++yy)		if (extractedStr.compare(selected_productNum_name[yy]) == 0)		visibleLayerList.push_back(layerName);
					}
				}

				// ���ο� ���̾� ��Ͽ��� �ߺ��� �׸� ����
				visibleLayerList.erase(unique(visibleLayerList.begin(), visibleLayerList.end()), visibleLayerList.end());

				// ���ο� ���̾� ����� ���� ���� ������ �ڵ带 ������
				for (xx = 0; xx < visibleLayerList.size(); ++xx) {
					sprintf(layerNameStr, "%s", visibleLayerList[xx].c_str());
					layerName = visibleLayerList[xx];

					// ���� ���� �ڵ� ����
					token = strtok(layerNameStr, "-");
					if (token == NULL)	continue;
					sprintf(tempStr, "%s", token);
					token = strtok(NULL, "-");
					if (token == NULL)	continue;
					strcat(tempStr, "-");
					strcat(tempStr, token);
					visible_code_name.push_back(tempStr);

					// �� ���� �ڵ� ����
					token = strtok(NULL, "-");
					if (token == NULL)	continue;
					visible_dong_name.push_back(token);

					// �� ���� �ڵ� ����
					token = strtok(NULL, "-");
					if (token == NULL)	continue;
					visible_floor_name.push_back(token);

					// Ÿ����ȣ �ڵ� ����
					token = strtok(NULL, "-");
					if (token == NULL)	continue;
					visible_cast_name.push_back(token);

					// CJ ���� �ڵ� ����
					token = strtok(NULL, "-");
					if (token == NULL)	continue;
					visible_CJ_name.push_back(token);

					// �ð����� �ڵ� ����
					token = strtok(NULL, "-");
					if (token == NULL)	continue;
					visible_orderInCJ_name.push_back(token);

					// ���� �ڵ� ����
					token = strtok(NULL, "-");
					if (token == NULL)	continue;
					visible_obj_name.push_back(token);

					// ����ó ���� �ڵ� ����
					token = strtok(NULL, "-");
					if (token == NULL)	continue;
					visible_productSite_name.push_back(token);

					// ���� ��ȣ �ڵ� ����
					token = strtok(NULL, "-");
					if (token == NULL)	continue;
					visible_productNum_name.push_back(token);
				}

				// ����� �ڵ� ���ڿ��� �ߺ��� ������
				visible_code_name.erase(unique(visible_code_name.begin(), visible_code_name.end()), visible_code_name.end());
				visible_dong_name.erase(unique(visible_dong_name.begin(), visible_dong_name.end()), visible_dong_name.end());
				visible_floor_name.erase(unique(visible_floor_name.begin(), visible_floor_name.end()), visible_floor_name.end());
				visible_cast_name.erase(unique(visible_cast_name.begin(), visible_cast_name.end()), visible_cast_name.end());
				visible_CJ_name.erase(unique(visible_CJ_name.begin(), visible_CJ_name.end()), visible_CJ_name.end());
				visible_orderInCJ_name.erase(unique(visible_orderInCJ_name.begin(), visible_orderInCJ_name.end()), visible_orderInCJ_name.end());
				visible_obj_name.erase(unique(visible_obj_name.begin(), visible_obj_name.end()), visible_obj_name.end());
				visible_productSite_name.erase(unique(visible_productSite_name.begin(), visible_productSite_name.end()), visible_productSite_name.end());
				visible_productNum_name.erase(unique(visible_productNum_name.begin(), visible_productNum_name.end()), visible_productNum_name.end());

				// ���� ���� �ڵ� ��ư Ȱ��ȭ
				for (xx = 0; xx < layerInfo.code_name.size(); ++xx)
					for (yy = 0; yy < visible_code_name.size(); ++yy)
						if (layerInfo.code_name[xx].compare(visible_code_name[yy]) == 0)
							DGEnableItem(dialogID, layerInfo.code_idx[xx]);

				// �� ���� �ڵ� ��ư Ȱ��ȭ
				for (xx = 0; xx < layerInfo.dong_name.size(); ++xx)
					for (yy = 0; yy < visible_dong_name.size(); ++yy)
						if (layerInfo.dong_name[xx].compare(visible_dong_name[yy]) == 0)
							DGEnableItem(dialogID, layerInfo.dong_idx[xx]);

				// �� ���� �ڵ� ��ư Ȱ��ȭ
				for (xx = 0; xx < layerInfo.floor_name.size(); ++xx)
					for (yy = 0; yy < visible_floor_name.size(); ++yy)
						if (layerInfo.floor_name[xx].compare(visible_floor_name[yy]) == 0)
							DGEnableItem(dialogID, layerInfo.floor_idx[xx]);

				// Ÿ����ȣ �ڵ� ��ư Ȱ��ȭ
				for (xx = 0; xx < layerInfo.cast_name.size(); ++xx)
					for (yy = 0; yy < visible_cast_name.size(); ++yy)
						if (layerInfo.cast_name[xx].compare(visible_cast_name[yy]) == 0)
							DGEnableItem(dialogID, layerInfo.cast_idx[xx]);

				// CJ ���� �ڵ� ��ư Ȱ��ȭ
				for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx)
					for (yy = 0; yy < visible_CJ_name.size(); ++yy)
						if (layerInfo.CJ_name[xx].compare(visible_CJ_name[yy]) == 0)
							DGEnableItem(dialogID, layerInfo.CJ_idx[xx]);

				// �ð����� �ڵ� ��ư Ȱ��ȭ
				for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx)
					for (yy = 0; yy < visible_orderInCJ_name.size(); ++yy)
						if (layerInfo.orderInCJ_name[xx].compare(visible_orderInCJ_name[yy]) == 0)
							DGEnableItem(dialogID, layerInfo.orderInCJ_idx[xx]);

				// ���� �ڵ� ��ư Ȱ��ȭ
				for (xx = 0; xx < layerInfo.obj_name.size(); ++xx)
					for (yy = 0; yy < visible_obj_name.size(); ++yy)
						if (layerInfo.obj_name[xx].compare(visible_obj_name[yy]) == 0)
							DGEnableItem(dialogID, layerInfo.obj_idx[xx]);

				// ����ó ���� �ڵ� ��ư Ȱ��ȭ
				for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx)
					for (yy = 0; yy < visible_productSite_name.size(); ++yy)
						if (layerInfo.productSite_name[xx].compare(visible_productSite_name[yy]) == 0)
							DGEnableItem(dialogID, layerInfo.productSite_idx[xx]);

				// ���� ��ȣ �ڵ� ��ư Ȱ��ȭ
				for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx)
					for (yy = 0; yy < visible_productNum_name.size(); ++yy)
						if (layerInfo.productNum_name[xx].compare(visible_productNum_name[yy]) == 0)
							DGEnableItem(dialogID, layerInfo.productNum_idx[xx]);
			}

			// ��� ��ư�� �ٽ� ���� �� �ִ� ���·� ������Ŵ
			if (clickedBtnItemIdx == BUTTON_UNFILTER) {
				// �ڵ� ��ư ��ü Ȱ��ȭ
				for (xx = 0; xx < layerInfo.code_name.size(); ++xx)			if (layerInfo.code_state[xx] == true)			DGEnableItem(dialogID, layerInfo.code_idx[xx]);
				for (xx = 0; xx < layerInfo.dong_name.size(); ++xx)			if (layerInfo.dong_state[xx] == true)			DGEnableItem(dialogID, layerInfo.dong_idx[xx]);
				for (xx = 0; xx < layerInfo.floor_name.size(); ++xx)			if (layerInfo.floor_state[xx] == true)			DGEnableItem(dialogID, layerInfo.floor_idx[xx]);
				for (xx = 0; xx < layerInfo.cast_name.size(); ++xx)			if (layerInfo.cast_state[xx] == true)			DGEnableItem(dialogID, layerInfo.cast_idx[xx]);
				for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx)			if (layerInfo.CJ_state[xx] == true)			DGEnableItem(dialogID, layerInfo.CJ_idx[xx]);
				for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx)		if (layerInfo.orderInCJ_state[xx] == true)		DGEnableItem(dialogID, layerInfo.orderInCJ_idx[xx]);
				for (xx = 0; xx < layerInfo.obj_name.size(); ++xx)			if (layerInfo.obj_state[xx] == true)			DGEnableItem(dialogID, layerInfo.obj_idx[xx]);
				for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx)	if (layerInfo.productSite_state[xx] == true)	DGEnableItem(dialogID, layerInfo.productSite_idx[xx]);
				for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx)	if (layerInfo.productNum_state[xx] == true)	DGEnableItem(dialogID, layerInfo.productNum_idx[xx]);
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

// �ֱ� ��ư ���� �����ϱ� (���̾� ���� �����ֱ�)
GSErrCode	saveButtonStatus_show(void)
{
	GSErrCode err = NoError;

	short	xx;
	API_ModulData	info;
	BNZeroMemory(&info, sizeof(API_ModulData));
	info.dataVersion = 1;
	info.platformSign = GS::Act_Platform_Sign;
	info.dataHdl = BMAllocateHandle(sizeof(selectedInfoSaved), 0, 0);
	if (info.dataHdl != NULL) {

		for (xx = 0; xx < layerInfo.code_name.size(); ++xx)
			selectedInfoSaved.code_state[xx] = selectedInfo.code_state[xx];
		for (xx = 0; xx < layerInfo.dong_name.size(); ++xx)
			selectedInfoSaved.dong_state[xx] = selectedInfo.dong_state[xx];
		for (xx = 0; xx < layerInfo.floor_name.size(); ++xx)
			selectedInfoSaved.floor_state[xx] = selectedInfo.floor_state[xx];
		for (xx = 0; xx < layerInfo.cast_name.size(); ++xx)
			selectedInfoSaved.cast_state[xx] = selectedInfo.cast_state[xx];
		for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx)
			selectedInfoSaved.CJ_state[xx] = selectedInfo.CJ_state[xx];
		for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx)
			selectedInfoSaved.orderInCJ_state[xx] = selectedInfo.orderInCJ_state[xx];
		for (xx = 0; xx < layerInfo.obj_name.size(); ++xx)
			selectedInfoSaved.obj_state[xx] = selectedInfo.obj_state[xx];
		for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx)
			selectedInfoSaved.productSite_state[xx] = selectedInfo.productSite_state[xx];
		for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx)
			selectedInfoSaved.productNum_state[xx] = selectedInfo.productNum_state[xx];

		*(reinterpret_cast<StatusOfLayerNameSystem*> (*info.dataHdl)) = selectedInfoSaved;
		err = ACAPI_ModulData_Store(&info, "ButtonStatus_show");
		BMKillHandle(&info.dataHdl);
	}
	else {
		err = APIERR_MEMFULL;
	}

	return	err;
}

// ���̾� ���� �����
GSErrCode	makeLayersEasily(void)
{
	GSErrCode	err = NoError;

	string	insElem;		// ��ū�� string���� ��ȯ�ؼ� vector�� ����

	API_Attribute	attrib;
	API_AttributeDef  defs;
	short			xx;

	char	tempStr[128];

	short	result;


	// ���̾� ���� ���� ��������
	importLayerInfo(&layerInfo);

	// ����ü �ʱ�ȭ
	allocateMemory(&layerInfo);

	// [���̾�α� �ڽ�] ���̾� ���� �����
	result = DGBlankModalDialog(1100, 120, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerMakeHandler, 0);

	// OK ��ư�� ������ �ʾҰų� ���̾� �ڵ� �ʵ尡 �ϼ��Ǿ� ���� ������ �޸𸮸� �����ϰ� ����
	if ((result != DG_OK) || (isFullLayer(&layerInfo) == false)) {
		deallocateMemory(&layerInfo);
		return	err;
	}

	short	z;
	char	code1[25][32];		// ���� �ڵ�
	short	LenCode1;
	char	code2[1600][32];	// �� �ڵ�
	short	LenCode2;
	char	code3[120][32];	// �� �ڵ�
	short	LenCode3;
	char	code4[120][32];	// Ÿ����ȣ �ڵ�
	short	LenCode4;
	char	code5[120][32];	// CJ �ڵ�
	short	LenCode5;
	char	code6[120][32];	// CJ �� �ð����� �ڵ�
	short	LenCode6;
	char	code7[120][32];	// ���� �ڵ�
	short	LenCode7;
	char	code8[10][32];		// ����ó �ڵ�
	short	LenCode8;
	char	code9[1000][32];	// ���۹�ȣ �ڵ�
	short	LenCode9;

	char	fullLayerName[128];
	short	madeLayers;
	bool	bNormalLayer;
	short	x1, x2, x3, x4, x5, x6, x7, x8, x9;

	// 1. ���� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
		if (layerInfo.code_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.code_name[xx].c_str());
			strcpy(code1[z++], tempStr);
		}
	}
	LenCode1 = z;

	// 2. �� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
		if (layerInfo.dong_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.dong_name[xx].c_str());
			strcpy(code2[z++], tempStr);
		}
	}
	LenCode2 = z;

	// 3. �� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
		if (layerInfo.floor_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.floor_name[xx].c_str());
			strcpy(code3[z++], tempStr);
		}
	}
	LenCode3 = z;

	// 4. Ÿ����ȣ �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
		if (layerInfo.cast_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.cast_name[xx].c_str());
			strcpy(code4[z++], tempStr);
		}
	}
	LenCode4 = z;

	// 5. CJ �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
		if (layerInfo.CJ_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.CJ_name[xx].c_str());
			strcpy(code5[z++], tempStr);
		}
	}
	LenCode5 = z;

	// 6. CJ �� �ð����� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
		if (layerInfo.orderInCJ_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.orderInCJ_name[xx].c_str());
			strcpy(code6[z++], tempStr);
		}
	}
	LenCode6 = z;

	// 7. ���� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
		if (layerInfo.obj_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.obj_name[xx].c_str());
			strcpy(code7[z++], tempStr);
		}
	}
	LenCode7 = z;

	if (layerInfo.extendedLayer == true) {
		// 8. ����ó ���� �ڵ� ���ڿ� �����
		z = 0;
		for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
			if (layerInfo.productSite_state[xx] == true) {
				sprintf(tempStr, "%s", layerInfo.productSite_name[xx].c_str());
				strcpy(code8[z++], tempStr);
			}
		}
		LenCode8 = z;

		// 9. ���� ��ȣ �ڵ� ���ڿ� �����
		z = 0;
		for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
			if (layerInfo.productNum_state[xx] == true) {
				sprintf(tempStr, "%s", layerInfo.productNum_name[xx].c_str());
				strcpy(code9[z++], tempStr);
			}
		}
		LenCode9 = z;
	}

	// ������ ���̾� ����
	madeLayers = 0;

	// ���̾� �̸� �����ϱ�
	if (layerInfo.extendedLayer == true) {
		for (x1 = 0; x1 < LenCode1; ++x1) {
			for (x2 = 0; x2 < LenCode2; ++x2) {
				for (x3 = 0; x3 < LenCode3; ++x3) {
					for (x4 = 0; x4 < LenCode4; ++x4) {
						for (x5 = 0; x5 < LenCode5; ++x5) {
							for (x6 = 0; x6 < LenCode6; ++x6) {
								for (x7 = 0; x7 < LenCode7; ++x7) {
									for (x8 = 0; x8 < LenCode8; ++x8) {
										for (x9 = 0; x9 < LenCode9; ++x9) {
											bNormalLayer = isFullLayer(&layerInfo);	// ���̾� �̸��� �������� ü���� �̸��ΰ�?

											// ���� ����
											strcpy(fullLayerName, "");
											strcpy(fullLayerName, code1[x1]);

											// �� ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code2[x2]);

											// �� ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code3[x3]);

											// Ÿ����ȣ
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code4[x4]);

											// CJ ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code5[x5]);

											// CJ �� �ð�����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code6[x6]);

											// ���� ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code7[x7]);

											// ����ó ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code8[x8]);

											// ���� ��ȣ
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code9[x9]);

											// �������� ���̾� �̸��̸� ���̾� �Ӽ��� ������
											if (bNormalLayer == true) {
												// ���̾� �����ϱ�
												BNZeroMemory(&attrib, sizeof(API_Attribute));
												BNZeroMemory(&defs, sizeof(API_AttributeDef));

												attrib.header.typeID = API_LayerID;
												attrib.layer.conClassId = 1;
												CHCopyC(fullLayerName, attrib.header.name);
												err = ACAPI_Attribute_Create(&attrib, &defs);

												ACAPI_DisposeAttrDefsHdls(&defs);

												// �����ϸ� ���̾� ���� ���� ����
												if (err == NoError)
													madeLayers++;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else {
		for (x1 = 0; x1 < LenCode1; ++x1) {
			for (x2 = 0; x2 < LenCode2; ++x2) {
				for (x3 = 0; x3 < LenCode3; ++x3) {
					for (x4 = 0; x4 < LenCode4; ++x4) {
						for (x5 = 0; x5 < LenCode5; ++x5) {
							for (x6 = 0; x6 < LenCode6; ++x6) {
								for (x7 = 0; x7 < LenCode7; ++x7) {
									bNormalLayer = isFullLayer(&layerInfo);	// ���̾� �̸��� �������� ü���� �̸��ΰ�?

									// ���� ����
									strcpy(fullLayerName, "");
									strcpy(fullLayerName, code1[x1]);

									// �� ����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code2[x2]);

									// �� ����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code3[x3]);

									// Ÿ����ȣ
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code4[x4]);

									// CJ ����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code5[x5]);

									// CJ �� �ð�����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code6[x6]);

									// ���� ����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code7[x7]);

									// �������� ���̾� �̸��̸� ���̾� �Ӽ��� ������
									if (bNormalLayer == true) {
										// ���̾� �����ϱ�
										BNZeroMemory(&attrib, sizeof(API_Attribute));
										BNZeroMemory(&defs, sizeof(API_AttributeDef));

										attrib.header.typeID = API_LayerID;
										attrib.layer.conClassId = 1;
										CHCopyC(fullLayerName, attrib.header.name);
										err = ACAPI_Attribute_Create(&attrib, &defs);

										ACAPI_DisposeAttrDefsHdls(&defs);

										// �����ϸ� ���̾� ���� ���� ����
										if (err == NoError)
											madeLayers++;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// �޸� ����
	deallocateMemory(&layerInfo);

	// ������ ���̾� ���� �����ֱ�
	WriteReport_Alert("�� %d ���� ���̾ �����Ǿ����ϴ�.", madeLayers);

	return	err;
}

// [���̾�α� �ڽ�] ���̾� ���� �����
short DGCALLBACK layerMakeHandler(short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	dialogSizeX, dialogSizeY;
	short	xx;
	short	anyTrue;

	switch (message) {
	case DG_MSG_INIT:
		// ���̾�α� Ÿ��Ʋ
		DGSetDialogTitle(dialogID, L"���̾� ���� �����");

		// Ȯ�� ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 1020, 25, 60, 25);
		DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_OK, L"����");
		DGShowItem(dialogID, DG_OK);

		// ��� ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 1020, 60, 60, 25);
		DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_CANCEL, L"���");
		DGShowItem(dialogID, DG_CANCEL);

		// ��ư: ���� ����
		itmPosX = 30;
		itmPosY = 50;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_CODE, L"���� ����");
		DGShowItem(dialogID, BUTTON_CODE);

		// ��ư: �� ����
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_DONG, L"�� ����");
		DGShowItem(dialogID, BUTTON_DONG);

		// ��ư: �� ����
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_FLOOR, L"�� ����");
		DGShowItem(dialogID, BUTTON_FLOOR);

		// ��ư: Ÿ�� ��ȣ
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_CAST, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_CAST, L"Ÿ�� ��ȣ");
		DGShowItem(dialogID, BUTTON_CAST);

		// ��ư: CJ
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_CJ, L"CJ");
		DGShowItem(dialogID, BUTTON_CJ);

		// ��ư: �ð�����
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_ORDER, L"�ð�����");
		DGShowItem(dialogID, BUTTON_ORDER);

		// ��ư: ����
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_OBJ, L"����");
		DGShowItem(dialogID, BUTTON_OBJ);

		// ��ư: ����ó ����
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_PRODUCT_SITE, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_PRODUCT_SITE, L"����ó ����");
		DGShowItem(dialogID, BUTTON_PRODUCT_SITE);

		// ��ư: ���� ��ȣ
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_PRODUCT_NUM, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_PRODUCT_NUM, L"���� ��ȣ");
		DGShowItem(dialogID, BUTTON_PRODUCT_NUM);

		// ������
		itmPosX = 115;
		itmPosY = 60;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_1);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_2);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_3);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_4);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_5);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_6);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_7);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_8);

		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, 1000, 5, 1, 110);
		DGShowItem(dialogID, SEPARATOR_9);

		// üũ�ڽ� ǥ��
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 840, 25, 140, 20);
		DGSetItemFont(dialogID, CHECKBOX_PRODUCT_SITE_NUM, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, CHECKBOX_PRODUCT_SITE_NUM, L"����ó/��ȣ ����");
		DGShowItem(dialogID, CHECKBOX_PRODUCT_SITE_NUM);
		DGSetItemValLong(dialogID, CHECKBOX_PRODUCT_SITE_NUM, TRUE);

		break;

	case DG_MSG_CHANGE:
		switch (item) {
		case CHECKBOX_PRODUCT_SITE_NUM:
			if (DGGetItemValLong(dialogID, CHECKBOX_PRODUCT_SITE_NUM) == TRUE) {
				DGEnableItem(dialogID, BUTTON_PRODUCT_SITE);
				DGEnableItem(dialogID, BUTTON_PRODUCT_NUM);
			}
			else {
				DGDisableItem(dialogID, BUTTON_PRODUCT_SITE);
				DGDisableItem(dialogID, BUTTON_PRODUCT_NUM);
			}

			break;
		}

		break;

	case DG_MSG_CLICK:
		switch (item) {
		case DG_OK:
			if (DGGetItemValLong(dialogID, CHECKBOX_PRODUCT_SITE_NUM) == TRUE)
				layerInfo.extendedLayer = true;
			else
				layerInfo.extendedLayer = false;
			break;

		case DG_CANCEL:
			break;

		default:
			clickedBtnItemIdx = item;
			item = 0;	// �ٸ� ��ư�� ������ �� ���̾�αװ� ������ �ʰ� ��

			dialogSizeX = 600;
			dialogSizeY = 500;

			// ���� â ũ��
			if (clickedBtnItemIdx == BUTTON_CODE) {
				dialogSizeX = 600;
				dialogSizeY = 180;
			}
			else if (clickedBtnItemIdx == BUTTON_DONG) {
				dialogSizeX = 600;
				dialogSizeY = 250;
			}
			else if (clickedBtnItemIdx == BUTTON_FLOOR) {
				dialogSizeX = 1000;
				dialogSizeY = 550;
			}
			else if (clickedBtnItemIdx == BUTTON_CAST) {
				dialogSizeX = 500;
				dialogSizeY = 350;
			}
			else if (clickedBtnItemIdx == BUTTON_CJ) {
				dialogSizeX = 500;
				dialogSizeY = 350;
			}
			else if (clickedBtnItemIdx == BUTTON_ORDER) {
				dialogSizeX = 500;
				dialogSizeY = 350;
			}
			else if (clickedBtnItemIdx == BUTTON_OBJ) {
				dialogSizeX = 1200;
				dialogSizeY = 800;
			}
			else if (clickedBtnItemIdx == BUTTON_PRODUCT_SITE) {
				dialogSizeX = 600;
				dialogSizeY = 90;
			}
			else if (clickedBtnItemIdx == BUTTON_PRODUCT_NUM) {
				dialogSizeX = 600;
				dialogSizeY = 250;
			}

			// [���̾�α� �ڽ�] ���̾� ���� ����� 2��
			result = DGBlankModalDialog(dialogSizeX, dialogSizeY, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerMakeHandler_2, 0);

			// ��ư�� �۲� ���� (���� ����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.code_name.size(); ++xx)
				if (layerInfo.code_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (�� ����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.dong_name.size(); ++xx)
				if (layerInfo.dong_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (�� ����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.floor_name.size(); ++xx)
				if (layerInfo.floor_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (Ÿ�� ��ȣ)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.cast_name.size(); ++xx)
				if (layerInfo.cast_state[xx] == true) anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_CAST, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_CAST, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (CJ)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx)
				if (layerInfo.CJ_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (�ð�����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx)
				if (layerInfo.orderInCJ_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.obj_name.size(); ++xx)
				if (layerInfo.obj_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (����ó ����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx)
				if (layerInfo.productSite_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_PRODUCT_SITE, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_PRODUCT_SITE, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (���� ��ȣ)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx)
				if (layerInfo.productNum_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_PRODUCT_NUM, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_PRODUCT_NUM, DG_IS_LARGE | DG_IS_PLAIN);
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

// [���̾�α� �ڽ�] ���̾� ���� ����� 2��
short DGCALLBACK layerMakeHandler_2(short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy;
	char	tempStr[128];

	char	dongStr[256];			// Edit ��Ʈ�ѷκ��� �Է¹��� ","�� ���е� �� ���ڿ��� ���� ����
	short	restIdx;				// ���ڰ� �ƴ� ���ڷ� �� �� ���ڿ� ��ư�� �ε���

	char	productNumStr[256];	// Edit ��Ʈ�ѷκ��� �Է¹��� "-"�� ���е� �� ���ڿ��� ���� ����
	bool	bFoundFirstNum;			// ���۵Ǵ� ���� ��ȣ�� ã�Ҵ��� ����
	bool	bFoundLastNum;			// ������ ���� ��ȣ�� ã�Ҵ��� ����
	char	firstNumStr[10], lastNumStr[10];		// ���۵Ǵ� ���� ��ȣ, ������ ���� ��ȣ

	char* token;			// �о�� ���ڿ��� ��ū
	short	count;
	short	nButton;

	switch (message) {
	case DG_MSG_INIT:

		// ���̾�α� Ÿ��Ʋ
		DGSetDialogTitle(dialogID, L"���� ����");

		// Ȯ�� ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 10, 60, 25);
		DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_OK, L"Ȯ��");
		DGShowItem(dialogID, DG_OK);

		// ��� ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 60, 25);
		DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_CANCEL, L"���");
		DGShowItem(dialogID, DG_CANCEL);

		if (clickedBtnItemIdx == BUTTON_CODE) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				sprintf(tempStr, "%s %s", layerInfo.code_name[xx].c_str(), layerInfo.code_desc[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				(layerInfo.code_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

				itmPosX += 100;
				if (itmPosX >= 500) {
					itmPosX = 90;
					itmPosY += 30;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_DONG) {
			itmPosX = 90;
			itmPosY = 10;

			// �� ��ȣ�� ������ �Է� ���� ��
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 300, 50);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itmIdx, L"�� ��ȣ [101~1599������ ����] (��: 0101,0105,1102)\n��, �� ���� ���� �� 0000");
			DGShowItem(dialogID, itmIdx);

			itmPosY += 45;

			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 90, 20);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itmIdx, L"�� ��ȣ �Է�");
			DGShowItem(dialogID, itmIdx);

			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_TEXT, 256, itmPosX + 90, itmPosY - 5, 400, 20);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem(dialogID, itmIdx);
			DONG_STRING_EDITCONTROL = itmIdx;

			itmPosY += 30;

			// �̹� ��ϵ� �� ��ȣ ����ϱ�
			strcpy(dongStr, "");
			for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
				if (strlen(layerInfo.dong_name[xx].c_str()) == 3)
					sprintf(tempStr, "0%s", layerInfo.dong_name[xx].c_str());
				else if (strlen(layerInfo.dong_name[xx].c_str()) == 1)
					sprintf(tempStr, "000%s", layerInfo.dong_name[xx].c_str());
				else
					sprintf(tempStr, "%s", layerInfo.dong_name[xx].c_str());

				if (isStringDouble(tempStr) == TRUE) {
					if (layerInfo.dong_state[xx] == true) {
						strcat(dongStr, tempStr);
						strcat(dongStr, ",");
					}
				}
			}
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 500, 23);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itmIdx, charToWchar(dongStr));
			DGShowItem(dialogID, itmIdx);

			// �̹� ��ϵ� �� ��ȣ�� Edit ��Ʈ�ѿ��� �־�� ��
			DGSetItemText(dialogID, DONG_STRING_EDITCONTROL, charToWchar(dongStr));

			itmPosY += 40;

			// ���� �� ��ȣ�̸� �����ϰ�, �� ��ȣ�� �ƴϸ� ����� ��
			restIdx = 0;
			for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
				if (strlen(layerInfo.dong_name[xx].c_str()) == 3)
					sprintf(tempStr, "0%s", layerInfo.dong_name[xx].c_str());
				else if (strlen(layerInfo.dong_name[xx].c_str()) == 1)
					sprintf(tempStr, "000%s", layerInfo.dong_name[xx].c_str());
				else
					sprintf(tempStr, layerInfo.dong_name[xx].c_str());

				// �����̸� ���, �����̸� ��ŵ
				if (isStringDouble(tempStr) == FALSE) {
					itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 35);
					DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					sprintf(tempStr, "%s\n%s", layerInfo.dong_name[xx].c_str(), layerInfo.dong_desc[xx].c_str());
					DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
					DGShowItem(dialogID, itmIdx);
					DONG_REST_BUTTONS[restIdx] = itmIdx;
					restIdx++;
					(layerInfo.dong_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

					itmPosX += 100;
					if (itmPosX >= 500) {
						itmPosX = 90;
						itmPosY += 40;
					}
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_FLOOR) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 35);
				DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				sprintf(tempStr, "%s\n%s", layerInfo.floor_name[xx].c_str(), layerInfo.floor_desc[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				(layerInfo.floor_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

				itmPosX += 100;
				if (itmPosX >= 950) {
					itmPosX = 90;
					itmPosY += 40;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_CAST) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 35, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				sprintf(tempStr, "%s", layerInfo.cast_name[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				(layerInfo.cast_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

				itmPosX += 40;
				if (itmPosX >= 480) {
					itmPosX = 90;
					itmPosY += 30;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_CJ) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 35, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				sprintf(tempStr, "%s", layerInfo.CJ_name[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				(layerInfo.CJ_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

				itmPosX += 40;
				if (itmPosX >= 480) {
					itmPosX = 90;
					itmPosY += 30;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_ORDER) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 35, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				sprintf(tempStr, "%s", layerInfo.orderInCJ_name[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				(layerInfo.orderInCJ_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

				itmPosX += 40;
				if (itmPosX >= 480) {
					itmPosX = 90;
					itmPosY += 30;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_OBJ) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < 100; ++xx) {
				OBJ_BUTTONS[xx] = 0;
			}
			count = 0;
			for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
				nButton = 0;
				for (yy = 0; yy < layerInfo.obj_name.size(); ++yy) {
					if (yy == 0) {
						itmPosY += 5;

						itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 100, 23);
						DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
						sprintf(tempStr, "%s %s", layerInfo.code_name[xx].c_str(), layerInfo.code_desc[xx].c_str());
						DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
						DGShowItem(dialogID, itmIdx);

						itmPosY += 23;
					}

					if (strncmp(layerInfo.code_name[xx].c_str(), layerInfo.obj_cat[yy].c_str(), 4) == 0) {
						itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 95, 28);
						DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
						sprintf(tempStr, "%s\n%s", layerInfo.obj_name[yy].c_str(), layerInfo.obj_desc[yy].c_str());
						DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
						DGShowItem(dialogID, itmIdx);
						OBJ_BUTTONS[count] = itmIdx;
						(layerInfo.obj_state[yy] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);
						count++;
						nButton++;

						itmPosX += 100;
						if (itmPosX >= 1100) {
							itmPosX = 90;
							itmPosY += 30;
						}
					}
				}
				itmPosX = 90;
				if (nButton == 0) {
					DGHideItem(dialogID, itmIdx);
					itmPosY -= 28;
				}
				else {
					itmPosY += 30;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_PRODUCT_SITE) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				sprintf(tempStr, "%s", layerInfo.productSite_name[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				(layerInfo.productSite_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

				itmPosX += 100;
				if (itmPosX >= 500) {
					itmPosX = 90;
					itmPosY += 30;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_PRODUCT_NUM) {
			itmPosX = 90;
			itmPosY = 10;

			// ���� ��ȣ�� ������ �Է� ���� ��
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 400, 23);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itmIdx, L"���� ��ȣ [001~999���� ����] (��: 001-100, �������� ������)");
			DGShowItem(dialogID, itmIdx);

			itmPosY += 30;

			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 90, 20);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itmIdx, L"���� ��ȣ �Է�");
			DGShowItem(dialogID, itmIdx);

			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_TEXT, 256, itmPosX + 90, itmPosY - 5, 400, 20);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem(dialogID, itmIdx);
			PRODUCT_NUM_STRING_EDITCONTROL = itmIdx;

			itmPosY += 30;

			// �̹� ��ϵ� ���� ��ȣ ����ϱ�
			bFoundFirstNum = false;
			bFoundLastNum = false;
			firstNumStr[0] = '\0';
			lastNumStr[0] = '\0';
			strcpy(productNumStr, "");
			for (xx = 0; xx < (layerInfo.productNum_name.size() - 1); ++xx) {
				if (layerInfo.productNum_state[xx] == true) {
					// ó�� ã�� ���� ��ȣ�� ���۵Ǵ� ���� ��ȣ�� ������
					if ((bFoundFirstNum == false) && (firstNumStr[0] == '\0')) {
						strcpy(firstNumStr, layerInfo.productNum_name[xx].c_str());
						bFoundFirstNum = true;
					}

					// �������� ���� ��ȣ �� ���������� �߰ߵ� ���� ��ȣ�� ������ ���� ��ȣ�� ������
					if ((bFoundLastNum == false) && (bFoundFirstNum == true) && (layerInfo.productNum_state[xx + 1] == false)) {
						strcpy(lastNumStr, layerInfo.productNum_name[xx].c_str());
						bFoundLastNum = true;
					}
				}
			}

			strcpy(productNumStr, "");
			if (bFoundFirstNum == true) {
				if (strlen(firstNumStr) == 3) {
					sprintf(tempStr, "%s", firstNumStr);
				}
				else if (strlen(firstNumStr) == 2) {
					sprintf(tempStr, "0%s", firstNumStr);
				}
				else if (strlen(firstNumStr) == 1) {
					sprintf(tempStr, "00%s", firstNumStr);
				}
				strcat(productNumStr, tempStr);
			}
			if ((bFoundFirstNum == true) || (bFoundLastNum == true))
				strcat(productNumStr, "-");
			if (bFoundLastNum == true) {
				if (strlen(lastNumStr) == 3) {
					sprintf(tempStr, "%s", lastNumStr);
				}
				else if (strlen(lastNumStr) == 2) {
					sprintf(tempStr, "0%s", lastNumStr);
				}
				else if (strlen(lastNumStr) == 1) {
					sprintf(tempStr, "00%s", lastNumStr);
				}
				strcat(productNumStr, tempStr);
			}

			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 500, 23);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itmIdx, charToWchar(productNumStr));
			DGShowItem(dialogID, itmIdx);

			// �̹� ��ϵ� ���� ��ȣ�� Edit ��Ʈ�ѿ��� �־�� ��
			DGSetItemText(dialogID, PRODUCT_NUM_STRING_EDITCONTROL, charToWchar(productNumStr));
		}

		break;

	case DG_MSG_CLICK:
		switch (item) {
		case DG_OK:

			if (clickedBtnItemIdx == BUTTON_CODE) {
				for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, 3 + xx) == TRUE)
						layerInfo.code_state[xx] = true;
					else
						layerInfo.code_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_DONG) {
				// �ϴ� false�� �ʱ�ȭ
				for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
					layerInfo.dong_state[xx] = false;
				}

				// DONG_STRING_EDITCONTROL ���ڿ��� ","�� �߶� �ش� �� ��ȣ�� ã�Ƽ� true
				strcpy(dongStr, DGGetItemText(dialogID, DONG_STRING_EDITCONTROL).ToCStr().Get());
				token = strtok(dongStr, ",");
				while (token != NULL) {
					// ���ڰ� �´ٸ�,
					if (isStringDouble(token) == TRUE) {
						for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
							if (strncmp(layerInfo.dong_name[xx].c_str(), token, 4) == 0) {
								layerInfo.dong_state[xx] = true;
							}
						}
					}

					token = strtok(NULL, ",");
				}

				// ���� �� ��ȣ�̸� �����ϰ�, �� ��ȣ�� �ƴϸ� ���¸� ������ ��
				restIdx = 0;
				for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
					if (strlen(layerInfo.dong_name[xx].c_str()) != 4)
						sprintf(tempStr, "0%s", layerInfo.dong_name[xx].c_str());
					else
						sprintf(tempStr, "%s", layerInfo.dong_name[xx].c_str());

					// �����̸� ���, �����̸� ��ŵ
					if (isStringDouble(tempStr) == FALSE) {
						if (DGGetItemValLong(dialogID, DONG_REST_BUTTONS[restIdx]) == TRUE) {
							layerInfo.dong_state[xx] = true;
						}
						else {
							layerInfo.dong_state[xx] = false;
						}

						restIdx++;
					}
				}
			}
			else if (clickedBtnItemIdx == BUTTON_FLOOR) {
				for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, 3 + xx) == TRUE)
						layerInfo.floor_state[xx] = true;
					else
						layerInfo.floor_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_CAST) {
				for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, 3 + xx) == TRUE)
						layerInfo.cast_state[xx] = true;
					else
						layerInfo.cast_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_CJ) {
				for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, 3 + xx) == TRUE)
						layerInfo.CJ_state[xx] = true;
					else
						layerInfo.CJ_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_ORDER) {
				for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, 3 + xx) == TRUE)
						layerInfo.orderInCJ_state[xx] = true;
					else
						layerInfo.orderInCJ_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_OBJ) {
				for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, OBJ_BUTTONS[xx]) == TRUE)
						layerInfo.obj_state[xx] = true;
					else
						layerInfo.obj_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_PRODUCT_SITE) {
				for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, 3 + xx) == TRUE)
						layerInfo.productSite_state[xx] = true;
					else
						layerInfo.productSite_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_PRODUCT_NUM) {
				// �ϴ� false�� �ʱ�ȭ
				for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
					layerInfo.productNum_state[xx] = false;
				}

				// PRODUCT_NUM_STRING_EDITCONTROL ���ڿ��� "-"�� �߶� �� ��ȣ�� ���۵Ǵ� ���� ��ȣ��, �� ��ȣ�� ������ ���� ��ȣ�� ������
				bFoundFirstNum = false;
				bFoundLastNum = false;
				firstNumStr[0] = '\0';
				lastNumStr[0] = '\0';
				strcpy(productNumStr, DGGetItemText(dialogID, PRODUCT_NUM_STRING_EDITCONTROL).ToCStr().Get());
				token = strtok(productNumStr, "-");
				while (token != NULL) {
					// ���ڰ� �´ٸ�,
					if (isStringDouble(token) == TRUE) {
						if ((bFoundFirstNum == false) && (firstNumStr[0] == '\0')) {
							// ó�� ��ȯ�� ���� ���� ���� ��ȣ�� ��
							if (bFoundFirstNum == false) {
								if (strlen(token) == 3) {
									sprintf(firstNumStr, "%s", token);
								}
								else if (strlen(token) == 2) {
									sprintf(firstNumStr, "0%s", token);
								}
								else if (strlen(token) == 1) {
									sprintf(firstNumStr, "00%s", token);
								}
								bFoundFirstNum = true;
								token = strtok(NULL, "-");
							}

							// ���߿� ��ȯ�� ���� ���� �� ��ȣ�� ��
							if (token != NULL) {
								if ((bFoundLastNum == false) && (bFoundFirstNum == true) && (lastNumStr[0] == '\0')) {
									if (strlen(token) == 3) {
										sprintf(lastNumStr, "%s", token);
									}
									else if (strlen(token) == 2) {
										sprintf(lastNumStr, "0%s", token);
									}
									else if (strlen(token) == 1) {
										sprintf(lastNumStr, "00%s", token);
									}
									bFoundLastNum = true;
								}
							}
						}
					}

					token = strtok(NULL, "-");
				}

				for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
					if ((atoi(layerInfo.productNum_name[xx].c_str()) >= atoi(firstNumStr)) && (atoi(layerInfo.productNum_name[xx].c_str()) <= atoi(lastNumStr))) {
						layerInfo.productNum_state[xx] = true;
					}
				}
			}

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

// ���̾� ���� �����ϱ�
GSErrCode	assignLayerEasily(void)
{
	GSErrCode	err = NoError;

	string	insElem;		// ��ū�� string���� ��ȯ�ؼ� vector�� ����

	GS::Array<API_Guid>		objects;
	long					nObjects;
	API_Element				elem, mask;

	API_Attribute	attrib;
	API_AttributeDef  defs;
	short	xx;

	char	tempStr[128];

	short	result;


	// �׷�ȭ �Ͻ����� ON
	suspendGroups(true);

	// ������ ��ü�� �ִ��� Ȯ����
	err = getGuidsOfSelection(&objects, API_ObjectID, &nObjects);
	if (err == APIERR_NOPLAN) {
		DGAlert(DG_ERROR, L"����", L"���� ������Ʈ â�� �����ϴ�.", "", L"Ȯ��", "", "");
	}
	if (err == APIERR_NOSEL) {
		DGAlert(DG_ERROR, L"����", L"��ü�� �ƹ� �͵� �������� �ʾҽ��ϴ�.", "", L"Ȯ��", "", "");
	}
	if (err != NoError) {
		return err;
	}

	// ���̾� ���� ���� ��������
	importLayerInfo(&layerInfo);

	// ����ü �ʱ�ȭ
	allocateMemory(&layerInfo);
	selectedInfo = layerInfo;	// selectedInfo���� vector�� ��� �����Ƿ� �ʱ�ȭ�� ���� ������ ��
	allocateMemory(&selectedInfo);

	// [���̾�α� �ڽ�] ���̾� ���� �����ϱ�
	result = DGBlankModalDialog(1100, 120, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerAssignHandler, 0);

	// OK ��ư�� �ƴϸ� �޸� �����ϰ� ����
	if (result != DG_OK) {
		deallocateMemory(&layerInfo);
		deallocateMemory(&selectedInfo);
		return	err;
	}

	short	z;
	char	code1[25][32];		// ���� �ڵ�
	short	LenCode1;
	char	code2[1600][32];	// �� �ڵ�
	short	LenCode2;
	char	code3[120][32];	// �� �ڵ�
	short	LenCode3;
	char	code4[120][32];	// Ÿ����ȣ �ڵ�
	short	LenCode4;
	char	code5[120][32];	// CJ �ڵ�
	short	LenCode5;
	char	code6[120][32];	// CJ �� �ð����� �ڵ�
	short	LenCode6;
	char	code7[120][32];	// ���� �ڵ�
	short	LenCode7;
	char	code8[10][32];		// ����ó �ڵ�
	short	LenCode8;
	char	code9[1000][32];	// ���۹�ȣ �ڵ�
	short	LenCode9;

	char	fullLayerName[128];
	bool	bNormalLayer;
	short	x1, x2, x3, x4, x5, x6, x7, x8, x9;

	// 1. ���� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
		if (layerInfo.code_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.code_name[xx].c_str());
			strcpy(code1[z++], tempStr);
		}
	}
	LenCode1 = z;

	// 2. �� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
		if (layerInfo.dong_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.dong_name[xx].c_str());
			strcpy(code2[z++], tempStr);
		}
	}
	LenCode2 = z;

	// 3. �� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
		if (layerInfo.floor_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.floor_name[xx].c_str());
			strcpy(code3[z++], tempStr);
		}
	}
	LenCode3 = z;

	// 4. Ÿ����ȣ �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
		if (layerInfo.cast_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.cast_name[xx].c_str());
			strcpy(code4[z++], tempStr);
		}
	}
	LenCode4 = z;

	// 5. CJ �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
		if (layerInfo.CJ_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.CJ_name[xx].c_str());
			strcpy(code5[z++], tempStr);
		}
	}
	LenCode5 = z;

	// 6. CJ �� �ð����� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
		if (layerInfo.orderInCJ_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.orderInCJ_name[xx].c_str());
			strcpy(code6[z++], tempStr);
		}
	}
	LenCode6 = z;

	// 7. ���� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
		if (layerInfo.obj_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.obj_name[xx].c_str());
			strcpy(code7[z++], tempStr);
		}
	}
	LenCode7 = z;

	// 8. ����ó ���� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
		if (layerInfo.productSite_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.productSite_name[xx].c_str());
			strcpy(code8[z++], tempStr);
		}
	}
	LenCode8 = z;

	// 9. ���� ��ȣ ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
		if (layerInfo.productNum_state[xx] == true) {
			sprintf(tempStr, "%s", layerInfo.productNum_name[xx].c_str());
			strcpy(code9[z++], tempStr);
		}
	}
	LenCode9 = z;

	// ���̾� �̸� �����ϱ�
	if (layerInfo.extendedLayer == true) {
		for (x1 = 0; x1 < LenCode1; ++x1) {
			for (x2 = 0; x2 < LenCode2; ++x2) {
				for (x3 = 0; x3 < LenCode3; ++x3) {
					for (x4 = 0; x4 < LenCode4; ++x4) {
						for (x5 = 0; x5 < LenCode5; ++x5) {
							for (x6 = 0; x6 < LenCode6; ++x6) {
								for (x7 = 0; x7 < LenCode7; ++x7) {
									for (x8 = 0; x8 < LenCode8; ++x8) {
										for (x9 = 0; x9 < LenCode9; ++x9) {
											bNormalLayer = isFullLayer(&layerInfo);	// ���̾� �̸��� �������� ü���� �̸��ΰ�?

											// ���� ����
											strcpy(fullLayerName, "");
											strcpy(fullLayerName, code1[x1]);

											// �� ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code2[x2]);

											// �� ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code3[x3]);

											// Ÿ����ȣ
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code4[x4]);

											// CJ ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code5[x5]);

											// CJ �� �ð�����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code6[x6]);

											// ���� ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code7[x7]);

											// ����ó ����
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code8[x8]);

											// ���� ��ȣ
											strcat(fullLayerName, "-");
											strcat(fullLayerName, code9[x9]);

											// �������� ���̾� �̸��̸�,
											if (bNormalLayer == true) {
												// ������ ���̾ �����ϴ��� Ȯ��
												BNZeroMemory(&attrib, sizeof(API_Attribute));
												attrib.header.typeID = API_LayerID;
												CHCopyC(fullLayerName, attrib.header.name);
												err = ACAPI_Attribute_Get(&attrib);

												// ���̾ �����ϸ�,
												if (err == NoError) {
													// ��ü���� ���̾� �Ӽ��� ������
													for (xx = 0; xx < nObjects; ++xx) {
														BNZeroMemory(&elem, sizeof(API_Element));
														elem.header.guid = objects.Pop();
														err = ACAPI_Element_Get(&elem);

														ACAPI_ELEMENT_MASK_CLEAR(mask);
														ACAPI_ELEMENT_MASK_SET(mask, API_Elem_Head, layer);
														elem.header.layer = attrib.layer.head.index;	// ������ ���̾� �Ӽ��� �ε����� �ο���

														err = ACAPI_Element_Change(&elem, &mask, NULL, 0, true);
													}

													// ���̾ �������� ������,
												}
												else {

													result = DGAlert(DG_INFORMATION, L"���̾ �������� ����", L"������ ���̾ �������� �ʽ��ϴ�.\n���� ����ðڽ��ϱ�?", "", L"��", L"�ƴϿ�", "");

													if (result == DG_OK) {
														// ���̾ ���� ������
														BNZeroMemory(&attrib, sizeof(API_Attribute));
														BNZeroMemory(&defs, sizeof(API_AttributeDef));

														attrib.header.typeID = API_LayerID;
														attrib.layer.conClassId = 1;
														CHCopyC(fullLayerName, attrib.header.name);
														err = ACAPI_Attribute_Create(&attrib, &defs);

														ACAPI_DisposeAttrDefsHdls(&defs);

														// ��ü���� ���̾� �Ӽ��� ������
														for (xx = 0; xx < nObjects; ++xx) {
															BNZeroMemory(&elem, sizeof(API_Element));
															elem.header.guid = objects.Pop();
															err = ACAPI_Element_Get(&elem);

															ACAPI_ELEMENT_MASK_CLEAR(mask);
															ACAPI_ELEMENT_MASK_SET(mask, API_Elem_Head, layer);
															elem.header.layer = attrib.layer.head.index;	// ������ ���̾� �Ӽ��� �ε����� �ο���

															err = ACAPI_Element_Change(&elem, &mask, NULL, 0, true);
														}
													}
													else {
														DGAlert(DG_ERROR, L"����", L"���̾ �����Ƿ� ������ �����մϴ�.", "", L"Ȯ��", "", "");
														return	err;
													}
												}
											}
											else {
												// �������� ���̾ �ƴ�
												DGAlert(DG_ERROR, L"����", L"���̾� ������ �߸��ϼ̽��ϴ�.", "", L"Ȯ��", "", "");
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else {
		for (x1 = 0; x1 < LenCode1; ++x1) {
			for (x2 = 0; x2 < LenCode2; ++x2) {
				for (x3 = 0; x3 < LenCode3; ++x3) {
					for (x4 = 0; x4 < LenCode4; ++x4) {
						for (x5 = 0; x5 < LenCode5; ++x5) {
							for (x6 = 0; x6 < LenCode6; ++x6) {
								for (x7 = 0; x7 < LenCode7; ++x7) {
									bNormalLayer = isFullLayer(&layerInfo);	// ���̾� �̸��� �������� ü���� �̸��ΰ�?

									// ���� ����
									strcpy(fullLayerName, "");
									strcpy(fullLayerName, code1[x1]);

									// �� ����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code2[x2]);

									// �� ����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code3[x3]);

									// Ÿ����ȣ
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code4[x4]);

									// CJ ����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code5[x5]);

									// CJ �� �ð�����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code6[x6]);

									// ���� ����
									strcat(fullLayerName, "-");
									strcat(fullLayerName, code7[x7]);

									// �������� ���̾� �̸��̸�,
									if (bNormalLayer == true) {
										// ������ ���̾ �����ϴ��� Ȯ��
										BNZeroMemory(&attrib, sizeof(API_Attribute));
										attrib.header.typeID = API_LayerID;
										CHCopyC(fullLayerName, attrib.header.name);
										err = ACAPI_Attribute_Get(&attrib);

										// ���̾ �����ϸ�,
										if (err == NoError) {
											// ��ü���� ���̾� �Ӽ��� ������
											for (xx = 0; xx < nObjects; ++xx) {
												BNZeroMemory(&elem, sizeof(API_Element));
												elem.header.guid = objects.Pop();
												err = ACAPI_Element_Get(&elem);

												ACAPI_ELEMENT_MASK_CLEAR(mask);
												ACAPI_ELEMENT_MASK_SET(mask, API_Elem_Head, layer);
												elem.header.layer = attrib.layer.head.index;	// ������ ���̾� �Ӽ��� �ε����� �ο���

												err = ACAPI_Element_Change(&elem, &mask, NULL, 0, true);
											}

											// ���̾ �������� ������,
										}
										else {

											result = DGAlert(DG_INFORMATION, "���̾ �������� ����", "������ ���̾ �������� �ʽ��ϴ�.\n���� ����ðڽ��ϱ�?", "", "��", "�ƴϿ�", "");

											if (result == DG_OK) {
												// ���̾ ���� ������
												BNZeroMemory(&attrib, sizeof(API_Attribute));
												BNZeroMemory(&defs, sizeof(API_AttributeDef));

												attrib.header.typeID = API_LayerID;
												attrib.layer.conClassId = 1;
												CHCopyC(fullLayerName, attrib.header.name);
												err = ACAPI_Attribute_Create(&attrib, &defs);

												ACAPI_DisposeAttrDefsHdls(&defs);

												// ��ü���� ���̾� �Ӽ��� ������
												for (xx = 0; xx < nObjects; ++xx) {
													BNZeroMemory(&elem, sizeof(API_Element));
													elem.header.guid = objects.Pop();
													err = ACAPI_Element_Get(&elem);

													ACAPI_ELEMENT_MASK_CLEAR(mask);
													ACAPI_ELEMENT_MASK_SET(mask, API_Elem_Head, layer);
													elem.header.layer = attrib.layer.head.index;	// ������ ���̾� �Ӽ��� �ε����� �ο���

													err = ACAPI_Element_Change(&elem, &mask, NULL, 0, true);
												}
											}
											else {
												DGAlert(DG_ERROR, L"����", L"���̾ �����Ƿ� ������ �����մϴ�.", "", L"Ȯ��", "", "");
												return	err;
											}
										}
									}
									else {
										// �������� ���̾ �ƴ�
										DGAlert(DG_ERROR, L"����", L"���̾� ������ �߸��ϼ̽��ϴ�.", "", L"Ȯ��", "", "");
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// �޸� ����
	deallocateMemory(&layerInfo);
	deallocateMemory(&selectedInfo);

	// �׷�ȭ �Ͻ����� OFF
	suspendGroups(false);

	return	err;
}

// [���̾�α� �ڽ�] ���̾� ���� �����ϱ�
short DGCALLBACK layerAssignHandler(short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	dialogSizeX, dialogSizeY;
	short	xx;
	short	anyTrue;

	GSErrCode err = NoError;
	API_ModulData  info;

	switch (message) {
	case DG_MSG_INIT:
		// ���̾�α� Ÿ��Ʋ
		DGSetDialogTitle(dialogID, L"���̾� ���� �����ϱ�");

		// Ȯ�� ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 1020, 10, 60, 25);
		DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_OK, L"����");
		DGShowItem(dialogID, DG_OK);

		// ��� ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 1020, 45, 60, 25);
		DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_CANCEL, L"���");
		DGShowItem(dialogID, DG_CANCEL);

		// ��ư: ���� ����
		itmPosX = 30;
		itmPosY = 50;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_CODE, L"���� ����");
		DGShowItem(dialogID, BUTTON_CODE);

		// ��ư: �� ����
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_DONG, L"�� ����");
		DGShowItem(dialogID, BUTTON_DONG);

		// ��ư: �� ����
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_FLOOR, L"�� ����");
		DGShowItem(dialogID, BUTTON_FLOOR);

		// ��ư: Ÿ�� ��ȣ
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_CAST, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_CAST, L"Ÿ�� ��ȣ");
		DGShowItem(dialogID, BUTTON_CAST);

		// ��ư: CJ
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_CJ, L"CJ");
		DGShowItem(dialogID, BUTTON_CJ);

		// ��ư: �ð�����
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_ORDER, L"�ð�����");
		DGShowItem(dialogID, BUTTON_ORDER);

		// ��ư: ����
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_OBJ, L"����");
		DGShowItem(dialogID, BUTTON_OBJ);

		// ��ư: ����ó ����
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_PRODUCT_SITE, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_PRODUCT_SITE, L"����ó ����");
		DGShowItem(dialogID, BUTTON_PRODUCT_SITE);

		// ��ư: ���� ��ȣ
		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
		DGSetItemFont(dialogID, BUTTON_PRODUCT_NUM, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_PRODUCT_NUM, L"���� ��ȣ");
		DGShowItem(dialogID, BUTTON_PRODUCT_NUM);

		// ������
		itmPosX = 115;
		itmPosY = 60;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_1);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_2);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_3);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_4);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_5);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_6);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_7);

		itmPosX += 110;
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
		DGShowItem(dialogID, SEPARATOR_8);

		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, 1000, 5, 1, 110);
		DGShowItem(dialogID, SEPARATOR_9);

		// üũ�ڽ� ǥ��
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 840, 25, 140, 20);
		DGSetItemFont(dialogID, CHECKBOX_PRODUCT_SITE_NUM, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, CHECKBOX_PRODUCT_SITE_NUM, L"����ó/��ȣ ����");
		DGShowItem(dialogID, CHECKBOX_PRODUCT_SITE_NUM);
		DGSetItemValLong(dialogID, CHECKBOX_PRODUCT_SITE_NUM, TRUE);

		// ��ư: �ε�
		BUTTON_LOAD = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 1020, 80, 60, 25);
		DGSetItemFont(dialogID, BUTTON_LOAD, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_LOAD, L"�ε�");
		DGShowItem(dialogID, BUTTON_LOAD);

		// ��: ���̾� �̸�
		LABEL_LAYER_NAME = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 15, 600, 23);
		DGSetItemFont(dialogID, LABEL_LAYER_NAME, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, LABEL_LAYER_NAME, L"���̾� �̸��� ǥ����");
		DGShowItem(dialogID, LABEL_LAYER_NAME);

		break;

	case DG_MSG_CHANGE:
		switch (item) {
		case CHECKBOX_PRODUCT_SITE_NUM:
			if (DGGetItemValLong(dialogID, CHECKBOX_PRODUCT_SITE_NUM) == TRUE) {
				DGEnableItem(dialogID, BUTTON_PRODUCT_SITE);
				DGEnableItem(dialogID, BUTTON_PRODUCT_NUM);
			}
			else {
				DGDisableItem(dialogID, BUTTON_PRODUCT_SITE);
				DGDisableItem(dialogID, BUTTON_PRODUCT_NUM);
			}

			break;
		}

		break;

	case DG_MSG_CLICK:
		if (item == DG_OK) {
			if (DGGetItemValLong(dialogID, CHECKBOX_PRODUCT_SITE_NUM) == TRUE)
				layerInfo.extendedLayer = true;
			else
				layerInfo.extendedLayer = false;

			// ��ư ���� ����
			saveButtonStatus_assign();

		}
		else if (item == DG_CANCEL) {

		}
		else if (item == BUTTON_LOAD) {
			item = 0;	// �ٸ� ��ư�� ������ �� ���̾�αװ� ������ �ʰ� ��

			char	currentLayerName[128] = "";
			char	fragOfLayer[32] = "";

			// ����� ��ư ���¸� �ҷ���
			BNZeroMemory(&info, sizeof(API_ModulData));
			err = ACAPI_ModulData_Get(&info, "ButtonStatus_assign");

			if (err == NoError && info.dataVersion == 1) {
				selectedInfoSaved = *(reinterpret_cast<StatusOfLayerNameSystem*> (*info.dataHdl));

				strcpy(fragOfLayer, "____");
				for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
					if (selectedInfoSaved.code_state[xx] == true) {
						layerInfo.code_state[xx] = true;
						strcpy(fragOfLayer, layerInfo.code_name[xx].c_str());
					}
				}
				strcat(currentLayerName, fragOfLayer);
				strcat(currentLayerName, " - ");
				strcpy(fragOfLayer, "____");
				for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
					if (selectedInfoSaved.dong_state[xx] == true) {
						layerInfo.dong_state[xx] = true;
						strcpy(fragOfLayer, layerInfo.dong_name[xx].c_str());
					}
				}
				strcat(currentLayerName, fragOfLayer);
				strcat(currentLayerName, " - ");
				strcpy(fragOfLayer, "___");
				for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
					if (selectedInfoSaved.floor_state[xx] == true) {
						layerInfo.floor_state[xx] = true;
						strcpy(fragOfLayer, layerInfo.floor_name[xx].c_str());
					}
				}
				strcat(currentLayerName, fragOfLayer);
				strcat(currentLayerName, " - ");
				strcpy(fragOfLayer, "__");
				for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
					if (selectedInfoSaved.cast_state[xx] == true) {
						layerInfo.cast_state[xx] = true;
						strcpy(fragOfLayer, layerInfo.cast_name[xx].c_str());
					}
				}
				strcat(currentLayerName, fragOfLayer);
				strcat(currentLayerName, " - ");
				strcpy(fragOfLayer, "__");
				for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
					if (selectedInfoSaved.CJ_state[xx] == true) {
						layerInfo.CJ_state[xx] = true;
						strcpy(fragOfLayer, layerInfo.CJ_name[xx].c_str());
					}
				}
				strcat(currentLayerName, fragOfLayer);
				strcat(currentLayerName, " - ");
				strcpy(fragOfLayer, "__");
				for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
					if (selectedInfoSaved.orderInCJ_state[xx] == true) {
						layerInfo.orderInCJ_state[xx] = true;
						strcpy(fragOfLayer, layerInfo.orderInCJ_name[xx].c_str());
					}
				}
				strcat(currentLayerName, fragOfLayer);
				strcat(currentLayerName, " - ");
				strcpy(fragOfLayer, "____");
				for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
					if (selectedInfoSaved.obj_state[xx] == true) {
						layerInfo.obj_state[xx] = true;
						strcpy(fragOfLayer, layerInfo.obj_name[xx].c_str());
					}
				}
				strcat(currentLayerName, fragOfLayer);
				if (DGGetItemValLong(dialogID, CHECKBOX_PRODUCT_SITE_NUM) == TRUE) {
					strcat(currentLayerName, " - ");
					strcpy(fragOfLayer, "____");
					for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
						if (selectedInfoSaved.productSite_state[xx] == true) {
							layerInfo.productSite_state[xx] = true;
							strcpy(fragOfLayer, layerInfo.productSite_name[xx].c_str());
						}
					}
					strcat(currentLayerName, fragOfLayer);
					strcat(currentLayerName, " - ");
					strcpy(fragOfLayer, "___");
					for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
						if (selectedInfoSaved.productNum_state[xx] == true) {
							layerInfo.productNum_state[xx] = true;
							strcpy(fragOfLayer, layerInfo.productNum_name[xx].c_str());
						}
					}
					strcat(currentLayerName, fragOfLayer);
				}
			}

			BMKillHandle(&info.dataHdl);

			// ���̾� �̸� �󺧿� ������� ������ ���̾��� �̸��� ǥ����
			DGSetItemText(dialogID, LABEL_LAYER_NAME, charToWchar(currentLayerName));

			// ��ư�� �۲� ���� (���� ����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.code_name.size(); ++xx)
				if (layerInfo.code_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (�� ����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.dong_name.size(); ++xx)
				if (layerInfo.dong_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (�� ����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.floor_name.size(); ++xx)
				if (layerInfo.floor_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (Ÿ�� ��ȣ)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.cast_name.size(); ++xx)
				if (layerInfo.cast_state[xx] == true) anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_CAST, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_CAST, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (CJ)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx)
				if (layerInfo.CJ_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (�ð�����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx)
				if (layerInfo.orderInCJ_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.obj_name.size(); ++xx)
				if (layerInfo.obj_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (����ó ����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx)
				if (layerInfo.productSite_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_PRODUCT_SITE, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_PRODUCT_SITE, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (���� ��ȣ)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx)
				if (layerInfo.productNum_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_PRODUCT_NUM, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_PRODUCT_NUM, DG_IS_LARGE | DG_IS_PLAIN);
			}
		}
		else {
			clickedBtnItemIdx = item;
			item = 0;	// �ٸ� ��ư�� ������ �� ���̾�αװ� ������ �ʰ� ��

			dialogSizeX = 600;
			dialogSizeY = 500;

			// ���� â ũ��
			if (clickedBtnItemIdx == BUTTON_CODE) {
				dialogSizeX = 600;
				dialogSizeY = 180;
			}
			else if (clickedBtnItemIdx == BUTTON_DONG) {
				dialogSizeX = 600;
				dialogSizeY = 250;
			}
			else if (clickedBtnItemIdx == BUTTON_FLOOR) {
				dialogSizeX = 1000;
				dialogSizeY = 550;
			}
			else if (clickedBtnItemIdx == BUTTON_CAST) {
				dialogSizeX = 500;
				dialogSizeY = 350;
			}
			else if (clickedBtnItemIdx == BUTTON_CJ) {
				dialogSizeX = 500;
				dialogSizeY = 350;
			}
			else if (clickedBtnItemIdx == BUTTON_ORDER) {
				dialogSizeX = 500;
				dialogSizeY = 350;
			}
			else if (clickedBtnItemIdx == BUTTON_OBJ) {
				dialogSizeX = 1200;
				dialogSizeY = 800;
			}
			else if (clickedBtnItemIdx == BUTTON_PRODUCT_SITE) {
				dialogSizeX = 600;
				dialogSizeY = 90;
			}
			else if (clickedBtnItemIdx == BUTTON_PRODUCT_NUM) {
				dialogSizeX = 600;
				dialogSizeY = 250;
			}

			// [���̾�α� �ڽ�] ���̾� ���� �����ϱ� 2��
			result = DGBlankModalDialog(dialogSizeX, dialogSizeY, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerAssignHandler_2, 0);

			// ��ư�� �۲� ���� (���� ����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.code_name.size(); ++xx)
				if (layerInfo.code_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (�� ����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.dong_name.size(); ++xx)
				if (layerInfo.dong_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (�� ����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.floor_name.size(); ++xx)
				if (layerInfo.floor_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (Ÿ�� ��ȣ)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.cast_name.size(); ++xx)
				if (layerInfo.cast_state[xx] == true) anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_CAST, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_CAST, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (CJ)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx)
				if (layerInfo.CJ_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (�ð�����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx)
				if (layerInfo.orderInCJ_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.obj_name.size(); ++xx)
				if (layerInfo.obj_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (����ó ����)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx)
				if (layerInfo.productSite_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_PRODUCT_SITE, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_PRODUCT_SITE, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ��ư�� �۲� ���� (���� ��ȣ)
			anyTrue = 0;
			for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx)
				if (layerInfo.productNum_state[xx] == true)	anyTrue++;
			if (anyTrue > 0) {
				DGSetItemFont(dialogID, BUTTON_PRODUCT_NUM, DG_IS_LARGE | DG_IS_BOLD);
			}
			else {
				DGSetItemFont(dialogID, BUTTON_PRODUCT_NUM, DG_IS_LARGE | DG_IS_PLAIN);
			}

			// ���̾� �̸� �󺧿� ������� ������ ���̾��� �̸��� ǥ����
			char	currentLayerName[128] = "";
			char	fragOfLayer[32] = "";

			strcpy(fragOfLayer, "____");
			for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
				if (layerInfo.code_state[xx] == true)
					strcpy(fragOfLayer, layerInfo.code_name[xx].c_str());
			}
			strcat(currentLayerName, fragOfLayer);
			strcat(currentLayerName, " - ");
			strcpy(fragOfLayer, "____");
			for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
				if (layerInfo.dong_state[xx] == true)
					strcpy(fragOfLayer, layerInfo.dong_name[xx].c_str());
			}
			strcat(currentLayerName, fragOfLayer);
			strcat(currentLayerName, " - ");
			strcpy(fragOfLayer, "___");
			for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
				if (layerInfo.floor_state[xx] == true)
					strcpy(fragOfLayer, layerInfo.floor_name[xx].c_str());
			}
			strcat(currentLayerName, fragOfLayer);
			strcat(currentLayerName, " - ");
			strcpy(fragOfLayer, "__");
			for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
				if (layerInfo.cast_state[xx] == true)
					strcpy(fragOfLayer, layerInfo.cast_name[xx].c_str());
			}
			strcat(currentLayerName, fragOfLayer);
			strcat(currentLayerName, " - ");
			strcpy(fragOfLayer, "__");
			for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
				if (layerInfo.CJ_state[xx] == true)
					strcpy(fragOfLayer, layerInfo.CJ_name[xx].c_str());
			}
			strcat(currentLayerName, fragOfLayer);
			strcat(currentLayerName, " - ");
			strcpy(fragOfLayer, "__");
			for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
				if (layerInfo.orderInCJ_state[xx] == true)
					strcpy(fragOfLayer, layerInfo.orderInCJ_name[xx].c_str());
			}
			strcat(currentLayerName, fragOfLayer);
			strcat(currentLayerName, " - ");
			strcpy(fragOfLayer, "____");
			for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
				if (layerInfo.obj_state[xx] == true)
					strcpy(fragOfLayer, layerInfo.obj_name[xx].c_str());
			}
			strcat(currentLayerName, fragOfLayer);
			if (DGGetItemValLong(dialogID, CHECKBOX_PRODUCT_SITE_NUM) == TRUE) {
				strcat(currentLayerName, " - ");
				strcpy(fragOfLayer, "____");
				for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
					if (layerInfo.productSite_state[xx] == true)
						strcpy(fragOfLayer, layerInfo.productSite_name[xx].c_str());
				}
				strcat(currentLayerName, fragOfLayer);
				strcat(currentLayerName, " - ");
				strcpy(fragOfLayer, "___");
				for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
					if (layerInfo.productNum_state[xx] == true)
						strcpy(fragOfLayer, layerInfo.productNum_name[xx].c_str());
				}
				strcat(currentLayerName, fragOfLayer);
			}

			DGSetItemText(dialogID, LABEL_LAYER_NAME, charToWchar(currentLayerName));
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

// [���̾�α� �ڽ�] ���̾� ���� �����ϱ� 2��
short DGCALLBACK layerAssignHandler_2(short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy;
	char	tempStr[128];

	char	dongStr[256];			// Edit ��Ʈ�ѷκ��� �Է¹��� ","�� ���е� �� ���ڿ��� ���� ����
	short	restIdx;				// ���ڰ� �ƴ� ���ڷ� �� �� ���ڿ� ��ư�� �ε���

	char	productNumStr[256];	// Edit ��Ʈ�ѷκ��� �Է¹��� "-"�� ���е� �� ���ڿ��� ���� ����
	bool	bFoundFirstNum;			// ���۵Ǵ� ���� ��ȣ�� ã�Ҵ��� ����
	bool	bFoundLastNum;			// ������ ���� ��ȣ�� ã�Ҵ��� ����
	char	firstNumStr[10], lastNumStr[10];		// ���۵Ǵ� ���� ��ȣ, ������ ���� ��ȣ

	char* token;			// �о�� ���ڿ��� ��ū
	bool	bFirstTok;		// ù��° ��ū�ΰ�?
	short	count;
	short	nButton;

	switch (message) {
	case DG_MSG_INIT:

		// ���̾�α� Ÿ��Ʋ
		DGSetDialogTitle(dialogID, L"���� ����");

		// Ȯ�� ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 10, 60, 25);
		DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_OK, L"Ȯ��");
		DGShowItem(dialogID, DG_OK);

		// ��� ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 60, 25);
		DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_CANCEL, L"���");
		DGShowItem(dialogID, DG_CANCEL);

		if (clickedBtnItemIdx == BUTTON_CODE) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_CODE, itmPosX, itmPosY, 90, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				sprintf(tempStr, "%s %s", layerInfo.code_name[xx].c_str(), layerInfo.code_desc[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				(layerInfo.code_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

				itmPosX += 100;
				if (itmPosX >= 500) {
					itmPosX = 90;
					itmPosY += 30;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_DONG) {
			itmPosX = 90;
			itmPosY = 10;

			// �� ��ȣ�� ������ �Է� ���� ��
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 300, 50);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itmIdx, L"�� ��ȣ [101~1599������ ����] (��: 0101)\n��, �� ���� ���� �� 0000");
			DGShowItem(dialogID, itmIdx);

			itmPosY += 45;

			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_TEXT, BUTTON_DONG, itmPosX, itmPosY, 90, 20);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itmIdx, L"�� ��ȣ �Է�");
			DGShowItem(dialogID, itmIdx);
			DONG_STRING_RADIOBUTTON = itmIdx;

			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_TEXT, 256, itmPosX + 90, itmPosY, 400, 20);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem(dialogID, itmIdx);
			DONG_STRING_EDITCONTROL = itmIdx;

			itmPosY += 30;

			// �̹� ��ϵ� �� ��ȣ ����ϱ�
			strcpy(dongStr, "");
			for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
				if (strlen(layerInfo.dong_name[xx].c_str()) == 3)
					sprintf(tempStr, "0%s", layerInfo.dong_name[xx].c_str());
				else if (strlen(layerInfo.dong_name[xx].c_str()) == 1)
					sprintf(tempStr, "000%s", layerInfo.dong_name[xx].c_str());
				else
					sprintf(tempStr, "%s", layerInfo.dong_name[xx].c_str());

				if (isStringDouble(tempStr) == TRUE) {
					if (layerInfo.dong_state[xx] == true) {
						strcat(dongStr, tempStr);
					}
				}
			}
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 500, 23);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itmIdx, charToWchar(dongStr));
			DGShowItem(dialogID, itmIdx);

			// �̹� ��ϵ� �� ��ȣ�� Edit ��Ʈ�ѿ��� �־�� ��
			DGSetItemText(dialogID, DONG_STRING_EDITCONTROL, charToWchar(dongStr));

			itmPosY += 40;

			// ���� �� ��ȣ�̸� �����ϰ�, �� ��ȣ�� �ƴϸ� ����� ��
			restIdx = 0;
			for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
				if (strlen(layerInfo.dong_name[xx].c_str()) == 3)
					sprintf(tempStr, "0%s", layerInfo.dong_name[xx].c_str());
				else if (strlen(layerInfo.dong_name[xx].c_str()) == 1)
					sprintf(tempStr, "000%s", layerInfo.dong_name[xx].c_str());
				else
					sprintf(tempStr, layerInfo.dong_name[xx].c_str());

				// �����̸� ���, �����̸� ��ŵ
				if (isStringDouble(tempStr) == FALSE) {
					itmIdx = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_DONG, itmPosX, itmPosY, 90, 35);
					DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					sprintf(tempStr, "%s\n%s", layerInfo.dong_name[xx].c_str(), layerInfo.dong_desc[xx].c_str());
					DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
					DGShowItem(dialogID, itmIdx);
					DONG_REST_BUTTONS[restIdx] = itmIdx;
					restIdx++;
					(layerInfo.dong_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

					itmPosX += 100;
					if (itmPosX >= 500) {
						itmPosX = 90;
						itmPosY += 40;
					}
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_FLOOR) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_FLOOR, itmPosX, itmPosY, 90, 35);
				DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				sprintf(tempStr, "%s\n%s", layerInfo.floor_name[xx].c_str(), layerInfo.floor_desc[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				(layerInfo.floor_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

				itmPosX += 100;
				if (itmPosX >= 950) {
					itmPosX = 90;
					itmPosY += 40;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_CAST) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_CAST, itmPosX, itmPosY, 35, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				sprintf(tempStr, "%s", layerInfo.cast_name[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				(layerInfo.cast_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

				itmPosX += 40;
				if (itmPosX >= 480) {
					itmPosX = 90;
					itmPosY += 30;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_CJ) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_CJ, itmPosX, itmPosY, 35, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				sprintf(tempStr, "%s", layerInfo.CJ_name[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				(layerInfo.CJ_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

				itmPosX += 40;
				if (itmPosX >= 480) {
					itmPosX = 90;
					itmPosY += 30;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_ORDER) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_ORDER, itmPosX, itmPosY, 35, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				sprintf(tempStr, "%s", layerInfo.orderInCJ_name[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				(layerInfo.orderInCJ_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

				itmPosX += 40;
				if (itmPosX >= 480) {
					itmPosX = 90;
					itmPosY += 30;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_OBJ) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < 100; ++xx) {
				OBJ_BUTTONS[xx] = 0;
			}
			count = 0;
			for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
				nButton = 0;
				for (yy = 0; yy < layerInfo.obj_name.size(); ++yy) {
					if (yy == 0) {
						itmPosY += 5;

						itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 100, 23);
						DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
						sprintf(tempStr, "%s %s", layerInfo.code_name[xx].c_str(), layerInfo.code_desc[xx].c_str());
						DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
						DGShowItem(dialogID, itmIdx);

						itmPosY += 23;
					}

					if (strncmp(layerInfo.code_name[xx].c_str(), layerInfo.obj_cat[yy].c_str(), 4) == 0) {
						itmIdx = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_OBJ, itmPosX, itmPosY, 95, 28);
						DGSetItemFont(dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
						sprintf(tempStr, "%s\n%s", layerInfo.obj_name[yy].c_str(), layerInfo.obj_desc[yy].c_str());
						DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
						DGShowItem(dialogID, itmIdx);
						OBJ_BUTTONS[count] = itmIdx;
						(layerInfo.obj_state[yy] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);
						count++;
						nButton++;

						itmPosX += 100;
						if (itmPosX >= 1100) {
							itmPosX = 90;
							itmPosY += 30;
						}
					}
				}
				itmPosX = 90;
				if (nButton == 0) {
					DGHideItem(dialogID, itmIdx);
					itmPosY -= 28;
				}
				else {
					itmPosY += 30;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_PRODUCT_SITE) {
			itmPosX = 90;
			itmPosY = 10;
			for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
				itmIdx = DGAppendDialogItem(dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_PRODUCT_SITE, itmPosX, itmPosY, 90, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				sprintf(tempStr, "%s", layerInfo.productSite_name[xx].c_str());
				DGSetItemText(dialogID, itmIdx, charToWchar(tempStr));
				DGShowItem(dialogID, itmIdx);
				(layerInfo.productSite_state[xx] == true) ? DGSetItemValLong(dialogID, itmIdx, TRUE) : DGSetItemValLong(dialogID, itmIdx, FALSE);

				itmPosX += 100;
				if (itmPosX >= 500) {
					itmPosX = 90;
					itmPosY += 30;
				}
			}
		}
		else if (clickedBtnItemIdx == BUTTON_PRODUCT_NUM) {
			itmPosX = 90;
			itmPosY = 10;

			// ���� ��ȣ�� ������ �Է� ���� ��
			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 400, 23);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itmIdx, L"���� ��ȣ [001~999���� ����] (��: 001,002,100)");
			DGShowItem(dialogID, itmIdx);

			itmPosY += 30;

			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 90, 20);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itmIdx, L"���� ��ȣ");
			DGShowItem(dialogID, itmIdx);
			PRODUCT_NUM_STRING_RADIOBUTTON = itmIdx;

			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_TEXT, 256, itmPosX + 90, itmPosY - 5, 400, 20);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem(dialogID, itmIdx);
			PRODUCT_NUM_STRING_EDITCONTROL = itmIdx;

			itmPosY += 30;

			// �̹� ��ϵ� ���� ��ȣ ����ϱ�
			bFoundFirstNum = false;
			bFoundLastNum = false;
			firstNumStr[0] = '\0';
			lastNumStr[0] = '\0';
			strcpy(productNumStr, "");
			for (xx = 0; xx < (layerInfo.productNum_name.size() - 1); ++xx) {
				if (layerInfo.productNum_state[xx] == true) {
					// ó�� ã�� ���� ��ȣ�� ���۵Ǵ� ���� ��ȣ�� ������
					if ((bFoundFirstNum == false) && (firstNumStr[0] == '\0')) {
						strcpy(firstNumStr, layerInfo.productNum_name[xx].c_str());
						bFoundFirstNum = true;
					}

					// �������� ���� ��ȣ �� ���������� �߰ߵ� ���� ��ȣ�� ������ ���� ��ȣ�� ������
					if ((bFoundLastNum == false) && (bFoundFirstNum == true) && (layerInfo.productNum_state[xx + 1] == false)) {
						strcpy(lastNumStr, layerInfo.productNum_name[xx].c_str());
						bFoundLastNum = true;
					}
				}
			}

			strcpy(productNumStr, "");
			if (bFoundFirstNum == true) {
				if (strlen(firstNumStr) == 3) {
					sprintf(tempStr, "%s", firstNumStr);
				}
				else if (strlen(firstNumStr) == 2) {
					sprintf(tempStr, "0%s", firstNumStr);
				}
				else if (strlen(firstNumStr) == 1) {
					sprintf(tempStr, "00%s", firstNumStr);
				}
				strcat(productNumStr, tempStr);
			}
			if ((bFoundFirstNum == true) || (bFoundLastNum == true))
				strcat(productNumStr, "-");
			if (bFoundLastNum == true) {
				if (strlen(lastNumStr) == 3) {
					sprintf(tempStr, "%s", lastNumStr);
				}
				else if (strlen(lastNumStr) == 2) {
					sprintf(tempStr, "0%s", lastNumStr);
				}
				else if (strlen(lastNumStr) == 1) {
					sprintf(tempStr, "00%s", lastNumStr);
				}
				strcat(productNumStr, tempStr);
			}

			itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 500, 23);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText(dialogID, itmIdx, charToWchar(firstNumStr));
			DGShowItem(dialogID, itmIdx);

			// �̹� ��ϵ� ���� ��ȣ�� Edit ��Ʈ�ѿ��� �־�� ��
			DGSetItemText(dialogID, PRODUCT_NUM_STRING_EDITCONTROL, charToWchar(firstNumStr));
		}

		break;

	case DG_MSG_CLICK:
		switch (item) {
		case DG_OK:

			if (clickedBtnItemIdx == BUTTON_CODE) {
				for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, 3 + xx) == TRUE)
						layerInfo.code_state[xx] = true;
					else
						layerInfo.code_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_DONG) {
				// �ϴ� false�� �ʱ�ȭ
				for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
					layerInfo.dong_state[xx] = false;
				}

				// �� ��ȣ �Է��� ������ ���
				if (DGGetItemValLong(dialogID, DONG_STRING_RADIOBUTTON) == TRUE) {
					bFirstTok = true;

					// DONG_STRING_EDITCONTROL ���ڿ��� ","�� �߶� �ش� �� ��ȣ�� ã�Ƽ� true
					strcpy(dongStr, DGGetItemText(dialogID, DONG_STRING_EDITCONTROL).ToCStr().Get());
					token = strtok(dongStr, ",");
					while (token != NULL) {
						if (bFirstTok == true) {
							// ���ڰ� �´ٸ�,
							if (isStringDouble(token) == TRUE) {
								for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
									if (strncmp(layerInfo.dong_name[xx].c_str(), token, 4) == 0) {
										layerInfo.dong_state[xx] = true;
									}
								}
							}
						}

						token = strtok(NULL, ",");
						bFirstTok = false;
					}
				}

				// ���� �� ��ȣ�̸� �����ϰ�, �� ��ȣ�� �ƴϸ� ���¸� ������ ��
				restIdx = 0;
				for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
					if (strlen(layerInfo.dong_name[xx].c_str()) != 4)
						sprintf(tempStr, "0%s", layerInfo.dong_name[xx].c_str());
					else
						sprintf(tempStr, "%s", layerInfo.dong_name[xx].c_str());

					// �����̸� ���, �����̸� ��ŵ
					if (isStringDouble(tempStr) == FALSE) {
						if (DGGetItemValLong(dialogID, DONG_REST_BUTTONS[restIdx]) == TRUE) {
							layerInfo.dong_state[xx] = true;
						}
						else {
							layerInfo.dong_state[xx] = false;
						}

						restIdx++;
					}
				}
			}
			else if (clickedBtnItemIdx == BUTTON_FLOOR) {
				for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, 3 + xx) == TRUE)
						layerInfo.floor_state[xx] = true;
					else
						layerInfo.floor_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_CAST) {
				for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, 3 + xx) == TRUE)
						layerInfo.cast_state[xx] = true;
					else
						layerInfo.cast_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_CJ) {
				for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, 3 + xx) == TRUE)
						layerInfo.CJ_state[xx] = true;
					else
						layerInfo.CJ_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_ORDER) {
				for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, 3 + xx) == TRUE)
						layerInfo.orderInCJ_state[xx] = true;
					else
						layerInfo.orderInCJ_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_OBJ) {
				for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, OBJ_BUTTONS[xx]) == TRUE)
						layerInfo.obj_state[xx] = true;
					else
						layerInfo.obj_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_PRODUCT_SITE) {
				for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
					if (DGGetItemValLong(dialogID, 3 + xx) == TRUE)
						layerInfo.productSite_state[xx] = true;
					else
						layerInfo.productSite_state[xx] = false;
				}
			}
			else if (clickedBtnItemIdx == BUTTON_PRODUCT_NUM) {
				// �ϴ� false�� �ʱ�ȭ
				for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
					layerInfo.productNum_state[xx] = false;
				}

				// PRODUCT_NUM_STRING_EDITCONTROL ���ڿ�(��ȣ)�� ���� ��ȣ�� ������
				bFirstTok = true;

				strcpy(productNumStr, DGGetItemText(dialogID, PRODUCT_NUM_STRING_EDITCONTROL).ToCStr().Get());
				token = strtok(productNumStr, "-");
				while (token != NULL) {
					if (bFirstTok == true) {
						// ���ڰ� �´ٸ�,
						if (isStringDouble(token) == TRUE) {
							// ��ȯ�� ���� ���� ���� ��ȣ�� ��
							if (strlen(token) == 3) {
								sprintf(firstNumStr, "%s", token);
							}
							else if (strlen(token) == 2) {
								sprintf(firstNumStr, "0%s", token);
							}
							else if (strlen(token) == 1) {
								sprintf(firstNumStr, "00%s", token);
							}
						}
					}

					token = strtok(NULL, "-");
					bFirstTok = false;
				}

				for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
					if (atoi(layerInfo.productNum_name[xx].c_str()) == atoi(firstNumStr)) {
						layerInfo.productNum_state[xx] = true;
					}
				}
			}

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

// �ֱ� ��ư ���� �����ϱ� (���̾� ���� �����ϱ�)
GSErrCode	saveButtonStatus_assign(void)
{
	GSErrCode err = NoError;

	short	xx;
	API_ModulData	info;
	BNZeroMemory(&info, sizeof(API_ModulData));
	info.dataVersion = 1;
	info.platformSign = GS::Act_Platform_Sign;
	info.dataHdl = BMAllocateHandle(sizeof(selectedInfoSaved), 0, 0);
	if (info.dataHdl != NULL) {

		for (xx = 0; xx < layerInfo.code_name.size(); ++xx)
			selectedInfoSaved.code_state[xx] = layerInfo.code_state[xx];
		for (xx = 0; xx < layerInfo.dong_name.size(); ++xx)
			selectedInfoSaved.dong_state[xx] = layerInfo.dong_state[xx];
		for (xx = 0; xx < layerInfo.floor_name.size(); ++xx)
			selectedInfoSaved.floor_state[xx] = layerInfo.floor_state[xx];
		for (xx = 0; xx < layerInfo.cast_name.size(); ++xx)
			selectedInfoSaved.cast_state[xx] = layerInfo.cast_state[xx];
		for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx)
			selectedInfoSaved.CJ_state[xx] = layerInfo.CJ_state[xx];
		for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx)
			selectedInfoSaved.orderInCJ_state[xx] = layerInfo.orderInCJ_state[xx];
		for (xx = 0; xx < layerInfo.obj_name.size(); ++xx)
			selectedInfoSaved.obj_state[xx] = layerInfo.obj_state[xx];
		for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx)
			selectedInfoSaved.productSite_state[xx] = layerInfo.productSite_state[xx];
		for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx)
			selectedInfoSaved.productNum_state[xx] = layerInfo.productNum_state[xx];

		*(reinterpret_cast<StatusOfLayerNameSystem*> (*info.dataHdl)) = selectedInfoSaved;
		err = ACAPI_ModulData_Store(&info, "ButtonStatus_assign");
		BMKillHandle(&info.dataHdl);
	}
	else {
		err = APIERR_MEMFULL;
	}

	return	err;
}

// ���̾� �̸� �˻��ϱ�
GSErrCode	inspectLayerNames(void)
{
	GSErrCode	err = NoError;

	FILE* fp;			// ���� ������
	char	line[20480];	// ���Ͽ��� �о�� ���� �ϳ�
	char	buffer[256];	// ���̾� �̸��� ������ ����
	string	insElem;		// ��ū�� string���� ��ȯ�ؼ� vector�� ����
	string	insElem2;
	char* token;			// �о�� ���ڿ��� ��ū

	API_Attribute	attrib;
	short	xx, yy, i;
	short	nLayers;

	short	nBaseFields;
	short	nExtendFields;
	bool	success;
	bool	extSuccess;
	char	tempStr[128];
	char	tok1[32];
	char	tok2[32];
	char	tok3[32];
	char	tok4[32];
	char	tok5[32];
	char	tok6[32];
	char	tok7[32];
	char	tok8[32];
	char	tok9[32];
	char	tok10[32];
	char	constructionCode[8];

	bool	bSkip;				// ������ ���̾��ΰ�?
	bool	bValidLayerName;	// ��ȿ�� ���̾� �̸��ΰ�?
	short	nValidCountBase;
	short	nValidCountExtend;

	short	result;


	// �׷�ȭ �Ͻ����� ON
	suspendGroups(true);

	// ���̾� ���� ���� ��������
	importLayerInfo(&layerInfo);

	// ����ü �ʱ�ȭ
	allocateMemory(&layerInfo);

	short	z;
	char	code1[25][32];		// ���� �ڵ�
	short	LenCode1;
	char	code2[1600][32];	// �� �ڵ�
	short	LenCode2;
	char	code3[120][32];	// �� �ڵ�
	short	LenCode3;
	char	code4[120][32];	// Ÿ����ȣ �ڵ�
	short	LenCode4;
	char	code5[120][32];	// CJ �ڵ�
	short	LenCode5;
	char	code6[120][32];	// CJ �� �ð����� �ڵ�
	short	LenCode6;
	char	code7[120][32];	// ���� �ڵ�
	short	LenCode7;
	char	code8[10][32];		// ����ó �ڵ�
	short	LenCode8;
	char	code9[1000][32];	// ���۹�ȣ �ڵ�
	short	LenCode9;

	vector<string>	exceptionLayerNames;	// ���� ó���� ���̾� �̸� ���
	vector<string>	invalidLayerNames;		// ��ȿ���� ���� ���̾� �̸� ���


	// 1. ���� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.code_name.size(); ++xx) {
		sprintf(tempStr, "%s", layerInfo.code_name[xx].c_str());
		strcpy(code1[z++], tempStr);
	}
	LenCode1 = z;

	// 2. �� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.dong_name.size(); ++xx) {
		sprintf(tempStr, "%s", layerInfo.dong_name[xx].c_str());
		strcpy(code2[z++], tempStr);
	}
	LenCode2 = z;

	// 3. �� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.floor_name.size(); ++xx) {
		sprintf(tempStr, "%s", layerInfo.floor_name[xx].c_str());
		strcpy(code3[z++], tempStr);
	}
	LenCode3 = z;

	// 4. Ÿ����ȣ �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.cast_name.size(); ++xx) {
		sprintf(tempStr, "%s", layerInfo.cast_name[xx].c_str());
		strcpy(code4[z++], tempStr);
	}
	LenCode4 = z;

	// 5. CJ �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.CJ_name.size(); ++xx) {
		sprintf(tempStr, "%s", layerInfo.CJ_name[xx].c_str());
		strcpy(code5[z++], tempStr);
	}
	LenCode5 = z;

	// 6. CJ �� �ð����� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.orderInCJ_name.size(); ++xx) {
		sprintf(tempStr, "%s", layerInfo.orderInCJ_name[xx].c_str());
		strcpy(code6[z++], tempStr);
	}
	LenCode6 = z;

	// 7. ���� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.obj_name.size(); ++xx) {
		sprintf(tempStr, "%s", layerInfo.obj_name[xx].c_str());
		strcpy(code7[z++], tempStr);
	}
	LenCode7 = z;

	// 8. ����ó ���� ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.productSite_name.size(); ++xx) {
		sprintf(tempStr, "%s", layerInfo.productSite_name[xx].c_str());
		strcpy(code8[z++], tempStr);
	}
	LenCode8 = z;

	// 9. ���� ��ȣ ���ڿ� �����
	z = 0;
	for (xx = 0; xx < layerInfo.productNum_name.size(); ++xx) {
		sprintf(tempStr, "%s", layerInfo.productNum_name[xx].c_str());
		strcpy(code9[z++], tempStr);
	}
	LenCode9 = z;

	// ���� �̸� ���̾� �̸� �߰�
	strcpy(line, "");
	insElem = "";
	insElem2 = "";
	fp = fopen("C:\\exceptionLayer.csv", "r");
	if (fp != NULL) {
		do {
			fgets(line, 256, fp);
			insElem = line;
			if (insElem.compare(insElem2) == 0)
				break;
			exceptionLayerNames.push_back(insElem);
			insElem2 = insElem;
		} while (strlen(line) > 2);
	}
	fclose(fp);

	// ���̾� �̸� �����ϱ�
	nLayers = getLayerCount();

	for (xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.header.typeID = API_LayerID;
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			strcpy(tok1, "");
			strcpy(tok2, "");
			strcpy(tok3, "");
			strcpy(tok4, "");
			strcpy(tok5, "");
			strcpy(tok6, "");
			strcpy(tok7, "");
			strcpy(tok8, "");
			strcpy(tok9, "");
			i = 1;
			success = false;
			extSuccess = false;
			nBaseFields = 0;
			nExtendFields = 0;
			// ����(�⺻): 05-T-0000-F01-01-01-01-WALL
			// ����(Ȯ��): 05-T-0000-F01-01-01-01-WALL-��������-001
			// ���̾� �̸��� "-" ���� �������� �ɰ���
			strcpy(buffer, attrib.layer.head.name);
			token = strtok(buffer, "-");
			while (token != NULL) {
				// ���� �� ���� Ȯ��
				// 1�� (�Ϸù�ȣ) - �ʼ� (2����, ����)
				if (i == 1) {
					strcpy(tempStr, token);
					if (strlen(tempStr) == 2) {
						strcpy(tok1, tempStr);
						success = true;
						nBaseFields++;
					}
					else {
						i = 100;
						success = false;
					}
				}
				// 2�� (���� ����) - �ʼ� (1����, ����)
				else if (i == 2) {
					strcpy(tempStr, token);
					if (strlen(tempStr) == 1) {
						strcpy(tok2, tempStr);
						success = true;
						nBaseFields++;
					}
					else {
						i = 100;
						success = false;
					}
				}
				// 3�� (�� ����) - �ʼ� (4����)
				else if (i == 3) {
					strcpy(tempStr, token);
					if (isStringDouble(tempStr) == TRUE) {
						// ������ ���
						sprintf(tok3, "%04d", atoi(tempStr));
					}
					else {
						// ���ڿ��� ���
						strcpy(tok3, tempStr);
					}
					success = true;
					nBaseFields++;
				}
				// 4�� (�� ����) - �ʼ� (3����)
				else if (i == 4) {
					strcpy(tempStr, token);
					if (strlen(tempStr) == 3) {
						strcpy(tok4, tempStr);
						success = true;
						nBaseFields++;
					}
					else {
						i = 100;
						success = false;
					}
				}
				// 5�� (Ÿ����ȣ) - �ʼ� (2����, ����)
				else if (i == 5) {
					strcpy(tempStr, token);
					if (isStringDouble(tempStr) == TRUE) {
						// ������ ���
						sprintf(tok5, "%02d", atoi(tempStr));
					}
					else {
						// ���ڿ��� ���
						strcpy(tok5, tempStr);
					}
					success = true;
					nBaseFields++;
				}
				// 6�� (CJ ����) - �ʼ� (2����, ����)
				else if (i == 6) {
					strcpy(tempStr, token);
					if (isStringDouble(tempStr) == TRUE) {
						// ������ ���
						sprintf(tok6, "%02d", atoi(tempStr));
					}
					else {
						// ���ڿ��� ���
						strcpy(tok6, tempStr);
					}
					success = true;
					nBaseFields++;
				}
				// 7�� (CJ �� �ð�����) - �ʼ� (2����, ����)
				else if (i == 7) {
					strcpy(tempStr, token);
					if (isStringDouble(tempStr) == TRUE) {
						// ������ ���
						sprintf(tok7, "%02d", atoi(tempStr));
					}
					else {
						// ���ڿ��� ���
						strcpy(tok7, tempStr);
					}
					success = true;
					nBaseFields++;
				}
				// 8�� (���� ����) - �ʼ� (3���� �̻�)
				else if (i == 8) {
					strcpy(tempStr, token);
					if (strlen(tempStr) >= 3) {
						strcpy(tok8, tempStr);
						success = true;
						nBaseFields++;
					}
					else {
						i = 100;
						success = false;
					}
				}
				// 9�� (����ó ����) - ���� (�ѱ� 4����..)
				else if (i == 9) {
					strcpy(tempStr, token);
					if (strlen(tempStr) >= 4) {
						strcpy(tok9, tempStr);
						extSuccess = true;
						nExtendFields++;
					}
					else {
						i = 100;
						extSuccess = false;
					}
				}
				// 10�� (���� ��ȣ) - �ʼ� (3����, ����)
				else if (i == 10) {
					strcpy(tempStr, token);
					if (isStringDouble(tempStr) == TRUE) {
						// ������ ���
						sprintf(tok10, "%03d", atoi(tempStr));
					}
					else {
						// ���ڿ��� ���
						strcpy(tok10, tempStr);
					}
					extSuccess = true;
					nExtendFields++;
				}
				++i;
				token = strtok(NULL, "-");
			}

			// ���̾� �̸��� ���� �̸��� ��ġ�� ��� ����
			bSkip = false;
			for (yy = 0; yy < exceptionLayerNames.size(); ++yy) {
				insElem = attrib.layer.head.name;
				if (strncmp(insElem.c_str(), exceptionLayerNames.at(yy).c_str(), strlen(insElem.c_str())) == 0) {
					bSkip = true;
				}
			}

			// 8�ܰ� �Ǵ� 10�ܰ���� ���������� �Ϸ�Ǹ�
			bValidLayerName = false;
			nValidCountBase = 0;
			nValidCountExtend = 0;
			if ((success == true) && (nBaseFields == 8)) {
				// �Ϸ� ��ȣ�� ���� ���� ���ڸ� ��ħ
				sprintf(constructionCode, "%s-%s", tok1, tok2);

				// �⺻��
				for (yy = 0; yy < layerInfo.code_name.size(); ++yy) {
					if (my_strcmp(layerInfo.code_name.at(yy).c_str(), constructionCode) == 0)
						++nValidCountBase;
				}
				for (yy = 0; yy < layerInfo.dong_name.size(); ++yy) {
					if (my_strcmp(layerInfo.dong_name.at(yy).c_str(), tok3) == 0) {
						++nValidCountBase;
					}
				}
				for (yy = 0; yy < layerInfo.floor_name.size(); ++yy) {
					if (my_strcmp(layerInfo.floor_name.at(yy).c_str(), tok4) == 0)
						++nValidCountBase;
				}
				for (yy = 0; yy < layerInfo.cast_name.size(); ++yy) {
					if (my_strcmp(layerInfo.cast_name.at(yy).c_str(), tok5) == 0)
						++nValidCountBase;
				}
				for (yy = 0; yy < layerInfo.CJ_name.size(); ++yy) {
					if (my_strcmp(layerInfo.CJ_name.at(yy).c_str(), tok6) == 0)
						++nValidCountBase;
				}
				for (yy = 0; yy < layerInfo.orderInCJ_name.size(); ++yy) {
					if (my_strcmp(layerInfo.orderInCJ_name.at(yy).c_str(), tok7) == 0)
						++nValidCountBase;
				}
				for (yy = 0; yy < layerInfo.obj_name.size(); ++yy) {
					if (my_strcmp(layerInfo.obj_name.at(yy).c_str(), tok8) == 0)
						++nValidCountBase;
				}

				if ((extSuccess == true) && (nExtendFields == 2)) {
					// Ȯ����
					for (yy = 0; yy < layerInfo.productSite_name.size(); ++yy) {
						if (my_strcmp(layerInfo.productSite_name.at(yy).c_str(), tok9) == 0)
							++nValidCountExtend;
					}
					for (yy = 0; yy < layerInfo.productNum_name.size(); ++yy) {
						if (my_strcmp(layerInfo.productNum_name.at(yy).c_str(), tok10) == 0)
							++nValidCountExtend;
					}
				}

				// ��ȿ�� ���̾� �̸��ΰ�?
				if (((extSuccess == false) && (nValidCountBase == 7) && (nValidCountExtend == 0)) || ((extSuccess == true) && (nValidCountBase == 7) && (nValidCountExtend == 2)))
					bValidLayerName = true;
			}

			// ��ȿ���� ���� ���̾� �̸��� ���� ������
			if ((bValidLayerName == false) && (bSkip == false)) {
				insElem = attrib.layer.head.name;
				invalidLayerNames.push_back(insElem);
			}
		}
		if (err == APIERR_DELETED)
			err = NoError;
	}

	// [���̾�α� �ڽ�] ���̾� ���� �����ϱ�
	result = DGBlankModalDialog(500, 450, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerNameInspectionHandler, (DGUserData)&invalidLayerNames);

	// OK ��ư�� �ƴϸ� �޸� �����ϰ� ����
	if (result != DG_OK) {
		deallocateMemory(&layerInfo);
		return	err;
	}

	// �޸� ����
	deallocateMemory(&layerInfo);

	// �׷�ȭ �Ͻ����� OFF
	suspendGroups(false);

	return	err;
}

// [���̾�α� �ڽ�] ���̾� �̸� �˻��ϱ�
short DGCALLBACK layerNameInspectionHandler(short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	vector<string>* invalidLayerNames = (vector<string> *) userData;
	short	result;
	short	itmIdx;
	short	xx;

	switch (message) {
	case DG_MSG_INIT:
		// ���̾�α� Ÿ��Ʋ
		DGSetDialogTitle(dialogID, L"���̾� �̸� �˻��ϱ�");

		// Ȯ�� ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 150 + 60, 400, 70, 25);
		DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_OK, L"Ȯ��");
		DGShowItem(dialogID, DG_OK);

		// ��� ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 280, 400, 70, 25);
		DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_CANCEL, L"���");
		DGHideItem(dialogID, DG_CANCEL);

		// ��: �ȳ� ����
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 10, 460, 50);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, itmIdx, L"MaxBIM ���̾� ü�迡 ���� �ʴ� ��ȿ���� ���� ���̾� ����Դϴ�.\n��, exceptionLayer�� ���Ե� �̸����� ���ܵ˴ϴ�.");
		DGShowItem(dialogID, itmIdx);

		// ����Ʈ ��: ��ȿ���� ���� ���̾� �̸� ���
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_LISTVIEW, DG_LVT_SINGLESELECT, DG_LVVM_SINGLECOLUMN, 20, 70, 460, 300);
		DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
		for (xx = 0; xx < invalidLayerNames->size(); ++xx) {
			DGListViewInsertItem(dialogID, itmIdx, DG_LIST_BOTTOM);
			DGListViewSetItemText(dialogID, itmIdx, DG_LIST_BOTTOM, invalidLayerNames->at(xx).c_str());
		}
		DGShowItem(dialogID, itmIdx);

		break;

	case DG_MSG_CHANGE:
		break;

	case DG_MSG_CLICK:
		switch (item) {
		case DG_OK:
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