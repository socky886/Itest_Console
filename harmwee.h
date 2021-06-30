#pragma once
#include <iostream>
#include <string.h>
//#include "visa.h"
// #include <windows.h>
// #include <stdafx.h>
void Nxp_Download_Firmware(void);
// test flow
void harmwee_test(char *p);
void harwwee_test_by_VSGVSA(void);
// test action
int Nxp_Open_Serial(void);
int Nxp_Verify_DUT(void);
int Nxp_Enter_Factory_Mode(void);
int Nxp_Exit_Factory_Mode(void);
int  Nxp_Write_Config_info(char *p);
int Verify_Frequency_And_TxPower(void);
int Calibration_50dbm_Rssi(void);
int Calibration_100dbm_Rssi(void);
int Verify_Sensitivity(void);
int Peak_Frequency_Power(double*f,double *p);

void dut_Start_SingleWave(void);
void dut_Stop_SingleWave(void);
static int Search_Specified_Field(const char *p, char *str, int len);
int Scan_Rssi_Value(const char *p, char *str, int len);
int Grab_Good_Packet_harmwee(char *str, int len);