#include "StdAfx.h"
#include "zigbee_demo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <string>
#include <iostream>
#include "common.h"
using namespace std;

static const int MAX_DEMO_LOOP = 10;
zigbee_demo::zigbee_demo(void)
	: m_handle_VSG(INVALID_HANDLE_VALUE)
	, m_handle_VSA(INVALID_HANDLE_VALUE)
	, m_connect_id(0)
	, m_external_gain(0.0)
	, m_dut_peek_power(0.0)
	, m_freq(2480 * 1e6)
	, rf_port(0)
{
	memset(&m_VsaParameter, 0, sizeof(VsaParameter));
	memset(&m_VsgParameter, 0, sizeof(VsgParameter));
}



zigbee_demo::~zigbee_demo(void)
{
	if(m_handle_VSA != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(m_handle_VSA, INFINITE);
		CloseHandle(m_handle_VSA);
	}
	if(m_handle_VSG != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(m_handle_VSG, INFINITE);
		CloseHandle(m_handle_VSG);
	}
}

bool zigbee_demo::run_vsa()
{
	m_handle_VSA = (HANDLE)_beginthreadex(NULL, 0, NULL, this, 0, NULL);
	if(m_handle_VSA != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(m_handle_VSA, INFINITE);
		CloseHandle(m_handle_VSA);
		m_handle_VSA = INVALID_HANDLE_VALUE;
		return true;
	}
	return false;
}

bool zigbee_demo::run_vsg()
{
	m_handle_VSG = (HANDLE)_beginthreadex(NULL, 0, NULL, this, 0, NULL);
	if(m_handle_VSG != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(m_handle_VSG, INFINITE);
		CloseHandle(m_handle_VSG);
		m_handle_VSG = INVALID_HANDLE_VALUE;
		return true;
	}
	return false;
}

int zigbee_demo::set_zigbee_vsa_parm()
{
	int ret = WT_SetExternalGain(m_connect_id, m_external_gain);

	printf("set tester external cable loss %lf dB\n", m_external_gain);
	// m_VsaParameter max_power = DUT peek power + 3dBm
	// if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get m_VsaParameter max_power
	m_VsaParameter.max_power = m_dut_peek_power + 3;
	m_VsaParameter.freq = m_freq;
	m_VsaParameter.demod = WT_DEMOD_ZIGBEE;
	rf_port = get_rf_vsa_config();
	m_VsaParameter.rfPort = WT_PORT_OFF + rf_port;
	m_VsaParameter.smp_time = 500;
	m_VsaParameter.TimeoutWaiting = 5;
	//trig_pretime set to larger than BT frame header transmit time, set it to 150us
	m_VsaParameter.trig_pretime = 150;
	//trig_level set to -28.0dB
	m_VsaParameter.trig_level = -28.0;
	m_VsaParameter.trig_type = WT_TRIG_TYPE_IF;
	m_VsaParameter.trig_timeout = 2;

	printf("Set vsa parm:\n");
	printf("\tMax_power:%lfdBm\n", m_VsaParameter.max_power);
	printf("\tFreq:%lf MHz\n", m_VsaParameter.freq / 1e6);
	printf("\tVSA port:RF%d\n", rf_port);
	printf("\tTrigger level:%lfdBm\n", m_VsaParameter.trig_level);
	printf("\tTrigger pretime:%lfus\n", m_VsaParameter.trig_pretime);
	printf("\tTrigger timeout:%lf s\n", m_VsaParameter.trig_timeout);
	printf("\tSample time:%lfms\n", m_VsaParameter.smp_time);

	ret = WT_SetVSA(m_connect_id, &m_VsaParameter);
	return ret;
}

int zigbee_demo::set_zigbee_vsa_Autorange()
{
	int ret = 0;

	ret = WT_SetExternalGain(m_connect_id, m_external_gain);
	printf("Set tester external cable loss %lf dB\n", m_external_gain);
	// m_VsaParameter max_power = DUT peek power + 3dBm
	// if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get m_VsaParameter max_power
	// here we don't know DUT Max power input, so just set max_power=0
	m_VsaParameter.max_power = m_dut_peek_power + 3;
	m_VsaParameter.freq = m_freq;
	//must set this value
	m_VsaParameter.demod = WT_DEMOD_ZIGBEE;
	//must set packet type
	m_VsaParameter.bt_packet_type = m_packet_type;
	//must set data rate
	m_VsaParameter.bt_Rate = m_Rate;
	//set rf port
	rf_port = get_rf_vsa_config();
	m_VsaParameter.rfPort = WT_PORT_OFF + rf_port;
	//set sample time
	m_VsaParameter.smp_time = 500;
	m_VsaParameter.TimeoutWaiting = 5;
	//trig_pretime set to larger than BT frame header transmit time, set it to 150us
	m_VsaParameter.trig_pretime = 150;
	//trigger level set to -28dB
	m_VsaParameter.trig_level = -28.0;
	m_VsaParameter.trig_type = WT_TRIG_TYPE_IF;
	m_VsaParameter.trig_timeout = 2;

	printf("Set auto range VSA parm:\n");
	printf("\tAuto range max_power:%lf dBm\n", m_VsaParameter.max_power);
	printf("\tTrigger pretime:%lf us\n", m_VsaParameter.trig_pretime);
	printf("\tTrigger level:%lf dBm\n", m_VsaParameter.trig_level);
	printf("\tTrigger timeout:%lf s\n", m_VsaParameter.trig_timeout);
	printf("\tFreq:%lf MHz\n", m_VsaParameter.freq / 1e6);
	printf("\tVSA port:RF%d\n", rf_port);
	printf("\tSample time:%lf ms\n", m_VsaParameter.smp_time);
	printf("\tSet Zigbee AutoRange timeout 500ms\n");
	printf("We are going to run VSA Auto range\n");

	//set VSA max wave gap timeout
	ret = WT_SetVSAMaxIFG(m_connect_id, 500);
	ret = WT_SetVSA_AutoRange(m_connect_id, &m_VsaParameter);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("After run VSA Auto range:\n\tmax power level:%lf dBm\n\ttrigger level:%lf dBm\n", m_VsaParameter.max_power, m_VsaParameter.trig_level);
	}
	else
	{
		printf("%s Fail\n", __FUNCTION__);
	}
	return ret;
}


unsigned int __stdcall zigbee_demo::ThreadZigBeeVSAFun(LPVOID pM)
{
	int ret = 0;
	zigbee_demo *pthis = (zigbee_demo *)(LPVOID)pM;
	//ret =pthis->set_zigbee_vsa_Autorange();
	ret = pthis->set_zigbee_vsa_parm();

	int vsa_loop = 0;
	while((vsa_loop++) < MAX_DEMO_LOOP)
	{

		ret = WT_DataCapture(pthis->m_connect_id);
		if(WT_ERR_CODE_OK != ret)
		{
			printf("WT_DataCapture ret: %d\n", ret);
			break;
		}
		printf("\n========Loop %d=============\n", vsa_loop);
		ret = print_power(pthis->m_connect_id);
		if(WT_ERR_CODE_OK != ret)
		{
			break;
		}
		ret = print_evm(pthis->m_connect_id);
		if(WT_ERR_CODE_OK != ret)
		{
			break;
		}
		ret = print_zigbee_info(pthis->m_connect_id);
		if(WT_ERR_CODE_OK != ret)
		{
			break;
		}
		print_freq_err(pthis->m_connect_id);
		Sleep(100);
	}
	ret = WT_StopDataCapture(pthis->m_connect_id);
	printf("\n============================\n");
	printf("BT VSA demo finish !!!\n");
	printf("\n============================\n");
	return ret;
}

unsigned int __stdcall zigbee_demo::ThreadZigBeeVSGFun(LPVOID pM)
{
	int ret = 0;
	zigbee_demo *pthis = (zigbee_demo *)(LPVOID)pM;

	int vsg_loop = 0;
	while((vsg_loop++) < MAX_DEMO_LOOP)
	{
		//pthis->WT208_SendZigBeePacket(2480,0,0,1000,3);
	}
	printf("\n============================\n");
	printf("BT VSG demo finish !!!\n");
	printf("\n============================\n");
	return ret;
}

void zigbee_demo::exec()
{
	std::cout << "\n=============================" << std::endl;
	std::cout << "ZigBee demo please choose [1~2] " << std::endl;
	std::cout << "\t1:Demo ZigBee VSA" << std::endl;
	std::cout << "\t2:Demo ZigBee VSG" << std::endl;
	std::cout << "\n=============================" << std::endl;
	switch(get_index())
	{
		case 1:
			demo_zigbee_vsa();
			break;
		case 2:
			demo_zigbee_vsg();
			break;
	}
}
bool zigbee_demo::demo_zigbee_vsa()
{
	printf("Demo ZigBee VSA? (Y/N)\n>");

	if(get_Choose())
	{
		if(false == demo_connect_tester(&m_connect_id))
		{
			return false;
		}
		printf("\n============================\n");
		printf("We are going to demo ZigBee VSA, you should input ZigBee VSG first\n");
		printf("Demo ZigBee VSG setting is:\n");
		printf("\tDUT peek power:%lf dBm\n", m_dut_peek_power);
		printf("\tFreq:%lf MHz\n", m_freq / 1e6);
		printf("\tCable loss:%lf dB\n", m_external_gain);
		printf("\n============================\n");
		printf("ZigBee VSG is already start ? (Y/N)\n>");

		if(get_Choose())
		{
			printf("Now we are going to demo ZigBee VSA, waiting for awhile\n");
			run_vsa();
		}
		WT_DisConnect(m_connect_id);
	}

	return true;
}

bool zigbee_demo::demo_zigbee_vsg()
{
	if(false == demo_connect_tester(&m_connect_id))
	{
		return false;
	}
	run_vsg();

	WT_DisConnect(m_connect_id);
	return true;
}

bool zigbee_demo::SendZigBeePacket(double centFreq_MHz, double loss, double gain, int count)
{
	int ret = WT_ERR_CODE_OK;
	double freq = centFreq_MHz;
	char VSGwaveFName[254];
	VsgParameter txParam;
	VsaParameter rxParam;
	string aaa ;
	get_exe_path(VSGwaveFName);
	aaa = string(VSGwaveFName) + string("\\OQPSK.bwv");

	if(!FileExists(aaa.c_str()))
	{
		printf("VSG Wave file %s is not exits!", aaa.c_str());
		return false;
	}

	if(ret == WT_ERR_CODE_OK) { ret = WT_GetDefaultParameter(&rxParam, &txParam); }

	if(ret == WT_ERR_CODE_OK) { ret = WT_SetExternalGain(m_connect_id , -loss); }
	if(ret == WT_ERR_CODE_OK)
	{
		txParam.freq = freq * 1e6;
		txParam.power = gain;
		txParam.rfPort =  get_rf_vsg_config();
		txParam.waveType = SIG_USERFILE;
		txParam.wave = (char *)aaa.c_str();
		txParam.repeat = count;
		txParam.wave_gap = 1;
		txParam.TimeoutWaiting = 120;
		printf("Set VSG parm\n");
		printf("\tVSG path:%s\n", txParam.wave);
		printf("\tVSG freq:%lf MHz\n", txParam.freq / 1e6);
		printf("\tVSG power:%lf dBm\n", txParam.power);
		printf("\tVSG repeat:%d\n", txParam.repeat);
		printf("\tVSG RF port:RF%d\n", get_rf_vsg_config());
		printf("\tVSG TimeoutWaiting:%d\n", txParam.TimeoutWaiting);
		printf("\tVSG wave gap:%lf us\n", txParam.wave_gap);
	}
	if(ret == WT_ERR_CODE_OK) { ret = WT_SetVSG(m_connect_id, &txParam); }
	else { printf("[]WT_SetVSG FAIL\n"); }
	if(ret == WT_ERR_CODE_OK) { ret = WT_StartVSG(m_connect_id); }

	if(ret == WT_ERR_CODE_OK)
	{
		printf("[%d] VSG OK\n", m_connect_id);
		return true;
	}
	else
	{
		printf("[%d] VSG FAIL\n", m_connect_id);
		return false;
	}
}