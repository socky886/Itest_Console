#include "StdAfx.h"
#include "bt_demo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <string>
#include "common.h"
using namespace std;

const int MAX_DEMO_LOOP = 10;

bt_demo::bt_demo(void)
	: m_handle_VSG(INVALID_HANDLE_VALUE)
	, m_handle_VSA(INVALID_HANDLE_VALUE)
	, m_connect_id(0)
	, m_external_gain(0.0)
	, m_dut_peek_power(0.0)
	, m_freq(2480 * 1e6)
	, m_packet_type(WT_BT_PACKETTYPE_3_DH5)
	, m_Rate(WT_BT_DATARATE_3M)
	, rf_port(1)
	, is_wt_208(false)
{
	memset(&m_VsaParameter, 0, sizeof(VsaParameter));
	memset(&m_VsgParameter, 0, sizeof(VsgParameter));
}


bt_demo::~bt_demo(void)
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

bool bt_demo::run_vsa()
{
	m_handle_VSA = (HANDLE)_beginthreadex(NULL, 0, ThreadBTVSAFun, this, 0, NULL);
	if(m_handle_VSA != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(m_handle_VSA, INFINITE);
		CloseHandle(m_handle_VSA);
		m_handle_VSA = INVALID_HANDLE_VALUE;
		return true;
	}
	return false;
}

bool bt_demo::run_vsg()
{
	m_handle_VSG = (HANDLE)_beginthreadex(NULL, 0, ThreadBTVSGFun, this, 0, NULL);
	if(m_handle_VSG != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(m_handle_VSG, INFINITE);
		CloseHandle(m_handle_VSG);
		m_handle_VSG = INVALID_HANDLE_VALUE;
		return true;
	}
	return false;
}

bool bt_demo::demo_bt_vsa()
{

	printf("demo BT VSA? (Y/N)\n>");

	if(get_Choose())
	{
		if(false == demo_connect_tester(&m_connect_id))
		{
			return false;
		}
		printf("\n============================\n");
		printf("Demo BT VSA, you should input BT VSG first\n");
		printf("Demo BT VSG setting is:\n");
		printf("\tDUT peek power:%lf dBm\n", m_dut_peek_power);
		printf("\tFreq:%lf MHz\n", m_freq / 1e6);
		printf("\tBT packet type:3-DH5\n");
		printf("\tBT datarate:3M\n");
		printf("\tCable loss:%lf dB\n", m_external_gain);
		printf("\n============================\n");
		printf("BT VSG is already ? (Y/N)\n>");

		if(get_Choose())
		{
			printf("Demo BT VSA, waiting for awhile...\n");
			run_vsa();
		}
		WT_DisConnect(m_connect_id);
	}

	return true;
}

unsigned int __stdcall bt_demo::ThreadBTVSAFun(LPVOID pM)
{
	int ret = 0;
	bt_demo *pthis = (bt_demo *)(LPVOID)pM;
	//ret = set_bt_vsa_Autorange();
	ret = pthis->set_bt_vsa_parm();

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
		print_bt_info(pthis->m_connect_id);

		Sleep(100);
	}
	ret = WT_StopDataCapture(pthis->m_connect_id);
	printf("\n============================\n");
	printf("BT VSA demo finish !!!\n");
	printf("\n============================\n");
	return ret;
}

unsigned int __stdcall bt_demo::ThreadBTVSGFun(LPVOID pM)
{
	int ret = 0;
	bt_demo *pthis = (bt_demo *)(LPVOID)pM;

	int vsg_loop = 0;

	while((vsg_loop++) < MAX_DEMO_LOOP)
	{
		printf("\n=======Loop %d==========\n", vsg_loop);
		pthis->WT208_SendBTPacket(2480, 0, 0, 1000);
	}
	printf("\n============================\n");
	printf("BT VSG demo finish !!!\n");
	printf("\n============================\n");
	return ret;
}

unsigned int __stdcall bt_demo::ThreadBT_ACPFun_VSG(LPVOID pM)
{
	int ret = 0;
	bt_demo *pthis = (bt_demo *)(LPVOID)pM;

	VsgParameter vsgParam = {0};
	VsaParameter vsaParam = {0};
	ret = WT_GetDefaultParameter(&vsaParam, &vsgParam);
	if(ret != WT_ERR_CODE_OK)
	{
		printf("Get Default VSG Parameter Fail!!\n Vsg will Stop Now!!\n ");
		return false;
	}
	vsgParam.freq = 2480 * 1e6;
	vsgParam.power = 0.0;
	vsgParam.repeat = 10000;
	vsgParam.rfPort =  get_rf_vsg_config();
	vsgParam.TimeoutWaiting = 120;
	vsgParam.wave_gap = 10;
	vsgParam.waveType = SIG_USERFILE;
	string aaa ;
	char VSGwaveFName[254];
	get_exe_path(VSGwaveFName);
	aaa = string(VSGwaveFName) + string("\\") + string(BT_VSG_1M_WAVEFORM);
	vsgParam.wave = (char *)aaa.c_str();

	printf("\tVSG freq:%lf MHz\n", vsgParam.freq / 1e6);
	printf("\tVSG power:%lf dBm\n", vsgParam.power);
	printf("\tVSG path:%s\n", vsgParam.wave);
	printf("\tVSG repeat:%d\n", vsgParam.repeat);
	printf("\tVSG rf port:RF%d\n", vsgParam.rfPort);
	printf("\tVSG wave gap:%lf us\n", vsgParam.wave_gap);

	ret = WT_SetVSG(pthis->m_connect_id, &vsgParam);
	if(ret == WT_ERR_CODE_OK)
	{
		ret = WT_AsynStartVSG(pthis->m_connect_id);

		if(ret == WT_ERR_CODE_OK)
		{
			printf("VSG Start!\n");
		}
		else
		{
			printf("VSG Fail!\n");
		}
	}
	return true;
}

unsigned int __stdcall bt_demo::ThreadBT_ACPFun_VSA(LPVOID pM)
{
	int ret = 0;
	double *result = 0;
	int elementSize = 0, elementCount = 0;
	bt_demo *pthis = (bt_demo *)(LPVOID)pM;

	VsaParameter vsaParam = {0};
	VsgParameter vsgParam = {0};
	ret = WT_GetDefaultParameter(&vsaParam, &vsgParam);
	if(ret != WT_ERR_CODE_OK)
	{
		printf("Get Default VSa Parameter Fail!!\n Vsa will Stop Now!!\n ");
		return false;
	}
	vsaParam.max_power = 0.0 + 3;
	vsaParam.freq = 2480 * 1e6;
	vsaParam.demod = WT_DEMOD_BT;
	vsaParam.rfPort = WT_PORT_OFF + get_rf_vsa_config();
	vsaParam.smp_time = 2000;
	vsaParam.bt_packet_type = WT_BT_PACKETTYPE_DH5;
	vsaParam.TimeoutWaiting = 5;
	vsaParam.trig_pretime = 20;
	vsaParam.trig_level = -28.0;
	vsaParam.trig_type = WT_TRIG_TYPE_IF;
	vsaParam.trig_timeout = 2;

	printf("\tFreq:%lf MHz\n", vsaParam.freq);
	printf("\tMax_power:%lf dBm\n", vsaParam.max_power);
	printf("\tTrigger level:%lf dBm\n", vsaParam.trig_level);
	printf("\tVSA port:RF%d\n", vsaParam.rfPort);
	printf("\tSample time:%lf ms\n", vsaParam.smp_time);
	printf("\tStandard enums: %d\n", vsaParam.demod);

	WT_SetExternalGain(pthis->m_connect_id, 0.0);
	ret = WT_SetVSA_AutoRange(pthis->m_connect_id, &vsaParam);
	if(ret != WT_ERR_CODE_OK)
	{
		printf("Set Vsa Fail !\n");
		return false;
	}
	ret = WT_DataCapture(pthis->m_connect_id);
	if(ret != WT_ERR_CODE_OK)
	{
		printf("Data Capture Fail!\n");
		return false;
	}

	ret = WT_GetVectorResultElementCount(pthis->m_connect_id, WT_RES_BT_SPETRUM_ADJACENT_CHANNEL_POWER, &elementCount);
	ret = WT_GetVectorResultElementSize(pthis->m_connect_id, WT_RES_BT_SPETRUM_ADJACENT_CHANNEL_POWER, &elementSize);
	if(elementSize * elementCount == 0)
	{
		return false;
	}
	result = (double *)malloc(elementSize * elementCount);
	memset(result, 0, elementSize * elementCount);
	ret = WT_GetVectorResult(pthis->m_connect_id, WT_RES_BT_SPETRUM_ADJACENT_CHANNEL_POWER, result, TYPE_double, elementSize * elementCount);
	if(ret != 0)
	{
		printf("\n============================\n");
		printf("Get BT ACP result Success!!\n");
		printf("ACP result Count:\t%d\n", elementCount);
		printf("ACP result Size:\t%d\n", elementSize);
		printf("ACP result:\n");
		for(int i = 0 ; i < elementCount; i++)
		{
			printf("\t%0.4lf\n", result[i]);
		}
		printf("\n============Demo BT ACP End===============\n");
		printf("Input any key to continue test!\n>");
	}
	return true;
}

int bt_demo::set_bt_vsa_parm()
{
	int ret = WT_SetExternalGain(m_connect_id, m_external_gain);

	printf("set tester external cable loss %lf dB\n", m_external_gain);
	// m_VsaParameter max_power = DUT peek power + 3dBm
	// if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get m_VsaParameter max_power
	m_VsaParameter.max_power = m_dut_peek_power + 3;
	m_VsaParameter.freq = m_freq;
	m_VsaParameter.demod = WT_DEMOD_BT;
	m_VsaParameter.bt_packet_type = m_packet_type;
	m_VsaParameter.bt_Rate = m_Rate;
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
	printf("\tBT packet type:3-DH5\n");
	printf("\tBT datarate:3M\n");

	ret = WT_SetVSA(m_connect_id, &m_VsaParameter);
	return ret;
}

int bt_demo::set_bt_vsa_Autorange()
{
	int ret = 0;

	ret = WT_SetExternalGain(m_connect_id, m_external_gain);
	printf("set tester external cable loss %lf dB\n", m_external_gain);
	// m_VsaParameter max_power = DUT peek power + 3dBm
	// if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get m_VsaParameter max_power
	// here we don't know DUT Max power input, so just set max_power=0
	m_VsaParameter.max_power = m_dut_peek_power + 3;
	m_VsaParameter.freq = m_freq;
	//must set this value
	m_VsaParameter.demod = WT_DEMOD_BT;
	//must set packet type
	m_VsaParameter.bt_packet_type = m_packet_type;
	//must set data rate
	m_VsaParameter.bt_Rate = m_Rate;
	//set rf port
	rf_port = get_rf_vsa_config();
	if(rf_port <= 0 || rf_port > 4)
	{
		rf_port = 1;
		m_VsaParameter.rfPort = WT_PORT_RF1;
	}
	else
	{
		m_VsaParameter.rfPort = WT_PORT_OFF + rf_port;
	}
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
	printf("\tBT packet type:3-DH3\n");
	printf("\tBT datarate:3M\n");
	printf("We are going to run VSA Auto range\n");
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

bool bt_demo::WT208_SendBTPacket(double centFreq_MHz, double loss, double gain, int count)
{
	int ret = WT_ERR_CODE_OK;
	double freq = centFreq_MHz;
	char VSGwaveFName[254];
	VsgParameter txParam;
	VsaParameter rxParam;
	string aaa ;
	get_exe_path(VSGwaveFName);
	aaa = string(VSGwaveFName) + string("\\") + string(BT_VSG_WAVEFORM);
	if(!FileExists(aaa.c_str()))  //20110608
	{
		printf("VSG Wave file [%s] is not exits!\n", aaa.c_str());
		return false;
	}
	rf_port = get_rf_vsg_config();
	if(ret == WT_ERR_CODE_OK) { ret = WT_GetDefaultParameter(&rxParam, &txParam); }

	if(ret == WT_ERR_CODE_OK) { ret = WT_SetExternalGain(m_connect_id , -loss); }
	if(ret == WT_ERR_CODE_OK)
	{
		txParam.freq = freq * 1e6;
		txParam.power = gain;

		if(rf_port <= 0 || rf_port > 4)
		{
			rf_port = 1;
		}
		txParam.rfPort = WT_PORT_OFF + rf_port;
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
		printf("\tVSG RF port:RF%d\n", rf_port);
		printf("\tVSG TimeoutWaiting:%d\n", txParam.TimeoutWaiting);
		printf("\tVSG wave gap:%lf us\n", txParam.wave_gap);
	}
	if(ret == WT_ERR_CODE_OK) { ret = WT_SetVSG(m_connect_id, &txParam); }
	else { printf("Connect[]WT_SetVSG FAIL\n"); }

	if(ret == WT_ERR_CODE_OK) { ret = WT_StartVSG(m_connect_id); }

	if(ret == WT_ERR_CODE_OK)
	{
		printf("Connect[%d] VSG OK\n", m_connect_id);
		return true;
	}
	else
	{
		printf("Connect[%d] VSG FAIL\n", m_connect_id);
		return false;
	}
}

bool bt_demo::demo_bt_vsg()
{

	if(false == demo_Multi_connect_tester(&m_connect_id))
	{
		return false;
	}

	run_vsa();
	run_vsg();

	WT_DisConnect(m_connect_id);
	return true;
}

bool bt_demo::demo_bt_acp()
{
	double  _dut_peek_power = 0.0;
	double  _freq = 2480 * 1e6;
	double  _external_gain = 0.0;
	int     _packet_type = WT_BT_PACKETTYPE_DH3;
	int     _Rate = WT_BT_DATARATE_1M;

	printf("Demo BT ACP? (Y/N)\n>");

	if(get_Choose())
	{
		if(false == demo_connect_tester(&m_connect_id))
		{
			return false;
		}
		printf("\n============================\n");
		printf("We are going to demo BT ACP\n");
		printf("Demo BT VSG setting is:\n");
		printf("\tDUT peek power:%lf dBm\n", _dut_peek_power);
		printf("\tFreq:%lf MHz\n", _freq / 1e6);
		printf("\tBT packet type:1-DH3\n");
		printf("\tBT datarate:1M\n");
		printf("\tCable loss:%lf dB\n", _external_gain);
		printf("\n============================\n");

		printf("Demo BT VSA, waiting for awhile...\n");
		m_handle_VSG = (HANDLE)_beginthreadex(NULL, 0, ThreadBT_ACPFun_VSG, this, 0, NULL);
		Sleep(100);

		m_handle_VSA = (HANDLE)_beginthreadex(NULL, 0, ThreadBT_ACPFun_VSA, this, 0, NULL);
		get_Choose();

		WT_DisConnect(m_connect_id);
	}

	return true;
}
