#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <tester.h>
#include <testerCommon.h>
#include <Windows.h>

using namespace std;
#define WT208_ASYN_VSG_WAVEFORM "3DH3.bwv"
class wt208_vsg
{
public:
	wt208_vsg(bool is_bt);
	~wt208_vsg(void);
	bool exec();
protected:
	bool init();
	int asyn_vsg();
	static DWORD WINAPI ThreadVSGFun(LPVOID pM);
private:
	VsaParameter m_VsaParameter;
	VsgParameter m_VsgParameter;
	int m_connect_id;
	double m_external_gain;
	double m_freq;
	int m_repeat;
	bool is_wifi_vsg;
private:
	double m_dut_peek_power;
	HANDLE m_handle_VSG;
	HANDLE m_handle_VSA;
	bool m_vsg_runing;
};

