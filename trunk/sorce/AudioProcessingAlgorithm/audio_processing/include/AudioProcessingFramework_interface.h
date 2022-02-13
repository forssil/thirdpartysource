/***********************************************************************
 *  Author
;*      Gao Hua
;*      
;*     
;*
;*  History
;*      1/15/2022 Created
;*
;*
;*************************************************************************/
#ifndef _AUDOPROCESSINGFRAMEWORK_INTERFACE_
#define _AUDOPROCESSINGFRAMEWORK_INTERFACE_
#include "audiotypedef.h"
#include "AudioModuleImplBase.h"


class CAudioProcessingFrameworkInterface: public IAudioProcessImplBase
{
public:
	virtual int Init()=0;
//	virtual int process(audio_pro_share& aShareData)=0;
	virtual void SetOffset(float fre_start)=0;
	virtual void SetAEC_OnOff(bool OnOff)=0;
	virtual void SetNR_OnOff(bool OnOff)=0;
	virtual void Reset()=0;
	virtual bool SetDelay(int nDelay)=0;
	virtual int  ProcessRefferData(audio_pro_share& aShareData)=0;
	virtual void SetMainMicIndex(int micindx) = 0;
protected:
	CAudioProcessingFrameworkInterface(){};
	
	virtual ~CAudioProcessingFrameworkInterface() {};

};
extern "C"
{
	CAudioProcessingFrameworkInterface* CreateIApfInst_int(int mic_nums, int Fs,int fftlen,int framlen);
	int DeleteIAPFInst (CAudioProcessingFrameworkInterface*  IAecInst);	
};

#endif //_AUDOPROCESSINGFRAMEWORK_INTERFACE_