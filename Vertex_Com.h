#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <Windows.h>

using namespace std;

// metal function
void enumerate_ports();
void getFiles( string, vector<string>& );
void getFileContent(string file);
void getCentNV(string file);
int  isCharOrDigital(char c);

// full function
void Grab_Calibration_Verify_Data(void);
int Final_Verify_Flow(void);
int Total_Test_Flow(void);