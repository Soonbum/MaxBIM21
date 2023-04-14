#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "MaxBIM21.h"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.h"
#include "Export.h"

using namespace exportDG;

VisibleObjectInfo	visibleObjInfo;		// ���̴� ���̾� ���� ��ü�� ��Ī, ���� ����, ���̱� ����
short	EDITCONTROL_SCALE_VALUE;		// ��ô ���� �Է��ϴ� Edit��Ʈ���� ID ��
short	EDITCONTROL_FILENAME;			// ���ϸ��� �Է��ϴ� Edit��Ʈ���� ID ��


// �迭 �ʱ�ȭ �Լ�
void initArray (double arr [], short arrSize)
{
	short	xx;

	for (xx = 0 ; xx < arrSize ; ++xx)
		arr [xx] = 0.0;
}

// ������������ ������ �� ����ϴ� ���Լ� (����Ʈ)
int compare (const void* first, const void* second)
{
    if (*(double*)first > *(double*)second)
        return 1;
    else if (*(double*)first < *(double*)second)
        return -1;
    else
        return 0;
}

// vector �� ���� ���� ����ü ������ ���� �� �Լ� (��ǥ�� X ����)
bool comparePosX (const objectInBeamTableform& a, const objectInBeamTableform& b)
{
	return	(a.minPos.x + (a.maxPos.x - a.minPos.x)/2) < (b.minPos.x + (b.maxPos.x - b.minPos.x)/2);
}

// vector �� ���� ���� ����ü ������ ���� �� �Լ� (��ǥ�� Y ����)
bool comparePosY (const objectInBeamTableform& a, const objectInBeamTableform& b)
{
	return	(a.minPos.y + (a.maxPos.y - a.minPos.y)/2) < (b.minPos.y + (b.maxPos.y - b.minPos.y)/2);
}

// vector �� ���̾� ���� ����ü ������ ���� �� �Լ� (���̾� �̸� ����)
bool compareLayerName (const LayerList& a, const LayerList& b)
{
	if (a.layerName.compare (b.layerName) < 0)
		return true;
	else
		return false;
}

// vector �� ���ڵ� �� �ʵ带 �������� �������� ������ ���� �� �Լ�
bool compareVectorString (const vector<string>& a, const vector<string>& b)
{
	int	xx;
	size_t	aLen = a.size ();
	size_t	bLen = b.size ();

	const char* aStr;
	const char* bStr;

	// a�� b�� 0��° ���ڿ��� ����, ������ ���̵� ���� ��� ��� ����
	if (aLen == bLen) {
		// 1��° ���ڿ����� n��° ���ڿ����� ���������� ����
		for (xx = 0 ; xx < aLen ; ++xx) {
			aStr = a.at(xx).c_str ();
			bStr = b.at(xx).c_str ();

			// ���ڷ� �� ���ڿ��̸�,
			if ((isStringDouble (aStr) == TRUE) && (isStringDouble (bStr) == TRUE)) {
				if (atof (aStr) - atof (bStr) > EPS)
					return true;
				else if (atof (bStr) - atof (aStr) > EPS)
					return false;
				else
					continue;
			} else {
				if (my_strcmp (aStr, bStr) > 0)
					return true;
				else if (my_strcmp (aStr, bStr) < 0)
					return false;
				else
					continue;
			}
		}
	} else {
		if (aLen > bLen)
			return true;
		else
			return false;
	}

	return	true;
}

// ������ ������� ��� ����: ������
SummaryOfObjectInfo::SummaryOfObjectInfo ()
{
	FILE	*fp;			// ���� ������
	char	line [2048];	// ���Ͽ��� �о�� ���� �ϳ�
	char	*token;			// �о�� ���ڿ��� ��ū
	short	lineCount;		// �о�� ���� ��
	short	tokCount;		// �о�� ��ū ����
	short	xx;
	int		count;

	char	nthToken [200][256];	// n��° ��ū

	// ��ü ���� ���� ��������
	fp = fopen ("C:\\objectInfo.csv", "r");

	if (fp == NULL) {
		DGAlert (DG_ERROR, L"����", L"objectInfo.csv ������ C:\\�� �����Ͻʽÿ�.", "", L"Ȯ��", "", "");
	} else {
		lineCount = 0;

		while (!feof (fp)) {
			tokCount = 0;
			fgets (line, sizeof(line), fp);

			token = strtok (line, ",");
			tokCount ++;
			lineCount ++;

			// �� ���ξ� ó��
			while (token != NULL) {
				if (strlen (token) > 0) {
					strncpy (nthToken [tokCount-1], token, strlen (token)+1);
				}
				token = strtok (NULL, ",");
				tokCount ++;
			}

			// ��ū ������ 2�� �̻��� �� ��ȿ��
			if ((tokCount-1) >= 2) {
				// ��ū ������ 2 + 3�� ��� ������ ����� (�ʰ��� �׸��� ����)
				if (((tokCount-1) - 2) % 3 != 0) {
					tokCount --;
				}

				this->keyName.push_back (nthToken [0]);		// ��: u_comp
				this->keyDesc.push_back (nthToken [1]);		// ��: ������
				count = atoi (nthToken [2]);
				this->nInfo.push_back (count);				// ��: 5

				vector<string>	varNames;	// �ش� ��ü�� ���� �̸���
				vector<string>	varDescs;	// �ش� ��ü�� ���� �̸��� ���� ������

				for (xx = 1 ; xx <= count ; ++xx) {
					varNames.push_back (nthToken [1 + xx*2]);
					varDescs.push_back (nthToken [1 + xx*2 + 1]);
				}

				this->varName.push_back (varNames);
				this->varDesc.push_back (varDescs);
			}
		}

		// ���� �ݱ�
		fclose (fp);

		// ��ü ���� ����
		this->nKnownObjects = lineCount - 1;
		this->nUnknownObjects = 0;
	}
}

// ��ü�� ���ڵ� ���� 1 ���� (������ ����, ������ �ű� �߰�)
int	SummaryOfObjectInfo::quantityPlus1 (vector<string> record)
{
	int		xx, yy;
	size_t	vecLen;
	size_t	inVecLen1, inVecLen2;
	int		diff;
	int		value;
	char	tempStr [512];

	vecLen = this->records.size ();

	try {
		for (xx = 0 ; xx < vecLen ; ++xx) {
			// ���� ���� ������ ���
			inVecLen1 = this->records.at(xx).size () - 1;		// ���� ���� �ʵ带 ������ ����
			inVecLen2 = record.size ();

			if (inVecLen1 == inVecLen2) {
				// ��ġ���� �ʴ� �ʵ尡 �ϳ��� �ִ��� ã�ƺ� ��
				diff = 0;
				for (yy = 0 ; yy < inVecLen1 ; ++yy) {
					if (my_strcmp (this->records.at(xx).at(yy).c_str (), record.at(yy).c_str ()) != 0)
						diff++;
				}

				// ��� �ʵ尡 ��ġ�ϸ�
				if (diff == 0) {
					value = atoi (this->records.at(xx).back ().c_str ());
					value ++;
					sprintf (tempStr, "%d", value);
					this->records.at(xx).pop_back ();
					this->records.at(xx).push_back (tempStr);
					return value;
				}
			}
		}
	} catch (exception& ex) {
		WriteReport_Alert ("quantityPlus1 �Լ����� ���� �߻�: %s", ex.what ());
	}

	// ������ �ű� ���ڵ� �߰��ϰ� 1 ����
	record.push_back ("1");
	this->records.push_back (record);

	return 1;
}

// ���ڵ� ���� �����
void SummaryOfObjectInfo::clear ()
{
	unsigned int xx;
	
	try {
		for (xx = 0 ; xx < this->records.size () ; ++xx) {
			this->records.at(xx).clear ();
		}
	} catch (exception& ex) {
		WriteReport_Alert ("clear �Լ����� ���� �߻�: %s", ex.what ());
	}
	this->records.clear ();
}

// ������ ���� ���� �������� (Single ���)
GSErrCode	exportSelectedElementInfo (void)
{
	GSErrCode	err = NoError;
	short		result;
	unsigned short		xx, yy, zz;
	//bool		regenerate = true;
	
	// ������ ��Ұ� ������ ����
	GS::Array<API_Guid>		objects;
	//GS::Array<API_Guid>		beams;
	long					nObjects = 0;
	//long					nBeams = 0;

	// ������ ��ҵ��� ���� ����ϱ�
	API_Element			elem;
	API_ElementMemo		memo;
	SummaryOfObjectInfo	objectInfo;

	char			buffer [512];
	char			tempStr [512];
	const char*		foundStr;
	bool			foundObject;
	bool			foundExistValue;
	int				retVal;
	int				nInfo;
	API_AddParID	varType;
	vector<string>	record;

	// GS::Array �ݺ���
	GS::Array<API_Guid>::Iterator	iterObj;
	API_Guid	curGuid;

	// Single ��� ����
	vector<vector<string>>	plywoodInfo;	// ���� ������ ����
	vector<vector<string>>	darukiInfo;		// ���� ������ ���� (���ǿ� ������ ����Ʋ��)
	char*	token;
	char	infoText [256];


	// �׷�ȭ �Ͻ����� ON
	suspendGroups (true);

	// ������ ��� ��������
	if (getGuidsOfSelection (&objects, API_ObjectID, &nObjects) != NoError) {
		DGAlert (DG_ERROR, L"����", L"��ҵ��� �����ؾ� �մϴ�.", "", L"Ȯ��", "", "");
		return err;
	}

	// ���� ���Ϸ� ��� ���� ��������
	// ���� ������ ���� ����
	FILE				*fp;
	FILE				*fp_interReport;

	GS::UniString		inputFilename;
	GS::UniString		madeFilename;
	char				filename[256];
	result = DGBlankModalDialog (300, 150, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, filenameQuestionHandler, (DGUserData) &inputFilename);

	// !!! ���ϸ� �ѱ��� �ȵ�
	madeFilename = inputFilename + GS::UniString(".csv");
	strcpy(filename, madeFilename.ToCStr().Get());
	fp = fopen (filename, "w+");

	madeFilename = inputFilename + " (REPORT).txt";
	strcpy(filename, madeFilename.ToCStr().Get());
	fp_interReport = fopen (filename, "w+");

	if ((fp == NULL) || (fp_interReport == NULL)) {
		DGAlert (DG_ERROR, L"����", L"������ �� �� �����ϴ�.", "", L"Ȯ��", "", "");
		return err;
	}

	iterObj = objects.Enumerate ();

	while (iterObj != NULL) {
		foundObject = false;

		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		curGuid = *iterObj;
		elem.header.guid = curGuid;
		err = ACAPI_Element_Get (&elem);

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (err == NoError) {
				// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
				ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);
				bool	bForce = true;
				ACAPI_Database (APIDb_RefreshElementID, &elem.header, &bForce);

				try {
					for (yy = 0 ; yy < objectInfo.keyName.size () ; ++yy) {
						strcpy (tempStr, objectInfo.keyName.at(yy).c_str ());
						foundStr = getParameterStringByName (&memo, tempStr);

						// ��ü ������ ã�Ҵٸ�,
						if (my_strcmp (foundStr, "") != 0) {
							// !!!
							//retVal = objectInfo.keyDesc.at(yy).compare(foundStr);
							retVal = convertStr(objectInfo.keyDesc.at(yy).c_str()).Compare(foundStr);

							if (retVal == true) {
								foundObject = true;
								foundExistValue = false;

								// �߰��� ��ü�� �����͸� ������� ���ڵ� �߰�
								if (!record.empty ())
									record.clear ();

								record.push_back (objectInfo.keyDesc.at(yy));		// ��ü �̸�
								nInfo = objectInfo.nInfo.at(yy);
								for (zz = 0 ; zz < nInfo ; ++zz) {
									sprintf (buffer, "%s", objectInfo.varName.at(yy).at(zz).c_str ());
									varType = getParameterTypeByName (&memo, buffer);

									if ((varType != APIParT_Separator) || (varType != APIParT_Title) || (varType != API_ZombieParT)) {
										if (varType == APIParT_CString)
											sprintf (tempStr, "%s", getParameterStringByName (&memo, buffer));	// ���ڿ�
										else
											sprintf (tempStr, "%.3f", getParameterValueByName (&memo, buffer));	// ����
									}
									record.push_back (tempStr);		// ������
								}

								objectInfo.quantityPlus1 (record);
							}
						}
					}
				} catch (exception& ex) {
					WriteReport_Alert ("��ü ���� �������� ���� �߻�: %s", ex.what ());
				}

				// ���� ã�� ���ϸ� �� �� ���� ��ü�� �����
				if (foundObject == false)
					objectInfo.nUnknownObjects ++;
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}

		++iterObj;
	}

	// �� ���� ����
	//for (xx = 0 ; xx < nBeams ; ++xx) {
	//	BNZeroMemory (&elem, sizeof (API_Element));
	//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//	elem.header.guid = beams.Pop ();
	//	err = ACAPI_Element_Get (&elem);
	//	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

	//	foundExistValue = false;

	//	int len;

	//	len = static_cast<int> (round (GetDistance (elem.beam.begC, elem.beam.endC) * 1000, 0));

	//	// �ߺ� �׸��� ������ ����
	//	for (zz = 0 ; zz < objectInfo.nCountsBeam ; ++zz) {
	//		if (objectInfo.beamLength [zz] == len) {
	//			objectInfo.beamCount [zz] ++;
	//			foundExistValue = true;
	//			break;
	//		}
	//	}

	//	// �ű� �׸� �߰��ϰ� ������ ����
	//	if ( !foundExistValue ) {
	//		objectInfo.beamLength.push_back (len);
	//		objectInfo.beamCount.push_back (1);
	//		objectInfo.nCountsBeam ++;
	//	}

	//	ACAPI_DisposeElemMemoHdls (&memo);
	//}

	// ���� �ؽ�Ʈ ǥ��
	// APIParT_Length�� ��� 1000�� ���ؼ� ǥ��
	// APIParT_Boolean�� ��� ��/�ƴϿ� ǥ��
	double	length, length2, length3;
	bool	bTitleAppeared;

	// ���ڵ� ���� (�������� ����)
	sort (objectInfo.records.begin (), objectInfo.records.end (), compareVectorString);

	// *����, ���� ���� ������ ����ϱ� ���� ����
	double	totalAreaOfPlywoods = 0.0;
	double	totalLengthOfTimbers_40x50 = 0.0;	// �ٷ糢 (40*50)
	double	totalLengthOfTimbers_50x80 = 0.0;	// ������ (50*80)
	double	totalLengthOfTimbers_80x80 = 0.0;	// ��°� (80*80)
	double	totalLengthOfTimbersEtc = 0.0;		// ��԰�
	int		count;	// ����

	// *����, �ٷ糢 ���� ������ ����ϱ� ���� ����
	plywoodInfo.clear ();
	darukiInfo.clear ();

	// ��ü �������� ���� ���
	try {
		for (xx = 0 ; xx < objectInfo.keyDesc.size () ; ++xx) {
			bTitleAppeared = false;

			// ���ڵ带 ���� ��ȸ
			for (yy = 0 ; yy < objectInfo.records.size () ; ++yy) {
				// ��ü ���� �̸��� ���ڵ��� 1�� �ʵ尡 ��ġ�ϴ� ��츸 ã�Ƽ� �����
				retVal = my_strcmp (objectInfo.keyDesc.at(xx).c_str (), objectInfo.records.at(yy).at(0).c_str ());

				count = atoi (objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());

				if (retVal == 0) {
					// ���� ���
					if (bTitleAppeared == false) {
						sprintf (buffer, "\n[%s]\n", objectInfo.keyDesc.at(xx).c_str ());
						fprintf (fp, buffer);
						bTitleAppeared = true;
					}

					// ������ �� ���
					if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������ ��ũ") == 0) {
						// ����
						if (objectInfo.records.at(yy).at(2).compare ("����") == 0) {
							sprintf (buffer, "���� / %s", objectInfo.records.at(yy).at(1));
						}

						// �簢
						if (objectInfo.records.at(yy).at(2).compare ("�簢") == 0) {
							sprintf (buffer, "�簢 / %s", objectInfo.records.at(yy).at(1));
						}
						fprintf (fp, buffer);

					} else if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "��ƿ��") == 0)) {
					//} else if ((convertStr(objectInfo.keyDesc.at(xx).c_str()).Compare(L"������") == true) || (convertStr(objectInfo.keyDesc.at(xx).c_str()).Compare(L"��ƿ��") == true)) {
					// !!!
						// �԰���
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
							sprintf (buffer, "%s X %s ", objectInfo.records.at(yy).at(2), objectInfo.records.at(yy).at(3));

						// ��԰�ǰ
						} else {
							length = atof (objectInfo.records.at(yy).at(4).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
							sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("����") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
						length3 = atof (objectInfo.records.at(yy).at(3).c_str ());
						sprintf (buffer, "%.0f X %.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0), round (length3*1000, 0));
						if ( ((abs (length - 0.040) < EPS) && (abs (length2 - 0.050) < EPS)) || ((abs (length - 0.050) < EPS) && (abs (length2 - 0.040) < EPS)) )
							totalLengthOfTimbers_40x50 += (length3 * count);
						else if ( ((abs (length - 0.050) < EPS) && (abs (length2 - 0.080) < EPS)) || ((abs (length - 0.080) < EPS) && (abs (length2 - 0.050) < EPS)) )
							totalLengthOfTimbers_50x80 += (length3 * count);
						else if ((abs (length - 0.080) < EPS) && (abs (length2 - 0.080) < EPS))
							totalLengthOfTimbers_80x80 += (length3 * count);
						else
							totalLengthOfTimbersEtc += (length3 * count);
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("������") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
						length3 = atof (objectInfo.records.at(yy).at(3).c_str ());
						sprintf (buffer, "%s X %.0f (Ŀ�ð���: %s��)", objectInfo.records.at(yy).at(4).c_str (), round (length3*1000, 0), objectInfo.records.at(yy).at(5).c_str ());
						if ( ((abs (length - 0.040) < EPS) && (abs (length2 - 0.050) < EPS)) || ((abs (length - 0.050) < EPS) && (abs (length2 - 0.040) < EPS)) )
							totalLengthOfTimbers_40x50 += (length3 * count);
						else if ( ((abs (length - 0.050) < EPS) && (abs (length2 - 0.080) < EPS)) || ((abs (length - 0.080) < EPS) && (abs (length2 - 0.050) < EPS)) )
							totalLengthOfTimbers_50x80 += (length3 * count);
						else if ((abs (length - 0.080) < EPS) && (abs (length2 - 0.080) < EPS))
							totalLengthOfTimbers_80x80 += (length3 * count);
						else
							totalLengthOfTimbersEtc += (length3 * count);
						fprintf (fp, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "���ǳ�") == 0) {
						if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "��԰�") == 0) {
							// ���� X ���� X �β�
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);
						}

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����") == 0) {
						if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (0.900 * 1.800 * count);
							fprintf (fp, buffer);
							
							// ���� ���� DB�� ����
							record.clear ();
							record.push_back (buffer);
							quantityPlusN (&plywoodInfo, record, count);

							// ����Ʋ ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
								
								// �ٷ糢 DB�� ����
								strcpy (infoText, objectInfo.records.at(yy).at(7).c_str ());
								token = strtok (infoText, " /");
								while (token != NULL) {
									// �����̸� push
									if (atoi (token) != 0) {
										record.clear ();
										record.push_back (token);
										quantityPlusN (&darukiInfo, record, 1);
									}
									token = strtok (NULL, " /");
								}
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (1.200 * 2.400 * count);
							fprintf (fp, buffer);

							// ���� ���� DB�� ����
							record.clear ();
							record.push_back (buffer);
							quantityPlusN (&plywoodInfo, record, count);

							// ����Ʋ ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);

								// �ٷ糢 DB�� ����
								strcpy (infoText, objectInfo.records.at(yy).at(7).c_str ());
								token = strtok (infoText, " /");
								while (token != NULL) {
									// �����̸� push
									if (atoi (token) != 0) {
										record.clear ();
										record.push_back (token);
										quantityPlusN (&darukiInfo, record, 1);
									}
									token = strtok (NULL, " /");
								}
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (0.600 * 1.500 * count);
							fprintf (fp, buffer);

							// ���� ���� DB�� ����
							record.clear ();
							record.push_back (buffer);
							quantityPlusN (&plywoodInfo, record, count);

							// ����Ʋ ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);

								// �ٷ糢 DB�� ����
								strcpy (infoText, objectInfo.records.at(yy).at(7).c_str ());
								token = strtok (infoText, " /");
								while (token != NULL) {
									// �����̸� push
									if (atoi (token) != 0) {
										record.clear ();
										record.push_back (token);
										quantityPlusN (&darukiInfo, record, 1);
									}
									token = strtok (NULL, " /");
								}
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (0.600 * 1.800 * count);
							fprintf (fp, buffer);

							// ���� ���� DB�� ����
							record.clear ();
							record.push_back (buffer);
							quantityPlusN (&plywoodInfo, record, count);

							// ����Ʋ ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);

								// �ٷ糢 DB�� ����
								strcpy (infoText, objectInfo.records.at(yy).at(7).c_str ());
								token = strtok (infoText, " /");
								while (token != NULL) {
									// �����̸� push
									if (atoi (token) != 0) {
										record.clear ();
										record.push_back (token);
										quantityPlusN (&darukiInfo, record, 1);
									}
									token = strtok (NULL, " /");
								}
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (0.900 * 1.500 * count);
							fprintf (fp, buffer);

							// ���� ���� DB�� ����
							record.clear ();
							record.push_back (buffer);
							quantityPlusN (&plywoodInfo, record, count);

							// ����Ʋ ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);

								// �ٷ糢 DB�� ����
								strcpy (infoText, objectInfo.records.at(yy).at(7).c_str ());
								token = strtok (infoText, " /");
								while (token != NULL) {
									// �����̸� push
									if (atoi (token) != 0) {
										record.clear ();
										record.push_back (token);
										quantityPlusN (&darukiInfo, record, 1);
									}
									token = strtok (NULL, " /");
								}
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "��԰�") == 0) {
							// ���� X ���� X �β�
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (length * length2 * count);
							fprintf (fp, buffer);

							// ���� ���� DB�� ����
							record.clear ();
							record.push_back (buffer);
							quantityPlusN (&plywoodInfo, record, count);

							// ����Ʋ ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);

								// �ٷ糢 DB�� ����
								strcpy (infoText, objectInfo.records.at(yy).at(7).c_str ());
								token = strtok (infoText, " /");
								while (token != NULL) {
									// �����̸� push
									if (atoi (token) != 0) {
										record.clear ();
										record.push_back (token);
										quantityPlusN (&darukiInfo, record, 1);
									}
									token = strtok (NULL, " /");
								}
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "������") == 0) {
							sprintf (buffer, "������ ");
							fprintf (fp, buffer);

						} else {
							sprintf (buffer, "�ٰ��� ");
							fprintf (fp, buffer);
						}

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����(�ٰ���)") == 0) {
						// ���� ����
						sprintf (buffer, "����: %.2f ", atof (objectInfo.records.at(yy).at(1).c_str ()));
						totalAreaOfPlywoods += (atof (objectInfo.records.at(yy).at(1).c_str ()) * count);
						fprintf (fp, buffer);

						// ����Ʋ ON
						if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
							sprintf (buffer, "(���� �ѱ���: %.0f) ", round (atof (objectInfo.records.at(yy).at(3).c_str ())*1000, 0));
							totalLengthOfTimbers_40x50 += (atof (objectInfo.records.at(yy).at(3).c_str ()) * count);
							fprintf (fp, buffer);
						}

					} else if (objectInfo.keyDesc.at(xx).compare ("RS Push-Pull Props") == 0) {
						// ���̽� �÷���Ʈ ����
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) == 1) {
							sprintf (buffer, "���̽� �÷���Ʈ(����) ");
						} else {
							sprintf (buffer, "���̽� �÷���Ʈ(����) ");
						}
						fprintf (fp, buffer);

						// �԰�(���)
						sprintf (buffer, "�԰�(���): %s ", objectInfo.records.at(yy).at(2).c_str ());
						fprintf (fp, buffer);

						// �԰�(�Ϻ�) - ���û���
						if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
							sprintf (buffer, "�԰�(�Ϻ�): %s ", objectInfo.records.at(yy).at(3).c_str ());
						}
						fprintf (fp, buffer);
				
					} else if (objectInfo.keyDesc.at(xx).compare ("Push-Pull Props (�⼺ǰ �� �������ǰ)") == 0) {
						// ���̽� �÷���Ʈ ����
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) == 1) {
							sprintf (buffer, "���̽� �÷���Ʈ(����) ");
						} else {
							sprintf (buffer, "���̽� �÷���Ʈ(����) ");
						}
						fprintf (fp, buffer);

						// �԰�(���)
						sprintf (buffer, "�԰�(���): %s ", objectInfo.records.at(yy).at(2).c_str ());
						fprintf (fp, buffer);

						// �԰�(�Ϻ�) - ���û���
						if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
							sprintf (buffer, "�԰�(�Ϻ�): %s ", objectInfo.records.at(yy).at(3).c_str ());
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("�簢������") == 0) {
						// �簢������
						if (atof (objectInfo.records.at(yy).at(1).c_str ()) < EPS) {
							length = atof (objectInfo.records.at(yy).at(2).c_str ());
							sprintf (buffer, "50 x 50 x %.0f ", round (length*1000, 0));

						// ���簢������
						} else {
							length = atof (objectInfo.records.at(yy).at(2).c_str ());
							sprintf (buffer, "%s x %.0f ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("����������") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f ", round (length*1000, 0));
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("�ƿ��ڳʾޱ�") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f ", round (length*1000, 0));
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("������") == 0) {
						if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							length3 = atof (objectInfo.records.at(yy).at(5).c_str ());
							totalAreaOfPlywoods += (atof (objectInfo.records.at(yy).at(6).c_str ()) * count);
							sprintf (buffer, "%.0f / ����(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(1).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
						} else {
							length = atof (objectInfo.records.at(yy).at(1).c_str ());
							sprintf (buffer, "%.0f ", round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("�����ƿ��ڳ�") == 0) {
						sprintf (buffer, "Ÿ��(%s) %.0f ", objectInfo.records.at(yy).at(1).c_str (), round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0));
						fprintf (fp, buffer);
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) > 0) {
							totalAreaOfPlywoods += (atof (objectInfo.records.at(yy).at(6).c_str ()) * count);
							sprintf (buffer, "����1(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(4).c_str ())*1000, 0));
							fprintf (fp, buffer);
							sprintf (buffer, "����2(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(5).c_str ())*1000, 0));
							fprintf (fp, buffer);
						}

					} else if (objectInfo.keyDesc.at(xx).compare ("�������ڳ�") == 0) {
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) > 0) {
							length = atof (objectInfo.records.at(yy).at(4).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
							length3 = atof (objectInfo.records.at(yy).at(6).c_str ());
							totalAreaOfPlywoods += (atof (objectInfo.records.at(yy).at(7).c_str ()) * count);
							sprintf (buffer, "%.0f / ����(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
						} else {
							length = atof (objectInfo.records.at(yy).at(2).c_str ());
							sprintf (buffer, "%.0f ", round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("���纸 ����� v2") == 0) {
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
							length = atof (objectInfo.records.at(yy).at(2).c_str ()) / 1000;
							totalLengthOfTimbers_40x50 += (length * count);
							sprintf (buffer, "����(%.0f) ", round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("�ܿ���") == 0) {
						sprintf (buffer, "����ũ��: %.0f X %.0f / ����ũ��: %.0f X %.0f (���������� �ڸ�: %s)",
							round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(3).c_str ())*1000, 0),
							round (atof (objectInfo.records.at(yy).at(4).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(5).c_str ())*1000, 0),
							(atoi (objectInfo.records.at(yy).at(5).c_str ()) ? "�ڸ�" : "�ڸ��� ����"));
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("PERI���ٸ� ������") == 0) {
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) == 1) {
							sprintf (buffer, "�԰�(%s) ����(%.0f) ũ�ν����(%s) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0), objectInfo.records.at(yy).at(4).c_str ());
						} else {
							sprintf (buffer, "�԰�(%s) ����(%.0f) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("����Ʈ") == 0) {
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) == 1) {
							sprintf (buffer, "�԰�(%s) ����(%.0f) ũ�ν����(%s) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0), objectInfo.records.at(yy).at(4).c_str ());
						} else {
							sprintf (buffer, "�԰�(%s) ����(%.0f) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("â�� ������ ���ǰ�Ǫ��") == 0) {
						// �ʺ� X ���� X �� �β�
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(3).c_str ());
						length3 = atof (objectInfo.records.at(yy).at(4).c_str ());
						sprintf (buffer, "%.0f X %.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0), round (length3*1000, 0));
						fprintf (fp, buffer);

						// ���� ����
						totalAreaOfPlywoods += ((atof (objectInfo.records.at(yy).at(5).c_str ())) * count);

						// �ٷ糢 ����
						totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ())) * count);
						
					} else {
						for (zz = 0 ; zz < objectInfo.nInfo.at(xx) ; ++zz) {
							// ������ �� ���
							sprintf (buffer, "%s(%s) ", objectInfo.varDesc.at(xx).at(zz).c_str (), objectInfo.records.at(yy).at(zz+1).c_str ());
							fprintf (fp, buffer);
						}
					}

					// ���� ���
					sprintf (buffer, ": %s EA\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());
					fprintf (fp, buffer);
				}
			}
		}
	} catch (exception& ex) {
		WriteReport_Alert ("��� �Լ����� ���� �߻�: %s", ex.what ());
	}

	// �Ϲ� ��� - ��
	//for (xx = 0 ; xx < objectInfo.nCountsBeam ; ++xx) {
	//	if (xx == 0) {
	//		fprintf (fp, "\n[��]\n");
	//	}
	//	sprintf (buffer, "%d : %d EA\n", objectInfo.beamLength [xx], objectInfo.beamCount [xx]);
	//	fprintf (fp, buffer);
	//}

	// �� �� ���� ��ü
	if (objectInfo.nUnknownObjects > 0) {
		sprintf (buffer, "\n�� �� ���� ��ü : %d EA\n", objectInfo.nUnknownObjects);
		fprintf (fp, buffer);
	}

	// *����, �ٷ糢 ���� ���� ���
	sort (plywoodInfo.begin (), plywoodInfo.end (), compareVectorString);
	sort (darukiInfo.begin (), darukiInfo.end (), compareVectorString);

	if (plywoodInfo.size () > 0) {
		sprintf (buffer, "\n���� �԰ݺ� ���� ������ ������ �����ϴ�.");
		fprintf (fp, buffer);

		for (xx = 0 ; xx < plywoodInfo.size () ; ++xx) {
			sprintf (buffer, "\n%s : %s EA", plywoodInfo.at(xx).at(0).c_str (), plywoodInfo.at(xx).at(1).c_str ());
			fprintf (fp, buffer);
		}
		fprintf (fp, "\n");
	}

	if (darukiInfo.size () > 0) {
		sprintf (buffer, "\n���� ����Ʋ �ٷ糢 (40*50) �԰ݺ� ���� ������ ������ �����ϴ�.");
		fprintf (fp, buffer);
		
		for (xx = 0 ; xx < darukiInfo.size () ; ++xx) {
			sprintf (buffer, "\n%s : %s EA", darukiInfo.at(xx).at(0).c_str (), darukiInfo.at(xx).at(1).c_str ());
			fprintf (fp, buffer);
		}
		fprintf (fp, "\n");
	}

	// *����, ���� ���� ���� ���
	// ���� 4x8 �԰� (1200*2400) �������� �� ������ ������ ���� ���� ������ ����
	if (totalAreaOfPlywoods > EPS) {
		sprintf (buffer, "\n���� ���� ������ ������ �����ϴ�.\n�� ���� (%.2f ��) �� ���� 4x8 �԰� (1200*2400) = %.0f �� (���� 5�ۼ�Ʈ �����)\n", totalAreaOfPlywoods, ceil ((totalAreaOfPlywoods / 2.88)*1.05));
		fprintf (fp, buffer);
	}
	// ���� �ٷ糢(40*50), ������(50*80), ��°�(80*80), 1���� 3600mm
	if ((totalLengthOfTimbers_40x50 > EPS) || (totalLengthOfTimbers_50x80 > EPS) || (totalLengthOfTimbers_80x80 > EPS) || (totalLengthOfTimbersEtc > EPS)) {
		sprintf (buffer, "\n���� ���� ������ ������ �����ϴ�.\n");
		fprintf (fp, buffer);
		if (totalLengthOfTimbers_40x50 > EPS) {
			sprintf (buffer, "�ٷ糢 (40*50) : �� ���� (%.3f) �� 1�� (3600) = %.0f �� (���� 5�ۼ�Ʈ �����)\n", totalLengthOfTimbers_40x50, ceil ((totalLengthOfTimbers_40x50 / 3.6)*1.05));
			fprintf (fp, buffer);
		}
		if (totalLengthOfTimbers_50x80 > EPS) {
			sprintf (buffer, "������ (50*80) : �� ���� (%.3f) �� 1�� (3600) = %.0f �� (���� 5�ۼ�Ʈ �����)\n", totalLengthOfTimbers_50x80, ceil ((totalLengthOfTimbers_50x80 / 3.6)*1.05));
			fprintf (fp, buffer);
		}
		if (totalLengthOfTimbers_80x80 > EPS) {
			sprintf (buffer, "��°� (80*80) : �� ���� (%.3f) �� 1�� (3600) = %.0f �� (���� 5�ۼ�Ʈ �����)\n", totalLengthOfTimbers_80x80, ceil ((totalLengthOfTimbers_80x80 / 3.6)*1.05));
			fprintf (fp, buffer);
		}
		if (totalLengthOfTimbersEtc > EPS) {
			sprintf (buffer, "��԰� : �� ���� (%.3f) �� 1�� (3600) = %.0f �� (���� 5�ۼ�Ʈ �����)\n", totalLengthOfTimbersEtc, ceil ((totalLengthOfTimbersEtc / 3.6)*1.05));
			fprintf (fp, buffer);
		}
	}
	if ((totalAreaOfPlywoods > EPS) || (totalLengthOfTimbers_40x50 > EPS) || (totalLengthOfTimbers_50x80 > EPS) || (totalLengthOfTimbers_80x80 > EPS) || (totalLengthOfTimbersEtc > EPS)) {
		sprintf (buffer, "\n���ǻ���: ����/���� ���� ������ ���� ��ü�� ���ؼ��� ���Ǿ����ϴ�. �߰��� ��ü�� �ִٸ� �����ڿ��� �����Ͻʽÿ�.\n���� / ����(�ٰ���) / ���� / ������ / ������ / �����ƿ��ڳ� / �������ڳ� / ���纸 ����� v2 / â�� ������ ���ǰ�Ǫ��\n");
		fprintf (fp, buffer);
	}

	fclose (fp);

	// ��ü �������� ���� ��� (�߰�������)
	try {
		for (xx = 0 ; xx < objectInfo.keyDesc.size () ; ++xx) {
			// ���ڵ带 ���� ��ȸ
			for (yy = 0 ; yy < objectInfo.records.size () ; ++yy) {
				// ��ü ���� �̸��� ���ڵ��� 1�� �ʵ尡 ��ġ�ϴ� ��츸 ã�Ƽ� �����
				retVal = my_strcmp (objectInfo.keyDesc.at(xx).c_str (), objectInfo.records.at(yy).at(0).c_str ());

				if (retVal == 0) {
					// ǰ��
					sprintf (buffer, "%s | ", objectInfo.keyDesc.at(xx).c_str ());
					fprintf (fp_interReport, buffer);

					// ������ �� ���
					if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "��ƿ��") == 0)) {
						// �԰�
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
							// �԰���
							sprintf (buffer, "%s X %s | ", objectInfo.records.at(yy).at(2), objectInfo.records.at(yy).at(3));
						} else {
							// ��԰�ǰ
							length = atof (objectInfo.records.at(yy).at(4).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
							sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length2*1000, 0));
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "���ڳ��ǳ�") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�ƿ��ڳ��ǳ�") == 0)) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
						length3 = atof (objectInfo.records.at(yy).at(3).c_str ());

						// �԰�
						sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length2*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%.0f | ", round (length3*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�ƿ��ڳʾޱ�") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());

						// �԰�
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
						length3 = atof (objectInfo.records.at(yy).at(3).c_str ());

						// �԰�
						sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length2*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%.0f | ", round (length3*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "�� | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�ٷ������̼�") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(2).c_str ());

						// �԰�
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%.0f | ", round (length2*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����������") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());

						// �԰�
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�簢������") == 0) {
						length = atof (objectInfo.records.at(yy).at(2).c_str ());

						// �԰�
						if (atof (objectInfo.records.at(yy).at(1).c_str ()) < EPS) {
							// �簢������
							sprintf (buffer, "50 x 50 | ");
						} else {
							// ���簢������
							sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "�� | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "���ǳ�") == 0) {
						// �԰�
						if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "��԰�") == 0) {
							// ���� X ���� X �β�
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							sprintf (buffer, "%.0f X %.0f X %s | ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "�� | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����") == 0) {
						// �԰�
						if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "��԰�") == 0) {
							// ���� X ���� X �β�
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							sprintf (buffer, "%.0f X %.0f X %s | ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "�� | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "RS Push-Pull Props ����ǽ� (�ξ���� ����)") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "RS Push-Pull Props") == 0) {
						// �԰�
						if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
							// �Ϻ� ������ ���� ���
							sprintf (buffer, "%s, %s | ", objectInfo.records.at(yy).at(2).c_str (), objectInfo.records.at(yy).at(3).c_str ());
						} else {
							// �Ϻ� ������ ���� ���
							sprintf (buffer, "%s, - | ", objectInfo.records.at(yy).at(2).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�ɺ�Ʈ��Ʈ") == 0) {
						// �԰�
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����ö�� (�簢�ͼ�Ȱ��)") == 0) {
						// �԰�
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�� �ۿ���") == 0) {
						// �԰�
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "PERI���ٸ� ������") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(2).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "PERI���ٸ� ������") == 0) {
						// �԰�
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "GT24 �Ŵ�") == 0) {
						// �԰�
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������") == 0) {
						// �԰� (������ ��ü ����)
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ���� (���� �ʺ� X ����)
						length = atof (objectInfo.records.at(yy).at(3).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
						if (atoi (objectInfo.records.at(yy).at(2).c_str ()) == 1) {
							sprintf (buffer, "%s X %.0f | ", objectInfo.records.at(yy).at(5).c_str (), abs (round (length*1000, 0) - round (length2*1000, 0)));
						} else {
							sprintf (buffer, "- | ");
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�������ڳ�") == 0) {
						// �԰� (������ �ʺ� X ����)
						sprintf (buffer, "%s X %s | ", objectInfo.records.at(yy).at(1).c_str (), objectInfo.records.at(yy).at(2).c_str ());
						fprintf (fp_interReport, buffer);

						// ���� (���� �ʺ� X ����)
						length = atof (objectInfo.records.at(yy).at(4).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) == 1) {
							sprintf (buffer, "%s X %.0f | ", objectInfo.records.at(yy).at(6).c_str (), abs (round (length*1000, 0) - round (length2*1000, 0)));
						} else {
							sprintf (buffer, "- | ");
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������ ��ũ") == 0) {
						// �԰�
						if (objectInfo.records.at(yy).at(2).compare ("����") == 0) {
							sprintf (buffer, "%s, ���� | ", objectInfo.records.at(yy).at(1).c_str ());
						} else if (objectInfo.records.at(yy).at(2).compare ("�簢") == 0) {
							sprintf (buffer, "%s, �簢 | ", objectInfo.records.at(yy).at(1).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����Ʈ") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������ ���̺��� (���ǳ�)") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����Ŭ����") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������Ʈ�� Push-Pull Props ����ǽ�") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ���� 
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "���� �� �����") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�ܿ���") == 0) {
						// �԰�
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) == 1) {
							// ���� ���� X ����
							sprintf (buffer, "%s X %s | ", objectInfo.records.at(yy).at(2).c_str (), objectInfo.records.at(yy).at(3).c_str ());
						} else {
							// ���� ���� X ����
							sprintf (buffer, "%s X %s | ", objectInfo.records.at(yy).at(4).c_str (), objectInfo.records.at(yy).at(5).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "�� | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "Push-Pull Props (�⼺ǰ �� �������ǰ)") == 0) {
						// �԰�
						if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
							// �Ϻ� ������ ���� ���
							sprintf (buffer, "%s, %s | ", objectInfo.records.at(yy).at(2).c_str (), objectInfo.records.at(yy).at(3).c_str ());
						} else {
							// �Ϻ� ������ ���� ���
							sprintf (buffer, "%s, - | ", objectInfo.records.at(yy).at(2).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else {
						// �԰�, ���� ���� ������ ǥ���� ���

						// �԰�
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);
					}

					// ���� ���
					sprintf (buffer, "%s\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());
					fprintf (fp_interReport, buffer);
				}
			}
		}
	} catch (exception& ex) {
		WriteReport_Alert ("��� �Լ����� ���� �߻�: %s", ex.what ());
	}

	// �� �� ���� ��ü
	if (objectInfo.nUnknownObjects > 0) {
		sprintf (buffer, "�� �� ���� ��ü | - | - | - | %d\n", objectInfo.nUnknownObjects);
		fprintf (fp_interReport, buffer);
	}

	fclose (fp_interReport);

	// �׷�ȭ �Ͻ����� OFF
	suspendGroups (false);

	//ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	//location.ToDisplayText (&resultString);
	//WriteReport_Alert ("������� ���� ��ġ�� �����߽��ϴ�.\n\n%s\n�Ǵ� ������Ʈ ������ �ִ� ����", resultString.ToCStr ().Get ());

	return	err;
}

// ������ ���� ���� �������� (Multi ���)
GSErrCode	exportElementInfoOnVisibleLayers (void)
{
	GSErrCode	err = NoError;
	unsigned short		xx, yy, zz;
	short		mm;
	//bool		regenerate = true;

	// ��� ��ü���� ���� ��ǥ�� ���� ������
	vector<API_Coord3D>	vecPos;

	// ��� ��ü, �� ����
	GS::Array<API_Guid>		elemList;
	GS::Array<API_Guid>		objects;
	GS::Array<API_Guid>		beams;
	long					nObjects = 0;
	long					nBeams = 0;

	// ������ ��ҵ��� ���� ����ϱ�
	API_Element			elem;
	API_ElementMemo		memo;
	SummaryOfObjectInfo	objectInfo;

	char			tempStr [512];
	const char*		foundStr;
	bool			foundObject;
	bool			foundExistValue;
	int				retVal;
	int				nInfo;
	API_AddParID	varType;
	vector<string>	record;

	// GS::Array �ݺ���
	//GS::Array<API_Guid>::Iterator	iterObj;
	//API_Guid	curGuid;

	// ���̾� ���� ����
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];
	vector<LayerList>	layerList;

	// ���̾� Ÿ�Կ� ���� ĸ�� ���� ����
	char*			foundLayerName;
	short			layerType = UNDEFINED;

	// ��Ÿ
	char			buffer [512];
	char			filename [512];

	// �۾� �� ����
	API_StoryInfo	storyInfo;
	double			workLevel_object;		// ��ü�� �۾� �� ����


	// ����ٸ� ǥ���ϱ� ���� ����
	GS::UniString       title ("�������� ���� ��Ȳ");
	GS::UniString       subtitle ("������...");
	short	nPhase;
	Int32	cur, total;

	// ���� ���Ϸ� ��� ���� ��������
	// ���� ������ ���� ����
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp;
	FILE				*fp_unite;


	// �׷�ȭ �Ͻ����� ON
	suspendGroups (true);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

	// [���] ���̾�α׿��� ��ü �̹����� ĸ������ ���θ� ���
	//result = DGAlert (DG_INFORMATION, "ĸ�� ���� ����", "ĸ�� �۾��� �����Ͻðڽ��ϱ�?", "", "��", "�ƴϿ�", "");
	//result = DG_CANCEL;

	// ������Ʈ �� ���̾� ������ �˾Ƴ�
	nLayers = getLayerCount ();

	// ���̴� ���̾���� ��� �����ϱ�
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList [nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// ���̾� �̸��� �ε��� ����
	for (xx = 0 ; xx < nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx];
		err = ACAPI_Attribute_Get (&attrib);

		sprintf (fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName [strlen (fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList [xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back (newLayerItem);
	}

	// ���̾� �̸� �������� �����Ͽ� ���̾� �ε��� ���� ����
	sort (layerList.begin (), layerList.end (), compareLayerName);		// ���̾� �̸� ���� �������� ����

	// �Ͻ������� ��� ���̾� �����
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

	ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
	sprintf (filename, "%s - ������ ���� ���� (����).csv", miscAppInfo.caption);
	fp_unite = fopen (filename, "w+");

	if (fp_unite == NULL) {
		DGAlert (DG_ERROR, L"����", L"���� ���� ���������� ���� �� �����ϴ�.", "", L"Ȯ��", "", "");
		return	NoError;
	}

	// ���� ��Ȳ ǥ���ϴ� ��� - �ʱ�ȭ
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface (APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &total);

	// ���̴� ���̾���� �ϳ��� ��ȸ�ϸ鼭 ��ü ��ҵ��� ������ �� "������ ���� ���� ��������" ��ƾ ����
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList [mm-1].layerInd;
		err = ACAPI_Attribute_Get (&attrib);

		// �ʱ�ȭ
		objects.Clear ();
		beams.Clear ();
		vecPos.clear ();
		objectInfo.clear ();
		objectInfo.nUnknownObjects = 0;

		if (err == NoError) {
			// ���̾� ���̱�
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// ��� ��� ��������
			ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����, ��ü Ÿ�Ը�
			while (elemList.GetSize () > 0) {
				objects.Push (elemList.Pop ());
			}

			ACAPI_Element_GetElemList (API_BeamID, &elemList, APIFilt_OnVisLayer);		// ���̴� ���̾ ����, �� Ÿ�Ը�
			while (elemList.GetSize () > 0) {
				beams.Push (elemList.Pop ());
			}
			nObjects = objects.GetSize ();
			nBeams = beams.GetSize ();

			if ((nObjects == 0) && (nBeams == 0))
				continue;

			// ���̾� �̸� ������
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			// ���̾� �̸� �ĺ��ϱ� (WALL: ��, SLAB: ������, COLU: ���, BEAM: ��, WLBM: ���纸)
			layerType = UNDEFINED;
			foundLayerName = strstr (fullLayerName, "WALL");
			if (foundLayerName != NULL)	layerType = WALL;
			foundLayerName = strstr (fullLayerName, "SLAB");
			if (foundLayerName != NULL)	layerType = SLAB;
			foundLayerName = strstr (fullLayerName, "COLU");
			if (foundLayerName != NULL)	layerType = COLU;
			foundLayerName = strstr (fullLayerName, "BEAM");
			if (foundLayerName != NULL)	layerType = BEAM;
			foundLayerName = strstr (fullLayerName, "WLBM");
			if (foundLayerName != NULL)	layerType = WLBM;

			sprintf (filename, "%s - ������ ���� ����.csv", fullLayerName);
			fp = fopen (filename, "w+");

			if (fp == NULL) {
				WriteReport_Alert ("���̾� %s�� ���ϸ��� �� �� �����Ƿ� �����մϴ�.", fullLayerName);
				continue;
			}

			// ���̾� �̸�
			sprintf (buffer, "\n\n<< ���̾� : %s >>\n", fullLayerName);
			fprintf (fp, buffer);
			fprintf (fp_unite, buffer);

			sprintf (buffer, "%s\n", getExplanationOfLayerCode (fullLayerName));
			fprintf (fp, buffer);
			fprintf (fp_unite, buffer);

			for (xx = 0 ; xx < nObjects ; ++xx) {
				foundObject = false;

				BNZeroMemory (&elem, sizeof (API_Element));
				BNZeroMemory (&memo, sizeof (API_ElementMemo));
				elem.header.guid = objects.Pop ();
				err = ACAPI_Element_Get (&elem);

				if (err == NoError && elem.header.hasMemo) {
					err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

					if (err == NoError) {
						// ��ü�� ���� �����ϱ� ==================================
						//API_Coord3D	coord;

						//coord.x = elem.object.pos.x;
						//coord.y = elem.object.pos.y;
						//coord.z = elem.object.level;
					
						//vecPos.push_back (coord);
						// ��ü�� ���� �����ϱ� ==================================

						// �۾� �� ���� �ݿ� -- ��ü
						if (xx == 0) {
							BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
							workLevel_object = 0.0;
							ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
							for (yy = 0 ; yy <= (storyInfo.lastStory - storyInfo.firstStory) ; ++yy) {
								if (storyInfo.data [0][yy].index == elem.header.floorInd) {
									workLevel_object = storyInfo.data [0][yy].level;
									break;
								}
							}
							BMKillHandle ((GSHandle *) &storyInfo.data);
						}

						// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
						ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);
						bool	bForce = true;
						ACAPI_Database (APIDb_RefreshElementID, &elem.header, &bForce);

						try {
							for (yy = 0 ; yy < objectInfo.keyName.size () ; ++yy) {
								strcpy (tempStr, objectInfo.keyName.at(yy).c_str ());
								foundStr = getParameterStringByName (&memo, tempStr);

								// ��ü ������ ã�Ҵٸ�,
								if (my_strcmp (foundStr, "") != 0) {
									retVal = my_strcmp (objectInfo.keyDesc.at(yy).c_str (), foundStr);

									if (retVal == 0) {
										foundObject = true;
										foundExistValue = false;

										// �߰��� ��ü�� �����͸� ������� ���ڵ� �߰�
										if (!record.empty ())
											record.clear ();

										record.push_back (objectInfo.keyDesc.at(yy));		// ��ü �̸�
										nInfo = objectInfo.nInfo.at(yy);
										for (zz = 0 ; zz < nInfo ; ++zz) {
											sprintf (buffer, "%s", objectInfo.varName.at(yy).at(zz).c_str ());
											varType = getParameterTypeByName (&memo, buffer);

											if ((varType != APIParT_Separator) || (varType != APIParT_Title) || (varType != API_ZombieParT)) {
												if (varType == APIParT_CString)
													sprintf (tempStr, "%s", getParameterStringByName (&memo, buffer));	// ���ڿ�
												else
													sprintf (tempStr, "%.3f", getParameterValueByName (&memo, buffer));	// ����
											}
											record.push_back (tempStr);		// ������
										}

										objectInfo.quantityPlus1 (record);

									}
								}
							}
						} catch (exception& ex) {
							WriteReport_Alert ("��ü ���� �������� ���� �߻�: %s", ex.what ());
						}

						// ���� ã�� ���ϸ� �� �� ���� ��ü�� �����
						if (foundObject == false)
							objectInfo.nUnknownObjects ++;
					}

					ACAPI_DisposeElemMemoHdls (&memo);
				}
			}

			// �� ���� ����
			//for (xx = 0 ; xx < nBeams ; ++xx) {
			//	BNZeroMemory (&elem, sizeof (API_Element));
			//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
			//	elem.header.guid = beams.Pop ();
			//	err = ACAPI_Element_Get (&elem);
			//	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			//	foundExistValue = false;

			//	int len;

			//	len = static_cast<int> (round (GetDistance (elem.beam.begC, elem.beam.endC) * 1000, 0));

			//	// �ߺ� �׸��� ������ ����
			//	for (zz = 0 ; zz < objectInfo.nCountsBeam ; ++zz) {
			//		if (objectInfo.beamLength [zz] == len) {
			//			objectInfo.beamCount [zz] ++;
			//			foundExistValue = true;
			//			break;
			//		}
			//	}

			//	// �ű� �׸� �߰��ϰ� ������ ����
			//	if ( !foundExistValue ) {
			//		objectInfo.beamLength.push_back (len);
			//		objectInfo.beamCount.push_back (1);
			//		objectInfo.nCountsBeam ++;
			//	}

			//	ACAPI_DisposeElemMemoHdls (&memo);
			//}

			// APIParT_Length�� ��� 1000�� ���ؼ� ǥ��
			// APIParT_Boolean�� ��� ��/�ƴϿ� ǥ��
			double	length, length2, length3;
			bool	bTitleAppeared;

			// ���ڵ� ���� (�������� ����)
			sort (objectInfo.records.begin (), objectInfo.records.end (), compareVectorString);

			// ��ü �������� ���� ���
			try {
				for (xx = 0 ; xx < objectInfo.keyDesc.size () ; ++xx) {
					bTitleAppeared = false;

					// ���ڵ带 ���� ��ȸ
					for (yy = 0 ; yy < objectInfo.records.size () ; ++yy) {
						// ��ü ���� �̸��� ���ڵ��� 1�� �ʵ尡 ��ġ�ϴ� ��츸 ã�Ƽ� �����
						retVal = my_strcmp (objectInfo.keyDesc.at(xx).c_str (), objectInfo.records.at(yy).at(0).c_str ());

						if (retVal == 0) {
							// ���� ���
							if (bTitleAppeared == false) {
								sprintf (buffer, "\n[%s]\n", objectInfo.keyDesc.at(xx).c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
								bTitleAppeared = true;
							}

							// ������ �� ���
							if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������ ��ũ") == 0) {
								// ����
								if (objectInfo.records.at(yy).at(2).compare ("����") == 0) {
									sprintf (buffer, "���� / %s", objectInfo.records.at(yy).at(1));
								}

								// �簢
								if (objectInfo.records.at(yy).at(2).compare ("�簢") == 0) {
									sprintf (buffer, "�簢 / %s", objectInfo.records.at(yy).at(1));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "��ƿ��") == 0)) {
								// �԰���
								if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
									sprintf (buffer, "%s X %s ", objectInfo.records.at(yy).at(2), objectInfo.records.at(yy).at(3));

								// ��԰�ǰ
								} else {
									length = atof (objectInfo.records.at(yy).at(4).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
									sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("����") == 0) {
								length = atof (objectInfo.records.at(yy).at(1).c_str ());
								length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
								length3 = atof (objectInfo.records.at(yy).at(3).c_str ());
								sprintf (buffer, "%.0f X %.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0), round (length3*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("������") == 0) {
								length = atof (objectInfo.records.at(yy).at(1).c_str ());
								length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
								length3 = atof (objectInfo.records.at(yy).at(3).c_str ());
								sprintf (buffer, "%s X %.0f (Ŀ�ð���: %s��)", objectInfo.records.at(yy).at(4).c_str (), round (length3*1000, 0), objectInfo.records.at(yy).at(5).c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "���ǳ�") == 0) {
								if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
									sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
									sprintf (buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
									sprintf (buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
									sprintf (buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
									sprintf (buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "��԰�") == 0) {
									// ���� X ���� X �β�
									length = atof (objectInfo.records.at(yy).at(3).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
									sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}

							} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����") == 0) {
								if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
									sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// ����Ʋ ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(���� �ѱ���: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
									sprintf (buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// ����Ʋ ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(���� �ѱ���: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
									sprintf (buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// ����Ʋ ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(���� �ѱ���: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
									sprintf (buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// ����Ʋ ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(���� �ѱ���: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
									sprintf (buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// ����Ʋ ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(���� �ѱ���: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "��԰�") == 0) {
									// ���� X ���� X �β�
									length = atof (objectInfo.records.at(yy).at(3).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
									sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// ����Ʋ ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(���� �ѱ���: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "������") == 0) {
									sprintf (buffer, "������ ");
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else {
									sprintf (buffer, "�ٰ��� ");
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}

							} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����(�ٰ���)") == 0) {
								// ���� ����
								sprintf (buffer, "����: %.2f ", atof (objectInfo.records.at(yy).at(1).c_str ()));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

								// ����Ʋ ON
								if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
									sprintf (buffer, "(���� �ѱ���: %.0f) ", round (atof (objectInfo.records.at(yy).at(3).c_str ())*1000, 0));
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}

							} else if (objectInfo.keyDesc.at(xx).compare ("RS Push-Pull Props") == 0) {
								// ���̽� �÷���Ʈ ����
								if (atoi (objectInfo.records.at(yy).at(1).c_str ()) == 1) {
									sprintf (buffer, "���̽� �÷���Ʈ(����) ");
								} else {
									sprintf (buffer, "���̽� �÷���Ʈ(����) ");
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

								// �԰�(���)
								sprintf (buffer, "�԰�(���): %s ", objectInfo.records.at(yy).at(2).c_str ());
								fprintf (fp, buffer);

								// �԰�(�Ϻ�) - ���û���
								if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
									sprintf (buffer, "�԰�(�Ϻ�): %s ", objectInfo.records.at(yy).at(3).c_str ());
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
				
							} else if (objectInfo.keyDesc.at(xx).compare ("Push-Pull Props (�⼺ǰ �� �������ǰ)") == 0) {
								// ���̽� �÷���Ʈ ����
								if (atoi (objectInfo.records.at(yy).at(1).c_str ()) == 1) {
									sprintf (buffer, "���̽� �÷���Ʈ(����) ");
								} else {
									sprintf (buffer, "���̽� �÷���Ʈ(����) ");
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

								// �԰�(���)
								sprintf (buffer, "�԰�(���): %s ", objectInfo.records.at(yy).at(2).c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

								// �԰�(�Ϻ�) - ���û���
								if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
									sprintf (buffer, "�԰�(�Ϻ�): %s ", objectInfo.records.at(yy).at(3).c_str ());
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("�簢������") == 0) {
								// �簢������
								if (atof (objectInfo.records.at(yy).at(1).c_str ()) < EPS) {
									length = atof (objectInfo.records.at(yy).at(2).c_str ());
									sprintf (buffer, "50 x 50 x %.0f ", round (length*1000, 0));

								// ���簢������
								} else {
									length = atof (objectInfo.records.at(yy).at(2).c_str ());
									sprintf (buffer, "%s x %.0f ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("����������") == 0) {
								length = atof (objectInfo.records.at(yy).at(1).c_str ());
								sprintf (buffer, "%.0f ", round (length*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("�ƿ��ڳʾޱ�") == 0) {
								length = atof (objectInfo.records.at(yy).at(1).c_str ());
								sprintf (buffer, "%.0f ", round (length*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("������") == 0) {
								if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
									length = atof (objectInfo.records.at(yy).at(3).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
									length3 = atof (objectInfo.records.at(yy).at(5).c_str ());
									sprintf (buffer, "%.0f / ����(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(1).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
								} else {
									length = atof (objectInfo.records.at(yy).at(1).c_str ());
									sprintf (buffer, "%.0f ", round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("�����ƿ��ڳ�") == 0) {
								sprintf (buffer, "Ÿ��(%s) %.0f ", objectInfo.records.at(yy).at(1).c_str (), round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
								if (atoi (objectInfo.records.at(yy).at(3).c_str ()) > 0) {
									sprintf (buffer, "����1(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(4).c_str ())*1000, 0));
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
									sprintf (buffer, "����2(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(5).c_str ())*1000, 0));
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}

							} else if (objectInfo.keyDesc.at(xx).compare ("�������ڳ�") == 0) {
								if (atoi (objectInfo.records.at(yy).at(3).c_str ()) > 0) {
									length = atof (objectInfo.records.at(yy).at(4).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
									length3 = atof (objectInfo.records.at(yy).at(6).c_str ());
									sprintf (buffer, "%.0f / ����(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
								} else {
									length = atof (objectInfo.records.at(yy).at(2).c_str ());
									sprintf (buffer, "%.0f ", round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("���纸 ����� v2") == 0) {
								if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
									length = atof (objectInfo.records.at(yy).at(2).c_str ());
									sprintf (buffer, "����(%.0f) ", round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("�ܿ���") == 0) {
								sprintf (buffer, "����ũ��: %.0f X %.0f / ����ũ��: %.0f X %.0f (���������� �ڸ�: %s)",
									round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(3).c_str ())*1000, 0),
									round (atof (objectInfo.records.at(yy).at(4).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(5).c_str ())*1000, 0),
									(atoi (objectInfo.records.at(yy).at(5).c_str ()) ? "�ڸ�" : "�ڸ��� ����"));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
						
							} else if (objectInfo.keyDesc.at(xx).compare ("PERI���ٸ� ������") == 0) {
								length = atof (objectInfo.records.at(yy).at(2).c_str ());
								if (atoi (objectInfo.records.at(yy).at(3).c_str ()) == 1) {
									sprintf (buffer, "�԰�(%s) ����(%.0f) ũ�ν����(%s) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0), objectInfo.records.at(yy).at(4).c_str ());
								} else {
									sprintf (buffer, "�԰�(%s) ����(%.0f) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("����Ʈ") == 0) {
								length = atof (objectInfo.records.at(yy).at(2).c_str ());
								if (atoi (objectInfo.records.at(yy).at(3).c_str ()) == 1) {
									sprintf (buffer, "�԰�(%s) ����(%.0f) ũ�ν����(%s) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0), objectInfo.records.at(yy).at(4).c_str ());
								} else {
									sprintf (buffer, "�԰�(%s) ����(%.0f) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else {
								for (zz = 0 ; zz < objectInfo.nInfo.at(xx) ; ++zz) {
									// ������ �� ���
									sprintf (buffer, "%s(%s) ", objectInfo.varDesc.at(xx).at(zz).c_str (), objectInfo.records.at(yy).at(zz+1).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}
							}

							// ���� ���
							sprintf (buffer, ": %s EA\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());
							fprintf (fp, buffer);
							fprintf (fp_unite, buffer);
						}
					}
				}
			} catch (exception& ex) {
				WriteReport_Alert ("��� �Լ����� ���� �߻�: %s", ex.what ());
			}

			// �Ϲ� ��� - ��
			//for (xx = 0 ; xx < objectInfo.nCountsBeam ; ++xx) {
			//	if (xx == 0) {
			//		fprintf (fp, "\n[��]\n");
			//	}
			//	sprintf (buffer, "%d : %d EA\n", objectInfo.beamLength [xx], objectInfo.beamCount [xx]);
			//	fprintf (fp, buffer);
			//	fprintf (fp_unite, buffer);
			//}

			// �� �� ���� ��ü
			if (objectInfo.nUnknownObjects > 0) {
				sprintf (buffer, "\n�� �� ���� ��ü : %d EA\n", objectInfo.nUnknownObjects);
				fprintf (fp, buffer);
				fprintf (fp_unite, buffer);
			}

			fclose (fp);

			// ���̾� �����
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}

		// ���� ��Ȳ ǥ���ϴ� ��� - ����
		cur = mm;
		ACAPI_Interface (APIIo_SetProcessValueID, &cur, NULL);
		if (ACAPI_Interface (APIIo_IsProcessCanceledID, NULL, NULL) == APIERR_CANCEL)
			break;
	}

	// ���� ��Ȳ ǥ���ϴ� ��� - ������
	ACAPI_Interface (APIIo_CloseProcessWindowID, NULL, NULL);

	fclose (fp_unite);

	// ��� ���μ����� ��ġ�� ó���� �����ߴ� ���̴� ���̾���� �ٽ� �ѳ��� ��
	for (xx = 1 ; xx <= nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx-1];
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}
		}
	}

	// �׷�ȭ �Ͻ����� OFF
	suspendGroups (false);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	//location.ToDisplayText (&resultString);
	WriteReport_Alert ("������� ���� ��ġ�� �����߽��ϴ�.\n\n%s\n�Ǵ� ������Ʈ ������ �ִ� ����", resultString.ToCStr ().Get ());

	return	err;
}

// ��ü�� ���ڵ� ���� n ����
int		quantityPlusN (vector<vector<string>> *db, vector<string> record, int n)
{
	int		xx, yy;
	size_t	vecLen;
	size_t	inVecLen1, inVecLen2;
	int		diff;
	int		value;
	char	tempStr [512];

	vecLen = db->size ();

	try {
		for (xx = 0 ; xx < vecLen ; ++xx) {
			// ���� ���� ������ ���
			inVecLen1 = db->at(xx).size () - 1;		// ���� ���� �ʵ带 ������ ����
			inVecLen2 = record.size ();

			if (inVecLen1 == inVecLen2) {
				// ��ġ���� �ʴ� �ʵ尡 �ϳ��� �ִ��� ã�ƺ� ��
				diff = 0;
				for (yy = 0 ; yy < inVecLen1 ; ++yy) {
					if (my_strcmp (db->at(xx).at(yy).c_str (), record.at(yy).c_str ()) != 0)
						diff++;
				}

				// ��� �ʵ尡 ��ġ�ϸ�
				if (diff == 0) {
					value = atoi (db->at(xx).back ().c_str ());
					value += n;
					sprintf (tempStr, "%d", value);
					db->at(xx).pop_back ();
					db->at(xx).push_back (tempStr);
					return value;
				}
			}
		}
	} catch (exception& ex) {
		WriteReport_Alert ("quantityPlusN �Լ����� ���� �߻�: %s", ex.what ());
	}

	// ������ �ű� ���ڵ� �߰��ϰ� n ����
	sprintf (tempStr, "%d", n);
	record.push_back (tempStr);
	db->push_back (record);

	return n;
}

// ���纰 ���� �� �����ֱ�
GSErrCode	filterSelection (void)
{
	GSErrCode	err = NoError;
	short		xx, yy;
	short		result;
	const char*	tempStr;
	bool		foundObj;

	FILE	*fp;				// ���� ������
	char	line [10240];		// ���Ͽ��� �о�� ���� �ϳ�
	char	*token;				// �о�� ���ڿ��� ��ū
	short	lineCount;			// �о�� ���� ��
	short	tokCount;			// �о�� ��ū ����
	char	nthToken [200][50];	// n��° ��ū

	API_Element			elem;
	API_ElementMemo		memo;

	// GUID ������ ���� ����
	GS::Array<API_Guid>	objects;	long nObjects	= 0;
	GS::Array<API_Guid>	walls;		long nWalls		= 0;
	GS::Array<API_Guid>	columns;	long nColumns	= 0;
	GS::Array<API_Guid>	beams;		long nBeams		= 0;
	GS::Array<API_Guid>	slabs;		long nSlabs		= 0;
	GS::Array<API_Guid>	roofs;		long nRoofs		= 0;
	GS::Array<API_Guid>	meshes;		long nMeshes	= 0;
	GS::Array<API_Guid>	morphs;		long nMorphs	= 0;
	GS::Array<API_Guid>	shells;		long nShells	= 0;

	// ���ǿ� �´� ��ü���� GUID ����
	GS::Array<API_Guid> selection_known;
	GS::Array<API_Guid> selection_unknown;

	
	// �׷�ȭ �Ͻ����� ON
	suspendGroups (true);

	ACAPI_Element_GetElemList (API_ObjectID, &objects, APIFilt_OnVisLayer);	nObjects = objects.GetSize ();	// ���̴� ���̾� ���� ��ü Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_WallID, &walls, APIFilt_OnVisLayer);		nWalls = walls.GetSize ();		// ���̴� ���̾� ���� �� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_ColumnID, &columns, APIFilt_OnVisLayer);	nColumns = columns.GetSize ();	// ���̴� ���̾� ���� ��� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_BeamID, &beams, APIFilt_OnVisLayer);		nBeams = beams.GetSize ();		// ���̴� ���̾� ���� �� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_SlabID, &slabs, APIFilt_OnVisLayer);		nSlabs = slabs.GetSize ();		// ���̴� ���̾� ���� ������ Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_RoofID, &roofs, APIFilt_OnVisLayer);		nRoofs = roofs.GetSize ();		// ���̴� ���̾� ���� ���� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_MeshID, &meshes, APIFilt_OnVisLayer);	nMeshes = meshes.GetSize ();	// ���̴� ���̾� ���� �޽� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_MorphID, &morphs, APIFilt_OnVisLayer);	nMorphs = morphs.GetSize ();	// ���̴� ���̾� ���� ���� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_ShellID, &shells, APIFilt_OnVisLayer);	nShells = shells.GetSize ();	// ���̴� ���̾� ���� �� Ÿ�Ը� ��������

	if (nObjects == 0 && nWalls == 0 && nColumns == 0 && nBeams == 0 && nSlabs == 0 && nRoofs == 0 && nMeshes == 0 && nMorphs == 0 && nShells == 0) {
		result = DGAlert (DG_INFORMATION, L"���� �˸�", L"�ƹ� ��ü�� �������� �ʽ��ϴ�.", "", L"Ȯ��", "", "");
		return	err;
	}

	// ��ü ���� ���� ��������
	fp = fopen ("C:\\objectInfo.csv", "r");

	if (fp == NULL) {
		result = DGAlert (DG_ERROR, L"���� ����", L"objectInfo.csv ������ C:\\�� �����Ͻʽÿ�.", "", L"Ȯ��", "", "");
		return	err;
	}

	lineCount = 0;

	while (!feof (fp)) {
		tokCount = 0;
		fgets (line, sizeof (line), fp);

		token = strtok (line, ",");
		tokCount ++;
		lineCount ++;

		// �� ���ξ� ó��
		while (token != NULL) {
			if (strlen (token) > 0) {
				strncpy (nthToken [tokCount-1], token, strlen (token)+1);
			}
			token = strtok (NULL, ",");
			tokCount ++;
		}

		sprintf (visibleObjInfo.varName [lineCount-1], "%s", nthToken [0]);
		sprintf (visibleObjInfo.objName [lineCount-1], "%s", nthToken [1]);
	}

	visibleObjInfo.nKinds = lineCount;

	// ���� ���� �׸��� 2�� �� �� �����Ƿ� �ߺ� ����
	if (lineCount >= 2) {
		if (my_strcmp (visibleObjInfo.varName [lineCount-1], visibleObjInfo.varName [lineCount-2]) == 0) {
			visibleObjInfo.nKinds --;
		}
	}

	// ���� �ݱ�
	fclose (fp);

	// ���� ����, ǥ�� ���� �ʱ�ȭ
	for (xx = 0 ; xx < 50 ; ++xx) {
		visibleObjInfo.bExist [xx] = false;
		visibleObjInfo.bShow [xx] = false;
	}
	visibleObjInfo.bExist_Walls = false;
	visibleObjInfo.bShow_Walls = false;
	visibleObjInfo.bExist_Columns = false;
	visibleObjInfo.bShow_Columns = false;
	visibleObjInfo.bExist_Beams = false;
	visibleObjInfo.bShow_Beams = false;
	visibleObjInfo.bExist_Slabs = false;
	visibleObjInfo.bShow_Slabs = false;
	visibleObjInfo.bExist_Roofs = false;
	visibleObjInfo.bShow_Roofs = false;
	visibleObjInfo.bExist_Meshes = false;
	visibleObjInfo.bShow_Meshes = false;
	visibleObjInfo.bExist_Morphs = false;
	visibleObjInfo.bShow_Morphs = false;
	visibleObjInfo.bExist_Shells = false;
	visibleObjInfo.bShow_Shells = false;

	// ���� ���� üũ
	for (xx = 0 ; xx < nObjects ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objects [xx];
		err = ACAPI_Element_Get (&elem);

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (err == NoError) {
				foundObj = false;

				for (yy = 0 ; yy < visibleObjInfo.nKinds ; ++yy) {
					tempStr = getParameterStringByName (&memo, visibleObjInfo.varName [yy]);
					if (tempStr != NULL) {
						if (my_strcmp (tempStr, visibleObjInfo.objName [yy]) == 0) {
							visibleObjInfo.bExist [yy] = true;
							foundObj = true;
						}
					}
				}

				// ���� ã�� ���ϸ� �˷����� ���� Object Ÿ�� ����Ʈ�� �߰�
				if (foundObj == false)
					selection_unknown.Push (objects [xx]);
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}
	}
	
	visibleObjInfo.nUnknownObjects = selection_unknown.GetSize ();

	if (nWalls > 0)		visibleObjInfo.bExist_Walls = true;
	if (nColumns > 0)	visibleObjInfo.bExist_Columns = true;
	if (nBeams > 0)		visibleObjInfo.bExist_Beams = true;
	if (nSlabs > 0)		visibleObjInfo.bExist_Slabs = true;
	if (nRoofs > 0)		visibleObjInfo.bExist_Roofs = true;
	if (nMeshes > 0)	visibleObjInfo.bExist_Meshes = true;
	if (nMorphs > 0)	visibleObjInfo.bExist_Morphs = true;
	if (nShells > 0)	visibleObjInfo.bExist_Shells = true;

	visibleObjInfo.nItems = visibleObjInfo.nKinds +
		(visibleObjInfo.bExist_Walls * 1) +
		(visibleObjInfo.bExist_Columns * 1) +
		(visibleObjInfo.bExist_Beams * 1) +
		(visibleObjInfo.bExist_Slabs * 1) +
		(visibleObjInfo.bExist_Roofs * 1) +
		(visibleObjInfo.bExist_Meshes * 1) +
		(visibleObjInfo.bExist_Morphs * 1) +
		(visibleObjInfo.bExist_Shells * 1);

	// [DIALOG] ���̾�α׿��� ���̴� ���̾� �� �ִ� ��ü���� ������ �����ְ�, üũ�� ������ ��ü�鸸 ���� �� ������
	result = DGBlankModalDialog (750, 500, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, filterSelectionHandler, 0);

	if (result == DG_OK) {
		// ������ ���ǿ� �ش��ϴ� ��ü�� �����ϱ�
		for (xx = 0 ; xx < nObjects ; ++xx) {
			BNZeroMemory (&elem, sizeof (API_Element));
			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			elem.header.guid = objects [xx];
			err = ACAPI_Element_Get (&elem);
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			for (yy = 0 ; yy < visibleObjInfo.nKinds ; ++yy) {
				tempStr = getParameterStringByName (&memo, visibleObjInfo.varName [yy]);
				
				if (tempStr != NULL) {
					if ((my_strcmp (tempStr, visibleObjInfo.objName [yy]) == 0) && (visibleObjInfo.bShow [yy] == true)) {
						selection_known.Push (objects [xx]);
					}
				}
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}

		// �˷��� Object Ÿ�� ����
		selectElements (selection_known);

		// �˷����� ���� Object Ÿ�� ����
		if (visibleObjInfo.bShow_Unknown == true)	selectElements (selection_unknown);

		// ������ Ÿ��
		if (visibleObjInfo.bShow_Walls == true)		selectElements (walls);
		if (visibleObjInfo.bShow_Columns == true)	selectElements (columns);
		if (visibleObjInfo.bShow_Beams == true)		selectElements (beams);
		if (visibleObjInfo.bShow_Slabs == true)		selectElements (slabs);
		if (visibleObjInfo.bShow_Roofs == true)		selectElements (roofs);
		if (visibleObjInfo.bShow_Meshes == true)	selectElements (meshes);
		if (visibleObjInfo.bShow_Morphs == true)	selectElements (morphs);
		if (visibleObjInfo.bShow_Shells == true)	selectElements (shells);
		
		// ������ �͸� 3D�� �����ֱ�
		ACAPI_Automate (APIDo_ShowSelectionIn3DID, NULL, NULL);
	}

	// �׷�ȭ �Ͻ����� OFF
	suspendGroups (false);

	return	err;
}

// �� �迭 ���� ������
BeamTableformCellArray::BeamTableformCellArray () {
	objectInBeamTableform	initCell;
	initCell.objType = NONE;
	initCell.attachPosition = LEFT_SIDE;
	initCell.origin.x = 0.0;	initCell.origin.y = 0.0;	initCell.origin.z = 0.0;
	initCell.minPos.x = 0.0;	initCell.minPos.y = 0.0;	initCell.minPos.z = 0.0;
	initCell.maxPos.x = 0.0;	initCell.maxPos.y = 0.0;	initCell.maxPos.z = 0.0;
	initCell.width = 0.0;
	initCell.length = 0.0;
	initCell.beginPos = 0.0;
	initCell.endPos = 0.0;

	this->iBeamDirection = HORIZONTAL_DIRECTION;
	this->nCells_Left = 0;
	this->nCells_Right = 0;
	this->nCells_Bottom = 0;

	for (short xx = 0 ; xx < 2 ; ++xx) {
		this->cellElev [xx] = 0.0;
		this->cellPos [xx] = 0.0;

		for (short yy = 0 ; yy < 30 ; ++yy) {
			this->cells_Left [yy][xx] = initCell;
			this->cells_Right [yy][xx] = initCell;
			this->cells_Bottom [yy][xx] = initCell;
		}
	}
}

// �� �迭 ���� �ʱ�ȭ
void BeamTableformCellArray::init ()
{
	objectInBeamTableform	initCell;
	initCell.objType = NONE;
	initCell.attachPosition = LEFT_SIDE;
	initCell.origin.x = 0.0;	initCell.origin.y = 0.0;	initCell.origin.z = 0.0;
	initCell.minPos.x = 0.0;	initCell.minPos.y = 0.0;	initCell.minPos.z = 0.0;
	initCell.maxPos.x = 0.0;	initCell.maxPos.y = 0.0;	initCell.maxPos.z = 0.0;
	initCell.width = 0.0;
	initCell.length = 0.0;
	initCell.beginPos = 0.0;
	initCell.endPos = 0.0;

	this->iBeamDirection = HORIZONTAL_DIRECTION;
	this->nCells_Left = 0;
	this->nCells_Right = 0;
	this->nCells_Bottom = 0;

	for (short xx = 0 ; xx < 2 ; ++xx) {
		this->cellElev [xx] = 0.0;
		this->cellPos [xx] = 0.0;

		for (short yy = 0 ; yy < 30 ; ++yy) {
			this->cells_Left [yy][xx] = initCell;
			this->cells_Right [yy][xx] = initCell;
			this->cells_Bottom [yy][xx] = initCell;
		}
	}
}

// �� ���̺��� ���� ���� ��������
GSErrCode	exportBeamTableformInformation (void)
{
	GSErrCode	err = NoError;
	unsigned short		xx;
	short	mm;
	//bool	regenerate = true;

	GS::Array<API_Guid>		objects;
	long					nObjects = 0;
	long					nObjects_Left = 0;
	long					nObjects_Right = 0;
	long					nObjects_Bottom = 0;

	API_Element			elem;
	API_ElementMemo		memo;
	API_ElemInfo3D		info3D;

	vector<objectInBeamTableform>	objectList;
	vector<objectInBeamTableform>	objectList_Left;
	vector<objectInBeamTableform>	objectList_Right;
	vector<objectInBeamTableform>	objectList_Bottom;

	objectInBeamTableform			newObject;
	BeamTableformCellArray			tableformInfo;

	double				xmin, xmax, ymin, ymax;
	int					ang_x, ang_y;
	bool				bValid, bFirst;
	double				xcenter, ycenter;

	// ���̾� ���� ����
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [2048];
	char			fullLayerName [512];
	vector<LayerList>	layerList;

	// ��Ÿ
	char			tempStr [512];
	char			buffer_s [512];
	char			buffer [1024];
	char			filename [512];

	// ���� ���Ϸ� ��� ���� ��������
	// ���� ������ ���� ����
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp;


	// �׷�ȭ �Ͻ����� ON
	suspendGroups (true);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

	// ������Ʈ �� ���̾� ������ �˾Ƴ�
	nLayers = getLayerCount ();

	// ���̴� ���̾���� ��� �����ϱ�
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList [nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// ���̾� �̸��� �ε��� ����
	for (xx = 0 ; xx < nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx];
		err = ACAPI_Attribute_Get (&attrib);

		sprintf (fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName [strlen (fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList [xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back (newLayerItem);
	}

	// ���̾� �̸� �������� �����Ͽ� ���̾� �ε��� ���� ����
	sort (layerList.begin (), layerList.end (), compareLayerName);		// ���̾� �̸� ���� �������� ����

	// �Ͻ������� ��� ���̾� �����
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

	ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
	sprintf (filename, "%s - �� ���̺��� ����ǥ.csv", miscAppInfo.caption);
	fp = fopen (filename, "w+");

	if (fp == NULL) {
		DGAlert (DG_ERROR, L"����", L"���������� ���� �� �����ϴ�.", "", L"Ȯ��", "", "");
		return	NoError;
	}

	// ���̴� ���̾���� �ϳ��� ��ȸ�ϸ鼭 ��ü ��ҵ��� ������ �� "�� ���̺��� ����ǥ" ��ƾ ����
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList [mm-1].layerInd;
		err = ACAPI_Attribute_Get (&attrib);

		if (err == NoError) {
			// ���̾� ���̱�
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// �ʱ�ȭ
			objectList.clear ();
			objectList_Left.clear ();
			objectList_Right.clear ();
			objectList_Bottom.clear ();
			tableformInfo.init ();

			// ��� ��� ��������
			ACAPI_Element_GetElemList (API_ObjectID, &objects, APIFilt_OnVisLayer);	// ���̴� ���̾ ����, ��ü Ÿ�Ը�
			nObjects = objects.GetSize ();

			if (nObjects == 0)
				continue;

			// ���̾� �̸� ������
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			for (xx = 0 ; xx < nObjects ; ++xx) {
				BNZeroMemory (&elem, sizeof (API_Element));
				BNZeroMemory (&memo, sizeof (API_ElementMemo));
				elem.header.guid = objects [xx];
				err = ACAPI_Element_Get (&elem);
				err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

				if (err == NoError && elem.header.hasMemo) {
					err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

					if (err == NoError) {
						bValid = false;

						// �ʱ�ȭ
						newObject.attachPosition = NO_POSITION;
						newObject.objType = NONE;
						newObject.length = 0.0;
						newObject.width = 0.0;
						newObject.length = 0.0;
						newObject.origin.x = 0.0;
						newObject.origin.y = 0.0;
						newObject.origin.z = 0.0;
						newObject.minPos.x = 0.0;
						newObject.minPos.y = 0.0;
						newObject.minPos.z = 0.0;
						newObject.maxPos.x = 0.0;
						newObject.maxPos.y = 0.0;
						newObject.maxPos.z = 0.0;

						// ���� ��ǥ ����
						newObject.origin.x = elem.object.pos.x;
						newObject.origin.y = elem.object.pos.y;
						newObject.origin.z = elem.object.level;

						// �ּ���, �ִ��� ��ǥ ����
						newObject.minPos.x = info3D.bounds.xMin;
						newObject.minPos.y = info3D.bounds.yMin;
						newObject.minPos.z = info3D.bounds.zMin;
						newObject.maxPos.x = info3D.bounds.xMax;
						newObject.maxPos.y = info3D.bounds.yMax;
						newObject.maxPos.z = info3D.bounds.zMax;
						
						// ��ü�� Ÿ��, �ʺ�� ���̸� ����
						if (my_strcmp (getParameterStringByName (&memo, "u_comp"), "������") == 0) {
							if (my_strcmp (getParameterStringByName (&memo, "u_ins"), "��������") == 0) {
								newObject.objType = EUROFORM;

								sprintf (tempStr, "%s", getParameterStringByName (&memo, "eu_wid"));
								newObject.width = atof (tempStr) / 1000.0;
								sprintf (tempStr, "%s", getParameterStringByName (&memo, "eu_hei"));
								newObject.length = atof (tempStr) / 1000.0;

								ang_x = (int)round (RadToDegree (getParameterValueByName (&memo, "ang_x")), 0);
								ang_y = (int)round (RadToDegree (getParameterValueByName (&memo, "ang_y")), 0);

								if (ang_x == 90)
									newObject.attachPosition = LEFT_SIDE;
								else if (ang_x == 0)
									newObject.attachPosition = BOTTOM_SIDE;

								bValid = true;
							}
						}

						else if (my_strcmp (getParameterStringByName (&memo, "g_comp"), "����") == 0) {
							newObject.objType = PLYWOOD;

							ang_x = (int)round (RadToDegree (getParameterValueByName (&memo, "p_ang")), 0);

							if (my_strcmp (getParameterStringByName (&memo, "w_dir"), "��������") == 0) {
								newObject.width = getParameterValueByName (&memo, "p_wid");
								newObject.length = getParameterValueByName (&memo, "p_leng");
								newObject.attachPosition = LEFT_SIDE;

								bValid = true;
							} else if (my_strcmp (getParameterStringByName (&memo, "w_dir"), "�������") == 0) {
								newObject.width = getParameterValueByName (&memo, "p_leng");
								newObject.length = getParameterValueByName (&memo, "p_wid");
								newObject.attachPosition = LEFT_SIDE;

								bValid = true;
							} else if (my_strcmp (getParameterStringByName (&memo, "w_dir"), "�ٴڱ��") == 0) {
								newObject.width = getParameterValueByName (&memo, "p_wid");
								newObject.length = getParameterValueByName (&memo, "p_leng");
								newObject.attachPosition = BOTTOM_SIDE;

								bValid = true;
							}
						}

						else if (my_strcmp (getParameterStringByName (&memo, "g_comp"), "����(�ٰ���)") == 0) {
							newObject.objType = PLYWOOD_POLY;

							if (my_strcmp (getParameterStringByName (&memo, "w_dir"), "�������") == 0) {
								newObject.width = 0.0;
								newObject.length = 0.0;
								newObject.attachPosition = LEFT_SIDE;

								bValid = true;
							} else if (my_strcmp (getParameterStringByName (&memo, "w_dir"), "�ٴڱ��") == 0) {
								newObject.width = 0.0;
								newObject.length = 0.0;
								newObject.attachPosition = BOTTOM_SIDE;

								bValid = true;
							}
						}

						if (bValid == true)
							objectList.push_back (newObject);
					}

					ACAPI_DisposeElemMemoHdls (&memo);
				}
			}

			nObjects = (long)objectList.size ();

			// �� ������ ã�Ƴ� (�����ΰ�, �����ΰ�?)
			bFirst = false;
			for (xx = 0 ; xx < nObjects ; ++xx) {
				if (objectList [xx].attachPosition != BOTTOM_SIDE) {
					if (bFirst == false) {
						xmin = xmax = objectList [xx].origin.x;
						ymin = ymax = objectList [xx].origin.y;
						bFirst = true;
					} else {
						if (xmin > objectList [xx].origin.x)	xmin = objectList [xx].origin.x;
						if (ymin > objectList [xx].origin.y)	ymin = objectList [xx].origin.y;
						if (xmax < objectList [xx].origin.x)	xmax = objectList [xx].origin.x;
						if (ymax < objectList [xx].origin.y)	ymax = objectList [xx].origin.y;
					}
				}
			}
			if (abs (xmax - xmin) > abs (ymax - ymin))
				tableformInfo.iBeamDirection = HORIZONTAL_DIRECTION;
			else
				tableformInfo.iBeamDirection = VERTICAL_DIRECTION;

			// ��ü �����ϱ�
			for (xx = 0 ; xx < nObjects ; ++xx) {
				if (tableformInfo.iBeamDirection == HORIZONTAL_DIRECTION)
					sort (objectList.begin (), objectList.end (), comparePosX);		// X �������� ����
				else
					sort (objectList.begin (), objectList.end (), comparePosY);		// Y �������� ����
			}

			// ���� ��ġ ã��
			xcenter = (xmax - xmin) / 2 + xmin;
			ycenter = (ymax - ymin) / 2 + ymin;

			// ������ ���� �з��ϱ�
			for (xx = 0 ; xx < nObjects ; ++xx) {
				// ����/�Ʒ���(�ּ�)�� ������/����(�ִ�) ���� ������ �߰����� �������� ����
				if (tableformInfo.iBeamDirection == HORIZONTAL_DIRECTION) {
					if (objectList [xx].origin.y > ycenter) {
						if (objectList [xx].attachPosition == LEFT_SIDE)
							objectList [xx].attachPosition = RIGHT_SIDE;	// ����
					}
				} else {
					if (objectList [xx].origin.x > xcenter) {
						if (objectList [xx].attachPosition == LEFT_SIDE)
							objectList [xx].attachPosition = RIGHT_SIDE;	// ������
					}
				}
			}

			// ������� ������/���� ��ǥ�� ������Ʈ
			for (xx = 0 ; xx < nObjects ; ++xx) {
				if (tableformInfo.iBeamDirection == HORIZONTAL_DIRECTION) {
					// ���� ����
					objectList [xx].beginPos = objectList [xx].minPos.x;
					objectList [xx].endPos = objectList [xx].beginPos + objectList [xx].length;
				} else {
					// ���� ����
					objectList [xx].beginPos = objectList [xx].minPos.y;
					objectList [xx].endPos = objectList [xx].beginPos + objectList [xx].length;
				}
			}

			// ������� �ּ����� �������� �������� ��ĵ (���� ���� ���� 1�� ����, ���� ū ���� 2�� �������� ����)
			double	lowElev, highElev;
			lowElev = highElev = objectList [0].minPos.z;

			for (xx = 0 ; xx < nObjects ; ++xx) {
				if (objectList [xx].minPos.z < lowElev)		lowElev = objectList [xx].minPos.z;
				if (objectList [xx].minPos.z > highElev)	highElev = objectList [xx].minPos.z;
			}

			tableformInfo.cellElev [0] = lowElev;
			tableformInfo.cellElev [1] = highElev;

			// ������� �ּ����� �������� X,Y���� ��ĵ
			if (tableformInfo.iBeamDirection == VERTICAL_DIRECTION) {
				lowElev = highElev = objectList [0].minPos.x;

				for (xx = 0 ; xx < nObjects ; ++xx) {
					if (objectList [xx].minPos.x < lowElev)		lowElev = objectList [xx].minPos.x;
					if (objectList [xx].minPos.x > highElev)	highElev = objectList [xx].minPos.x;
				}
			} else {
				lowElev = highElev = objectList [0].minPos.y;

				for (xx = 0 ; xx < nObjects ; ++xx) {
					if (objectList [xx].minPos.y > lowElev)		lowElev = objectList [xx].minPos.y;
					if (objectList [xx].minPos.y < highElev)	highElev = objectList [xx].minPos.y;
				}
			}

			tableformInfo.cellPos [0] = lowElev;
			tableformInfo.cellPos [1] = highElev;

			// ������� ��ġ���� �з���
			for (xx = 0 ; xx < nObjects ; ++xx) {
				if (objectList [xx].attachPosition == LEFT_SIDE)	objectList_Left.push_back (objectList [xx]);
				if (objectList [xx].attachPosition == RIGHT_SIDE)	objectList_Right.push_back (objectList [xx]);
				if (objectList [xx].attachPosition == BOTTOM_SIDE)	objectList_Bottom.push_back (objectList [xx]);
			}
			nObjects_Left = (long)objectList_Left.size ();
			nObjects_Right = (long)objectList_Right.size ();
			nObjects_Bottom = (long)objectList_Bottom.size ();

			// �� �迭 ���� ���� - ������
			double	curCell, adjCell;
			xx = 0;
			while (xx < nObjects_Left) {
				// ������ ���� �ƴ� ���
				if (xx < nObjects_Left - 1) {
					curCell = objectList_Left [xx].minPos.z;
					adjCell = objectList_Left [xx+1].minPos.z;

					if ((objectList_Left [xx].objType == objectList_Left [xx+1].objType) && ((abs (objectList_Left [xx].beginPos - objectList_Left [xx+1].beginPos) < 0.080) || (abs (objectList_Left [xx].endPos - objectList_Left [xx+1].endPos) < 0.080))) {
						// ����/���� ���� ������-������ ��ġ�� ���
						if (curCell < adjCell) {
							tableformInfo.cells_Left [tableformInfo.nCells_Left][0] = objectList_Left [xx];
							tableformInfo.cells_Left [tableformInfo.nCells_Left][1] = objectList_Left [xx+1];
						} else {
							tableformInfo.cells_Left [tableformInfo.nCells_Left][0] = objectList_Left [xx+1];
							tableformInfo.cells_Left [tableformInfo.nCells_Left][1] = objectList_Left [xx];
						}

						xx += 2;
						tableformInfo.nCells_Left ++;
					} else {
						// ����/���� ���� ������-������ ��ġ���� ���� ���
						if (abs (tableformInfo.cellElev [0] - curCell) < abs (tableformInfo.cellElev [1] - curCell))
							tableformInfo.cells_Left [tableformInfo.nCells_Left][0] = objectList_Left [xx];
						else
							tableformInfo.cells_Left [tableformInfo.nCells_Left][1] = objectList_Left [xx];

						xx ++;
						tableformInfo.nCells_Left ++;
					}

				// ������ ���� ���
				} else {
					curCell = objectList_Left [xx].minPos.z;
					adjCell = objectList_Left [xx-1].minPos.z;

					if ((objectList_Left [xx].objType == objectList_Left [xx-1].objType) && ((abs (objectList_Left [xx].beginPos - objectList_Left [xx-1].beginPos) < 0.080) || (abs (objectList_Left [xx].endPos - objectList_Left [xx-1].endPos) < 0.080))) {
						// ����/���� ���� ������-������ ��ġ�� ���
						if (abs (tableformInfo.cellElev [0] - curCell) < abs (tableformInfo.cellElev [1] - curCell))
							tableformInfo.cells_Left [tableformInfo.nCells_Left - 1][0] = objectList_Left [xx];
						else
							tableformInfo.cells_Left [tableformInfo.nCells_Left - 1][1] = objectList_Left [xx];

						xx ++;
					} else {
						// ����/���� ���� ������-������ ��ġ���� ���� ���
						if (abs (tableformInfo.cellElev [0] - curCell) < abs (tableformInfo.cellElev [1] - curCell))
							tableformInfo.cells_Left [tableformInfo.nCells_Left][0] = objectList_Left [xx];
						else
							tableformInfo.cells_Left [tableformInfo.nCells_Left][1] = objectList_Left [xx];

						xx ++;
						tableformInfo.nCells_Left ++;
					}
				}
			}

			// �� �迭 ���� ���� - ������
			xx = 0;
			while (xx < nObjects_Right) {
				// ������ ���� �ƴ� ���
				if (xx < nObjects_Right - 1) {
					curCell = objectList_Right [xx].minPos.z;
					adjCell = objectList_Right [xx+1].minPos.z;

					if ((objectList_Right [xx].objType == objectList_Right [xx+1].objType) && ((abs (objectList_Right [xx].beginPos - objectList_Right [xx+1].beginPos) < 0.080) || (abs (objectList_Right [xx].endPos - objectList_Right [xx+1].endPos) < 0.080))) {
						// ����/���� ���� ������-������ ��ġ�� ���
						if (curCell < adjCell) {
							tableformInfo.cells_Right [tableformInfo.nCells_Right][0] = objectList_Right [xx];
							tableformInfo.cells_Right [tableformInfo.nCells_Right][1] = objectList_Right [xx+1];
						} else {
							tableformInfo.cells_Right [tableformInfo.nCells_Right][0] = objectList_Right [xx+1];
							tableformInfo.cells_Right [tableformInfo.nCells_Right][1] = objectList_Right [xx];
						}

						xx += 2;
						tableformInfo.nCells_Right ++;
					} else {
						// ����/���� ���� ������-������ ��ġ���� ���� ���
						if (abs (tableformInfo.cellElev [0] - curCell) < abs (tableformInfo.cellElev [1] - curCell))
							tableformInfo.cells_Right [tableformInfo.nCells_Right][0] = objectList_Right [xx];
						else
							tableformInfo.cells_Right [tableformInfo.nCells_Right][1] = objectList_Right [xx];

						xx ++;
						tableformInfo.nCells_Right ++;
					}

				// ������ ���� ���
				} else {
					curCell = objectList_Right [xx].minPos.z;
					adjCell = objectList_Right [xx-1].minPos.z;

					if ((objectList_Right [xx].objType == objectList_Right [xx-1].objType) && ((abs (objectList_Right [xx].beginPos - objectList_Right [xx-1].beginPos) < 0.080) || (abs (objectList_Right [xx].endPos - objectList_Right [xx-1].endPos) < 0.080))) {
						// ����/���� ���� ������-������ ��ġ�� ���
						if (abs (tableformInfo.cellElev [0] - curCell) < abs (tableformInfo.cellElev [1] - curCell))
							tableformInfo.cells_Right [tableformInfo.nCells_Right - 1][0] = objectList_Right [xx];
						else
							tableformInfo.cells_Right [tableformInfo.nCells_Right - 1][1] = objectList_Right [xx];

						xx ++;
					} else {
						// ����/���� ���� ������-������ ��ġ���� ���� ���
						if (abs (tableformInfo.cellElev [0] - curCell) < abs (tableformInfo.cellElev [1] - curCell))
							tableformInfo.cells_Right [tableformInfo.nCells_Right][0] = objectList_Right [xx];
						else
							tableformInfo.cells_Right [tableformInfo.nCells_Right][1] = objectList_Right [xx];

						xx ++;
						tableformInfo.nCells_Right ++;
					}
				}
			}

			// �� �迭 ���� ���� - �Ϻθ�
			xx = 0;
			if (tableformInfo.iBeamDirection == VERTICAL_DIRECTION) {
				while (xx < nObjects_Bottom) {
					// ������ ���� �ƴ� ���
					if (xx < nObjects_Bottom - 1) {
						curCell = objectList_Bottom [xx].minPos.x;
						adjCell = objectList_Bottom [xx+1].minPos.x;

						if ((objectList_Bottom [xx].objType == objectList_Bottom [xx+1].objType) && ((abs (objectList_Bottom [xx].beginPos - objectList_Bottom [xx+1].beginPos) < 0.080) || (abs (objectList_Bottom [xx].endPos - objectList_Bottom [xx+1].endPos) < 0.080))) {
							// ����/���� ���� ������-������ ��ġ�� ���
							if (curCell < adjCell) {
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][0] = objectList_Bottom [xx];
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][1] = objectList_Bottom [xx+1];
							} else {
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][0] = objectList_Bottom [xx+1];
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][1] = objectList_Bottom [xx];
							}

							xx += 2;
							tableformInfo.nCells_Bottom ++;
						} else {
							// ����/���� ���� ������-������ ��ġ���� ���� ���
							if (abs (tableformInfo.cellPos [0] - curCell) < abs (tableformInfo.cellPos [1] - curCell))
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][0] = objectList_Bottom [xx];
							else
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][1] = objectList_Bottom [xx];

							xx ++;
							tableformInfo.nCells_Bottom ++;
						}

					// ������ ���� ���
					} else {
						curCell = objectList_Bottom [xx].minPos.x;
						adjCell = objectList_Bottom [xx-1].minPos.x;

						if ((objectList_Bottom [xx].objType == objectList_Bottom [xx-1].objType) && ((abs (objectList_Bottom [xx].beginPos - objectList_Bottom [xx-1].beginPos) < 0.080) || (abs (objectList_Bottom [xx].endPos - objectList_Bottom [xx-1].endPos) < 0.080))) {
							// ����/���� ���� ������-������ ��ġ�� ���
							if (abs (tableformInfo.cellPos [0] - curCell) < abs (tableformInfo.cellPos [1] - curCell))
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom - 1][0] = objectList_Bottom [xx];
							else
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom - 1][1] = objectList_Bottom [xx];

							xx ++;
						} else {
							// ����/���� ���� ������-������ ��ġ���� ���� ���
							if (abs (tableformInfo.cellPos [0] - curCell) < abs (tableformInfo.cellPos [1] - curCell))
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][0] = objectList_Bottom [xx];
							else
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][1] = objectList_Bottom [xx];

							xx ++;
							tableformInfo.nCells_Bottom ++;
						}
					}
				}
			} else {
				while (xx < nObjects_Bottom) {
					// ������ ���� �ƴ� ���
					if (xx < nObjects_Bottom - 1) {
						curCell = objectList_Bottom [xx].minPos.y;
						adjCell = objectList_Bottom [xx+1].minPos.y;

						if ((objectList_Bottom [xx].objType == objectList_Bottom [xx+1].objType) && ((abs (objectList_Bottom [xx].beginPos - objectList_Bottom [xx+1].beginPos) < 0.080) || (abs (objectList_Bottom [xx].endPos - objectList_Bottom [xx+1].endPos) < 0.080))) {
							// ����/���� ���� ������-������ ��ġ�� ���
							if (curCell < adjCell) {
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][1] = objectList_Bottom [xx];
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][0] = objectList_Bottom [xx+1];
							} else {
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][1] = objectList_Bottom [xx+1];
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][0] = objectList_Bottom [xx];
							}

							xx += 2;
							tableformInfo.nCells_Bottom ++;
						} else {
							// ����/���� ���� ������-������ ��ġ���� ���� ���
							if (abs (tableformInfo.cellPos [0] - curCell) < abs (tableformInfo.cellPos [1] - curCell))
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][1] = objectList_Bottom [xx];
							else
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][0] = objectList_Bottom [xx];

							xx ++;
							tableformInfo.nCells_Bottom ++;
						}

					// ������ ���� ���
					} else {
						curCell = objectList_Bottom [xx].minPos.x;
						adjCell = objectList_Bottom [xx-1].minPos.x;

						if ((objectList_Bottom [xx].objType == objectList_Bottom [xx-1].objType) && ((abs (objectList_Bottom [xx].beginPos - objectList_Bottom [xx-1].beginPos) < 0.080) || (abs (objectList_Bottom [xx].endPos - objectList_Bottom [xx-1].endPos) < 0.080))) {
							// ����/���� ���� ������-������ ��ġ�� ���
							if (abs (tableformInfo.cellPos [0] - curCell) < abs (tableformInfo.cellPos [1] - curCell))
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom - 1][1] = objectList_Bottom [xx];
							else
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom - 1][0] = objectList_Bottom [xx];

							xx ++;
						} else {
							// ����/���� ���� ������-������ ��ġ���� ���� ���
							if (abs (tableformInfo.cellPos [0] - curCell) < abs (tableformInfo.cellPos [1] - curCell))
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][1] = objectList_Bottom [xx];
							else
								tableformInfo.cells_Bottom [tableformInfo.nCells_Bottom][0] = objectList_Bottom [xx];

							xx ++;
							tableformInfo.nCells_Bottom ++;
						}
					}
				}
			}

			// ���� ���
			if (tableformInfo.nCells_Left + tableformInfo.nCells_Right + tableformInfo.nCells_Bottom > 0) {
				sprintf (buffer, "<< ���̾� : %s >>\n", fullLayerName);
				fprintf (fp, buffer);

				sprintf (buffer, "%s\n\n", getExplanationOfLayerCode (fullLayerName));
				fprintf (fp, buffer);

				if (tableformInfo.nCells_Left > 0) {
					fprintf (fp, "����1\n");
					// ���
					strcpy (buffer, "");
					for (xx = 0 ; xx < tableformInfo.nCells_Left ; ++xx) {
						if (tableformInfo.cells_Left [xx][0].objType != NONE) {
							if (tableformInfo.cells_Left [xx][0].objType == EUROFORM)
								strcat (buffer, "������,");
							if (tableformInfo.cells_Left [xx][0].objType == PLYWOOD)
								strcat (buffer, "����,");
							if (tableformInfo.cells_Left [xx][0].objType == PLYWOOD_POLY)
								strcat (buffer, "����(�ٰ���),");
						} else if (tableformInfo.cells_Left [xx][1].objType != NONE) {
							if (tableformInfo.cells_Left [xx][1].objType == EUROFORM)
								strcat (buffer, "������,");
							if (tableformInfo.cells_Left [xx][1].objType == PLYWOOD)
								strcat (buffer, "����,");
							if (tableformInfo.cells_Left [xx][1].objType == PLYWOOD_POLY)
								strcat (buffer, "����(�ٰ���),");
						}
					}
					// 2��
					strcat (buffer, "\n");
					for (xx = 0 ; xx < tableformInfo.nCells_Left ; ++xx) {
						if (tableformInfo.cells_Left [xx][1].objType != NONE) {
							sprintf (buffer_s, "%.0f x %.0f,", tableformInfo.cells_Left [xx][1].width * 1000, tableformInfo.cells_Left [xx][1].length * 1000);
							strcat (buffer, buffer_s);
						} else {
							sprintf (buffer_s, "��,");
							strcat (buffer, buffer_s);
						}
					}
					// 1��
					strcat (buffer, "\n");
					for (xx = 0 ; xx < tableformInfo.nCells_Left ; ++xx) {
						if (tableformInfo.cells_Left [xx][0].objType != NONE) {
							sprintf (buffer_s, "%.0f x %.0f,", tableformInfo.cells_Left [xx][0].width * 1000, tableformInfo.cells_Left [xx][0].length * 1000);
							strcat (buffer, buffer_s);
						} else {
							sprintf (buffer_s, "��,");
							strcat (buffer, buffer_s);
						}
					}
					strcat (buffer, "\n\n");
					fprintf (fp, buffer);
				}

				if (tableformInfo.nCells_Right > 0) {
					fprintf (fp, "����2\n");
					// ���
					strcpy (buffer, "");
					for (xx = 0 ; xx < tableformInfo.nCells_Right ; ++xx) {
						if (tableformInfo.cells_Right [xx][0].objType != NONE) {
							if (tableformInfo.cells_Right [xx][0].objType == EUROFORM)
								strcat (buffer, "������,");
							if (tableformInfo.cells_Right [xx][0].objType == PLYWOOD)
								strcat (buffer, "����,");
							if (tableformInfo.cells_Right [xx][0].objType == PLYWOOD_POLY)
								strcat (buffer, "����(�ٰ���),");
						} else if (tableformInfo.cells_Right [xx][1].objType != NONE) {
							if (tableformInfo.cells_Right [xx][1].objType == EUROFORM)
								strcat (buffer, "������,");
							if (tableformInfo.cells_Right [xx][1].objType == PLYWOOD)
								strcat (buffer, "����,");
							if (tableformInfo.cells_Right [xx][1].objType == PLYWOOD_POLY)
								strcat (buffer, "����(�ٰ���),");
						}
					}
					// 2��
					strcat (buffer, "\n");
					for (xx = 0 ; xx < tableformInfo.nCells_Right ; ++xx) {
						if (tableformInfo.cells_Right [xx][1].objType != NONE) {
							sprintf (buffer_s, "%.0f x %.0f,", tableformInfo.cells_Right [xx][1].width * 1000, tableformInfo.cells_Right [xx][1].length * 1000);
							strcat (buffer, buffer_s);
						} else {
							sprintf (buffer_s, "��,");
							strcat (buffer, buffer_s);
						}
					}
					// 1��
					strcat (buffer, "\n");
					for (xx = 0 ; xx < tableformInfo.nCells_Right ; ++xx) {
						if (tableformInfo.cells_Right [xx][0].objType != NONE) {
							sprintf (buffer_s, "%.0f x %.0f,", tableformInfo.cells_Right [xx][0].width * 1000, tableformInfo.cells_Right [xx][0].length * 1000);
							strcat (buffer, buffer_s);
						} else {
							sprintf (buffer_s, "��,");
							strcat (buffer, buffer_s);
						}
					}
					strcat (buffer, "\n\n");
					fprintf (fp, buffer);
				}

				if (tableformInfo.nCells_Bottom > 0) {
					fprintf (fp, "�Ϻθ�\n");
					// ���
					strcpy (buffer, "");
					for (xx = 0 ; xx < tableformInfo.nCells_Bottom ; ++xx) {
						if (tableformInfo.cells_Bottom [xx][0].objType != NONE) {
							if (tableformInfo.cells_Bottom [xx][0].objType == EUROFORM)
								strcat (buffer, "������,");
							if (tableformInfo.cells_Bottom [xx][0].objType == PLYWOOD)
								strcat (buffer, "����,");
							if (tableformInfo.cells_Bottom [xx][0].objType == PLYWOOD_POLY)
								strcat (buffer, "����(�ٰ���),");
						} else if (tableformInfo.cells_Bottom [xx][1].objType != NONE) {
							if (tableformInfo.cells_Bottom [xx][1].objType == EUROFORM)
								strcat (buffer, "������,");
							if (tableformInfo.cells_Bottom [xx][1].objType == PLYWOOD)
								strcat (buffer, "����,");
							if (tableformInfo.cells_Bottom [xx][1].objType == PLYWOOD_POLY)
								strcat (buffer, "����(�ٰ���),");
						}
					}
					// 2��
					strcat (buffer, "\n");
					for (xx = 0 ; xx < tableformInfo.nCells_Bottom ; ++xx) {
						if (tableformInfo.cells_Bottom [xx][1].objType != NONE) {
							sprintf (buffer_s, "%.0f x %.0f,", tableformInfo.cells_Bottom [xx][1].width * 1000, tableformInfo.cells_Bottom [xx][1].length * 1000);
							strcat (buffer, buffer_s);
						} else {
							sprintf (buffer_s, "��,");
							strcat (buffer, buffer_s);
						}
					}
					// 1��
					strcat (buffer, "\n");
					for (xx = 0 ; xx < tableformInfo.nCells_Bottom ; ++xx) {
						if (tableformInfo.cells_Bottom [xx][0].objType != NONE) {
							sprintf (buffer_s, "%.0f x %.0f,", tableformInfo.cells_Bottom [xx][0].width * 1000, tableformInfo.cells_Bottom [xx][0].length * 1000);
							strcat (buffer, buffer_s);
						} else {
							sprintf (buffer_s, "��,");
							strcat (buffer, buffer_s);
						}
					}
					strcat (buffer, "\n\n");
					fprintf (fp, buffer);
				}

				strcpy (buffer, "\n\n");
				fprintf (fp, buffer);

			} else {
				// ������ ���
				sprintf (buffer, "<< ���̾� : %s >>\n", fullLayerName);
				fprintf (fp, buffer);

				sprintf (buffer, "\n����ȭ�� �� ���̺��� ���̾ �ƴմϴ�.\n");
				fprintf (fp, buffer);

				strcpy (buffer, "\n\n");
				fprintf (fp, buffer);
			}

			// ��ü ����
			objects.Clear ();
			objectList.clear ();
			objectList_Left.clear ();
			objectList_Right.clear ();
			objectList_Bottom.clear ();

			// ���̾� �����
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

	fclose (fp);

	// ��� ���μ����� ��ġ�� ó���� �����ߴ� ���̴� ���̾���� �ٽ� �ѳ��� ��
	for (xx = 1 ; xx <= nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx-1];
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}
		}
	}

	// �׷�ȭ �Ͻ����� OFF
	suspendGroups (false);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	//location.ToDisplayText (&resultString);
	sprintf (buffer, "������� ���� ��ġ�� �����߽��ϴ�.\n\n%s\n�Ǵ� ������Ʈ ������ �ִ� ����", resultString.ToCStr ().Get ());
	WriteReport_Alert (buffer);

	return err;
}

// ���̺��� ���� ���
GSErrCode	calcTableformArea (void)
{
	GSErrCode	err = NoError;
	unsigned short		xx;
	short		mm;
	//bool		regenerate = true;

	// ��� ��ü, �� ����
	GS::Array<API_Guid>		elemList;
	GS::Array<API_Guid>		objects;
	long					nObjects = 0;

	// ������ ��ҵ��� ���� ����ϱ�
	API_Element			elem;
	API_ElementMemo		memo;

	// ���̾� ���� ����
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];
	vector<LayerList>	layerList;

	// ��Ÿ
	char			buffer [512];
	char			filename [512];

	// ����ٸ� ǥ���ϱ� ���� ����
	GS::UniString       title ("�������� ���� ��Ȳ");
	GS::UniString       subtitle ("������...");
	short	nPhase;
	Int32	cur, total;

	// ���� ���Ϸ� ��� ���� ��������
	// ���� ������ ���� ����
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp_unite;

	bool	bTargetObject;		// ����� �Ǵ� ��ü�ΰ�?
	double	totalArea;			// �� ������
	double	totalAreaAll;		// �� ������ (��� ���̾� �ջ�)


	// �׷�ȭ �Ͻ����� ON
	suspendGroups (true);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

	// ������Ʈ �� ���̾� ������ �˾Ƴ�
	nLayers = getLayerCount ();

	// ���̴� ���̾���� ��� �����ϱ�
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList [nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// ���̾� �̸��� �ε��� ����
	for (xx = 0 ; xx < nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx];
		err = ACAPI_Attribute_Get (&attrib);

		sprintf (fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName [strlen (fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList [xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back (newLayerItem);
	}

	// ���̾� �̸� �������� �����Ͽ� ���̾� �ε��� ���� ����
	sort (layerList.begin (), layerList.end (), compareLayerName);		// ���̾� �̸� ���� �������� ����

	// �Ͻ������� ��� ���̾� �����
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

	ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
	sprintf (filename, "%s - ���̾ ���̺��� ���� (����).csv", miscAppInfo.caption);
	fp_unite = fopen (filename, "w+");

	if (fp_unite == NULL) {
		DGAlert (DG_ERROR, L"����", L"���� ���� ���������� ���� �� �����ϴ�.", "", L"Ȯ��", "", "");
		return	NoError;
	}

	// ���� ��Ȳ ǥ���ϴ� ��� - �ʱ�ȭ
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface (APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &total);

	totalAreaAll = 0.0;
	sprintf (buffer, "�ȳ�: ���� ���� ������ m2(��������)�Դϴ�.\n�����Ǵ� ��ü: ������ / ���� / �ƿ��ڳ��ǳ� / ���ڳ��ǳ� / �������ڳ��ǳ� / ���ڳ�M���ǳ� / ����\n\n");
	fprintf (fp_unite, buffer);

	// ���̴� ���̾���� �ϳ��� ��ȸ�ϸ鼭 ��ü ��ҵ��� ������ �� ���̺����� ���� ���� ���� ��ü���� ���� ���� �����ͼ� �����
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList [mm-1].layerInd;
		err = ACAPI_Attribute_Get (&attrib);

		// �ʱ�ȭ
		objects.Clear ();

		if (err == NoError) {
			// ���̾� ���̱�
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// ��� ��� ��������
			ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����, ��ü Ÿ�Ը�
			while (elemList.GetSize () > 0) {
				objects.Push (elemList.Pop ());
			}
			nObjects = objects.GetSize ();

			if (nObjects == 0)
				continue;

			// ���̾� �̸� ������
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			// ���̾� �̸�
			sprintf (buffer, "<< ���̾� : %s >> : ", fullLayerName);
			fprintf (fp_unite, buffer);

			totalArea = 0.0;

			// ��ü���� ���� �� �����ͼ� �ջ��ϱ�
			for (xx = 0 ; xx < nObjects ; ++xx) {
				BNZeroMemory (&elem, sizeof (API_Element));
				BNZeroMemory (&memo, sizeof (API_ElementMemo));
				elem.header.guid = objects [xx];
				err = ACAPI_Element_Get (&elem);

				if (err == NoError && elem.header.hasMemo) {
					err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

					// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
					ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);
					bool	bForce = true;
					ACAPI_Database (APIDb_RefreshElementID, &elem.header, &bForce);

					if (err == NoError) {
						bTargetObject = false;

						if (my_strcmp (getParameterStringByName (&memo, "u_comp"), "������") == 0) {
							bTargetObject = true;
						} else if (my_strcmp (getParameterStringByName (&memo, "g_comp"), "����") == 0) {
							bTargetObject = true;
						} else if (my_strcmp (getParameterStringByName (&memo, "in_comp"), "�ƿ��ڳ��ǳ�") == 0) {
							bTargetObject = true;
						} else if (my_strcmp (getParameterStringByName (&memo, "in_comp"), "���ڳ��ǳ�") == 0) {
							bTargetObject = true;
						} else if (my_strcmp (getParameterStringByName (&memo, "in_comp"), "�������ڳ��ǳ�") == 0) {
							bTargetObject = true;
						} else if (my_strcmp (getParameterStringByName (&memo, "in_comp"), "���ڳ�M���ǳ�") == 0) {
							bTargetObject = true;
						} else if (my_strcmp (getParameterStringByName (&memo, "w_comp"), "����") == 0) {
							bTargetObject = true;
						}

						if (bTargetObject == true) {
							sprintf (buffer, "%s", getParameterStringByName (&memo, "gs_list_custom04"));
							removeCharInStr (buffer, ',');
							totalArea += atof (buffer);
							totalAreaAll += totalArea;
						}
					}

					ACAPI_DisposeElemMemoHdls (&memo);
				}
			}

			// ���� �� ����ϱ�
			sprintf (buffer, "%lf\n", totalArea);
			fprintf (fp_unite, buffer);

			// ���̾� �����
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}

		// ���� ��Ȳ ǥ���ϴ� ��� - ����
		cur = mm;
		ACAPI_Interface (APIIo_SetProcessValueID, &cur, NULL);
		if (ACAPI_Interface (APIIo_IsProcessCanceledID, NULL, NULL) == APIERR_CANCEL)
			break;
	}

	// ��� ���̾��� ���� ���� �ջ��� ���� ǥ����
	sprintf (buffer, "\n��� ���� �� �ջ갪: %lf\n", totalAreaAll);
	fprintf (fp_unite, buffer);

	// ���� ��Ȳ ǥ���ϴ� ��� - ������
	ACAPI_Interface (APIIo_CloseProcessWindowID, NULL, NULL);

	fclose (fp_unite);

	// ��� ���μ����� ��ġ�� ó���� �����ߴ� ���̴� ���̾���� �ٽ� �ѳ��� ��
	for (xx = 1 ; xx <= nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx-1];
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}
		}
	}

	// �׷�ȭ �Ͻ����� OFF
	suspendGroups (false);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	//location.ToDisplayText (&resultString);
	WriteReport_Alert ("������� ���� ��ġ�� �����߽��ϴ�.\n\n%s\n�Ǵ� ������Ʈ ������ �ִ� ����", resultString.ToCStr ().Get ());

	return err;
}

// ��� �Ը鵵 PDF�� �������� (Single ���)
GSErrCode	exportAllElevationsToPDFSingleMode (void)
{
	GSErrCode	err = NoError;
	bool		regenerate = true;
	char		filename [256];
	bool		bAsked = false;

	// �Ը鵵 DB�� �������� ���� ����
	API_DatabaseUnId*	dbases = NULL;
	GSSize				nDbases = 0;
	API_WindowInfo		windowInfo;
	API_DatabaseInfo	currentDB;

	// ���� �������⸦ ���� ����
	API_FileSavePars	fsp;
	API_SavePars_Pdf	pars_pdf;

	// �Ը鵵 Ȯ�븦 ���� ����
	API_Box		extent;
	double		scale = 100.0;
	bool		zoom = true;

	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;

	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
						
	// PDF ���� ������ ����
	BNZeroMemory (&pars_pdf, sizeof (API_SavePars_Pdf));
	pars_pdf.leftMargin = 5.0;
	pars_pdf.rightMargin = 5.0;
	pars_pdf.topMargin = 5.0;
	pars_pdf.bottomMargin = 5.0;
	pars_pdf.sizeX = 297.0;
	pars_pdf.sizeY = 210.0;

	// �Ը� �� DB�� ID���� ȹ����
	err = ACAPI_Database (APIDb_GetElevationDatabasesID, &dbases, NULL);
	if (err == NoError)
		nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);

	// �Ը� ����� �ϳ��� ��ȸ��
	for (GSIndex i = 0; i < nDbases; i++) {
		API_DatabaseInfo dbPars;
		BNZeroMemory (&dbPars, sizeof (API_DatabaseInfo));
		dbPars.databaseUnId = dbases [i];

		// â�� ������
		BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
		windowInfo.typeID = APIWind_ElevationID;
		windowInfo.databaseUnId = dbPars.databaseUnId;
		ACAPI_Automate (APIDo_ChangeWindowID, &windowInfo, NULL);

		// ���� �����ͺ��̽��� ������
		ACAPI_Database (APIDb_GetCurrentDatabaseID, &currentDB, NULL);

		// ���� ������ ����� ������ ������
		ACAPI_Database (APIDb_GetExtentID, &extent, NULL);

		// ��ô �����ϱ�
		if (bAsked == false) {
			scale = (abs (extent.xMax - extent.xMin) < abs (extent.yMax - extent.yMin)) ? abs (extent.xMax - extent.xMin) : abs (extent.yMax - extent.yMin);
			scale *= 2;
			DGBlankModalDialog (300, 150, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, scaleQuestionHandler, (DGUserData) &scale);
			bAsked = true;
		}
		ACAPI_Database (APIDb_ChangeDrawingScaleID, &scale, &zoom);

		// �����ϱ�
		BNZeroMemory (&fsp, sizeof (API_FileSavePars));
		fsp.fileTypeID = APIFType_PdfFile;
		ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
		sprintf (filename, "%s.pdf", GS::UniString (currentDB.title).ToCStr ().Get ());
		fsp.file = new IO::Location (location, IO::Name (filename));

		err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pdf);

		delete	fsp.file;
	}

	if (dbases != NULL)
		BMpFree (reinterpret_cast<GSPtr>(dbases));

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	//location.ToDisplayText (&resultString);
	WriteReport_Alert ("������� ���� ��ġ�� �����߽��ϴ�.\n\n%s\n�Ǵ� ������Ʈ ������ �ִ� ����", resultString.ToCStr ().Get ());

	return err;
}

// ��� �Ը鵵 PDF�� �������� (Multi ���)
GSErrCode	exportAllElevationsToPDFMultiMode (void)
{
	GSErrCode	err = NoError;
	short		xx, mm;
	bool		regenerate = true;
	char		filename [256];
	bool		bAsked = false;

	// ���̾� ���� ����
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];
	vector<LayerList>	layerList;

	// ����ٸ� ǥ���ϱ� ���� ����
	GS::UniString       title ("�������� ���� ��Ȳ");
	GS::UniString       subtitle ("������...");
	short	nPhase;
	Int32	cur, total;

	// �Ը鵵 DB�� �������� ���� ����
	API_DatabaseUnId*	dbases = NULL;
	GSSize				nDbases = 0;
	API_WindowInfo		windowInfo;
	API_DatabaseInfo	currentDB;

	// ���� �������⸦ ���� ����
	API_FileSavePars	fsp;
	API_SavePars_Pdf	pars_pdf;

	// �Ը鵵 Ȯ�븦 ���� ����
	API_Box		extent;
	double		scale = 100.0;
	bool		zoom = true;

	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;

	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
						
	// ������Ʈ �� ���̾� ������ �˾Ƴ�
	nLayers = getLayerCount ();

	// ���̴� ���̾���� ��� �����ϱ�
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList [nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// ���̾� �̸��� �ε��� ����
	for (xx = 0 ; xx < nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx];
		err = ACAPI_Attribute_Get (&attrib);

		sprintf (fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName [strlen (fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList [xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back (newLayerItem);
	}

	// ���̾� �̸� �������� �����Ͽ� ���̾� �ε��� ���� ����
	sort (layerList.begin (), layerList.end (), compareLayerName);		// ���̾� �̸� ���� �������� ����

	// �Ͻ������� ��� ���̾� �����
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

	// �Ը� �� DB�� ID���� ȹ����
	err = ACAPI_Database (APIDb_GetElevationDatabasesID, &dbases, NULL);
	if (err == NoError)
		nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);

	// PDF ���� ������ ����
	BNZeroMemory (&pars_pdf, sizeof (API_SavePars_Pdf));
	pars_pdf.leftMargin = 5.0;
	pars_pdf.rightMargin = 5.0;
	pars_pdf.topMargin = 5.0;
	pars_pdf.bottomMargin = 5.0;
	pars_pdf.sizeX = 297.0;
	pars_pdf.sizeY = 210.0;

	// ���� ��Ȳ ǥ���ϴ� ��� - �ʱ�ȭ
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface (APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &total);

	// ���̴� ���̾���� �ϳ��� ��ȸ�ϸ鼭 ��ü ��ҵ��� ������ �� ���̺����� ���� ���� ���� ��ü���� ���� ���� �����ͼ� �����
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList [mm-1].layerInd;
		err = ACAPI_Attribute_Get (&attrib);

		if (err == NoError) {
			// ���̾� ���̱�
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// ���̾� �̸� ������
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			// �Ը� ����� �ϳ��� ��ȸ��
			for (GSIndex i = 0; i < nDbases; i++) {
				API_DatabaseInfo dbPars;
				BNZeroMemory (&dbPars, sizeof (API_DatabaseInfo));
				dbPars.databaseUnId = dbases [i];

				// â�� ������
				BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
				windowInfo.typeID = APIWind_ElevationID;
				windowInfo.databaseUnId = dbPars.databaseUnId;
				ACAPI_Automate (APIDo_ChangeWindowID, &windowInfo, NULL);

				// ���� �����ͺ��̽��� ������
				ACAPI_Database (APIDb_GetCurrentDatabaseID, &currentDB, NULL);

				// ���� ������ ����� ������ ������
				ACAPI_Database (APIDb_GetExtentID, &extent, NULL);

				// ��ô �����ϱ�
				if (bAsked == false) {
					scale = (abs (extent.xMax - extent.xMin) < abs (extent.yMax - extent.yMin)) ? abs (extent.xMax - extent.xMin) : abs (extent.yMax - extent.yMin);
					scale *= 2;
					DGBlankModalDialog (300, 150, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, scaleQuestionHandler, (DGUserData) &scale);
					bAsked = true;
				}
				ACAPI_Database (APIDb_ChangeDrawingScaleID, &scale, &zoom);

				// �����ϱ�
				BNZeroMemory (&fsp, sizeof (API_FileSavePars));
				fsp.fileTypeID = APIFType_PdfFile;
				ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
				sprintf (filename, "%s - %s.pdf", fullLayerName, GS::UniString (currentDB.title).ToCStr ().Get ());
				fsp.file = new IO::Location (location, IO::Name (filename));

				err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pdf);

				delete	fsp.file;
			}

			// ���̾� �����
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}

		// ���� ��Ȳ ǥ���ϴ� ��� - ����
		cur = mm;
		ACAPI_Interface (APIIo_SetProcessValueID, &cur, NULL);
		if (ACAPI_Interface (APIIo_IsProcessCanceledID, NULL, NULL) == APIERR_CANCEL)
			break;
	}

	// ���� ��Ȳ ǥ���ϴ� ��� - ������
	ACAPI_Interface (APIIo_CloseProcessWindowID, NULL, NULL);

	if (dbases != NULL)
		BMpFree (reinterpret_cast<GSPtr>(dbases));

	// ��� ���μ����� ��ġ�� ó���� �����ߴ� ���̴� ���̾���� �ٽ� �ѳ��� ��
	for (xx = 1 ; xx <= nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx-1];
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}
		}
	}

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	//location.ToDisplayText (&resultString);
	WriteReport_Alert ("������� ���� ��ġ�� �����߽��ϴ�.\n\n%s\n�Ǵ� ������Ʈ ������ �ִ� ����", resultString.ToCStr ().Get ());

	return err;
}

// [���̾�α�] ���̾�α׿��� ���̴� ���̾� �� �ִ� ��ü���� ������ �����ְ�, üũ�� ������ ��ü�鸸 ���� �� ������
short DGCALLBACK filterSelectionHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	xx;
	short	itmIdx;
	short	itmPosX, itmPosY;
	char	buffer [64];

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"������ Ÿ���� ��ü ���� �� �����ֱ�");

			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 10, 80, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"Ȯ��");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 10, 80, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"���");
			DGShowItem (dialogID, DG_CANCEL);

			// ��ư: ��ü����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 50, 80, 25);
			DGSetItemFont (dialogID, BUTTON_ALL_SEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ALL_SEL, L"��ü����");
			DGShowItem (dialogID, BUTTON_ALL_SEL);

			// ��ư: ��ü���� ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 50, 80, 25);
			DGSetItemFont (dialogID, BUTTON_ALL_UNSEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ALL_UNSEL, L"��ü����\n����");
			DGShowItem (dialogID, BUTTON_ALL_UNSEL);

			// üũ�ڽ�: �˷����� ���� ��ü ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 220, 50, 250, 25);
			DGSetItemFont (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT, DG_IS_LARGE | DG_IS_PLAIN);
			sprintf (buffer, "�˷����� ���� ��ü ���� (%d)", visibleObjInfo.nUnknownObjects);
			DGSetItemText (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT, buffer);
			DGShowItem (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT);
			if (visibleObjInfo.nUnknownObjects > 0)
				DGEnableItem (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT);
			else
				DGDisableItem (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT);

			// ������
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 5, 90, 740, 1);
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ� �׸�� ��ġ�� ��
			itmPosX = 20;	itmPosY = 105;	// Y�� ���� 105 ~ 500����

			if (visibleObjInfo.bExist_Walls == true) {
				visibleObjInfo.itmIdx_Walls = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Columns == true) {
				visibleObjInfo.itmIdx_Columns = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"���");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Beams == true) {
				visibleObjInfo.itmIdx_Beams = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Slabs == true) {
				visibleObjInfo.itmIdx_Slabs = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"������");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Roofs == true) {
				visibleObjInfo.itmIdx_Roofs = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"����");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Meshes == true) {
				visibleObjInfo.itmIdx_Meshes = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"�޽�");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Morphs == true) {
				visibleObjInfo.itmIdx_Morphs = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"����");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Shells == true) {
				visibleObjInfo.itmIdx_Shells = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}

			for (xx = 0 ; xx < visibleObjInfo.nKinds ; ++xx) {
				if (visibleObjInfo.bExist [xx] == true) {
					visibleObjInfo.itmIdx [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, visibleObjInfo.objName [xx]);
					DGShowItem (dialogID, itmIdx);
					itmPosY += 30;

					// 1�࿡ 12��
					if (itmPosY > 430) {
						itmPosX += 200;
						itmPosY = 105;
					}
				}
			}

			break;
		
		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// Object Ÿ��
					for (xx = 0 ; xx < visibleObjInfo.nKinds ; ++xx) {
						if (DGGetItemValLong (dialogID, visibleObjInfo.itmIdx [xx]) == TRUE) {
							visibleObjInfo.bShow [xx] = true;
						} else {
							visibleObjInfo.bShow [xx] = false;
						}
					}

					// �˷����� ���� Object Ÿ���� ��ü ���̱� ����
					(DGGetItemValLong (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT) == TRUE)	? visibleObjInfo.bShow_Unknown = true	: visibleObjInfo.bShow_Unknown = false;

					// ������ Ÿ��
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Walls) == TRUE)		? visibleObjInfo.bShow_Walls = true		: visibleObjInfo.bShow_Walls = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Columns) == TRUE)	? visibleObjInfo.bShow_Columns = true	: visibleObjInfo.bShow_Columns = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Beams) == TRUE)		? visibleObjInfo.bShow_Beams = true		: visibleObjInfo.bShow_Beams = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Slabs) == TRUE)		? visibleObjInfo.bShow_Slabs = true		: visibleObjInfo.bShow_Slabs = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Roofs) == TRUE)		? visibleObjInfo.bShow_Roofs = true		: visibleObjInfo.bShow_Roofs = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Meshes) == TRUE)		? visibleObjInfo.bShow_Meshes = true	: visibleObjInfo.bShow_Meshes = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Morphs) == TRUE)		? visibleObjInfo.bShow_Morphs = true	: visibleObjInfo.bShow_Morphs = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Shells) == TRUE)		? visibleObjInfo.bShow_Shells = true	: visibleObjInfo.bShow_Shells = false;

					break;

				case DG_CANCEL:
					break;

				case BUTTON_ALL_SEL:
					item = 0;

					// ��� üũ�ڽ��� ��
					if (visibleObjInfo.bExist_Walls == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Walls, TRUE);
					if (visibleObjInfo.bExist_Columns == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Columns, TRUE);
					if (visibleObjInfo.bExist_Beams == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Beams, TRUE);
					if (visibleObjInfo.bExist_Slabs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Slabs, TRUE);
					if (visibleObjInfo.bExist_Roofs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Roofs, TRUE);
					if (visibleObjInfo.bExist_Meshes == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Meshes, TRUE);
					if (visibleObjInfo.bExist_Morphs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Morphs, TRUE);
					if (visibleObjInfo.bExist_Shells == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Shells, TRUE);
					for (xx = 0 ; xx < visibleObjInfo.nKinds ; ++xx) {
						if (visibleObjInfo.bExist [xx] == true) {
							DGSetItemValLong (dialogID, visibleObjInfo.itmIdx [xx], TRUE);
						}
					}

					break;

				case BUTTON_ALL_UNSEL:
					item = 0;

					// ��� üũ�ڽ��� ��
					if (visibleObjInfo.bExist_Walls == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Walls, FALSE);
					if (visibleObjInfo.bExist_Columns == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Columns, FALSE);
					if (visibleObjInfo.bExist_Beams == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Beams, FALSE);
					if (visibleObjInfo.bExist_Slabs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Slabs, FALSE);
					if (visibleObjInfo.bExist_Roofs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Roofs, FALSE);
					if (visibleObjInfo.bExist_Meshes == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Meshes, FALSE);
					if (visibleObjInfo.bExist_Morphs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Morphs, FALSE);
					if (visibleObjInfo.bExist_Shells == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Shells, FALSE);
					for (xx = 0 ; xx < visibleObjInfo.nKinds ; ++xx) {
						if (visibleObjInfo.bExist [xx] == true) {
							DGSetItemValLong (dialogID, visibleObjInfo.itmIdx [xx], FALSE);
						}
					}

					break;

				default:
					item = 0;
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

// [���̾�α�] ����ڰ� ��ô ���� ���� �Է��� �� �ֵ��� ��
short DGCALLBACK scaleQuestionHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	short	result;
	short	idxItem;
	double	*scale = (double*) userData;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"�Ը鵵 ��ô�� �Է��ϱ�");

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 70, 110, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"��");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 160, 110, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"�ƴϿ�");
			DGShowItem (dialogID, DG_CANCEL);

			// ��: �ȳ���
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 15, 200, 25);
			DGSetItemFont (dialogID, idxItem, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, idxItem, L"�Ը鵵�� ��ô�� �����Ͻðڽ��ϱ�?\n���� �۾������� ������ Ȯ��˴ϴ�.");
			DGShowItem (dialogID, idxItem);

			// ��: ��ô
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 130, 60, 30, 23);
			DGSetItemFont (dialogID, idxItem, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, idxItem, L"1 : ");
			DGShowItem (dialogID, idxItem);

			// Edit ��Ʈ��: ��ô
			EDITCONTROL_SCALE_VALUE = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 161, 54, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_SCALE_VALUE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, EDITCONTROL_SCALE_VALUE, *scale);
			DGShowItem (dialogID, EDITCONTROL_SCALE_VALUE);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					*scale = DGGetItemValDouble (dialogID, EDITCONTROL_SCALE_VALUE);
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

// [���̾�α�] ���ϸ��� �Է��� �� �ֵ��� ��
short DGCALLBACK filenameQuestionHandler(short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	short	result;
	short	idxItem;
	GS::UniString* filename = (GS::UniString*)userData;

	switch (message) {
	case DG_MSG_INIT:
		// ���̾�α� Ÿ��Ʋ
		DGSetDialogTitle(dialogID, L"���ϸ� �Է��ϱ�");

		// Yes ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 70, 110, 70, 25);
		DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_OK, L"��");
		DGShowItem(dialogID, DG_OK);

		// No ��ư
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 160, 110, 70, 25);
		DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_CANCEL, L"�ƴϿ�");
		DGShowItem(dialogID, DG_CANCEL);

		// ��: �ȳ���
		idxItem = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 15, 200, 25);
		DGSetItemFont(dialogID, idxItem, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, idxItem, L"������ ������ �̸��� �Է��Ͻʽÿ�.");
		DGShowItem(dialogID, idxItem);

		// Edit ��Ʈ��
		EDITCONTROL_FILENAME = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_TEXT, 0, 70, 54, 160, 25);
		DGSetItemFont(dialogID, EDITCONTROL_FILENAME, DG_IS_LARGE | DG_IS_PLAIN);
		//DGSetItemText(dialogID, EDITCONTROL_FILENAME, *filename);
		DGShowItem(dialogID, EDITCONTROL_FILENAME);

		break;

	case DG_MSG_CLICK:
		switch (item) {
		case DG_OK:
			*filename = DGGetItemText(dialogID, EDITCONTROL_FILENAME);
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