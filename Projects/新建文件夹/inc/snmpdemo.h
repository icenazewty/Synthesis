#ifndef _SNMPDEMO_H_
#define _SNMPDEMO_H_

extern dataEntryType snmpData[];
extern const int maxData;

void initTable(void);
void currentUptime(void *ptr, unsigned char *len);
void getWIZnetLed(void *ptr,  unsigned char *len);
void getInputStatus(void *ptr,  unsigned char *len);
void getOutputStatus(void *ptr,  unsigned char *len);
void getDipStatus(void *ptr,  unsigned char *len);
void getAd(void *ptr,  unsigned char *len);
void getBat(void *ptr,  unsigned char *len);
void get12V(void *ptr,  unsigned char *len);
void getRTS(void *ptr,  unsigned char *len);
void getDeviceTempe(void *ptr,  unsigned char *len);
void getAndroid_Ctrl(void *ptr,  unsigned char *len);

void getModbus_ID(void *ptr,  unsigned char *len);
void getRj45(void *ptr,  unsigned char *len);
void getComInfo(void *ptr,  unsigned char *len);



void setAndroid_Ctrl(int val);
void setWIZnetLed(int val);
void setOutput_On(int val);
void setOutput_Off(int val);
void setRj45_MAC(void *ptr,  unsigned char *len);



void UserSnmpDemo(void);

#endif



