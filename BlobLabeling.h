#pragma once
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>
#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"
//#include "opencv2/contrib/contrib.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

typedef struct 
{
	bool	bVisitedFlag;
	CvPoint ptReturnPoint;
} Visited;

class  CBlobLabeling
{
public:
	CBlobLabeling(void);
public:
	~CBlobLabeling(void);

public:
	IplImage*	m_Image;				// 레이블링을 위한 이미지
	int			m_nThreshold;			// 레이블링 스레스홀드 값
	Visited*	m_vPoint;				// 레이블링시 방문정보
	int			m_nBlobs;				// 레이블의 갯수
	CvRect*		m_recBlobs;				// 각 레이블 정보
	bool		m_isInited;

public:
	void		SetParam(IplImage* image, int nThreshold);	// 레이블링 이미지 선택
	void		DoLabeling();								// 레이블링(실행)

private:
	int		 Labeling(IplImage* image, int nThreshold);		// 레이블링(동작)
	void	 InitvPoint(int nWidth, int nHeight);			// 포인트 초기화
	void	 DeletevPoint();								// 포인트 초기화
	void	 DetectLabelingRegion(int nLabelNumber, unsigned char *DataBuf, int nWidth, int nHeight);	// 레이블링 결과 얻기
	int		_Labeling(unsigned char *DataBuf, int nWidth, int nHeight, int nThreshold);					// 레이블링(실제 알고리즘)
	int		__NRFIndNeighbor(unsigned char *DataBuf, int nWidth, int nHeight, int nPosX, int nPosY, int *StartX, int *StartY, int *EndX, int *EndY );	// _Labling 내부 사용 함수
	int		__Area(unsigned char *DataBuf, int StartX, int StartY, int EndX, int EndY, int nWidth, int nLevel);											// _Labling 내부 사용 함수
};