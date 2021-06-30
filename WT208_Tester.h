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

// int Itest_Connect_Tester(void);

class WT208_Tester
{
public:
	WT208_Tester(void);
	~WT208_Tester(void);
	void set_vsa_port(int port);
	bool exec();
    int  multi_connect(void);
    void tester_unit_test(void);
// protected:
	bool init();
	bool data_capture();
	int set_vsa_Autorange();
    int set_continue_wave_Autorange(bool isTrigger);
    int set_vsa_parm();
    int set_modulation_vsa_parm();
    int GrabVSAContinueWave(void);
    int print_continuewave_power_and_freq_err(int id);
    int Grab_CW_Frequency_and_power(void);
    int Grab_FSK_Data(void);
	static DWORD WINAPI ThreadVSAFun(LPVOID pM);

    int set_vsg_parm(void);
	int set_vsg_parm(double power);
	int asyn_vsg(double power);
    int asyn_vsg();
    int stop_vsg();
// private:
	VsaParameter m_VsaParameter;
	VsgParameter m_VsgParameter;
    LrwpanFskParam m_LrwpanFskParam;
	int m_connect_id;
	double m_external_gain;
	double m_freq;
    double m_freq_offset;
	double m_obw;
	int m_repeat;
	bool is_wifi_vsg;
	int m_port;
	string m_print_string;
// private:
	double m_dut_peek_power;
	// double m_dut_Verify_Power;
	HANDLE m_handle_VSG;
	HANDLE m_handle_VSA;
	bool m_vsg_runing;
    bool m_Connected;
	bool m_single_autorange;
};

