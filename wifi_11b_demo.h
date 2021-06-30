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


class wifi_11b_demo
{
public:
	wifi_11b_demo();
	~wifi_11b_demo(void);
	bool exec();
protected:
	bool Vsa_Run_Default();
	bool Vsa_Run_DcRemove();
	bool Vsa_Run_EqTaps();
	bool Vsa_Run_CCKPhCorr();
	bool init();
	void finish();
	bool run();
	bool init_vsa_default();
	bool init_vsa_dc_remove(bool isEnable);
	bool init_vsa_eq_taps(int index);
	bool init_vsa_cck_ph_corr(bool isEnable);
private:
	VsaParameter m_VsaParameter;
	VsgParameter m_VsgParameter;
	int m_connect_id;
	double m_external_gain;
	double m_freq;
private:
	double m_dut_peek_power;
	HANDLE m_handle_VSG;
	HANDLE m_handle_VSA;
};

