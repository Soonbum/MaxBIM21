#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "MaxBIM21.h"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.h"
#include "Export.h"

using namespace exportDG;

VisibleObjectInfo	visibleObjInfo;		// 보이는 레이어 상의 객체별 명칭, 존재 여부, 보이기 여부
short	EDITCONTROL_SCALE_VALUE;		// 축척 값을 입력하는 Edit컨트롤의 ID 값
short	EDITCONTROL_FILENAME;			// 파일명을 입력하는 Edit컨트롤의 ID 값


// 배열 초기화 함수
void initArray(double arr[], short arrSize)
{
	short	xx;

	for (xx = 0; xx < arrSize; ++xx)
		arr[xx] = 0.0;
}

// 오름차순으로 정렬할 때 사용하는 비교함수 (퀵소트)
int compare(const void* first, const void* second)
{
	if (*(double*)first > *(double*)second)
		return 1;
	else if (*(double*)first < *(double*)second)
		return -1;
	else
		return 0;
}

// vector 내 자재 정보 구조체 정렬을 위한 비교 함수 (좌표값 X 기준)
bool comparePosX(const objectInBeamTableform& a, const objectInBeamTableform& b)
{
	return	(a.minPos.x + (a.maxPos.x - a.minPos.x) / 2) < (b.minPos.x + (b.maxPos.x - b.minPos.x) / 2);
}

// vector 내 자재 정보 구조체 정렬을 위한 비교 함수 (좌표값 Y 기준)
bool comparePosY(const objectInBeamTableform& a, const objectInBeamTableform& b)
{
	return	(a.minPos.y + (a.maxPos.y - a.minPos.y) / 2) < (b.minPos.y + (b.maxPos.y - b.minPos.y) / 2);
}

// vector 내 레이어 정보 구조체 정렬을 위한 비교 함수 (레이어 이름 기준)
bool compareLayerName(const LayerList& a, const LayerList& b)
{
	if (a.layerName.compare(b.layerName) < 0)
		return true;
	else
		return false;
}

// vector 내 레코드 내 필드를 기준으로 내림차순 정렬을 위한 비교 함수
bool compareVectorString(const vector<string>& a, const vector<string>& b)
{
	int	xx;
	size_t	aLen = a.size();
	size_t	bLen = b.size();

	const char* aStr;
	const char* bStr;

	// a와 b의 0번째 문자열이 같고, 벡터의 길이도 같을 경우 계속 진행
	if (aLen == bLen) {
		// 1번째 문자열부터 n번째 문자열까지 순차적으로 비교함
		for (xx = 0; xx < aLen; ++xx) {
			aStr = a.at(xx).c_str();
			bStr = b.at(xx).c_str();

			// 숫자로 된 문자열이면,
			if ((isStringDouble(aStr) == TRUE) && (isStringDouble(bStr) == TRUE)) {
				if (atof(aStr) - atof(bStr) > EPS)
					return true;
				else if (atof(bStr) - atof(aStr) > EPS)
					return false;
				else
					continue;
			}
			else {
				if (my_strcmp(aStr, bStr) > 0)
					return true;
				else if (my_strcmp(aStr, bStr) < 0)
					return false;
				else
					continue;
			}
		}
	}
	else {
		if (aLen > bLen)
			return true;
		else
			return false;
	}

	return	true;
}

// 선택한 부재들의 요약 정보: 생성자
SummaryOfObjectInfo::SummaryOfObjectInfo()
{
	FILE* fp;			// 파일 포인터
	char	line[2048];	// 파일에서 읽어온 라인 하나
	char* token;			// 읽어온 문자열의 토큰
	short	lineCount;		// 읽어온 라인 수
	short	tokCount;		// 읽어온 토큰 개수
	short	xx;
	int		count;

	char	nthToken[200][256];	// n번째 토큰

	// 객체 정보 파일 가져오기
	fp = fopen("C:\\objectInfo.csv", "r");

	if (fp == NULL) {
		DGAlert(DG_ERROR, L"오류", L"objectInfo.csv 파일을 C:\\로 복사하십시오.", "", L"확인", "", "");
	}
	else {
		lineCount = 0;

		while (!feof(fp)) {
			tokCount = 0;
			fgets(line, sizeof(line), fp);

			token = strtok(line, ",");
			tokCount++;
			lineCount++;

			// 한 라인씩 처리
			while (token != NULL) {
				if (strlen(token) > 0) {
					strncpy(nthToken[tokCount - 1], token, strlen(token) + 1);
				}
				token = strtok(NULL, ",");
				tokCount++;
			}

			// 토큰 개수가 2개 이상일 때 유효함
			if ((tokCount - 1) >= 2) {
				// 토큰 개수가 2 + 3의 배수 개씩만 저장됨 (초과된 항목은 제외)
				if (((tokCount - 1) - 2) % 3 != 0) {
					tokCount--;
				}

				this->keyName.push_back(nthToken[0]);		// 예: u_comp
				this->keyDesc.push_back(nthToken[1]);		// 예: 유로폼
				count = atoi(nthToken[2]);
				this->nInfo.push_back(count);				// 예: 5

				vector<string>	varNames;	// 해당 객체의 변수 이름들
				vector<string>	varDescs;	// 해당 객체의 변수 이름에 대한 설명들

				for (xx = 1; xx <= count; ++xx) {
					varNames.push_back(nthToken[1 + xx * 2]);
					varDescs.push_back(nthToken[1 + xx * 2 + 1]);
				}

				this->varName.push_back(varNames);
				this->varDesc.push_back(varDescs);
			}
		}

		// 파일 닫기
		fclose(fp);

		// 객체 종류 개수
		this->nKnownObjects = lineCount - 1;
		this->nUnknownObjects = 0;
	}
}

// 객체의 레코드 수량 1 증가 (있으면 증가, 없으면 신규 추가)
int	SummaryOfObjectInfo::quantityPlus1(vector<string> record)
{
	int		xx, yy;
	size_t	vecLen;
	size_t	inVecLen1, inVecLen2;
	int		diff;
	int		value;
	char	tempStr[512];

	vecLen = this->records.size();

	try {
		for (xx = 0; xx < vecLen; ++xx) {
			// 변수 값도 동일할 경우
			inVecLen1 = this->records.at(xx).size() - 1;		// 끝의 개수 필드를 제외한 길이
			inVecLen2 = record.size();

			if (inVecLen1 == inVecLen2) {
				// 일치하지 않는 필드가 하나라도 있는지 찾아볼 것
				diff = 0;
				for (yy = 0; yy < inVecLen1; ++yy) {
					if (my_strcmp(this->records.at(xx).at(yy).c_str(), record.at(yy).c_str()) != 0)
						diff++;
				}

				// 모든 필드가 일치하면
				if (diff == 0) {
					value = atoi(this->records.at(xx).back().c_str());
					value++;
					sprintf(tempStr, "%d", value);
					this->records.at(xx).pop_back();
					this->records.at(xx).push_back(tempStr);
					return value;
				}
			}
		}
	}
	catch (exception& ex) {
		WriteReport_Alert("quantityPlus1 함수에서 오류 발생: %s", ex.what());
	}

	// 없으면 신규 레코드 추가하고 1 리턴
	record.push_back("1");
	this->records.push_back(record);

	return 1;
}

// 레코드 내용 지우기
void SummaryOfObjectInfo::clear()
{
	unsigned int xx;

	try {
		for (xx = 0; xx < this->records.size(); ++xx) {
			this->records.at(xx).clear();
		}
	}
	catch (exception& ex) {
		WriteReport_Alert("clear 함수에서 오류 발생: %s", ex.what());
	}
	this->records.clear();
}

// 객체의 레코드 수량 n 증가
int		quantityPlusN(vector<vector<string>>* db, vector<string> record, int n)
{
	int		xx, yy;
	size_t	vecLen;
	size_t	inVecLen1, inVecLen2;
	int		diff;
	int		value;
	char	tempStr[512];

	vecLen = db->size();

	try {
		for (xx = 0; xx < vecLen; ++xx) {
			// 변수 값도 동일할 경우
			inVecLen1 = db->at(xx).size() - 1;		// 끝의 개수 필드를 제외한 길이
			inVecLen2 = record.size();

			if (inVecLen1 == inVecLen2) {
				// 일치하지 않는 필드가 하나라도 있는지 찾아볼 것
				diff = 0;
				for (yy = 0; yy < inVecLen1; ++yy) {
					if (my_strcmp(db->at(xx).at(yy).c_str(), record.at(yy).c_str()) != 0)
						diff++;
				}

				// 모든 필드가 일치하면
				if (diff == 0) {
					value = atoi(db->at(xx).back().c_str());
					value += n;
					sprintf(tempStr, "%d", value);
					db->at(xx).pop_back();
					db->at(xx).push_back(tempStr);
					return value;
				}
			}
		}
	}
	catch (exception& ex) {
		WriteReport_Alert("quantityPlusN 함수에서 오류 발생: %s", ex.what());
	}

	// 없으면 신규 레코드 추가하고 n 리턴
	sprintf(tempStr, "%d", n);
	record.push_back(tempStr);
	db->push_back(record);

	return n;
}

// 가설재 품목별 수량 내보내기 (선택한 가설재)
GSErrCode	exportSelectedElementInfo(void)
{
	GSErrCode	err = NoError;
	short		result;
	unsigned short		xx, yy, zz;

	// 선택한 요소가 없으면 오류
	GS::Array<API_Guid>		objects;
	long					nObjects = 0;

	// 선택한 요소들의 정보 요약하기
	API_Element			elem;
	API_ElementMemo		memo;
	SummaryOfObjectInfo	objectInfo;

	char			buffer[512];
	char			tempStr[512];
	const char* foundStr;
	bool			foundObject;
	bool			foundExistValue;
	int				retVal;
	int				nInfo;
	API_AddParID	varType;
	vector<string>	record;

	// GS::Array 반복자
	GS::Array<API_Guid>::Iterator	iterObj;
	API_Guid	curGuid;

	// Single 모드 전용
	vector<vector<string>>	plywoodInfo;	// 합판 종류별 정보
	vector<vector<string>>	darukiInfo;		// 각재 종류별 정보 (합판에 부착된 제작틀만)
	char* token;
	char	infoText[256];


	// 그룹화 일시정지 ON
	suspendGroups(true);

	// 선택한 요소 가져오기
	if (getGuidsOfSelection(&objects, API_ObjectID, &nObjects) != NoError) {
		DGAlert(DG_ERROR, L"오류", L"요소들을 선택해야 합니다.", "", L"확인", "", "");
		return err;
	}

	// 파일 저장을 위한 변수
	FILE* fp;
	FILE* fp_interReport;

	GS::UniString		inputFilename;
	GS::UniString		madeFilename;
	char				filename[512];
	result = DGBlankModalDialog(300, 150, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, filenameQuestionHandler, (DGUserData)&inputFilename);

	if (result == DG_CANCEL) {
		DGAlert(DG_INFORMATION, L"알림", L"작업을 취소하였습니다.", "", L"확인", "", "");
		return	NoError;
	}

	if (inputFilename.GetLength() <= 0)
		inputFilename = "notitle";

	madeFilename = inputFilename + L".csv";
	strcpy(filename, wcharToChar(madeFilename.ToUStr().Get()));
	fp = fopen(filename, "w+");

	madeFilename = inputFilename + L" (중간보고서).txt";
	strcpy(filename, wcharToChar(madeFilename.ToUStr().Get()));
	fp_interReport = fopen(filename, "w+");

	if ((fp == NULL) || (fp_interReport == NULL)) {
		DGAlert(DG_ERROR, L"오류", L"파일을 열 수 없습니다.", "", L"확인", "", "");
		return err;
	}

	iterObj = objects.Enumerate();

	while (iterObj != NULL) {
		foundObject = false;

		BNZeroMemory(&elem, sizeof(API_Element));
		BNZeroMemory(&memo, sizeof(API_ElementMemo));
		curGuid = *iterObj;
		elem.header.guid = curGuid;
		err = ACAPI_Element_Get(&elem);

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo(elem.header.guid, &memo);

			if (err == NoError) {
				// 파라미터 스크립트를 강제로 실행시킴
				ACAPI_Goodies(APIAny_RunGDLParScriptID, &elem.header, 0);
				bool	bForce = true;
				ACAPI_Database(APIDb_RefreshElementID, &elem.header, &bForce);

				try {
					for (yy = 0; yy < objectInfo.keyName.size(); ++yy) {
						strcpy(tempStr, objectInfo.keyName.at(yy).c_str());
						foundStr = getParameterStringByName(&memo, tempStr);

						// 객체 종류를 찾았다면,
						if (my_strcmp(foundStr, "") != 0) {
							retVal = my_strcmp(objectInfo.keyDesc.at(yy).c_str(), foundStr);

							if (retVal == 0) {
								foundObject = true;
								foundExistValue = false;

								// 발견한 객체의 데이터를 기반으로 레코드 추가
								if (!record.empty())
									record.clear();

								record.push_back(objectInfo.keyDesc.at(yy));		// 객체 이름
								nInfo = objectInfo.nInfo.at(yy);
								for (zz = 0; zz < nInfo; ++zz) {
									sprintf(buffer, "%s", objectInfo.varName.at(yy).at(zz).c_str());
									varType = getParameterTypeByName(&memo, buffer);

									if ((varType != APIParT_Separator) || (varType != APIParT_Title) || (varType != API_ZombieParT)) {
										if (varType == APIParT_CString)
											sprintf(tempStr, "%s", getParameterStringByName(&memo, buffer));	// 문자열
										else
											sprintf(tempStr, "%.3f", getParameterValueByName(&memo, buffer));	// 숫자
									}
									record.push_back(tempStr);		// 변수값
								}

								objectInfo.quantityPlus1(record);
							}
						}
					}
				}
				catch (exception& ex) {
					WriteReport_Alert("객체 정보 수집에서 오류 발생: %s", ex.what());
				}

				// 끝내 찾지 못하면 알 수 없는 객체로 취급함
				if (foundObject == false)
					objectInfo.nUnknownObjects++;
			}

			ACAPI_DisposeElemMemoHdls(&memo);
		}

		++iterObj;
	}

	// 최종 텍스트 표시
	// APIParT_Length인 경우 1000배 곱해서 표현
	// APIParT_Boolean인 경우 예/아니오 표현
	double	length, length2, length3;
	bool	bTitleAppeared;

	// 레코드 정렬 (내림차순 정렬)
	sort(objectInfo.records.begin(), objectInfo.records.end(), compareVectorString);

	// *합판, 각재 구매 수량을 계산하기 위한 변수
	double	totalAreaOfPlywoods = 0.0;
	double	totalLengthOfTimbers_40x50 = 0.0;	// 다루끼 (40*50)
	double	totalLengthOfTimbers_50x80 = 0.0;	// 투바이 (50*80)
	double	totalLengthOfTimbers_80x80 = 0.0;	// 산승각 (80*80)
	double	totalLengthOfTimbersEtc = 0.0;		// 비규격
	int		count;	// 개수

	// *합판, 다루끼 제작 수량을 계산하기 위한 변수
	plywoodInfo.clear();
	darukiInfo.clear();

	// 객체 종류별로 수량 출력
	try {
		for (xx = 0; xx < objectInfo.keyDesc.size(); ++xx) {
			bTitleAppeared = false;

			// 레코드를 전부 순회
			for (yy = 0; yy < objectInfo.records.size(); ++yy) {
				// 객체 종류 이름과 레코드의 1번 필드가 일치하는 경우만 찾아서 출력함
				retVal = my_strcmp(objectInfo.keyDesc.at(xx).c_str(), objectInfo.records.at(yy).at(0).c_str());

				count = atoi(objectInfo.records.at(yy).at(objectInfo.records.at(yy).size() - 1).c_str());

				if (retVal == 0) {
					// 제목 출력
					if (bTitleAppeared == false) {
						sprintf(buffer, "\n[%s]\n", objectInfo.keyDesc.at(xx).c_str());
						fprintf(fp, buffer);
						bTitleAppeared = true;
					}

					// 변수별 값 출력
					if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "유로폼 후크") == 0) {
						// 원형
						if (objectInfo.records.at(yy).at(2).compare("원형") == 0) {
							sprintf(buffer, "원형 / %s", objectInfo.records.at(yy).at(1).c_str());
						}

						// 사각
						if (objectInfo.records.at(yy).at(2).compare("사각") == 0) {
							sprintf(buffer, "사각 / %s", objectInfo.records.at(yy).at(1).c_str());
						}
						fprintf(fp, buffer);

					}
					else if ((my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "유로폼") == 0) || (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "스틸폼") == 0)) {
						// 규격폼
						if (atoi(objectInfo.records.at(yy).at(1).c_str()) > 0) {
							sprintf(buffer, "%s X %s ", objectInfo.records.at(yy).at(2).c_str(), objectInfo.records.at(yy).at(3).c_str());

						}
						// 비규격품
						else {
							length = atof(objectInfo.records.at(yy).at(4).c_str());
							length2 = atof(objectInfo.records.at(yy).at(5).c_str());
							sprintf(buffer, "%.0f X %.0f ", round(length * 1000, 0), round(length2 * 1000, 0));
						}
						fprintf(fp, buffer);

					}
					else if (objectInfo.keyDesc.at(xx).compare("목재") == 0) {
						length = atof(objectInfo.records.at(yy).at(1).c_str());
						length2 = atof(objectInfo.records.at(yy).at(2).c_str());
						length3 = atof(objectInfo.records.at(yy).at(3).c_str());
						sprintf(buffer, "%.0f X %.0f X %.0f ", round(length * 1000, 0), round(length2 * 1000, 0), round(length3 * 1000, 0));
						if (((abs(length - 0.040) < EPS) && (abs(length2 - 0.050) < EPS)) || ((abs(length - 0.050) < EPS) && (abs(length2 - 0.040) < EPS)))
							totalLengthOfTimbers_40x50 += (length3 * count);
						else if (((abs(length - 0.050) < EPS) && (abs(length2 - 0.080) < EPS)) || ((abs(length - 0.080) < EPS) && (abs(length2 - 0.050) < EPS)))
							totalLengthOfTimbers_50x80 += (length3 * count);
						else if ((abs(length - 0.080) < EPS) && (abs(length2 - 0.080) < EPS))
							totalLengthOfTimbers_80x80 += (length3 * count);
						else
							totalLengthOfTimbersEtc += (length3 * count);
						fprintf(fp, buffer);

					}
					else if (objectInfo.keyDesc.at(xx).compare("각도목") == 0) {
						length = atof(objectInfo.records.at(yy).at(1).c_str());
						length2 = atof(objectInfo.records.at(yy).at(2).c_str());
						length3 = atof(objectInfo.records.at(yy).at(3).c_str());
						sprintf(buffer, "%s X %.0f (커팅각도: %s도)", objectInfo.records.at(yy).at(4).c_str(), round(length3 * 1000, 0), objectInfo.records.at(yy).at(5).c_str());
						if (((abs(length - 0.040) < EPS) && (abs(length2 - 0.050) < EPS)) || ((abs(length - 0.050) < EPS) && (abs(length2 - 0.040) < EPS)))
							totalLengthOfTimbers_40x50 += (length3 * count);
						else if (((abs(length - 0.050) < EPS) && (abs(length2 - 0.080) < EPS)) || ((abs(length - 0.080) < EPS) && (abs(length2 - 0.050) < EPS)))
							totalLengthOfTimbers_50x80 += (length3 * count);
						else if ((abs(length - 0.080) < EPS) && (abs(length2 - 0.080) < EPS))
							totalLengthOfTimbers_80x80 += (length3 * count);
						else
							totalLengthOfTimbersEtc += (length3 * count);
						fprintf(fp, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "콘판넬") == 0) {
						if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "3x6 [910x1820]") == 0) {
							sprintf(buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str());
							fprintf(fp, buffer);

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "4x8 [1220x2440]") == 0) {
							sprintf(buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str());
							fprintf(fp, buffer);

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "2x5 [606x1520]") == 0) {
							sprintf(buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str());
							fprintf(fp, buffer);

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "2x6 [606x1820]") == 0) {
							sprintf(buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str());
							fprintf(fp, buffer);

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "3x5 [910x1520]") == 0) {
							sprintf(buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str());
							fprintf(fp, buffer);

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "비규격") == 0) {
							// 가로 X 세로 X 두께
							length = atof(objectInfo.records.at(yy).at(3).c_str());
							length2 = atof(objectInfo.records.at(yy).at(4).c_str());
							sprintf(buffer, "%.0f X %.0f X %s ", round(length * 1000, 0), round(length2 * 1000, 0), objectInfo.records.at(yy).at(2).c_str());
							fprintf(fp, buffer);
						}

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "합판") == 0) {
						if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "3x6 [910x1820]") == 0) {
							sprintf(buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str());
							totalAreaOfPlywoods += (0.900 * 1.800 * count);
							fprintf(fp, buffer);

							// 합판 정보 DB에 삽입
							record.clear();
							record.push_back(buffer);
							quantityPlusN(&plywoodInfo, record, count);

							// 제작틀 ON
							if (atoi(objectInfo.records.at(yy).at(5).c_str()) > 0) {
								sprintf(buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str());
								totalLengthOfTimbers_40x50 += ((atof(objectInfo.records.at(yy).at(6).c_str()) / 1000) * count);
								fprintf(fp, buffer);

								sprintf(buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str());
								fprintf(fp, buffer);

								// 다루끼 DB에 삽입
								strcpy(infoText, objectInfo.records.at(yy).at(7).c_str());
								token = strtok(infoText, " /");
								while (token != NULL) {
									// 숫자이면 push
									if (atoi(token) != 0) {
										record.clear();
										record.push_back(token);
										quantityPlusN(&darukiInfo, record, 1);
									}
									token = strtok(NULL, " /");
								}
							}

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "4x8 [1220x2440]") == 0) {
							sprintf(buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str());
							totalAreaOfPlywoods += (1.200 * 2.400 * count);
							fprintf(fp, buffer);

							// 합판 정보 DB에 삽입
							record.clear();
							record.push_back(buffer);
							quantityPlusN(&plywoodInfo, record, count);

							// 제작틀 ON
							if (atoi(objectInfo.records.at(yy).at(5).c_str()) > 0) {
								sprintf(buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str());
								totalLengthOfTimbers_40x50 += ((atof(objectInfo.records.at(yy).at(6).c_str()) / 1000) * count);
								fprintf(fp, buffer);

								sprintf(buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str());
								fprintf(fp, buffer);

								// 다루끼 DB에 삽입
								strcpy(infoText, objectInfo.records.at(yy).at(7).c_str());
								token = strtok(infoText, " /");
								while (token != NULL) {
									// 숫자이면 push
									if (atoi(token) != 0) {
										record.clear();
										record.push_back(token);
										quantityPlusN(&darukiInfo, record, 1);
									}
									token = strtok(NULL, " /");
								}
							}

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "2x5 [606x1520]") == 0) {
							sprintf(buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str());
							totalAreaOfPlywoods += (0.600 * 1.500 * count);
							fprintf(fp, buffer);

							// 합판 정보 DB에 삽입
							record.clear();
							record.push_back(buffer);
							quantityPlusN(&plywoodInfo, record, count);

							// 제작틀 ON
							if (atoi(objectInfo.records.at(yy).at(5).c_str()) > 0) {
								sprintf(buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str());
								totalLengthOfTimbers_40x50 += ((atof(objectInfo.records.at(yy).at(6).c_str()) / 1000) * count);
								fprintf(fp, buffer);

								sprintf(buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str());
								fprintf(fp, buffer);

								// 다루끼 DB에 삽입
								strcpy(infoText, objectInfo.records.at(yy).at(7).c_str());
								token = strtok(infoText, " /");
								while (token != NULL) {
									// 숫자이면 push
									if (atoi(token) != 0) {
										record.clear();
										record.push_back(token);
										quantityPlusN(&darukiInfo, record, 1);
									}
									token = strtok(NULL, " /");
								}
							}

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "2x6 [606x1820]") == 0) {
							sprintf(buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str());
							totalAreaOfPlywoods += (0.600 * 1.800 * count);
							fprintf(fp, buffer);

							// 합판 정보 DB에 삽입
							record.clear();
							record.push_back(buffer);
							quantityPlusN(&plywoodInfo, record, count);

							// 제작틀 ON
							if (atoi(objectInfo.records.at(yy).at(5).c_str()) > 0) {
								sprintf(buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str());
								totalLengthOfTimbers_40x50 += ((atof(objectInfo.records.at(yy).at(6).c_str()) / 1000) * count);
								fprintf(fp, buffer);

								sprintf(buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str());
								fprintf(fp, buffer);

								// 다루끼 DB에 삽입
								strcpy(infoText, objectInfo.records.at(yy).at(7).c_str());
								token = strtok(infoText, " /");
								while (token != NULL) {
									// 숫자이면 push
									if (atoi(token) != 0) {
										record.clear();
										record.push_back(token);
										quantityPlusN(&darukiInfo, record, 1);
									}
									token = strtok(NULL, " /");
								}
							}

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "3x5 [910x1520]") == 0) {
							sprintf(buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str());
							totalAreaOfPlywoods += (0.900 * 1.500 * count);
							fprintf(fp, buffer);

							// 합판 정보 DB에 삽입
							record.clear();
							record.push_back(buffer);
							quantityPlusN(&plywoodInfo, record, count);

							// 제작틀 ON
							if (atoi(objectInfo.records.at(yy).at(5).c_str()) > 0) {
								sprintf(buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str());
								totalLengthOfTimbers_40x50 += ((atof(objectInfo.records.at(yy).at(6).c_str()) / 1000) * count);
								fprintf(fp, buffer);

								sprintf(buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str());
								fprintf(fp, buffer);

								// 다루끼 DB에 삽입
								strcpy(infoText, objectInfo.records.at(yy).at(7).c_str());
								token = strtok(infoText, " /");
								while (token != NULL) {
									// 숫자이면 push
									if (atoi(token) != 0) {
										record.clear();
										record.push_back(token);
										quantityPlusN(&darukiInfo, record, 1);
									}
									token = strtok(NULL, " /");
								}
							}

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "비규격") == 0) {
							// 가로 X 세로 X 두께
							length = atof(objectInfo.records.at(yy).at(3).c_str());
							length2 = atof(objectInfo.records.at(yy).at(4).c_str());
							sprintf(buffer, "%.0f X %.0f X %s ", round(length * 1000, 0), round(length2 * 1000, 0), objectInfo.records.at(yy).at(2).c_str());
							totalAreaOfPlywoods += (length * length2 * count);
							fprintf(fp, buffer);

							// 합판 정보 DB에 삽입
							record.clear();
							record.push_back(buffer);
							quantityPlusN(&plywoodInfo, record, count);

							// 제작틀 ON
							if (atoi(objectInfo.records.at(yy).at(5).c_str()) > 0) {
								sprintf(buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str());
								totalLengthOfTimbers_40x50 += ((atof(objectInfo.records.at(yy).at(6).c_str()) / 1000) * count);
								fprintf(fp, buffer);

								sprintf(buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str());
								fprintf(fp, buffer);

								// 다루끼 DB에 삽입
								strcpy(infoText, objectInfo.records.at(yy).at(7).c_str());
								token = strtok(infoText, " /");
								while (token != NULL) {
									// 숫자이면 push
									if (atoi(token) != 0) {
										record.clear();
										record.push_back(token);
										quantityPlusN(&darukiInfo, record, 1);
									}
									token = strtok(NULL, " /");
								}
							}

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "비정형") == 0) {
							sprintf(buffer, "비정형 ");
							fprintf(fp, buffer);

						}
						else {
							sprintf(buffer, "다각형 ");
							fprintf(fp, buffer);
						}

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "합판(다각형)") == 0) {
						// 합판 면적
						sprintf(buffer, "면적: %.2f ", atof(objectInfo.records.at(yy).at(1).c_str()));
						totalAreaOfPlywoods += (atof(objectInfo.records.at(yy).at(1).c_str()) * count);
						fprintf(fp, buffer);

						// 제작틀 ON
						if (atoi(objectInfo.records.at(yy).at(2).c_str()) > 0) {
							sprintf(buffer, "(각재 총길이: %.0f) ", round(atof(objectInfo.records.at(yy).at(3).c_str()) * 1000, 0));
							totalLengthOfTimbers_40x50 += (atof(objectInfo.records.at(yy).at(3).c_str()) * count);
							fprintf(fp, buffer);
						}

					}
					else if (objectInfo.keyDesc.at(xx).compare("RS Push-Pull Props") == 0) {
						// 베이스 플레이트 유무
						if (atoi(objectInfo.records.at(yy).at(1).c_str()) == 1) {
							sprintf(buffer, "베이스 플레이트(있음) ");
						}
						else {
							sprintf(buffer, "베이스 플레이트(없음) ");
						}
						fprintf(fp, buffer);

						// 규격(상부)
						sprintf(buffer, "규격(상부): %s ", objectInfo.records.at(yy).at(2).c_str());
						fprintf(fp, buffer);

						// 규격(하부) - 선택사항
						if (atoi(objectInfo.records.at(yy).at(4).c_str()) == 1) {
							sprintf(buffer, "규격(하부): %s ", objectInfo.records.at(yy).at(3).c_str());
						}
						fprintf(fp, buffer);

					}
					else if (objectInfo.keyDesc.at(xx).compare("Push-Pull Props (기성품 및 당사제작품)") == 0) {
						// 베이스 플레이트 유무
						if (atoi(objectInfo.records.at(yy).at(1).c_str()) == 1) {
							sprintf(buffer, "베이스 플레이트(있음) ");
						}
						else {
							sprintf(buffer, "베이스 플레이트(없음) ");
						}
						fprintf(fp, buffer);

						// 규격(상부)
						sprintf(buffer, "규격(상부): %s ", objectInfo.records.at(yy).at(2).c_str());
						fprintf(fp, buffer);

						// 규격(하부) - 선택사항
						if (atoi(objectInfo.records.at(yy).at(4).c_str()) == 1) {
							sprintf(buffer, "규격(하부): %s ", objectInfo.records.at(yy).at(3).c_str());
						}
						fprintf(fp, buffer);

					}
					else if (objectInfo.keyDesc.at(xx).compare("사각파이프") == 0) {
						// 사각파이프
						if (atof(objectInfo.records.at(yy).at(1).c_str()) < EPS) {
							length = atof(objectInfo.records.at(yy).at(2).c_str());
							sprintf(buffer, "50 x 50 x %.0f ", round(length * 1000, 0));

							// 직사각파이프
						}
						else {
							length = atof(objectInfo.records.at(yy).at(2).c_str());
							sprintf(buffer, "%s x %.0f ", objectInfo.records.at(yy).at(1).c_str(), round(length * 1000, 0));
						}
						fprintf(fp, buffer);

					}
					else if (objectInfo.keyDesc.at(xx).compare("원형파이프") == 0) {
						length = atof(objectInfo.records.at(yy).at(1).c_str());
						sprintf(buffer, "%.0f ", round(length * 1000, 0));
						fprintf(fp, buffer);

					}
					else if (objectInfo.keyDesc.at(xx).compare("아웃코너앵글") == 0) {
						length = atof(objectInfo.records.at(yy).at(1).c_str());
						sprintf(buffer, "%.0f ", round(length * 1000, 0));
						fprintf(fp, buffer);

					}
					else if (objectInfo.keyDesc.at(xx).compare("매직바") == 0) {
						if (atoi(objectInfo.records.at(yy).at(2).c_str()) > 0) {
							length = atof(objectInfo.records.at(yy).at(3).c_str());
							length2 = atof(objectInfo.records.at(yy).at(4).c_str());
							length3 = atof(objectInfo.records.at(yy).at(5).c_str());
							totalAreaOfPlywoods += (atof(objectInfo.records.at(yy).at(6).c_str()) * count);
							sprintf(buffer, "%.0f / 합판(%.0f X %.0f) ", round(atof(objectInfo.records.at(yy).at(1).c_str()) * 1000, 0), round((length - length2) * 1000, 0), round(length3 * 1000, 0));
						}
						else {
							length = atof(objectInfo.records.at(yy).at(1).c_str());
							sprintf(buffer, "%.0f ", round(length * 1000, 0));
						}
						fprintf(fp, buffer);

					}
					else if (objectInfo.keyDesc.at(xx).compare("매직아웃코너") == 0) {
						sprintf(buffer, "타입(%s) %.0f ", objectInfo.records.at(yy).at(1).c_str(), round(atof(objectInfo.records.at(yy).at(2).c_str()) * 1000, 0));
						fprintf(fp, buffer);
						if (atoi(objectInfo.records.at(yy).at(3).c_str()) > 0) {
							totalAreaOfPlywoods += (atof(objectInfo.records.at(yy).at(6).c_str()) * count);
							sprintf(buffer, "합판1(%.0f X %.0f) ", round(atof(objectInfo.records.at(yy).at(2).c_str()) * 1000, 0), round(atof(objectInfo.records.at(yy).at(4).c_str()) * 1000, 0));
							fprintf(fp, buffer);
							sprintf(buffer, "합판2(%.0f X %.0f) ", round(atof(objectInfo.records.at(yy).at(2).c_str()) * 1000, 0), round(atof(objectInfo.records.at(yy).at(5).c_str()) * 1000, 0));
							fprintf(fp, buffer);
						}

					}
					else if (objectInfo.keyDesc.at(xx).compare("매직인코너") == 0) {
						if (atoi(objectInfo.records.at(yy).at(3).c_str()) > 0) {
							length = atof(objectInfo.records.at(yy).at(4).c_str());
							length2 = atof(objectInfo.records.at(yy).at(5).c_str());
							length3 = atof(objectInfo.records.at(yy).at(6).c_str());
							totalAreaOfPlywoods += (atof(objectInfo.records.at(yy).at(7).c_str()) * count);
							sprintf(buffer, "%.0f / 합판(%.0f X %.0f) ", round(atof(objectInfo.records.at(yy).at(2).c_str()) * 1000, 0), round((length - length2) * 1000, 0), round(length3 * 1000, 0));
						}
						else {
							length = atof(objectInfo.records.at(yy).at(2).c_str());
							sprintf(buffer, "%.0f ", round(length * 1000, 0));
						}
						fprintf(fp, buffer);

					}
					else if (objectInfo.keyDesc.at(xx).compare("눈썹보 브라켓 v2") == 0) {
						if (atoi(objectInfo.records.at(yy).at(1).c_str()) > 0) {
							length = atof(objectInfo.records.at(yy).at(2).c_str()) / 1000;
							totalLengthOfTimbers_40x50 += (length * count);
							sprintf(buffer, "각재(%.0f) ", round(length * 1000, 0));
						}
						fprintf(fp, buffer);

					}
					else if (objectInfo.keyDesc.at(xx).compare("단열재") == 0) {
						sprintf(buffer, "원장크기: %.0f X %.0f / 실제크기: %.0f X %.0f (ㄱ형상으로 자름: %s)",
							round(atof(objectInfo.records.at(yy).at(2).c_str()) * 1000, 0), round(atof(objectInfo.records.at(yy).at(3).c_str()) * 1000, 0),
							round(atof(objectInfo.records.at(yy).at(4).c_str()) * 1000, 0), round(atof(objectInfo.records.at(yy).at(5).c_str()) * 1000, 0),
							(atoi(objectInfo.records.at(yy).at(5).c_str()) ? "자름" : "자르지 않음"));
						fprintf(fp, buffer);

					}
					else if (objectInfo.keyDesc.at(xx).compare("PERI동바리 수직재") == 0) {
						length = atof(objectInfo.records.at(yy).at(2).c_str());
						if (atoi(objectInfo.records.at(yy).at(3).c_str()) == 1) {
							sprintf(buffer, "규격(%s) 길이(%.0f) 크로스헤드(%s) ", objectInfo.records.at(yy).at(1).c_str(), round(length * 1000, 0), objectInfo.records.at(yy).at(4).c_str());
						}
						else {
							sprintf(buffer, "규격(%s) 길이(%.0f) ", objectInfo.records.at(yy).at(1).c_str(), round(length * 1000, 0));
						}
						fprintf(fp, buffer);

					}
					else if (objectInfo.keyDesc.at(xx).compare("서포트") == 0) {
						length = atof(objectInfo.records.at(yy).at(2).c_str());
						if (atoi(objectInfo.records.at(yy).at(3).c_str()) == 1) {
							sprintf(buffer, "규격(%s) 길이(%.0f) 크로스헤드(%s) ", objectInfo.records.at(yy).at(1).c_str(), round(length * 1000, 0), objectInfo.records.at(yy).at(4).c_str());
						}
						else {
							sprintf(buffer, "규격(%s) 길이(%.0f) ", objectInfo.records.at(yy).at(1).c_str(), round(length * 1000, 0));
						}
						fprintf(fp, buffer);

					}
					else if (objectInfo.keyDesc.at(xx).compare("창문 개구부 합판거푸집") == 0) {
						// 너비 X 높이 X 벽 두께
						length = atof(objectInfo.records.at(yy).at(2).c_str());
						length2 = atof(objectInfo.records.at(yy).at(3).c_str());
						length3 = atof(objectInfo.records.at(yy).at(4).c_str());
						sprintf(buffer, "%.0f X %.0f X %.0f ", round(length * 1000, 0), round(length2 * 1000, 0), round(length3 * 1000, 0));
						fprintf(fp, buffer);

						// 합판 면적
						totalAreaOfPlywoods += ((atof(objectInfo.records.at(yy).at(5).c_str())) * count);

						// 다루끼 길이
						totalLengthOfTimbers_40x50 += ((atof(objectInfo.records.at(yy).at(6).c_str())) * count);

					}
					else {
						for (zz = 0; zz < objectInfo.nInfo.at(xx); ++zz) {
							// 변수별 값 출력
							sprintf(buffer, "%s(%s) ", objectInfo.varDesc.at(xx).at(zz).c_str(), objectInfo.records.at(yy).at(zz + 1).c_str());
							fprintf(fp, buffer);
						}
					}

					// 수량 출력
					sprintf(buffer, ": %s EA\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size() - 1).c_str());
					fprintf(fp, buffer);
				}
			}
		}
	}
	catch (exception& ex) {
		WriteReport_Alert("출력 함수에서 오류 발생: %s", ex.what());
	}

	// 알 수 없는 객체
	if (objectInfo.nUnknownObjects > 0) {
		sprintf(buffer, "\n알 수 없는 객체 : %d EA\n", objectInfo.nUnknownObjects);
		fprintf(fp, buffer);
	}

	// *합판, 다루끼 제작 수량 계산
	sort(plywoodInfo.begin(), plywoodInfo.end(), compareVectorString);
	sort(darukiInfo.begin(), darukiInfo.end(), compareVectorString);

	if (plywoodInfo.size() > 0) {
		sprintf(buffer, "\n합판 규격별 제작 수량은 다음과 같습니다.");
		fprintf(fp, buffer);

		for (xx = 0; xx < plywoodInfo.size(); ++xx) {
			sprintf(buffer, "\n%s : %s EA", plywoodInfo.at(xx).at(0).c_str(), plywoodInfo.at(xx).at(1).c_str());
			fprintf(fp, buffer);
		}
		fprintf(fp, "\n");
	}

	if (darukiInfo.size() > 0) {
		sprintf(buffer, "\n합판 제작틀 다루끼 (40*50) 규격별 제작 수량은 다음과 같습니다.");
		fprintf(fp, buffer);

		for (xx = 0; xx < darukiInfo.size(); ++xx) {
			sprintf(buffer, "\n%s : %s EA", darukiInfo.at(xx).at(0).c_str(), darukiInfo.at(xx).at(1).c_str());
			fprintf(fp, buffer);
		}
		fprintf(fp, "\n");
	}

	// *합판, 각재 구매 수량 계산
	// 합판 4x8 규격 (1200*2400) 기준으로 총 면적을 나누면 합판 구매 수량이 나옴
	if (totalAreaOfPlywoods > EPS) {
		sprintf(buffer, "\n합판 구매 수량은 다음과 같습니다.\n총 면적 (%.2f ㎡) ÷ 합판 4x8 규격 (1200*2400) = %.0f 개 (할증 5퍼센트 적용됨)\n", totalAreaOfPlywoods, ceil((totalAreaOfPlywoods / 2.88) * 1.05));
		fprintf(fp, buffer);
	}
	// 각재 다루끼(40*50), 투바이(50*80), 산승각(80*80), 1본은 3600mm
	if ((totalLengthOfTimbers_40x50 > EPS) || (totalLengthOfTimbers_50x80 > EPS) || (totalLengthOfTimbers_80x80 > EPS) || (totalLengthOfTimbersEtc > EPS)) {
		sprintf(buffer, "\n각재 구매 수량은 다음과 같습니다.\n");
		fprintf(fp, buffer);
		if (totalLengthOfTimbers_40x50 > EPS) {
			sprintf(buffer, "다루끼 (40*50) : 총 길이 (%.3f) ÷ 1본 (3600) = %.0f 개 (할증 5퍼센트 적용됨)\n", totalLengthOfTimbers_40x50, ceil((totalLengthOfTimbers_40x50 / 3.6) * 1.05));
			fprintf(fp, buffer);
		}
		if (totalLengthOfTimbers_50x80 > EPS) {
			sprintf(buffer, "투바이 (50*80) : 총 길이 (%.3f) ÷ 1본 (3600) = %.0f 개 (할증 5퍼센트 적용됨)\n", totalLengthOfTimbers_50x80, ceil((totalLengthOfTimbers_50x80 / 3.6) * 1.05));
			fprintf(fp, buffer);
		}
		if (totalLengthOfTimbers_80x80 > EPS) {
			sprintf(buffer, "산승각 (80*80) : 총 길이 (%.3f) ÷ 1본 (3600) = %.0f 개 (할증 5퍼센트 적용됨)\n", totalLengthOfTimbers_80x80, ceil((totalLengthOfTimbers_80x80 / 3.6) * 1.05));
			fprintf(fp, buffer);
		}
		if (totalLengthOfTimbersEtc > EPS) {
			sprintf(buffer, "비규격 : 총 길이 (%.3f) ÷ 1본 (3600) = %.0f 개 (할증 5퍼센트 적용됨)\n", totalLengthOfTimbersEtc, ceil((totalLengthOfTimbersEtc / 3.6) * 1.05));
			fprintf(fp, buffer);
		}
	}
	if ((totalAreaOfPlywoods > EPS) || (totalLengthOfTimbers_40x50 > EPS) || (totalLengthOfTimbers_50x80 > EPS) || (totalLengthOfTimbers_80x80 > EPS) || (totalLengthOfTimbersEtc > EPS)) {
		sprintf(buffer, "\n주의사항: 합판/목재 구매 수량은 다음 객체에 대해서만 계산되었습니다. 추가될 객체가 있다면 개발자에게 문의하십시오.\n합판 / 합판(다각형) / 목재 / 각도목 / 매직바 / 매직아웃코너 / 매직인코너 / 눈썹보 브라켓 v2 / 창문 개구부 합판거푸집\n");
		fprintf(fp, buffer);
	}

	fclose(fp);

	// 객체 종류별로 수량 출력 (중간보고서)
	try {
		for (xx = 0; xx < objectInfo.keyDesc.size(); ++xx) {
			// 레코드를 전부 순회
			for (yy = 0; yy < objectInfo.records.size(); ++yy) {
				// 객체 종류 이름과 레코드의 1번 필드가 일치하는 경우만 찾아서 출력함
				retVal = my_strcmp(objectInfo.keyDesc.at(xx).c_str(), objectInfo.records.at(yy).at(0).c_str());

				if (retVal == 0) {
					// 품목
					sprintf(buffer, "%s | ", objectInfo.keyDesc.at(xx).c_str());
					fprintf(fp_interReport, buffer);

					// 변수별 값 출력
					if ((my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "유로폼") == 0) || (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "스틸폼") == 0)) {
						// 규격
						if (atoi(objectInfo.records.at(yy).at(1).c_str()) > 0) {
							// 규격폼
							sprintf(buffer, "%s X %s | ", objectInfo.records.at(yy).at(2).c_str(), objectInfo.records.at(yy).at(3).c_str());
						}
						else {
							// 비규격품
							length = atof(objectInfo.records.at(yy).at(4).c_str());
							length2 = atof(objectInfo.records.at(yy).at(5).c_str());
							sprintf(buffer, "%.0f X %.0f | ", round(length * 1000, 0), round(length2 * 1000, 0));
						}
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if ((my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "인코너판넬") == 0) || (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "아웃코너판넬") == 0)) {
						length = atof(objectInfo.records.at(yy).at(1).c_str());
						length2 = atof(objectInfo.records.at(yy).at(2).c_str());
						length3 = atof(objectInfo.records.at(yy).at(3).c_str());

						// 규격
						sprintf(buffer, "%.0f X %.0f | ", round(length * 1000, 0), round(length2 * 1000, 0));
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "%.0f | ", round(length3 * 1000, 0));
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "아웃코너앵글") == 0) {
						length = atof(objectInfo.records.at(yy).at(1).c_str());

						// 규격
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "%.0f | ", round(length * 1000, 0));
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "목재") == 0) {
						length = atof(objectInfo.records.at(yy).at(1).c_str());
						length2 = atof(objectInfo.records.at(yy).at(2).c_str());
						length3 = atof(objectInfo.records.at(yy).at(3).c_str());

						// 규격
						sprintf(buffer, "%.0f X %.0f | ", round(length * 1000, 0), round(length2 * 1000, 0));
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "%.0f | ", round(length3 * 1000, 0));
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "본 | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "휠러스페이서") == 0) {
						length = atof(objectInfo.records.at(yy).at(1).c_str());
						length2 = atof(objectInfo.records.at(yy).at(2).c_str());

						// 규격
						sprintf(buffer, "%.0f | ", round(length * 1000, 0));
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "%.0f | ", round(length2 * 1000, 0));
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "원형파이프") == 0) {
						length = atof(objectInfo.records.at(yy).at(1).c_str());

						// 규격
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "%.0f | ", round(length * 1000, 0));
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "사각파이프") == 0) {
						length = atof(objectInfo.records.at(yy).at(2).c_str());

						// 규격
						if (atof(objectInfo.records.at(yy).at(1).c_str()) < EPS) {
							// 사각파이프
							sprintf(buffer, "50 x 50 | ");
						}
						else {
							// 직사각파이프
							sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str());
						}
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "%.0f | ", round(length * 1000, 0));
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "본 | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "콘판넬") == 0) {
						// 규격
						if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "3x6 [910x1820]") == 0) {
							sprintf(buffer, "910 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str());

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "4x8 [1220x2440]") == 0) {
							sprintf(buffer, "1220 X 2440 X %s | ", objectInfo.records.at(yy).at(2).c_str());

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "2x5 [606x1520]") == 0) {
							sprintf(buffer, "606 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str());

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "2x6 [606x1820]") == 0) {
							sprintf(buffer, "606 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str());

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "3x5 [910x1520]") == 0) {
							sprintf(buffer, "910 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str());

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "비규격") == 0) {
							// 가로 X 세로 X 두께
							length = atof(objectInfo.records.at(yy).at(3).c_str());
							length2 = atof(objectInfo.records.at(yy).at(4).c_str());
							sprintf(buffer, "%.0f X %.0f X %s | ", round(length * 1000, 0), round(length2 * 1000, 0), objectInfo.records.at(yy).at(2).c_str());
						}
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "장 | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "합판") == 0) {
						// 규격
						if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "3x6 [910x1820]") == 0) {
							sprintf(buffer, "910 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str());

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "4x8 [1220x2440]") == 0) {
							sprintf(buffer, "1220 X 2440 X %s | ", objectInfo.records.at(yy).at(2).c_str());

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "2x5 [606x1520]") == 0) {
							sprintf(buffer, "606 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str());

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "2x6 [606x1820]") == 0) {
							sprintf(buffer, "606 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str());

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "3x5 [910x1520]") == 0) {
							sprintf(buffer, "910 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str());

						}
						else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "비규격") == 0) {
							// 가로 X 세로 X 두께
							length = atof(objectInfo.records.at(yy).at(3).c_str());
							length2 = atof(objectInfo.records.at(yy).at(4).c_str());
							sprintf(buffer, "%.0f X %.0f X %s | ", round(length * 1000, 0), round(length2 * 1000, 0), objectInfo.records.at(yy).at(2).c_str());
						}
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "장 | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "RS Push-Pull Props 헤드피스 (인양고리 포함)") == 0) {
						// 규격
						sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str());
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "RS Push-Pull Props") == 0) {
						// 규격
						if (atoi(objectInfo.records.at(yy).at(4).c_str()) == 1) {
							// 하부 지지대 있을 경우
							sprintf(buffer, "%s, %s | ", objectInfo.records.at(yy).at(2).c_str(), objectInfo.records.at(yy).at(3).c_str());
						}
						else {
							// 하부 지지대 없을 경우
							sprintf(buffer, "%s, - | ", objectInfo.records.at(yy).at(2).c_str());
						}
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "핀볼트세트") == 0) {
						// 규격
						length = atof(objectInfo.records.at(yy).at(2).c_str());
						sprintf(buffer, "%.0f X %.0f | ", round(length * 1000, 0), round(length * 1000, 0));
						fprintf(fp_interReport, buffer);

						// 길이
						length = atof(objectInfo.records.at(yy).at(1).c_str());
						sprintf(buffer, "%.0f | ", round(length * 1000, 0));
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "결합철물 (사각와셔활용)") == 0) {
						// 규격
						length = atof(objectInfo.records.at(yy).at(2).c_str());
						sprintf(buffer, "%.0f X %.0f | ", round(length * 1000, 0), round(length * 1000, 0));
						fprintf(fp_interReport, buffer);

						// 길이
						length = atof(objectInfo.records.at(yy).at(1).c_str());
						sprintf(buffer, "%.0f | ", round(length * 1000, 0));
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "보 멍에제") == 0) {
						// 규격
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str());
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "PERI동바리 수직재") == 0) {
						// 규격
						sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str());
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(2).c_str());
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "PERI동바리 수평재") == 0) {
						// 규격
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str());
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "GT24 거더") == 0) {
						// 규격
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str());
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "매직바") == 0) {
						// 규격 (매직바 전체 길이)
						sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str());
						fprintf(fp_interReport, buffer);

						// 길이 (합판 너비 X 길이)
						length = atof(objectInfo.records.at(yy).at(3).c_str());
						length2 = atof(objectInfo.records.at(yy).at(4).c_str());
						if (atoi(objectInfo.records.at(yy).at(2).c_str()) == 1) {
							sprintf(buffer, "%s X %.0f | ", objectInfo.records.at(yy).at(5).c_str(), abs(round(length * 1000, 0) - round(length2 * 1000, 0)));
						}
						else {
							sprintf(buffer, "- | ");
						}
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "매직인코너") == 0) {
						// 규격 (매직바 너비 X 길이)
						sprintf(buffer, "%s X %s | ", objectInfo.records.at(yy).at(1).c_str(), objectInfo.records.at(yy).at(2).c_str());
						fprintf(fp_interReport, buffer);

						// 길이 (합판 너비 X 길이)
						length = atof(objectInfo.records.at(yy).at(4).c_str());
						length2 = atof(objectInfo.records.at(yy).at(5).c_str());
						if (atoi(objectInfo.records.at(yy).at(3).c_str()) == 1) {
							sprintf(buffer, "%s X %.0f | ", objectInfo.records.at(yy).at(6).c_str(), abs(round(length * 1000, 0) - round(length2 * 1000, 0)));
						}
						else {
							sprintf(buffer, "- | ");
						}
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "유로폼 후크") == 0) {
						// 규격
						if (objectInfo.records.at(yy).at(2).compare("원형") == 0) {
							sprintf(buffer, "%s, 원형 | ", objectInfo.records.at(yy).at(1).c_str());
						}
						else if (objectInfo.records.at(yy).at(2).compare("사각") == 0) {
							sprintf(buffer, "%s, 사각 | ", objectInfo.records.at(yy).at(1).c_str());
						}
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "블루목심") == 0) {
						// 규격
						sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str());
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "서포트") == 0) {
						// 규격
						sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str());
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "슬래브 테이블폼 (콘판넬)") == 0) {
						// 규격
						sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str());
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "블루클램프") == 0) {
						// 규격
						sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str());
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "빔조인트용 Push-Pull Props 헤드피스") == 0) {
						// 규격
						sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str());
						fprintf(fp_interReport, buffer);

						// 길이 
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "블루 보 브라켓") == 0) {
						// 규격
						sprintf(buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str());
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "단열재") == 0) {
						// 규격
						if (atoi(objectInfo.records.at(yy).at(1).c_str()) == 1) {
							// 원장 가로 X 세로
							sprintf(buffer, "%s X %s | ", objectInfo.records.at(yy).at(2).c_str(), objectInfo.records.at(yy).at(3).c_str());
						}
						else {
							// 실제 가로 X 세로
							sprintf(buffer, "%s X %s | ", objectInfo.records.at(yy).at(4).c_str(), objectInfo.records.at(yy).at(5).c_str());
						}
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "장 | ");
						fprintf(fp_interReport, buffer);

					}
					else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "Push-Pull Props (기성품 및 당사제작품)") == 0) {
						// 규격
						if (atoi(objectInfo.records.at(yy).at(4).c_str()) == 1) {
							// 하부 지지대 있을 경우
							sprintf(buffer, "%s, %s | ", objectInfo.records.at(yy).at(2).c_str(), objectInfo.records.at(yy).at(3).c_str());
						}
						else {
							// 하부 지지대 없을 경우
							sprintf(buffer, "%s, - | ", objectInfo.records.at(yy).at(2).c_str());
						}
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);

					}
					else {
						// 규격, 길이 없고 수량만 표현할 경우

						// 규격
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 길이
						sprintf(buffer, "- | ");
						fprintf(fp_interReport, buffer);

						// 단위
						sprintf(buffer, "개수(EA) | ");
						fprintf(fp_interReport, buffer);
					}

					// 수량 출력
					sprintf(buffer, "%s\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size() - 1).c_str());
					fprintf(fp_interReport, buffer);
				}
			}
		}
	}
	catch (exception& ex) {
		WriteReport_Alert("출력 함수에서 오류 발생: %s", ex.what());
	}

	// 알 수 없는 객체
	if (objectInfo.nUnknownObjects > 0) {
		sprintf(buffer, "알 수 없는 객체 | - | - | - | %d\n", objectInfo.nUnknownObjects);
		fprintf(fp_interReport, buffer);
	}

	char path[1024];
	char* ptr;
	GetFullPathName(filename, 1024, path, &ptr);

	// 마지막 디렉토리 구분자의 위치를 찾음
	ptr = strrstr(path, "\\");
	if (ptr == NULL) {
		ptr = strrstr(path, "/");
		if (ptr == NULL) {
			// 디렉토리 구분자가 없음
			return 1;
		}
	}

	// 마지막 디렉토리 구분자 이후의 문자열을 제거함
	*ptr = '\0';

	GS::UniString infoStr = L"다음 경로에 파일이 저장되었습니다.\n" + GS::UniString(charToWchar(path));

	fclose(fp_interReport);

	// 그룹화 일시정지 OFF
	suspendGroups(false);

	DGAlert(DG_INFORMATION, L"알림", infoStr, "", L"확인", "", "");

	return	err;
}

// 가설재 품목별 수량 내보내기 (켜져 있는 레이어 전부)
GSErrCode	exportElementInfoOnVisibleLayers(void)
{
	GSErrCode	err = NoError;
	short		result;
	unsigned short		xx, yy, zz;
	short		mm;

	// 모든 객체들의 원점 좌표를 전부 저장함
	vector<API_Coord3D>	vecPos;

	// 모든 객체, 보 저장
	GS::Array<API_Guid>		elemList;
	GS::Array<API_Guid>		objects;
	long					nObjects = 0;

	// 선택한 요소들의 정보 요약하기
	API_Element			elem;
	API_ElementMemo		memo;
	SummaryOfObjectInfo	objectInfo;

	char			tempStr[512];
	const char* foundStr;
	bool			foundObject;
	bool			foundExistValue;
	int				retVal;
	int				nInfo;
	API_AddParID	varType;
	vector<string>	record;

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList[1024];
	char			fullLayerName[512];
	vector<LayerList>	layerList;

	// 레이어 타입에 따라 캡쳐 방향 지정
	char* foundLayerName;
	short			layerType = UNDEFINED;

	// 기타
	char			buffer[512];
	char			filename[512];
	GS::UniString	inputFilename;
	GS::UniString	madeFilename;

	// 작업 층 정보
	API_StoryInfo	storyInfo;
	double			workLevel_object;		// 객체의 작업 층 높이


	// 진행바를 표현하기 위한 변수
	GS::UniString       title(L"내보내기 진행 상황");
	GS::UniString       subtitle(L"진행중...");
	short	nPhase;
	Int32	cur, total;

	// 파일 저장을 위한 변수
	FILE* fp;
	FILE* fp_unite;


	// 그룹화 일시정지 ON
	suspendGroups(true);

	result = DGBlankModalDialog(300, 150, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, filenameQuestionHandler, (DGUserData)&inputFilename);

	if (result == DG_CANCEL) {
		DGAlert(DG_INFORMATION, L"알림", L"작업을 취소하였습니다.", "", L"확인", "", "");
		return	NoError;
	}

	if (inputFilename.GetLength() <= 0)
		inputFilename = "notitle";

	madeFilename = inputFilename + L" (통합).csv";
	strcpy(filename, wcharToChar(madeFilename.ToUStr().Get()));

	// [경고] 다이얼로그에서 객체 이미지를 캡쳐할지 여부를 물어봄
	//result = DGAlert (DG_INFORMATION, "캡쳐 여부 질문", "캡쳐 작업을 수행하시겠습니까?", "", "예", "아니오", "");
	//result = DG_CANCEL;

	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount();

	// 보이는 레이어들의 목록 저장하기
	for (xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList[nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// 레이어 이름과 인덱스 저장
	for (xx = 0; xx < nVisibleLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList[xx];
		err = ACAPI_Attribute_Get(&attrib);

		sprintf(fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName[strlen(fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList[xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back(newLayerItem);
	}

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort(layerList.begin(), layerList.end(), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

	// 일시적으로 모든 레이어 숨기기
	for (xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
		}
	}

	fp_unite = fopen(filename, "w+");

	if (fp_unite == NULL) {
		DGAlert(DG_ERROR, L"오류", L"통합 버전 엑셀파일을 만들 수 없습니다.", "", L"확인", "", "");
		return	NoError;
	}

	// 진행 상황 표시하는 기능 - 초기화
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface(APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &total);

	// 보이는 레이어들을 하나씩 순회하면서 전체 요소들을 선택한 후 "선택한 부재 정보 내보내기" 루틴 실행
	for (mm = 1; mm <= nVisibleLayers; ++mm) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList[mm - 1].layerInd;
		err = ACAPI_Attribute_Get(&attrib);

		// 초기화
		objects.Clear();
		vecPos.clear();
		objectInfo.clear();
		objectInfo.nUnknownObjects = 0;

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify(&attrib, NULL);
			}

			// 모든 요소 가져오기
			ACAPI_Element_GetElemList(API_ObjectID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 객체 타입만
			while (elemList.GetSize() > 0) {
				objects.Push(elemList.Pop());
			}
			nObjects = objects.GetSize();

			if (nObjects == 0)
				continue;

			// 레이어 이름 가져옴
			sprintf(fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName[strlen(fullLayerName)] = '\0';

			// 레이어 이름 식별하기 (WALL: 벽, SLAB: 슬래브, COLU: 기둥, BEAM: 보, WLBM: 눈썹보)
			layerType = UNDEFINED;
			foundLayerName = strstr(fullLayerName, "WALL");
			if (foundLayerName != NULL)	layerType = WALL;
			foundLayerName = strstr(fullLayerName, "SLAB");
			if (foundLayerName != NULL)	layerType = SLAB;
			foundLayerName = strstr(fullLayerName, "COLU");
			if (foundLayerName != NULL)	layerType = COLU;
			foundLayerName = strstr(fullLayerName, "BEAM");
			if (foundLayerName != NULL)	layerType = BEAM;
			foundLayerName = strstr(fullLayerName, "WLBM");
			if (foundLayerName != NULL)	layerType = WLBM;

			madeFilename = inputFilename + L" - " + fullLayerName + L".csv";
			strcpy(filename, wcharToChar(madeFilename.ToUStr().Get()));
			fp = fopen(filename, "w+");

			if (fp == NULL) {
				GS::UniString warningStr = L"레이어 " + GS::UniString(fullLayerName) + "은(는) 파일명이 될 수 없으므로 생략합니다.";
				DGAlert(DG_ERROR, L"오류", warningStr, "", L"확인", "", "");
				continue;
			}

			// 레이어 이름
			sprintf(buffer, "\n\n<< 레이어 : %s >>\n", wcharToChar(GS::UniString(fullLayerName).ToUStr().Get()));
			fprintf(fp, buffer);
			fprintf(fp_unite, buffer);

			sprintf(buffer, "%s\n", getExplanationOfLayerCode(fullLayerName));
			fprintf(fp, buffer);
			fprintf(fp_unite, buffer);

			for (xx = 0; xx < nObjects; ++xx) {
				foundObject = false;

				BNZeroMemory(&elem, sizeof(API_Element));
				BNZeroMemory(&memo, sizeof(API_ElementMemo));
				elem.header.guid = objects.Pop();
				err = ACAPI_Element_Get(&elem);

				if (err == NoError && elem.header.hasMemo) {
					err = ACAPI_Element_GetMemo(elem.header.guid, &memo);

					if (err == NoError) {
						// 객체의 원점 수집하기 ==================================
						//API_Coord3D	coord;

						//coord.x = elem.object.pos.x;
						//coord.y = elem.object.pos.y;
						//coord.z = elem.object.level;

						//vecPos.push_back (coord);
						// 객체의 원점 수집하기 ==================================

						// 작업 층 높이 반영 -- 객체
						if (xx == 0) {
							BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
							workLevel_object = 0.0;
							ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo);
							for (yy = 0; yy <= (storyInfo.lastStory - storyInfo.firstStory); ++yy) {
								if (storyInfo.data[0][yy].index == elem.header.floorInd) {
									workLevel_object = storyInfo.data[0][yy].level;
									break;
								}
							}
							BMKillHandle((GSHandle*)&storyInfo.data);
						}

						// 파라미터 스크립트를 강제로 실행시킴
						ACAPI_Goodies(APIAny_RunGDLParScriptID, &elem.header, 0);
						bool	bForce = true;
						ACAPI_Database(APIDb_RefreshElementID, &elem.header, &bForce);

						try {
							for (yy = 0; yy < objectInfo.keyName.size(); ++yy) {
								strcpy(tempStr, objectInfo.keyName.at(yy).c_str());
								foundStr = getParameterStringByName(&memo, tempStr);

								// 객체 종류를 찾았다면,
								if (my_strcmp(foundStr, "") != 0) {
									retVal = my_strcmp(objectInfo.keyDesc.at(yy).c_str(), foundStr);

									if (retVal == 0) {
										foundObject = true;
										foundExistValue = false;

										// 발견한 객체의 데이터를 기반으로 레코드 추가
										if (!record.empty())
											record.clear();

										record.push_back(objectInfo.keyDesc.at(yy));		// 객체 이름
										nInfo = objectInfo.nInfo.at(yy);
										for (zz = 0; zz < nInfo; ++zz) {
											sprintf(buffer, "%s", objectInfo.varName.at(yy).at(zz).c_str());
											varType = getParameterTypeByName(&memo, buffer);

											if ((varType != APIParT_Separator) || (varType != APIParT_Title) || (varType != API_ZombieParT)) {
												if (varType == APIParT_CString)
													sprintf(tempStr, "%s", getParameterStringByName(&memo, buffer));	// 문자열
												else
													sprintf(tempStr, "%.3f", getParameterValueByName(&memo, buffer));	// 숫자
											}
											record.push_back(tempStr);		// 변수값
										}

										objectInfo.quantityPlus1(record);

									}
								}
							}
						}
						catch (exception& ex) {
							WriteReport_Alert("객체 정보 수집에서 오류 발생: %s", ex.what());
						}

						// 끝내 찾지 못하면 알 수 없는 객체로 취급함
						if (foundObject == false)
							objectInfo.nUnknownObjects++;
					}

					ACAPI_DisposeElemMemoHdls(&memo);
				}
			}

			// APIParT_Length인 경우 1000배 곱해서 표현
			// APIParT_Boolean인 경우 예/아니오 표현
			double	length, length2, length3;
			bool	bTitleAppeared;

			// 레코드 정렬 (내림차순 정렬)
			sort(objectInfo.records.begin(), objectInfo.records.end(), compareVectorString);

			// 객체 종류별로 수량 출력
			try {
				for (xx = 0; xx < objectInfo.keyDesc.size(); ++xx) {
					bTitleAppeared = false;

					// 레코드를 전부 순회
					for (yy = 0; yy < objectInfo.records.size(); ++yy) {
						// 객체 종류 이름과 레코드의 1번 필드가 일치하는 경우만 찾아서 출력함
						retVal = my_strcmp(objectInfo.keyDesc.at(xx).c_str(), objectInfo.records.at(yy).at(0).c_str());

						if (retVal == 0) {
							// 제목 출력
							if (bTitleAppeared == false) {
								sprintf(buffer, "\n[%s]\n", objectInfo.keyDesc.at(xx).c_str());
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);
								bTitleAppeared = true;
							}

							// 변수별 값 출력
							if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "유로폼 후크") == 0) {
								// 원형
								if (objectInfo.records.at(yy).at(2).compare("원형") == 0) {
									sprintf(buffer, "원형 / %s", objectInfo.records.at(yy).at(1).c_str());
								}

								// 사각
								if (objectInfo.records.at(yy).at(2).compare("사각") == 0) {
									sprintf(buffer, "사각 / %s", objectInfo.records.at(yy).at(1).c_str());
								}
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if ((my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "유로폼") == 0) || (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "스틸폼") == 0)) {
								// 규격폼
								if (atoi(objectInfo.records.at(yy).at(1).c_str()) > 0) {
									sprintf(buffer, "%s X %s ", objectInfo.records.at(yy).at(2).c_str(), objectInfo.records.at(yy).at(3).c_str());

								}
								// 비규격품
								else {
									length = atof(objectInfo.records.at(yy).at(4).c_str());
									length2 = atof(objectInfo.records.at(yy).at(5).c_str());
									sprintf(buffer, "%.0f X %.0f ", round(length * 1000, 0), round(length2 * 1000, 0));
								}
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if (objectInfo.keyDesc.at(xx).compare("목재") == 0) {
								length = atof(objectInfo.records.at(yy).at(1).c_str());
								length2 = atof(objectInfo.records.at(yy).at(2).c_str());
								length3 = atof(objectInfo.records.at(yy).at(3).c_str());
								sprintf(buffer, "%.0f X %.0f X %.0f ", round(length * 1000, 0), round(length2 * 1000, 0), round(length3 * 1000, 0));
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if (objectInfo.keyDesc.at(xx).compare("각도목") == 0) {
								length = atof(objectInfo.records.at(yy).at(1).c_str());
								length2 = atof(objectInfo.records.at(yy).at(2).c_str());
								length3 = atof(objectInfo.records.at(yy).at(3).c_str());
								sprintf(buffer, "%s X %.0f (커팅각도: %s도)", objectInfo.records.at(yy).at(4).c_str(), round(length3 * 1000, 0), objectInfo.records.at(yy).at(5).c_str());
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "콘판넬") == 0) {
								if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "3x6 [910x1820]") == 0) {
									sprintf(buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str());
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);

								}
								else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "4x8 [1220x2440]") == 0) {
									sprintf(buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str());
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);

								}
								else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "2x5 [606x1520]") == 0) {
									sprintf(buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str());
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);

								}
								else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "2x6 [606x1820]") == 0) {
									sprintf(buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str());
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);

								}
								else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "3x5 [910x1520]") == 0) {
									sprintf(buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str());
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);

								}
								else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "비규격") == 0) {
									// 가로 X 세로 X 두께
									length = atof(objectInfo.records.at(yy).at(3).c_str());
									length2 = atof(objectInfo.records.at(yy).at(4).c_str());
									sprintf(buffer, "%.0f X %.0f X %s ", round(length * 1000, 0), round(length2 * 1000, 0), objectInfo.records.at(yy).at(2).c_str());
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);
								}

							}
							else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "합판") == 0) {
								if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "3x6 [910x1820]") == 0) {
									sprintf(buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str());
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);

									// 제작틀 ON
									if (atoi(objectInfo.records.at(yy).at(5).c_str()) > 0) {
										sprintf(buffer, "(각재 총길이: %s) \n", objectInfo.records.at(yy).at(6).c_str());
										fprintf(fp, buffer);
										fprintf(fp_unite, buffer);

										sprintf(buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str());
										fprintf(fp, buffer);
										fprintf(fp_unite, buffer);
									}

								}
								else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "4x8 [1220x2440]") == 0) {
									sprintf(buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str());
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);

									// 제작틀 ON
									if (atoi(objectInfo.records.at(yy).at(5).c_str()) > 0) {
										sprintf(buffer, "(각재 총길이: %s) \n", objectInfo.records.at(yy).at(6).c_str());
										fprintf(fp, buffer);
										fprintf(fp_unite, buffer);

										sprintf(buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str());
										fprintf(fp, buffer);
										fprintf(fp_unite, buffer);
									}

								}
								else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "2x5 [606x1520]") == 0) {
									sprintf(buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str());
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);

									// 제작틀 ON
									if (atoi(objectInfo.records.at(yy).at(5).c_str()) > 0) {
										sprintf(buffer, "(각재 총길이: %s) \n", objectInfo.records.at(yy).at(6).c_str());
										fprintf(fp, buffer);
										fprintf(fp_unite, buffer);

										sprintf(buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str());
										fprintf(fp, buffer);
										fprintf(fp_unite, buffer);
									}

								}
								else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "2x6 [606x1820]") == 0) {
									sprintf(buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str());
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);

									// 제작틀 ON
									if (atoi(objectInfo.records.at(yy).at(5).c_str()) > 0) {
										sprintf(buffer, "(각재 총길이: %s) \n", objectInfo.records.at(yy).at(6).c_str());
										fprintf(fp, buffer);
										fprintf(fp_unite, buffer);

										sprintf(buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str());
										fprintf(fp, buffer);
										fprintf(fp_unite, buffer);
									}

								}
								else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "3x5 [910x1520]") == 0) {
									sprintf(buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str());
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);

									// 제작틀 ON
									if (atoi(objectInfo.records.at(yy).at(5).c_str()) > 0) {
										sprintf(buffer, "(각재 총길이: %s) \n", objectInfo.records.at(yy).at(6).c_str());
										fprintf(fp, buffer);
										fprintf(fp_unite, buffer);

										sprintf(buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str());
										fprintf(fp, buffer);
										fprintf(fp_unite, buffer);
									}

								}
								else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "비규격") == 0) {
									// 가로 X 세로 X 두께
									length = atof(objectInfo.records.at(yy).at(3).c_str());
									length2 = atof(objectInfo.records.at(yy).at(4).c_str());
									sprintf(buffer, "%.0f X %.0f X %s ", round(length * 1000, 0), round(length2 * 1000, 0), objectInfo.records.at(yy).at(2).c_str());
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);

									// 제작틀 ON
									if (atoi(objectInfo.records.at(yy).at(5).c_str()) > 0) {
										sprintf(buffer, "(각재 총길이: %s) \n", objectInfo.records.at(yy).at(6).c_str());
										fprintf(fp, buffer);
										fprintf(fp_unite, buffer);

										sprintf(buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str());
										fprintf(fp, buffer);
										fprintf(fp_unite, buffer);
									}

								}
								else if (my_strcmp(objectInfo.records.at(yy).at(1).c_str(), "비정형") == 0) {
									sprintf(buffer, "비정형 ");
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);

								}
								else {
									sprintf(buffer, "다각형 ");
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);
								}

							}
							else if (my_strcmp(objectInfo.keyDesc.at(xx).c_str(), "합판(다각형)") == 0) {
								// 합판 면적
								sprintf(buffer, "면적: %.2f ", atof(objectInfo.records.at(yy).at(1).c_str()));
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

								// 제작틀 ON
								if (atoi(objectInfo.records.at(yy).at(2).c_str()) > 0) {
									sprintf(buffer, "(각재 총길이: %.0f) ", round(atof(objectInfo.records.at(yy).at(3).c_str()) * 1000, 0));
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);
								}

							}
							else if (objectInfo.keyDesc.at(xx).compare("RS Push-Pull Props") == 0) {
								// 베이스 플레이트 유무
								if (atoi(objectInfo.records.at(yy).at(1).c_str()) == 1) {
									sprintf(buffer, "베이스 플레이트(있음) ");
								}
								else {
									sprintf(buffer, "베이스 플레이트(없음) ");
								}
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

								// 규격(상부)
								sprintf(buffer, "규격(상부): %s ", objectInfo.records.at(yy).at(2).c_str());
								fprintf(fp, buffer);

								// 규격(하부) - 선택사항
								if (atoi(objectInfo.records.at(yy).at(4).c_str()) == 1) {
									sprintf(buffer, "규격(하부): %s ", objectInfo.records.at(yy).at(3).c_str());
								}
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if (objectInfo.keyDesc.at(xx).compare("Push-Pull Props (기성품 및 당사제작품)") == 0) {
								// 베이스 플레이트 유무
								if (atoi(objectInfo.records.at(yy).at(1).c_str()) == 1) {
									sprintf(buffer, "베이스 플레이트(있음) ");
								}
								else {
									sprintf(buffer, "베이스 플레이트(없음) ");
								}
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

								// 규격(상부)
								sprintf(buffer, "규격(상부): %s ", objectInfo.records.at(yy).at(2).c_str());
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

								// 규격(하부) - 선택사항
								if (atoi(objectInfo.records.at(yy).at(4).c_str()) == 1) {
									sprintf(buffer, "규격(하부): %s ", objectInfo.records.at(yy).at(3).c_str());
								}
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if (objectInfo.keyDesc.at(xx).compare("사각파이프") == 0) {
								// 사각파이프
								if (atof(objectInfo.records.at(yy).at(1).c_str()) < EPS) {
									length = atof(objectInfo.records.at(yy).at(2).c_str());
									sprintf(buffer, "50 x 50 x %.0f ", round(length * 1000, 0));

									// 직사각파이프
								}
								else {
									length = atof(objectInfo.records.at(yy).at(2).c_str());
									sprintf(buffer, "%s x %.0f ", objectInfo.records.at(yy).at(1).c_str(), round(length * 1000, 0));
								}
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if (objectInfo.keyDesc.at(xx).compare("원형파이프") == 0) {
								length = atof(objectInfo.records.at(yy).at(1).c_str());
								sprintf(buffer, "%.0f ", round(length * 1000, 0));
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if (objectInfo.keyDesc.at(xx).compare("아웃코너앵글") == 0) {
								length = atof(objectInfo.records.at(yy).at(1).c_str());
								sprintf(buffer, "%.0f ", round(length * 1000, 0));
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if (objectInfo.keyDesc.at(xx).compare("매직바") == 0) {
								if (atoi(objectInfo.records.at(yy).at(2).c_str()) > 0) {
									length = atof(objectInfo.records.at(yy).at(3).c_str());
									length2 = atof(objectInfo.records.at(yy).at(4).c_str());
									length3 = atof(objectInfo.records.at(yy).at(5).c_str());
									sprintf(buffer, "%.0f / 합판(%.0f X %.0f) ", round(atof(objectInfo.records.at(yy).at(1).c_str()) * 1000, 0), round((length - length2) * 1000, 0), round(length3 * 1000, 0));
								}
								else {
									length = atof(objectInfo.records.at(yy).at(1).c_str());
									sprintf(buffer, "%.0f ", round(length * 1000, 0));
								}
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if (objectInfo.keyDesc.at(xx).compare("매직아웃코너") == 0) {
								sprintf(buffer, "타입(%s) %.0f ", objectInfo.records.at(yy).at(1).c_str(), round(atof(objectInfo.records.at(yy).at(2).c_str()) * 1000, 0));
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);
								if (atoi(objectInfo.records.at(yy).at(3).c_str()) > 0) {
									sprintf(buffer, "합판1(%.0f X %.0f) ", round(atof(objectInfo.records.at(yy).at(2).c_str()) * 1000, 0), round(atof(objectInfo.records.at(yy).at(4).c_str()) * 1000, 0));
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);
									sprintf(buffer, "합판2(%.0f X %.0f) ", round(atof(objectInfo.records.at(yy).at(2).c_str()) * 1000, 0), round(atof(objectInfo.records.at(yy).at(5).c_str()) * 1000, 0));
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);
								}

							}
							else if (objectInfo.keyDesc.at(xx).compare("매직인코너") == 0) {
								if (atoi(objectInfo.records.at(yy).at(3).c_str()) > 0) {
									length = atof(objectInfo.records.at(yy).at(4).c_str());
									length2 = atof(objectInfo.records.at(yy).at(5).c_str());
									length3 = atof(objectInfo.records.at(yy).at(6).c_str());
									sprintf(buffer, "%.0f / 합판(%.0f X %.0f) ", round(atof(objectInfo.records.at(yy).at(2).c_str()) * 1000, 0), round((length - length2) * 1000, 0), round(length3 * 1000, 0));
								}
								else {
									length = atof(objectInfo.records.at(yy).at(2).c_str());
									sprintf(buffer, "%.0f ", round(length * 1000, 0));
								}
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if (objectInfo.keyDesc.at(xx).compare("눈썹보 브라켓 v2") == 0) {
								if (atoi(objectInfo.records.at(yy).at(1).c_str()) > 0) {
									length = atof(objectInfo.records.at(yy).at(2).c_str());
									sprintf(buffer, "각재(%.0f) ", round(length * 1000, 0));
								}
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if (objectInfo.keyDesc.at(xx).compare("단열재") == 0) {
								sprintf(buffer, "원장크기: %.0f X %.0f / 실제크기: %.0f X %.0f (ㄱ형상으로 자름: %s)",
									round(atof(objectInfo.records.at(yy).at(2).c_str()) * 1000, 0), round(atof(objectInfo.records.at(yy).at(3).c_str()) * 1000, 0),
									round(atof(objectInfo.records.at(yy).at(4).c_str()) * 1000, 0), round(atof(objectInfo.records.at(yy).at(5).c_str()) * 1000, 0),
									(atoi(objectInfo.records.at(yy).at(5).c_str()) ? "자름" : "자르지 않음"));
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if (objectInfo.keyDesc.at(xx).compare("PERI동바리 수직재") == 0) {
								length = atof(objectInfo.records.at(yy).at(2).c_str());
								if (atoi(objectInfo.records.at(yy).at(3).c_str()) == 1) {
									sprintf(buffer, "규격(%s) 길이(%.0f) 크로스헤드(%s) ", objectInfo.records.at(yy).at(1).c_str(), round(length * 1000, 0), objectInfo.records.at(yy).at(4).c_str());
								}
								else {
									sprintf(buffer, "규격(%s) 길이(%.0f) ", objectInfo.records.at(yy).at(1).c_str(), round(length * 1000, 0));
								}
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else if (objectInfo.keyDesc.at(xx).compare("서포트") == 0) {
								length = atof(objectInfo.records.at(yy).at(2).c_str());
								if (atoi(objectInfo.records.at(yy).at(3).c_str()) == 1) {
									sprintf(buffer, "규격(%s) 길이(%.0f) 크로스헤드(%s) ", objectInfo.records.at(yy).at(1).c_str(), round(length * 1000, 0), objectInfo.records.at(yy).at(4).c_str());
								}
								else {
									sprintf(buffer, "규격(%s) 길이(%.0f) ", objectInfo.records.at(yy).at(1).c_str(), round(length * 1000, 0));
								}
								fprintf(fp, buffer);
								fprintf(fp_unite, buffer);

							}
							else {
								for (zz = 0; zz < objectInfo.nInfo.at(xx); ++zz) {
									// 변수별 값 출력
									sprintf(buffer, "%s(%s) ", objectInfo.varDesc.at(xx).at(zz).c_str(), objectInfo.records.at(yy).at(zz + 1).c_str());
									fprintf(fp, buffer);
									fprintf(fp_unite, buffer);
								}
							}

							// 수량 출력
							sprintf(buffer, ": %s EA\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size() - 1).c_str());
							fprintf(fp, buffer);
							fprintf(fp_unite, buffer);
						}
					}
				}
			}
			catch (exception& ex) {
				WriteReport_Alert("출력 함수에서 오류 발생: %s", ex.what());
			}

			// 알 수 없는 객체
			if (objectInfo.nUnknownObjects > 0) {
				sprintf(buffer, "\n알 수 없는 객체 : %d EA\n", objectInfo.nUnknownObjects);
				fprintf(fp, buffer);
				fprintf(fp_unite, buffer);
			}

			fclose(fp);

			// 레이어 숨기기
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
		}

		// 진행 상황 표시하는 기능 - 진행
		cur = mm;
		ACAPI_Interface(APIIo_SetProcessValueID, &cur, NULL);
		if (ACAPI_Interface(APIIo_IsProcessCanceledID, NULL, NULL) == APIERR_CANCEL)
			break;
	}

	// 진행 상황 표시하는 기능 - 마무리
	ACAPI_Interface(APIIo_CloseProcessWindowID, NULL, NULL);

	char path[1024];
	char* ptr;
	GetFullPathName(filename, 1024, path, &ptr);

	// 마지막 디렉토리 구분자의 위치를 찾음
	ptr = strrstr(path, "\\");
	if (ptr == NULL) {
		ptr = strrstr(path, "/");
		if (ptr == NULL) {
			// 디렉토리 구분자가 없음
			return 1;
		}
	}

	// 마지막 디렉토리 구분자 이후의 문자열을 제거함
	*ptr = '\0';

	GS::UniString infoStr = L"다음 경로에 파일이 저장되었습니다.\n" + GS::UniString(charToWchar(path));

	fclose(fp_unite);

	// 모든 프로세스를 마치면 처음에 수집했던 보이는 레이어들을 다시 켜놓을 것
	for (xx = 1; xx <= nVisibleLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList[xx - 1];
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify(&attrib, NULL);
			}
		}
	}

	// 그룹화 일시정지 OFF
	suspendGroups(false);

	DGAlert(DG_INFORMATION, L"알림", infoStr, "", L"확인", "", "");

	return	err;
}

// 선택한 객체(부재/가설재)만 보여주기
GSErrCode	filterSelection(void)
{
	GSErrCode	err = NoError;
	short		xx, yy;
	short		result;
	const char* tempStr;
	bool		foundObj;

	FILE* fp;					// 파일 포인터
	char	line[10240];		// 파일에서 읽어온 라인 하나
	char* token;				// 읽어온 문자열의 토큰
	short	lineCount;			// 읽어온 라인 수
	short	tokCount;			// 읽어온 토큰 개수
	char	nthToken[200][50];	// n번째 토큰

	API_Element			elem;
	API_ElementMemo		memo;

	// GUID 저장을 위한 변수
	GS::Array<API_Guid>	objects;	long nObjects = 0;
	GS::Array<API_Guid>	walls;		long nWalls = 0;
	GS::Array<API_Guid>	columns;	long nColumns = 0;
	GS::Array<API_Guid>	beams;		long nBeams = 0;
	GS::Array<API_Guid>	slabs;		long nSlabs = 0;
	GS::Array<API_Guid>	roofs;		long nRoofs = 0;
	GS::Array<API_Guid>	meshes;		long nMeshes = 0;
	GS::Array<API_Guid>	morphs;		long nMorphs = 0;
	GS::Array<API_Guid>	shells;		long nShells = 0;

	// 조건에 맞는 객체들의 GUID 저장
	GS::Array<API_Guid> selection_known;
	GS::Array<API_Guid> selection_unknown;


	// 그룹화 일시정지 ON
	suspendGroups(true);

	ACAPI_Element_GetElemList(API_ObjectID, &objects, APIFilt_OnVisLayer);	nObjects = objects.GetSize();	// 보이는 레이어 상의 객체 타입만 가져오기
	ACAPI_Element_GetElemList(API_WallID, &walls, APIFilt_OnVisLayer);		nWalls = walls.GetSize();		// 보이는 레이어 상의 벽 타입만 가져오기
	ACAPI_Element_GetElemList(API_ColumnID, &columns, APIFilt_OnVisLayer);	nColumns = columns.GetSize();	// 보이는 레이어 상의 기둥 타입만 가져오기
	ACAPI_Element_GetElemList(API_BeamID, &beams, APIFilt_OnVisLayer);		nBeams = beams.GetSize();		// 보이는 레이어 상의 보 타입만 가져오기
	ACAPI_Element_GetElemList(API_SlabID, &slabs, APIFilt_OnVisLayer);		nSlabs = slabs.GetSize();		// 보이는 레이어 상의 슬래브 타입만 가져오기
	ACAPI_Element_GetElemList(API_RoofID, &roofs, APIFilt_OnVisLayer);		nRoofs = roofs.GetSize();		// 보이는 레이어 상의 루프 타입만 가져오기
	ACAPI_Element_GetElemList(API_MeshID, &meshes, APIFilt_OnVisLayer);		nMeshes = meshes.GetSize();	// 보이는 레이어 상의 메시 타입만 가져오기
	ACAPI_Element_GetElemList(API_MorphID, &morphs, APIFilt_OnVisLayer);	nMorphs = morphs.GetSize();	// 보이는 레이어 상의 모프 타입만 가져오기
	ACAPI_Element_GetElemList(API_ShellID, &shells, APIFilt_OnVisLayer);	nShells = shells.GetSize();	// 보이는 레이어 상의 셸 타입만 가져오기

	if (nObjects == 0 && nWalls == 0 && nColumns == 0 && nBeams == 0 && nSlabs == 0 && nRoofs == 0 && nMeshes == 0 && nMorphs == 0 && nShells == 0) {
		result = DGAlert(DG_INFORMATION, L"종료 알림", L"아무 객체도 존재하지 않습니다.", "", L"확인", "", "");
		return	err;
	}

	// 객체 정보 파일 가져오기
	fp = fopen("C:\\objectInfo.csv", "r");

	if (fp == NULL) {
		result = DGAlert(DG_ERROR, L"파일 오류", L"objectInfo.csv 파일을 C:\\로 복사하십시오.", "", L"확인", "", "");
		return	err;
	}

	lineCount = 0;

	while (!feof(fp)) {
		tokCount = 0;
		fgets(line, sizeof(line), fp);

		token = strtok(line, ",");
		tokCount++;
		lineCount++;

		// 한 라인씩 처리
		while (token != NULL) {
			if (strlen(token) > 0) {
				strncpy(nthToken[tokCount - 1], token, strlen(token) + 1);
			}
			token = strtok(NULL, ",");
			tokCount++;
		}

		sprintf(visibleObjInfo.varName[lineCount - 1], "%s", nthToken[0]);
		sprintf(visibleObjInfo.objName[lineCount - 1], "%s", nthToken[1]);
	}

	visibleObjInfo.nKinds = lineCount;

	// 끝에 같은 항목이 2번 들어갈 수 있으므로 중복 제거
	if (lineCount >= 2) {
		if (my_strcmp(visibleObjInfo.varName[lineCount - 1], visibleObjInfo.varName[lineCount - 2]) == 0) {
			visibleObjInfo.nKinds--;
		}
	}

	// 파일 닫기
	fclose(fp);

	// 존재 여부, 표시 여부 초기화
	for (xx = 0; xx < 50; ++xx) {
		visibleObjInfo.bExist[xx] = false;
		visibleObjInfo.bShow[xx] = false;
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

	// 존재 여부 체크
	for (xx = 0; xx < nObjects; ++xx) {
		BNZeroMemory(&elem, sizeof(API_Element));
		BNZeroMemory(&memo, sizeof(API_ElementMemo));
		elem.header.guid = objects[xx];
		err = ACAPI_Element_Get(&elem);

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo(elem.header.guid, &memo);

			if (err == NoError) {
				foundObj = false;

				for (yy = 0; yy < visibleObjInfo.nKinds; ++yy) {
					tempStr = getParameterStringByName(&memo, visibleObjInfo.varName[yy]);
					if (tempStr != NULL) {
						if (my_strcmp(tempStr, visibleObjInfo.objName[yy]) == 0) {
							visibleObjInfo.bExist[yy] = true;
							foundObj = true;
						}
					}
				}

				// 끝내 찾지 못하면 알려지지 않은 Object 타입 리스트에 추가
				if (foundObj == false)
					selection_unknown.Push(objects[xx]);
			}

			ACAPI_DisposeElemMemoHdls(&memo);
		}
	}

	visibleObjInfo.nUnknownObjects = selection_unknown.GetSize();

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

	// [DIALOG] 다이얼로그에서 보이는 레이어 상에 있는 객체들의 종류를 보여주고, 체크한 종류의 객체들만 선택 후 보여줌
	result = DGBlankModalDialog(750, 500, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, filterSelectionHandler, 0);

	if (result == DG_OK) {
		// 선택한 조건에 해당하는 객체들 선택하기
		for (xx = 0; xx < nObjects; ++xx) {
			BNZeroMemory(&elem, sizeof(API_Element));
			BNZeroMemory(&memo, sizeof(API_ElementMemo));
			elem.header.guid = objects[xx];
			err = ACAPI_Element_Get(&elem);
			err = ACAPI_Element_GetMemo(elem.header.guid, &memo);

			for (yy = 0; yy < visibleObjInfo.nKinds; ++yy) {
				tempStr = getParameterStringByName(&memo, visibleObjInfo.varName[yy]);

				if (tempStr != NULL) {
					if ((my_strcmp(tempStr, visibleObjInfo.objName[yy]) == 0) && (visibleObjInfo.bShow[yy] == true)) {
						selection_known.Push(objects[xx]);
					}
				}
			}

			ACAPI_DisposeElemMemoHdls(&memo);
		}

		// 알려진 Object 타입 선택
		selectElements(selection_known);

		// 알려지지 않은 Object 타입 선택
		if (visibleObjInfo.bShow_Unknown == true)	selectElements(selection_unknown);

		// 나머지 타입
		if (visibleObjInfo.bShow_Walls == true)		selectElements(walls);
		if (visibleObjInfo.bShow_Columns == true)	selectElements(columns);
		if (visibleObjInfo.bShow_Beams == true)		selectElements(beams);
		if (visibleObjInfo.bShow_Slabs == true)		selectElements(slabs);
		if (visibleObjInfo.bShow_Roofs == true)		selectElements(roofs);
		if (visibleObjInfo.bShow_Meshes == true)	selectElements(meshes);
		if (visibleObjInfo.bShow_Morphs == true)	selectElements(morphs);
		if (visibleObjInfo.bShow_Shells == true)	selectElements(shells);

		// 선택한 것만 3D로 보여주기
		ACAPI_Automate(APIDo_ShowSelectionIn3DID, NULL, NULL);
	}

	// 그룹화 일시정지 OFF
	suspendGroups(false);

	return	err;
}

// 셀 배열 정보 생성자
BeamTableformCellArray::BeamTableformCellArray() {
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

	for (short xx = 0; xx < 2; ++xx) {
		this->cellElev[xx] = 0.0;
		this->cellPos[xx] = 0.0;

		for (short yy = 0; yy < 30; ++yy) {
			this->cells_Left[yy][xx] = initCell;
			this->cells_Right[yy][xx] = initCell;
			this->cells_Bottom[yy][xx] = initCell;
		}
	}
}

// 셀 배열 정보 초기화
void BeamTableformCellArray::init()
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

	for (short xx = 0; xx < 2; ++xx) {
		this->cellElev[xx] = 0.0;
		this->cellPos[xx] = 0.0;

		for (short yy = 0; yy < 30; ++yy) {
			this->cells_Left[yy][xx] = initCell;
			this->cells_Right[yy][xx] = initCell;
			this->cells_Bottom[yy][xx] = initCell;
		}
	}
}

// 보 테이블폼 가설재 배치도 내보내기
GSErrCode	exportBeamTableformInformation(void)
{
	GSErrCode	err = NoError;
	unsigned short		xx;
	short	result;
	short	mm;

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

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList[2048];
	char			fullLayerName[512];
	vector<LayerList>	layerList;

	// 기타
	char			tempStr[512];
	char			buffer_s[512];
	char			buffer[1024];
	char			filename[512];
	GS::UniString		inputFilename;
	GS::UniString		madeFilename;

	// 파일 저장을 위한 변수
	FILE* fp;


	// 그룹화 일시정지 ON
	suspendGroups(true);

	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount();

	// 보이는 레이어들의 목록 저장하기
	for (xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList[nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// 레이어 이름과 인덱스 저장
	for (xx = 0; xx < nVisibleLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList[xx];
		err = ACAPI_Attribute_Get(&attrib);

		sprintf(fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName[strlen(fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList[xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back(newLayerItem);
	}

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort(layerList.begin(), layerList.end(), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

	// 일시적으로 모든 레이어 숨기기
	for (xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
		}
	}

	result = DGBlankModalDialog(300, 150, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, filenameQuestionHandler, (DGUserData)&inputFilename);

	if (result == DG_CANCEL) {
		DGAlert(DG_INFORMATION, L"알림", L"작업을 취소하였습니다.", "", L"확인", "", "");
		return	NoError;
	}

	if (inputFilename.GetLength() <= 0)
		inputFilename = "notitle";

	madeFilename = inputFilename + L" - 보 테이블폼 물량표.csv";
	strcpy(filename, wcharToChar(madeFilename.ToUStr().Get()));
	fp = fopen(filename, "w+");

	if (fp == NULL) {
		DGAlert(DG_ERROR, L"오류", L"엑셀파일을 만들 수 없습니다.", "", L"확인", "", "");
		return	NoError;
	}

	// 보이는 레이어들을 하나씩 순회하면서 전체 요소들을 선택한 후 "보 테이블폼 물량표" 루틴 실행
	for (mm = 1; mm <= nVisibleLayers; ++mm) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList[mm - 1].layerInd;
		err = ACAPI_Attribute_Get(&attrib);

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify(&attrib, NULL);
			}

			// 초기화
			objectList.clear();
			objectList_Left.clear();
			objectList_Right.clear();
			objectList_Bottom.clear();
			tableformInfo.init();

			// 모든 요소 가져오기
			ACAPI_Element_GetElemList(API_ObjectID, &objects, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 객체 타입만
			nObjects = objects.GetSize();

			if (nObjects == 0)
				continue;

			// 레이어 이름 가져옴
			sprintf(fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName[strlen(fullLayerName)] = '\0';

			for (xx = 0; xx < nObjects; ++xx) {
				BNZeroMemory(&elem, sizeof(API_Element));
				BNZeroMemory(&memo, sizeof(API_ElementMemo));
				elem.header.guid = objects[xx];
				err = ACAPI_Element_Get(&elem);
				err = ACAPI_Element_Get3DInfo(elem.header, &info3D);

				if (err == NoError && elem.header.hasMemo) {
					err = ACAPI_Element_GetMemo(elem.header.guid, &memo);

					if (err == NoError) {
						bValid = false;

						// 초기화
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

						// 원점 좌표 저장
						newObject.origin.x = elem.object.pos.x;
						newObject.origin.y = elem.object.pos.y;
						newObject.origin.z = elem.object.level;

						// 최소점, 최대점 좌표 저장
						newObject.minPos.x = info3D.bounds.xMin;
						newObject.minPos.y = info3D.bounds.yMin;
						newObject.minPos.z = info3D.bounds.zMin;
						newObject.maxPos.x = info3D.bounds.xMax;
						newObject.maxPos.y = info3D.bounds.yMax;
						newObject.maxPos.z = info3D.bounds.zMax;

						// 객체의 타입, 너비와 길이를 저장
						if (my_strcmp(getParameterStringByName(&memo, "u_comp"), "유로폼") == 0) {
							if (my_strcmp(getParameterStringByName(&memo, "u_ins"), "벽눕히기") == 0) {
								newObject.objType = EUROFORM;

								sprintf(tempStr, "%s", getParameterStringByName(&memo, "eu_wid"));
								newObject.width = atof(tempStr) / 1000.0;
								sprintf(tempStr, "%s", getParameterStringByName(&memo, "eu_hei"));
								newObject.length = atof(tempStr) / 1000.0;

								ang_x = (int)round(RadToDegree(getParameterValueByName(&memo, "ang_x")), 0);
								ang_y = (int)round(RadToDegree(getParameterValueByName(&memo, "ang_y")), 0);

								if (ang_x == 90)
									newObject.attachPosition = LEFT_SIDE;
								else if (ang_x == 0)
									newObject.attachPosition = BOTTOM_SIDE;

								bValid = true;
							}
						}

						else if (my_strcmp(getParameterStringByName(&memo, "g_comp"), "합판") == 0) {
							newObject.objType = PLYWOOD;

							ang_x = (int)round(RadToDegree(getParameterValueByName(&memo, "p_ang")), 0);

							if (my_strcmp(getParameterStringByName(&memo, "w_dir"), "벽눕히기") == 0) {
								newObject.width = getParameterValueByName(&memo, "p_wid");
								newObject.length = getParameterValueByName(&memo, "p_leng");
								newObject.attachPosition = LEFT_SIDE;

								bValid = true;
							}
							else if (my_strcmp(getParameterStringByName(&memo, "w_dir"), "벽세우기") == 0) {
								newObject.width = getParameterValueByName(&memo, "p_leng");
								newObject.length = getParameterValueByName(&memo, "p_wid");
								newObject.attachPosition = LEFT_SIDE;

								bValid = true;
							}
							else if (my_strcmp(getParameterStringByName(&memo, "w_dir"), "바닥깔기") == 0) {
								newObject.width = getParameterValueByName(&memo, "p_wid");
								newObject.length = getParameterValueByName(&memo, "p_leng");
								newObject.attachPosition = BOTTOM_SIDE;

								bValid = true;
							}
						}

						else if (my_strcmp(getParameterStringByName(&memo, "g_comp"), "합판(다각형)") == 0) {
							newObject.objType = PLYWOOD_POLY;

							if (my_strcmp(getParameterStringByName(&memo, "w_dir"), "벽세우기") == 0) {
								newObject.width = 0.0;
								newObject.length = 0.0;
								newObject.attachPosition = LEFT_SIDE;

								bValid = true;
							}
							else if (my_strcmp(getParameterStringByName(&memo, "w_dir"), "바닥깔기") == 0) {
								newObject.width = 0.0;
								newObject.length = 0.0;
								newObject.attachPosition = BOTTOM_SIDE;

								bValid = true;
							}
						}

						if (bValid == true)
							objectList.push_back(newObject);
					}

					ACAPI_DisposeElemMemoHdls(&memo);
				}
			}

			nObjects = (long)objectList.size();

			// 보 방향을 찾아냄 (가로인가, 세로인가?)
			bFirst = false;
			for (xx = 0; xx < nObjects; ++xx) {
				if (objectList[xx].attachPosition != BOTTOM_SIDE) {
					if (bFirst == false) {
						xmin = xmax = objectList[xx].origin.x;
						ymin = ymax = objectList[xx].origin.y;
						bFirst = true;
					}
					else {
						if (xmin > objectList[xx].origin.x)	xmin = objectList[xx].origin.x;
						if (ymin > objectList[xx].origin.y)	ymin = objectList[xx].origin.y;
						if (xmax < objectList[xx].origin.x)	xmax = objectList[xx].origin.x;
						if (ymax < objectList[xx].origin.y)	ymax = objectList[xx].origin.y;
					}
				}
			}
			if (abs(xmax - xmin) > abs(ymax - ymin))
				tableformInfo.iBeamDirection = HORIZONTAL_DIRECTION;
			else
				tableformInfo.iBeamDirection = VERTICAL_DIRECTION;

			// 객체 정렬하기
			for (xx = 0; xx < nObjects; ++xx) {
				if (tableformInfo.iBeamDirection == HORIZONTAL_DIRECTION)
					sort(objectList.begin(), objectList.end(), comparePosX);		// X 오름차순 정렬
				else
					sort(objectList.begin(), objectList.end(), comparePosY);		// Y 오름차순 정렬
			}

			// 센터 위치 찾기
			xcenter = (xmax - xmin) / 2 + xmin;
			ycenter = (ymax - ymin) / 2 + ymin;

			// 수집된 정보 분류하기
			for (xx = 0; xx < nObjects; ++xx) {
				// 왼쪽/아래쪽(최소)과 오른쪽/위쪽(최대) 측판 사이의 중간점을 기준으로 구분
				if (tableformInfo.iBeamDirection == HORIZONTAL_DIRECTION) {
					if (objectList[xx].origin.y > ycenter) {
						if (objectList[xx].attachPosition == LEFT_SIDE)
							objectList[xx].attachPosition = RIGHT_SIDE;	// 위쪽
					}
				}
				else {
					if (objectList[xx].origin.x > xcenter) {
						if (objectList[xx].attachPosition == LEFT_SIDE)
							objectList[xx].attachPosition = RIGHT_SIDE;	// 오른쪽
					}
				}
			}

			// 자재들의 시작점/끝점 좌표를 업데이트
			for (xx = 0; xx < nObjects; ++xx) {
				if (tableformInfo.iBeamDirection == HORIZONTAL_DIRECTION) {
					// 가로 방향
					objectList[xx].beginPos = objectList[xx].minPos.x;
					objectList[xx].endPos = objectList[xx].beginPos + objectList[xx].length;
				}
				else {
					// 세로 방향
					objectList[xx].beginPos = objectList[xx].minPos.y;
					objectList[xx].endPos = objectList[xx].beginPos + objectList[xx].length;
				}
			}

			// 자재들의 최소점을 기준으로 고도점을 스캔 (가장 작은 값을 1단 기준, 가장 큰 값을 2단 기준으로 정함)
			double	lowElev, highElev;
			lowElev = highElev = objectList[0].minPos.z;

			for (xx = 0; xx < nObjects; ++xx) {
				if (objectList[xx].minPos.z < lowElev)		lowElev = objectList[xx].minPos.z;
				if (objectList[xx].minPos.z > highElev)	highElev = objectList[xx].minPos.z;
			}

			tableformInfo.cellElev[0] = lowElev;
			tableformInfo.cellElev[1] = highElev;

			// 자재들의 최소점을 기준으로 X,Y점을 스캔
			if (tableformInfo.iBeamDirection == VERTICAL_DIRECTION) {
				lowElev = highElev = objectList[0].minPos.x;

				for (xx = 0; xx < nObjects; ++xx) {
					if (objectList[xx].minPos.x < lowElev)		lowElev = objectList[xx].minPos.x;
					if (objectList[xx].minPos.x > highElev)	highElev = objectList[xx].minPos.x;
				}
			}
			else {
				lowElev = highElev = objectList[0].minPos.y;

				for (xx = 0; xx < nObjects; ++xx) {
					if (objectList[xx].minPos.y > lowElev)		lowElev = objectList[xx].minPos.y;
					if (objectList[xx].minPos.y < highElev)	highElev = objectList[xx].minPos.y;
				}
			}

			tableformInfo.cellPos[0] = lowElev;
			tableformInfo.cellPos[1] = highElev;

			// 자재들을 위치별로 분류함
			for (xx = 0; xx < nObjects; ++xx) {
				if (objectList[xx].attachPosition == LEFT_SIDE)	objectList_Left.push_back(objectList[xx]);
				if (objectList[xx].attachPosition == RIGHT_SIDE)	objectList_Right.push_back(objectList[xx]);
				if (objectList[xx].attachPosition == BOTTOM_SIDE)	objectList_Bottom.push_back(objectList[xx]);
			}
			nObjects_Left = (long)objectList_Left.size();
			nObjects_Right = (long)objectList_Right.size();
			nObjects_Bottom = (long)objectList_Bottom.size();

			// 셀 배열 정보 구축 - 좌측면
			double	curCell, adjCell;
			xx = 0;
			while (xx < nObjects_Left) {
				// 마지막 셀이 아닌 경우
				if (xx < nObjects_Left - 1) {
					curCell = objectList_Left[xx].minPos.z;
					adjCell = objectList_Left[xx + 1].minPos.z;

					if ((objectList_Left[xx].objType == objectList_Left[xx + 1].objType) && ((abs(objectList_Left[xx].beginPos - objectList_Left[xx + 1].beginPos) < 0.080) || (abs(objectList_Left[xx].endPos - objectList_Left[xx + 1].endPos) < 0.080))) {
						// 현재/다음 셀의 시작점-끝점이 일치할 경우
						if (curCell < adjCell) {
							tableformInfo.cells_Left[tableformInfo.nCells_Left][0] = objectList_Left[xx];
							tableformInfo.cells_Left[tableformInfo.nCells_Left][1] = objectList_Left[xx + 1];
						}
						else {
							tableformInfo.cells_Left[tableformInfo.nCells_Left][0] = objectList_Left[xx + 1];
							tableformInfo.cells_Left[tableformInfo.nCells_Left][1] = objectList_Left[xx];
						}

						xx += 2;
						tableformInfo.nCells_Left++;
					}
					else {
						// 현재/다음 셀의 시작점-끝점이 일치하지 않을 경우
						if (abs(tableformInfo.cellElev[0] - curCell) < abs(tableformInfo.cellElev[1] - curCell))
							tableformInfo.cells_Left[tableformInfo.nCells_Left][0] = objectList_Left[xx];
						else
							tableformInfo.cells_Left[tableformInfo.nCells_Left][1] = objectList_Left[xx];

						xx++;
						tableformInfo.nCells_Left++;
					}

					// 마지막 셀인 경우
				}
				else {
					curCell = objectList_Left[xx].minPos.z;
					adjCell = objectList_Left[xx - 1].minPos.z;

					if ((objectList_Left[xx].objType == objectList_Left[xx - 1].objType) && ((abs(objectList_Left[xx].beginPos - objectList_Left[xx - 1].beginPos) < 0.080) || (abs(objectList_Left[xx].endPos - objectList_Left[xx - 1].endPos) < 0.080))) {
						// 이전/현재 셀의 시작점-끝점이 일치할 경우
						if (abs(tableformInfo.cellElev[0] - curCell) < abs(tableformInfo.cellElev[1] - curCell))
							tableformInfo.cells_Left[tableformInfo.nCells_Left - 1][0] = objectList_Left[xx];
						else
							tableformInfo.cells_Left[tableformInfo.nCells_Left - 1][1] = objectList_Left[xx];

						xx++;
					}
					else {
						// 현재/다음 셀의 시작점-끝점이 일치하지 않을 경우
						if (abs(tableformInfo.cellElev[0] - curCell) < abs(tableformInfo.cellElev[1] - curCell))
							tableformInfo.cells_Left[tableformInfo.nCells_Left][0] = objectList_Left[xx];
						else
							tableformInfo.cells_Left[tableformInfo.nCells_Left][1] = objectList_Left[xx];

						xx++;
						tableformInfo.nCells_Left++;
					}
				}
			}

			// 셀 배열 정보 구축 - 우측면
			xx = 0;
			while (xx < nObjects_Right) {
				// 마지막 셀이 아닌 경우
				if (xx < nObjects_Right - 1) {
					curCell = objectList_Right[xx].minPos.z;
					adjCell = objectList_Right[xx + 1].minPos.z;

					if ((objectList_Right[xx].objType == objectList_Right[xx + 1].objType) && ((abs(objectList_Right[xx].beginPos - objectList_Right[xx + 1].beginPos) < 0.080) || (abs(objectList_Right[xx].endPos - objectList_Right[xx + 1].endPos) < 0.080))) {
						// 현재/다음 셀의 시작점-끝점이 일치할 경우
						if (curCell < adjCell) {
							tableformInfo.cells_Right[tableformInfo.nCells_Right][0] = objectList_Right[xx];
							tableformInfo.cells_Right[tableformInfo.nCells_Right][1] = objectList_Right[xx + 1];
						}
						else {
							tableformInfo.cells_Right[tableformInfo.nCells_Right][0] = objectList_Right[xx + 1];
							tableformInfo.cells_Right[tableformInfo.nCells_Right][1] = objectList_Right[xx];
						}

						xx += 2;
						tableformInfo.nCells_Right++;
					}
					else {
						// 현재/다음 셀의 시작점-끝점이 일치하지 않을 경우
						if (abs(tableformInfo.cellElev[0] - curCell) < abs(tableformInfo.cellElev[1] - curCell))
							tableformInfo.cells_Right[tableformInfo.nCells_Right][0] = objectList_Right[xx];
						else
							tableformInfo.cells_Right[tableformInfo.nCells_Right][1] = objectList_Right[xx];

						xx++;
						tableformInfo.nCells_Right++;
					}

					// 마지막 셀인 경우
				}
				else {
					curCell = objectList_Right[xx].minPos.z;
					adjCell = objectList_Right[xx - 1].minPos.z;

					if ((objectList_Right[xx].objType == objectList_Right[xx - 1].objType) && ((abs(objectList_Right[xx].beginPos - objectList_Right[xx - 1].beginPos) < 0.080) || (abs(objectList_Right[xx].endPos - objectList_Right[xx - 1].endPos) < 0.080))) {
						// 이전/현재 셀의 시작점-끝점이 일치할 경우
						if (abs(tableformInfo.cellElev[0] - curCell) < abs(tableformInfo.cellElev[1] - curCell))
							tableformInfo.cells_Right[tableformInfo.nCells_Right - 1][0] = objectList_Right[xx];
						else
							tableformInfo.cells_Right[tableformInfo.nCells_Right - 1][1] = objectList_Right[xx];

						xx++;
					}
					else {
						// 현재/다음 셀의 시작점-끝점이 일치하지 않을 경우
						if (abs(tableformInfo.cellElev[0] - curCell) < abs(tableformInfo.cellElev[1] - curCell))
							tableformInfo.cells_Right[tableformInfo.nCells_Right][0] = objectList_Right[xx];
						else
							tableformInfo.cells_Right[tableformInfo.nCells_Right][1] = objectList_Right[xx];

						xx++;
						tableformInfo.nCells_Right++;
					}
				}
			}

			// 셀 배열 정보 구축 - 하부면
			xx = 0;
			if (tableformInfo.iBeamDirection == VERTICAL_DIRECTION) {
				while (xx < nObjects_Bottom) {
					// 마지막 셀이 아닌 경우
					if (xx < nObjects_Bottom - 1) {
						curCell = objectList_Bottom[xx].minPos.x;
						adjCell = objectList_Bottom[xx + 1].minPos.x;

						if ((objectList_Bottom[xx].objType == objectList_Bottom[xx + 1].objType) && ((abs(objectList_Bottom[xx].beginPos - objectList_Bottom[xx + 1].beginPos) < 0.080) || (abs(objectList_Bottom[xx].endPos - objectList_Bottom[xx + 1].endPos) < 0.080))) {
							// 현재/다음 셀의 시작점-끝점이 일치할 경우
							if (curCell < adjCell) {
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][0] = objectList_Bottom[xx];
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][1] = objectList_Bottom[xx + 1];
							}
							else {
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][0] = objectList_Bottom[xx + 1];
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][1] = objectList_Bottom[xx];
							}

							xx += 2;
							tableformInfo.nCells_Bottom++;
						}
						else {
							// 현재/다음 셀의 시작점-끝점이 일치하지 않을 경우
							if (abs(tableformInfo.cellPos[0] - curCell) < abs(tableformInfo.cellPos[1] - curCell))
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][0] = objectList_Bottom[xx];
							else
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][1] = objectList_Bottom[xx];

							xx++;
							tableformInfo.nCells_Bottom++;
						}

						// 마지막 셀인 경우
					}
					else {
						curCell = objectList_Bottom[xx].minPos.x;
						adjCell = objectList_Bottom[xx - 1].minPos.x;

						if ((objectList_Bottom[xx].objType == objectList_Bottom[xx - 1].objType) && ((abs(objectList_Bottom[xx].beginPos - objectList_Bottom[xx - 1].beginPos) < 0.080) || (abs(objectList_Bottom[xx].endPos - objectList_Bottom[xx - 1].endPos) < 0.080))) {
							// 이전/현재 셀의 시작점-끝점이 일치할 경우
							if (abs(tableformInfo.cellPos[0] - curCell) < abs(tableformInfo.cellPos[1] - curCell))
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom - 1][0] = objectList_Bottom[xx];
							else
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom - 1][1] = objectList_Bottom[xx];

							xx++;
						}
						else {
							// 현재/다음 셀의 시작점-끝점이 일치하지 않을 경우
							if (abs(tableformInfo.cellPos[0] - curCell) < abs(tableformInfo.cellPos[1] - curCell))
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][0] = objectList_Bottom[xx];
							else
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][1] = objectList_Bottom[xx];

							xx++;
							tableformInfo.nCells_Bottom++;
						}
					}
				}
			}
			else {
				while (xx < nObjects_Bottom) {
					// 마지막 셀이 아닌 경우
					if (xx < nObjects_Bottom - 1) {
						curCell = objectList_Bottom[xx].minPos.y;
						adjCell = objectList_Bottom[xx + 1].minPos.y;

						if ((objectList_Bottom[xx].objType == objectList_Bottom[xx + 1].objType) && ((abs(objectList_Bottom[xx].beginPos - objectList_Bottom[xx + 1].beginPos) < 0.080) || (abs(objectList_Bottom[xx].endPos - objectList_Bottom[xx + 1].endPos) < 0.080))) {
							// 현재/다음 셀의 시작점-끝점이 일치할 경우
							if (curCell < adjCell) {
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][1] = objectList_Bottom[xx];
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][0] = objectList_Bottom[xx + 1];
							}
							else {
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][1] = objectList_Bottom[xx + 1];
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][0] = objectList_Bottom[xx];
							}

							xx += 2;
							tableformInfo.nCells_Bottom++;
						}
						else {
							// 현재/다음 셀의 시작점-끝점이 일치하지 않을 경우
							if (abs(tableformInfo.cellPos[0] - curCell) < abs(tableformInfo.cellPos[1] - curCell))
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][1] = objectList_Bottom[xx];
							else
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][0] = objectList_Bottom[xx];

							xx++;
							tableformInfo.nCells_Bottom++;
						}

						// 마지막 셀인 경우
					}
					else {
						curCell = objectList_Bottom[xx].minPos.x;
						adjCell = objectList_Bottom[xx - 1].minPos.x;

						if ((objectList_Bottom[xx].objType == objectList_Bottom[xx - 1].objType) && ((abs(objectList_Bottom[xx].beginPos - objectList_Bottom[xx - 1].beginPos) < 0.080) || (abs(objectList_Bottom[xx].endPos - objectList_Bottom[xx - 1].endPos) < 0.080))) {
							// 이전/현재 셀의 시작점-끝점이 일치할 경우
							if (abs(tableformInfo.cellPos[0] - curCell) < abs(tableformInfo.cellPos[1] - curCell))
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom - 1][1] = objectList_Bottom[xx];
							else
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom - 1][0] = objectList_Bottom[xx];

							xx++;
						}
						else {
							// 현재/다음 셀의 시작점-끝점이 일치하지 않을 경우
							if (abs(tableformInfo.cellPos[0] - curCell) < abs(tableformInfo.cellPos[1] - curCell))
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][1] = objectList_Bottom[xx];
							else
								tableformInfo.cells_Bottom[tableformInfo.nCells_Bottom][0] = objectList_Bottom[xx];

							xx++;
							tableformInfo.nCells_Bottom++;
						}
					}
				}
			}

			// 정보 출력
			if (tableformInfo.nCells_Left + tableformInfo.nCells_Right + tableformInfo.nCells_Bottom > 0) {
				sprintf(buffer, "<< 레이어 : %s >>\n", wcharToChar(GS::UniString(fullLayerName).ToUStr().Get()));
				fprintf(fp, buffer);

				sprintf(buffer, "%s\n\n", getExplanationOfLayerCode(fullLayerName));
				fprintf(fp, buffer);

				if (tableformInfo.nCells_Left > 0) {
					fprintf(fp, "측면1\n");
					// 헤더
					strcpy(buffer, "");
					for (xx = 0; xx < tableformInfo.nCells_Left; ++xx) {
						if (tableformInfo.cells_Left[xx][0].objType != NONE) {
							if (tableformInfo.cells_Left[xx][0].objType == EUROFORM)
								strcat(buffer, "유로폼,");
							if (tableformInfo.cells_Left[xx][0].objType == PLYWOOD)
								strcat(buffer, "합판,");
							if (tableformInfo.cells_Left[xx][0].objType == PLYWOOD_POLY)
								strcat(buffer, "합판(다각형),");
						}
						else if (tableformInfo.cells_Left[xx][1].objType != NONE) {
							if (tableformInfo.cells_Left[xx][1].objType == EUROFORM)
								strcat(buffer, "유로폼,");
							if (tableformInfo.cells_Left[xx][1].objType == PLYWOOD)
								strcat(buffer, "합판,");
							if (tableformInfo.cells_Left[xx][1].objType == PLYWOOD_POLY)
								strcat(buffer, "합판(다각형),");
						}
					}
					// 2단
					strcat(buffer, "\n");
					for (xx = 0; xx < tableformInfo.nCells_Left; ++xx) {
						if (tableformInfo.cells_Left[xx][1].objType != NONE) {
							sprintf(buffer_s, "%.0f x %.0f,", tableformInfo.cells_Left[xx][1].width * 1000, tableformInfo.cells_Left[xx][1].length * 1000);
							strcat(buffer, buffer_s);
						}
						else {
							sprintf(buffer_s, "↑,");
							strcat(buffer, buffer_s);
						}
					}
					// 1단
					strcat(buffer, "\n");
					for (xx = 0; xx < tableformInfo.nCells_Left; ++xx) {
						if (tableformInfo.cells_Left[xx][0].objType != NONE) {
							sprintf(buffer_s, "%.0f x %.0f,", tableformInfo.cells_Left[xx][0].width * 1000, tableformInfo.cells_Left[xx][0].length * 1000);
							strcat(buffer, buffer_s);
						}
						else {
							sprintf(buffer_s, "→,");
							strcat(buffer, buffer_s);
						}
					}
					strcat(buffer, "\n\n");
					fprintf(fp, buffer);
				}

				if (tableformInfo.nCells_Right > 0) {
					fprintf(fp, "측면2\n");
					// 헤더
					strcpy(buffer, "");
					for (xx = 0; xx < tableformInfo.nCells_Right; ++xx) {
						if (tableformInfo.cells_Right[xx][0].objType != NONE) {
							if (tableformInfo.cells_Right[xx][0].objType == EUROFORM)
								strcat(buffer, "유로폼,");
							if (tableformInfo.cells_Right[xx][0].objType == PLYWOOD)
								strcat(buffer, "합판,");
							if (tableformInfo.cells_Right[xx][0].objType == PLYWOOD_POLY)
								strcat(buffer, "합판(다각형),");
						}
						else if (tableformInfo.cells_Right[xx][1].objType != NONE) {
							if (tableformInfo.cells_Right[xx][1].objType == EUROFORM)
								strcat(buffer, "유로폼,");
							if (tableformInfo.cells_Right[xx][1].objType == PLYWOOD)
								strcat(buffer, "합판,");
							if (tableformInfo.cells_Right[xx][1].objType == PLYWOOD_POLY)
								strcat(buffer, "합판(다각형),");
						}
					}
					// 2단
					strcat(buffer, "\n");
					for (xx = 0; xx < tableformInfo.nCells_Right; ++xx) {
						if (tableformInfo.cells_Right[xx][1].objType != NONE) {
							sprintf(buffer_s, "%.0f x %.0f,", tableformInfo.cells_Right[xx][1].width * 1000, tableformInfo.cells_Right[xx][1].length * 1000);
							strcat(buffer, buffer_s);
						}
						else {
							sprintf(buffer_s, "↑,");
							strcat(buffer, buffer_s);
						}
					}
					// 1단
					strcat(buffer, "\n");
					for (xx = 0; xx < tableformInfo.nCells_Right; ++xx) {
						if (tableformInfo.cells_Right[xx][0].objType != NONE) {
							sprintf(buffer_s, "%.0f x %.0f,", tableformInfo.cells_Right[xx][0].width * 1000, tableformInfo.cells_Right[xx][0].length * 1000);
							strcat(buffer, buffer_s);
						}
						else {
							sprintf(buffer_s, "→,");
							strcat(buffer, buffer_s);
						}
					}
					strcat(buffer, "\n\n");
					fprintf(fp, buffer);
				}

				if (tableformInfo.nCells_Bottom > 0) {
					fprintf(fp, "하부면\n");
					// 헤더
					strcpy(buffer, "");
					for (xx = 0; xx < tableformInfo.nCells_Bottom; ++xx) {
						if (tableformInfo.cells_Bottom[xx][0].objType != NONE) {
							if (tableformInfo.cells_Bottom[xx][0].objType == EUROFORM)
								strcat(buffer, "유로폼,");
							if (tableformInfo.cells_Bottom[xx][0].objType == PLYWOOD)
								strcat(buffer, "합판,");
							if (tableformInfo.cells_Bottom[xx][0].objType == PLYWOOD_POLY)
								strcat(buffer, "합판(다각형),");
						}
						else if (tableformInfo.cells_Bottom[xx][1].objType != NONE) {
							if (tableformInfo.cells_Bottom[xx][1].objType == EUROFORM)
								strcat(buffer, "유로폼,");
							if (tableformInfo.cells_Bottom[xx][1].objType == PLYWOOD)
								strcat(buffer, "합판,");
							if (tableformInfo.cells_Bottom[xx][1].objType == PLYWOOD_POLY)
								strcat(buffer, "합판(다각형),");
						}
					}
					// 2단
					strcat(buffer, "\n");
					for (xx = 0; xx < tableformInfo.nCells_Bottom; ++xx) {
						if (tableformInfo.cells_Bottom[xx][1].objType != NONE) {
							sprintf(buffer_s, "%.0f x %.0f,", tableformInfo.cells_Bottom[xx][1].width * 1000, tableformInfo.cells_Bottom[xx][1].length * 1000);
							strcat(buffer, buffer_s);
						}
						else {
							sprintf(buffer_s, "↑,");
							strcat(buffer, buffer_s);
						}
					}
					// 1단
					strcat(buffer, "\n");
					for (xx = 0; xx < tableformInfo.nCells_Bottom; ++xx) {
						if (tableformInfo.cells_Bottom[xx][0].objType != NONE) {
							sprintf(buffer_s, "%.0f x %.0f,", tableformInfo.cells_Bottom[xx][0].width * 1000, tableformInfo.cells_Bottom[xx][0].length * 1000);
							strcat(buffer, buffer_s);
						}
						else {
							sprintf(buffer_s, "→,");
							strcat(buffer, buffer_s);
						}
					}
					strcat(buffer, "\n\n");
					fprintf(fp, buffer);
				}

				strcpy(buffer, "\n\n");
				fprintf(fp, buffer);

			}
			else {
				// 실패한 경우
				sprintf(buffer, "<< 레이어 : %s >>\n", wcharToChar(GS::UniString(fullLayerName).ToUStr().Get()));
				fprintf(fp, buffer);

				sprintf(buffer, "\n정규화된 보 테이블폼 레이어가 아닙니다.\n");
				fprintf(fp, buffer);

				strcpy(buffer, "\n\n");
				fprintf(fp, buffer);
			}

			// 객체 비우기
			objects.Clear();
			objectList.clear();
			objectList_Left.clear();
			objectList_Right.clear();
			objectList_Bottom.clear();

			// 레이어 숨기기
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
		}
	}

	char path[1024];
	char* ptr;
	GetFullPathName(filename, 1024, path, &ptr);

	// 마지막 디렉토리 구분자의 위치를 찾음
	ptr = strrstr(path, "\\");
	if (ptr == NULL) {
		ptr = strrstr(path, "/");
		if (ptr == NULL) {
			// 디렉토리 구분자가 없음
			return 1;
		}
	}

	// 마지막 디렉토리 구분자 이후의 문자열을 제거함
	*ptr = '\0';

	GS::UniString infoStr = L"다음 경로에 파일이 저장되었습니다.\n" + GS::UniString(charToWchar(path));

	// 파일 닫기
	fclose(fp);

	// 모든 프로세스를 마치면 처음에 수집했던 보이는 레이어들을 다시 켜놓을 것
	for (xx = 1; xx <= nVisibleLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList[xx - 1];
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify(&attrib, NULL);
			}
		}
	}

	// 그룹화 일시정지 OFF
	suspendGroups(false);

	DGAlert(DG_INFORMATION, L"알림", infoStr, "", L"확인", "", "");

	return err;
}

// 테이블 단위별 거푸집 면적 내보내기
GSErrCode	calcTableformArea(void)
{
	GSErrCode	err = NoError;
	unsigned short		xx, yy;
	short	result;
	short	action_mode;
	short	mm;

	// 모든 객체, 모프 저장
	GS::Array<API_Guid>		elemList;
	GS::Array<API_Guid>		objects;
	long					nObjects = 0;
	GS::Array<API_Guid>		morphs;
	long					nMorphs = 0;

	// 선택한 요소들의 정보 요약하기
	API_Element			elem;
	API_ElementMemo		memo;

	API_ElementQuantity	quantity;
	API_Quantities		quantities;
	API_QuantitiesMask	mask;
	API_QuantityPar		params;

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList[1024];
	char			fullLayerName[512];
	vector<LayerList>	layerList;

	// 기타
	char			buffer[512];
	char			filename[512];
	GS::UniString		inputFilename;
	GS::UniString		madeFilename;

	// 진행바를 표현하기 위한 변수
	GS::UniString       title(L"내보내기 진행 상황");
	GS::UniString       subtitle(L"진행중...");
	short	nPhase;
	Int32	cur, total;

	// 파일 저장을 위한 변수
	FILE* fp_unite;

	// 면적 값 저장
	double	extractedValue;
	double	totalArea = 0.0;		// 총 면적값 (각 레이어 합산)
	double	totalAreaAll = 0.0;		// 총 면적값 (모든 레이어 합산)

	// 레이어 체계에 따라 면적값을 보기 쉽게 표로 만들기 위한 변수
	GS::Array<std::string>	table[4];	// [0] 동, [1] 층, [2] 부재명, [3] 면적
	bool	bFound;
	char	areaValueStr[16];
	char	codeStr[9][16];


	// 그룹화 일시정지 ON
	suspendGroups(true);

	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount();

	// 보이는 레이어들의 목록 저장하기
	for (xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList[nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// 레이어 이름과 인덱스 저장
	for (xx = 0; xx < nVisibleLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList[xx];
		err = ACAPI_Attribute_Get(&attrib);

		sprintf(fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName[strlen(fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList[xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back(newLayerItem);
	}

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort(layerList.begin(), layerList.end(), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

	// 일시적으로 모든 레이어 숨기기
	for (xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
		}
	}

	result = DGBlankModalDialog(300, 150, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, filenameQuestionHandler, (DGUserData)&inputFilename);

	if (result == DG_CANCEL) {
		DGAlert(DG_INFORMATION, L"알림", L"작업을 취소하였습니다.", "", L"확인", "", "");
		return	NoError;
	}

	if (inputFilename.GetLength() <= 0)
		inputFilename = "notitle";

	madeFilename = inputFilename + L" - 레이어별 테이블폼 면적 (통합).csv";
	strcpy(filename, wcharToChar(madeFilename.ToUStr().Get()));
	fp_unite = fopen(filename, "w+");

	if (fp_unite == NULL) {
		DGAlert(DG_ERROR, L"오류", L"통합 버전 엑셀파일을 만들 수 없습니다.", "", L"확인", "", "");
		return	NoError;
	}

	action_mode = DGAlert(DG_INFORMATION, L"파일 출력 모드", L"레이어 이름을 구분해서 출력하시겠습니까?", "", L"예-느림", L"아니오-빠름", "");

	// 진행 상황 표시하는 기능 - 초기화
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface(APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &total);

	totalAreaAll = 0.0;
	sprintf(buffer, "안내: 면적 값의 단위는 m2(제곱미터)입니다.\n고려되는 객체: 유로폼 / 합판 / 아웃코너판넬 / 인코너판넬 / 변각인코너판넬 / 인코너M형판넬 / 목재 / 매직바(합판) / 매직아웃코너(합판) / 매직인코너(합판) / 모프\n\n");
	fprintf(fp_unite, buffer);

	// 헤더 출력
	if (action_mode == 1) {
		sprintf(buffer, "공사코드,동,층,타설번호,CJ,시공순서,부재,제작처,제작번호,면적,비고\n");
		fprintf(fp_unite, buffer);
	}

	// 보이는 레이어들을 하나씩 순회하면서 전체 요소들을 선택한 후 테이블폼의 면적 값을 가진 객체들의 변수 값을 가져와서 계산함
	for (mm = 1; mm <= nVisibleLayers; ++mm) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList[mm - 1].layerInd;
		err = ACAPI_Attribute_Get(&attrib);

		// 초기화
		objects.Clear();
		morphs.Clear();

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify(&attrib, NULL);
			}

			// 모든 요소 가져오기
			ACAPI_Element_GetElemList(API_ObjectID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 객체 타입만
			while (elemList.GetSize() > 0) {
				objects.Push(elemList.Pop());
			}
			nObjects = objects.GetSize();

			ACAPI_Element_GetElemList(API_MorphID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 모프 타입만
			while (elemList.GetSize() > 0) {
				morphs.Push(elemList.Pop());
			}
			nMorphs = morphs.GetSize();

			if (nObjects == 0 && nMorphs == 0)
				continue;

			// 레이어 이름 가져옴
			sprintf(fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName[strlen(fullLayerName)] = '\0';

			// 상세 모드 (느림)
			if (action_mode == 1) {
				if (verifyLayerName(fullLayerName) == true) {
					for (yy = 0; yy < 9; yy++)
						strcpy(codeStr[yy], getLayerCode(fullLayerName, yy + 1));
					sprintf(buffer, "%s,%s,%s,%s,%s,%s,%s,%s,%s,", codeStr[0], codeStr[1], codeStr[2], codeStr[3], codeStr[4], codeStr[5], codeStr[6], codeStr[7], codeStr[8]);
					fprintf(fp_unite, buffer);
				}
				else {
					sprintf(buffer, "레이어: %s,,,,,,,,,", wcharToChar(GS::UniString(fullLayerName).ToUStr().Get()));
					fprintf(fp_unite, buffer);
				}
			}
			// 고속 모드 (빠름)
			else {
				sprintf(buffer, "레이어: %s,,,,,,,,,", wcharToChar(GS::UniString(fullLayerName).ToUStr().Get()));
				fprintf(fp_unite, buffer);
			}

			totalArea = 0.0;

			// 객체에서 면적 값 가져와서 합산하기
			for (xx = 0; xx < nObjects; ++xx) {
				BNZeroMemory(&elem, sizeof(API_Element));
				BNZeroMemory(&memo, sizeof(API_ElementMemo));
				elem.header.guid = objects[xx];
				err = ACAPI_Element_Get(&elem);

				if (err == NoError && elem.header.hasMemo) {
					err = ACAPI_Element_GetMemo(elem.header.guid, &memo);

					// 파라미터 스크립트를 강제로 실행시킴
					ACAPI_Goodies(APIAny_RunGDLParScriptID, &elem.header, 0);
					bool	bForce = true;
					ACAPI_Database(APIDb_RefreshElementID, &elem.header, &bForce);

					if (err == NoError) {
						extractedValue = 0.0;
						bFound = false;

						if (my_strcmp(getParameterStringByName(&memo, "u_comp"), "유로폼") == 0) {
							sprintf(buffer, "%s", getParameterStringByName(&memo, "gs_list_custom04"));
							removeCharInStr(buffer, ',');
							extractedValue = atof(buffer);

							bFound = true;
						}
						else if (my_strcmp(getParameterStringByName(&memo, "g_comp"), "합판") == 0) {
							sprintf(buffer, "%s", getParameterStringByName(&memo, "gs_list_custom04"));
							removeCharInStr(buffer, ',');
							extractedValue = atof(buffer);

							bFound = true;
						}
						else if (my_strcmp(getParameterStringByName(&memo, "in_comp"), "아웃코너판넬") == 0) {
							sprintf(buffer, "%s", getParameterStringByName(&memo, "gs_list_custom04"));
							removeCharInStr(buffer, ',');
							extractedValue = atof(buffer);

							bFound = true;
						}
						else if (my_strcmp(getParameterStringByName(&memo, "in_comp"), "인코너판넬") == 0) {
							sprintf(buffer, "%s", getParameterStringByName(&memo, "gs_list_custom04"));
							removeCharInStr(buffer, ',');
							extractedValue = atof(buffer);

							bFound = true;
						}
						else if (my_strcmp(getParameterStringByName(&memo, "in_comp"), "변각인코너판넬") == 0) {
							sprintf(buffer, "%s", getParameterStringByName(&memo, "gs_list_custom04"));
							removeCharInStr(buffer, ',');
							extractedValue = atof(buffer);

							bFound = true;
						}
						else if (my_strcmp(getParameterStringByName(&memo, "in_comp"), "인코너M형판넬") == 0) {
							sprintf(buffer, "%s", getParameterStringByName(&memo, "gs_list_custom04"));
							removeCharInStr(buffer, ',');
							extractedValue = atof(buffer);

							bFound = true;
						}
						else if (my_strcmp(getParameterStringByName(&memo, "w_comp"), "목재") == 0) {
							sprintf(buffer, "%s", getParameterStringByName(&memo, "gs_list_custom04"));
							removeCharInStr(buffer, ',');
							extractedValue = atof(buffer);

							bFound = true;
						}
						else if (my_strcmp(getParameterStringByName(&memo, "sup_type"), "매직바") == 0) {
							extractedValue = getParameterValueByName(&memo, "plywoodArea");

							bFound = true;
						}
						else if (my_strcmp(getParameterStringByName(&memo, "sup_type"), "매직아웃코너") == 0) {
							extractedValue = getParameterValueByName(&memo, "plywoodArea");

							bFound = true;
						}
						else if (my_strcmp(getParameterStringByName(&memo, "sup_type"), "매직인코너") == 0) {
							extractedValue = getParameterValueByName(&memo, "plywoodArea");

							bFound = true;
						}

						if (bFound == true) {
							totalArea += extractedValue;
							totalAreaAll += extractedValue;

							if (action_mode == 1) {
								if (verifyLayerName(fullLayerName) == true) {
									// 레이어 체계에 속한 레이어의 경우, 표로 정리해서 면적 값을 표시할 것
									// 기존 필드를 검색해보고 존재할 경우 면적을 합산함
									for (yy = 0; yy < table[0].GetSize(); yy++) {
										if ((my_strcmp(table[0][yy].c_str(), getLayerCode(fullLayerName, 2)) == 0) &&
											(my_strcmp(table[1][yy].c_str(), getLayerCode(fullLayerName, 3)) == 0) &&
											(my_strcmp(table[2][yy].c_str(), getLayerCode(fullLayerName, 7)) == 0)) {
											sprintf(areaValueStr, "%.2f", atof(table[3][yy].c_str()) + extractedValue);
											table[3][yy] = areaValueStr;
											break;
										}
									}

									// 기존 필드가 없으면 새로운 필드 생성
									if (yy == table[0].GetSize()) {
										table[0].Push(getLayerCode(fullLayerName, 2));
										table[1].Push(getLayerCode(fullLayerName, 3));
										table[2].Push(getLayerCode(fullLayerName, 7));

										sprintf(areaValueStr, "%.2f", extractedValue);
										table[3].Push(areaValueStr);
									}
								}
							}
						}
					}

					ACAPI_DisposeElemMemoHdls(&memo);
				}
			}

			// 모프에서 면적 값 가져와서 합산하기
			for (xx = 0; xx < nMorphs; ++xx) {
				extractedValue = 0.0;
				bFound = false;

				ACAPI_ELEMENT_QUANTITY_MASK_CLEAR(mask);
				ACAPI_ELEMENT_QUANTITY_MASK_SET(mask, morph, surface);
				quantities.elements = &quantity;

				if (ACAPI_Element_GetQuantities(morphs[xx], &params, &quantities, &mask) == NoError) {
					extractedValue = quantity.morph.surface;

					bFound = true;
				}

				if (bFound == true) {
					totalArea += extractedValue;
					totalAreaAll += extractedValue;

					if (action_mode == 1) {
						if (verifyLayerName(fullLayerName) == true) {
							// 레이어 체계에 속한 레이어의 경우, 표로 정리해서 면적 값을 표시할 것
							// 기존 필드를 검색해보고 존재할 경우 면적을 합산함
							for (yy = 0; yy < table[0].GetSize(); yy++) {
								if ((my_strcmp(table[0][yy].c_str(), getLayerCode(fullLayerName, 2)) == 0) &&
									(my_strcmp(table[1][yy].c_str(), getLayerCode(fullLayerName, 3)) == 0) &&
									(my_strcmp(table[2][yy].c_str(), getLayerCode(fullLayerName, 7)) == 0)) {
									sprintf(areaValueStr, "%.2f", atof(table[3][yy].c_str()) + extractedValue);
									table[3][yy] = areaValueStr;
									break;
								}
							}

							// 기존 필드가 없으면 새로운 필드 생성
							if (yy == table[0].GetSize()) {
								table[0].Push(getLayerCode(fullLayerName, 2));
								table[1].Push(getLayerCode(fullLayerName, 3));
								table[2].Push(getLayerCode(fullLayerName, 7));

								sprintf(areaValueStr, "%.2f", extractedValue);
								table[3].Push(areaValueStr);
							}
						}
					}
				}
			}

			// 면적 값 출력하기
			sprintf(buffer, "%.2f,", totalArea);
			fprintf(fp_unite, buffer);
			
			sprintf(buffer, "\n", totalArea);
			fprintf(fp_unite, buffer);

			// 레이어 숨기기
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
		}

		// 진행 상황 표시하는 기능 - 진행
		cur = mm;
		ACAPI_Interface(APIIo_SetProcessValueID, &cur, NULL);
		if (ACAPI_Interface(APIIo_IsProcessCanceledID, NULL, NULL) == APIERR_CANCEL)
			break;
	}

	// 모든 레이어의 면적 값을 합산한 값도 표시함
	sprintf(buffer, "\n모든 면적 값 합산값: %.2f\n", totalAreaAll);
	fprintf(fp_unite, buffer);

	// 표 내용 출력하기
	if (action_mode == 1) {
		sprintf(buffer, "\n동,층,부재명,면적");
		fprintf(fp_unite, buffer);
		for (yy = 0; yy < table[0].GetSize(); yy++) {
			sprintf(buffer, "\n%s,%s,%s,%s", table[0][yy].c_str(), table[1][yy].c_str(), table[2][yy].c_str(), table[3][yy].c_str());
			fprintf(fp_unite, buffer);
		}
	}

	// 진행 상황 표시하는 기능 - 마무리
	ACAPI_Interface(APIIo_CloseProcessWindowID, NULL, NULL);

	char path[1024];
	char* ptr;
	GetFullPathName(filename, 1024, path, &ptr);

	// 마지막 디렉토리 구분자의 위치를 찾음
	ptr = strrstr(path, "\\");
	if (ptr == NULL) {
		ptr = strrstr(path, "/");
		if (ptr == NULL) {
			// 디렉토리 구분자가 없음
			return 1;
		}
	}

	// 마지막 디렉토리 구분자 이후의 문자열을 제거함
	*ptr = '\0';

	GS::UniString infoStr = L"다음 경로에 파일이 저장되었습니다.\n" + GS::UniString(charToWchar(path));

	// 파일 닫기
	fclose(fp_unite);

	// 모든 프로세스를 마치면 처음에 수집했던 보이는 레이어들을 다시 켜놓을 것
	for (xx = 1; xx <= nVisibleLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList[xx - 1];
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify(&attrib, NULL);
			}
		}
	}

	// 그룹화 일시정지 OFF
	suspendGroups(false);

	DGAlert(DG_INFORMATION, L"알림", infoStr, "", L"확인", "", "");

	return err;
}

// 선택한 물량합판 면적 보여주기
GSErrCode	exportSelectedQuantityPlywoodArea(void)
{
	GSErrCode	err = NoError;

	GS::Array<API_Guid>		objects;
	long					nObjects = 0;

	API_Element			elem;
	API_ElementMemo		memo;

	// GS::Array 반복자
	GS::Array<API_Guid>::Iterator	iterObj;
	API_Guid	curGuid;

	const char* sup_type;
	const char* m_type;
	const char* area_str;
	
	double area_value;
	double total_area = 0.0;
	double total_area_beam = 0.0;				// 보
	double total_area_column_isolation = 0.0;	// 기둥(독립)
	double total_area_column_in_wall = 0.0;		// 기둥(벽체)
	double total_area_column_circular = 0.0;	// 기둥(원형)
	double total_area_wall_inside = 0.0;		// 벽체(내벽)
	double total_area_wall_outside = 0.0;		// 벽체(외벽)
	double total_area_wall_basement = 0.0;		// 벽체(합벽)
	double total_area_wall_parapet = 0.0;		// 벽체(파라펫)
	double total_area_wall_water_block = 0.0;	// 벽체(방수턱)
	double total_area_slab_basement = 0.0;		// 스라브(기초)
	double total_area_slab_rc = 0.0;			// 스라브(RC)
	double total_area_slab_deck = 0.0;			// 스라브(데크)
	double total_area_slab_ramp = 0.0;			// 스라브(램프)
	double total_area_stair = 0.0;				// 계단

	bool bBeam = false;
	bool bColumnIsolation = false;
	bool bColumnInWall = false;
	bool bColumnCircular = false;
	bool bWallInside = false;
	bool bWallOutside = false;
	bool bWallBasement = false;
	bool bWallParapet = false;
	bool bWallWaterBlock = false;
	bool bSlabBasement = false;
	bool bSlabRC = false;
	bool bSlabDeck = false;
	bool bSlabRamp = false;
	bool bStair = false;


	// 그룹화 일시정지 ON
	suspendGroups(true);

	// 선택한 요소 가져오기
	if (getGuidsOfSelection(&objects, API_ObjectID, &nObjects) != NoError) {
		DGAlert(DG_ERROR, L"오류", L"요소들을 선택해야 합니다.", "", L"확인", "", "");
		return err;
	}

	iterObj = objects.Enumerate();

	while (iterObj != NULL) {
		BNZeroMemory(&elem, sizeof(API_Element));
		BNZeroMemory(&memo, sizeof(API_ElementMemo));
		curGuid = *iterObj;
		elem.header.guid = curGuid;
		err = ACAPI_Element_Get(&elem);

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo(elem.header.guid, &memo);

			if (err == NoError) {
				try {
					sup_type = getParameterStringByName(&memo, "sup_type");

					if (my_strcmp(sup_type, "물량합판") == 0) {
						area_str = getParameterStringByName(&memo, "gs_list_custom4");
						area_value = atof(area_str);

						m_type = getParameterStringByName(&memo, "m_type");

						if (my_strcmp(m_type, "보") == 0) {
							total_area_beam += area_value;
							bBeam = true;
						}
						else if (my_strcmp(m_type, "기둥(독립)") == 0) {
							total_area_column_isolation += area_value;
							bColumnIsolation = true;
						}
						else if (my_strcmp(m_type, "기둥(벽체)") == 0) {
							total_area_column_in_wall += area_value;
							bColumnInWall = true;
						}
						else if (my_strcmp(m_type, "기둥(원형)") == 0) {
							total_area_column_circular += area_value;
							bColumnCircular = true;
						}
						else if (my_strcmp(m_type, "벽체(내벽)") == 0) {
							total_area_wall_inside += area_value;
							bWallInside = true;
						}
						else if (my_strcmp(m_type, "벽체(외벽)") == 0) {
							total_area_wall_outside += area_value;
							bWallOutside = true;
						}
						else if (my_strcmp(m_type, "벽체(합벽)") == 0) {
							total_area_wall_basement += area_value;
							bWallBasement = true;
						}
						else if (my_strcmp(m_type, "벽체(파라펫)") == 0) {
							total_area_wall_parapet += area_value;
							bWallParapet = true;
						}
						else if (my_strcmp(m_type, "벽체(방수턱)") == 0) {
							total_area_wall_water_block += area_value;
							bWallWaterBlock = true;
						}
						else if (my_strcmp(m_type, "스라브(기초)") == 0) {
							total_area_slab_basement += area_value;
							bSlabBasement = true;
						}
						else if (my_strcmp(m_type, "스라브(RC)") == 0) {
							total_area_slab_rc += area_value;
							bSlabRC = true;
						}
						else if (my_strcmp(m_type, "스라브(데크)") == 0) {
							total_area_slab_deck += area_value;
							bSlabDeck = true;
						}
						else if (my_strcmp(m_type, "스라브(램프)") == 0) {
							total_area_slab_ramp += area_value;
							bSlabRamp = true;
						}
						else if (my_strcmp(m_type, "계단") == 0) {
							total_area_stair += area_value;
							bStair = true;
						}

						total_area += area_value;
					}
				}
				catch (exception& ex) {
					WriteReport_Alert("객체 정보 수집에서 오류 발생: %s", ex.what());
				}
			}

			ACAPI_DisposeElemMemoHdls(&memo);
		}

		++iterObj;
	}

	char totalArea[128];

	sprintf(totalArea, "%.3f", total_area);
	GS::UniString totalAreaStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_beam);
	GS::UniString totalAreaBeamStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_column_isolation);
	GS::UniString totalAreaColumnIsolationStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_column_in_wall);
	GS::UniString totalAreaColumnInWallStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_column_circular);
	GS::UniString totalAreaColumnCircularStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_wall_inside);
	GS::UniString totalAreaWallInsideStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_wall_outside);
	GS::UniString totalAreaWallOutsideStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_wall_basement);
	GS::UniString totalAreaWallBasementStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_wall_parapet);
	GS::UniString totalAreaWallParapetStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_wall_water_block);
	GS::UniString totalAreaWallWaterBlockStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_slab_basement);
	GS::UniString totalAreaSlabBasementStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_slab_rc);
	GS::UniString totalAreaSlabRCStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_slab_deck);
	GS::UniString totalAreaSlabDeckStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_slab_ramp);
	GS::UniString totalAreaSlabRampStr = charToWchar(totalArea);
	sprintf(totalArea, "%.3f", total_area_stair);
	GS::UniString totalAreaStairStr = charToWchar(totalArea);

	GS::UniString infoStr = L"물량합판 총 면적은 " + totalAreaStr + L"㎡ 입니다." + "\n";
	if (bBeam == true)
		infoStr += L"보: " + totalAreaBeamStr + "\n";
	if (bColumnIsolation == true)
		infoStr += L"기둥(독립): " + totalAreaColumnIsolationStr + "\n";
	if (bColumnInWall == true)
		infoStr += L"기둥(벽체): " + totalAreaColumnInWallStr + "\n";
	if (bColumnCircular == true)
		infoStr += L"기둥(원형): " + totalAreaColumnCircularStr + "\n";
	if (bWallInside == true)
		infoStr += L"벽체(내벽): " + totalAreaWallInsideStr + "\n";
	if (bWallOutside == true)
		infoStr += L"벽체(외벽): " + totalAreaWallOutsideStr + "\n";
	if (bWallBasement == true)
		infoStr += L"벽체(합벽): " + totalAreaWallBasementStr + "\n";
	if (bWallParapet == true)
		infoStr += L"벽체(파라펫): " + totalAreaWallParapetStr + "\n";
	if (bWallWaterBlock == true)
		infoStr += L"벽체(방수턱): " + totalAreaWallWaterBlockStr + "\n";
	if (bSlabBasement == true)
		infoStr += L"슬래브(기초): " + totalAreaSlabBasementStr + "\n";
	if (bSlabRC == true)
		infoStr += L"슬래브(RC): " + totalAreaSlabRCStr + "\n";
	if (bSlabDeck == true)
		infoStr += L"슬래브(데크): " + totalAreaSlabDeckStr + "\n";
	if (bSlabRamp == true)
		infoStr += L"슬래브(램프): " + totalAreaSlabRampStr + "\n";
	if (bStair == true)
		infoStr += L"계단: " + totalAreaStairStr;
	DGAlert(DG_INFORMATION, L"물량합판 면적 합계", infoStr, "", L"확인", "", "");

	return err;
}

// 물량합판 면적 내보내기 (켜져 있는 레이어 전부)
GSErrCode	exportQuantityPlywoodAreaOnVisibleLayers(void)
{
	GSErrCode	err = NoError;
	short		result;

	GS::Array<API_Guid>		objects;
	long					nObjects = 0;

	API_Element			elem;
	API_ElementMemo		memo;

	const char* sup_type;
	const char* area_str;
	const char* m_type;
	
	double area_value;
	double total_area = 0.0;
	double total_area_beam = 0.0;				// 보
	double total_area_column_isolation = 0.0;	// 기둥(독립)
	double total_area_column_in_wall = 0.0;		// 기둥(벽체)
	double total_area_column_circular = 0.0;	// 기둥(원형)
	double total_area_wall_inside = 0.0;		// 벽체(내벽)
	double total_area_wall_outside = 0.0;		// 벽체(외벽)
	double total_area_wall_basement = 0.0;		// 벽체(합벽)
	double total_area_wall_parapet = 0.0;		// 벽체(파라펫)
	double total_area_wall_water_block = 0.0;	// 벽체(방수턱)
	double total_area_slab_basement = 0.0;		// 스라브(기초)
	double total_area_slab_rc = 0.0;			// 스라브(RC)
	double total_area_slab_deck = 0.0;			// 스라브(데크)
	double total_area_slab_ramp = 0.0;			// 스라브(램프)
	double total_area_stair = 0.0;				// 계단

	bool bBeam = false;
	bool bColumnIsolation = false;
	bool bColumnInWall = false;
	bool bColumnCircular = false;
	bool bWallInside = false;
	bool bWallOutside = false;
	bool bWallBasement = false;
	bool bWallParapet = false;
	bool bWallWaterBlock = false;
	bool bSlabBasement = false;
	bool bSlabRC = false;
	bool bSlabDeck = false;
	bool bSlabRamp = false;
	bool bStair = false;

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	API_AttributeDef  defs;
	short			nVisibleLayers = 0;
	short			visLayerList[1024];
	char			fullLayerName[512];
	vector<LayerList>	layerList;

	// 기타
	char			buffer[1024];
	char			filename[512];
	GS::UniString	inputFilename;
	GS::UniString	madeFilename;

	char			buffer_01[32];
	char			buffer_02[32];
	char			buffer_03[32];
	char			buffer_04[32];
	char			buffer_05[32];
	char			buffer_06[32];
	char			buffer_07[32];
	char			buffer_08[32];
	char			buffer_09[32];
	char			buffer_10[32];
	char			buffer_11[32];
	char			buffer_12[32];
	char			buffer_13[32];
	char			buffer_14[32];

	// 진행바를 표현하기 위한 변수
	GS::UniString       title(L"내보내기 진행 상황");
	GS::UniString       subtitle(L"진행중...");
	short	nPhase;
	Int32	cur, total;

	// 파일 저장을 위한 변수
	FILE* fp;
	FILE* fp_unite;


	// 그룹화 일시정지 ON
	suspendGroups(true);

	result = DGBlankModalDialog(300, 150, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, filenameQuestionHandler, (DGUserData)&inputFilename);

	if (result == DG_CANCEL) {
		DGAlert(DG_INFORMATION, L"알림", L"작업을 취소하였습니다.", "", L"확인", "", "");
		return	NoError;
	}

	if (inputFilename.GetLength() <= 0)
		inputFilename = "notitle";

	madeFilename = inputFilename + L" (통합).csv";
	strcpy(filename, wcharToChar(madeFilename.ToUStr().Get()));

	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount();

	// 보이는 레이어들의 목록 저장하기
	for (short xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList[nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// 레이어 이름과 인덱스 저장
	for (short xx = 0; xx < nVisibleLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList[xx];
		err = ACAPI_Attribute_Get(&attrib);

		sprintf(fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName[strlen(fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList[xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back(newLayerItem);
	}

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort(layerList.begin(), layerList.end(), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

	// 일시적으로 모든 레이어 숨기기
	for (short xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
		}
	}

	fp_unite = fopen(filename, "w+");

	if (fp_unite == NULL) {
		DGAlert(DG_ERROR, L"오류", L"통합 버전 엑셀파일을 만들 수 없습니다.", "", L"확인", "", "");
		return	NoError;
	}

	// 진행 상황 표시하는 기능 - 초기화
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface(APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &total);

	// 보이는 레이어들을 하나씩 순회
	for (short mm = 1; mm <= nVisibleLayers; ++mm) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = layerList[mm - 1].layerInd;
		err = ACAPI_Attribute_Get(&attrib);

		// 초기화
		objects.Clear();
		nObjects = 0;

		total_area = 0.0;
		total_area_beam = 0.0;				// 보
		total_area_column_isolation = 0.0;	// 기둥(독립)
		total_area_column_in_wall = 0.0;	// 기둥(벽체)
		total_area_column_circular = 0.0;	// 기둥(원형)
		total_area_wall_inside = 0.0;		// 벽체(내벽)
		total_area_wall_outside = 0.0;		// 벽체(외벽)
		total_area_wall_basement = 0.0;		// 벽체(합벽)
		total_area_wall_parapet = 0.0;		// 벽체(파라펫)
		total_area_wall_water_block = 0.0;	// 벽체(방수턱)
		total_area_slab_basement = 0.0;		// 스라브(기초)
		total_area_slab_rc = 0.0;			// 스라브(RC)
		total_area_slab_deck = 0.0;			// 스라브(데크)
		total_area_slab_ramp = 0.0;			// 스라브(램프)
		total_area_stair = 0.0;				// 계단

		bBeam = false;
		bColumnIsolation = false;
		bColumnInWall = false;
		bColumnCircular = false;
		bWallInside = false;
		bWallOutside = false;
		bWallBasement = false;
		bWallParapet = false;
		bWallWaterBlock = false;
		bSlabBasement = false;
		bSlabRC = false;
		bSlabDeck = false;
		bSlabRamp = false;
		bStair = false;

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify(&attrib, NULL);
			}

			// 모든 요소 가져오기
			ACAPI_Element_GetElemList(API_ObjectID, &objects, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 객체 타입만
			nObjects = objects.GetSize();

			if (nObjects == 0)
				continue;
			
			// 레이어 이름 가져옴
			sprintf(fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName[strlen(fullLayerName)] = '\0';

			madeFilename = inputFilename + L" - " + fullLayerName + L".csv";
			strcpy(filename, wcharToChar(madeFilename.ToUStr().Get()));
			fp = fopen(filename, "w+");

			if (fp == NULL) {
				GS::UniString warningStr = L"레이어 " + GS::UniString(fullLayerName) + "은(는) 파일명이 될 수 없으므로 생략합니다.";
				DGAlert(DG_ERROR, L"오류", warningStr, "", L"확인", "", "");
				continue;
			}

			// 레이어 이름
			sprintf(buffer, "\n\n<< 레이어 : %s >>\n", wcharToChar(GS::UniString(fullLayerName).ToUStr().Get()));
			fprintf(fp, buffer);
			fprintf(fp_unite, buffer);

			sprintf(buffer, "%s\n", getExplanationOfLayerCode(fullLayerName));
			fprintf(fp, buffer);
			fprintf(fp_unite, buffer);

			for (short xx = 0; xx < nObjects; ++xx) {
				BNZeroMemory(&elem, sizeof(API_Element));
				BNZeroMemory(&memo, sizeof(API_ElementMemo));
				elem.header.guid = objects.Pop();
				err = ACAPI_Element_Get(&elem);

				if (err == NoError && elem.header.hasMemo) {
					err = ACAPI_Element_GetMemo(elem.header.guid, &memo);

					if (err == NoError) {
						try {
							sup_type = getParameterStringByName(&memo, "sup_type");

							if (my_strcmp(sup_type, "물량합판") == 0) {
								area_str = getParameterStringByName(&memo, "gs_list_custom4");
								area_value = atof(area_str);

								m_type = getParameterStringByName(&memo, "m_type");

								if (my_strcmp(m_type, "보") == 0)
									total_area_beam += area_value;
								else if (my_strcmp(m_type, "기둥(독립)") == 0)
									total_area_column_isolation += area_value;
								else if (my_strcmp(m_type, "기둥(벽체)") == 0)
									total_area_column_in_wall += area_value;
								else if (my_strcmp(m_type, "기둥(원형)") == 0)
									total_area_column_circular += area_value;
								else if (my_strcmp(m_type, "벽체(내벽)") == 0)
									total_area_wall_inside += area_value;
								else if (my_strcmp(m_type, "벽체(외벽)") == 0)
									total_area_wall_outside += area_value;
								else if (my_strcmp(m_type, "벽체(합벽)") == 0)
									total_area_wall_basement += area_value;
								else if (my_strcmp(m_type, "벽체(파라펫)") == 0)
									total_area_wall_parapet += area_value;
								else if (my_strcmp(m_type, "벽체(방수턱)") == 0)
									total_area_wall_water_block += area_value;
								else if (my_strcmp(m_type, "스라브(기초)") == 0)
									total_area_slab_basement += area_value;
								else if (my_strcmp(m_type, "스라브(RC)") == 0)
									total_area_slab_rc += area_value;
								else if (my_strcmp(m_type, "스라브(데크)") == 0)
									total_area_slab_deck += area_value;
								else if (my_strcmp(m_type, "스라브(램프)") == 0)
									total_area_slab_ramp += area_value;
								else if (my_strcmp(m_type, "계단") == 0)
									total_area_stair += area_value;

								total_area += area_value;
							}
						}
						catch (exception& ex) {
							WriteReport_Alert("출력 함수에서 오류 발생: %s", ex.what());
						}
					}

					ACAPI_DisposeElemMemoHdls(&memo);
				}
			}

			sprintf(buffer, "물량합판 총 면적(㎡): %.3f\n", total_area);

			if (total_area_beam > EPS) {
				sprintf(buffer_01, "보: %.3f\n", total_area_beam);
				strcat(buffer, buffer_01);
			}
			if (total_area_column_isolation > EPS) {
				sprintf(buffer_02, "기둥(독립): %.3f\n", total_area_column_isolation);
				strcat(buffer, buffer_02);
			}
			if (total_area_column_in_wall > EPS) {
				sprintf(buffer_03, "기둥(벽체): %.3f\n", total_area_column_in_wall);
				strcat(buffer, buffer_03);
			}
			if (total_area_column_circular > EPS) {
				sprintf(buffer_04, "기둥(원형): %.3f\n", total_area_column_circular);
				strcat(buffer, buffer_04);
			}
			if (total_area_wall_inside > EPS) {
				sprintf(buffer_05, "벽체(내벽): %.3f\n", total_area_wall_inside);
				strcat(buffer, buffer_05);
			}
			if (total_area_wall_outside > EPS) {
				sprintf(buffer_06, "벽체(외벽): %.3f\n", total_area_wall_outside);
				strcat(buffer, buffer_06);
			}
			if (total_area_wall_basement > EPS) {
				sprintf(buffer_07, "벽체(합벽): %.3f\n", total_area_wall_basement);
				strcat(buffer, buffer_07);
			}
			if (total_area_wall_parapet > EPS) {
				sprintf(buffer_08, "벽체(파라펫): %.3f\n", total_area_wall_parapet);
				strcat(buffer, buffer_08);
			}
			if (total_area_wall_water_block > EPS) {
				sprintf(buffer_09, "벽체(방수턱): %.3f\n", total_area_wall_water_block);
				strcat(buffer, buffer_09);
			}
			if (total_area_slab_basement > EPS) {
				sprintf(buffer_10, "슬래브(기초): %.3f\n", total_area_slab_basement);
				strcat(buffer, buffer_10);
			}
			if (total_area_slab_rc > EPS) {
				sprintf(buffer_11, "슬래브(RC): %.3f\n", total_area_slab_rc);
				strcat(buffer, buffer_11);
			}
			if (total_area_slab_deck > EPS) {
				sprintf(buffer_12, "슬래브(데크): %.3f\n", total_area_slab_deck);
				strcat(buffer, buffer_12);
			}
			if (total_area_slab_ramp > EPS) {
				sprintf(buffer_13, "슬래브(램프): %.3f\n", total_area_slab_ramp);
				strcat(buffer, buffer_13);
			}
			if (total_area_stair > EPS) {
				sprintf(buffer_14, "계단: %.3f\n", total_area_stair);
				strcat(buffer, buffer_14);
			}

			fprintf(fp, buffer);
			fprintf(fp_unite, buffer);

			fclose(fp);

			// 레이어 숨기기
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
		}

		// 진행 상황 표시하는 기능 - 진행
		cur = mm;
		ACAPI_Interface(APIIo_SetProcessValueID, &cur, NULL);
		if (ACAPI_Interface(APIIo_IsProcessCanceledID, NULL, NULL) == APIERR_CANCEL)
			break;
	}

	// 진행 상황 표시하는 기능 - 마무리
	ACAPI_Interface(APIIo_CloseProcessWindowID, NULL, NULL);

	char path[1024];
	char* ptr;
	GetFullPathName(filename, 1024, path, &ptr);

	// 마지막 디렉토리 구분자의 위치를 찾음
	ptr = strrstr(path, "\\");
	if (ptr == NULL) {
		ptr = strrstr(path, "/");
		if (ptr == NULL) {
			// 디렉토리 구분자가 없음
			return 1;
		}
	}

	// 마지막 디렉토리 구분자 이후의 문자열을 제거함
	*ptr = '\0';

	GS::UniString infoStr = L"다음 경로에 파일이 저장되었습니다.\n" + GS::UniString(charToWchar(path));

	fclose(fp_unite);

	// 모든 프로세스를 마치면 처음에 수집했던 보이는 레이어들을 다시 켜놓을 것
	for (short xx = 1; xx <= nVisibleLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList[xx - 1];
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify(&attrib, NULL);
			}
		}
	}

	// 그룹화 일시정지 OFF
	suspendGroups(false);

	DGAlert(DG_INFORMATION, L"알림", infoStr, "", L"확인", "", "");

	return err;
}

// 모든 입면도 PDF로 내보내기 (현재 보이는 화면에 한해)
GSErrCode	exportAllElevationsToPDFSingleMode(void)
{
	GSErrCode	err = NoError;
	bool		regenerate = true;
	char		filename[256];
	bool		bAsked = false;

	// 입면도 DB를 가져오기 위한 변수
	API_DatabaseUnId* dbases = NULL;
	GSSize				nDbases = 0;
	API_WindowInfo		windowInfo;
	API_DatabaseInfo	currentDB;

	// 파일 내보내기를 위한 변수
	API_FileSavePars	fsp;
	API_SavePars_Pdf	pars_pdf;

	// 입면도 확대를 위한 변수
	API_Box		extent;
	double		scale = 100.0;
	bool		zoom = true;

	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;

	ACAPI_Automate(APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate(APIDo_RebuildID, &regenerate, NULL);

	// PDF 파일 페이지 설정
	BNZeroMemory(&pars_pdf, sizeof(API_SavePars_Pdf));
	pars_pdf.leftMargin = 5.0;
	pars_pdf.rightMargin = 5.0;
	pars_pdf.topMargin = 5.0;
	pars_pdf.bottomMargin = 5.0;
	pars_pdf.sizeX = 297.0;
	pars_pdf.sizeY = 210.0;

	// 입면 뷰 DB의 ID들을 획득함
	err = ACAPI_Database(APIDb_GetElevationDatabasesID, &dbases, NULL);
	if (err == NoError)
		nDbases = BMpGetSize(reinterpret_cast<GSPtr>(dbases)) / Sizeof32(API_DatabaseUnId);

	// 입면 뷰들을 하나씩 순회함
	for (GSIndex i = 0; i < nDbases; i++) {
		API_DatabaseInfo dbPars;
		BNZeroMemory(&dbPars, sizeof(API_DatabaseInfo));
		dbPars.databaseUnId = dbases[i];

		// 창을 변경함
		BNZeroMemory(&windowInfo, sizeof(API_WindowInfo));
		windowInfo.typeID = APIWind_ElevationID;
		windowInfo.databaseUnId = dbPars.databaseUnId;
		ACAPI_Automate(APIDo_ChangeWindowID, &windowInfo, NULL);

		// 현재 데이터베이스를 가져옴
		ACAPI_Database(APIDb_GetCurrentDatabaseID, &currentDB, NULL);

		// 현재 도면의 드로잉 범위를 가져옴
		ACAPI_Database(APIDb_GetExtentID, &extent, NULL);

		// 축척 변경하기
		if (bAsked == false) {
			scale = (abs(extent.xMax - extent.xMin) < abs(extent.yMax - extent.yMin)) ? abs(extent.xMax - extent.xMin) : abs(extent.yMax - extent.yMin);
			scale *= 2;
			DGBlankModalDialog(300, 150, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, scaleQuestionHandler, (DGUserData)&scale);
			bAsked = true;
		}
		ACAPI_Database(APIDb_ChangeDrawingScaleID, &scale, &zoom);

		// 저장하기
		BNZeroMemory(&fsp, sizeof(API_FileSavePars));
		fsp.fileTypeID = APIFType_PdfFile;
		ACAPI_Environment(APIEnv_GetSpecFolderID, &specFolderID, &location);
		sprintf(filename, "%s.pdf", GS::UniString(currentDB.title).ToCStr().Get());
		fsp.file = new IO::Location(location, IO::Name(filename));

		err = ACAPI_Automate(APIDo_SaveID, &fsp, &pars_pdf);

		delete	fsp.file;
	}

	if (dbases != NULL)
		BMpFree(reinterpret_cast<GSPtr>(dbases));

	ACAPI_Environment(APIEnv_GetSpecFolderID, &specFolderID, &location);
	resultString = location.ToDisplayText();
	DGAlert(DG_INFORMATION, L"알림", L"결과물을 다음 위치에 저장했습니다.\n" + resultString, "", L"확인", "", "");

	return err;
}

// 모든 입면도 PDF로 내보내기 (켜져 있는 레이어 각각)
GSErrCode	exportAllElevationsToPDFMultiMode(void)
{
	GSErrCode	err = NoError;
	short		xx, mm;
	bool		regenerate = true;
	char		filename[256];
	bool		bAsked = false;

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList[1024];
	char			fullLayerName[512];
	vector<LayerList>	layerList;

	// 진행바를 표현하기 위한 변수
	GS::UniString       title(L"내보내기 진행 상황");
	GS::UniString       subtitle(L"진행중...");
	short	nPhase;
	Int32	cur, total;

	// 입면도 DB를 가져오기 위한 변수
	API_DatabaseUnId* dbases = NULL;
	GSSize				nDbases = 0;
	API_WindowInfo		windowInfo;
	API_DatabaseInfo	currentDB;

	// 파일 내보내기를 위한 변수
	API_FileSavePars	fsp;
	API_SavePars_Pdf	pars_pdf;

	// 입면도 확대를 위한 변수
	API_Box		extent;
	double		scale = 100.0;
	bool		zoom = true;

	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;

	ACAPI_Automate(APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate(APIDo_RebuildID, &regenerate, NULL);

	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount();

	// 보이는 레이어들의 목록 저장하기
	for (xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList[nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// 레이어 이름과 인덱스 저장
	for (xx = 0; xx < nVisibleLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList[xx];
		err = ACAPI_Attribute_Get(&attrib);

		sprintf(fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName[strlen(fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList[xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back(newLayerItem);
	}

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort(layerList.begin(), layerList.end(), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

	// 일시적으로 모든 레이어 숨기기
	for (xx = 1; xx <= nLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
		}
	}

	// 입면 뷰 DB의 ID들을 획득함
	err = ACAPI_Database(APIDb_GetElevationDatabasesID, &dbases, NULL);
	if (err == NoError)
		nDbases = BMpGetSize(reinterpret_cast<GSPtr>(dbases)) / Sizeof32(API_DatabaseUnId);

	// PDF 파일 페이지 설정
	BNZeroMemory(&pars_pdf, sizeof(API_SavePars_Pdf));
	pars_pdf.leftMargin = 5.0;
	pars_pdf.rightMargin = 5.0;
	pars_pdf.topMargin = 5.0;
	pars_pdf.bottomMargin = 5.0;
	pars_pdf.sizeX = 297.0;
	pars_pdf.sizeY = 210.0;

	// 진행 상황 표시하는 기능 - 초기화
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface(APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &total);

	// 보이는 레이어들을 하나씩 순회하면서 전체 요소들을 선택한 후 테이블폼의 면적 값을 가진 객체들의 변수 값을 가져와서 계산함
	for (mm = 1; mm <= nVisibleLayers; ++mm) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList[mm - 1].layerInd;
		err = ACAPI_Attribute_Get(&attrib);

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify(&attrib, NULL);
			}

			// 레이어 이름 가져옴
			sprintf(fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName[strlen(fullLayerName)] = '\0';

			// 입면 뷰들을 하나씩 순회함
			for (GSIndex i = 0; i < nDbases; i++) {
				API_DatabaseInfo dbPars;
				BNZeroMemory(&dbPars, sizeof(API_DatabaseInfo));
				dbPars.databaseUnId = dbases[i];

				// 창을 변경함
				BNZeroMemory(&windowInfo, sizeof(API_WindowInfo));
				windowInfo.typeID = APIWind_ElevationID;
				windowInfo.databaseUnId = dbPars.databaseUnId;
				ACAPI_Automate(APIDo_ChangeWindowID, &windowInfo, NULL);

				// 현재 데이터베이스를 가져옴
				ACAPI_Database(APIDb_GetCurrentDatabaseID, &currentDB, NULL);

				// 현재 도면의 드로잉 범위를 가져옴
				ACAPI_Database(APIDb_GetExtentID, &extent, NULL);

				// 축척 변경하기
				if (bAsked == false) {
					scale = (abs(extent.xMax - extent.xMin) < abs(extent.yMax - extent.yMin)) ? abs(extent.xMax - extent.xMin) : abs(extent.yMax - extent.yMin);
					scale *= 2;
					DGBlankModalDialog(300, 150, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, scaleQuestionHandler, (DGUserData)&scale);
					bAsked = true;
				}
				ACAPI_Database(APIDb_ChangeDrawingScaleID, &scale, &zoom);

				// 저장하기
				BNZeroMemory(&fsp, sizeof(API_FileSavePars));
				fsp.fileTypeID = APIFType_PdfFile;
				ACAPI_Environment(APIEnv_GetSpecFolderID, &specFolderID, &location);
				sprintf(filename, "%s - %s.pdf", fullLayerName, GS::UniString(currentDB.title).ToCStr().Get());
				fsp.file = new IO::Location(location, IO::Name(filename));

				err = ACAPI_Automate(APIDo_SaveID, &fsp, &pars_pdf);

				delete	fsp.file;
			}

			// 레이어 숨기기
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify(&attrib, NULL);
		}

		// 진행 상황 표시하는 기능 - 진행
		cur = mm;
		ACAPI_Interface(APIIo_SetProcessValueID, &cur, NULL);
		if (ACAPI_Interface(APIIo_IsProcessCanceledID, NULL, NULL) == APIERR_CANCEL)
			break;
	}

	// 진행 상황 표시하는 기능 - 마무리
	ACAPI_Interface(APIIo_CloseProcessWindowID, NULL, NULL);

	if (dbases != NULL)
		BMpFree(reinterpret_cast<GSPtr>(dbases));

	// 모든 프로세스를 마치면 처음에 수집했던 보이는 레이어들을 다시 켜놓을 것
	for (xx = 1; xx <= nVisibleLayers; ++xx) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList[xx - 1];
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify(&attrib, NULL);
			}
		}
	}

	ACAPI_Environment(APIEnv_GetSpecFolderID, &specFolderID, &location);
	resultString = location.ToDisplayText();
	DGAlert(DG_INFORMATION, L"알림", L"결과물을 다음 위치에 저장했습니다.\n" + resultString, "", L"확인", "", "");

	return err;
}

// [다이얼로그] 다이얼로그에서 보이는 레이어 상에 있는 객체들의 종류를 보여주고, 체크한 종류의 객체들만 선택 후 보여줌
short DGCALLBACK filterSelectionHandler(short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	xx;
	short	itmIdx;
	short	itmPosX, itmPosY;
	char	buffer[64];

	switch (message) {
	case DG_MSG_INIT:
		// 다이얼로그 타이틀
		DGSetDialogTitle(dialogID, L"선택한 타입의 객체 선택 후 보여주기");

		// 확인 버튼
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 10, 80, 25);
		DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_OK, L"확인");
		DGShowItem(dialogID, DG_OK);

		// 취소 버튼
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 10, 80, 25);
		DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_CANCEL, L"취소");
		DGShowItem(dialogID, DG_CANCEL);

		// 버튼: 전체선택
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 50, 80, 25);
		DGSetItemFont(dialogID, BUTTON_ALL_SEL, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_ALL_SEL, L"전체선택");
		DGShowItem(dialogID, BUTTON_ALL_SEL);

		// 버튼: 전체선택 해제
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 50, 80, 25);
		DGSetItemFont(dialogID, BUTTON_ALL_UNSEL, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, BUTTON_ALL_UNSEL, L"전체선택\n해제");
		DGShowItem(dialogID, BUTTON_ALL_UNSEL);

		// 체크박스: 알려지지 않은 객체 포함
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 220, 50, 250, 25);
		DGSetItemFont(dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT, DG_IS_LARGE | DG_IS_PLAIN);
		sprintf(buffer, "알려지지 않은 객체 포함 (%d)", visibleObjInfo.nUnknownObjects);
		DGSetItemText(dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT, charToWchar(buffer));
		DGShowItem(dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT);
		if (visibleObjInfo.nUnknownObjects > 0)
			DGEnableItem(dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT);
		else
			DGDisableItem(dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT);

		// 구분자
		itmIdx = DGAppendDialogItem(dialogID, DG_ITM_SEPARATOR, 0, 0, 5, 90, 740, 1);
		DGShowItem(dialogID, itmIdx);

		// 체크박스 항목들 배치할 것
		itmPosX = 20;	itmPosY = 105;	// Y의 범위 105 ~ 500까지

		if (visibleObjInfo.bExist_Walls == true) {
			visibleObjInfo.itmIdx_Walls = itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"벽");
			DGShowItem(dialogID, itmIdx);
			itmPosY += 30;
		}
		if (visibleObjInfo.bExist_Columns == true) {
			visibleObjInfo.itmIdx_Columns = itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"기둥");
			DGShowItem(dialogID, itmIdx);
			itmPosY += 30;
		}
		if (visibleObjInfo.bExist_Beams == true) {
			visibleObjInfo.itmIdx_Beams = itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"보");
			DGShowItem(dialogID, itmIdx);
			itmPosY += 30;
		}
		if (visibleObjInfo.bExist_Slabs == true) {
			visibleObjInfo.itmIdx_Slabs = itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"슬래브");
			DGShowItem(dialogID, itmIdx);
			itmPosY += 30;
		}
		if (visibleObjInfo.bExist_Roofs == true) {
			visibleObjInfo.itmIdx_Roofs = itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"루프");
			DGShowItem(dialogID, itmIdx);
			itmPosY += 30;
		}
		if (visibleObjInfo.bExist_Meshes == true) {
			visibleObjInfo.itmIdx_Meshes = itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"메시");
			DGShowItem(dialogID, itmIdx);
			itmPosY += 30;
		}
		if (visibleObjInfo.bExist_Morphs == true) {
			visibleObjInfo.itmIdx_Morphs = itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"모프");
			DGShowItem(dialogID, itmIdx);
			itmPosY += 30;
		}
		if (visibleObjInfo.bExist_Shells == true) {
			visibleObjInfo.itmIdx_Shells = itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
			DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText(dialogID, itmIdx, L"셸");
			DGShowItem(dialogID, itmIdx);
			itmPosY += 30;
		}

		for (xx = 0; xx < visibleObjInfo.nKinds; ++xx) {
			if (visibleObjInfo.bExist[xx] == true) {
				visibleObjInfo.itmIdx[xx] = itmIdx = DGAppendDialogItem(dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont(dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText(dialogID, itmIdx, charToWchar(visibleObjInfo.objName[xx]));
				DGShowItem(dialogID, itmIdx);
				itmPosY += 30;

				// 1행에 12개
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
			// Object 타입
			for (xx = 0; xx < visibleObjInfo.nKinds; ++xx) {
				if (DGGetItemValLong(dialogID, visibleObjInfo.itmIdx[xx]) == TRUE) {
					visibleObjInfo.bShow[xx] = true;
				}
				else {
					visibleObjInfo.bShow[xx] = false;
				}
			}

			// 알려지지 않은 Object 타입의 객체 보이기 여부
			(DGGetItemValLong(dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT) == TRUE) ? visibleObjInfo.bShow_Unknown = true : visibleObjInfo.bShow_Unknown = false;

			// 나머지 타입
			(DGGetItemValLong(dialogID, visibleObjInfo.itmIdx_Walls) == TRUE) ? visibleObjInfo.bShow_Walls = true : visibleObjInfo.bShow_Walls = false;
			(DGGetItemValLong(dialogID, visibleObjInfo.itmIdx_Columns) == TRUE) ? visibleObjInfo.bShow_Columns = true : visibleObjInfo.bShow_Columns = false;
			(DGGetItemValLong(dialogID, visibleObjInfo.itmIdx_Beams) == TRUE) ? visibleObjInfo.bShow_Beams = true : visibleObjInfo.bShow_Beams = false;
			(DGGetItemValLong(dialogID, visibleObjInfo.itmIdx_Slabs) == TRUE) ? visibleObjInfo.bShow_Slabs = true : visibleObjInfo.bShow_Slabs = false;
			(DGGetItemValLong(dialogID, visibleObjInfo.itmIdx_Roofs) == TRUE) ? visibleObjInfo.bShow_Roofs = true : visibleObjInfo.bShow_Roofs = false;
			(DGGetItemValLong(dialogID, visibleObjInfo.itmIdx_Meshes) == TRUE) ? visibleObjInfo.bShow_Meshes = true : visibleObjInfo.bShow_Meshes = false;
			(DGGetItemValLong(dialogID, visibleObjInfo.itmIdx_Morphs) == TRUE) ? visibleObjInfo.bShow_Morphs = true : visibleObjInfo.bShow_Morphs = false;
			(DGGetItemValLong(dialogID, visibleObjInfo.itmIdx_Shells) == TRUE) ? visibleObjInfo.bShow_Shells = true : visibleObjInfo.bShow_Shells = false;

			break;

		case DG_CANCEL:
			break;

		case BUTTON_ALL_SEL:
			item = 0;

			// 모든 체크박스를 켬
			if (visibleObjInfo.bExist_Walls == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Walls, TRUE);
			if (visibleObjInfo.bExist_Columns == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Columns, TRUE);
			if (visibleObjInfo.bExist_Beams == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Beams, TRUE);
			if (visibleObjInfo.bExist_Slabs == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Slabs, TRUE);
			if (visibleObjInfo.bExist_Roofs == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Roofs, TRUE);
			if (visibleObjInfo.bExist_Meshes == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Meshes, TRUE);
			if (visibleObjInfo.bExist_Morphs == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Morphs, TRUE);
			if (visibleObjInfo.bExist_Shells == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Shells, TRUE);
			for (xx = 0; xx < visibleObjInfo.nKinds; ++xx) {
				if (visibleObjInfo.bExist[xx] == true) {
					DGSetItemValLong(dialogID, visibleObjInfo.itmIdx[xx], TRUE);
				}
			}

			break;

		case BUTTON_ALL_UNSEL:
			item = 0;

			// 모든 체크박스를 끔
			if (visibleObjInfo.bExist_Walls == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Walls, FALSE);
			if (visibleObjInfo.bExist_Columns == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Columns, FALSE);
			if (visibleObjInfo.bExist_Beams == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Beams, FALSE);
			if (visibleObjInfo.bExist_Slabs == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Slabs, FALSE);
			if (visibleObjInfo.bExist_Roofs == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Roofs, FALSE);
			if (visibleObjInfo.bExist_Meshes == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Meshes, FALSE);
			if (visibleObjInfo.bExist_Morphs == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Morphs, FALSE);
			if (visibleObjInfo.bExist_Shells == true)	DGSetItemValLong(dialogID, visibleObjInfo.itmIdx_Shells, FALSE);
			for (xx = 0; xx < visibleObjInfo.nKinds; ++xx) {
				if (visibleObjInfo.bExist[xx] == true) {
					DGSetItemValLong(dialogID, visibleObjInfo.itmIdx[xx], FALSE);
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

// [다이얼로그] 사용자가 축척 값을 직접 입력할 수 있도록 함
short DGCALLBACK scaleQuestionHandler(short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	short	result;
	short	idxItem;
	double* scale = (double*)userData;

	switch (message) {
	case DG_MSG_INIT:
		// 다이얼로그 타이틀
		DGSetDialogTitle(dialogID, L"입면도 축척값 입력하기");

		// 적용 버튼
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 70, 110, 70, 25);
		DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_OK, L"예");
		DGShowItem(dialogID, DG_OK);

		// 종료 버튼
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 160, 110, 70, 25);
		DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_CANCEL, L"아니오");
		DGShowItem(dialogID, DG_CANCEL);

		// 라벨: 안내문
		idxItem = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 15, 200, 25);
		DGSetItemFont(dialogID, idxItem, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, idxItem, L"입면도의 축척을 변경하시겠습니까?\n값이 작아질수록 도면이 확대됩니다.");
		DGShowItem(dialogID, idxItem);

		// 라벨: 축척
		idxItem = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 130, 60, 30, 23);
		DGSetItemFont(dialogID, idxItem, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, idxItem, L"1 : ");
		DGShowItem(dialogID, idxItem);

		// Edit 컨트롤: 축척
		EDITCONTROL_SCALE_VALUE = DGAppendDialogItem(dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 161, 54, 60, 25);
		DGSetItemFont(dialogID, EDITCONTROL_SCALE_VALUE, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemValDouble(dialogID, EDITCONTROL_SCALE_VALUE, *scale);
		DGShowItem(dialogID, EDITCONTROL_SCALE_VALUE);

		break;

	case DG_MSG_CLICK:
		switch (item) {
		case DG_OK:
			*scale = DGGetItemValDouble(dialogID, EDITCONTROL_SCALE_VALUE);
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

// [다이얼로그] 파일명을 입력할 수 있도록 함
short DGCALLBACK filenameQuestionHandler(short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	short	result;
	short	idxItem;
	GS::UniString* filename = (GS::UniString*)userData;

	switch (message) {
	case DG_MSG_INIT:
		// 다이얼로그 타이틀
		DGSetDialogTitle(dialogID, L"파일명 입력하기");

		// Yes 버튼
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 70, 110, 70, 25);
		DGSetItemFont(dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_OK, L"예");
		DGShowItem(dialogID, DG_OK);

		// No 버튼
		DGAppendDialogItem(dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 160, 110, 70, 25);
		DGSetItemFont(dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, DG_CANCEL, L"아니오");
		DGShowItem(dialogID, DG_CANCEL);

		// 라벨: 안내문
		idxItem = DGAppendDialogItem(dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 15, 200, 25);
		DGSetItemFont(dialogID, idxItem, DG_IS_LARGE | DG_IS_PLAIN);
		DGSetItemText(dialogID, idxItem, L"내보낼 파일의 이름을 입력하십시오.");
		DGShowItem(dialogID, idxItem);

		// Edit 컨트롤
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
