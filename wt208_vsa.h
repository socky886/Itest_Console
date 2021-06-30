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

class wt208_vsa
{
public:
	wt208_vsa(void);
	~wt208_vsa(void);
	void set_vsa_port(int port);
	bool exec();
protected:
	bool init();
	bool data_capture();
	int set_vsa_Autorange();
	static DWORD WINAPI ThreadVSAFun(LPVOID pM);
private:
	VsaParameter m_VsaParameter;
	VsgParameter m_VsgParameter;
	int m_connect_id;
	double m_external_gain;
	double m_freq;
	int m_repeat;
	bool is_wifi_vsg;
	int m_port;
	string m_print_string;
private:
	double m_dut_peek_power;
	HANDLE m_handle_VSG;
	HANDLE m_handle_VSA;
	bool m_vsg_runing;
};

