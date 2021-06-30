#pragma once
#include <Windows.h>
#include <tester.h>
#include <testerCommon.h>

class zigbee_demo
{
public:
	zigbee_demo(void);
	~zigbee_demo(void);
	void exec();
protected:
	bool demo_zigbee_vsa();
	bool demo_zigbee_vsg();
	bool run_vsg();
	bool run_vsa();
	int set_zigbee_vsa_Autorange();
	int set_zigbee_vsa_parm();
	bool SendZigBeePacket(double centFreq_MHz, double loss, double gain, int count);
	static unsigned int __stdcall ThreadZigBeeVSAFun(LPVOID pM);
	static unsigned int __stdcall ThreadZigBeeVSGFun(LPVOID pM);
private:
	VsaParameter m_VsaParameter;
	VsgParameter m_VsgParameter;
	int m_packet_type;
	int m_Rate;
	double m_external_gain;
	double m_freq;
	double m_dut_peek_power;
	int rf_port;
private:
	int m_connect_id;
	HANDLE m_handle_VSG;
	HANDLE m_handle_VSA;
};

