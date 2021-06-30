// API_Demo.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <tester.h>
#include <testerCommon.h>
#include <Windows.h>
#include <regex>
#include <vector>
#include <string>
#include "common.h"
#include "wifi_11b_demo.h"
#include "wt208_vsg.h"
#include "bt_demo.h"
#include "download_waveform.h"
#include "zigbee_demo.h"
#include "wt208_vsa.h"
//#include "Vertex_Com.h"
#include "Vertex_CIU.h"
#include "WT208_Tester.h"
#include "harmwee.h"
#include "harm_nic.h"

// #include "serial\serial.h"
// using namespace serial;
using namespace std;

#pragma comment(lib,"WLAN.Tester.API.lib")
// #pragma comment(lib,"serial.lib")

static int connect_id = 0;
static volatile bool is_vsg_stop = false;
static volatile bool is_vsg_start = false;
static VsaParameter vsaParam;
static VsgParameter vsgParam;
static HANDLE handle_VSG = INVALID_HANDLE_VALUE;
static HANDLE handle_VSA = INVALID_HANDLE_VALUE;

char TesterIpaddress[32] = {0};
char VSGWavePath[256] = {0};
static bool is_enable_vsg = false;
const int MAX_DEMO_LOOP = 10;

double g_wifi_11ag_freq = 2412 * 1e6;
double g_bt_freq = 2480 * 1e6;
double g_external_gain = 0.0;
double g_continuewave_freq = 5500 * 1e6;


WT208_Tester c_tester;

// load the Itest library
void Init_Itest_library(void)
{
	WT_DLLInitialize();
}
// release the Itest library
void Realse_Itest_library(void)
{
	WT_DLLTerminate();
}
static int set_continue_wave_Autorange(bool isTrigger)
{
	int ret = 0;
	int rf_port = get_rf_vsa_config();
	ret = WT_SetExternalGain(connect_id, g_external_gain);
	printf("Set tester external cable loss %lf dB\n", g_external_gain);
	// vsaParam max_power = target dut power + (14~18dB)
	// if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get vsaParam max_power
	// here we don't know DUT Max power input, so just set max_power=0
	vsaParam.max_power = 0.0;
	//freq
	vsaParam.freq = g_wifi_11ag_freq;
	//WIFI standard
	vsaParam.demod = WT_DEMOD_CW;
	// rf port
	vsaParam.rfPort = WT_PORT_OFF + rf_port;
	//sample time
	vsaParam.smp_time = 10000;
	//timeout waiting
	vsaParam.TimeoutWaiting = 5;
	//trigger pretime
	vsaParam.trig_pretime = 20;
	//trigger level
	vsaParam.trig_level = -28.0;
	//trigger type
	vsaParam.trig_type = (isTrigger == true ? WT_TRIG_TYPE_IF : WT_TRIG_TYPE_FREE_RUN);
	//trigger timeout
	vsaParam.trig_timeout = 2;

	printf("Set auto range VSA parm:\n");
	printf("\tAuto range max_power:%lf dBm\n", vsaParam.max_power);
	printf("\tTrigger pretime:%lf us\n", vsaParam.trig_pretime);
	printf("\tTrigger level:%lf dBm\n", vsaParam.trig_level);
	printf("\tTrigger timeout:%lf s\n", vsaParam.trig_timeout);
	printf("\tFreq:%lf MHz\n", vsaParam.freq / 1e6);
	printf("\tVSA port:RF%d\n", rf_port);
	printf("\tSample time:%lf us = %lf ms\n", vsaParam.smp_time, (vsaParam.smp_time / 1000));
	printf("We are going to run VSA Auto range\n");
	ret = WT_SetVSA_AutoRange(connect_id, &vsaParam);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("After run VSA Auto range:\n\tMax power level:%lf dBm\n\tTrigger level:%lf dBm\n", vsaParam.max_power, vsaParam.trig_level);
	}
	else
	{
		printf("%s Fail\n", __FUNCTION__);
	}
	return ret;
}

static int set_vsa_Autorange()
{
	int ret = 0;
	int rf_port = get_rf_vsa_config();
	ret = WT_SetExternalGain(connect_id, g_external_gain);
	printf("Set tester external cable loss %lf dB\n", g_external_gain);
	// vsaParam max_power = target dut power + (14~18dB)
	// if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get vsaParam max_power
	// here we don't know DUT Max power input, so just set max_power=0
	vsaParam.max_power = 0.0;
	//freq
	vsaParam.freq = g_wifi_11ag_freq;
	//WIFI standard
	vsaParam.demod = WT_DEMOD_11AG;
	// rf port
	vsaParam.rfPort = WT_PORT_OFF + rf_port;
	//sample time
	vsaParam.smp_time = 2000;
	//timeout waiting
	vsaParam.TimeoutWaiting = 5;
	//trigger pretime
	vsaParam.trig_pretime = 20;
	//trigger level
	vsaParam.trig_level = -28.0;
	//trigger type
	vsaParam.trig_type = WT_TRIG_TYPE_IF;
	//trigger timeout
	vsaParam.trig_timeout = 2;

	printf("Set auto range VSA parm:\n");
	printf("\tAuto range max_power:%lf dBm\n", vsaParam.max_power);
	printf("\tTrigger pretime:%lf us\n", vsaParam.trig_pretime);
	printf("\tTrigger level:%lf dBm\n", vsaParam.trig_level);
	printf("\tTrigger timeout:%lf s\n", vsaParam.trig_timeout);
	printf("\tFreq:%lf MHz\n", vsaParam.freq / 1e6);
	printf("\tVSA port:RF%d\n", rf_port);
	printf("\tSample time:%lf ms\n", vsaParam.smp_time);
	printf("We are going to run VSA Auto range\n");
	ret = WT_SetVSA_AutoRange(connect_id, &vsaParam);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("After run VSA Auto range:\n\tMax power level:%lf dBm\n\tTrigger level:%lf dBm\n", vsaParam.max_power, vsaParam.trig_level);
	}
	else
	{
		printf("%s Fail\n", __FUNCTION__);
	}
	return ret;
}

static int set_vsa_parm()
{
	int ret = WT_SetExternalGain(connect_id, g_external_gain);
	int rf_port = get_rf_vsa_config();
	printf("Set tester external cable loss %lf dB\n", g_external_gain);
	// vsaParam max_power = target dut power + (14~18dB)
	// if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get vsaParam max_power
	// here we don't know DUT Max power input, so just set max_power=0
	vsaParam.max_power = 0.0;
	//freq
	vsaParam.freq = g_wifi_11ag_freq;
	//WIFI standard
	vsaParam.demod = WT_DEMOD_11AG;
	// rf port
	vsaParam.rfPort = WT_PORT_OFF + rf_port;
	//sample time
	vsaParam.smp_time = 2000;
	//timeout waiting
	vsaParam.TimeoutWaiting = 5;
	//trigger type
	vsaParam.trig_type = WT_TRIG_TYPE_IF;
	//trigger timeout
	vsaParam.trig_timeout = 2;
	//trigger level
	vsaParam.trig_level = -31;

	printf("Set vsa parm:\n");
	printf("\tMax_power:%lf dBm\n", vsaParam.max_power);
	printf("\tFreq:%lf MHz\n", vsaParam.freq);
	printf("\tTrigger level:%lf dBm\n", vsaParam.trig_level);
	printf("\tVSA port:RF%d\n", rf_port);
	printf("\tSample time:%lf ms\n", vsaParam.smp_time);
	printf("\tWIFI standard 802.11ag\n");

	ret = WT_SetVSA(connect_id, &vsaParam);
	printf("%s ret: %d\n", __FUNCTION__, ret);
	return ret;
}
/**
 * @brief Synchronise VSG
 * 
 * @return int 
 */
static int syn_vsg()
{
	int ret = WT_ERR_CODE_OK;
	int status = 0;
	char wave_path[256] = {0};
	strcpy(wave_path, VSGWavePath);
	//strcat(wave_path,"\\wave\\120M\\54 Mbps(OFDM).csv");
	vsgParam.waveType = SIG_USERFILE;
	vsgParam.freq = g_wifi_11ag_freq;
	vsgParam.power = -10.0;
	vsgParam.repeat = 1000;
	vsgParam.rfPort =  get_rf_vsg_config();
	vsgParam.wave = wave_path;
	vsgParam.wave_gap = 50;
	printf("Set syn vsg parm\n");
	printf("\tVSG path:%s\n", vsgParam.wave);
	printf("\tVSG freq:%lf MHz\n", vsgParam.freq / 1e6);
	printf("\tVSG power:%lf dBm\n", vsgParam.power);
	printf("\tVSG repeat:%d\n", vsgParam.repeat);
	printf("\tVSG rf port:RF%d\n", get_rf_vsg_config());
	printf("\tVSG wave gap:%lf us\n", vsgParam.wave_gap);

	ret = WT_SetVSG(connect_id, &vsgParam);
	if(WT_ERR_CODE_OK == ret)
	{
		is_vsg_start = true;
		ret = WT_StartVSG(connect_id);//synchronize Start VSG
	}
	else
	{
		printf("%s set VSG fail\n", __FUNCTION__);
	}
	return ret;
}

static int asyn_vsg()
{
	int ret = WT_ERR_CODE_OK;
	int status = 0;
	int rf_port = get_rf_vsg_config();
	char wave_path[256] = {0};
	strcpy(wave_path, VSGWavePath);
	//strcat(wave_path,"\\wave\\120M\\54 Mbps(OFDM).csv");
	vsgParam.waveType = SIG_USERFILE;
	vsgParam.freq = g_wifi_11ag_freq;
	vsgParam.power = -10.0;
	vsgParam.repeat = 1000;
	vsgParam.rfPort = WT_PORT_OFF + rf_port;
	vsgParam.wave = wave_path;
	vsgParam.wave_gap = 50;

	printf("Set asyn vsg parm\n");
	printf("\tVSG path:%s\n", vsgParam.wave);
	printf("\tVSG freq:%lf MHz\n", vsgParam.freq / 1e6);
	printf("\tVSG power:%lf\n", vsgParam.power);
	printf("\tVSG repeat:%d\n", vsgParam.repeat);
	printf("\tVSG rf port:RF%d\n", rf_port);
	printf("\tVSG wave gap:%lf us\n", vsgParam.wave_gap);

	ret = WT_SetVSG(connect_id, &vsgParam);
	if(WT_ERR_CODE_OK == ret)
	{
		ret = WT_AsynStartVSG(connect_id);
		if(WT_ERR_CODE_OK == ret)
		{
			is_vsg_start = true;
		}
		while(1)
		{
			ret = WT_GetVSGCurrentState(connect_id, &status);
			if(WT_ERR_CODE_OK == ret &&
				WT_VSG_STATE_RUNNING != status && WT_VSG_STATE_WAITING != status)
			{
				break;
			}
			if(WT_ERR_CODE_OK != ret)
			{
				break;
			}
			Sleep(100);
		}
	}
	else
	{
		printf("%s set VSG fail\n", __FUNCTION__);
	}
	is_vsg_start = false;
	return ret;
}

static DWORD WINAPI  ThreadVSAFun(LPVOID pM)
{
	int vsa_loop = 0;
	int ret = set_vsa_Autorange();
	//ret = set_vsa_parm();

	while(!is_vsg_stop && (vsa_loop++) < MAX_DEMO_LOOP)
	{

		ret = WT_DataCapture(connect_id);
		if(WT_ERR_CODE_OK != ret)
		{
			printf("WT_DataCapture ret: %d\n", ret);
			break;
		}
		printf("\n========Loop %d=============\n", vsa_loop);
		print_power(connect_id);
		print_evm(connect_id);
		print_freq_err(connect_id);
		Sleep(100);
	}
	ret = WT_StopDataCapture(connect_id);
	return ret;
}

static DWORD WINAPI  ThreadVSGFun(LPVOID pM)
{
	int cycle_sum = 0;
	int ret = 0;
	is_vsg_stop = false;

	while(cycle_sum++ < MAX_DEMO_LOOP)
	{
		printf("\n=======Loop %d==========\n", cycle_sum);
		ret = syn_vsg();
		if(WT_ERR_CODE_OK != ret)
		{
			break;
		}
		WT_StopVSG(connect_id);
		is_vsg_start = false;

		ret = asyn_vsg();
		if(WT_ERR_CODE_OK != ret)
		{
			break;
		}
		WT_StopVSG(connect_id);
	}
	WT_StopVSG(connect_id);
	is_vsg_stop = true;
	is_vsg_start = false;
	return ret;
}

static DWORD WINAPI  ThreadLoopVSGFun(LPVOID pM)
{
	int ret = WT_ERR_CODE_OK;
	int status = 0;
	int rf_port = get_rf_vsg_config();
	char wave_path[256] = {0};
	strcpy(wave_path, VSGWavePath);
	//strcat(wave_path,"\\wave\\120M\\54 Mbps(OFDM).csv");
	vsgParam.waveType = SIG_USERFILE;
	vsgParam.freq = g_wifi_11ag_freq;
	vsgParam.power = -10.0;
	vsgParam.repeat = 0;
	vsgParam.rfPort = WT_PORT_OFF + rf_port;
	vsgParam.wave = wave_path;
	vsgParam.wave_gap = 50;

	printf("Set infinity asyn vsg parm\n");
	printf("\tVSG path:%s\n", vsgParam.wave);
	printf("\tVSG freq:%lf MHz\n", vsgParam.freq);
	printf("\tVSG power:%lf\n", vsgParam.power);
	printf("\tVSG repeat:%d\n", vsgParam.repeat);
	printf("\tVSG rf port:RF%d\n", rf_port);
	printf("\tVSG wave gap:%lf\n", vsgParam.wave_gap);
	is_vsg_stop = false;
	ret = WT_SetVSG(connect_id, &vsgParam);
	if(WT_ERR_CODE_OK == ret)
	{
		ret = WT_AsynStartVSG(connect_id);
		if(WT_ERR_CODE_OK == ret)
		{
			is_vsg_start = true;
		}
		while(!is_vsg_stop)
		{
			ret = WT_GetVSGCurrentState(connect_id, &status);
			if(WT_ERR_CODE_OK == ret &&
				WT_VSG_STATE_RUNNING != status && WT_VSG_STATE_WAITING != status)
			{
				break;
			}
			if(WT_ERR_CODE_OK != ret)
			{
				break;
			}
			Sleep(100);
		}
		WT_StopVSG(connect_id);
	}
	else
	{
		printf("%s set VSG fail\n", __FUNCTION__);
	}
	is_vsg_start = false;
	return ret;
}

static DWORD WINAPI  ThreadLoopVSAFun(LPVOID pM)
{
	SYSTEMTIME sys;
	int ret = set_vsa_Autorange();
	//ret = set_vsa_parm();

	while(!is_vsg_stop)
	{
		GetLocalTime(&sys);
		printf("##########[%02d:%02d:%02d.%03d]###########\n", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
		ret = WT_DataCapture(connect_id);
		if(WT_ERR_CODE_OK != ret)
		{
			printf("WT_DataCapture ret: %d\n", ret);
			break;
		}
		print_power(connect_id);
		print_evm(connect_id);
		print_freq_err(connect_id);
		Sleep(1000);
	}
	ret = WT_StopDataCapture(connect_id);
	return ret;
}

static DWORD WINAPI  ThreadVSAContinueWaveFun(LPVOID pM)
{
	int vsa_loop = 0;
	int ret = set_continue_wave_Autorange(false);
	int data_capture_fail_count = 0;
	while((vsa_loop++) < MAX_DEMO_LOOP)
	{

		ret = WT_DataCapture(connect_id);
		if(WT_ERR_CODE_OK != ret)
		{
			printf("WT_DataCapture ret: %d\n", ret);
			if(0 == data_capture_fail_count)
			{
				data_capture_fail_count++;
				set_continue_wave_Autorange(true);
				continue;
			}
			break;
		}
		printf("\n========Loop %d=============\n", vsa_loop);
		print_continuewave_power_and_freq_err(connect_id);

		Sleep(100);
	}
	ret = WT_StopDataCapture(connect_id);
	return ret;
}
static bool demo_vsg()
{
	printf("Demo WIFI VSG?(Y/N)\n>");
	if(get_Choose())
	{
		if(false == demo_connect_tester())
		{
			return false;
		}
		printf("Put 54 Mbps(OFDM).csv waveform in current path ?(Y/N)\n>");
		if(get_Choose())
		{
			do
			{
				is_enable_vsg = true;
				get_exe_path(VSGWavePath);
				strcat(VSGWavePath, "\\54 Mbps(OFDM).csv");
				if(false == find_file(VSGWavePath))
				{
					printf("%s not find\n", VSGWavePath);
					break;
				}
				printf("\n============================\n");
				printf("Demo WIFI VSG setting is:\n");
				printf("\tPower:-10dBm\n");
				printf("\tFreq:%lf MHz\n", g_wifi_11ag_freq / 1e6);
				printf("\tCable loss:%lf dB\n", g_external_gain);
				printf("\tStandard 802.11ag OFDM 54M\n");
				printf("\tWaveform is %s\n", VSGWavePath);
				printf("\n============================\n");
				handle_VSG = CreateThread(NULL, 0, ThreadVSGFun, NULL, 0, NULL);
				WaitForSingleObject(handle_VSG, INFINITE);
				CloseHandle(handle_VSG);
			}
			while(0);

		}

		WT_DisConnect(connect_id);
	}
	return true;
}

static bool demo_wifi_vsa()
{
	printf("Demo WIFI VSA (y/n)\n>");
	if(get_Choose())
	{
		if(false == demo_connect_tester())
		{
			return false;
		}
		printf("\n============================\n");
		printf("Please start WIFI VSG first\n");
		printf("Demo WIFI VSG setting is:\n");
		printf("\tPower:-10dBm\n");
		printf("\tFreq:%lf MHz\n", g_wifi_11ag_freq / 1e6);
		printf("\tStandard 802.11ag OFDM 54M\n");
		printf("\tCable loss:%lf dB\n", g_external_gain);
		printf("\n============================\n");
		printf("VSG is already ? (Y/N)\n>");
		if(get_Choose())
		{
			printf("Please waiting awhile...\n");
			handle_VSA = CreateThread(NULL, 0, ThreadVSAFun, NULL, 0, NULL);
			WaitForSingleObject(handle_VSA, INFINITE);
			CloseHandle(handle_VSA);
		}
		WT_DisConnect(connect_id);
	}

	return true;
}

static bool demo_vsg_vsa_loop()
{
	printf("Demo VSG VSA Loop Test? (Y/N)\n>");
	if(get_Choose())
	{
		if(false == demo_connect_tester())
		{
			return false;
		}
		printf("Demo VSG VSA Loop Test\n");
		printf("\tConnect RF1 to RF2 \n");
		printf("\t54 Mbps(OFDM).csv waveform in current path\n");
		printf("VSG setting is:\n");
		printf("\tPower:-10dBm\n");
		printf("\tFreq:%lf MHz\n", g_wifi_11ag_freq / 1e6);
		printf("\tStandard 802.11ag 54M\n");
		printf("\tCable loss:%lf dB\n", g_external_gain);
		printf("Start now? (Y/N)\n>");
		if(get_Choose())
		{
			do
			{
				get_exe_path(VSGWavePath);
				strcat(VSGWavePath, "\\54 Mbps(OFDM).csv");
				if(false == find_file(VSGWavePath))
				{
					printf("%s not find\n", VSGWavePath);
					break;;
				}

				printf("Demo VSG VSA Loop, waiting for awhile...\n");
				printf("Stop test, enter any key continue\n");

				handle_VSG = CreateThread(NULL, 0, ThreadLoopVSGFun, NULL, 0, NULL);
				Sleep(100);

				handle_VSA = CreateThread(NULL, 0, ThreadLoopVSAFun, NULL, 0, NULL);
				get_Choose();

				is_vsg_stop = true;

				WaitForSingleObject(handle_VSA, INFINITE);
				WaitForSingleObject(handle_VSG, INFINITE);
				CloseHandle(handle_VSA);
				CloseHandle(handle_VSG);

			}
			while(0);
		}
		WT_DisConnect(connect_id);
		printf("\n#############################\n");
		printf("Stop loop test now!!!\n");
	}

	return true;
}

static bool demo_multi_Vsg()
{
	bool is_wifi_vsg = true;
	const int vsg_size = get_thread_config();
	if(!TesterIpaddress[0])
	{
		printf("Please input tester ip address\n>");
		get_input(TesterIpaddress);
	}
	printf("Please choice [1~2]\n");
	printf("\t1: Demo WT-208 ASYN VSG\n");
	printf("\t2: Demo WT-208 BT VSG\n");
	printf(">");

	switch(get_index())
	{
		case 2:
			printf("Put %s waveform in current path ?(Y/N)\n>", BT_VSG_WAVEFORM);
			if(!get_Choose())
			{
				return false;
			}
			is_wifi_vsg = false;
			break;
		default:
			printf("Put %s waveform in current path ?(Y/N)\n>", WT208_ASYN_VSG_WAVEFORM);
			if(!get_Choose())
			{
				return false;
			}
			is_wifi_vsg = true;
			break;
	}
	wt208_vsg **vsg_demo = new wt208_vsg *[vsg_size];
	for(int i = 0; i < vsg_size; i++)
	{
		vsg_demo[i] = new wt208_vsg(is_wifi_vsg);
		vsg_demo[i]->exec();
		Sleep(get_connect_delta_config());
	}
	for(int i = 0; i < vsg_size; i++)
	{
		delete vsg_demo[i];
	}
	delete []vsg_demo;

	return true;
}

static bool demo_multi_Vsa()
{

	const int vsa_size = get_thread_config();
	if(!TesterIpaddress[0])
	{
		printf("Please input tester ip address\n>");
		get_input(TesterIpaddress);
	}
	printf("\n============================\n");
	printf("Demo WT-208 WIFI VSA\n");
	printf("Demo WIFI VSG setting is:\n");
	printf("\tPower:-10dBm\n");
	printf("\tFreq:%lf MHz\n", g_wifi_11ag_freq / 1e6);
	printf("\tStandard 802.11ag OFDM 54M\n");
	printf("\tCable loss:%lf dB\n", g_external_gain);
	printf("\n============================\n");
	printf("VSG is already ? (Y/N)\n>");
	if(get_Choose())
	{
		wt208_vsa **vsa_demo = new wt208_vsa *[vsa_size];

		for(int i = 0; i < vsa_size; i++)
		{
			vsa_demo[i] = new wt208_vsa();
			vsa_demo[i]->set_vsa_port(i + 1);
			vsa_demo[i]->exec();
			Sleep(get_connect_delta_config());
		}
		for(int i = 0; i < vsa_size; i++)
		{
			delete vsa_demo[i];
		}
		delete []vsa_demo;
	}
	return true;
}

static bool demo_continuewave()
{
	printf("Demo continue wave VSA VSG loop Test?(y/n)\n>");
	if(get_Choose())
	{
		if(false == demo_connect_tester())
		{
			return false;
		}
		printf("\n============================\n");
		printf("Start continue wave test? (Y/N)\n>");
		if(get_Choose())
		{
			get_exe_path(VSGWavePath);
			strcat(VSGWavePath, "\\Sin1MHz.bwv");
			if(false == find_file(VSGWavePath))
			{
				printf("%s not find\n", VSGWavePath);
			}
			printf("Stop test, enter any key continue\n");
			printf("Please waiting awhile ....\n");

			handle_VSG = CreateThread(NULL, 0, ThreadLoopVSGFun, NULL, 0, NULL);
			Sleep(100);

			handle_VSA = CreateThread(NULL, 0, ThreadVSAContinueWaveFun, NULL, 0, NULL);

			get_Choose();

			is_vsg_stop = true;

			WaitForSingleObject(handle_VSA, INFINITE);
			WaitForSingleObject(handle_VSG, INFINITE);
			CloseHandle(handle_VSA);
			CloseHandle(handle_VSG);
		}
		WT_DisConnect(connect_id);
		printf("\n#############################\n");
		printf("Continue wave test will end!\n");
	}

	return true;
}

bool demo_Multi_connect_tester(int *id)
{
	int ret = 0;
	int tmp_id = -1;
	if(!TesterIpaddress[0])
	{
		printf("Please input tester ip address\n>");
		get_input(TesterIpaddress);
	}

	ret = WT_Connect(TesterIpaddress, &tmp_id);

	printf("Connect[%d] %s %s\n", tmp_id, TesterIpaddress, (ret == WT_ERR_CODE_OK ? "OK" : "Fail"));
	if(WT_ERR_CODE_OK == ret)
	{
		if(id)
		{
			*id = tmp_id;
			connect_id = tmp_id;
		}
		ret = WT_GetDefaultParameter(&vsaParam, &vsgParam);
	}
	return ret == WT_ERR_CODE_OK;
}

bool demo_connect_tester(int *id)
{
	int ret = 0;
	printf("Please input tester ip address\n>");
	get_input(TesterIpaddress);

	ret = WT_ForceConnect(TesterIpaddress, &connect_id);

	printf("Connect %s %s\n", TesterIpaddress, (ret == WT_ERR_CODE_OK ? "OK" : "Fail"));
	if(WT_ERR_CODE_OK == ret)
	{
		ret = WT_GetDefaultParameter(&vsaParam, &vsgParam);
	}
	if(id)
	{
		*id = connect_id;
	}


	return ret == WT_ERR_CODE_OK;
}

static void init_all()
{
	memset(&vsgParam, 0, sizeof(VsgParameter));
	memset(&vsaParam, 0, sizeof(VsaParameter));
	connect_id = 0;
	is_vsg_stop = false;
	is_vsg_start = false;

	handle_VSG = INVALID_HANDLE_VALUE;
	handle_VSA = INVALID_HANDLE_VALUE;

	memset(TesterIpaddress, 0, sizeof(TesterIpaddress));
	memset(VSGWavePath, 0, sizeof(VSGWavePath));
	is_enable_vsg = false;

}

static void help_usage()
{
	printf("Config file name:%s\n", RF_CONFIG_FILE);
	printf("\tConfig VSA RF port you should set \"%s=[1-4]\"\n", VSA_RF_PORT_TOKEN);
	printf("\tConfig VSG RF port you should set \"%s=[1-4]\"\n", VSG_RF_PORT_TOKEN);
	printf("\tConfig WT208 user count you should set \"%s=[1-4]\" \n", USER_COUNT_TOKEN);
	printf("\tConfig WT208 user connect time delta set \"%s=[0-1000]\"\n", CONNECT_TIME_DELTA);

	printf("Please choice [1~6]\n");
	printf("\t1: Demo VSG\n");
	printf("\t2: Demo WIFI VSA\n");
	printf("\t3: Demo BT VSA\n");
	printf("\t4: Demo WIFI VSG VSA Loop Test\n");
	printf("\t5: Demo waveform download\n");
	printf("\t6: Demo 802.11b VSA\n");
	printf("\t7: Demo WT208 Mulit VSG\n");
	printf("\t8: Demo WT208 Mulit VSA\n");
	printf("\t9: Demo ZigBee\n");
	printf("\tA: Demo get continue wave power and freq_offset\n");
	printf("\tB: Demo BT ACP\n");
	printf("\tq: Exit\n");
	printf(">");
}


int main(int argc, _TCHAR *argv[])
{
	DWORD starttime, endtime;
	int ret = 0;
	char buf[128] = {0};
	//printf(argv[1]);
	printf("\n");
	get_exe_path(NULL);
	starttime = GetTickCount();

/* USER CODE BEGIN PTD */
	//ciu_unit_test();//CIU test
	//c_tester.tester_unit_test();
	// harmwee_test(argv[1]);//Harmwee test
	// for(ret=0;ret<10;ret++)
	{
		// harwwee_test_by_VSGVSA(); // harmwee test by VSG and VSA
		nic_unit_test();
	}
	

/* USER CODE END PTD */
	
	endtime = GetTickCount();
	printf("the test duration time is :%d ms\n", endtime - starttime);
	return ret;
}


// int _tmain1(int argc, _TCHAR *argv[])
// {
// 	DWORD starttime,endtime;
// 	int ret = 0;
// 	char buf[128] = {0};
// 	get_exe_path(NULL);
//     starttime = GetTickCount();
// // 	ret = WT_SetVSG(m_connect_id, &m_VsgParameter);
// // 	endtime = GetTickCount();
// // 	fprintf(stderr, "[%d]WT_SetVSG:%d ms\n", m_connect_id, endtime - starttime);
// 	WT_DLLInitialize();
// 	// string xx="ipaddress = 192.168.101.10 //ipaddress";
// 	// regex yy("*+\=\d+\.\d+\.\d+\.\d+");

// 	// dutSerial.setPort("COM1");
//     // dutSerial.open();
//     // dutSerial.setTimeout(serial::Timeout::max(), 100, 0, 100, 0); // set timeout is 100ms
// 	// if(dutSerial.isOpen())
// 	// {
// 	// 	printf("the dut serial port is open");
// 	// }
// 	// else
// 	// {
// 	// 	printf("the dut serial port is not open");
// 	// }
// 	ciu_unit_test();
// 	//c_tester.tester_unit_test();
// 	endtime = GetTickCount();
//     printf("the test duration time is :%d ms\n", endtime - starttime);
// 	do
// 	{
// 		;
// 	} while (1);
	
// 	// Final_Verify_Flow();
// 	// printf("my name is weijunfeng\n");
// 	// printf("this is my name\n");
// 	// if(get_Choose())
// 	// {
// 	// 	printf("you choose yes");
// 	// }
// 	// else
// 	// {
// 	// 	printf("you choose no");
// 	// }

// 	ret=get_connect_delta_config();
// 	printf("weijunfeng is %d\n",ret);
	
// 	while(1)
// 	{
// 		help_usage();
// 		fflush(stdin);//fresh the standard input buffer
// 		fgets(buf, 2, stdin);
// 		fflush(stdin);
// 		if('q' == buf[0] || 'Q' == buf[0])
// 		{
// 			break;
// 		}

// 		init_all();

// 		switch(toupper(buf[0]))
// 		{
// 			case '1':
// 			{
// 				demo_vsg();
// 				break;
// 			}
// 			case '2':
// 			{
// 				demo_wifi_vsa();
// 				break;
// 			}
// 			case '3':
// 			{
// 				bt_demo demo_bt;
// 				demo_bt.demo_bt_vsa();
// 				break;
// 			}
// 			case '4':
// 			{
// 				demo_vsg_vsa_loop();
// 				break;
// 			}
// 			case '5':
// 			{
// 				download_waveform waveDownload;
// 				waveDownload.exec();
// 				break;
// 			}
// 			case '6':
// 			{
// 				wifi_11b_demo demo_11b;
// 				demo_11b.exec();
// 				break;
// 			}
// 			case '7':
// 			{
// 				demo_multi_Vsg();
// 				break;
// 			}
// 			case '8':
// 			{
// 				demo_multi_Vsa();
// 				break;
// 			}
// 			case '9':
// 			{
// 				zigbee_demo demo;
// 				demo.exec();
// 				break;
// 			}
// 			case 'A':
// 			{
// 				demo_continuewave();
// 				break;
// 			}
// 			case 'B':
// 			{
// 				bt_demo demo_acp;
// 				demo_acp.demo_bt_acp();
// 				break;
// 			}
// 			default:
// 				printf("Unknown input\n");
// 				break;
// 		}
// 	}
// 	WT_DLLTerminate();

// 	system("PAUSE");
// 	return ret;
// }

