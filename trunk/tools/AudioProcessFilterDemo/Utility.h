#ifndef		_HD_UTILITY_MANAGER_H_
#define		_HD_UTILITY_MANAGER_H_
#include "strmif.h"

#define DEMO_CAPTURE_RENDERER_TEST        0   //TODO: this macro is used to test capture filter and renderer filter 2015-8-28 by sainan.

enum CONNECT_TYPE
{
	CON_MIC_IN = 0,
	CON_CLASSTOCLASS_IN,
	CON_ELECTRONICPIANO_IN,
	CON_ONLINECLASS_IN,

	CON_ONLINECLASS_OUT,
	CON_SPEAKER_OUT,
	CON_RECORDERER_OUT,
	CON_LOOP_OUT,

	CON_INFINITEPINTREE_IN,

};


class CUtility
{
public:
	CUtility();
	virtual ~CUtility();

public:

	// 连接2个filters
	static HRESULT	ConnectFilters(IGraphBuilder *pGraph, IPin *pOut, IBaseFilter *pDest);
	static HRESULT	ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, IBaseFilter *pDest);
	static HRESULT	ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, IBaseFilter *pDest, CONNECT_TYPE eConnectType);

	// 从filter处断开后面所有连接
	static void	NukeDownstream(IGraphBuilder* pGraph, IBaseFilter* pFilter, bool bRemove);

	// 获取未连接的pin
	static HRESULT GetUnconnectedPin(IBaseFilter * pFilter, PIN_DIRECTION PinDir, IPin** ppPin);

	// 两个filter是否连接
	static BOOL	IsConnected(IBaseFilter* pSourceFilter, IBaseFilter* pDestFilter);

	// 通过CLSID增加filter至graph
	static HRESULT AddFilterByCLSID(IGraphBuilder *pGraph, const GUID& clsid, 
		LPCWSTR wszName, IBaseFilter **ppF);


	///////////////////////////////

	// 获取当前dll路径
	static int GetCurPath(LPSTR path);

	static HRESULT _set_audio_buffer(IBaseFilter *pFilter, ULONG ulMillisecond, int nSampleRate);


};


#endif // _HD_UTILITY_MANAGER_H_