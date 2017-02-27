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
	IplImage*	m_Image;				// ���̺��� ���� �̹���
	int			m_nThreshold;			// ���̺� ������Ȧ�� ��
	Visited*	m_vPoint;				// ���̺��� �湮����
	int			m_nBlobs;				// ���̺��� ����
	CvRect*		m_recBlobs;				// �� ���̺� ����
	bool		m_isInited;

public:
	void		SetParam(IplImage* image, int nThreshold);	// ���̺� �̹��� ����
	void		DoLabeling();								// ���̺�(����)

private:
	int		 Labeling(IplImage* image, int nThreshold);		// ���̺�(����)
	void	 InitvPoint(int nWidth, int nHeight);			// ����Ʈ �ʱ�ȭ
	void	 DeletevPoint();								// ����Ʈ �ʱ�ȭ
	void	 DetectLabelingRegion(int nLabelNumber, unsigned char *DataBuf, int nWidth, int nHeight);	// ���̺� ��� ���
	int		_Labeling(unsigned char *DataBuf, int nWidth, int nHeight, int nThreshold);					// ���̺�(���� �˰���)
	int		__NRFIndNeighbor(unsigned char *DataBuf, int nWidth, int nHeight, int nPosX, int nPosY, int *StartX, int *StartY, int *EndX, int *EndY );	// _Labling ���� ��� �Լ�
	int		__Area(unsigned char *DataBuf, int StartX, int StartY, int EndX, int EndY, int nWidth, int nLevel);											// _Labling ���� ��� �Լ�
};