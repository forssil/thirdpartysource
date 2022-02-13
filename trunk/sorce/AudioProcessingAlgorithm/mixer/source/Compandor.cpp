/*! \file   Compandor.cpp
*   \brief  Audio Compandor Algorithm
*/

#include <stdlib.h>
#include <math.h>

#include "Amp_compute.h"
#include "Compandor.h"


CCompandor::CCompandor( const int fs,
						const int lengthofchannel, 
						CHANNELINFO mChannelInfo)
	: m_bIsInitial(false)
	, m_nFs(fs)
	, m_nSize(lengthofchannel)
	, m_fEnergy(0.f)
	, m_fSmoothGainOfCompandor(0.f)
	, m_fDelta(0.f)
	, m_pChannelInfo(mChannelInfo)

{
	//initial 
	__Init();
}

CCompandor::~CCompandor(void)
{

}

//! deal with audio compandor 
/*!
 \param   memory - input data
 \param   memory - output data
 \return  true if success, others false
*/
bool CCompandor::AudioCompandor(float* memory,int len)
{
	if(true != m_bIsInitial)
	{
		////AUDIO_PROCESSING_PRINTF("__Init() is failed!");
		return false;
	}
	if (NULL == memory)
	{
		//AUDIO_PROCESSING_PRINTF("memory cannot be setted NULL!");
		return false;
	}
	float* idata = memory;
    for(int i= 0;i<len/m_nSize;i++){
        __PerformCompandor(idata);
    }
	return true;
}

float CCompandor::AudioCompandorEnergy(float idata)
{
    if(true != m_bIsInitial)
    {
        ////AUDIO_PROCESSING_PRINTF("__Init() is failed!");
        return false;
    }
   
   return __PerformCompandorEnergy(idata);
    
}
//! initial compandor class.
/*!
 \param   none
 \return  true if success, others false
*/
bool CCompandor::__Init()
{
	if (m_nFs <= 0 ||
		m_nSize <= 0 ||
		m_pChannelInfo.fGain_ < 0 )
	{
		//AUDIO_PROCESSING_PRINTF("Input parameters are wrong!");
		return false;
	}
	m_fSmoothGainOfCompandor = 1.f;
    m_compandorParameter = &(m_pChannelInfo.compandor_para);
    m_compandorParameter->upThresholdOfGainSmooth = K/(K+m_compandorParameter->gain_smooth_up_time *m_nFs/1000.f);
    m_compandorParameter->downThresholdOfGainSmooth = K/(K+m_compandorParameter->gain_smooth_down_time *m_nFs/1000.f);

	//initial is success
	m_bIsInitial = true;

	return true;
}

float CCompandor::__PerformCompandorEnergy(float idata){
   
    //Level Smooth
    __LevelSmooth(idata);
    //GaUpdate
    float gain = __GainUpdate();
    m_fSmoothGainOfCompandor = __Recursive(m_fSmoothGainOfCompandor, gain, 0.7f, 0.4f);
    return m_fSmoothGainOfCompandor;

}
//! perform compandor
/*!
 \param   idata - input/output data
 \return  none
*/
void CCompandor::__PerformCompandor(float* idata)
{
	int i = 0;
	float gain = 1.f; //temporary ga
	float functionofamplitude = 0.f; //energy = pow(amplitude, 2) , envelope = abs(amplitude)
    
    for (i= 0; i< m_nSize; ++i){
       idata[i] *=  (m_pChannelInfo.fGain_);
    }
    functionofamplitude = GetPowerEnergy(idata, m_nSize);

    //Level Smooth
    __LevelSmooth(functionofamplitude);
    //GaUpdate
    gain = __GainUpdate();

    for (i=0; i<m_nSize; ++i)
    {
        //GaSmooth
        __FrameGainSmooth(gain);
        idata[i] *=  m_fSmoothGainOfCompandor ;
        Clipping(idata+i);
    }

	
}

//! level smooth
/*!
 \param   data - energy/envelope value
 \return  none
*/
void CCompandor::__LevelSmooth(const float data)
{
	//Level Smooth (energy tracking or evelope tracking)
	float recursivedata = 0.f;
	float newdata = data;
	float ratio1 = m_compandorParameter->upThresholdOfLevelSmooth;
	float ratio2 = m_compandorParameter->downThresholdOfLevelSmooth;
    
    recursivedata = m_fEnergy;
    if(newdata > 0.00002f)
    { //recursive way
        m_fEnergy = __Recursive(recursivedata, newdata, ratio1, ratio2);
    }
    else{
        m_fEnergy = __Recursive(recursivedata, newdata, 0.1f, 0.3f);
    }
}

//! gaupdate
/*!
 \return  output gavalue
*/
float CCompandor::__GainUpdate()
{
	float gain = 1.f;
	float recursivedata = 0.f;
	float upthreshold = 0.f;
	float downthreshold = 0.f;
	float upratio = m_compandorParameter->upRatioOfGainUpdate;
	float downratio = m_compandorParameter->downRatioOfGainUpdate;
	float concealLevel = m_compandorParameter->concealLevel;
	float moderateLevelGain = m_compandorParameter->moderateLevelGain;
	upthreshold = m_compandorParameter->upThresholdOfAmplitude;
	downthreshold = m_compandorParameter->downThresholdOfAmplitude;
    recursivedata = sqrt(m_fEnergy);

    if (recursivedata > upthreshold)
    { //if greater than the up limit
        gain = moderateLevelGain * pow((recursivedata / upthreshold), (upratio - 1));
    }
    else if (concealLevel < recursivedata && (recursivedata < downthreshold))
    { //if smaller than the down limit
        gain = moderateLevelGain * pow((recursivedata / downthreshold), (downratio - 1));
    }
    else if(concealLevel <= recursivedata)
    {
        gain= moderateLevelGain ;
    }
    else
    {
        gain = moderateLevelGain * pow((concealLevel / downthreshold), (downratio - 1));;
    }

	return gain;
}

//! frame gasmooth
/*!
 \param   ga- gavalue of current sample
 \return  none
*/
void CCompandor::__FrameGainSmooth(const float gain)
{
	float ratio1 = m_compandorParameter->upThresholdOfGainSmooth;
	float ratio2 = m_compandorParameter->downThresholdOfGainSmooth;
	float recursivedata = m_fSmoothGainOfCompandor;
	//recursive way
	m_fSmoothGainOfCompandor = __Recursive(recursivedata, gain, ratio1, ratio2);
}

//! recursive way
/*!
 \param   recursivedata - recursive data 
 \param   newdata - new data 
 \param   ratio1 - up ratio
 \param   ratio2 - down ratio
 \return  recursive result
*/
/*inline */float CCompandor::__Recursive( float recursivedata,
													const float newdata, 
													const float ratio1, 
													const float ratio2)
{
	if (newdata > recursivedata)
	{
		recursivedata = recursivedata*(1-ratio1) + newdata*ratio1;
	}
	else 
	{
		recursivedata = recursivedata*(1-ratio2) + newdata*ratio2;
	}
	return recursivedata;
}

