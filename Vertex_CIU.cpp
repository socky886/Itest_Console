
#include "stdafx.h"
#include "Vertex_CIU.h"
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
#include "serial\serial.h"
#include "WT208_Tester.h"
using namespace serial;
using namespace std;

#pragma comment(lib,"serial.lib")

Serial dutSerial("", 115200, Timeout::simpleTimeout(1000));
// Serial goldenSerial("", 115200, Timeout::simpleTimeout(1000));
Serial switchSerial("", 115200, Timeout::simpleTimeout(1000));

extern WT208_Tester c_tester;
char dut_com[10];
char golden_com[10];

char switch_com[10]; //control the switch to tx and rx

char m_RX_Buf[256];// Serial Port Receive buffer
int m_RX_Len;// Serial Port Receive character length

char m_Serial_Number[20];
int m_Serial_len;
char m_Debug='1';
char m_IpAddress[32];
char m_VSA_IpAddress[32];
int m_Frequency_up;
int m_Frequency_low;

int m_TxVerify_Count=0;

int m_nRssi[50];
int m_nRssi_cn = 0;

int m_nLQI[50];
int m_nLQI_cn = 0;

int m_nGood[50];
int m_nGood_cn = 0;

int m_nTotal[50];
int m_nTotal_cn = 0;

/**
 * @brief Unit test
 * 
 */
int ciu_unit_test(void)
{
    int ret = 0;
    char buf[128] = {0};
    float rs=1.0;
    int i=0;
    string res;
    std::string dut;
    char xx[100];
    // dutSerial.close();
    ret=CIU_Get_DUT_Port();
    sprintf(buf,"the dut is com%d\n",ret);
    printf(buf);

    // ret=CIU_Get_VSG_Port();
    // sprintf(buf,"the vsg is %d\n",ret);
    // printf(buf);

    // ret=CIU_Get_VSA_Port();
    // sprintf(buf,"the vsa is %d\n",ret);
    // printf(buf);

    // get the cable loss
    // rs=CIU_Get_Cable_Loss();
    // ostringstream oss;
    // oss<<rs;
    // res=oss.str();
    // printf("the cable loss is ");
    // printf(res.c_str());
    // printf("\n");

    // get the frequency offset range
    rs=CIU_Get_Frequency_Range();
    printf("the frequency offset up is  %d\n",m_Frequency_up);
    printf("the frequency offset low is %d\n",m_Frequency_low);

    // get TX verify count
    rs=CIU_Get_TX_Verify_Count();
    printf("the TX verify count is %d\n",m_TxVerify_Count);

    // open Serial Port of DUT
    ret = CIU_Open_DUT();
    if(ret==0)
    {
        printf("open serial port of dut fail\n");
        printf("falied\n");
        return 0;
    }
    printf("open serial port of dut successfully\n");

    // verify the DUT
    ret = CIU_Verify_DUT();
    if(ret==0)
    {
        printf("verify dut failed\n");
        printf("failed\n");
        return 0;
    }
    printf("verify dut successfully\n");
    // CIU_Get_DUT_SerialNumber();
    
    // manually calibration the xtal
    // CIU_Manu_Calibration_Xtal();
    // do
    // {
    //     ;
    // } while (1);

   
    
    // init tester
    ret=c_tester.init();
    if(ret==false)
    {
        printf("connect tester failed\n");
        printf("failed\n");
        return 0;
    }

    // write nv to config the dut and force dut into test mode
    ret=CIU_Force_To_Test();
    if(ret==0)
    {
        printf("ciu force to test mode failed\n");
        printf("failed\n");
        return 0;  
    }
    printf("ciu force to test mode successfully\n");

    // frequency calibration
    ret=CIU_Frequency_Calibration();
    if(ret==0)
    {
        printf("ciu xtal calibration failed\n");
        printf("failed\n");
        return 0;
    }
    printf("ciu xtal calibration successfully\n");

    // CIU reset
    CIU_Reset();

    // verify tx
    if(ret==3) // the grab peak power is lower than 14.5dbm
    {
        for(i=1;i<m_TxVerify_Count;i++)
        {
            ret=CIU_Verify_TX();
            if(ret==1)
            {
                // write nv tx_power_cal_offset
                sprintf_s(xx, "cent nv tx_power_cal_offset %d\r", int(c_tester.m_dut_peek_power*2));
                dut = xx;
                dutSerial.write(dut);
                Sleep(5);
                break;
            }
        }
    }
   
    
    // ret=CIU_Verify_TX();
    if(ret==0)
    {
        printf("ciu Verify TX timeout\n");
        printf("ciu Verify TX failed\n");
        return 0;
    }
    printf("ciu Verify TX successfully\n");
    
    // verify RX
    ret=CIU_Verify_RX();
    if(ret==0)
    {
        printf("ciu Verify RX failed\n");
        printf("failed\n");
        return 0;
    }

    CIU_Exit_RFTest();
    // totally tsst pass
    printf("ciu Verify RX successfully\n");
    
    printf("all items test passed.\n");

    if(dutSerial.isOpen())
    {
        dutSerial.close();
    }
    return 1;
    
}
/**
 * @brief Get config item value which value is integer
 * 
 * @param buf 
 * @param token 
 * @return int 
 */
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
		// printf(lines.at(i).c_str());
		// printf("\n");
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
	// return 1;
    return -1;
}

/**
 * @brief Get config item value which value is integer
 * 
 * @param buf 
 * @param token 
 * @return int 
 */
static float find_float_value(string buf, string token)
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
		// printf(lines.at(i).c_str());
		// printf("\n");
		string::size_type  pos = lines.at(i).find(token);
		if(pos != string::npos)
		{
			pos = lines.at(i).find("=");
			if(pos != string::npos)
			{
				lines.at(i) = lines.at(i).substr(pos + 1);
				return atof(lines.at(i).c_str());
			}
		}
	}
	// return 1;
    return -1.0;
}

/**
 * @brief get the item configration which value is integer
 * 
 * @param p 
 * @return int 
 */
int CIU_get_int_config(const char *p)
{
    char *buf = NULL;
    int num = -1;
    FILE *fp = fopen(".\\config\\api_config.txt", "rb");
    if (fp)
    {
        // printf("open api_config.txt successfully\n");

        fseek(fp, 0, SEEK_END);
        size_t len = ftell(fp);
        rewind(fp);
        buf = new char[len + 1];
        int ret = fread(buf, len, 1, fp);
        if (ret >= 0)
        {
            // num = find_int_value(buf, USER_COUNT_TOKEN);
            num = find_int_value(buf, p);
        }
        fclose(fp);
        delete[] buf;
    }
    // if (num <= 0 || num > 4)
    // {
    //     num = 1;
    // }
    // printf("open api_config.txt failed\n");
    return num;
}
/**
 * @brief Get DUT Com Port
 * 
 * @return int 
 */
int CIU_Get_DUT_Port(void)
{
    int rs=CIU_get_int_config(DUT_CONFIG);
    if(rs>=0)
        sprintf(dut_com,"COM%d",rs);
    return rs;
}

/**
 * @brief Get VSG Port
 * 
 * @return int 
 */
int CIU_Get_VSG_Port(void)
{
    return CIU_get_int_config(VSG_PORT_CONFIG);
}
/**
 * @brief Get TX Verify Count
 * 
 * @return int 
 */
int CIU_Get_TX_Verify_Count(void)
{
    m_TxVerify_Count=CIU_get_int_config(WT_TX_VERIFY_COUNT);
    return 1;
}
/**
 * @brief Get VSA Port
 * 
 * @return int 
 */
int CIU_Get_VSA_Port(void)
{
    return CIU_get_int_config(VSA_PORT_CONFIG);
}
int CIU_Get_Frequency_Range(void)
{
    m_Frequency_up=CIU_get_int_config(WT_WISUN_FREQ_XTALOFFSET_UP);
    m_Frequency_low=CIU_get_int_config(WT_WISUN_FREQ_XTALOFFSET_LOW);
    return 1;
}
/**
 * @brief get cable loss cofig
 * 
 * @return int 
 */
float CIU_Get_Cable_Loss(void)
{
    char *buf = NULL;
    float num = -1.0;
    FILE *fp = fopen(".\\config\\api_config.txt", "rb");
    if (fp)
    {
        // printf("open api_config.txt successfully\n");

        fseek(fp, 0, SEEK_END);
        size_t len = ftell(fp);
        rewind(fp);
        buf = new char[len + 1];
        int ret = fread(buf, len, 1, fp);
        if (ret >= 0)
        {
            // num = find_int_value(buf, USER_COUNT_TOKEN);
            num = find_float_value(buf, "WT_FIXED_ATTEN_SUB_1G");
        }
        fclose(fp);
        delete[] buf;
    }
    // if (num <= 0 || num > 4)
    // {
    //     num = 1;
    // }
    // printf("open api_config.txt failed\n");
    return num;
}
/**
 * @brief get VSG single level
 * 
 * @return int 
 */
float CIU_Get_VSG_Level(void)
{
    char *buf = NULL;
    float num = -1.0;
    FILE *fp = fopen(".\\config\\api_config.txt", "rb");
    if (fp)
    {
        // printf("open api_config.txt successfully\n");

        fseek(fp, 0, SEEK_END);
        size_t len = ftell(fp);
        rewind(fp);
        buf = new char[len + 1];
        int ret = fread(buf, len, 1, fp);
        if (ret >= 0)
        {
            // num = find_int_value(buf, USER_COUNT_TOKEN);
            num = find_float_value(buf, "VSG_OUTPUT_LEVEL");
        }
        fclose(fp);
        delete[] buf;
    }
    // if (num <= 0 || num > 4)
    // {
    //     num = 1;
    // }
    // printf("open api_config.txt failed\n");
    return num;
}
/**
 * @brief 
 * 
 * @return int
 */
int CIU_Get_TX_Packet_Number(void)
{
    return CIU_get_int_config(TX_PACKET_NUMBER);
}

/**
 * @brief 
 * 
 * @return float
 */
float CIU_Get_Tx_Power_Lower(void)
{
    char *buf = NULL;
    float num = -1.0;
    FILE *fp = fopen(".\\config\\api_config.txt", "rb");
    if (fp)
    {
        // printf("open api_config.txt successfully\n");

        fseek(fp, 0, SEEK_END);
        size_t len = ftell(fp);
        rewind(fp);
        buf = new char[len + 1];
        int ret = fread(buf, len, 1, fp);
        if (ret >= 0)
        {
            // num = find_int_value(buf, USER_COUNT_TOKEN);
            num = find_float_value(buf, "TX_POWER_LOWER");
        }
        fclose(fp);
        delete[] buf;
    }
    // if (num <= 0 || num > 4)
    // {
    //     num = 1;
    // }
    // printf("open api_config.txt failed\n");
    return num;
}
/**
 * @brief 
 * 
 * @return int 
 */
int CIU_Open_DUT(void)
{
    printf(dut_com);
    printf("\n");
    dutSerial.setPort(dut_com);
    dutSerial.open();
    dutSerial.setTimeout(serial::Timeout::max(), 100, 0, 100, 0); // set timeout is 100ms
    if (dutSerial.isOpen())
        return 1;
    else
        return 0;
    
}
/**
 * @brief 
 * 
 * @return int 
 */
int CIU_Verify_DUT(void)
{
    int rs;
    rs = Verify_Device_Connect(DUT_DEVICE);
    return rs;
}
/**
 * @brief 
 * 
 * @return int 
 */
int CIU_Get_DUT_SerialNumber(void)
{
    int rs;
    // get the serial number
    rs = Get_Serial_Number();
    if (rs == 0)
    {
        if (m_Debug == '1')
            printf("get serial number error\n");
        // goldenSerial.close();
        dutSerial.close();
        return 0;
    }
    else
    {
        if (m_Debug == '1')
            printf("get serial number successfully\n\n\n");

        return 1;
    }
}

/**
 * @brief verify the connection of dut and golden node
 * 
 * @param char 
 * @return int 
 */
int Verify_Device_Connect(unsigned char deviceno)
{
    string temp;
    int len;
    char str[256];
    // int i;
    int rs;
    if (deviceno == GOLDEN_DEVICE)
    {
        // // force the device into rftest mode
        // temp="cent rftest 1\r";
        // // goldenSerial.write(temp);
        // Sleep(6);

        // // enable data whiten
        // // temp="cent  dwen 1\r";
        // // dutSerial.write(temp);
        // // Sleep(10);

        // m_RX_Len = 0;
        // memset(m_RX_Buf, 0, 256);
        // // reboot
        // // temp="reboot\r";
        // // goldenSerial.write(temp);
        // // Sleep(3000);
        // // sent test command
        // temp = "\r";
        // // goldenSerial.write(temp);
        // while (1)
        // {
        //     memset(str,0,256);
        //     len = goldenSerial.read((uint8_t *)str, 256);
        //     if (len)
        //     {
        //         rs = Search_Specified_Field("vc#", str, len);
        //         if (rs)
        //             return 1;
        //     }
        //     else
        //     {
        //         break;
        //     }
        // }

        return 0;
    }
    else if(deviceno==DUT_DEVICE)
	{
		// force the device into rftest mode
        temp="cent  rftest 1\r";
        dutSerial.write(temp);
        Sleep(10);

        // // enable data whiten
        // temp="cent  dwen 1\r";
        // dutSerial.write(temp);
        // Sleep(10);

        // temp="cent  band 915000000\r";
        // dutSerial.write(temp);
        // Sleep(10);

        // temp="cent  mode 1000\r";
        // dutSerial.write(temp);
        // Sleep(10);

        // temp="cent  rate 50000\r";
        // dutSerial.write(temp);
        // Sleep(10);

        // temp = "cent  txc f6 1 0\r";
        // dutSerial.write(temp);
        // Sleep(10);

        while (1)
        {
            memset(str,0,256);
            len = dutSerial.read((uint8_t *)str, 256);
            if(len==0x00)
                break;
        }
        
        m_RX_Len = 0;
        memset(m_RX_Buf, 0, 256);
        // reboot
        // temp="reboot\r";
        // dutSerial.write(temp);
        // Sleep(3000);
        // sent test command
        temp = "\r";
        dutSerial.write(temp);
        while (1)
        {
            memset(str,0,256);
            len = dutSerial.read((uint8_t *)str, 256);
            if (len)
            {
                // printf(str);
                rs = Search_Specified_Field("vc#", str, len);
                if (rs)
                    return 1;
            }
            else
            {
                break;
            }
        }

        return 0;
		
	}
	else if(deviceno==VSA_DEVICE)
	{
		return 0;
	}
	else
	{
		return 0;
	}
}

/**
 * @brief search specified field in the stream of string
 * 
 * @param char 
 * @param str 
 * @param len 
 * @return int 1: find out the string
 *             0: do not find out the string
 */
int Search_Specified_Field(const char *p, char *str, int len)
{
    int i;
    int rs = 0;
    for (i = 0; i < len; i++)
    {
        // copy valid character to buffer
        if ((str[i] != 13) && (str[i] != 10))
        {
            m_RX_Buf[m_RX_Len++] = str[i];
        }
        else // encount return line
        {
            // the current string is valid for analyse
            // if (strncmp("vc#", m_RX_Buf, strlen("vc#")) == 0)
            if (strncmp(p, m_RX_Buf, strlen(p)) == 0)
            {
                rs = 1;
                break;
            }

            memset(m_RX_Buf, 0, 256);
            m_RX_Len = 0;
        }

        if (i == (len - 1))
        {
            if (strncmp(p, m_RX_Buf, strlen(p)) == 0)
            {
                rs = 1;
            }
        }
    }

    return rs;
}

/**
 * @brief Get the Serial Number
 * 
 * @return int 
 */
int Get_Serial_Number(void)
{
    int rs = 0x00;
    std::string dut;
    char str[256];
    int strlen;
    // 	set the parameter of the golden of RX
    if (dutSerial.isOpen()) //dutSerial
    {
        // dut = "cent band 915000000\r";
        // dutSerial.write(dut);
        // Sleep(5);

        // dut = "cent mode 1000\r";
        // dutSerial.write(dut);
        // Sleep(5);
        // get the serial number
        dut = "cent nv series_number\r";
        dutSerial.write(dut);
        // Sleep(5);
        while (1)
        {
            memset(str, 0, 256);
            strlen = dutSerial.read((uint8_t *)str, 256);
            if (strlen)
            {
                if (m_Debug == '1')
                    printf(str);

                rs = Grab_Serial_Number(str, strlen);
                if(rs==0x01)
                {
                     printf(m_Serial_Number);
                     printf("\n");
                }
                   
                return rs;
            }

            else
                break;
        }
    }

    return rs;
}

/**
 * @brief Grab Serial Number
 * 
 * @param str 
 * @param len 
 * @return int 
 */
int Grab_Serial_Number(char *str,int len)
{
    int i,k;
	// int sign=1;
	// int rssi_value;
    // int rs=0x00;

    for (i = 0; i < len; i++)
    {
        // copy vaild character to buffer
        if ((str[i] != 13) && (str[i] != 10))
        {
            m_RX_Buf[m_RX_Len++] = str[i];
        }
        else // encount return line
        {
            // the current string is valid for analyse
            if (strncmp("series_number", m_RX_Buf, strlen("series_number")) == 0)
            {
                // char m_Serial_Number[20];
                // int m_Serial_len;
                memset(m_Serial_Number, 0, 20);
                m_Serial_len = 0;
                k = strlen("series_number");
                while (k < m_RX_Len)
                {
                    if ((m_RX_Buf[k] >= '0') && (m_RX_Buf[k] <= '9'))
                        m_Serial_Number[m_Serial_len++] = m_RX_Buf[k];
                    k++;
                }

                // get the real rssi value
                // rssi_value *= sign;
                // if (m_nRssiCount < 50)
                //     m_nCalRSSi[m_nRssiCount++] = rssi_value;

                return 0x01;
            }
            // else
            // {
            // 	str1="none rssi string found\n"));
            // }

            memset(m_RX_Buf, 0, 256);
            m_RX_Len = 0;

            // while (++i < len)
            // {
            //     if ((str[i] != 13) && (str[i] != 10))
            //     {
            //         i--;
            //         break;
            //     }
            // }
        }
    }
    return 0x00;
}

/**
 * @brief Get Ip Address
 * 
 * @return int 
 */
int CIU_Get_IP_Address(void)
{
    int nRetCode = 0;
    int rs=0;
    CStdioFile newfile;
    BOOL rs1;
    char yy[100];
    int len;
    int i, k;

    rs1 = newfile.Open(".\\config\\api_config.txt", CStdioFile::modeRead, NULL);
    if (rs1)
    {

        while (newfile.ReadString(yy, 100))
        {
            len = strlen("WT_IP_ADDRESS=");
            //get the ip address of the tester
            if (strncmp("WT_IP_ADDRESS=", yy, len) == 0)
            {
                memset(m_IpAddress, 0, 32);
                i = len;
                k = 0;
                while (yy[i] != '\r' && yy[i]!='\n')
                {
                    if (yy[i] == ' ')
                    {
                        break;                      
                    }
                    else
                    {
                        m_IpAddress[k++] = yy[i];
                    }
                    i++;
                }
                printf("Junfeng.wei get the ip address is:");
                printf(m_IpAddress);
                printf("\n");
                rs=1;
            }
        }
    }
    newfile.Close();
    return rs;
}
/**
 * @brief 
 * 
 * @return int 
 */
int CIU_Get_VSA_IP_Address(void)
{
    int nRetCode = 0;
    int rs=0;
    CStdioFile newfile;
    BOOL rs1;
    char yy[100];
    int len;
    int i, k;

    rs1 = newfile.Open(".\\config\\api_config.txt", CStdioFile::modeRead, NULL);
    if (rs1)
    {

        while (newfile.ReadString(yy, 100))
        {
            len = strlen("VSA_IP_ADDRESS=");
            //get the ip address of the tester
            if (strncmp("VSA_IP_ADDRESS=", yy, len) == 0)
            {
                memset(m_VSA_IpAddress, 0, 32);
                i = len;
                k = 0;
                while (yy[i] != '\r' && yy[i]!='\n')
                {
                    if (yy[i] == ' ')
                    {
                        break;                      
                    }
                    else
                    {
                        m_VSA_IpAddress[k++] = yy[i];
                    }
                    i++;
                }
                printf("Junfeng.wei get the ip address is:");
                printf(m_VSA_IpAddress);
                printf("\n");
                rs=1;
            }
        }
    }
    newfile.Close();
    return rs;
}
/**
 * @brief force the DUT enter test mode and config the DUT
 * 
 * @return int 
 */
int CIU_Force_To_Test(void)
{
    int len;
    char str[256];
    std::string dut;
    //	set the parameter of the DUT of TX
    if (dutSerial.isOpen())
    {
        // enter rftest mode by write flash
        dut = "cent nv rftest 1\r";
        dutSerial.write(dut);
        Sleep(3);

        // dut = "reboot\r";
        // dutSerial.write(dut);
        // Sleep(2000);

        // enter rftest mode by write ram
        dut = "cent  rftest 1\r";
        dutSerial.write(dut);
        Sleep(3);

        // cent dwen 0
        // cent nv carrier_frequency 915000000.000000
        // cent nv fe_hw_cfg 1
        // cent nv tx_power_default 20
        // cent nv pa_sel 1
        dut = "cent dwen 0\r";
        dutSerial.write(dut);
        Sleep(5);
        dut = "cent nv carrier_frequency 915000000\r";
        dutSerial.write(dut);
        Sleep(5);
        dut = "cent nv fe_hw_cfg 1\r";
        dutSerial.write(dut);
        Sleep(5);
        dut = "cent nv tx_power_default 20\r";
        dutSerial.write(dut);
        Sleep(5);
        dut = "cent nv pa_sel 1\r";
        dutSerial.write(dut);
        Sleep(5);

        // set elna_rssi_offset to 22
        dut = "cent nv elna_rssi_offset 22\r";
        dutSerial.write(dut);
        Sleep(3);

        // set rssi_offset to 23
        dut = "cent nv rssi_offset 23\r";
        dutSerial.write(dut);
        Sleep(5);

        // clear the rx buffer of PC
        while (1)
        {
            memset(str, 0, 256);
            len = dutSerial.read((uint8_t *)str, 256);
            if (len == 0x00)
                break;
        }
        return 1;
    }
    return 0;
}

void CIU_Exit_RFTest(void)
{
    string dut;
    dut = "cent nv rftest 0\r";
    dutSerial.write(dut);
    Sleep(5);
}
/**
 * @brief DUT tx continous wave the frequency is 915M
 * 
 * @return int 
 */
int CIU_Tx_CW(void)
{
    int len;
    char str[256];
    std::string dut;
    //	set the parameter of the DUT of TX
    if (dutSerial.isOpen())
    {
        // set band to 915M
        dut = "cent band 915000000\r";
        dutSerial.write(dut);
        Sleep(3);

        // start transmit continous wave
        dut = "cent txcw 1\r";
        dutSerial.write(dut);
        Sleep(1);

        while (1)
        {
            memset(str, 0, 256);
            len = dutSerial.read((uint8_t *)str, 256);
            if (len == 0x00)
                break;
        }
        return 1;
    }
    return 0;
}

int CIU_Stop_CW(void)
{
    int len;
    char str[256];
    std::string dut;
    //	set the parameter of the DUT of TX
    if (dutSerial.isOpen())
    {
        // start transmit continous wave
        dut = "cent txcw 0\r";
        dutSerial.write(dut);
        Sleep(1);

        while (1)
        {
            memset(str, 0, 256);
            len = dutSerial.read((uint8_t *)str, 256);
            if (len == 0x00)
                break;
        }
        return 1;
    }
    return 0;
}
int CIU_Manu_Calibration_Xtal(void)
{
    char buf[30];
    char xx[30];
    char xx1[30];
    std::string dut;
    int rs;

    // dut transmit single wave
    rs = CIU_Tx_CW();
    if (rs == 0)
        return 0;

    // manual calibration the xtal_offset
    do
    {
        fgets(buf,30,stdin);
        printf(buf);
        if(strncmp("999",buf,3)==0)
        {
            printf("Calibration finished\n");
            //printf(dut.c_str());
            sprintf(xx,"cent nv xtal_offset %s\r",xx1);
            dut=xx;
            printf(dut.c_str());
            dutSerial.write(dut);
            Sleep(3);
            break;
        }
        else
        {
            sprintf(xx, "cent dctune %s\r", buf);
            printf(xx);
            dut = xx;
            dutSerial.write(dut);
            memset(xx1,0,30);
            strcpy(xx1,buf);
            Sleep(3);
        }
        
    } while (1);

    //stop txcw
    CIU_Stop_CW();
    
}
/**
 * @brief Calibration the frequency of the CIU
 * 
 * @return int 
 */
int CIU_Frequency_Calibration(void)
{
    int i, k;
    int rs;
    int res = 0;
    char xx[100];
    double fre1;
    uint8_t offset;
    string dut;
    int adjust=0;
    // get frequency range
    CIU_Get_Frequency_Range();

    //dut transmit single wave
    rs = CIU_Tx_CW();
    if (rs == 0)
        return 0;
    
    printf("\t\tdut is transmit the single wave\n");
    
    // Calibration the xtal offset
    offset = m_Frequency_low;
    k = 0;
    do
    {

        sprintf_s(xx, "cent dctune %d\r", offset);
        dut = xx;
        dutSerial.write(dut);
        rs = c_tester.Grab_CW_Frequency_and_power();
        if (rs == 0)
        {
            printf("Grab the frequency and power failed,please check\n");
            break;
        }
        else
        {
            // the max adjustment is 20 times
            k++;
            if (k >= 20)
            {
                res=0;
                printf("\tthe frequency calibration over 20times\n");
                break;
            }
                

            fre1 = c_tester.m_freq - 915 * 1e6;
            // check the deviation is right or left
            if (k == 1)
            {
                if (fre1 >= 0)
                    adjust = 1;
                else
                    adjust = 0;
            }

            if (fre1 >= 0) // right shift
            {
                if (adjust == 1)
                {
                    if (c_tester.m_freq_offset >= 20000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 2.0);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 15000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 2.0);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 10000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 2.0);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 5000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 1.5);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 3000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 1.5);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 2000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 1.5);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 1000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 1.0);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 0.00)
                    {
                        res = 1;
                        break;
                    }
                    else
                    {
                        res = 1;
                        break;
                    }
                }
            }
            else //if (fre1 < 0) // left shift
            {
                if (adjust == 0)
                {
                    if (c_tester.m_freq_offset >= 20000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 2.0);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 15000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 2.0);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 10000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 2.0);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 5000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 1.5);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 3000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 1.5);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 2000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 1.5);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 1000.00)
                    {
                        i = int(c_tester.m_freq_offset / 1000);
                        i = int(i * 1.0);
                        offset += i;
                        if (offset >= 128)
                            offset = 128;
                    }
                    else if (c_tester.m_freq_offset >= 0.00)
                    {
                        res = 1;
                        break;
                    }
                    else
                    {
                        res = 1;
                        break;
                    }
                }
                else 
                {
                    if(i>=30) // diff is more than 15k
                    {
                        offset-=10;
                    }
                    else if(i>=20) //diff is more than 10k
                    {
                        offset-=8;
                    }
                    else if(i>=10) // diff is more than 5k
                    {
                        offset-=4; //3 is option
                    }
                    else // differ is less than 3k
                    {
                        offset=-2;// 1 is option
                    }
                    

                }
               
            }
            // else // fre1=0; the same as m_freq_offset=0;
            // {
            //     res = 1;
            //     break;
            // }
        }

    } while (1);

    // for(i=0;i<2;i++)
    // {
    //     // set the xtal_offset to adjust the frequency
    //     sprintf_s(xx, "cent dctune %d\r", i);
    //     dut = xx;
    //     dutSerial.write(dut);
    //     rs = c_tester.Grab_CW_Frequency_and_power();
    //     if(rs==0)
    //     {
    //         printf("grab single wave frequency and power failed for %time\n",i);
    //         break;
    //     }
    //     else
    //     {
    //         printf("---------------------------------\n\n");
    //         printf("grab single wave frequency and pwoer successfully\n");
    //         printf("\tthe frequency is %lf\n",fabs(c_tester.m_freq));
    //         printf("\tthe obw is %lf\n",c_tester.m_obw);
    //         printf("\tthe frequency offset is %lf\n",c_tester.m_freq_offset);
    //         printf("\tthe power is %lf\n",c_tester.m_dut_peek_power);
    //         printf("\t\t\t the frequency differ is %lf\n", fabs( fabs(c_tester.m_freq) - fabs(915 * 1e6)));
    //     }
    // }
   

    //stop txcw
    CIU_Stop_CW();

    if(rs)
    {
        if (res == 1)
        {
            // write xtal_offset
            sprintf_s(xx, "cent nv xtal_offset %d\r", offset);
            dut = xx;
            dutSerial.write(dut);
            Sleep(2);
            printf("cnet nv xtal_offset %d\n",offset);
            // write power
            printf("   TX_[0]  Pow: %lfdBm [ 0]            ( 16.50 .....O............  22.00)\n",c_tester.m_dut_peek_power);
            if(c_tester.m_dut_peek_power>=14.5)
            {
                if(c_tester.m_dut_peek_power>=20.00)
                {
                    c_tester.m_dut_peek_power = 20.00;
                }
                //sprintf_s(xx, "cent nv tx_power_cal_offset %lf\r", c_tester.m_dut_peek_power*2);
                sprintf_s(xx, "cent nv tx_power_cal_offset %d\r", int(c_tester.m_dut_peek_power*2));
                dut = xx;
                dutSerial.write(dut);
                Sleep(5);


            }
            else
            {
                printf("TX Power %lfdbm is too Lower,the \n",c_tester.m_dut_peek_power);
                res=3;
            }
            
        }
    }

    
    
    return res;
}
/**
 * @brief CIU Reset
 * 
*/
void CIU_Reset(void)
{
    char str[256];
    int len;
    string dut;
    dut = "reboot\r";
    dutSerial.write(dut);
    Sleep(2000);

    // clear rx buffer of dut serial port
    while (1)
    {
        memset(str, 0, 256);
        len = dutSerial.read((uint8_t *)str, 256);
        if (len == 0x00)
            break;
    }
}
/**
 * @brief Verify TX
 * 
 * @return int 
 */
int CIU_Verify_TX(void)
{
    std::string dut;
    std::string golden;
    char str[256];
    int len;
    int rs=0x00;
    //	set dut to tx modulation data
    if(dutSerial.isOpen()==false)
    {
        printf("dut serial port is not open\n");
        return 0;
    }
    printf("dut serial port open successfully\n");
    // if (dutSerial.isOpen())
    {
        dut = "cent band 915000000\r";
        dutSerial.write(dut);
        Sleep(5);

        // set modulation index to 1.0
        dut = "cent mod 1000\r";
        dutSerial.write(dut);
        Sleep(5);

        // set symbol rate to 50k
        dut = "cent rate 50000\r";
        dutSerial.write(dut);
        Sleep(5);

        // transmit PN9 packet
        dut = "cent txc -p 1FF 2000\r";
        dutSerial.write(dut);
        // Sleep(5);
        // while (1)
        // {
        //     memset(str, 0, 256);
        //     len = dutSerial.read((uint8_t *)str, 256);
        //     if(len==0x00)
        //         break;
        // }
        
    }
    
    // Grab Demodulation data
    rs=c_tester.Grab_FSK_Data();
    if(rs)
    {
        printf("\n\t   TX_[0]     0M  Power[ 0]    : %lfdBm	( 16.50 .O................  22.00)\n",c_tester.m_dut_peek_power);
        printf("\t   TX_[0]     0M  FreqErr      : %lfHz	(-40.00 ........O.........  40.00)\n",c_tester.m_freq_offset);
        // check the peak power
        if(c_tester.m_dut_peek_power<=14.0)
            rs=0;
    }

    // dut stop to transmit
    dut = "cent txc f6 1 0\r";
    dutSerial.write(dut);
    Sleep(3);

    return rs;
}

int CIU_Verify_RX(void)
{
    std::string dut;
    // std::string golden;
    int i = 0;
    int k = 0;
    int rssi;
    // char xx[30];
    char str[256];
    int len;

    // 	set the parameter of the golden of RX
    if(dutSerial.isOpen()==false)
        return 0;

    // clear the ber count
    dut = "cent ber -c\r";
    dutSerial.write(dut);
    Sleep(5);

    // disable datawhiten
    dut = "cent dwen 0\r";
    dutSerial.write(dut);
    Sleep(5);
    // sent band frequency
    dut = "cent band 915000000\r";
    dutSerial.write(dut);
    Sleep(5);

    // set modulation index to 1.0
    dut = "cent mod 1000\r";
    dutSerial.write(dut);
    Sleep(5);

    // set symbol rate to 50k
    dut = "cent rate 50000\r";
    dutSerial.write(dut);
    Sleep(5);


    dut = "cent rxc f6\r";
    dutSerial.write(dut);
    Sleep(5);

    // reset all buffer and indicator
    memset(m_nRssi, 0, 50);
    memset(m_nLQI, 0, 50);
    memset(m_nGood, 0, 50);
    memset(m_nTotal, 0, 50);
    m_nRssi_cn = 0;
    m_nLQI_cn = 0;
    m_nGood_cn = 0;
    m_nTotal_cn = 0;

    // set the receive packet number is 50
    dut = "cent ber 20\r";
    dutSerial.write(dut);
    Sleep(2);
    // clear the rx buffer of pc
    // do
    // {
    //     memset(str, 0, 256);
    //     len = dutSerial.read((uint8_t *)str, 256);
    //     if (len == 0)
    //         break;
    // } while (1);

    // start vsg
    c_tester.asyn_vsg();

    // grab packet information
    while (1)
    {
        memset(str, 0, 256);
        len = dutSerial.read((uint8_t *)str, 256);
        printf(str);
        if (len)
            Grab_Good_Packet(str, len);
        else
            break;
    }
    
    
    // stop VSG transmit
    c_tester.stop_vsg();

    //  Grab the rssi value and the good packet
    if (m_nTotal_cn == 20)
    {
        // printf("   the count of good is %d, total is %d, the count of rssi is %d, the count of lqi is %d,", m_nGood_cn, m_nTotal_cn, m_nRssi_cn, m_nLQI_cn);
        rssi = Get_Real_Rssi_Value(m_nRssi, 10);
        printf("\tthe real rssi is %d,\n", rssi);
        printf("\tthe good packet  is %d,\n", m_nGood[m_nGood_cn - 1]);
        printf("\tthe total packet  is %d,\n", m_nTotal[m_nTotal_cn - 1]);
        printf(" the good rate is [%d%%]\n", m_nGood[m_nGood_cn - 1] * 100 / m_nTotal_cn);
        if ((m_nGood[m_nGood_cn - 1] * 100 / m_nTotal_cn) >= 80)
            return 1;
        else
            return 0;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief Grab the good packet
 * 
 * @param str 
 * @param len 
 */
void Grab_Good_Packet(char *str, int len)
{
	int i,k,n;
	int sign=1;
	int rssi_value;
	int lqi=0;
	int ngood,ntotal;
	char temp[10];
	// if (m_bGrabRxPacket)
	{
		for (i = 0; i < len; i++)
		{
			// copy vaild character to buffer
			if ((str[i] != 13) && (str[i] != 10))
			{
				m_RX_Buf[m_RX_Len++] = str[i];
			}
			else // encount return line
			{
				// the current string is valid for analyse
				// grab rssi and lqi:
				// expect len=246, rcvd len=246, rssi=-5, lqi=252
				if (strncmp("   expect len=246, rcvd len=246, rssi=", m_RX_Buf, strlen("   expect len=246, rcvd len=246, rssi=")) == 0)
				{
					k = strlen("   expect len=246, rcvd len=246, rssi=");
					sign = 1;
					n = 0;
					while (k < m_RX_Len)
					{
						if (m_RX_Buf[k] == '-')
						{
							sign = -1;
						}
						else if ((m_RX_Buf[k] >= '0') && (m_RX_Buf[k] <= '9'))
						{
							temp[n++] = m_RX_Buf[k] - '0';
						}
						else if (m_RX_Buf[k] == ',')
						{
							if (n == 3)
								rssi_value = temp[0] * 100 + temp[1] * 10 + temp[2];
							else if (n == 2)
								rssi_value = temp[0] * 10 + temp[1];
							else if (n == 1)
								rssi_value = temp[0];
							else
								rssi_value = 0;

							rssi_value *= sign;
							m_nRssi[m_nRssi_cn++] = rssi_value;

							// check the lqi field
							if ((k + 5) < m_RX_Len)
							{
								if (m_RX_Buf[k + 1] == ' ')
								{
									if (strncmp(" lqi=", m_RX_Buf + k + 1, strlen(" lqi=")) == 0)
									{
										n = 0;
										k += 5;
									}
								}
							}
						}
						else
							;

						k++;
					}
					// get the lqi value
					if (n == 3)
						lqi = temp[0] * 100 + temp[1] * 10 + temp[2];
					else if (n == 2)
						lqi = temp[0] * 10 + temp[1];
					else if (n == 1)
						lqi = temp[0];
					else
						lqi = 0;
					m_nLQI[m_nLQI_cn++] = lqi;
					//save to global array
				}

				// grab the good packet count and current receive total packet count
				// good/count=1/1, fcs err 0, len err 0, ber=0/1968
				if (strncmp("   good/count=", m_RX_Buf, strlen("   good/count=")) == 0)
				{

					k = strlen("   good/count=");
					n = 0;
					while (k < m_RX_Len)
					{

						if ((m_RX_Buf[k] >= '0') && (m_RX_Buf[k] <= '9'))
						{
							temp[n++] = m_RX_Buf[k] - '0';
						}
						else if (m_RX_Buf[k] == '/')
						{
							if (n == 3)
								ngood = temp[0] * 100 + temp[1] * 10 + temp[2];
							else if (n == 2)
								ngood = temp[0] * 10 + temp[1];
							else if (n == 1)
								ngood = temp[0];
							else
								ngood = 0;
                            
							m_nGood[m_nGood_cn++] = ngood;
                            printf("the good value is %d\n",ngood);
							n = 0;
						}
						else if (m_RX_Buf[k] == ',')
						{
							if (n == 3)
								ntotal = temp[0] * 100 + temp[1] * 10 + temp[2];
							else if (n == 2)
								ntotal = temp[0] * 10 + temp[1];
							else if (n == 1)
								ntotal = temp[0];
							else
								ntotal = 0;

							m_nTotal[m_nTotal_cn++] = ntotal;

							//break;
							k = m_RX_Len;
						}
						else
							;

						k++;
					}
				}

				memset(m_RX_Buf, 0, 256);
				m_RX_Len = 0;
			}
		}
	}
}


/**
 * @brief // Get the Real Rssi Value
 * 
 * @param p 
 * @param len default the len is 50
 * @return int 
 */
int Get_Real_Rssi_Value(int * p, int len)
{
	// unsigned char *p1,*p2;
	// p1= new unsigned char [len];
	// p2= new unsigned char [len];
	unsigned char p1[50];
	unsigned char p2[50];
	int i,k,n;
	int xx;
	CString temp;

	// reset all value to 0
	for (i = 0; i < len; i++)
	{
		p1[i] = 0;
		p2[i]=0;
	}
		

	// statistic from the first array element
	n = 1;
	for (i = 0; i < len; i++)
	{
		// the current element is not statistic
		if (p1[i] == 0)
		{
			xx = p[i];
			p1[i] = n;
			p2[n - 1] = 1;
			for (k = i + 1; k < len; k++)
			{
				if (p[k] == xx)
				{
					p1[k] = n;
					p2[n - 1]++;
				}
			}
			n++;
		}
	}
    
	// analyse the result of the statistic
	// find out the max value( the most precense of the value)
	xx=p2[0];
	k=0;
	for(i=1;i<n;i++)
	{
		if(p2[i-1]>xx)
		{
			xx=p2[i-1];
			k=i-1;
		}
		// temp.Format("the %d is %d",i-1,p2[i-1]);
		// AfxMessageBox(temp);
	}
	k++;

	// find the index of  precense value 
	for(i=0;i<len;i++)
	{
		if(p1[i]==k)
		{
			break;
		}
	}

    // return the value
	// delete []p1;
	// delete []p2;

	// temp.Format("the is %d",p[i]);
	// 	printf(temp);
	return p[i];
	
}

