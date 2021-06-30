#include "StdAfx.h"
#include "wt208_vsa.h"
#include <iostream>
#include "common.h"
#include <assert.h>
#include <Windows.h>
#include "Mutex.h"
#include "bt_demo.h"

static volatile unsigned int g_print_rc = 0;

void print_debug_info(const char *buf)
{
	ENTER_LOCK(g_print_rc);
	printf("%s", buf);
	EXIT_LOCK(g_print_rc);
}

wt208_vsa::wt208_vsa(void)
	: m_handle_VSG(INVALID_HANDLE_VALUE)
	, m_handle_VSA(INVALID_HANDLE_VALUE)
	, m_external_gain(0.0)
	, m_dut_peek_power(-8.0)
	, m_freq(2412 * 1e6)
	, m_repeat(10000)
	, m_vsg_runing(false)
	, m_connect_id(-1)
	, m_port(-1)
{
	memset(&m_VsaParameter, 0, sizeof(VsaParameter));
	memset(&m_VsgParameter, 0, sizeof(VsgParameter));
}


wt208_vsa::~wt208_vsa(void)
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
	if(m_connect_id != -1)
	{
		WT_DisConnect(m_connect_id);
		printf("[%d] disconnect tester\n", m_connect_id);
	}
}

void wt208_vsa::set_vsa_port(int port)
{
	m_port = port;
}

bool wt208_vsa::init()
{
	int ret = WT_ERR_CODE_OK;

	if(false == demo_Multi_connect_tester(&m_connect_id))
	{
		return false;
	}

	ret = WT_GetDefaultParameter(&m_VsaParameter, &m_VsgParameter);
	assert(WT_ERR_CODE_OK == ret);

	ret = WT_SetExternalGain(m_connect_id, m_external_gain);
	assert(WT_ERR_CODE_OK == ret);

	//fprintf(stderr, "[%d]set tester external cable loss %lf dB\n",m_connect_id,m_external_gain);

	return WT_ERR_CODE_OK == ret;
}

int wt208_vsa::set_vsa_Autorange()
{
	int ret = 0;
	int rf_port = m_port;
	if(rf_port <= 0)
	{
		rf_port = get_rf_vsa_config();
	}

	ret = WT_SetExternalGain(m_connect_id, m_external_gain);
	// vsaParam max_power = target dut power + (14~18dB)
	// if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get vsaParam max_power
	// here we don't know DUT Max power input, so just set max_power=0
	m_VsaParameter.max_power = 0.0;
	//freq
	m_VsaParameter.freq = m_freq;
	//WIFI standard
	m_VsaParameter.demod = WT_DEMOD_11AG;
	// rf port
	m_VsaParameter.rfPort = WT_PORT_OFF + rf_port;
	//sample time
	m_VsaParameter.smp_time = 2000;
	//timeout waiting
	m_VsaParameter.TimeoutWaiting = 8;
	//trigger pretime
	m_VsaParameter.trig_pretime = 20;
	//trigger level
	m_VsaParameter.trig_level = -28.0;
	//trigger type
	m_VsaParameter.trig_type = WT_TRIG_TYPE_IF;
	//trigger timeout.
	//if trig_timeout > TimeoutWaiting maybe cause WT_SetVSA_AutoRange return WT_ERR_CODE_TIMEOUT
	m_VsaParameter.trig_timeout = 2;

	ret = WT_SetVSA_AutoRange(m_connect_id, &m_VsaParameter);

	char buf[256] = {0};
	sprintf(buf, "[%d]WT_SetVSA_AutoRange RF_Port:%d %s\n", m_connect_id, rf_port, (ret == WT_ERR_CODE_OK ? "OK" : "FAIL"));
	m_print_string += string(buf);

	return ret;
}

bool wt208_vsa::data_capture()
{
	int ret = 0;
	int vsa_loop = 0;
	char buf[2048] = {0};
	while(vsa_loop++ < 400)
	{
		sprintf(buf, "\n========Connect[%d] loop %d=============\n", m_connect_id, vsa_loop);
		m_print_string += string(buf);

		memset(buf, 0, sizeof(buf));
		ret = set_vsa_Autorange();
		if(WT_ERR_CODE_OK == ret)
		{
			memset(buf, 0, sizeof(buf));
			ret = WT_DataCapture(m_connect_id);
			sprintf(buf, "[%d]WT_DataCapture %s [ret: %d]\n", m_connect_id, (WT_ERR_CODE_OK == ret ? "OK" : "FAIL"), ret);
			m_print_string += string(buf);

			if(WT_ERR_CODE_OK == ret)
			{
				memset(buf, 0, sizeof(buf));
				print_power_2(m_connect_id, buf);
				m_print_string += string(buf);

				memset(buf, 0, sizeof(buf));
				print_evm_2(m_connect_id, buf);
				m_print_string += string(buf);

				memset(buf, 0, sizeof(buf));
				print_freq_err_2(m_connect_id, buf);
				m_print_string += string(buf);
			}
		}
		print_debug_info(m_print_string.c_str());
		m_print_string.clear();
		memset(buf, 0, sizeof(buf));

		Sleep(100);
	}
	ret = WT_StopDataCapture(m_connect_id);
	return WT_ERR_CODE_OK == ret;
}

DWORD WINAPI wt208_vsa::ThreadVSAFun(LPVOID pM)
{
	wt208_vsa *pThis = (wt208_vsa *)(LPVOID)pM;
	pThis->data_capture();
	return 1;
}

bool wt208_vsa::exec()
{
	if(init())
	{
		m_handle_VSA = CreateThread(NULL, 0, ThreadVSAFun, this, 0, NULL);
		return true;
	}
	return false;
}
