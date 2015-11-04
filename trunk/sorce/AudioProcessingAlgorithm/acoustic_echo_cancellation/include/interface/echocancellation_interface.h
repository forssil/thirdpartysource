/***********************************************************************
 *  Author
;*      Gao Hua
;*      
;*     
;*
;*  History
;*      10/15/2014 Created
;*
;*
;*************************************************************************/
#ifndef _ECHOCANCELLATION_INTERFACE_
#define _ECHOCANCELLATION_INTERFACE_
#include "audiotypedef.h"
#include "AudioModuleImplBase.h"


class CEchoCancellationInterface: public IAudioProcessImplBase
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
protected:
	CEchoCancellationInterface(){};
	
	virtual ~CEchoCancellationInterface() {};

};
extern "C"
{
	CEchoCancellationInterface* CreateIAECInst(int Fs,float fftlen_ms,float framlen_ms);
	CEchoCancellationInterface* CreateIAECInst_int(int Fs,int fftlen,int framlen);
	int DeleteIAECInst (CEchoCancellationInterface*  IAecInst);	
};

#endif //_ECHOCANCELLATION_INTERFACE_