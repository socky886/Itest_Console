#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <Windows.h>

#define GOLDEN_DEVICE 	1
#define DUT_DEVICE 		2
#define VSA_DEVICE 		3

#define FRQ_902M 	    1
#define FRQ_915M 		2
#define FRQ_927M 		3

int Verify_Device_Connect(unsigned  char deviceno);// verify the connection of dut and golden node
int Search_Specified_Field(const char *p, char *str,int len);// search specified field
void Golden_Tx_Valid_Packet(void);// golden transmit valid packet
int Calibration_Rssi(void);
int Calibration_Rssi_new(void);
int Get_Real_Rssi_Value(int * p, int len);
void Save_Raw_Rssi_To_file(const char* filename);
void Grab_Cal_Rssi_Value(char *str,int len);
int Dut_Power_Gain_Calibration(void);
int Xtal_Calibration(void);
void Grab_Good_Packet(char *str, int len);
int Verify_Tx(int frequency);
int Verify_Rx(int frequency); 
int Get_Serial_Number(void);
int Grab_Serial_Number(char *str,int len);
int Final_Verify(void);

