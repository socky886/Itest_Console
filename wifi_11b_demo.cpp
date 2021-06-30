#include "StdAfx.h"
#include "wifi_11b_demo.h"
#include <iostream>
#include "common.h"
#include <assert.h>

#define MAX_DEMO_LOOP 5

wifi_11b_demo::wifi_11b_demo()
	: m_handle_VSG(INVALID_HANDLE_VALUE)
	, m_handle_VSA(INVALID_HANDLE_VALUE)
	, m_connect_id(0)
	, m_external_gain(0.0)
	, m_dut_peek_power(-8.0)
	, m_freq(2412 * 1e6)
{
	memset(&m_VsaParameter, 0, sizeof(VsaParameter));
	memset(&m_VsgParameter, 0, sizeof(VsgParameter));
}

wifi_11b_demo::~wifi_11b_demo(void)
{
	if(m_handle_VSA != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_handle_VSA);
	}
	if(m_handle_VSG != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_handle_VSG);
	}
}

bool wifi_11b_demo::exec()
{
	std::cout << "\n=============================" << std::endl;
	std::cout << "802.11b demo please choose [1~4] " << std::endl;
	std::cout << "\t1:Demo default VSA parm" << std::endl;
	std::cout << "\t2:Demo VSA parm DC remove" << std::endl;
	std::cout << "\t3:Demo VSA parm equalizer taps" << std::endl;
	std::cout << "\t4:Demo VSA parm phase tracking" << std::endl;
	std::cout << "\n=============================" << std::endl;
	switch(get_index())
	{
		case 1:
			Vsa_Run_Default();
			break;
		case 2:
			Vsa_Run_DcRemove();
			break;
		case 3:
			Vsa_Run_EqTaps();
			break;
		case 4:
			Vsa_Run_CCKPhCorr();
			break;
	}
	return true;
}

bool wifi_11b_demo::init()
{
	if(false == demo_connect_tester(&m_connect_id))
	{
		return false;
	}

	int ret = 0;

	ret = WT_GetDefaultParameter(&m_VsaParameter, &m_VsgParameter);
	assert(WT_ERR_CODE_OK == ret);

	ret = WT_SetExternalGain(m_connect_id, m_external_gain);
	assert(WT_ERR_CODE_OK == ret);

	std::cout << "Set tester external cable loss " << m_external_gain << "dB" << std::endl;

	return true;
}

void wifi_11b_demo::finish()
{
	WT_DisConnect(m_connect_id);
}

bool wifi_11b_demo::run()
{
	int vsa_loop = 0;
	int ret = WT_ERR_CODE_GENERAL_ERROR;
	while(vsa_loop++ < MAX_DEMO_LOOP)
	{

		ret = WT_DataCapture(m_connect_id);
		if(WT_ERR_CODE_OK != ret)
		{
			printf("WT_DataCapture ret: %d\n", ret);
			break;
		}
		printf("\n========Loop %d=============\n", vsa_loop);
		print_power(m_connect_id);
		print_evm(m_connect_id);
		print_freq_err(m_connect_id);
		Sleep(100);
	}
	ret = WT_StopDataCapture(m_connect_id);
	return ret == WT_ERR_CODE_OK;
}

bool wifi_11b_demo::init_vsa_default()
{
	// m_VsaParameter max_power = DUT peek power + 3dBm
	// if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get m_VsaParameter max_power
	// here we don't know DUT Max power input, so just set max_power=0
	m_VsaParameter.max_power = m_dut_peek_power + 3;
	//freq
	m_VsaParameter.freq = m_freq;
	//WIFI standard
	m_VsaParameter.demod = WT_DEMOD_11B;
	// rf port
	m_VsaParameter.rfPort = WT_PORT_RF1;
	//sample time
	m_VsaParameter.smp_time = 2000;
	//timeout waiting
	m_VsaParameter.TimeoutWaiting = 5;
	//trigger pretime
	m_VsaParameter.trig_pretime = 20;
	//trigger level
	m_VsaParameter.trig_level = -28.0;
	//trigger type
	m_VsaParameter.trig_type = WT_TRIG_TYPE_IF;
	//trigger timeout
	m_VsaParameter.trig_timeout = 2;
	std::cout << "\n=============================" << std::endl;
	std::cout << "Set auto range VSA parm:" << std::endl;
	std::cout << "\tAuto range max_power:" << m_VsaParameter.max_power << "dBm" << std::endl;
	std::cout << "\tTrigger pretime:" << m_VsaParameter.trig_pretime << "us" << std::endl;
	std::cout << "\tTrigger level:" << m_VsaParameter.trig_level << "dBm" << std::endl;
	std::cout << "\tTrigger timeout:" << m_VsaParameter.trig_timeout << "s" << std::endl;
	std::cout << "\tFreq:" << m_VsaParameter.freq / 1e6 << "MHz" << std::endl;
	std::cout << "\tVSA port:RF1" << std::endl;
	std::cout << "\tSample time:" << m_VsaParameter.smp_time << "ms" << std::endl;
	std::cout << "Run Set VSA parm...." << std::endl;
	std::cout << "\n=============================" << std::endl;
	int ret = WT_SetVSA_AutoRange(m_connect_id, &m_VsaParameter);
	if(WT_ERR_CODE_OK == ret)
	{
		std::cout << "After run VSA Auto range:" << std::endl;
		std::cout << "\tMax power level:" << m_VsaParameter.max_power << "dBm" << std::endl;
		std::cout << "\tTrigger level:" << m_VsaParameter.trig_level << "dBm" << std::endl;
	}
	else
	{
		std::cout << __FUNCTION__ << "Fail" << std::endl;
	}
	return ret == WT_ERR_CODE_OK;
}

bool wifi_11b_demo::Vsa_Run_Default()
{
	std::cout << "Demo 802.11b default VSA " << std::endl;
	int ret = WT_ERR_CODE_GENERAL_ERROR;
	if(false == init())
	{
		goto toss;
	}
	if(false == init_vsa_default())
	{
		goto toss;
	}
	if(run())
	{
		ret = WT_ERR_CODE_OK;
	}
	std::cout << "==============================" << std::endl;
	std::cout << "Demo 802.11b default VSA end !!!" << std::endl;
	std::cout << "==============================" << std::endl;
toss:
	finish();
	return ret == WT_ERR_CODE_OK;

}

bool wifi_11b_demo::init_vsa_dc_remove(bool isEnable)
{
	// m_VsaParameter max_power = DUT peek power + 3dBm
	// if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get m_VsaParameter max_power
	// here we don't know DUT Max power input, so just set max_power=0
	m_VsaParameter.max_power = m_dut_peek_power + 3;
	//freq
	m_VsaParameter.freq = m_freq;
	//WIFI standard
	m_VsaParameter.demod = WT_DEMOD_11B;
	// rf port
	m_VsaParameter.rfPort = WT_PORT_RF1;
	//sample time
	m_VsaParameter.smp_time = 2000;
	//timeout waiting
	m_VsaParameter.TimeoutWaiting = 5;
	//trigger pretime
	m_VsaParameter.trig_pretime = 20;
	//trigger level
	m_VsaParameter.trig_level = -28.0;
	//trigger type
	m_VsaParameter.trig_type = WT_TRIG_TYPE_IF;
	//trigger timeout
	m_VsaParameter.trig_timeout = 2;


	//802.11b setting
	m_VsaParameter.dc_removal = (isEnable ?  WT_DC_REMOVAL_ON : WT_DC_REMOVAL_OFF);
	m_VsaParameter.eq_taps = WT_EQ_OFF;
	m_VsaParameter.cck_ph_corr = WT_PH_CORR_11b_ON;
	std::cout << "\n=============================" << std::endl;
	std::cout << "Set auto range VSA parm:" << std::endl;
	std::cout << "\tAuto range max_power:" << m_VsaParameter.max_power << "dBm" << std::endl;
	std::cout << "\tTrigger pretime:" << m_VsaParameter.trig_pretime << "us" << std::endl;
	std::cout << "\tTrigger level:" << m_VsaParameter.trig_level << "dBm" << std::endl;
	std::cout << "\tTrigger timeout:" << m_VsaParameter.trig_timeout << "s" << std::endl;
	std::cout << "\tFreq:" << m_VsaParameter.freq / 1e6 << "MHz" << std::endl;
	std::cout << "\tVSA port:RF1" << std::endl;
	std::cout << "\tSample time:" << m_VsaParameter.smp_time << "ms" << std::endl;
	std::cout << "\tSet 802.11b DC remove: " << (isEnable ? "ON" : "OFF") << std::endl;
	std::cout << "Run Set VSA" << std::endl;
	std::cout << "\n=============================" << std::endl;
	int ret = WT_SetVSA(m_connect_id, &m_VsaParameter);

	return ret == WT_ERR_CODE_OK;
}

bool wifi_11b_demo::Vsa_Run_DcRemove()
{
	std::cout << "Demo 802.11b VSA DC remove" << std::endl;
	int ret = WT_ERR_CODE_GENERAL_ERROR;
	if(false == init())
	{
		goto toss;
	}
	for(int i = 0; i < 2; i++)
	{
		if(false == init_vsa_dc_remove(i == 0))
		{
			goto toss;
		}
		if(run())
		{
			ret = WT_ERR_CODE_OK;
		}
	}
	std::cout << "==============================" << std::endl;
	std::cout << "Demo 802.11b VSA DC remove end !!!" << std::endl;
	std::cout << "==============================" << std::endl;
toss:
	finish();
	return ret == WT_ERR_CODE_OK;
}

bool wifi_11b_demo::init_vsa_eq_taps(int index)
{
	// m_VsaParameter max_power = DUT peek power + 3dBm
	// if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get m_VsaParameter max_power
	// here we don't know DUT Max power input, so just set max_power=0
	m_VsaParameter.max_power = m_dut_peek_power + 3;
	//freq
	m_VsaParameter.freq = m_freq;
	//WIFI standard
	m_VsaParameter.demod = WT_DEMOD_11B;
	// rf port
	m_VsaParameter.rfPort = WT_PORT_RF1;
	//sample time
	m_VsaParameter.smp_time = 2000;
	//timeout waiting
	m_VsaParameter.TimeoutWaiting = 5;
	//trigger pretime
	m_VsaParameter.trig_pretime = 20;
	//trigger level
	m_VsaParameter.trig_level = -28.0;
	//trigger type
	m_VsaParameter.trig_type = WT_TRIG_TYPE_IF;
	//trigger timeout
	m_VsaParameter.trig_timeout = 2;


	//802.11b setting
	m_VsaParameter.dc_removal = WT_DC_REMOVAL_ON;
	m_VsaParameter.cck_ph_corr = WT_PH_CORR_11b_ON;
	struct eq_taps_map
	{
		int index;
		char name[32];
	};
	struct eq_taps_map m_taps[4];
	memset(&m_taps[0], 0, sizeof(struct eq_taps_map) * 4);
	switch(index)
	{
		case 1:
			m_VsaParameter.eq_taps = WT_EQ_5_TAPS;
			m_taps[index].index = index;
			strcpy(m_taps[index].name, "WT_EQ_5_TAPS");
			break;
		case 2:
			m_VsaParameter.eq_taps = WT_EQ_7_TAPS;
			m_taps[index].index = index;
			strcpy(m_taps[index].name, "WT_EQ_7_TAPS");
			break;
		case 3:
			m_VsaParameter.eq_taps = WT_EQ_9_TAPS;
			m_taps[index].index = index;
			strcpy(m_taps[index].name, "WT_EQ_9_TAPS");
			break;
		default:
			index = 0;
			m_VsaParameter.eq_taps = WT_EQ_OFF;
			m_taps[0].index = index;
			strcpy(m_taps[0].name, "WT_EQ_OFF");
			break;
	}

	std::cout << "\n=============================" << std::endl;
	std::cout << "Set auto range VSA parm:" << std::endl;
	std::cout << "\tAuto range max_power:" << m_VsaParameter.max_power << "dBm" << std::endl;
	std::cout << "\tTrigger pretime:" << m_VsaParameter.trig_pretime << "us" << std::endl;
	std::cout << "\tTrigger level:" << m_VsaParameter.trig_level << "dBm" << std::endl;
	std::cout << "\tTrigger timeout:" << m_VsaParameter.trig_timeout << "s" << std::endl;
	std::cout << "\tFreq:" << m_VsaParameter.freq / 1e6 << "MHz" << std::endl;
	std::cout << "\tVSA port:RF1" << std::endl;
	std::cout << "\tSample time:" << m_VsaParameter.smp_time << "ms" << std::endl;
	std::cout << "\tSet 802.11b equalizer taps type: " << m_taps[index].name << std::endl;
	std::cout << "Run Set VSA" << std::endl;
	std::cout << "\n=============================" << std::endl;
	int ret = WT_SetVSA(m_connect_id, &m_VsaParameter);

	return ret == WT_ERR_CODE_OK;
}

bool wifi_11b_demo::Vsa_Run_EqTaps()
{
	std::cout << "Demo 802.11b VSA Equalizer taps" << std::endl;
	int ret = WT_ERR_CODE_GENERAL_ERROR;
	if(false == init())
	{
		goto toss;
	}
	for(int i = 0; i < 4; i++)
	{
		if(false == init_vsa_eq_taps(i))
		{
			goto toss;
		}
		if(run())
		{
			ret = WT_ERR_CODE_OK;
		}
	}
	std::cout << "==============================" << std::endl;
	std::cout << "Demo 802.11b VSA Equalizer taps end !!!" << std::endl;
	std::cout << "==============================" << std::endl;
toss:
	finish();
	return ret == WT_ERR_CODE_OK;
}

bool wifi_11b_demo::init_vsa_cck_ph_corr(bool isEnable)
{
	// m_VsaParameter max_power = DUT peek power + 3dBm
	// if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get m_VsaParameter max_power
	// here we don't know DUT Max power input, so just set max_power=0
	m_VsaParameter.max_power = m_dut_peek_power + 3;
	//freq
	m_VsaParameter.freq = m_freq;
	//WIFI standard
	m_VsaParameter.demod = WT_DEMOD_11B;
	// rf port
	m_VsaParameter.rfPort = WT_PORT_RF1;
	//sample time
	m_VsaParameter.smp_time = 2000;
	//timeout waiting
	m_VsaParameter.TimeoutWaiting = 5;
	//trigger pretime
	m_VsaParameter.trig_pretime = 20;
	//trigger level
	m_VsaParameter.trig_level = -28.0;
	//trigger type
	m_VsaParameter.trig_type = WT_TRIG_TYPE_IF;
	//trigger timeout
	m_VsaParameter.trig_timeout = 2;


	//802.11b setting
	m_VsaParameter.dc_removal = WT_DC_REMOVAL_ON;
	m_VsaParameter.eq_taps = WT_EQ_OFF;
	m_VsaParameter.cck_ph_corr = (isEnable ? WT_PH_CORR_11b_ON : WT_PH_CORR_11b_OFF);
	std::cout << "\n=============================" << std::endl;
	std::cout << "Set auto range VSA parm:" << std::endl;
	std::cout << "\tAuto range max_power:" << m_VsaParameter.max_power << "dBm" << std::endl;
	std::cout << "\tTrigger pretime:" << m_VsaParameter.trig_pretime << "us" << std::endl;
	std::cout << "\tTrigger level:" << m_VsaParameter.trig_level << "dBm" << std::endl;
	std::cout << "\tTrigger timeout:" << m_VsaParameter.trig_timeout << "s" << std::endl;
	std::cout << "\tFreq:" << m_VsaParameter.freq / 1e6 << "MHz" << std::endl;
	std::cout << "\tVSA port:RF1" << std::endl;
	std::cout << "\tSample time:" << m_VsaParameter.smp_time << "ms" << std::endl;
	std::cout << "\tSet 802.11b CCK Phase Tracking: " << (isEnable ? "ON" : "OFF") << std::endl;
	std::cout << "Run Set VSA" << std::endl;
	std::cout << "\n=============================" << std::endl;
	int ret = WT_SetVSA_AutoRange(m_connect_id, &m_VsaParameter);

	return ret == WT_ERR_CODE_OK;
}

bool wifi_11b_demo::Vsa_Run_CCKPhCorr()
{
	std::cout << "Demo 802.11b VSA phase tracking" << std::endl;
	int ret = WT_ERR_CODE_GENERAL_ERROR;
	if(false == init())
	{
		goto toss;
	}
	for(int i = 0; i < 2; i++)
	{
		if(false == init_vsa_cck_ph_corr(i == 0))
		{
			goto toss;
		}
		if(run())
		{
			ret = WT_ERR_CODE_OK;
		}
	}
	std::cout << "==============================" << std::endl;
	std::cout << "Demo 802.11b VSA phase tracking end !!!" << std::endl;
	std::cout << "==============================" << std::endl;
toss:
	finish();
	return ret == WT_ERR_CODE_OK;
}
