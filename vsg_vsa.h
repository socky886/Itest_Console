#ifndef __VSG_VSA_H_
#define __VSG_VSA_H_


int Vsa_Get_Switch_Port(void);//get the serial port of switch board
int Vsa_Open_Switch_Port(void);//open the serial port of switch board
void Vsa_Switch2Tx(void); //switch to tx mode 
void Vsa_Switch2Rx(void); // switch to rx mode
void Vsa_Peak_Frequency(double*f,double *p);// get the peak frequency and power of the single wave 
void Vsa_Reset_dut(void);// reset the dut 
#endif