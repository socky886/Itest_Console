#include "stdafx.h"
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <tester.h>
#include <testerCommon.h>
#include <Windows.h>
#include <vector>
#include <string>
#include <regex>
using namespace std;


bool FileExists(const char *path)
{
	return find_file(path);
}

void get_exe_path(char *buf)
{
	char path[256] = {0};
	char *ptr = NULL;
	GetModuleFileNameA(NULL, path, sizeof(path));
	ptr = strrchr(path, '\\');
	if(ptr)
	{
		path[ptr - path] = '\0';
	}
	if(buf)
	{
		strcpy(buf, path);
	}
	SetCurrentDirectoryA(path);
}

bool find_file(const char *path)
{
	FILE *fp = fopen(path, "rb");
	if(fp)
	{
		fclose(fp);
		return true;
	}
	return false;
}
/**
 * @brief remove new line and return character
 * 
 * @param buf 
 */
void get_input(char *buf)
{
	char *ptr = NULL;
	fgets(buf, MAX_PATH, stdin);

	fflush(stdin);
	ptr = strrchr(buf, '\r');
	if(ptr)
	{
		buf[ptr - buf] = '\0';
	}
	ptr = strrchr(buf, '\n');
	if(ptr)
	{
		buf[ptr - buf] = '\0';
	}
}
/**
 * @brief Get the input value whether yes or no
 * 
 * @return true 
 * @return false 
 */
bool get_Choose()
{
	char buf[128] = {0};
	char *ptr = NULL;
	fgets(buf, 2, stdin);
	fflush(stdin);
	if('Y' == buf[0] || 'y' == buf[0])
	{
		return true;
	}
	return false;
}

int get_index()
{
	char buf[128] = {0};
	char *ptr = NULL;
	fgets(buf, 2, stdin);
	fflush(stdin);
	return (buf[0] - 0x30);
}
/**
 * @brief print all power
 * 
 * @param id 
 * @return int 
 */
int print_power(int id)
{
	double data_result = 0;
	char description[128] = {0};
	char unit[128] = {0};
	int ret = WT_GetResult(id, WT_RES_POWER_FRAME_DB, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Power frame: %lf dBm\n", data_result);
	}
	ret = WT_GetResult(id, WT_RES_POWER_ALL_DB, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Power all: %lf  dBm\n", data_result);
	}
	ret = WT_GetResult(id, WT_RES_POWER_PEAK_DB, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Power peak: %lf  dBm\n", data_result);
	}
	return ret;
}
// 
int print_evm(int id)
{
	double data_result = 0;
	char description[128] = {0};
	char unit[128] = {0};
	int ret = WT_GetResult(id, WT_RES_FRAME_EVM_ALL, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("EVM all: %lf dB\n", data_result);
	}
	ret = WT_GetResult(id, WT_RES_FRAME_EVM_PEAK, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("EVM peak: %lf dB\n", data_result);
	}
	ret = WT_GetResult(id, WT_RES_FRAME_EVM_PILOT_DB, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("EVM pilot: %lf dB\n", data_result);
	}
	return ret;
}

int print_freq_err(int id)
{
	double data_result = 0;
	char description[128] = {0};
	char unit[128] = {0};
	int ret = WT_GetResult(id, WT_RES_FRAME_FREQ_ERR, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Freq err: %lf Hz\n", data_result);
	}
	return ret;
}

int print_continuewave_power_and_freq_err(int id)
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
	ret = WT_GetResult(id, WT_RES_CW_FREQ_OFFSET, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Freq err: %lf Hz\n", data_result);
	}
	return ret;
}

int print_bt_info(int connect_id)
{
	double data_result = 0;
	char description[128] = {0};
	char unit[128] = {0};
	int ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_CARR_FREQ_BUF, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Initial freq error: %lf KHz\n", data_result / 1e3);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_EDR_Omega_I, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("EDR Omega I: %lf KHz\n", data_result / 1e3);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_EDR_Omega_O, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("EDR Omega O: %lf KHz\n", data_result / 1e3);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_EDR_Omega_IO, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("EDR Omega IO: %lf KHz\n", data_result / 1e3);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_BT_DEVM, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("EDR DEVM Avg: %lf%%\n", data_result);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_BT_DEVM_PEAK, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("EDR DEVM Peak: %lf%%\n", data_result);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_BT_POW_DIFF, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("EDR Power Diff: %lf dB\n", data_result);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_MAX_CARR_FREQ, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Max Initial freq: %lf KHz\n", data_result / 1e3);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_CARR_FREQ_DRIFT, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Freq.Drift: %lf KHz\n", data_result / 1e3);
	}

	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_DELTA_F1_AVG, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Delta F1 avg: %lf KHz\n", data_result / 1e3);
	}

	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_DELTA_F2_AVG, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Delta F2 avg: %lf KHz\n", data_result / 1e3);
	}

	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_DELTA_F2_MAX, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Delta F2 max: %lf KHz\n", data_result / 1e3);
	}
	return ret;
}

int print_zigbee_info(int connect_id)
{
	double data_result = 0;
	char description[128] = {0};
	char unit[128] = {0};
	int ret = WT_GetResult(connect_id, WT_RES_ZIGBEE_FRAME_EVM_PSDU, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Zigbee EVM(PSDU) %lf dB\n", data_result);
	}
	ret = WT_GetResult(connect_id, WT_RES_ZIGBEE_FRAME_EVM_PSDU_PERCENT, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Zigbee EVM(PSDU) %lf %%\n", data_result);
	}
	ret = WT_GetResult(connect_id, WT_RES_ZIGBEE_FRAME_EVM_SHRPHR, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Zigbee EVM(SHR+PHR) %lf dB\n", data_result);
	}
	ret = WT_GetResult(connect_id, WT_RES_ZIGBEE_FRAME_EVM_SHRPHR_PERCENT, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		printf("Zigbee EVM(SHR+PHR) %lf %%\n", data_result);
	}
	return ret;
}

int print_power_2(int id, char *result)
{
	double data_result = 0;
	char description[128] = {0};
	char unit[128] = {0};
	char buf[256] = {0};
	string print_result;

	int ret = WT_GetResult(id, WT_RES_POWER_FRAME_DB, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Power frame: %lf dBm\n", data_result);
		print_result += string(buf);
	}
	ret = WT_GetResult(id, WT_RES_POWER_ALL_DB, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Power all: %lf  dBm\n", data_result);
		print_result += string(buf);
	}
	ret = WT_GetResult(id, WT_RES_POWER_PEAK_DB, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Power peak: %lf  dBm\n", data_result);
		print_result += string(buf);
	}
	if(result && buf[0])
	{
		strcpy(result, print_result.c_str());
	}
	return ret;
}

int print_evm_2(int id, char *result)
{
	double data_result = 0;
	char description[128] = {0};
	char unit[128] = {0};
	char buf[256] = {0};
	string print_result;

	int ret = WT_GetResult(id, WT_RES_FRAME_EVM_ALL, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "EVM all: %lf dB\n", data_result);
		print_result += string(buf);
	}
	ret = WT_GetResult(id, WT_RES_FRAME_EVM_PEAK, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "EVM peak: %lf dB\n", data_result);
		print_result += string(buf);
	}
	ret = WT_GetResult(id, WT_RES_FRAME_EVM_PILOT_DB, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "EVM pilot: %lf dB\n", data_result);
		print_result += string(buf);
	}
	if(result && buf[0])
	{
		strcpy(result, print_result.c_str());
	}
	return ret;
}

int print_freq_err_2(int id, char *result)
{
	double data_result = 0;
	char description[128] = {0};
	char unit[128] = {0};
	char buf[256] = {0};
	string print_result;
	int ret = WT_GetResult(id, WT_RES_FRAME_FREQ_ERR, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Freq err: %lf Hz\n", data_result);
		print_result += string(buf);
	}
	if(result && buf[0])
	{
		strcpy(result, print_result.c_str());
	}
	return ret;
}

int print_bt_info_2(int connect_id, char *result)
{
	double data_result = 0;
	char description[128] = {0};
	char unit[128] = {0};
	char buf[256] = {0};
	string print_result;

	int ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_CARR_FREQ_BUF, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Initial freq error: %lf KHz\n", data_result / 1e3);
		print_result += string(buf);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_EDR_Omega_I, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "EDR Omega I: %lf KHz\n", data_result / 1e3);
		print_result += string(buf);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_EDR_Omega_O, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "EDR Omega O: %lf KHz\n", data_result / 1e3);
		print_result += string(buf);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_EDR_Omega_IO, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "EDR Omega IO: %lf KHz\n", data_result / 1e3);
		print_result += string(buf);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_BT_DEVM, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "EDR DEVM Avg: %lf%%\n", data_result);
		print_result += string(buf);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_BT_DEVM_PEAK, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "EDR DEVM Peak: %lf%%\n", data_result);
		print_result += string(buf);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_BT_POW_DIFF, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "EDR Power Diff: %lf dB\n", data_result);
		print_result += string(buf);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_MAX_CARR_FREQ, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Max Initial freq: %lf KHz\n", data_result / 1e3);
		print_result += string(buf);
	}
	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_CARR_FREQ_DRIFT, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Freq.Drift: %lf KHz\n", data_result / 1e3);
		print_result += string(buf);
	}

	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_DELTA_F1_AVG, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Delta F1 avg: %lf KHz\n", data_result / 1e3);
		print_result += string(buf);
	}

	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_DELTA_F2_AVG, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Delta F2 avg: %lf KHz\n", data_result / 1e3);
		print_result += string(buf);
	}

	ret = WT_GetResult(connect_id, WT_RES_BT_FRAME_DELTA_F2_MAX, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Delta F2 max: %lf KHz\n", data_result / 1e3);
		print_result += string(buf);
	}
	if(result && buf[0])
	{
		strcpy(result, print_result.c_str());
	}
	return ret;
}

int print_zigbee_info_2(int connect_id, char *result)
{
	double data_result = 0;
	char description[128] = {0};
	char unit[128] = {0};
	char buf[256] = {0};
	string print_result;
	int ret = WT_GetResult(connect_id, WT_RES_ZIGBEE_FRAME_EVM_PSDU, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Zigbee EVM(PSDU) %lf dB\n", data_result);
		print_result += string(buf);
	}
	ret = WT_GetResult(connect_id, WT_RES_ZIGBEE_FRAME_EVM_PSDU_PERCENT, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Zigbee EVM(PSDU) %lf %%\n", data_result);
		print_result += string(buf);
	}
	ret = WT_GetResult(connect_id, WT_RES_ZIGBEE_FRAME_EVM_SHRPHR, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Zigbee EVM(SHR+PHR) %lf dB\n", data_result);
		print_result += string(buf);
	}
	ret = WT_GetResult(connect_id, WT_RES_ZIGBEE_FRAME_EVM_SHRPHR_PERCENT, &data_result, description, unit);
	if(WT_ERR_CODE_OK == ret)
	{
		sprintf(buf, "Zigbee EVM(SHR+PHR) %lf %%\n", data_result);
		print_result += string(buf);
	}
	if(result && buf[0])
	{
		strcpy(result, print_result.c_str());
	}
	return ret;
}


static int find_int_value(string buf, string token)
{
	char *ptr = NULL;
	char *p = NULL;
	vector<string>lines;
	ptr = strtok_s((char *)buf.c_str(), "\r\n", &p);
	while(ptr != NULL)
	{
		lines.push_back(ptr);
		ptr = strtok_s(NULL, "\r\n", &p);
	}

	std::regex_constants::syntax_option_type fl = std::regex_constants::icase;
	for(vector<string>::size_type i = 0; i < lines.size(); i++)
	{
		if(regex_match(lines.at(i), std::regex("\\n[\\s\\t]*$")))
		{
			continue;
		}
		//if(regex_match(lines.at(i),std::regex("(\/\\\*([^*]|[\\\r\\\n]|(\\\*([^*\/]|[\\\r\\\n])))*\\\*+\/)|(\/\/.*)|(#.*)")))
		if (regex_match(lines.at(i), std::regex("(\/\\*([^*]|[\\\r\\\n]|(\\*([^*\/]|[\\\r\\\n])))*\\*+\/)|(\/\/.*)|(#.*)")))
		{
		    continue;
		}
		// if(!regex_match(lines.at(i), std::regex("^.+\/\/{1}.*$")))
		// {
		// 	continue;
		// 	// break;
		// }
		
		lines.at(i) = regex_replace(lines.at(i), std::regex("^\\s*|\\s*$"), string(""));
		lines.at(i) = regex_replace(lines.at(i), std::regex("\\s"), string(""));
		printf(lines.at(i).c_str());
		printf("\n");
		string::size_type  pos = lines.at(i).find(token);
		if(pos != string::npos)
		{
			pos = lines.at(i).find("=");
			if(pos != string::npos)
			{
				lines.at(i) = lines.at(i).substr(pos + 1);
				return atoi(lines.at(i).c_str());
			}
		}
	}
	return 1;
}

int get_rf_vsa_config()
{
	char *buf = NULL;
	int rf_port = 1;
	FILE *fp = fopen(RF_CONFIG_FILE, "rb");
	if(fp)
	{
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);
		rewind(fp);
		buf = new char[len + 1];
		int ret = fread(buf, len, 1, fp);
		if(ret >= 0)
		{
			rf_port = find_int_value(buf, VSA_RF_PORT_TOKEN);
		}
		fclose(fp);
		delete []buf;
	}
	if(rf_port <= 0 || rf_port > 4)
	{
		rf_port = 1;
	}
	return rf_port;
}

int get_rf_vsg_config()
{
	char *buf = NULL;
	int rf_port = WT_PORT_RF1;
	FILE *fp = fopen(RF_CONFIG_FILE, "rb");
	if(fp)
	{
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);
		rewind(fp);
		buf = new char[len + 1];
		int ret = fread(buf, len, 1, fp);
		if(ret >= 0)
		{
			rf_port = find_int_value(buf, VSG_RF_PORT_TOKEN);
		}
		fclose(fp);
		delete []buf;
	}
	//
	if(rf_port < WT_PORT_RF1 || rf_port > WT_PORT_RF8)
	{
		rf_port = WT_PORT_RF1;
	}
	return rf_port;
}
int get_thread_config()
{
	char *buf = NULL;
	int num = 1;
	FILE *fp = fopen(RF_CONFIG_FILE, "rb");
	if(fp)
	{
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);
		rewind(fp);
		buf = new char[len + 1];
		int ret = fread(buf, len, 1, fp);
		if(ret >= 0)
		{
			num = find_int_value(buf, USER_COUNT_TOKEN);
		}
		fclose(fp);
		delete []buf;
	}
	if(num <= 0 || num > 4)
	{
		num = 1;
	}
	return num;
}

int get_connect_delta_config()
{
	char *buf = NULL;
	int delta = 0;
	FILE *fp = fopen(RF_CONFIG_FILE, "rb");
	if(fp)
	{
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);
		rewind(fp);
		buf = new char[len + 1];
		int ret = fread(buf, len, 1, fp);
		if(ret >= 0)
		{
			// delta = find_int_value(buf, CONNECT_TIME_DELTA);
			delta = find_int_value(buf, WEIJUNFENG);

		}
		fclose(fp);
		delete []buf;
	}
	if(delta <= 0 || delta > 1000)
	{
		delta = 1;
	}
	return delta;
}

