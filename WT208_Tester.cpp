
#include "StdAfx.h"
#include "WT208_Tester.h"
// #include "wt208_vsa.h"
#include <iostream>
#include "common.h"
#include <assert.h>
// #include <Windows.h>
#include "Mutex.h"
#include "bt_demo.h"
#include "Vertex_CIU.h"

extern char m_IpAddress[32];

/**
 * @brief unit test for wt208c tester
 * 
 */
void WT208_Tester::tester_unit_test(void)
{
    int rs;
    // rs=multi_connect();
    rs=init();
    if(rs==0)
    {
        printf("connect to the tester failed\n");
    }
    else
    {
        printf("connect to the tester successfully\n");
    }

    rs=set_vsa_Autorange();
    if(rs==1)
    {
        printf("set VSA successfully\n");
    }
    else
    {
        printf("set VSG failed\n");
    }
}
WT208_Tester::WT208_Tester(void)
{
    memset(&m_VsaParameter, 0, sizeof(VsaParameter));
	memset(&m_VsgParameter, 0, sizeof(VsgParameter));
    m_single_autorange=false;
}
WT208_Tester::~WT208_Tester(void)
{

}

/**
 * @brief Connect the Itest WT208C
 * WT_Connect(TesterIpaddress, &tmp_id) is multi connect
 * WT_ForceConnect(TesterIpaddress, &connect_id) is execlusive connect
 * 
 * @return int 
 */
int WT208_Tester::multi_connect(void)
{
    int ret = 0;
    int tmp_id = -1;
    //get ip address
    ret = CIU_Get_IP_Address();

    // get ip address failed and then return
    if (ret == 0)
    {
        printf("get ip address failed\n");
        return 0;
    }

    // multi connect and get the default parameter
    ret = WT_Connect(m_IpAddress, &tmp_id);
    //ret = WT_Connect("192.168.10.254", &tmp_id);
    if (WT_ERR_CODE_OK == ret)
    {
        //printf("connect 192.168.10.254 successfully\n");
        m_connect_id = tmp_id;
        ret = WT_GetDefaultParameter(&m_VsaParameter, &m_VsgParameter);
        assert(WT_ERR_CODE_OK == ret);

        // get cable loss and set cable loss
        m_external_gain=CIU_Get_Cable_Loss();
        // m_external_gain=-30.6;
        ret = WT_SetExternalGain(m_connect_id, m_external_gain);
        assert(WT_ERR_CODE_OK == ret);
    }

    // get the return value
    if (ret == WT_ERR_CODE_OK)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief Init the tester
 * 
 * @return true 
 * @return false 
 */
bool WT208_Tester::init(void)
{
    int rs;
    rs=multi_connect();
    if(rs==1)
    {
        m_Connected=true;
        return true;
    }
    else
    {
        m_Connected=false;
        return false;
    }
}

int WT208_Tester::set_vsa_Autorange()
{
	int ret = 0;
	int rf_port ;//= m_port;
	// if(rf_port <= 0)
	{
		// rf_port = get_rf_vsa_config();
        rf_port=CIU_Get_VSA_Port();
	}

	ret = WT_SetExternalGain(m_connect_id, m_external_gain);
	// vsaParam max_power = target dut power + (14~18dB)
	// if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get vsaParam max_power
	// here we don't know DUT Max power input, so just set max_power=0
	m_VsaParameter.max_power = 36.0; // dut power is 20dbm and then plus 16
	//freq
	m_VsaParameter.freq = 915 * 1e6; //m_freq;
	//WIFI standard
	m_VsaParameter.demod = WT_DEMOD_CW;
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

	// char buf[256] = {0};
	// sprintf(buf, "[%d]WT_SetVSA_AutoRange RF_Port:%d %s\n", m_connect_id, rf_port, (ret == WT_ERR_CODE_OK ? "OK" : "FAIL"));
	// m_print_string += string(buf);

    if(ret==WT_ERR_CODE_OK)
        return 1;
    else
        return 0;
    
	// return ret;
}
/**
 * @brief Set Single Wave AutoRange
 * 
 * @param isTrigger 
 * @return int 1: set single wave autorange ture
 *         int 0: set single wave autorange failed
 */
int WT208_Tester::set_continue_wave_Autorange(bool isTrigger)
{
    int ret = 0;
    int rf_port = CIU_Get_VSA_Port();
    ret = WT_SetExternalGain(m_connect_id, m_external_gain);
    // printf("weijunfeng Set tester external cable loss %lf dB\n", m_external_gain);
    // vsaParam max_power = target dut power + (14~18dB)
    // if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get vsaParam max_power
    // here we don't know DUT Max power input, so just set max_power=0
    m_VsaParameter.max_power = 0.0;
    //freq
    m_VsaParameter.freq = 915 * 1e6;
    //WIFI standard
    m_VsaParameter.demod = WT_DEMOD_CW;
    // rf port
    m_VsaParameter.rfPort = WT_PORT_OFF + rf_port;
    //sample time
    m_VsaParameter.smp_time = 10000;
    //timeout waiting
    m_VsaParameter.TimeoutWaiting = 5;
    //trigger pretime
    m_VsaParameter.trig_pretime = 20;
    //trigger level
    m_VsaParameter.trig_level = -28.0;
    //trigger type
    m_VsaParameter.trig_type = WT_TRIG_TYPE_FREE_RUN;//(isTrigger == true ? WT_TRIG_TYPE_IF : WT_TRIG_TYPE_FREE_RUN);
    //trigger timeout
    m_VsaParameter.trig_timeout = 2;

    // printf("Set auto range VSA parm:\n");
    // printf("\tAuto range max_power:%lf dBm\n", m_VsaParameter.max_power);
    // printf("\tTrigger pretime:%lf us\n", m_VsaParameter.trig_pretime);
    // printf("\tTrigger level:%lf dBm\n", m_VsaParameter.trig_level);
    // printf("\tTrigger timeout:%lf s\n", m_VsaParameter.trig_timeout);
    // printf("\tFreq:%lf MHz\n", m_VsaParameter.freq / 1e6);
    // printf("\tVSA port:RF%d\n", rf_port);
    // printf("\tSample time:%lf us = %lf ms\n", m_VsaParameter.smp_time, (m_VsaParameter.smp_time / 1000));
    // printf("We are going to run VSA Auto range\n");

    ret = WT_SetVSA_AutoRange(m_connect_id, &m_VsaParameter);
    if (WT_ERR_CODE_OK == ret)
    {
        // printf("After run VSA Auto range:\n\tMax power level:%lf dBm\n\tTrigger level:%lf dBm\n", vsaParam.max_power, vsaParam.trig_level);
        return 1;
    }
    else
    {
        // printf("%s Fail\n", __FUNCTION__);
        return 0;
    }
    // return ret;
}
/**
 * @brief set VSA parameter
 * 
 * @return int 
 */
int WT208_Tester::set_vsa_parm()
{
    int ret = WT_SetExternalGain(m_connect_id, m_external_gain);
    int rf_port =CIU_Get_VSA_Port();// get_rf_vsa_config();
    // printf("Set tester external cable loss %lf dB\n", m_external_gain);
    // vsaParam max_power = target dut power + (14~18dB)
    // if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get vsaParam max_power
    // here we don't know DUT Max power input, so just set max_power=0
    m_VsaParameter.max_power =48.0; // dut power is 20dbm and then plus 16
    //freq
    m_VsaParameter.freq = 915 * 1e6;
    //WIFI standard
    m_VsaParameter.demod = WT_DEMOD_CW;
    // rf port
    m_VsaParameter.rfPort = WT_PORT_OFF + rf_port;
    //sample time
    m_VsaParameter.smp_time = 2000;
    //timeout waiting
    m_VsaParameter.TimeoutWaiting = 5;
    //trigger type
    m_VsaParameter.trig_type = WT_TRIG_TYPE_IF;
    //trigger timeout
    m_VsaParameter.trig_timeout = 2;
    //trigger level
    m_VsaParameter.trig_level = -31;

    // printf("Set vsa parm:\n");
    // printf("\tMax_power:%lf dBm\n", m_VsaParameter.max_power);
    // printf("\tFreq:%lf MHz\n", m_VsaParameter.freq);
    // printf("\tTrigger level:%lf dBm\n", m_VsaParameter.trig_level);
    // printf("\tVSA port:RF%d\n", rf_port);
    // printf("\tSample time:%lf ms\n", m_VsaParameter.smp_time);
    // printf("\tWIFI standard 802.11ag\n");

    ret = WT_SetVSA(m_connect_id, &m_VsaParameter);
    // printf("%s ret: %d\n", __FUNCTION__, ret);
    if(ret==WT_ERR_CODE_OK)
        return 1;
    else
        return 0;
    // return ret;
}
/**
 * @brief 
 * 
 * @return int 
 */
int WT208_Tester::set_modulation_vsa_parm()
{
    int ret = WT_SetExternalGain(m_connect_id, m_external_gain);
    printf("the modulation vsa param use cable loss is %lf\n",m_external_gain);
    int rf_port = CIU_Get_VSA_Port(); // get_rf_vsa_config();
    // printf("Set tester external cable loss %lf dB\n", m_external_gain);
    // vsaParam max_power = target dut power + (14~18dB)
    // if not sure DUT Max power input ,we should run WT_SetVSA_AutoRange to get vsaParam max_power
    // here we don't know DUT Max power input, so just set max_power=0
    //m_VsaParameter.max_power = 48.0; // dut power is 20dbm and then plus 16
    m_VsaParameter.max_power =0;
    //freq
    m_VsaParameter.freq = 915 * 1e6;
    //WIFI standard
    m_VsaParameter.demod = WT_DEMOD_LRWPAN_FSK;
    // rf port
    m_VsaParameter.rfPort = WT_PORT_OFF + rf_port;
    //sample time
    m_VsaParameter.smp_time = 2000;
    //timeout waiting
    m_VsaParameter.TimeoutWaiting = 5;
    //trigger type
    m_VsaParameter.trig_type = WT_TRIG_TYPE_FREE_RUN;//WT_TRIG_TYPE_IF;
    //trigger timeout
    m_VsaParameter.trig_timeout = 2;
    //trigger level
    m_VsaParameter.trig_level = -31;

    // printf("Set vsa parm:\n");
    // printf("\tMax_power:%lf dBm\n", m_VsaParameter.max_power);
    // printf("\tFreq:%lf MHz\n", m_VsaParameter.freq);
    // printf("\tTrigger level:%lf dBm\n", m_VsaParameter.trig_level);
    // printf("\tVSA port:RF%d\n", rf_port);
    // printf("\tSample time:%lf ms\n", m_VsaParameter.smp_time);
    // printf("\tWIFI standard 802.11ag\n");

    // ret = WT_SetVSA(m_connect_id, &m_VsaParameter);
    ret = WT_SetVSA_AutoRange(m_connect_id, &m_VsaParameter);

    m_LrwpanFskParam.mrFskType = 0; //0: 2fsk, 1: 4fsk
    m_LrwpanFskParam.mrFskDataRate = 50000;
    m_LrwpanFskParam.fskModIndex = 1.0;
    // m_LrwpanFskParam.mrFskSfdEnable=0;
    // m_LrwpanFskParam.phyMrFskFecEnable=0;
    // m_LrwpanFskParam.phyMrFskSfdValue=0;

    ret = WT_SetLrwpanFskAnalyzeParam(m_connect_id,&m_LrwpanFskParam);
    // printf("%s ret: %d\n", __FUNCTION__, ret);
    if (ret == WT_ERR_CODE_OK)
        return 1;
    else
        return 0;
    // return ret;
}

int WT208_Tester::set_vsg_parm(void)
{
    int ret = WT_ERR_CODE_OK;
    int status = 0;
    int rf_port;

    char wave_path[256] = {0};
    strcpy(wave_path, ".\\config\\WT_GFSK_PMB8_PSDU246_FCS4_DataRate50K_h1_Fs15M.csv");
    rf_port = CIU_Get_VSG_Port();
    m_VsgParameter.waveType = SIG_USERFILE;
    m_VsgParameter.freq = 915 * 1e6;
    // m_VsgParameter.power = -30.0;
    m_VsgParameter.power = -103.0;
    m_VsgParameter.repeat = 200;
    m_VsgParameter.rfPort = WT_PORT_OFF + rf_port;
    m_VsgParameter.wave = wave_path;
    m_VsgParameter.wave_gap = 50;

    // printf("Set asyn vsg parm\n");
    // printf("\tVSG path:%s\n", m_VsgParameter.wave);
    // printf("\tVSG freq:%lf MHz\n", m_VsgParameter.freq / 1e6);
    // printf("\tVSG power:%lf\n", m_VsgParameter.power);
    // printf("\tVSG repeat:%d\n", m_VsgParameter.repeat);
    // printf("\tVSG rf port:RF%d\n", rf_port);
    // printf("\tVSG wave gap:%lf us\n", m_VsgParameter.wave_gap);

    ret = WT_SetVSG(m_connect_id, &m_VsgParameter);
    // if (WT_ERR_CODE_OK == ret)
    // {
    //     ret = WT_AsynStartVSG(m_connect_id);
    //     if (WT_ERR_CODE_OK == ret)
    //     {
            
    //     }
    //     while (1)
    //     {
    //         ret = WT_GetVSGCurrentState(m_connect_id, &status);
    //         if (WT_ERR_CODE_OK == ret &&
    //             WT_VSG_STATE_RUNNING != status && WT_VSG_STATE_WAITING != status)
    //         {
    //             break;
    //         }
    //         if (WT_ERR_CODE_OK != ret)
    //         {
    //             break;
    //         }
    //         Sleep(100);
    //     }
    // }
    if(ret==WT_ERR_CODE_OK)
        return 1;
    else
        return 0;
    // return ret;
}
int WT208_Tester::set_vsg_parm(double power)
{
    int ret = WT_ERR_CODE_OK;
    int status = 0;
    int rf_port;

    char wave_path[256] = {0};
    strcpy(wave_path, ".\\config\\WT_GFSK_PMB8_PSDU246_FCS4_DataRate50K_h1_Fs15M.csv");
    rf_port = CIU_Get_VSG_Port();
    m_VsgParameter.waveType = SIG_USERFILE;
    m_VsgParameter.freq = 915 * 1e6;
    // m_VsgParameter.power = -30.0;
    //m_VsgParameter.power = -103.0;
    m_VsgParameter.power = power;
    m_VsgParameter.repeat = 2000;
    m_VsgParameter.rfPort = WT_PORT_OFF + rf_port;
    m_VsgParameter.wave = wave_path;
    m_VsgParameter.wave_gap = 50;

    // printf("Set asyn vsg parm\n");
    // printf("\tVSG path:%s\n", m_VsgParameter.wave);
    // printf("\tVSG freq:%lf MHz\n", m_VsgParameter.freq / 1e6);
    // printf("\tVSG power:%lf\n", m_VsgParameter.power);
    // printf("\tVSG repeat:%d\n", m_VsgParameter.repeat);
    // printf("\tVSG rf port:RF%d\n", rf_port);
    // printf("\tVSG wave gap:%lf us\n", m_VsgParameter.wave_gap);

    ret = WT_SetVSG(m_connect_id, &m_VsgParameter);
    // if (WT_ERR_CODE_OK == ret)
    // {
    //     ret = WT_AsynStartVSG(m_connect_id);
    //     if (WT_ERR_CODE_OK == ret)
    //     {

    //     }
    //     while (1)
    //     {
    //         ret = WT_GetVSGCurrentState(m_connect_id, &status);
    //         if (WT_ERR_CODE_OK == ret &&
    //             WT_VSG_STATE_RUNNING != status && WT_VSG_STATE_WAITING != status)
    //         {
    //             break;
    //         }
    //         if (WT_ERR_CODE_OK != ret)
    //         {
    //             break;
    //         }
    //         Sleep(100);
    //     }
    // }
    if (ret == WT_ERR_CODE_OK)
        return 1;
    else
        return 0;
    // return ret;
}
/**
 * @brief start async VSG send wave
 * 
 * @return int 
 */
int WT208_Tester::asyn_vsg()
{
    int ret;
    // set VSG parameter
    ret=set_vsg_parm();
    if(ret==0)
    {
        printf("\t\tSet VSG parameter successfully\n");
        return 0;
    }
        

    // start Async start
    ret = WT_AsynStartVSG(m_connect_id);
    if(ret==WT_ERR_CODE_OK)
        return 1;
    else
        return 0;
    
}
/**
 * @brief 
 * 
 * @param power :the VSG power to set
 * @return int 
 */
int WT208_Tester::asyn_vsg(double power)
{
    int ret;
    // set VSG parameter
    ret = set_vsg_parm(power);
    if (ret == 0)
    {
        printf("\t\tSet VSG parameter successfully\n");
        return 0;
    }

    // start Async start
    ret = WT_AsynStartVSG(m_connect_id);
    if (ret == WT_ERR_CODE_OK)
        return 1;
    else
        return 0;
}
/**
 * @brief stop vsg transmit
 * 
 * @return int 
 */
int WT208_Tester::stop_vsg()
{
    int ret;
    ret=WT_StopVSG(m_connect_id);
    if(ret==WT_ERR_CODE_OK)
        return 1;
    else 
        return 0;
}
/**
 * @brief Grab VSA of continuous wave
 * 
 * @return int 
 */
int  WT208_Tester::GrabVSAContinueWave(void)
{
	int vsa_loop = 0;
	int ret = set_continue_wave_Autorange(false);
	int data_capture_fail_count = 0;
	while((vsa_loop++) < 10)
	{

		ret = WT_DataCapture(m_connect_id);
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
		print_continuewave_power_and_freq_err(m_connect_id);

		Sleep(100);
	}
	ret = WT_StopDataCapture(m_connect_id);
	return ret;
}
/**
 * @brief Grab the frequency and power of continuous wave 
 * 
 * @return int 
 */
int WT208_Tester::Grab_CW_Frequency_and_power(void)
{
    double data_result = 0;
    char description[128] = {0};
    char unit[128] = {0};
    int ret ;
    if(m_Connected==false)
        return 0;

    if(m_single_autorange==false)
    {
        ret = set_continue_wave_Autorange(false);
        if (ret)
        {
            m_single_autorange = true;
        }
        else
        {
            printf("set single wave autorange failed\n");
            return 0;
        }
    }
    
    ret = WT_DataCapture(m_connect_id);
    if (WT_ERR_CODE_OK != ret)
    {
        printf("============you should set the autorange paramter to true\n");
        set_continue_wave_Autorange(true);
        ret = WT_DataCapture(m_connect_id);
    }

    if (ret == WT_ERR_CODE_OK)
    {
        //WT_GetResult(m_connect_id, WT_RES_POWER_ALL_DB, &data_result, description, unit);

        WT_GetResult(m_connect_id, WT_RES_POWER_PEAK_DB, &data_result, description, unit);
        // m_dut_peek_power = data_result;

        // record the peak power when the power is more than 14.5
        // when the current power is high than origin,update the new value
        // if (data_result >= 14.5)
        {
            if (data_result > m_dut_peek_power)
                m_dut_peek_power = data_result;
        }

        WT_GetResult(m_connect_id, WT_RES_SPECTRUM_PEAK_POW_FREQ, &data_result, description, unit);
        m_freq = data_result;
        // WT_GetResult(m_connect_id, WT_RES_SPECTRUM_OBW_99, &data_result, description, unit);
        // m_obw = data_result;
        

        // WT_GetResult(m_connect_id, WT_RES_CW_FREQ_OFFSET, &data_result, description, unit);
        // m_freq_offset = data_result;
        m_freq_offset=fabs( fabs(m_freq) - fabs(915 * 1e6));
    }

    // Sleep(100);

    // ret = WT_StopDataCapture(m_connect_id);
    WT_StopDataCapture(m_connect_id);

    if (ret == WT_ERR_CODE_OK)
        return 1;
    else
        return 0;
    // return ret;
}
/**
 * @brief Grab FSK data
 * 
 * @return int 
 */
int WT208_Tester::Grab_FSK_Data(void)
{
    double data_result = 0;
    char description[128] = {0};
    char unit[128] = {0};
    int ret;
    if(m_Connected==false)
        return 0;
    printf("-------------itest connect successfully\n");   

    ret = set_modulation_vsa_parm();
    if (ret == 0)
        return 0;
    printf("-------------------set modulation vsa parameter successfully\n");  

    ret = WT_DataCapture(m_connect_id);
    if (ret != WT_ERR_CODE_OK)
        return 0;
    printf("-------------------capture demodulation data successfully\n");   
    if (ret == WT_ERR_CODE_OK)
    {
        // get power and frequency information
        WT_GetResult(m_connect_id, WT_RES_POWER_PEAK_DB, &data_result, description, unit);
        // WT_GetResult(m_connect_id, WT_RES_POWER_ALL_DB, &data_result, description, unit);
        // WT_GetResult(m_connect_id, WT_RES_POWER_FRAME_DB, &data_result, description, unit);
        m_dut_peek_power = data_result;

        WT_GetResult(m_connect_id, WT_RES_SPECTRUM_PEAK_POW_FREQ, &data_result, description, unit);
        m_freq = data_result;
        m_freq_offset=915*1e6 -m_freq;
        if(m_freq_offset<0)
            m_freq_offset=-m_freq_offset;

        // WT_GetResult(m_connect_id, WT_RES_SPECTRUM_MASK_ERR_PERCENT, &data_result, description, unit);

        // get fsk related information
        // WT_GetResult(m_connect_id, WT_RES_LRWPAN_FREQ_DEVIATION_TOLERANCE_RMS, &data_result, description, unit);
        // WT_GetResult(m_connect_id, WT_RES_LRWPAN_FREQ_DEVIATION_TOLERANCE_MAX, &data_result, description, unit);
        // WT_GetResult(m_connect_id, WT_RES_LRWPAN_FREQ_DEVIATION_TOLERANCE_MIN, &data_result, description, unit);
        // WT_GetResult(m_connect_id, WT_RES_LRWPAN_ZERO_CROSSING_TOLERANCE_RMS, &data_result, description, unit);
        // WT_GetResult(m_connect_id, WT_RES_LRWPAN_ZERO_CROSSING_TOLERANCE_PEAK, &data_result, description, unit);
        // WT_GetResult(m_connect_id, WT_RES_LRWPAN_ZERO_CROSSING_TOLERANCE_MIN, &data_result, description, unit);
        printf("----------------------------analyse result information successfully\n"); 
    }

    // Sleep(100);

    // ret = WT_StopDataCapture(m_connect_id);
    WT_StopDataCapture(m_connect_id);

    if (ret == WT_ERR_CODE_OK)
        return 1;
    else
        return 0;
    // return ret;
}
/**
 * @brief print power and frequency of continuous wave 
 * 
 * @param id 
 * @return int 
 */
int WT208_Tester::print_continuewave_power_and_freq_err(int id)
{
	double data_result = 0;
	char description[128] = {0};
	char unit[128] = {0};
	int ret = WT_GetResult(id, WT_RES_POWER_ALL_DB, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Power all: %lf  dBm\n", data_result);
	}

	ret = WT_GetResult(id, WT_RES_POWER_PEAK_DB, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Power peak: %lf  dBm\n", data_result);
	}
    
    ret = WT_GetResult(id, WT_RES_SPECTRUM_PEAK_POW_FREQ, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Peak frequency: %lf Hz\n", data_result);
	}

	ret = WT_GetResult(id, WT_RES_CW_FREQ_OFFSET, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Freq err: %lf Hz\n", data_result);
	}
	return ret;
}

