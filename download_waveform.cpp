#include "StdAfx.h"
#include "download_waveform.h"
#include "common.h"

download_waveform::download_waveform(void)
{
	m_connect_id = 0;
	gWaveName = (char *)malloc(MAX_PATH);
	memset(gWaveName, 0, MAX_PATH);
}


download_waveform::~download_waveform(void)
{
}

bool download_waveform::startWaveformVsg()
{
	int ret = WT_ERR_CODE_OK;
	int status = 0;
	int rf_port = get_rf_vsg_config();
	double external_gain = 0.0;

	VsgParameter vsgParam = {0};
	vsgParam.waveType = SIG_TESTERFILE;
	vsgParam.freq = 2412 * 1e6;
	vsgParam.power = -10.0;
	vsgParam.repeat = 1000;
	vsgParam.rfPort = WT_PORT_OFF + rf_port;
	vsgParam.wave = gWaveName;
	vsgParam.wave_gap = 50;

	printf("\n============================\n");
	printf("Start downloaded waveform VSG? (Y/N)\n>");
	if(get_Choose())
	{
		if(m_connect_id == 0)
		{
			return false;
		}
		do
		{
			printf("Demo WIFI VSG setting is:\n");
			printf("\tVSG power:%f\n", vsgParam.power);
			printf("\tVSG path:%s\n", vsgParam.wave);
			printf("\tVSG freq:%lf MHz\n", vsgParam.freq / 1e6);
			printf("\tVSG power:%lf\n", vsgParam.power);
			printf("\tVSG repeat:%d\n", vsgParam.repeat);
			printf("\tVSG rf port:RF%d\n", rf_port);
			printf("\tVSG wave gap:%lf us\n", vsgParam.wave_gap);
			printf("\tCable loss:%lf dB\n", external_gain);
			printf("\tWaveform is %s\n\n", gWaveName);

			ret = WT_SetVSG(m_connect_id, &vsgParam);
			if(WT_ERR_CODE_OK == ret)
			{
				ret = WT_AsynStartVSG(m_connect_id);
				if(WT_ERR_CODE_OK == ret)
				{
					printf("Start VSG now!\n");
					printf("\n============================\n");
				}
				while(1)
				{
					ret = WT_GetVSGCurrentState(m_connect_id, &status);
					if(WT_ERR_CODE_OK == ret
						&& WT_VSG_STATE_RUNNING != status
						&& WT_VSG_STATE_WAITING != status)
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
		}
		while(0);


		WT_DisConnect(m_connect_id);
	}
	return true;
}

bool download_waveform::exec()
{
	char waveform_path[MAX_PATH] = {0};
	char waveform_name[MAX_PATH] = {0};
	char *ptr = NULL;
	int iret = 0;
	int waveExists = 0;
	bool isAddWave = false;

	printf("Demo waveform download Test? (Y/N)\n>");
	if(get_Choose())
	{
		if(false == demo_connect_tester(&m_connect_id))
		{
			return false;
		}
		do
		{
			printf("\n============================\n");
			printf("We are going to demo download waveform to tester\n");
			printf("\n============================\n");
			printf("Please input your waveform path\n>");
			get_input(waveform_path);
			if(waveform_path[0])
			{
				ptr = strrchr(waveform_path, '\\');
				if(!ptr)
				{
					//get current path waveform
					char tmp[256] = {0};
					get_exe_path(tmp);
					strcat(tmp, "\\");
					strcat(tmp, waveform_path);
					strcpy(waveform_name, waveform_path);
					strcpy(waveform_path, tmp);
				}
				else
				{
					strcpy(waveform_name, (const char *)&waveform_path[ptr - waveform_path + 1]);
				}
				if(waveform_name[0])
				{
					iret = WT_QueryTesterWave(m_connect_id, waveform_name, &waveExists);
					if(WT_ERR_CODE_OK == iret)
					{
						if(waveExists)
						{
							printf("Find waveform %s already in tester, please choose which one your want to do\n", waveform_name);
							printf("\t[1]:Replace it\n");
							printf("\t[2]:Delete it\n");
							printf("\t[3]:Do nothing\n");
							printf(">");
							switch(get_index())
							{
								case 1:
									iret = WT_OperateTesterWave(m_connect_id, waveform_path, ADD_WAVE);
									isAddWave = true;
									break;
								case 2:
									iret = WT_OperateTesterWave(m_connect_id, waveform_path, REMOVE_WAVE);
									isAddWave = false;
									break;
								default:
									break;
							}
						}
						else
						{
							printf("Download waveform:\t %s to tester now !!!\n", waveform_path);
							iret = WT_OperateTesterWave(m_connect_id, waveform_path, ADD_WAVE);
							isAddWave = true;
						}
						if(WT_ERR_CODE_OK != iret)
						{
							printf("Waveform operate API exec fail !!!\n");
						}
						else
						{
							strcpy(gWaveName, waveform_name);
							printf("Waveform operate API exec OK !!!\n");
							if(isAddWave == true)
							{
								startWaveformVsg();
							}
						}
						break;
					}
				}
				else
				{
					printf("Unknown input waveform name\n");
				}
			}
			else
			{
				printf("Unknown input waveform path\n");
			}
			printf("Try again ? (Y/N)\n>");
			if(!get_Choose())
			{
				break;
			}
		}
		while(1);
		WT_DisConnect(m_connect_id);
	}
	return iret == WT_ERR_CODE_OK;
}
