#include "stdafx.h"
#include "vsg_vsa.h"
#include "Vertex_CIU.h"
//#include "visa.h"
#include "serial\serial.h"

using namespace serial;
using namespace std;
extern char switch_com[10];//control the switch to tx and rx
extern char m_VSA_IpAddress[32]; // the ip address of vsa
extern Serial switchSerial;
/**
 * @brief //get the serial port of switch board
 * 
 * @return int 
 *         0: get the serial port of switch failure
 *         otherwise : the number of the serial port
 */
int Vsa_Get_Switch_Port(void)
{
    int rs = CIU_get_int_config(RF_SWITCH_CONFIG);
    if (rs >= 0)
        sprintf(switch_com, "COM%d", rs);
    return rs;
}
/**
 * @brief open the serial port of switch board
 * 
 * @return int 
 *         0: open the serial port failure
 *         1: open the serial port of switch successfully
 */
int Vsa_Open_Switch_Port(void)
{
	int rs;
	// get the serial port of switch
	rs=Vsa_Get_Switch_Port();
	if(rs==0)
    {
		printf("get the serial port of switch failure\n");
		return 0;
	}

	// open the serial port of switch
	printf(switch_com);
    printf("\n");
    switchSerial.setPort(switch_com);
	if(switchSerial.isOpen()==false)
    	switchSerial.open();
    switchSerial.setTimeout(serial::Timeout::max(), 100, 0, 100, 0); // set timeout is 100ms
    if (switchSerial.isOpen())
        return 1;
    else
        return 0;

}
/**
 * @brief switch to tx mode 
 * 
 */
void Vsa_Switch2Tx(void)
{
	string temp;
	temp = "nic_tx\r\n";
	switchSerial.flush();
	switchSerial.write(temp);
	Sleep(20);
	switchSerial.flush();
}
/**
 * @brief switch to rx mode
 * 
 */
void Vsa_Switch2Rx(void)
{
	string temp;
	temp="nic_rx\r\n";
	switchSerial.flush();
	switchSerial.write(temp);
	Sleep(20);
	switchSerial.flush();
}
/**
 * @brief reset the dut 
 * 
 */
void Vsa_Reset_dut(void)
{
	string temp;
	temp="nic_reset\r\n";
	switchSerial.flush();
	switchSerial.write(temp);
	Sleep(10);
	// switchSerial.flush();
}
// Retrieve the spectrum data and the peak frequency marker from an X-Series signal analyzer.
// 
// Instructions for Running:
// 1. Connect an antenna to the signal analyzer input.  The sweep will be performed over the FM band.
// 2. Enter your instrument address in the Connect step.
// 3. Run the sequence.
// 
// Requirements:
// This example was written for signal analyzers in the Agilent X-Series (PXA, MXA, EXA, CXA).  This example was tested with the following instrument(s): N9020A (ver. A.02.07).
// 
// Operation:
// 1. Reset and configure the signal analyzer.
// 2. Set the start and stop frequencies.
// 3. Initiate a sweep.
// 4. Retrieve the spectrum data.
// 5. Perform a peak search using marker 1.
// 6. Retrieve the X value of marker 1.
// 
// You have a royalty-free right to use, modify, reproduce and distribute this example program (and/or any modified version) in any way you find useful, provided you agree that Agilent Technologies has no warranty, obligations, or liability for any example programs.
void Vsa_Peak_Frequency(double*f,double *p)
{
	// TODO: Before you run this code do the following:
	// * Set the maximum sizes for any array variables.
	// * If your code queries binary block data, make sure it sets the byte order of the instrument to big endian.
	// char ip[100];
	// ViSession rm;
	// viOpenDefaultRM(&rm);
	// ViSession XSeries_SA;
	// ViUInt16 io_prot;
	// ViUInt16 intfType;
	// ViString intfName[512];
	// int sAnalyzer_size = 10; // Set this variable to the maximum size of the array.
	// double *sAnalyzer = new double[sAnalyzer_size];
	// double freq = 0.0;
	// double real = 0.0;
	
	// CIU_Get_VSA_IP_Address();
	// memset(ip,0,100);
	// strcat_s(ip,"TCPIP0::");
	// strcat_s(ip,m_VSA_IpAddress);
	// strcat_s(ip,"::5025::SOCKET");
	// printf("the vsa ip is %s\n",ip);
	// viOpen(rm, ip, VI_NULL, VI_NULL, &XSeries_SA);
	// viOpen(rm, "TCPIP0::192.168.1.224::5025::SOCKET", VI_NULL, VI_NULL, &XSeries_SA);
	// // viOpen(rm, "TCPIP0::192.168.10.100::5025::SOCKET", VI_NULL, VI_NULL, &XSeries_SA);
	// ViBoolean termDefaultXSeries_SA1 = VI_FALSE;
	// // Determine whether the termination character should be enabled by default.
	// if ((VI_SUCCESS == viGetAttribute(XSeries_SA, VI_ATTR_RSRC_CLASS, intfName)) && (0 == strcmp("SOCKET", (ViString)intfName)))
	// 	termDefaultXSeries_SA1 = VI_TRUE;
	// else if ((VI_SUCCESS == viGetAttribute(XSeries_SA, VI_ATTR_INTF_TYPE, &intfType)) && (intfType == VI_INTF_ASRL))
	// 	termDefaultXSeries_SA1 = VI_TRUE;
	// viSetAttribute(XSeries_SA, VI_ATTR_TERMCHAR_EN, termDefaultXSeries_SA1);
	// viPrintf(XSeries_SA, "*RST\n");
	// viSetAttribute(XSeries_SA, VI_ATTR_TMO_VALUE, 2000);
	// viPrintf(XSeries_SA, ":FORM:TRAC:DATA %s,%@1d\n", "REAL", 64);
	// viPrintf(XSeries_SA, ":FORM:BORD %s\n", "SWAP");
	// viPrintf(XSeries_SA, ":INST:SEL \"%s\"\n", "SA");
	// viPrintf(XSeries_SA, ":CONF:SAN\n");
	// viPrintf(XSeries_SA, ":DISP:WIND:TRAC:Y:SCAL:RLEV %@3lf\n", 30.0);
	// viPrintf(XSeries_SA, ":SENS:AVER:COUN %@1d\n", 25);
	// viPrintf(XSeries_SA, ":DISP:WIND:TRAC:Y:SCAL:PDIV %@3lf\n", 10.0);
	// viPrintf(XSeries_SA, ":INIT:CONT %@1d\n", 0);
	// viPrintf(XSeries_SA, ":TRAC1:TYPE %s\n", "WRIT");
	// viPrintf(XSeries_SA, ":SENS:DET:TRAC1 %s\n", "NORM");
	// //(XSeries_SA, ":SENS:FREQ:STAR %@3lf MHZ\n", 433.835);
	// //viPrintf(XSeries_SA, ":SENS:FREQ:STOP %@3lf MHZ\n", 434.035);
	// viPrintf(XSeries_SA, ":SENS:FREQ:STAR %@3lf MHZ\n", 914.900);
	// viPrintf(XSeries_SA, ":SENS:FREQ:STOP %@3lf MHZ\n", 915.100);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
	// viPrintf(XSeries_SA, ":SENS:SWE:POIN %@1d\n", 1001);
	// viPrintf(XSeries_SA, ":INIT:IMM\n");
	// viPrintf(XSeries_SA, "*WAI\n");
	// ViBoolean termDefaultXSeries_SA = VI_FALSE;
	// viGetAttribute(XSeries_SA, VI_ATTR_TERMCHAR_EN, &termDefaultXSeries_SA);
	// viSetAttribute(XSeries_SA, VI_ATTR_TERMCHAR_EN, VI_FALSE);
	// viQueryf(XSeries_SA, ":FETC:SAN1?\n", "%#Zb", &sAnalyzer_size, sAnalyzer);
	// viSetAttribute(XSeries_SA, VI_ATTR_TERMCHAR_EN, termDefaultXSeries_SA);
	// if (termDefaultXSeries_SA) viScanf(XSeries_SA, "%*c"); // Clear the termination character from the buffer.
	// viPrintf(XSeries_SA, ":CALC:MARK1:MODE %s\n", "POS");
	// viPrintf(XSeries_SA, ":CALC:MARK1:MAX:PEAK\n");
	// viQueryf(XSeries_SA, ":CALC:MARK1:X?\n", "%lf", &freq);
	// viQueryf(XSeries_SA, ":CALC:MARK1:Y?\n", "%lf", &real);
	
	// real+=16.12;

	// printf("the frequency is %lf\n", freq);
	// printf("the power is %lf\n", real);
	// // if(real>0.0)
	// // {
		
	// // }
	// *f=freq;
	// *p=real;

	// viClose(XSeries_SA);
	// viClose(rm);
	// delete[] sAnalyzer;
	
}