#include "stdafx.h"
#include "harmwee.h"
#include "serial\serial.h"
#include <shellapi.h>
#include "WT208_Tester.h"
#include "Vertex_CIU.h"
#include "vsg_vsa.h"

using namespace serial;
using namespace std;
//Serial dutSerial("", 115200, Timeout::simpleTimeout(1000)); // just create the serial port than do not open the port because do not specified the com name
extern Serial dutSerial;
extern char m_RX_Buf[256];// Serial Port Receive buffer
extern WT208_Tester c_tester;
extern int m_RX_Len;// Serial Port Receive character length
extern int m_nRssi[50];
extern int m_nRssi_cn ;
extern char dut_com[10];

unsigned char m_Serial_Open=0;

int ngood,nCount,ntotal;

extern void Init_Itest_library(void);
extern void Realse_Itest_library(void);

/**
 * @brief download the firmware to external spi flash
 * 
 */
void Nxp_Download_Firmware(void)
{
    int rs;
    // CStdioFile newfile;
    FILE *newfile;
	BOOL rs1;
    char yy[100];

    // ShellExecute(NULL,"open","C:\\program.bat","","",SW_SHOW);
    ShellExecute(NULL,"open","C:\\program.bat","","",SW_HIDE);
    Sleep(100);
    while(INVALID_HANDLE_VALUE == CreateFile("C:\\exitcode.txt", 0, 0, NULL, OPEN_EXISTING, 0, NULL))
    {
         printf("...downloading..\r\n");
         Sleep(200);
    }
    printf("download finished\r\n");

    // if (INVALID_HANDLE_VALUE != CreateFile(
    //         "D:\\exitcode.txt", 0, 0, NULL, OPEN_EXISTING, 0, NULL))
    // {
    //     AfxMessageBox("File exists\n");
    // }
    newfile=fopen("c:\\exitcode.txt","r");
    if(newfile!=NULL)
    {
        while( fgets(yy, 100, newfile) != NULL ) 
        {
            // printf(yy);
            if (strncmp("1", yy, strlen("1")) == 0)
            {
                printf("downlaod firmware successfully\r\n");
                break;
            }

            if(strncmp("0", yy, strlen("0"))==0)
            {
                printf("downlaod firmware failed\r\n");
                break;
            }
        }
    }
    fclose(newfile);

    // rs1 = newfile.Open("c:\\exitcode.txt", CStdioFile::modeRead, NULL);
    // if (rs1)
    // {
    //     while (newfile.ReadString(yy, 100))
    //     {
    //         // get the  serial number of DUT
    //         if (strncmp("1", yy, strlen("1")) == 0)
    //         {
    //             printf("downlaod firmware successfully\r\n");
    //             break;
    //         }

    //         if(strncmp("0", yy, strlen("0"))==0)
    //         {
    //             printf("downlaod firmware failed\r\n");
    //             break;
    //         }
    //     }

    // }
    // else
    // {
    //      printf("downlaod firmware failed\r\n");
    // }
    
    // newfile.Close();
}
/**
 * @brief harmwee nic test flow chart by VSG and VSA
 * 
 */
void harwwee_test_by_VSGVSA1(void)
{
    int rs;
    double f, p;

    // Vsa_Switch2Tx();
    // dut_Start_SingleWave();
    Vsa_Peak_Frequency(&f, &p);
    // dut_Stop_SingleWave();
    //if ((p >= 28.0) && (abs(915000000-f)<=3000) )
    if (p < 28.0)
    {
        printf("single wave test failed\n");
        return;
    }
    // printf("single wave test successfully\n");
    printf("Verify tx power of 915m successfully\n");
}
void harwwee_test_by_VSGVSA(void)
{
    int rs;
    double f, p;
    int i;

    // step 1:open the Serial Port of dut
    rs = Nxp_Open_Serial();
    if (rs == 0)
    {
        printf("open serial port failed\r\n");
        return;
    }
    printf("open serial port successfully\r\n");
    // open the serial port of switch
    rs = Vsa_Open_Switch_Port();
    if (rs == 0)
    {
        printf("open serial port of switch failure\n");
        return;
    }
    printf("open serial port of switch successfully\n");

    // force to enter factory mode
    Nxp_Enter_Factory_Mode();

    // step 2: Verify the Dut
    rs = Nxp_Verify_DUT();
    if (rs == 0)
    {
        printf("Verify DUT failed\r\n");
        return;
    }
    printf("Verify DUT successfully\r\n");

    // step 3:calibration the rssi when the signal of vsg is -50dbm
    Calibration_50dbm_Rssi();

    // step 4:calibration the rssi when the signal of vsg is -100dbm
    Calibration_100dbm_Rssi();

    // step 5: single wave test frequency and tx power
    Vsa_Switch2Tx();
    dut_Start_SingleWave();
    // Sleep(1500);
    Sleep(10);
    //  for(i=0;i<2;i++)
    {
        Vsa_Peak_Frequency(&f, &p);

        if ((p >= 26.00) && (abs(915000000 - f) <= 3000))
        {
            ;// break;
        }
        else
        {
            printf("single wave test failed\n");
            return;
            // if (i == 9)
            //     return;
        }
        // Sleep(10);
    }
    dut_Stop_SingleWave();
    if (p >= 26.0)
    {
        printf("----------single wave test successfully\n");
        printf("----------Verify tx power of 915m successfully\n");
    }

    // step 6: verify sensitivity
    // Vsa_Reset_dut();
    // Sleep(3000); 

    Vsa_Switch2Rx();
    rs = Verify_Sensitivity();
    if (rs == 0)
    {
        printf("verify sensitivity failed\n");
        return;
    }
    printf("---------verify sensitivity successfully\n");

    // step 8: exit factory mode
    rs = Nxp_Exit_Factory_Mode();
    if (rs)
    {
        printf("exit factory mode successfully\n");
    }
    else
    {
        printf("exit factory mode failed\n");
        return ;
    }

    // step 9: clsoe serial port
    dutSerial.close();
    // switchSerial.close();

    //print all items passed
    printf("-------------\n*************all items test passed.*****************\n");

    //the code below is used to test
    // while(1)
    // {
    //     ;
    // }
}

void harmwee_test(char *p)
{
    int rs;
    int ret = 0;
    int i;
    // step 1:open the Serial Port
    if (m_Serial_Open == 0)
    {
        Nxp_Open_Serial();
    }
    //open serial port failed and return
    if (m_Serial_Open == 0)
    {
        printf("open serial port failed\r\n");
        return;
    }

    printf("open serial port successfully\r\n");

    // force to enter factory mode
    Nxp_Enter_Factory_Mode();

    // step 7:write configuration information
    // rs=Nxp_Write_Config_info(p);
    // if(rs)
    // {
    //     printf("write configuration successfully\n");
    // }
    // else
    // {
    //     printf("write configuration failed\n");
    //     return ;
    // }


    // step 2: Verify the Dut
    rs=Nxp_Verify_DUT();
    if (rs==1)
    {
        printf("Verify DUT successfully\r\n");
    }
    else
    {
        printf("Verify DUT failed\r\n");
        return ;
    }

    Init_Itest_library();

    // init tester
    // ret=c_tester.init();
    // if(ret==false)
    // {
    //     printf("connect tester failed\n");
    //     printf("failed\n");
    //     return ;
    // }
    // printf("connect itest_wt208c successfully\n");

    // step 3:calibration the rssi when the signal of vsg is -50dbm
    // c_tester.asyn_vsg(-50.0);// start vsg
    //  Calibration_50dbm_Rssi();
    // c_tester.stop_vsg();// stop VSG transmit
    // do
    // {
    //     ;
    // } while (1);
    
    // step 4:calibration the rssi when the signal of vsg is -100dbm
    // c_tester.asyn_vsg(-100.0);// start vsg
    // Calibration_100dbm_Rssi();
    // c_tester.stop_vsg();// stop VSG transmit
    // while (1)
    // {
    //     ;
    // }
    

    // step 5: single wave test frequency and tx power
    //  rs = Verify_Frequency_And_TxPower();
    //  if (rs == 0)
    //      return;
    //  printf("Verify tx power of 915m successfully\n");

    for (i = 0; i < 30; i++)
    {
        rs = Verify_Frequency_And_TxPower();
        if (rs == 1)
            break;
        else
        {
            Sleep(10);
        }
        
    }

    if(rs==0)
    {
        printf("single wave test failed\n");
        return;
    }
    else
    {
        printf("Verify tx power of 915m successfully\n");
    }
    

    
    // step 6: verify sensitivity
    // c_tester.asyn_vsg(-106.0);// start vsg with -106dBm 
    // rs=Verify_Sensitivity();
    // c_tester.stop_vsg();// stop VSG transmit
    // if(rs)
    // {
    //     printf("verify sensitivity successfully\n");
    // }
    // else
    // {
    //     printf("verify sensitivity failed\n");
    //     return ;
    // }

    

    // step 8: exit factory mode
    rs=Nxp_Exit_Factory_Mode();
    if(rs)
    {
        printf("exit factory mode successfully\n");

        //printf("\n all tests pass\n");
    }
    // else
    // {
    //     printf("exit factory mode failed\n");
    //     printf("\n tests failed\n");
    //     return ;
    // }


    
    
    // step 9: clsoe serial port
    if(dutSerial.isOpen())
    {
        dutSerial.close();
    }

    // releaes itest library
    Realse_Itest_library();
    //print all items passed
    printf("all items test passed.\n");

    //the code below is used to test
    // while(1)
    // {
    //     ;
    // }
    
}
/**
 * @brief Verify Serial Port
 * 
 */
int Nxp_Open_Serial(void)
{
    CIU_Get_DUT_Port();
    printf(dut_com);
    printf("\n");
    dutSerial.setPort(dut_com);
    //dutSerial.setPort("COM9");
    dutSerial.open();
    dutSerial.setTimeout(serial::Timeout::max(), 100, 0, 100, 0); // set timeout is 100ms
    if (dutSerial.isOpen())
    {
        m_Serial_Open = 1;
        return 1;
    }
    else
    {
        return 0;
    }
}
/**
 * @brief Verify DUT
 * 
 */
int Nxp_Verify_DUT(void)
{
    string temp;
    int len;
    char str[256];
    int rs;

    // clear the rx buffer of the serial port
    dutSerial.flush();
    // while (1)
    // {
    //     memset(str, 0, 256);
    //     len = dutSerial.read((uint8_t *)str, 256);
    //     if (len == 0x00)
    //         break;
    // }

    // reset the rx buffer 
    m_RX_Len = 0;
    memset(m_RX_Buf, 0, 256);

    // send default empty command
    temp = "\r\r";
    dutSerial.write(temp);
    Sleep(10);

    // get the response and veirfy
    while (1)
    {
        memset(str, 0, 256);
        len = dutSerial.read((uint8_t *)str, 256);
        if (len)
        {
            // printf(str);
            rs = Search_Specified_Field("[2K/>", str, len);
            if (rs)
                return 1;
        }
        else
        {
            break;
        }
    }
    // return value
    return 0;
}
/**
 * @brief enter factory mode
 * 
 */
int Nxp_Enter_Factory_Mode(void)
{
    // enter factory mode
    char str[256];
    int len;
    int i;
    string temp;
    // temp="rf_factory_mode_set  1\r";
    // dutSerial.flush();
    // dutSerial.write(temp);
    // Sleep(10);

    //reboot
    // temp = "reboot\r";
    // dutSerial.write(temp);
    // Vsa_Reset_dut();
    // Sleep(2000); // two seconds 
    Vsa_Reset_dut();
    Sleep(3500); // two seconds 
    dutSerial.flush();

    temp="rf_factory_mode_set  1\r";
    // dutSerial.flush();
    dutSerial.write(temp);
    Sleep(100);

    // print serial message
    while (1)
    {
        memset(str, 0, 256);
        len = dutSerial.read((uint8_t *)str, 256);
        if (len == 0)
            break;
        else
        {

            printf("\n----------------------the current time is :%d ms:-------------\n", GetTickCount());
            printf(str);
        }
            
    }

    
    // temp="rf_factory_mode_set  1\r";
    // dutSerial.flush();
    // dutSerial.write(temp);
    // Sleep(10);

    // temp="rf_factory_mode_set  1\r";
    // dutSerial.flush();
    // dutSerial.write(temp);
    // Sleep(10);

    Vsa_Reset_dut();
    Sleep(3500); // two seconds 
    dutSerial.flush();

    printf("enter factory mode successfully\n");
    // i=0;
    // while (1)
    // {
    //     memset(str, 0, 256);
    //     len = dutSerial.read((uint8_t *)str, 256);
    //     if (len == 0)
    //         break;
    // }
    
    // return value
    return 0x01;
}
/**
 * @brief exit factory mode
 * 
 */
int Nxp_Exit_Factory_Mode(void)
{
    // exit factory mode
    string temp;
    temp="rf_factory_mode_set  0\r";
    dutSerial.write(temp);
    Sleep(20);
    
    //reboot
    // temp = "reboot\r";
    // dutSerial.write(temp);
    // Sleep(2000); // two seconds
    // return value
    return 0x01;
}
/**
 * @brief write configuration information to flash
 * 
 * @return int 
 */
int  Nxp_Write_Config_info(char *p)
{
    // cfg wisun netname hisun_wisun
    // cfg wisun domain   1
    // cfg wisun class    2
    // cfg wisun mode  3

    // cfg wisun network_size 01 // 01:small 08:medium;0f:large ;19:xlarge
    // cfg wisun mac 00:00:01:02:03:04:05:06

    string temp;
    char str[256];
    int len;
    char *p1="01010101010:22";
    // clear the rx buffer of the serial port
    while (1)
    {
        memset(str, 0, 256);
        len = dutSerial.read((uint8_t *)str, 256);
        if (len == 0)
            break;
    }

    // write netname
    // temp="cfg wisun netname hisun_wisun\r";
    // dutSerial.write(temp);
    // Sleep(10);

    // // write domain
    // temp="cfg wisun domain 1\r";
    // dutSerial.write(temp);
    // Sleep(10);

    // // write class
    // temp="cfg wisun class 2\r";
    // dutSerial.write(temp);
    // Sleep(10);

    // // write mode
    // temp="cfg wisun mode 3\r";
    // dutSerial.write(temp);
    // Sleep(10);

    // // write network size
    // temp="cfg wisun network_size 01\r";
    // dutSerial.write(temp);
    // Sleep(10);

    // write wisun mac
    if(p!=NULL)
    {
        temp="cfg wisun mac ";
        memset(str,0,256);
        strcat_s(str,p);
        strcat_s(str,"\r");
        temp.append(str);
        //temp+=str;
        // temp+=p1;
        // temp+="\r";
    }
    else
    {
        temp="cfg wisun mac 00:00:00:00:00:00:00:01\r";
    }

    printf(temp.c_str());
    printf("\n");
    //printf(str);
    dutSerial.write(temp);
    Sleep(10);
    // return value
    return 0x01;

}
/**
 * @brief Verify the sensitivity
 * 
 */
int Verify_Sensitivity(void)
{
    string temp;
    int len;
    char str[256];
    int rs;
    int i,k;
    int no=0;
    // set symbol rate to 50000, the channel space is 200k
    // the channel 0 is 902.2M
    temp="rf_fsk_rate_set 50000\r";
    dutSerial.write(temp);
    Sleep(10);

    // set channel to 64 which frequency is 915M
    temp="rf_ch_set 64\r";
    dutSerial.write(temp);
    Sleep(10);

    // PER test
    no=CIU_Get_TX_Packet_Number();
    sprintf(str,"rf_ber 246 %d\r",no);
    //temp="rf_ber 246 10\r";
    // dutSerial.write(temp);
    temp=str;
    dutSerial.write(temp);
    Sleep(10);

    // get the response and veirfy
    i=0,k=0;
    while (1)
    {
        memset(str, 0, 256);
        len = dutSerial.read((uint8_t *)str, 256);
        if (len)
        {
            // printf(str);
            //rs = Search_Specified_Field("good/count/total=10", str, len);
            rs=Grab_Good_Packet_harmwee( str, len);
            // printf("test the sensitivity\n");
            if (rs)
            {
                // k++;
                // if(k>=10)
                if(ngood*(100/no)>=80)
                {
                    printf("NIC_LOG:");
                    printf("PER is %d%%\n",100-ngood*(100/no));
                    return 1;
                }
                    
                else
                    return 0;
            }
                
        }
        else
        {
            i++;
            Sleep(10);
            if (i >= 100)
            {
                printf("Test sensitivity,but receive nothing\n");
                break;
            }
                
        }
    }
    // abort the receive
    temp = "\3\r";
    dutSerial.write(temp);
    Sleep(10);
    return 0;
}
/**
 * @brief start single wave
 * 
 */
void dut_Start_SingleWave(void)
{
    string temp;
    dutSerial.flush();
    // set symbol rate to 50000, the channel space is 200k
    // the channel 0 is 902.2M
    // temp="rf_fsk_rate_set 50000\r";
    // dutSerial.write(temp);
    // Sleep(10);

    // set channel to 64 which frequency is 915M
    temp="rf_ch_set 64\r";
    dutSerial.write(temp);
    Sleep(10);

    // set tx power to 7 (from 7 to 10)
    //temp="rf_txpwr_set 7\r";
    temp="rf_txpwr_set 5\r";
    printf("NIC_LOG:");
    printf(temp.c_str());
    printf("\n");
    // temp="rf_txpwr_set 8\r";
    // temp="rf_txpwr_set 10\r";
    dutSerial.write(temp);
    Sleep(10);

    // enter single wave mode
    temp="rf_cw\r";
    dutSerial.write(temp);
    Sleep(10);
}
/**
 * @brief stop single wave
 * 
 */
void dut_Stop_SingleWave(void)
{
    string temp;
    dutSerial.flush();

    //exit single wave mode
    temp = "\3\r";
    dutSerial.write(temp);
    Sleep(10);
}
/**
 * @brief Verify single wave frequency and tx power
 * 
 */
int Verify_Frequency_And_TxPower(void)
{
    int rs = 0;
    double f, p;
    float power_low;
    // start single wave
    dut_Start_SingleWave();

    // grab frequency and power from Itest VSA
    Peak_Frequency_Power(&f, &p);
    //if ((p >= 28.0) && (abs(915000000-f)<=3000) )
    power_low=CIU_Get_Tx_Power_Lower();
    // if (p >= 28.0)
    if(p>=power_low)
    {
        printf("single wave test successfully\n");
        rs = 1;
    }
    else
    {
        //printf("single wave test failed\n");
    }

    // stop single wave
    dut_Stop_SingleWave();

    //return value
    return rs;
}
/**
 * @brief Calibration the rssi offset of chip
 * 
 */
int Calibration_50dbm_Rssi(void)
{
    string temp;
    int len;
    char str[256];
    char xx[100];
    int i;
    int rs;
    int rssi;
    int rssi_offset;
    //set default value of rssi offset
    
    temp="rf_bypassopen_rssi_offset_set -4\r";
    dutSerial.write(temp);
    Sleep(20);
    printf(temp.c_str());
    printf("\n");
    printf("scan rssi value of -50dbm successfully\n");
    return 1;
    // clear the rx buffer of the serial port
    // while (1)
    // {
    //     memset(str, 0, 256);
    //     len = dutSerial.read((uint8_t *)str, 256);
    //     if (len == 0x00)
    //         break;
    // }

    // set symbol rate to 50000, the channel space is 200k
    // the channel 0 is 902.2M
    temp="rf_fsk_rate_set 50000\r";
    dutSerial.write(temp);
    Sleep(10);

    // set channel to 64 which frequency is 915M
    temp="rf_ch_set 64\r";
    dutSerial.write(temp);
    Sleep(10);

    // clear the rx buffer of the serial port
    while (1)
    {
        memset(str, 0, 256);
        len = dutSerial.read((uint8_t *)str, 256);
        if (len == 0x00)
            break;
    }
    // reset the rx buffer 
    m_RX_Len = 0;
    memset(m_RX_Buf, 0, 256);

    memset(m_nRssi,0,50);
    m_nRssi_cn = 0;

    //read the current rssi value
    temp="rf_scan\r";
    dutSerial.write(temp);
    i=0;
    do
    {
        memset(str, 0, 256);
        len = dutSerial.read((uint8_t *)str, 256);
        if (len)
        {
            printf(str);
            rs=Scan_Rssi_Value("",str,len);
            if(rs==1)
                i++;
            if(i==5)
            {
                rs=1;
                break;
            }
                
        }
        else
        {
            // break;
        }
        
    } while (1);

    //stop scan :CTRL+C
    temp = "\3\r";
    dutSerial.write(temp);
    Sleep(10);

    // clear the buffer of the serial port
    do
    {
        memset(str,0,256);
        len=dutSerial.read((uint8_t *)str, 256);
        if(len)
        {
            // printf(str);
        }
        else
        {
            break;
        }
        
    } while (1);

    // output the captured rssi value
    for(i=0;i<5;i++)
    {
        sprintf(xx,"rssi is %d,%d\n",m_nRssi[i],m_nRssi[i+10]);
        printf(xx);
    }

    //collection the rssi value and caculate the  average of rssi
    if(rs)
    {
        rssi = 0;
        for (i = 0; i < 5; i++)
        {
            rssi += m_nRssi[i];
        }
        rssi =rssi/5;
        rssi_offset=-50-rssi;

        //calibration the rssi offset
        //printf("rf_bypassopen_rssi_offset_set %d\n", rssi_offset);
        sprintf(xx, "rf_bypassopen_rssi_offset_set %d\n", rssi_offset);
        temp=xx;
        printf(xx);
        dutSerial.write(temp);
        Sleep(10);

    }

    // return
    printf("scan rssi value of -50dbm successfully\n");
    return 1;
}

/**
 * @brief Calibration the external rssi offset 
 * 
 */
int Calibration_100dbm_Rssi(void)
{
    string temp;
    int len;
    char str[256];
    char xx[100];
    int i;
    int rs;
    int rssi;
    int rssi_offset;

    temp="rf_rssi_offset_set 2\r";
    dutSerial.write(temp);
    Sleep(20);
    printf(temp.c_str());
    printf("\n");
    printf("scan rssi value of -100dbm successfully\n");
    return 1;

    // clear the rx buffer of the serial port
    // while (1)
    // {
    //     memset(str, 0, 256);
    //     len = dutSerial.read((uint8_t *)str, 256);
    //     if (len == 0x00)
    //         break;
    // }

    // set symbol rate to 50000, the channel space is 200k
    // the channel 0 is 902.2M
    temp="rf_fsk_rate_set 50000\r\n";
    dutSerial.write(temp);
    Sleep(100);

    printf("-----------------set fsk rate ok\n");
    // set channel to 64 which frequency is 915M
    temp="rf_ch_set 64\r\n";
    dutSerial.write(temp);
    Sleep(100);
    printf("-------------set  channel ok\n");
    // clear the rx buffer of the serial port
    while (1)
    {
        memset(str, 0, 256);
        len = dutSerial.read((uint8_t *)str, 256);
        if (len == 0x00)
            break;
    }
    // reset the rx buffer 
    m_RX_Len = 0;
    memset(m_RX_Buf, 0, 256);

    memset(m_nRssi,0,50);
    m_nRssi_cn = 0;

    //read the current rssi value
    temp="rf_scan\r\n";
    dutSerial.write(temp);
    i=0;
    printf("----------------rf_scan start\n");
    do
    {
        Sleep(10);
        memset(str, 0, 256);
        len = dutSerial.read((uint8_t *)str, 256);
       // printf("*******************\n");
        if (len)
        {
            // printf(str);
            rs=Scan_Rssi_Value("",str,len);
            if(rs==1)
                i++;
            if(i==5)
            {
                rs=1;
                break;
            }
                
        }
        else
        {
            // break;
        }
        
    } while (1);

    temp = "\3\r";
    dutSerial.write(temp);
    Sleep(10);
    printf("------------stop scan\n");
    
    // clear the buffer of the serial port
    do
    {
        memset(str,0,256);
        len=dutSerial.read((uint8_t *)str, 256);
        if(len)
        {
            // printf(str);
        }
        else
        {
            break;
        }
        
    } while (1);


    // output the captured rssi value
    for(i=0;i<5;i++)
    {
        sprintf(xx,"rssi is %d,%d\n",m_nRssi[i],m_nRssi[i+10]);
        printf(xx);
    }

    //collection the rssi value and caculate the  averate of rssi
    if(rs)
    {
        rssi = 0;
        for (i = 0; i < 5; i++)
        {
            rssi += m_nRssi[i];
        }
        rssi =rssi/5;
        rssi_offset=-100-rssi;

        //calibration the rssi offset
        sprintf(xx, "rf_rssi_offset_set %d\n", rssi_offset);
        temp=xx;
        printf(xx);
        dutSerial.write(temp);
        Sleep(10);

    }

    // return
    printf("scan rssi value of -100dbm successfully\n");
    return 1;
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
static int Search_Specified_Field(const char *p, char *str, int len)
{
    int i;
    int rs = 0;
    for (i = 0; i < len; i++)
    {
        // copy valid character to buffer
        if ((str[i] != 13) && (str[i] != 10) && (str[i] != 27) && (str[i] != 32))
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

        // if (i == (len - 1))
        // {
        //     // if (strncmp(p, m_RX_Buf, strlen(p)) == 0)
        //     if (strncmp("[1D", m_RX_Buf, strlen("[1D")) == 0)
        //     {
        //         rs = 1;
        //     }
        // }
    }

    return rs;
}

/**
 * @brief Scan the rssi value
 * 
 * 
 * @param p 
 * @param str 
 * @param len 
 * @return int 
 */
int Scan_Rssi_Value(const char *p, char *str, int len)
{
    //SCAN:(origin_ed,ed,bypass)=(-49,-49,open)
    //SCAN:(origin_ed,ed,bypass)=(-105,-105,close)
    //SCAN:(origin_ed,ed)=(-46,-50,bypass open) //new indicator 20201012
    int i;
    int rs = 0;
    int k, n, m;
    int sign;
    int rssi1, rssi2;
    char temp[10];
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
            //if (strncmp("SCAN:(origin_ed,ed,bypass)=(", m_RX_Buf, strlen("SCAN:(origin_ed,ed,bypass)=(")) == 0)
            if (strncmp("SCAN:(origin_ed,ed)=(", m_RX_Buf, strlen("SCAN:(origin_ed,ed)=(")) == 0)
            {
                k = strlen("SCAN:(origin_ed,ed)=(");
                n = 0;
                sign = 1;
                m=0;
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
                    else if (m_RX_Buf[k] == ')')
                    {
                        // if (n == 3)
                        //     rssi2 = temp[0] * 100 + temp[1] * 10 + temp[2];
                        // else if (n == 2)
                        //     rssi2 = temp[0] * 10 + temp[1];
                        // else if (n == 1)
                        //     rssi2 = temp[0];
                        // else
                        //     rssi2 = 0;

                        // rssi2 *= sign;
                        // // m_nGood[m_nGood_cn++] = ngood;

                        // n = 0;
                        
                    }
                    else if (m_RX_Buf[k] == ',')
                    {
                        m++;
                        //store first Rssi
                        if (m == 1)
                        {
                            if (n == 3)
                                rssi1 = temp[0] * 100 + temp[1] * 10 + temp[2];
                            else if (n == 2)
                                rssi1 = temp[0] * 10 + temp[1];
                            else if (n == 1)
                                rssi1 = temp[0];
                            else
                                rssi1 = 0;

                            rssi1 = rssi1 * sign;

                            n = 0;
                            sign = 1;
                        }
                        //store second Rssi
                        if(m==2)
                        {
                            if (n == 3)
                                rssi2 = temp[0] * 100 + temp[1] * 10 + temp[2];
                            else if (n == 2)
                                rssi2 = temp[0] * 10 + temp[1];
                            else if (n == 1)
                                rssi2 = temp[0];
                            else
                                rssi2 = 0;

                            rssi2 *= sign;
                            // m_nGood[m_nGood_cn++] = ngood;

                            n = 0;
                        }
                    }
                    else
                        ;

                    k++;
                }

                m_nRssi[m_nRssi_cn]=rssi1;
                m_nRssi[m_nRssi_cn+10]=rssi2;
                m_nRssi_cn++;
                rs = 1;
                //break;
            }// one line begin with right rssi header

            memset(m_RX_Buf, 0, 256);
            m_RX_Len = 0;
        }//one line of rssi value

        // if (i == (len - 1))
        // {
        //     if (strncmp("SCAN:(origin_ed,ed,bypass)=(", m_RX_Buf, strlen("SCAN:(origin_ed,ed,bypass)=(")) == 0)
        //     {
        //         rs = 1;
        //     }
        // }
    }

    return rs;
}

/**
 * @brief Scan the rssi value
 * 
 * 
 * @param p 
 * @param str 
 * @param len 
 * @return int 
 */
int Scan_Rssi_Value_old(const char *p, char *str, int len)
{
    //SCAN:(origin_ed,ed)=(-49,-49)
    int i;
    int rs = 0;
    int k, n;
    int sign;
    int rssi1, rssi2;
    char temp[10];
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
            if (strncmp("SCAN:(origin_ed,ed)=(", m_RX_Buf, strlen("SCAN:(origin_ed,ed)=(")) == 0)
            {
                k = strlen("SCAN:(origin_ed,ed)=(");
                n = 0;
                sign = 1;
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
                    else if (m_RX_Buf[k] == ')')
                    {
                        if (n == 3)
                            rssi2 = temp[0] * 100 + temp[1] * 10 + temp[2];
                        else if (n == 2)
                            rssi2 = temp[0] * 10 + temp[1];
                        else if (n == 1)
                            rssi2 = temp[0];
                        else
                            rssi2 = 0;

                        rssi2 *= sign;
                        // m_nGood[m_nGood_cn++] = ngood;

                        n = 0;
                    }
                    else if (m_RX_Buf[k] == ',')
                    {
                        if (n == 3)
                            rssi1 = temp[0] * 100 + temp[1] * 10 + temp[2];
                        else if (n == 2)
                            rssi1 = temp[0] * 10 + temp[1];
                        else if (n == 1)
                            rssi1 = temp[0];
                        else
                            rssi1 = 0;

                        rssi1 = rssi1 * sign;

                        n = 0;
                        sign = 1;

                        
                    }
                    else
                        ;

                    k++;
                }
                m_nRssi[m_nRssi_cn]=rssi1;
                m_nRssi[m_nRssi_cn+10]=rssi2;
                m_nRssi_cn++;
                rs = 1;
                break;
            }

            memset(m_RX_Buf, 0, 256);
            m_RX_Len = 0;
        }

        if (i == (len - 1))
        {
            if (strncmp("SCAN:(origin_ed,ed)=(", m_RX_Buf, strlen("SCAN:(origin_ed,ed)=(")) == 0)
            {
                rs = 1;
            }
        }
    }

    return rs;
}

int Peak_Frequency_Power(double*f,double *p)
{
    int rs;
    rs = c_tester.Grab_CW_Frequency_and_power();
    if (rs == 0)
    {
        printf("Grab the frequency and power failed,please check\n");
        return 0;
    }
    else
    {
        *f=c_tester.m_freq_offset;
        //*f=c_tester.m_freq;
        *p=c_tester.m_dut_peek_power;
        printf("NIC_LOG:");
        printf("the grap frequency is %lf\n",-c_tester.m_freq);

        // printf("NIC_LOG:");
        // printf("the differ frequency is %lf\n", *f);

        printf("NIC_LOG:");
        printf("the power is %lf\n", *p);
        printf("grab the frequency and power from itest successfully\n");
        return 1;
    }
}



/**
 * @brief Grab the good packet
 * 
 * @param str 
 * @param len 
 */
int Grab_Good_Packet_harmwee(char *str, int len)
{
	int i,k,n,m;
	int sign=1;
	int rssi_value;
	int lqi=0;
	//int ngood,nCount,ntotal;
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
				// grab the good packet count and current receive total packet count
				// good/count=1/1, fcs err 0, len err 0, ber=0/1968
				if (strncmp("   good/count/total=", m_RX_Buf, strlen("   good/count/total=")) == 0)
				{

					k = strlen("   good/count/total=");
					n = 0;
                    m=0;
					while (k < m_RX_Len)
					{

						if ((m_RX_Buf[k] >= '0') && (m_RX_Buf[k] <= '9'))
						{
							temp[n++] = m_RX_Buf[k] - '0';
						}
						else if (m_RX_Buf[k] == '/')
						{
                            m++;
                            if(m==1)
                            {
                                if (n == 3)
                                    ngood = temp[0] * 100 + temp[1] * 10 + temp[2];
                                else if (n == 2)
                                    ngood = temp[0] * 10 + temp[1];
                                else if (n == 1)
                                    ngood = temp[0];
                                else
                                    ngood = 0;

                                //m_nGood[m_nGood_cn++] = ngood;
                                printf("the good value is %d\n", ngood);
                                n = 0;
                                
                            }

                            if(m==2)
                            {
                                if (n == 3)
                                    nCount = temp[0] * 100 + temp[1] * 10 + temp[2];
                                else if (n == 2)
                                    nCount = temp[0] * 10 + temp[1];
                                else if (n == 1)
                                    nCount = temp[0];
                                else
                                    nCount = 0;

                                //m_nGood[m_nGood_cn++] = ngood;
                                printf("the count value is %d\n", nCount);
                                n = 0;
                               

                            }
							
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

							//m_nTotal[m_nTotal_cn++] = ntotal;

							//break;
                            printf("the total value is %d\n", ntotal);
							k = m_RX_Len;
                            return 0x01;
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

    return 0x00;
}



