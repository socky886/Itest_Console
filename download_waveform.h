#pragma once
#include <Windows.h>
#include <tester.h>
#include <testerCommon.h>
class download_waveform
{
public:
	download_waveform(void);
	~download_waveform(void);

	bool exec();
	bool startWaveformVsg();
private:
	int m_connect_id;
	char *gWaveName;
};

