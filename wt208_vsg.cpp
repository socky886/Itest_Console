#include "StdAfx.h"
#include "wt208_vsg.h"
#include <iostream>
#include "common.h"
#include <assert.h>
#include <Windows.h>
#include "Mutex.h"
#include "bt_demo.h"

#define VSG_DEMO_LOOP       100


wt208_vsg::wt208_vsg(bool is_bt)
	: m_handle_VSG(INVALID_HANDLE_VALUE)
	, m_handle_VSA(INVALID_HANDLE_VALUE)
	, m_external_gain(0.0)
	, m_dut_peek_power(-8.0)
	, m_freq(2412 * 1e6)
	, m_repeat(10000)
	, m_vsg_runing(false)
	, m_connect_id(-1)
	, is_wifi_vsg(is_bt)
{
	memset(&m_VsaParameter, 0, sizeof(VsaParameter));
	memset(&m_VsgParameter, 0, sizeof(VsgParameter));
}


wt208_vsg::~wt208_vsg(void)
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

bool wt208_vsg::init()
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

	fprintf(stderr, "[%d]set tester external cable loss %lf dB\n", m_connect_id, m_external_gain);

	return WT_ERR_CODE_OK == ret;
}

int wt208_vsg::asyn_vsg()
{
	int ret = WT_ERR_CODE_OK;
	int status = 0;
	char wave_path[256] = {0};
	char rf_buf[32] = {0};
	DWORD starttime = 0, endtime = 0;
	int rf_port = 1;
	if(false == this->init())
	{
		fprintf(stderr, "[%d]init fail\n", m_connect_id);
		return -1;
	}
	rf_port = get_rf_vsg_config();
	get_exe_path(wave_path);
	strcat(wave_path, "\\");
	strcat(wave_path, WT208_ASYN_VSG_WAVEFORM);
	m_VsgParameter.waveType = SIG_USERFILE;
	m_VsgParameter.freq = m_freq;
	m_VsgParameter.power = m_dut_peek_power;
	m_VsgParameter.repeat = m_repeat;
	m_VsgParameter.rfPort = WT_PORT_OFF + rf_port;
	m_VsgParameter.wave = wave_path;
	m_VsgParameter.wave_gap = 50;
	sprintf(rf_buf, "RF%d", rf_port);

	starttime = GetTickCount();
	ret = WT_SetVSG(m_connect_id, &m_VsgParameter);
	endtime = GetTickCount();
	fprintf(stderr, "[%d]WT_SetVSG:%d ms\n", m_connect_id, endtime - starttime);
#if 0
	if(WT_ERR_CODE_OK == ret)
	{
		fprintf(stderr, "\n============================\n");
		fprintf(stderr, "[%d]demo BT VSG set SYNC vsg parm\n", m_connect_id);
		fprintf(stderr, "\tvsg path:%s\n", m_VsgParameter.wave);
		fprintf(stderr, "\tvsg freq:%lf MHz\n", m_VsgParameter.freq / 1e6);
		fprintf(stderr, "\tvsg power:%lf\n", m_VsgParameter.power);
		fprintf(stderr, "\tvsg repeat:%d\n", m_VsgParameter.repeat);
		fprintf(stderr, "\tvsg rf port:%s\n", rf_buf);
		fprintf(stderr, "\tvsg wave gap:%lf us\n", m_VsgParameter.wave_gap);
		fprintf(stderr, "\tbt packet type:3-DH3\n");
		fprintf(stderr, "\tbt datarate:3M\n");
		fprintf(stderr, "\tcable loss:%lf dB\n", m_external_gain);
		fprintf(stderr, "\n============================\n");

		starttime = GetTickCount();
		m_vsg_runing = true;
		ret = WT_StartVSG(m_connect_id);
		m_vsg_runing = false;
		endtime = GetTickCount();
		fprintf(stderr, "[%d]WT_StartVSG:%d ms\n", m_connect_id, endtime - starttime);
	}
	else
	{
		fprintf(stderr, "[%d]%s set VSG fail\n", m_connect_id, __FUNCTION__);
	}

#else

	if(WT_ERR_CODE_OK == ret)
	{
		fprintf(stderr, "\n============================\n");
		fprintf(stderr, "[%d]Demo BT VSG set ASYNC vsg parm\n", m_connect_id);
		fprintf(stderr, "\tVSG path:%s\n", m_VsgParameter.wave);
		fprintf(stderr, "\tVSG freq:%lf MHz\n", m_VsgParameter.freq / 1e6);
		fprintf(stderr, "\tVSG power:%lf\n", m_VsgParameter.power);
		fprintf(stderr, "\tVSG repeat:%d\n", m_VsgParameter.repeat);
		fprintf(stderr, "\tVSG rf port:%s\n", rf_buf);
		fprintf(stderr, "\tVSG wave gap:%lf us\n", m_VsgParameter.wave_gap);
		fprintf(stderr, "\tBT packet type:3-DH3\n");
		fprintf(stderr, "\tBT datarate:3M\n");
		fprintf(stderr, "\tCable loss:%lf dB\n", m_external_gain);
		fprintf(stderr, "\n============================\n");
		starttime = GetTickCount();
		m_vsg_runing = true;
		ret = WT_AsynStartVSG(m_connect_id);
		endtime = GetTickCount();
		fprintf(stderr, "[%d]WT_AsynStartVSG:%d ms\n", m_connect_id, endtime - starttime);
		while(WT_ERR_CODE_OK == ret)
		{
			ret = WT_GetVSGCurrentState(m_connect_id, &status);
			if(WT_ERR_CODE_OK == ret &&
				WT_VSG_STATE_RUNNING != status && WT_VSG_STATE_WAITING != status)
			{
				break;
			}
			if(WT_ERR_CODE_OK != ret)
			{
				break;
			}
			Sleep(10);
		}
		m_vsg_runing = false;
	}
	else
	{
		printf("%s set VSG fail\n", __FUNCTION__);
	}
#endif
	return ret;
}

DWORD WINAPI wt208_vsg::ThreadVSGFun(LPVOID pM)
{
	wt208_vsg *pThis = (wt208_vsg *)(LPVOID)pM;
	if(pThis->is_wifi_vsg)
	{
		return pThis->asyn_vsg();
	}
	for(int i = 0; i < VSG_DEMO_LOOP; i++)
	{
		bt_demo demo_bt;
		demo_bt.demo_bt_vsg();
	}
	return 1;
}

bool wt208_vsg::exec()
{
	m_handle_VSG = CreateThread(NULL, 0, ThreadVSGFun, this, 0, NULL);
	Sleep(100);
	return true;
}
