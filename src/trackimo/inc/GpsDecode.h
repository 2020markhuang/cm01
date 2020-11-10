#ifndef __GPS_DECODE_H__
#define __GPS_DECODE_H__
char NMEA_Comma_Pos(char *buf,char cx);
int NMEA_Pow(char m,char n);
int NMEA_Str2num(char *buf,char*dx);
void NMEA_GPRMC_Analysis( char *pBuf, int data_len);
void NMEA_GPVTG_Analysis(char *pBuf,int data_len);
void NMEA_GPGGA_Analysis(char *pBuf,int data_len);
void NMEA_GPGSA_Analysis(char *pBuf,int data_len);
#endif

