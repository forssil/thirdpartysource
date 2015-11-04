#include "DummyCapturer.h"
#include "DummyPlayer.h"
#include "MultipleSourceSync.h"

int main()
{
	int fs = 22050;
	int frameTime = 10; // 10 ms
	int frameSize = fs*frameTime/1000;
	int captureDriftTime = 2;
	int captureDriftSize = fs*captureDriftTime/1000;
	
	int threadNum = 3;
	DummyPlayer* dummyPlayer = new DummyPlayer;
	MultipleSourceSync sync(*dummyPlayer, 3, 2, frameTime, fs, SYNC_MODE_PUSH, REAL_FIX_DATA);

	int captureTimeDrift = 2;
	DummyCapturer capture1(0, frameSize+captureDriftSize, sync, "capture_thread_0", "test1.wav");
	DummyCapturer capture2(1, frameSize+captureDriftSize, sync, "capture_thread_1", "test2.wav");
	DummyCapturer capture3(2, frameSize+captureDriftSize, sync, "capture_thread_2", "test3.wav");

	SWavFileHead wavhead = capture1.getReadWavHead();
	dummyPlayer->setWaveHead(wavhead);

	int fs1 = capture1.getFs();
	if(fs1!=capture2.getFs() || fs1!=capture3.getFs() || fs!=capture3.getFs())
	{
		printf("sample rates are not the same!");
		return -1;
	}

	// must start sync first
	sync.StartProcess();
	capture1.StartCapture();
	capture2.StartCapture();
	capture3.StartCapture();

	getchar();
	delete dummyPlayer;
	getchar();
}

