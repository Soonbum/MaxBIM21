#ifndef	__UTILITY_FUNCTIONS__
#define __UTILITY_FUNCTIONS__

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdarg.h>
#include "MaxBIM21.h"

// 각도 변환
double	DegreeToRad(double degree);		// degree 각도를 radian 각도로 변환
double	RadToDegree(double rad);		// radian 각도를 degree 각도로 변환

// 거리 측정
double	GetDistance(const double begX, const double begY, const double endX, const double endY);											// 2차원에서 2점 간의 거리를 알려줌
double	GetDistance(const API_Coord begPoint, API_Coord endPoint);																			// 2차원에서 2점 간의 거리를 알려줌
double	GetDistance(const double begX, const double begY, const double begZ, const double endX, const double endY, const double endZ);		// 3차원에서 2점 간의 거리를 알려줌
double	GetDistance(const API_Coord3D begPoint, API_Coord3D endPoint);																		// 3차원에서 2점 간의 거리를 알려줌
double	distOfPointBetweenLine(API_Coord p, API_Coord a, API_Coord b);																		// 선분 AB와 점 P와의 거리를 구하는 함수
double	distOfPointBetweenLine(API_Coord3D p, API_Coord a, API_Coord b);																	// 선분 AB와 점 P와의 거리를 구하는 함수
int		getQuadrantOfPoint(API_Coord o, API_Coord a, API_Coord b, double radAng);															// 점 O를 기준으로 회전된 점 A이 있을 때, A로부터 떨어져 있는 점 B가 원래 어느 사분면에 있었는지 알아내는 함수
// 비교하기
long	compareDoubles(const double a, const double b);							// 어떤 수가 더 큰지 비교함
long	compareRanges(double aMin, double aMax, double bMin, double bMax);		// a와 b의 각 값 범위의 관계를 알려줌
int		my_strcmp(const char* str1, const char* str2);							// 문자열 비교

// 교환하기
void	exchangeDoubles(double* a, double* b);		// a와 b 값을 교환함

// 기하 (일반)
long	findDirection(const double begX, const double begY, const double endX, const double endY);					// 시작점에서 끝점으로 향하는 벡터의 방향을 확인
API_Coord	IntersectionPoint1(const API_Coord* p1, const API_Coord* p2, const API_Coord* p3, const API_Coord* p4);	// (p1, p2)를 이은 직선과 (p3, p4)를 이은 직선의 교차점을 구하는 함수
API_Coord	IntersectionPoint2(double m1, double b1, double m2, double b2);											// y = m1*x + b1, y = m2*x + b2 두 직선의 교차점을 구하는 함수
API_Coord	IntersectionPoint3(double a1, double b1, double c1, double a2, double b2, double c2);					// a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0 두 직선의 교차점을 구하는 함수
bool		isSamePoint(API_Coord3D aPoint, API_Coord3D bPoint);													// aPoint가 bPoint와 같은 점인지 확인함
short		moreCloserPoint(API_Coord3D curPoint, API_Coord3D p1, API_Coord3D p2);									// curPoint에 가까운 점이 p1, p2 중 어떤 점입니까?

// 기하 (범위)
bool	inRange(double srcPoint, double targetMin, double targetMax);						// srcPoint 값이 target 범위 안에 들어 있는가?
double	overlapRange(double srcMin, double srcMax, double targetMin, double targetMax);		// src 범위와 target 범위가 겹치는 길이를 리턴함

// 기하 (슬래브 관련)
bool		isAlreadyStored(API_Coord3D aPoint, API_Coord3D pointList[], short startInd, short endInd);		// aPoint가 pointList에 보관이 되었는지 확인함
bool		isNextPoint(API_Coord3D prevPoint, API_Coord3D curPoint, API_Coord3D nextPoint);				// nextPoint가 curPoint의 다음 점입니까?
API_Coord3D	getUnrotatedPoint(API_Coord3D rotatedPoint, API_Coord3D axisPoint, double ang);					// 회전이 적용되지 않았을 때의 위치 (배치되어야 할 본래 위치를 리턴), 각도는 Degree
API_Coord	getUnrotatedPoint(API_Coord rotatedPoint, API_Coord axisPoint, double ang);						// 회전이 적용되지 않았을 때의 위치 (배치되어야 할 본래 위치를 리턴), 각도는 Degree

// 산술
double	round(double x, int digit);		// 반올림

// 문자열
std::string	format_string(const std::string fmt, ...);		// std::string 변수 값에 formatted string을 입력 받음
short		isStringDouble(char* str);						// 문자열 s가 숫자로 된 문자열인지 알려줌 (숫자는 1, 문자열은 0)
short		isStringDouble(const char* str);				// 문자열 s가 숫자로 된 문자열인지 알려줌 (숫자는 1, 문자열은 0)
bool		removeCharInStr(char* str, const char ch);		// 문자열 str에서 특정 문자 ch를 제거함 (제거되면 true, 그대로이면 false)
char*		getResourceStr(short resID, short index);		// 리소스 resID의 (1-기반) index번째 문자열을 가져옴
wchar_t*	charToWchar(const char* str);					// char형 문자열을 wchar_t형 문자열로 변환
char*		wcharToChar(const wchar_t* wstr);				// wchar_t형 문자열을 char형 문자열로 변환
GS::UniString	convertStr(const char* c_str);				// C 문자열을 GS::UniString 문자열로 변환
char*			convertStr(const GS::UniString uni_str);	// GS::UniString 문자열을 C 문자열로 변환
char*			strrstr(const char* haystack, const char* needle);	// 마지막 디렉토리 구분자의 위치를 찾음
// 객체 배치
GSErrCode	placeCoordinateLabel(double xPos, double yPos, double zPos, bool bComment = false, std::string comment = "", short layerInd = 1, short floorInd = 0);	// 좌표 라벨을 배치함

// 클래스: 쉬운 객체 배치
class EasyObjectPlacement
{
private:
	const char*		paramName[50];
	API_AddParID	paramType[50];
	const char*		paramVal[50];
	short			nParams;

	short			layerInd;
	short			floorInd;
	const GS::uchar_t* gsmName;
public:
	double			posX;
	double			posY;
	double			posZ;
	double			radAng;

	EasyObjectPlacement();		// 생성자
	EasyObjectPlacement(const GS::uchar_t* gsmName, short layerInd, short floorInd, double posX, double posY, double posZ, double radAng);	// 생성자
	void		init(const GS::uchar_t* gsmName, short layerInd, short floorInd, double posX, double posY, double posZ, double radAng);	// 초기화 함수
	void		setGsmName(const GS::uchar_t* gsmName);	// GSM 파일명 지정
	void		setLayerInd(short layerInd);	// 레이어 인덱스 지정 (1부터 시작)
	void		setFloorInd(short floorInd);	// 층 인덱스 지정 (음수, 0, 양수 가능)
	void		setParameters(short nParams, const char* paramNameList[], API_AddParID* paramTypeList, const char* paramValList[]);	// 파라미터 지정
	API_Guid	placeObject(double posX, double posY, double posZ, double radAng);		// 객체 배치
	API_Guid	placeObject(const GS::uchar_t* gsmName, short nParams, const char* paramNameList[], API_AddParID* paramTypeList, const char* paramValList[], short layerInd, short floorInd, double posX, double posY, double posZ, double radAng);	// 객체 배치
	API_Guid	placeObject(short nParams, ...);	// 객체 배치 (파라미터의 개수 및 파라미터 이름/타입/값만 입력)
	API_Guid	placeObject(const GS::uchar_t* gsmName, short layerInd, short floorInd, double posX, double posY, double posZ, double radAng, short nParams, ...);				// 초기화를 하는 동시에 객체 배치 (가변 파라미터: 파라미터의 개수 및 파라미터 이름/타입/값만 입력)
	API_Guid	placeObjectMirrored(double posX, double posY, double posZ, double radAng);		// 객체 배치
	API_Guid	placeObjectMirrored(const GS::uchar_t* gsmName, short nParams, const char* paramNameList[], API_AddParID* paramTypeList, const char* paramValList[], short layerInd, short floorInd, double posX, double posY, double posZ, double radAng);	// 객체 배치
	API_Guid	placeObjectMirrored(short nParams, ...);	// 객체 배치 (파라미터의 개수 및 파라미터 이름/타입/값만 입력)
	API_Guid	placeObjectMirrored(const GS::uchar_t* gsmName, short layerInd, short floorInd, double posX, double posY, double posZ, double radAng, short nParams, ...);		// 초기화를 하는 동시에 객체 배치 (가변 파라미터: 파라미터의 개수 및 파라미터 이름/타입/값만 입력)

	// 사용법 안내
	/*
	EasyObjectPlacement objP;
	objP.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, cell.leftBottomX, cell.leftBottomY, cell.leftBottomZ, cell.ang);

	for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
		height = 0.0;
		for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
			height += placementInfo.height [yy];
			elemList.Push (objP.placeObject (5,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", round (placementInfo.width [xx]*1000, 0)).c_str (),
				"eu_hei", APIParT_CString, format_string ("%.0f", round (placementInfo.height [yy]*1000, 0)).c_str (),
				"u_ins", APIParT_CString, "벽세우기",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			moveIn3D ('z', objP.radAng, placementInfo.height [yy], &objP.posX, &objP.posY, &objP.posZ);
		}
		moveIn3D ('x', objP.radAng, placementInfo.width [xx], &objP.posX, &objP.posY, &objP.posZ);
		moveIn3D ('z', objP.radAng, -height, &objP.posX, &objP.posY, &objP.posZ);
	}
	*/
};

// 라이브러리 변수 접근 (Getter/Setter)
bool		setParameterByName(API_ElementMemo* memo, char* pName, double value);		// pName 파라미터의 값을 value로 설정함 (실수형) - 성공하면 true, 실패하면 false
bool		setParameterByName(API_ElementMemo* memo, char* pName, char* value);		// pName 파라미터의 값을 value로 설정함 (문자열) - 성공하면 true, 실패하면 false
double		getParameterValueByName(API_ElementMemo* memo, char* pName);				// pName 파라미터의 값을 가져옴 - 실수형
const char* getParameterStringByName(API_ElementMemo* memo, char* pName);				// pName 파라미터의 값을 가져옴 - 문자열
API_AddParID	getParameterTypeByName(API_ElementMemo* memo, char* pName);				// pName 파라미터의 타입을 가져옴 - API_AddParID

// 기하 (이동)
void		moveIn3D(char direction, double ang, double offset, API_Coord3D* curPos);							// X, Y, Z축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
void		moveIn3D(char direction, double ang, double offset, double* curX, double* curY, double* curZ);		// X, Y, Z축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
void		moveIn2D(char direction, double ang, double offset, API_Coord* curPos);								// X, Y축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
void		moveIn2D(char direction, double ang, double offset, double* curX, double* curY);					// X, Y축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)

void		moveIn3DSlope(char direction, double plainAng, double slopeAng, double offset, API_Coord3D* curPos);						// X, Y축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (평면 상에서의 회전각도 plainAng, 평면의 경사각도 slopeAng, 단위: radian)
void		moveIn3DSlope(char direction, double plainAng, double slopeAng, double offset, double* curX, double* curY, double* curZ);	// X, Y축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (평면 상에서의 회전각도 plainAng, 평면의 경사각도 slopeAng, 단위: radian)

// 레이어
short		findLayerIndex(const char* layerName);																	// 레이어 이름으로 레이어 인덱스 찾기
short		makeTemporaryLayer(API_Guid structurualObject, const char* suffix, char* returnedLayerName = NULL);		// 객체의 레이어 이름이 01-S로 시작하는 경우 접두사를 05-T로 바꾸고, 하이픈 + 접미사 문자열을 붙인 레이어 이름을 생성한 후 레이어 인덱스와 이름을 리턴함
char*		getExplanationOfLayerCode(char* layerName, bool bConstructionCode = true, bool bDong = true, bool bFloor = true, bool bCastNum = true, bool bCJ = true, bool bOrderInCJ = true, bool bObjName = true, bool bProductSite = true, bool bProductNum = true);		// 레이어 코드에 대한 설명을 리턴함
bool		verifyLayerName(const char* layerName);																	// 레이어 이름이 레이어 체계에 맞는 이름인지 확인함
char*		getLayerCode(const char* layerName, short nth_code);													// 레이어 이름에서 n번째 코드의 문자열을 가져옴 (1-공사코드, 2-동, 3-층, 4-타설번호, 5-CJ, 6-시공순서, 7-부재명, 8-제작처, 9-제작번호)

// 정보 수집
GSErrCode	getGuidsOfSelection(GS::Array<API_Guid>* guidList, API_ElemTypeID elemType, long* nElem);		// 선택한 요소들 중에서 요소 ID가 elemType인 객체들의 GUID를 가져옴, 가져온 수량을 리턴함
double		getWorkLevel(short floorInd);																	// 해당 층의 작업 층 고도를 가져옴
short		getLayerCount();																				// 레이어 개수를 가져옴
long		getVerticesOfMorph(API_Guid guid, GS::Array<API_Coord3D>* coords);								// 모프의 꼭지점들을 수집하고 꼭지점 개수를 리턴함

// 요소 조작
void		deleteElements(GS::Array<API_Element> elemList);	// 리스트에 있는 요소들을 모두 삭제함
void		deleteElements(GS::Array<API_Guid> elemList);		// 리스트에 있는 요소들을 모두 삭제함
void		groupElements(GS::Array<API_Guid> elemList);		// 리스트에 있는 요소들을 그룹화
void		selectElements(GS::Array<API_Guid> elemList);		// 리스트에 있는 요소들을 선택함
void		suspendGroups(bool on);								// 그룹화 일시정지 활성화/비활성화

#endif
