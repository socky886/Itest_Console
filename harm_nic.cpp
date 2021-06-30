#include "harm_nic.h"
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
#include "Vertex_CIU.h"
#include "harmwee.h"
using namespace serial;
using namespace std;

#pragma comment(lib,"serial.lib")


//Serial dutSerial("", 115200, Timeout::simpleTimeout(1000)); // just create the serial port than do not open the port because do not specified the com name
extern Serial dutSerial;
extern char m_RX_Buf[256];// Serial Port Receive buffer
extern WT208_Tester c_tester;
extern int m_RX_Len;// Serial Port Receive character length
extern int m_nRssi[50];
extern int m_nRssi_cn ;
extern char dut_com[10];

extern unsigned char m_Serial_Open;

extern int ngood,nCount,ntotal;

extern void Init_Itest_library(void);
extern void Realse_Itest_library(void);

/**
 * @brief Unit test
 * 
 */
int nic_unit_test(void)
{
    int ret = 0;
    char buf[128] = {0};
    int rs=0;
    float vsg_level=1.0;
    int i=0;
    string res;
    std::string dut;
    char xx[100];
    
    printf("this is dut test application\n");
    
   
    // step 1:open the Serial Port
    if (m_Serial_Open == 0)
    {
        Nic_Open_Serial();
    }
    
    if (m_Serial_Open == 0)//open serial port failed and return
    {
        printf("open serial port failed\r\n");
        return 0;
    }
    printf("open serial port of dut successfully\r\n");


   
    //step 2: force to enter factory mode
    Nic_Enter_Factory_Mode();
    

    //step 3: verify the DUT
    ret=Nic_Verify_DUT();
    if(ret==0)
    {
        printf("verify dut failed\n");
        // printf("failed\n");
        return 0;
    }
    printf("verify dut successfully\n");

    
    // step 4: init tester(WT208C)
    Init_Itest_library();
    ret=c_tester.init();
    if(ret==false)
    {
        printf("connect tester failed\n");
        printf("failed\n");
        return 0;
    }
    printf("connect wt208c successfully\n");
    
    //  step 5: calibration the rssi when the signal of vsg is -50dbm
    // c_tester.asyn_vsg(-50.0);// start vsg
    Nic_Calibration_50dbm_Rssi();
    // c_tester.stop_vsg();// stop VSG transmit
    
    
    // step 6:calibration the rssi when the signal of vsg is -100dbm
    // c_tester.asyn_vsg(-100.0);// start vsg
    Nic_Calibration_100dbm_Rssi();
    // c_tester.stop_vsg();// stop VSG transmit
    

    

   
   
    
    // step 7: single wave test frequency and tx power
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
        return 0;
    }
   
    printf("Verify tx power of 915m successfully\n");
    
    

    
    // step 8: verify sensitivity
    // c_tester.asyn_vsg(-106.0);// start vsg with -106dBm 
    vsg_level=CIU_Get_VSG_Level();
    for ( i = 0; i < 3; i++)
    {
        vsg_level+=i;
        c_tester.asyn_vsg(vsg_level); // the vsg level can config
        rs = Verify_Sensitivity();
        c_tester.stop_vsg(); // stop VSG transmit

        if(rs)
        {
            printf("NIC_LOG:");
            printf("the sensitivity is %lf dbm\n",vsg_level);
            break;
        }
        else
        {
            Sleep(200);
        }
    }
    
    // vsg_level=CIU_Get_VSG_Level();
    // c_tester.asyn_vsg(vsg_level);// the vsg level can config
    // rs=Verify_Sensitivity();
    // c_tester.stop_vsg();// stop VSG transmit
    if(rs)
    {
        printf("verify sensitivity successfully\n");
    }
    else
    {
        printf("verify sensitivity failed\n");
        return 0;
    }

    // close serial port of dut
    if(dutSerial.isOpen())
    {
        dutSerial.close();
    }
    // releaes itest library
    Realse_Itest_library();
    
    printf("all items test passed.\n");
    return 1;
    
}


/**
 * @brief Verify Serial Port
 * 
 */
int Nic_Open_Serial(void)
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
int Nic_Verify_DUT(void)
{
    string temp;
    int len;
    char str[256];
    int rs;

    // clear the rx buffer of the serial port
    dutSerial.flush();

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
int Nic_Enter_Factory_Mode(void)
{
    // enter factory mode
    char str[256];
    int len;
    int i;
    string temp;
    // enter factory mode
    temp="rf_factory_mode_set  1\r";
    dutSerial.flush();
    dutSerial.write(temp);
    Sleep(10);

    //reboot
    temp = "reboot\r";
    dutSerial.write(temp);
    Sleep(3500); // two seconds 
    dutSerial.flush();
    
    // return value
    return 0x01;
}
/**
 * @brief Calibration the rssi offset of chip
 * 
 */
int Nic_Calibration_50dbm_Rssi(void)
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
    printf("NIC_LOG:");
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
int Nic_Calibration_100dbm_Rssi(void)
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
    printf("NIC_LOG:");
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
 * @brief exit factory mode
 * 
 */
int Nic_Exit_Factory_Mode(void)
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



