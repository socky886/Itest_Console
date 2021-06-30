// #include "pch.h"
#include "stdafx.h"
#include "wisun_test_flow.h"
#include "serial\serial.h"

using namespace serial;
using namespace std;
extern Serial dutSerial;
extern Serial goldenSerial;


char m_RX_Buf[256];// Serial Port Receive buffer
int m_RX_Len;// Serial Port Receive character length
int m_nGolden_Reboot = 0; // Reboot the golden node

int m_nCalRSSi[50];
int m_nRssiCount = 0;

int m_nRssi[50];
int m_nRssi_cn = 0;

int m_nLQI[50];
int m_nLQI_cn = 0;

int m_nGood[50];
int m_nGood_cn = 0;

int m_nTotal[50];
int m_nTotal_cn = 0;

char m_Serial_Number[20];
int m_Serial_len;

extern char m_Debug;
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
        // force the device into rftest mode
        temp="cent rftest 1\r";
        goldenSerial.write(temp);
        Sleep(6);

        // enable data whiten
        // temp="cent  dwen 1\r";
        // dutSerial.write(temp);
        // Sleep(10);

        m_RX_Len = 0;
        memset(m_RX_Buf, 0, 256);
        // reboot
        // temp="reboot\r";
        // goldenSerial.write(temp);
        // Sleep(3000);
        // sent test command
        temp = "\r";
        goldenSerial.write(temp);
        while (1)
        {
            memset(str,0,256);
            len = goldenSerial.read((uint8_t *)str, 256);
            if (len)
            {
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
 * @brief Golden node tx valid packet
 * 
 */
void Golden_Tx_Valid_Packet(void)
{
    string temp;

    // reboot the Goden node
    if (m_nGolden_Reboot == 0)
    {
        // temp="reboot\r";
        // goldenSerial.write(temp);
        m_nGolden_Reboot = 1;
        // delay 2 seconds
        // Sleep(2000);

        // set tx power of ic to -10.0 dbm
        // the real tx power of link is -37dbm
        // temp="cent pwr -10.0\r";
        // goldenSerial.writ(temp);

        // sent band to 915000000
        temp="cent band 915000000\r";
        goldenSerial.write(temp);
        Sleep(10);

        // set symbol rate to 50k
        temp="cent rate 50000\r";
        goldenSerial.write(temp);
        Sleep(10);

        // set modulation index to 1.0
        temp="cent mod 1000\r";
        goldenSerial.write(temp);
        Sleep(10);
    }

    // send package,
    // the packet lenght is 246
    // the packet number is 256
    // the interval time is 20000us(every 20ms send one packet)
    temp="cent txc f6 50 20000\r";
    goldenSerial.write(temp);
}

/**
 * @brief Calibration the rssi_offset and elan_rssi_offset
 * 
 * @return int 1: Calibration rssi successfully
 *             0: calibration rssi faiture
 */
int Calibration_Rssi(void)
{
	string temp;
	int i;
	int rssi;
    char str[256];
    int len;
	// the VSG Serial Port is opened successfully
	if (dutSerial.isOpen())
	{
		// RSSI Cal power = -37.00
		// cent nv rssi_offset 0
		// cent nv elna_rssi_offset 0
		// cent band 915000000
		// cent elna 1
		// cent rssi 50

		//    Elna_Rssi_Offset:  -37.00			   (-39.00 ......O........... -34.00)
		// cent nv elna_rssi_offset 23
		// cent nv rssi_offset 6
		// set nv rssi_offset to 0
		// temp="cent nv rssi_offset 0\r";
		temp="cent  rssi_offset 0\r";
		dutSerial.write(temp);
		Sleep(10);

		// cent nv elna_rssi_offset 0
		// temp="cent nv elna_rssi_offset 0\r";
		temp="cent  elna_rssi_offset 0\r";
		dutSerial.write(temp);
		Sleep(10);

		// sent band to 915000000
		temp="cent band 915000000\r";
		dutSerial.write(temp);
		Sleep(10);

		// sent elan to bypass mode
		temp="cent elna 1\r";
		dutSerial.write(temp);
		Sleep(10);


        // m_bGrapCalRssi=true;
		m_RX_Len=0;
		m_nRssiCount=0;
		memset(m_RX_Buf,0,256);
		// cent rssi 50
		temp="cent rssi 50\r";
		dutSerial.write(temp);
        while(1)
        {
            memset(str,0,256);
            len = dutSerial.read((uint8_t *)str, 256);
            // printf(str);
            if (len)
                Grab_Cal_Rssi_Value(str,len);
            else
                break;
        }
		// Sleep(350);
        
        // send only one packet to force stop the transmit
        temp="cent txc f6 1 0\r";
        dutSerial.write(temp);
        Sleep(10);

        if(m_nRssiCount==50)
		{
			// m_bGrapCalRssi=false;
            printf("grab the rssi value successfully\n");
            // printf("the count of rssi is %d\n",m_nRssiCount);
            for(i=0;i<50;i++)
            {
                printf("%d,",m_nCalRSSi[i]);
                if((i+1)%10 ==0)
                    printf("\n");
            }
            rssi = Get_Real_Rssi_Value(m_nCalRSSi, 50);
            printf("the real rssi is %d\n",rssi);
            // elan_rssi_offset=?
            // rssi_offset=?
            return 1;
		}
        else
        {
            return 0;
        }		
		
	}
    return 0;
	
}

/**
 * @brief Calibration the rssi_offset and elan_rssi_offset
 * 
 * @return int 1: Calibration rssi successfully
 *             0: calibration rssi faiture
 */
int Calibration_Rssi_new(void)
{
	string temp;
	int i;
	int rssi;
    char str[256];
    int len;
	// the VSG Serial Port is opened successfully
	if (dutSerial.isOpen())
	{
		// RSSI Cal power = -37.00
		// cent nv rssi_offset 0
		// cent nv elna_rssi_offset 0
		// cent band 915000000
		// cent elna 1
		// cent rssi 50

		//    Elna_Rssi_Offset:  -37.00			   (-39.00 ......O........... -34.00)
		// cent nv elna_rssi_offset 23
		// cent nv rssi_offset 6
		// set nv rssi_offset to 0
		// temp="cent nv rssi_offset 0\r";
		// temp="cent  rssi_offset 0\r";
		// dutSerial.write(temp);
		// Sleep(10);

		// cent nv elna_rssi_offset 0
		// temp="cent nv elna_rssi_offset 0\r";
		// temp="cent  elna_rssi_offset 0\r";
		// dutSerial.write(temp);
		// Sleep(10);

		// sent band to 915000000
		temp="cent band 915000000\r";
		dutSerial.write(temp);
		Sleep(10);

		// sent elan to bypass mode
		// temp="cent elna 1\r";
		// dutSerial.write(temp);
		// Sleep(10);


        // m_bGrapCalRssi=true;
		m_RX_Len=0;
		m_nRssiCount=0;
		memset(m_RX_Buf,0,256);
		// cent rssi 50
		temp="cent rssi 50\r";
		dutSerial.write(temp);
        while(1)
        {
            memset(str,0,256);
            len = dutSerial.read((uint8_t *)str, 256);
            // printf(str);
            if (len)
                Grab_Cal_Rssi_Value(str,len);
            else
                break;
        }
		// Sleep(350);
        
        // send only one packet to force stop the transmit
        // temp="cent txc f6 1 0\r";
        // dutSerial.write(temp);
        // Sleep(10);

        if(m_nRssiCount==50)
		{
			// m_bGrapCalRssi=false;
            printf("grab the rssi value successfully\n");
            // printf("the count of rssi is %d\n",m_nRssiCount);
            for(i=0;i<50;i++)
            {
                printf("%d,",m_nCalRSSi[i]);
                if((i+1)%10 ==0)
                    printf("\n");
            }
            rssi = Get_Real_Rssi_Value(m_nCalRSSi, 50);
            printf("the real rssi is %d\n",rssi);
            // elan_rssi_offset=?
            // rssi_offset=?
            return 1;
		}
        else
        {
            return 0;
        }		
		
	}
    return 0;
	
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

/**
 * @brief save raw calibration rssi value to file
 * 
 * @param filename 
 */
void Save_Raw_Rssi_To_file(const char* filename)
{
    CStdioFile newfile;
    BOOL rs;
    int i;
    CString temp;
    int rssi;
    // create new file
    rs = newfile.Open("D:\\wisun_rssi_value.txt", CStdioFile::modeCreate | CStdioFile::modeWrite, NULL);
    if (rs)
    {

        for (i = 0; i < m_nRssiCount; i++)
        {
            temp.Format(_T("%d\n"), m_nCalRSSi[i]);
            newfile.WriteString(temp);
        }

        rssi=Get_Real_Rssi_Value(m_nCalRSSi,m_nRssiCount);
        temp.Format(_T("the rssi_value is %d\n"), rssi);
        newfile.WriteString(temp);

        newfile.Close();

        printf("write rssi calibration value to file successfully\n");
    }
    else
    {
        printf("create new file failed\n");
    }
}


/**
 * @brief Grab calibration rssi value from the stream of serial port
 * 
 */
void Grab_Cal_Rssi_Value(char *str,int len)
{
    int i,k;
	int sign=1;
	int rssi_value;
    //if (m_bGrapCalRssi)
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
                if (strncmp("rssi = ", m_RX_Buf, strlen("rssi = ")) == 0 || strncmp("vc# rssi = ", m_RX_Buf, strlen("vc# rssi = ")) == 0)
                {
                    k = m_RX_Len;
                    sign = 1;
                    rssi_value = 0;
                    while (--k > 0)
                    {
                        if (m_RX_Buf[k] == ' ')
                            break;

                        if (m_RX_Buf[k] == '-')
                            sign = -1;

                        else if ((m_RX_Buf[k] >= '0') && (m_RX_Buf[k] <= '9'))
                        {
                            if ((k + 3) == m_RX_Len)
                                rssi_value += (m_RX_Buf[k] - '0') * 100;
                            else if ((k + 2) == m_RX_Len)
                                rssi_value += (m_RX_Buf[k] - '0') * 10;
                            else if ((k + 1) == m_RX_Len)
                                rssi_value += m_RX_Buf[k] - '0';
                            else
                                ;
                        }
                    }

                    // get the real rssi value
                    rssi_value *= sign;
					if(m_nRssiCount<50)
                    	m_nCalRSSi[m_nRssiCount++] = rssi_value;
                }
                // else
                // {
                // 	str1="none rssi string found\n"));
                // }

                memset(m_RX_Buf, 0, 256);
                m_RX_Len = 0;

                while (++i < len)
                {
                    if ((str[i] != 13) && (str[i] != 10))
                    {
                        i--;
                        break;
                    }
                }
            }
        }
    }

}

/**
 * @brief Calibration the power gain of the DUT
 * 
 * @return int 
 */
int Dut_Power_Gain_Calibration(void)
{
    // Initiate tx power gain array
    unsigned char tx_gain_array[] = {10, 8, 7, 6, 4, 3, 2, 1};
    std::string golden;
    std::string dut;
    // int res;
    int i = 0;
    int len = sizeof(tx_gain_array) / sizeof(unsigned char);
    int rssi;
    char str[256];
    int strlen;
    char xx[100];
    // set the basic operation of the DUT
    if (dutSerial.isOpen())
    {
        // sent band to 915000000
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
    }

    // set the basic operation of the golden node
    if (goldenSerial.isOpen())
    {
        // sent band to 915000000
        golden = "cent band 915000000\r";
        goldenSerial.write(golden);
        Sleep(5);

        // set modulation index to 1.0
        golden = "cent mod 1000\r";
        goldenSerial.write(golden);
        Sleep(5);

        // set symbol rate to 50k
        golden = "cent rate 50000\r";
        goldenSerial.write(golden);
        Sleep(5);
    }

    for (i = 0; i < len; i++)
    // for (i = 0; i < 1; i++)
    {
        // set the tx power
        sprintf_s(xx, "cent pwr -%d.0\r", tx_gain_array[i]);
        dut = xx;
        // dut="cent pwr -10.0\r";
        dutSerial.write(dut);
        Sleep(5);

        // transmit pn9 packet
        // dut="cent txc -p 1FF 100\r";
        // transmit modulator packet
        dut = "cent txc f6 30 20000\r";
        // transmit single tone
        // dut="cent txcw 1\r";
        dutSerial.write(dut);
        //Sleep(10);

        // get the rssi value of the golden
        m_RX_Len = 0;
        m_nRssiCount = 0;
        memset(m_RX_Buf, 0, 256);
        golden = "cent rssi 10\r";
        goldenSerial.write(golden);
        while (1)
        {
            memset(str, 0, 256);
            strlen = goldenSerial.read((uint8_t *)str, 256);
            printf(str);
            if (strlen)
                Grab_Cal_Rssi_Value(str, strlen);
            else
                break;
        }

        // dut = "cent txc f6 1 10000\r";
        dut = "cent txc 10 1 10000\r";
        dutSerial.write(dut);
        Sleep(10);

        // dut="cent txcw 0\r";
        // dutSerial.write(dut);
        // Sleep(10);
        if (m_nRssiCount == 10)
        {
            // printf("the count of rssi is %d\n",m_nRssiCount);
            // printf("grab the rssi value successfully\n");
            rssi = Get_Real_Rssi_Value(m_nCalRSSi, 10);
            printf("       %d db tx power, the rssi_value is %d\n\n\n", tx_gain_array[i], rssi);
        }
        else
        {

            printf("receive packet is %d,  ", m_nRssiCount);
            printf("golden node do not get the rssi value of dut node\n");

            return 0;
        }
    }

    return 1;
}

/**
 * @brief Calibration the central frequency depanding on the xtal offset
 * 
 * @return int 
 */
int Xtal_Calibration(void)
{
    std::string dut;
    std::string golden;
    int i = 0;
    int k = 0;
    int rssi;
    char xx[30];
    char str[256];
    int len;
    //	set the parameter of the DUT of TX
    if (dutSerial.isOpen())
    {
        // sent band to 915000000
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

        // set pwr to -7dbm
        dut = "cent pwr -7.0\r";
        dutSerial.write(dut);
        Sleep(5);
    }
    // 	set the parameter of the golden of RX
    if (goldenSerial.isOpen())
    {
        // sent band to 915000000
        golden = "cent band 915000000\r";
        goldenSerial.write(golden);
        Sleep(5);

        // set modulation index to 1.0
        golden = "cent mod 1000\r";
        goldenSerial.write(golden);
        Sleep(5);

        // set symbol rate to 50k
        golden = "cent rate 50000\r";
        goldenSerial.write(golden);
        Sleep(5);
    }

    //  Calibration the link in the loop
    // for (i = 0; i <= 20; i++)
    for (i = 0; i < 5; i++)
    {
        //  set the golden node to receive 50 packets
        // set the receive packet length is 246bytes
        golden = "cent rxc f6\r";
        goldenSerial.write(golden);
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
        golden = "cent ber 10\r";
        goldenSerial.write(golden);
        Sleep(5);

        // set the parameter of the dctune
        sprintf_s(xx, "cent dctune %d\r", i);
        dut = xx;
        // dut="cent dctune ";
        // dut.assign(1,i);
        // dut.assign("\r");
        dutSerial.write(dut);
        Sleep(5);

        //  set the DUT to transmit packet
        // transmit pn9 packet
        // dut="cent txc -p 1FF 100\r";
        // transmit modulator packet
        dut = "cent txc f6 40 10000\r";
        // transmit single tone
        // dut="cent txcw 1\r";
        dutSerial.write(dut);
        while (1)
        {
            memset(str, 0, 256);
            len = goldenSerial.read((uint8_t *)str, 256);
            printf(str);
            if (len)
                Grab_Good_Packet(str,len);
            else
                break;
        }
        // Sleep(3000);//receive 50 packet
        // Sleep(600); //receive 10 packet

        dut = "cent txc f6 1 10000\r";
        // transmit single tone
        // dut="cent txcw 1\r";
        dutSerial.write(dut);
        Sleep(5);

        //  Grab the rssi value and the good packet
        // clear the ber count
        golden = "cent ber -c\r";
        goldenSerial.write(golden);
        Sleep(5);

        // save grab value
        if (m_nTotal_cn == 10)
        {
            // printf("   the count of good is %d, total is %d, the count of rssi is %d, the count of lqi is %d,", m_nGood_cn, m_nTotal_cn, m_nRssi_cn, m_nLQI_cn);
            rssi = Get_Real_Rssi_Value(m_nRssi, 10);
            printf("the real rssi is %d,", rssi);
            printf(" the good rate is%d\n", m_nGood_cn * 100 / m_nTotal_cn);
        }
        else
        {
            return 0;
        }
    }

    return 1;
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
 * @brief Verify the Tx performance
 * 
 * @param frequency 
 * @return int 
 */
int Verify_Tx(int frequency)
{
    std::string dut;
    std::string golden;
    int i = 0;
    int k = 0;
    int rssi;
    char xx[30];
    char str[256];
    int len;
    //	set the parameter of the DUT of TX
    if (dutSerial.isOpen())
    {
        // sent band frequency
        if(frequency==1)
            dut = "cent band 902000000\r";
        else if(frequency==2)
            dut = "cent band 915000000\r";
        else if(frequency==3)
            dut = "cent band 927000000\r";
        else
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

        // set pwr to -7dbm
        dut = "cent pwr -7.0\r";
        dutSerial.write(dut);
        Sleep(5);
    }
    // 	set the parameter of the golden of RX
    if (goldenSerial.isOpen())
    {
        
        // sent band frequency
        if(frequency==1)
            golden = "cent band 902000000\r";
        else if(frequency==2)
            golden = "cent band 915000000\r";
        else if(frequency==3)
            golden = "cent band 927000000\r";
        else
            golden = "cent band 915000000\r";
        
        goldenSerial.write(golden);
        Sleep(5);

        // set modulation index to 1.0
        golden = "cent mod 1000\r";
        goldenSerial.write(golden);
        Sleep(5);

        // set symbol rate to 50k
        golden = "cent rate 50000\r";
        goldenSerial.write(golden);
        Sleep(5);
    }

    //  Calibration the link in the loop
    // for (i = 0; i <= 20; i++)
    // for (i = 0; i < 5; i++)
    {
        //  set the golden node to receive 50 packets
        // set the receive packet length is 246bytes
        golden = "cent rxc f6\r";
        goldenSerial.write(golden);
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
        golden = "cent ber 10\r";
        goldenSerial.write(golden);
        Sleep(5);

        // set the parameter of the dctune
        sprintf_s(xx, "cent dctune %d\r", 7);
        dut = xx;
        dutSerial.write(dut);
        Sleep(5);

        //  set the DUT to transmit packet
        // transmit pn9 packet
        // dut="cent txc -p 1FF 100\r";
        // transmit modulator packet
        dut = "cent txc f6 40 10000\r";
        // transmit single tone
        // dut="cent txcw 1\r";
        dutSerial.write(dut);
        while (1)
        {
            memset(str, 0, 256);
            len = goldenSerial.read((uint8_t *)str, 256);
            printf(str);
            if (len)
                Grab_Good_Packet(str, len);
            else
                break;
        }
        // Sleep(3000);//receive 50 packet
        // Sleep(600); //receive 10 packet

        dut = "cent txc f6 1 10000\r";
        // transmit single tone
        // dut="cent txcw 1\r";
        dutSerial.write(dut);
        Sleep(5);

        //  Grab the rssi value and the good packet
        // clear the ber count
        golden = "cent ber -c\r";
        goldenSerial.write(golden);
        Sleep(5);

        // save grab value
        if (m_nTotal_cn == 10)
        {
            // printf("   the count of good is %d, total is %d, the count of rssi is %d, the count of lqi is %d,", m_nGood_cn, m_nTotal_cn, m_nRssi_cn, m_nLQI_cn);
            rssi = Get_Real_Rssi_Value(m_nRssi, 10);
            printf("the real rssi is %d,", rssi);
            printf(" the good rate is%d\n", m_nGood_cn * 100 / m_nTotal_cn);
        }
        else
        {
            return 0;
        }
    }

    return 1;
}
/**
 * @brief Verify the Rx performance
 * 
 * @param frequency 
 * @return int 
 */
int Verify_Rx(int frequency)
{
    std::string dut;
    std::string golden;
    int i = 0;
    int k = 0;
    int rssi;
    char xx[30];
    char str[256];
    int len;
    //	set the parameter of the DUT of TX
    if (goldenSerial.isOpen())
    {
        // sent band frequency
        if(frequency==1)
            dut = "cent band 902000000\r";
        else if(frequency==2)
            dut = "cent band 915000000\r";
        else if(frequency==3)
            dut = "cent band 927000000\r";
        else
            dut = "cent band 915000000\r";

        goldenSerial.write(dut);
        Sleep(5);

        // set modulation index to 1.0
        dut = "cent mod 1000\r";
        goldenSerial.write(dut);
        Sleep(5);

        // set symbol rate to 50k
        dut = "cent rate 50000\r";
        goldenSerial.write(dut);
        Sleep(5);

        // set pwr to -7dbm
        dut = "cent pwr -7.0\r";
        goldenSerial.write(dut);
        Sleep(5);
    }
    // 	set the parameter of the golden of RX
    if (dutSerial.isOpen()) //dutSerial
    {
        
        // sent band frequency
        if(frequency==1)
            golden = "cent band 902000000\r";
        else if(frequency==2)
            golden = "cent band 915000000\r";
        else if(frequency==3)
            golden = "cent band 927000000\r";
        else
            golden = "cent band 915000000\r";
        
        dutSerial.write(golden);
        Sleep(5);

        // set modulation index to 1.0
        golden = "cent mod 1000\r";
        dutSerial.write(golden);
        Sleep(5);

        // set symbol rate to 50k
        golden = "cent rate 50000\r";
        dutSerial.write(golden);
        Sleep(5);
    }

    //  Calibration the link in the loop
    // for (i = 0; i <= 20; i++)
    // for (i = 0; i < 5; i++)
    {
        //  set the golden node to receive 50 packets
        // set the receive packet length is 246bytes
        golden = "cent rxc f6\r";
        dutSerial.write(golden);
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
        golden = "cent ber 10\r";
        dutSerial.write(golden);
        Sleep(5);

        // set the parameter of the dctune
        sprintf_s(xx, "cent dctune %d\r", 7);
        dut = xx;
        goldenSerial.write(dut);
        Sleep(5);

        //  set the DUT to transmit packet
        // transmit pn9 packet
        // dut="cent txc -p 1FF 100\r";
        // transmit modulator packet
        dut = "cent txc f6 40 10000\r";
        // transmit single tone
        // dut="cent txcw 1\r";
        goldenSerial.write(dut);
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
        // Sleep(3000);//receive 50 packet
        // Sleep(600); //receive 10 packet

        dut = "cent txc f6 1 10000\r";
        // transmit single tone
        // dut="cent txcw 1\r";
        goldenSerial.write(dut);
        Sleep(5);

        //  Grab the rssi value and the good packet
        // clear the ber count
        golden = "cent ber -c\r";
        dutSerial.write(golden);
        Sleep(5);

        // save grab value
        if (m_nTotal_cn == 10)
        {
            // printf("   the count of good is %d, total is %d, the count of rssi is %d, the count of lqi is %d,", m_nGood_cn, m_nTotal_cn, m_nRssi_cn, m_nLQI_cn);
            rssi = Get_Real_Rssi_Value(m_nRssi, 10);
            printf("the real rssi is %d,", rssi);
            printf(" the good rate is%d\n", m_nGood_cn * 100 / m_nTotal_cn);
        }
        else
        {
            return 0;
        }
    }

    return 1;
}
/**
 * @brief Finally Verify
 * 
 * @return int 
 */
int Final_Verify(void)
{
    std::string dut;
    std::string golden;
    int i = 0;
    int k = 0;
    int rssi;
//    char xx[30];
    char str[256];
    int len;
    //	set the parameter of the golden of TX
    if (goldenSerial.isOpen())
    {
        dut = "cent dwen 1\r";
        dutSerial.write(golden);
        Sleep(5);
        // sent band frequency
        golden = "cent band 915000000\r";
        goldenSerial.write(golden);
        Sleep(10);

        // set modulation index to 1.0
        golden = "cent mod 1000\r";
        goldenSerial.write(golden);
        Sleep(10);

        // set symbol rate to 50k
        golden = "cent rate 50000\r";
        goldenSerial.write(golden);
        Sleep(10);

        // set pwr to -7dbm
        // golden = "cent pwr -0.0\r";
        // goldenSerial.write(golden);
        // Sleep(5);
    }
    // 	set the parameter of the dut of RX
    if (dutSerial.isOpen()) //dutSerial
    {
         // disable the data white
        dut = "cent dwen 1\r";
        dutSerial.write(golden);
        Sleep(5);


       
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
    }

    //  Calibration the link in the loop
    {
        // set the dut node to receive 51 packets
        // set the receive packet length is 246bytes
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
        golden = "cent ber 10\r";
        dutSerial.write(golden);
        // Sleep(5);

        
        //  set the DUT to transmit packet
        // transmit pn9 packet
        // dut="cent txc -p 1FF 100\r";
        // transmit modulator packet
        dut = "cent txc f6 40 10000\r";
        // transmit single tone
        // dut="cent txcw 1\r";
        goldenSerial.write(dut);
        while (1)
        {
            memset(str, 0, 256);
            len = dutSerial.read((uint8_t *)str, 256);
            if (m_Debug == '1')
                printf(str);

            if (len)
                Grab_Good_Packet(str, len);
            else
                break;
        }
        // Sleep(3000);//receive 50 packet
        // Sleep(600); //receive 10 packet

         dut = "cent txc f6 1 10000\r";
        // // transmit single tone
        // // dut="cent txcw 1\r";
         goldenSerial.write(dut);
         Sleep(5);

        //  Grab the rssi value and the good packet
        // clear the ber count
        golden = "cent ber -c\r";
        dutSerial.write(golden);
        Sleep(5);

        // save grab value
        if (m_nTotal_cn == 10)
        {
            // printf("   the count of good is %d, total is %d, the count of rssi is %d, the count of lqi is %d,", m_nGood_cn, m_nTotal_cn, m_nRssi_cn, m_nLQI_cn);
            rssi = Get_Real_Rssi_Value(m_nRssi, 10);
            // printf("the real rssi is %d,", rssi);
            // printf(" the good rate is%d\n", m_nGood_cn * 100 / m_nTotal_cn);
            m_nGood_cn=m_nGood_cn * 100 / m_nTotal_cn;

            if(m_nGood_cn<=50)
                return 0;
        }
        else
        {
            return 0;
        }
    }

    return 1;
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

        // get the serial number
        dut = "cent nv series_number\r";
        dutSerial.write(dut);
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