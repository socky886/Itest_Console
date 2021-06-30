#pragma once


#define DUT_CONFIG  "DUT"
#define VSG_PORT_CONFIG "vsg_port"
#define VSA_PORT_CONFIG "vsa_port"
#define WT_FIXED_ATTEN_SUB_1G "WT_FIXED_ATTEN_SUB_1G"
#define WT_WISUN_FREQ_XTALOFFSET_UP "WT_WISUN_FREQ_XTALOFFSET_UP"
#define WT_WISUN_FREQ_XTALOFFSET_LOW "WT_WISUN_FREQ_XTALOFFSET_LOW"
#define WT_TX_VERIFY_COUNT   "WT_TX_VERIFY_COUNT"
#define RF_SWITCH_CONFIG "SWITCH_TRX"


#define GOLDEN_DEVICE 	1
#define DUT_DEVICE 		2
#define VSA_DEVICE 		3

#define FRQ_902M 	    1
#define FRQ_915M 		2
#define FRQ_927M 		3


int nic_unit_test(void);

int Nic_Open_Serial(void);
int Nic_Enter_Factory_Mode(void);
int Nic_Verify_DUT(void);
int Nic_Calibration_50dbm_Rssi();
int Nic_Calibration_100dbm_Rssi();
int Nic_Exit_Factory_Mode(void);
static int Search_Specified_Field(const char *p, char *str, int len);





// int ciu_unit_test(void);
// int CIU_get_int_config(const char *p);

// int CIU_Get_DUT_Port(void);
// int CIU_Get_VSG_Port(void);
// int CIU_Get_VSA_Port(void);
// int CIU_Get_TX_Verify_Count(void);
// float CIU_Get_Cable_Loss(void);
// int CIU_Get_Frequency_Range(void);
// int CIU_Open_DUT(void);
// int CIU_Verify_DUT(void);
// int CIU_Get_DUT_SerialNumber(void);
// void CIU_Reset(void);
// int Verify_Device_Connect(unsigned char deviceno);
// int Search_Specified_Field(const char *p, char *str,int len);// search specified field
// int Get_Serial_Number(void);
// int Grab_Serial_Number(char *str,int len);

// int CIU_Get_IP_Address(void);
// int CIU_Get_VSA_IP_Address(void);
// int CIU_Force_To_Test(void);
// int CIU_Tx_CW(void);
// int CIU_Stop_CW(void);
// int CIU_Frequency_Calibration(void);
// int CIU_Manu_Calibration_Xtal(void);
// int CIU_Verify_TX(void);
// int CIU_Verify_RX(void);
// void CIU_Exit_RFTest(void);
// void Grab_Good_Packet(char *str, int len);
// int Get_Real_Rssi_Value(int * p, int len);
