#ifndef	__FACILITY__
#define __FACILITY__

#include "MaxBIM21.h"

namespace FacilityDG {
	// 원형 버블 방향
	enum	circularBubblePos {
		UP = 1,
		DOWN,
		LEFT,
		RIGHT
	};

	enum	dgCircularBubbleSettings {
		LABEL_DIAMETER = 3,
		EDITCONTROL_DIAMETER,
		LABEL_LETTER_SIZE,
		EDITCONTROL_LETTER_SIZE,
		LABEL_WITHDRAWAL_LENGTH,
		EDITCONTROL_WITHDRAWAL_LENGTH,
		LABEL_BUBBLE_POS,
		POPUPCONTROL_BUBBLE_POS,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_CIRCULAR_BUBBLE,
		USERCONTROL_LAYER_CIRCULAR_BUBBLE
	};

	enum	dgCameraManager {
		BUTTON_LOAD = 3,
		BUTTON_CLOSE,
		LISTVIEW_CAMERA_POS_NAME,
		EDITCONTROL_CAMERA_POS_NAME
	};
}

struct CircularBubble
{
	short	floorInd;		// 층 인덱스

	short	layerInd;		// 레이어 인덱스
	short	pos;			// 위치 (UP/DOWN/LEFT/RIGHT)
	double	szBubbleDia;	// 버블 직경
	double	szFont;			// 글자 크기
	double	lenWithdrawal;	// 인출선 길이 (오프셋)
};

struct Rect {
	API_Guid guid;	// 객체의 GUID
	double x;		// 원점의 x 좌표
	double y;		// 원점의 y 좌표
	double z;		// 원점의 z 좌표
	double horLen;	// 가로 길이
	double verLen;	// 세로 길이
	double height;	// 높이
	double radAng;	// 회전 각도
};

GSErrCode	select3DQuality (void);					// 3D 품질/속도 조정하기
GSErrCode	attach3DLabelOnZone (void);				// 영역에 3D 라벨 붙이기
short DGCALLBACK selectLayerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [다이얼로그 박스] 레이어 선택

GSErrCode	attachBubbleOnCurrentFloorPlan (void);	// 현재 평면도의 테이블폼에 버블 자동 배치
short DGCALLBACK setBubbleHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [다이얼로그 박스] 원형 버블 설정
GSErrCode	saveDialogStatus_bubble (CircularBubble	*cbInfo);		// 원형 버블 설정 상태 저장
GSErrCode	loadDialogStatus_bubble (CircularBubble	*cbInfo);		// 원형 버블 설정 상태 로드

GSErrCode	manageCameraInfo (void);				// 카메라 위치 저장하기/불러오기
short DGCALLBACK cameraPosManagerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [다이얼로그 박스] 카메라 위치 관리하기

GSErrCode	interferenceCheck(void);				// 간섭 체크 발견하기
int checkIntersect(Rect rect1, Rect rect2);			// 두 객체 간에 충돌이 발생하면 1, 충돌하지 않으면 0, 입력이 유효하지 않으면 -1

GSErrCode	rotateMultipleObjects(void);			// 여러 객체 동시에 회전시키기

#endif