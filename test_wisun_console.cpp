// test_wisun_console.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "test_wisun_console.h"
#include "serial\serial.h"
#include "wisun_test_flow.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include   <iostream>  
#include   <io.h>  
#include   <direct.h>  
#include   <string>  
#include   <vector>  
#include   <iomanip>  
#include   <ctime>  

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;

using namespace std;
using namespace serial;

// Serial dutSerial("COM4", 115200, Timeout::simpleTimeout(1000)); // Create serial port and open if you specified the com name
// Serial goldenSerial("COM43", 115200, Timeout::simpleTimeout(1000)); // create serial port and open if you specified the com name
Serial dutSerial("", 115200, Timeout::simpleTimeout(1000)); // just create the serial port than do not open the port because do not specified the com name
Serial goldenSerial("", 115200, Timeout::simpleTimeout(1000)); // just create the serial port than do not open the port because do not specified the com name

// 唯一的应用程序对象

CWinApp theApp;
char dut_com[10];
char golden_com[10];
char m_Debug='0';
string m_Log;

void enumerate_ports();
void getFiles( string, vector<string>& );
void getFileContent(string file);
void getCentNV(string file);
int  isCharOrDigital(char c);
int main1()
{
    int nRetCode = 0;
    int rs;
    CStdioFile newfile;
	BOOL rs1;
    char yy[100];
    char yy1[100];
    vector<string> files;

    HMODULE hModule = ::GetModuleHandle(nullptr);
    // the available com port list
    // enumerate_ports();
    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            
            // analyse the pass log file
            getFiles(".\\pass", files);
            // getFiles(".\\log", files);
            // rs1 = newfile.Open(".\\Serial_number_20200610.txt", CStdioFile::modeCreate | CStdioFile::modeWrite, NULL);
            rs1 = newfile.Open(".\\centnv_20200610.txt", CStdioFile::modeCreate | CStdioFile::modeWrite, NULL);
            // print the files get
            for (unsigned int j = 0; j < files.size(); ++j)
            {
                cout << files[j] << endl;

                // get series number from the filename
                // strncpy_s(yy1, files[j].c_str(), 30);
                // if (strncmp("_TestLog_", &yy1[16], strlen("_TestLog_")) == 0)
                // {
                //     cout << files[j] << endl;
                //     memset(yy, 0, 100);
                //     strncpy_s(yy, &yy1[7], 9);

                //     //  strncat_s(yy, ",", 1);
                //     strncat_s(yy, "\n", 1);
                //     //  printf(yy);

                //     // getFileContent(files[j]);
                //     //printf_s(m_Log.c_str());
                //     // newfile.Write(yy,10);

                //     //newfile.WriteString(m_Log.c_str());
                //     // break;

                //     newfile.WriteString(yy);
                // }




                // get key value from the pass log
                strncpy_s(yy1, files[j].c_str(), 30);
                if (strncmp("_TestLog_", &yy1[16], strlen("_TestLog_")) == 0)
                {
                    memset(yy, 0, 100);
                    strncpy_s(yy, &yy1[7], 9);

                    strncat_s(yy, ",", 1);
                    //  printf(yy);
                    m_Log="";
                    getFileContent(files[j]);
                    //printf_s(m_Log.c_str());
                    newfile.Write(yy, 10);
                    if(m_Log=="")
                        m_Log="\n";
                    newfile.WriteString(m_Log.c_str());
                    // break;
                }
                



                // get cent nv information
                // getCentNV(files[j]);
                // printf_s(m_Log.c_str());
                // newfile.WriteString(m_Log.c_str());
                // break;
            }
            newfile.Close();
            return 0;

            rs1 = newfile.Open(".\\finaltest.txt", CStdioFile::modeRead , NULL);
            if(rs1)
            {
               
                while (newfile.ReadString(yy,100))
                {
                    // get the  serial number of DUT
                    if(strncmp("DUT=", yy, strlen("DUT=")) == 0)
                    {
                        memset(dut_com,0,10);
                        if(yy[6]>='0' && yy[6]<='9')
                        {
                            sprintf_s(dut_com,"COM%c%c%c",yy[4],yy[5],yy[6]);
                        }
                        else if(yy[5]>='0' && yy[5]<='9')
                        {
                            sprintf_s(dut_com,"COM%c%c",yy[4],yy[5]);
                        } 
                        else
                        {
                            sprintf_s(dut_com,"COM%c",yy[4]);
                        }
                        
                        // printf_s(dut_com);
                    }

                    //get the serial number of Golden
                    if (strncmp("GOLDEN=", yy, strlen("GOLDEN=")) == 0)
                    {
                        memset(golden_com, 0, 10);
                        if (yy[9] >= '0' && yy[9] <= '9')
                        {
                            sprintf_s(golden_com, "COM%c%c%c", yy[7], yy[8], yy[9]);
                        }
                        else if (yy[8] >= '0' && yy[8] <= '9')
                        {
                            sprintf_s(golden_com, "COM%c%c", yy[7], yy[8]);
                        }
                        else
                        {
                            sprintf_s(golden_com, "COM%c", yy[7]);
                        }

                        // printf_s(golden_com);
                    }

                    // get the debug flag
                    if (strncmp("DEBUG=", yy, strlen("DEBUG=")) == 0)
                    {
                        if (yy[6] == '0')
                        {
                            m_Debug = '0';
                        }
                        else
                        {
                            m_Debug = '1';
                        }
                    }

                }
                
            }
            newfile.Close();
            
            // open Serial port of DUT
            // dutSerial.setPort("COM4");
            dutSerial.setPort(dut_com);
            dutSerial.open();
            dutSerial.setTimeout(serial::Timeout::max(), 100, 0, 100, 0); // set timeout is 100ms
            if (!dutSerial.isOpen())
            {
                if (m_Debug == '1')
                {
                    printf("open serial port of DUT failed\n");
                }

                return 0;
            }
            // open serial port successfully
            if (m_Debug == '1')
            {
                printf("open serial port of DUT successfully\n");
            }

            // open serial port of golden node
            // goldenSerial.setPort("COM48");
            goldenSerial.setPort(golden_com);
            goldenSerial.open();
            goldenSerial.setTimeout(serial::Timeout::max(), 100, 0, 100, 0);// set timeout is 100ms
            if (!goldenSerial.isOpen())
            {
                if (m_Debug == '1')
                {
                    printf("open serial port of Golden failed\n");
                }

                return 0;
            }
            // open serial port successfully
            if (m_Debug == '1')
            {
                printf("open serial port of golden successfully\n");
            }

            // step1: verify the golden node 
            rs=Verify_Device_Connect(GOLDEN_DEVICE);
            if(rs==0)
            {
                if(m_Debug=='1')
                    printf("verify golden node failed,please check\n");
                goldenSerial.close();
                dutSerial.close();
                return 0;
            }
            else
            {
                // printf("\n\n");
                // printf("-----------------------------------------------------\n");
                // printf("wisun final test start....\n");
                if(m_Debug=='1')
                    printf("verify golden node successfully\n\n\n");
            }

            //step2: verify the dut node and force to test mode
            rs = Verify_Device_Connect(DUT_DEVICE);
            if (rs == 0)
            {
                if(m_Debug=='1')
                    printf("verify dut node failure,please check\n");
                goldenSerial.close();
                dutSerial.close();
                return 0;
            }
            else
            {
                if(m_Debug=='1')
                    printf("verify dut node successfully\n\n\n");
            }

            // get the serial number
            rs=Get_Serial_Number();
            if (rs == 0)
            {
                if (m_Debug == '1')
                    printf("get serial number error\n");
                goldenSerial.close();
                dutSerial.close();
                return 0;
            }
            else
            {
                if (m_Debug == '1')
                    printf("get serial number successfully\n\n\n");
            }
            
            //  finally verify
            // printf("finally verify\n");
            rs=Final_Verify();
            if(rs==0)
            {
                printf("FAIL\n");
                goldenSerial.close();
                dutSerial.close();
                return 0;
            }
            else
            {
                printf("PASS\n");
            }

            // close the serial port of dut and golden
            goldenSerial.close();
            dutSerial.close();

            // step3: calibration RSSI
            // printf("3.Calibration  the rssi\n");
            // Golden_Tx_Valid_Packet();
            // rs=Calibration_Rssi();
            // if(rs==0)
            // {
            //     printf("calibration the rssi failure,please check\n");
            //     return 0; 
            // }
            // else
            // {
            //     printf("Calibration the rssi successfully\n\n\n");
            // }

            // step 4:power gain calibration
            // printf("4.Calibration  the power gain\n");
            // rs=Dut_Power_Gain_Calibration();
            // if(rs==0)
            // {
            //     printf("calibration the power gain failure,please check\n");
            //     return 0; 
            // }
            // else
            // {
            //     printf("Calibration the power gain successfully\n\n\n");
            // }

            // step 5:carrier frequency calibration
            // printf("5.Calibration  the frequency deviation\n");
            // rs=Xtal_Calibration();
            // if(rs==0)
            // {
            //     printf("calibration the frequency deviation failure,please check\n");
            //     return 0; 
            // }
            // else
            // {
            //     printf("Calibration the frequency deviation successfully\n\n\n");
            // }

            // step 6: verify the tx performance based on 902M
            // printf("6: verify the tx performance based on 902M\n");
            // rs=Verify_Tx(1);
            // if(rs==0)
            // {
            //     printf("verify the tx performance based on 902M failure,please check\n");
            //     return 0; 
            // }
            // else
            // {
            //     printf("verify the tx performance based on 902M successfully\n\n\n");
            // }

            // step 7: verify the tx performance based on 915M
            // printf("7: verify the tx performance based on 915M\n");
            // rs=Verify_Tx(2);
            // if(rs==0)
            // {
            //     printf("verify the tx performance based on 915M failure,please check\n");
            //     return 0; 
            // }
            // else
            // {
            //     printf("verify the tx performance based on 915M successfully\n\n\n");
            // }

            // step 8: verify the tx performance based on 927M
            // printf("8: verify the tx performance based on 927M\n");
            // rs=Verify_Tx(3);
            // if(rs==0)
            // {
            //     printf("verify the tx performance based on 927M failure,please check\n");
            //     return 0; 
            // }
            // else
            // {
            //     printf("verify the tx performance based on 927M successfully\n\n\n");
            // }

            // step 9: verify the rx performance based on 902M
            // printf("9: verify the rx performance based on 902M\n");
            // rs=Verify_Rx(1);
            // if(rs==0)
            // {
            //     printf("verify the rx performance based on 902M failure,please check\n");
            //     return 0; 
            // }
            // else
            // {
            //     printf("verify the rx performance based on 902M successfully\n\n\n");
            // }

            // step 10: verify the tx performance based on 915M
            // printf("10: verify the rx performance based on 915M\n");
            // rs=Verify_Rx(2);
            // if(rs==0)
            // {
            //     printf("verify the rx performance based on 915M failure,please check\n");
            //     return 0; 
            // }
            // else
            // {
            //     printf("verify the rx performance based on 915M successfully\n\n\n");
            // }

            // step 11: verify the rx performance based on 927M
            // printf("11: verify the rx performance based on 927M\n");
            // rs=Verify_Rx(3);
            // if(rs==0)
            // {
            //     printf("verify the rx performance based on 927M failure,please check\n");
            //     return 0; 
            // }
            // else
            // {
            //     printf("verify the rx performance based on 927M successfully\n\n\n");
            // }


            dutSerial.close();
            goldenSerial.close();


        }

    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}

void enumerate_ports()
{
	vector<serial::PortInfo> devices_found = serial::list_ports();

	vector<serial::PortInfo>::iterator iter = devices_found.begin();

	while( iter != devices_found.end() )
	{
		serial::PortInfo device = *iter++;

		printf( "(%s, %s, %s)\n", device.port.c_str(), device.description.c_str(),
     device.hardware_id.c_str() );
	}
}




void getFiles( string path, vector<string>& files ) {
    //文件句柄  
    long   hFile   =   0;  
    //文件信息  
    struct _finddata_t fileinfo;  

    string p;

    if   ((hFile   =   _findfirst(p.assign(path).append("/*").c_str(),&fileinfo))   !=   -1)  {  

        do  {  
            //如果是目录,迭代之
            //如果不是,加入列表
            if   ((fileinfo.attrib   &   _A_SUBDIR)) {  
                if   (strcmp(fileinfo.name,".")   !=   0   &&   strcmp(fileinfo.name,"..")   !=   0)  
                    getFiles( p.assign(path).append("\\").append(fileinfo.name), files   );  
            }  else  {  
                files.push_back(   p.assign(path).append("\\").append(fileinfo.name)  ); 
            }  
        }   while   (_findnext(   hFile,   &fileinfo   )   ==   0);  

        _findclose(hFile);  
    }
}

void getFileContent(string file)
{
    CStdioFile newfile;
    BOOL rs1;
    CString xx;
    int i,k;
    char yy[20];
    char yy1[20];
    int len;
    int start=0;
    CString a1,a2;
    rs1 = newfile.Open(file.c_str(), CStdioFile::modeRead, NULL);
    if (rs1)
    {
        while (newfile.ReadString(xx))
        {
            len = strlen("   Elna_Rssi_Offset:");
            if (strncmp("   Elna_Rssi_Offset:", xx.GetString(), len) == 0)
            {
                //printf(xx);
                // memset(yy, 0, 20);
                // strncpy_s(yy, xx.Mid(len, 10), 10);

                // printf(yy);
                start=1;
            }

            len = strlen("cent nv rssi_offset 23");
            if (strncmp("cent nv rssi_offset 23", xx.GetString(), len) == 0)
            {
                //printf(xx);
                // memset(yy, 0, 20);
                // strncpy_s(yy, xx.Mid(len, 10), 10);

                // printf(yy);
                start=1;
            }

            if (start)
            {
                // elan_rssi_offset
                len=strlen("cent nv elna_rssi_offset");
                if( strncmp("cent nv elna_rssi_offset",xx.GetString(),len)==0 )
                {
                    memset(yy, 0, 20);
                    memset(yy1, 0, 20);
                    strncpy_s(yy, xx.Mid(len, 10), 10);
                    k=0;
                    for(i=1; i<10;i++ )
                    {
                        if(yy[i]!=' ')
                            yy1[k++]=yy[i];
                        else
                            break;
                        
                    }

                    // printf(yy1);
                    // printf(",");
                    // m_Log=yy1;
                    // m_Log+=",";
                    a1=yy1;
                    a1+=",";

                }
                
                // rssi_offset
                len=strlen("cent nv rssi_offset");
                if( strncmp("cent nv rssi_offset",xx.GetString(),len)==0 )
                {
                    memset(yy, 0, 20);
                    memset(yy1, 0, 20);
                    strncpy_s(yy, xx.Mid(len, 10), 10);
                    k=0;
                    for(i=1; i<10;i++ )
                    {
                        if(yy[i]!=' ')
                            yy1[k++]=yy[i];
                        else
                            break;
                        
                    }

                    // printf(yy1);
                    // printf(",");
                    // m_Log+=yy1;
                    // m_Log+=",";
                    a2=yy1;
                    a2+=",";

                }
                
                // tx power which is measured
                len=strlen("   TX_[0]  Pow:");
                if( strncmp("   TX_[0]  Pow:",xx.GetString(),len)==0 )
                {
                    memset(yy, 0, 20);
                    memset(yy1, 0, 20);
                    strncpy_s(yy, xx.Mid(len, 10), 10);
                    k=0;
                    for(i=1; i<10;i++ )
                    {
                        if(i==6)
                            break;

                        if(yy[i]!=' ')
                            yy1[k++]=yy[i];
                        else
                            break;
                        
                    }

                    // printf(yy1);
                    // printf(",");
                    m_Log=a1;
                    m_Log+=a2;
                    m_Log+=yy1;
                    m_Log+=",";
                }

                // tx power offset 
                len=strlen("cent nv tx_power_cal_offset");
                if( strncmp("cent nv tx_power_cal_offset",xx.GetString(),len)==0 )
                {
                    memset(yy, 0, 20);
                    memset(yy1, 0, 20);
                    strncpy_s(yy, xx.Mid(len, 10), 10);
                    k=0;
                    for(i=1; i<10;i++ )
                    {
                        if(yy[i]!=' ')
                            yy1[k++]=yy[i];
                        else
                            break;
                        
                    }

                    // printf(yy1);
                    // printf(",");
                    m_Log+=yy1;
                    m_Log+=",";

                }

                // xtal offset 
                len=strlen("cent nv xtal_offset");
                if( strncmp("cent nv xtal_offset",xx.GetString(),len)==0 )
                {
                    memset(yy, 0, 20);
                    memset(yy1, 0, 20);
                    strncpy_s(yy, xx.Mid(len, 10), 10);
                    k=0;
                    for(i=1; i<10;i++ )
                    {
                        if(yy[i]!=' ')
                            yy1[k++]=yy[i];
                        else
                            break;
                        
                    }

                    // printf(yy1);
                    // printf(",");
                    m_Log+=yy1;
                    m_Log+=",";

                }

                 // verify tx power
                len=strlen("   TX_[0]     0M  Power[ 0]    :");
                if( strncmp("   TX_[0]     0M  Power[ 0]    :",xx.GetString(),len)==0 )
                {
                    memset(yy, 0, 20);
                    memset(yy1, 0, 20);
                    strncpy_s(yy, xx.Mid(len, 10), 10);
                    k=0;
                    for(i=1; i<10;i++ )
                    {

                        if(yy[i]!=' ')
                            yy1[k++]=yy[i];
                        else
                            break;
                        
                    }

                    // printf(yy1);
                    // printf(",");
                    m_Log+=yy1;
                    m_Log+=",";

                }

                // verify sensitivity
                len=strlen("	Dut Rssi:");
                if( strncmp("	Dut Rssi:",xx.GetString(),len)==0 )
                {
                    memset(yy, 0, 20);
                    memset(yy1, 0, 20);
                    strncpy_s(yy, xx.Mid(len, 10), 10);
                    // sprintf_s(yy1,"the is%d",yy[7]);
                    // printf_s(yy1);
                    k=0;
                    for(i=0; i<10;i++ )
                    {
                        if(yy[i]!=9)
                            yy1[k++]=yy[i];
                        else
                            break;
                        
                    }

                    // printf(yy1);
                    // printf(",");
                    m_Log+=yy1;
                    m_Log+=",";

                }

                // good packet
                len=strlen("          Pass:");
                if( strncmp("          Pass:",xx.GetString(),len)==0 )
                {
                    memset(yy, 0, 20);
                    memset(yy1, 0, 20);
                    strncpy_s(yy, xx.Mid(len, 10), 10);
                    k=0;
                    for(i=0; i<10;i++ )
                    {
                        if(yy[i]!='.')
                            yy1[k++]=yy[i];
                        else
                            break;
                        
                    }

                    // printf(yy1);
                    // printf("\n");
                    m_Log+=yy1;
                    m_Log+="\n";

                }




            }
            
           


        //    printf("\n");
        }
        
    }
}

/**
 * @brief Get the Cent NV object
 * 
 * @param file 
 */
void getCentNV(string file)
{
    CStdioFile newfile;
    BOOL rs1;
    CString xx;
    int i, k;
    char yy[20];
    char yy1[20];
    int len;
    int start = 0;
    CString a1,a2,a3,a4;
    rs1 = newfile.Open(file.c_str(), CStdioFile::modeRead, NULL);
    if (rs1)
    {
        while (newfile.ReadString(xx))
        {

            // series number
            len = strlen("series_number");
            if (strncmp("series_number", xx.GetString(), len) == 0)
            {
                memset(yy, 0, 20);
                memset(yy1, 0, 20);
                strncpy_s(yy, xx.Mid(len, 10), 10);
                k = 0;
                for (i = 1; i < 10; i++)
                {
                    if(isCharOrDigital(yy[i]))
                    {
                        yy1[k++]=yy[i];
                    }
                }

                // printf(yy1);
                // printf(",");
                m_Log = yy1;
                m_Log += ",";
            }

            // elna_rssi_offset
            len = strlen("elna_rssi_offset");
            if (strncmp("elna_rssi_offset", xx.GetString(), len) == 0)
            {
                memset(yy, 0, 20);
                memset(yy1, 0, 20);
                strncpy_s(yy, xx.Mid(len, 10), 10);
                k = 0;
                for (i = 1; i < 10; i++)
                {
                    if (isCharOrDigital(yy[i]))
                    {
                        yy1[k++] = yy[i];
                    }
                }

                // printf(yy1);
                // printf(",");
                // m_Log += yy1;
                // m_Log += ",";
                // m_Log += "\n";
                a1=yy1;
                a1+=",";
            }

            // rssi_offset
            len = strlen("rssi_offset");
            if (strncmp("rssi_offset", xx.GetString(), len) == 0)
            {
                memset(yy, 0, 20);
                memset(yy1, 0, 20);
                strncpy_s(yy, xx.Mid(len, 10), 10);
                k = 0;
                for (i = 1; i < 10; i++)
                {
                    if (isCharOrDigital(yy[i]))
                    {
                        yy1[k++] = yy[i];
                    }
                }

                // printf(yy1);
                // printf(",");
                // m_Log += yy1;
                // m_Log += ",";
                a2=yy1;
                a2+=",";
            }

            // tx_power_cal_offset
            len = strlen("tx_power_cal_offset");
            if (strncmp("tx_power_cal_offset", xx.GetString(), len) == 0)
            {
                memset(yy, 0, 20);
                memset(yy1, 0, 20);
                strncpy_s(yy, xx.Mid(len, 10), 10);
                k = 0;
                for (i = 1; i < 10; i++)
                {
                    if (isCharOrDigital(yy[i]))
                    {
                        yy1[k++] = yy[i];
                    }
                }

                // printf(yy1);
                // printf(",");
                a3 = "18.1,";
                a3+= yy1;
                a3+= ",";
            }

            // xtal_offset
            len = strlen("xtal_offset");
            if (strncmp("xtal_offset", xx.GetString(), len) == 0)
            {
                memset(yy, 0, 20);
                memset(yy1, 0, 20);
                strncpy_s(yy, xx.Mid(len, 10), 10);
                k = 0;
                for (i = 1; i < 10; i++)
                {
                    if (isCharOrDigital(yy[i]))
                    {
                        yy1[k++] = yy[i];
                    }
                }

                // printf(yy1);
                // printf(",");
                a4 = yy1;
                a4 += ",";
                a4 += "18.01,-106.4,96";
                a4+="\n";
            }

                // printf("\n");
        }
        m_Log+=a1;
        m_Log+=a2;
        m_Log+=a3;
        m_Log+=a4;
    }
}

/**
 * @brief the char is Character or Digital
 * 
 * @param c 
 * @return int 
 */
int  isCharOrDigital(char c)
{
    if(c>='a' && c<='z')
    {
        return 1;
    }
    else if(c>='A' && c<='Z')
    {
        return 1;
    }
    else if(c>='0' && c<='9')
    {
        return 1;
    }
    else
    {
        return 0;
    }
    
}
