/***********************************************************************
*  Author
;*      Gao Hua
;*
;*
;*
;*  History
;*      10/17/2014 Created
;*
;*
;*************************************************************************/

#ifndef DC_REMOVER_
#define DC_REMOVER_

#include "codyyAudioCommon.h"

#define DCREMOVERATE 0.004f

class DCRemover
{
public:
	DCRemover(int Fs, int frameLen, int chn);
	~DCRemover();
	void PrePosAnaInit();

public:
	void findLevelAndDcRemove(float *pData, int ch);

private:
	int m_nChn;
	int m_nFRAMESIZE;
	float m_fInvFRAMESIZE;

	float* m_fpDCRemovPre;
	float* m_fpDCRemov;
	int m_nPreEnhance;
};


#endif // DC_REMOVER_