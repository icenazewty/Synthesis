#ifndef _SNMPLIB_H_
#define _SNMPLIB_H_

//#define SUCCESS						0
#define OID_NOT_FOUND				-1
#define TABLE_FULL					-2
#define ILLEGAL_LENGTH				-3
#define INVALID_ENTRY_ID			-4
#define INVALID_DATA_TYPE			-5

#define NO_SUCH_NAME				2
#define BAD_VALUE						3

#define MAX_OID							12
#define MAX_STRING					100

//#define SNMP_V1						0
#define SNMP_V2C						1

#define COMMUNITY					"public\0"			//密码
#define COMMUNITY_SIZE				(strlen(COMMUNITY))

#define GET_REQUEST					0xa0
#define GET_NEXT_REQUEST		0xa1
#define GET_RESPONSE				0xa2
#define SET_REQUEST					0xa3

#define VALID_REQUEST(x)			((x == GET_REQUEST) || (x == GET_NEXT_REQUEST) || (x == SET_REQUEST))

#define SNMPDTYPE_INTEGER				0x02
#define SNMPDTYPE_OCTET_STRING	0x04
#define SNMPDTYPE_NULL_ITEM			0x05
#define SNMPDTYPE_OBJ_ID				0x06
#define SNMPDTYPE_SEQUENCE			0x30
#define SNMPDTYPE_SEQUENCE_OF		SNMPDTYPE_SEQUENCE

#define SNMPDTYPE_COUNTER				0x41
#define SNMPDTYPE_GAUGE  				0x42
#define SNMPDTYPE_TIME_TICKS		0x43
#define SNMPDTYPE_OPAQUE				0x44


#define UNUSED(x)					(void)x; // for IAR warning (declared but never referenced)
#define HTONL(x)					((((x)>>24) & 0xffL) | (((x)>>8) & 0xff00L) | (((x)<<8) & 0xff0000L) | (((x)<<24) & 0xff000000L))



#ifdef WIN32
typedef char char;
typedef unsigned char unsigned char;
typedef short int16;
typedef unsigned short uint16;
typedef int int;
typedef unsigned int uint;
#else
//#include "Types.h"
#endif

//mib定义
typedef struct 
{
	unsigned char oidlen;
	unsigned char oid[MAX_OID];
	unsigned char dataType;
	unsigned char dataLen;
	union 
	{
		unsigned char octetstring[MAX_STRING];		//48
		unsigned int  intval;
	} u;
	void (*getfunction)(void *, unsigned char *);
	void (*setfunction)(int);
} dataEntryType;		

struct messageStruct {
	unsigned char buffer[1025];
	int len;
	int index;
};

typedef struct {
	int start;	/* Absolute Index of the TLV */
	int len;		/* The L value of the TLV */
	int vstart;   /* Absolute Index of this TLV's Value */
	int nstart;   /* Absolute Index of the next TLV */
} tlvStructType;

void WDEBUG(char *fmt, ...);
int findEntry(unsigned char *oid, int len);
int getOID(int id, unsigned char *oid, unsigned char *len);
int getValue( unsigned char *vptr, int vlen);
int getEntry(int id, unsigned char *dataType, void *ptr, int *len);
int setEntry(int id, void *val, int vlen, unsigned char dataType, int index);

int parseLength(const unsigned char *msg, int *len);
int parseTLV(const unsigned char *msg, int index, tlvStructType *tlv);
void insertRespLen(int reqStart, int respStart, int size);
int parseVarBind(int reqType, int index);
int parseSequence(int reqType, int index);
int parseSequenceOf(int reqType);
int parseRequest(void);
int parseCommunity(void);
int parseVersion(void);
int parseSNMPMessage(void);

void ipToByteArray(char *ip, unsigned char *pDes);
int makeTrapVariableBindings(dataEntryType *oid_data, void *ptr, unsigned int *len);
int SnmpXInit(void);
int SnmpXTrapSend(char* managerIP, char* agentIP, char* community, dataEntryType enterprise_oid, unsigned int genericTrap, unsigned int specificTrap, unsigned int va_count, ...);
int SnmpXDaemon(void);

#endif
