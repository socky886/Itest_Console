#pragma once
#include <Windows.h>
#include <tester.h>
#include <testerCommon.h>
#define BT_VSG_WAVEFORM     "3DH1.bwv"
#define BT_VSG_1M_WAVEFORM  "1DH3-1111.bwv"
class bt_demo
{
public:
	bt_demo(void);
	~bt_demo(void);
	bool demo_bt_vsa();
	bool demo_bt_vsg();
	bool demo_bt_acp();
protected:
	bool run_vsg();
	bool run_vsa();
	bool WT208_SendBTPacket(double centFreq_MHz, double loss, double gain, int count);
	int set_bt_vsa_Autorange();
	int set_bt_vsa_parm();
	static unsigned int __stdcall ThreadBTVSAFun(LPVOID pM);
	static unsigned int __stdcall ThreadBTVSGFun(LPVOID pM);
	static unsigned int __stdcall ThreadBT_ACPFun_VSG(LPVOID pM);
	static unsigned int __stdcall ThreadBT_ACPFun_VSA(LPVOID pM);
private:
	VsaParameter m_VsaParameter;
	VsgParameter m_VsgParameter;
	int m_packet_type;
	int m_Rate;
	double m_external_gain;
	double m_freq;
	double m_dut_peek_power;
	int rf_port;
	bool is_wt_208;
private:
	int m_connect_id;
	HANDLE m_handle_VSG;
	HANDLE m_handle_VSA;
};

