/***********************************************************************
 *  Author
;*      Gao Hua
;*      
;*     
;*
;*  History
;*      10/16/2014 Created
;*
;*
;*************************************************************************/

#ifndef _DELAY_BUFFER_
#define _DELAY_BUFFER_

#define VAD_BAND_NUM 3
typedef struct Audioframe
{
	float* fp;
	bool  VAD;
	float* VADBand;
	int VADBandBuffSize;
}Audioframe_t;

class CDelayBuffer
{
public:
	CDelayBuffer(int rows,int columns);
	~CDelayBuffer();
	void getAudioFrame(int offSet, Audioframe_t* pFrame);
	int UpdateData(Audioframe_t* pFrame);
	bool Init();
	void Reset();
protected:
	float* m_fpBuffer;
	bool* m_VADBuff;
	float* m_VADBandBuff;
	int m_VADBandBuffSize;
	int vadBandBuffWriterPtr;
	int m_nWritInd;
	int m_nRow;
	int m_nColumn;
	bool m_bInit;
};


#endif //_DELAY_BUFFER_