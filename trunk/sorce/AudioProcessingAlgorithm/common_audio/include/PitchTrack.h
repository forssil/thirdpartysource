#ifndef _PITCH_TRACK_H
#define _PITCH_TRACK_H

#define PREVLENGTH              3
#define GROSSPITCHTHRES         50  //Difference between current and previous pitch
#define DELPITCHTHRES           50  //Difference between echo and err pitch
class CPitchTrack  
{
public:
	int m_bValid;
	int m_nPfStartBin;
private:
	int m_nHalfFFTLen;
    float m_nfPitchStart;
	float m_nfPitchEnd;
	float m_nfPitchStep;
	float m_nfTrackEnd;
	//int m_nfEnhEndBin; //max num of inter loop
	int m_nfPitchCnt;
	float m_nInvBinFFT;
	float* m_nfWeightArray;
	int m_nMaxHarmonicsCnt;

	int *m_pHarmonicsNum;		//m_nfPitchCnt
	float *m_pfPitch;			//m_nfPitchCnt
	float *m_pHarmonicsBuf;		//m_nfPitchCnt
	float *m_pHarmonicsFlagBuf;	//m_nHalfFFTLen
    int m_nEnhanceLowBin;
	float m_nfEchoPitchFre;
	float m_nfPitchFre; //out pitch
	float m_nfPrevPitchFre[PREVLENGTH];
	bool is_noise;
	bool is_echo;
	// Output variables
	bool is_pitch;
	int m_nfMaxEnhBin; // out max num of enhanced bin
	float m_nfNearEnergy;

	float GetPitch(float *fAmp);
	void UpdateProb(float sout_echo, float echo_sin, int far_activity);
public:	
	CPitchTrack(int m_nFFTLen=1024, int m_nHalfFFTLen=513,int m_nProcessFs=48000);
	~CPitchTrack();
	void Process(float *AmpEst, float *AmpErr, float corr_sout_echo, float corr_echo_sin, bool far_activity);
	void GetPitchGain(float *fModGain, float *fPwrRatio, float *fStateNoies);
	float *GetHarmonicsBin(){return m_pHarmonicsFlagBuf;};
    float GetPitchFre(){return m_nfPitchFre;};
	bool GetIsPitch() { return is_pitch; };
	int GetInvBinFFT() { return m_nInvBinFFT; };
	void SetNearEnergy(float NearEnergy) { m_nfNearEnergy = NearEnergy; };
	float GetNearEnergy() { return m_nfNearEnergy; };
};

#endif
