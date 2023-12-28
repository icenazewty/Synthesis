#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "ctype.h"
#include "w5500.h"

#include "snmplib.h"
#include "snmpDemo.h"
#define  SOCK_SNMP	      3
struct messageStruct request_msg;
struct messageStruct response_msg;

unsigned char errorStatus, errorIndex;

#define COPY_SEGMENT_TEMP20110929(x) \
{ \
	request_msg.index += seglen; \
	memcpy(&response_msg.buffer[response_msg.index], &request_msg.buffer[x.start], seglen ); \
	response_msg.index += seglen; \
}

void WDEBUG(char *fmt, ...)
{
	char zlog_string[100];
	va_list ap;	

	va_start(ap, fmt);
	vsprintf(zlog_string, fmt, ap);
	strcat(zlog_string, "\r\n");
	printf("%s",zlog_string);	
	va_end(ap);	
}

int findEntry(unsigned char *oid, int len)
{
	int i;

	for (i = 0 ; i < maxData ; i++)
	{
		if (len == snmpData[i].oidlen)
		{
			if (!memcmp(snmpData[i].oid, oid, len)) return(i);
		}
	}

	return OID_NOT_FOUND;
}


int getOID(int id, unsigned char *oid, unsigned char *len)
{
	int j;

	if (!((id >= 0) && (id < maxData))) return INVALID_ENTRY_ID;

	*len = snmpData[id].oidlen;

	for (j = 0 ; j < *len ; j++)
	{
		oid[j] = snmpData[id].oid[j];
	}

	return 1;
}


int getValue( unsigned char *vptr, int vlen)
{
	int index = 0;
	int value = 0;

	while (index < vlen)
	{
		if (index != 0) value <<= 8;
		value |= vptr[index++];
	}

	return value;
}


int getEntry(int id, unsigned char *dataType, void *ptr, int *len)
{
	if (!((id >= 0) && (id < maxData))) return INVALID_ENTRY_ID;

	*dataType = snmpData[id].dataType;

	switch(*dataType)
	{
	case SNMPDTYPE_OCTET_STRING :
	case SNMPDTYPE_OBJ_ID :
		{
			unsigned char *string = ptr;
			int j;

			if (snmpData[id].getfunction != NULL)
			{
				snmpData[id].getfunction( (void *)&snmpData[id].u.octetstring, &snmpData[id].dataLen );
			}

			if ( (*dataType)==SNMPDTYPE_OCTET_STRING )
			{
				snmpData[id].dataLen = (unsigned char)strlen((char*)&snmpData[id].u.octetstring);
			}


			*len = snmpData[id].dataLen;
			for (j = 0 ; j < *len ; j++)
			{
				string[j] = snmpData[id].u.octetstring[j];
			}
		}
		break;

	case SNMPDTYPE_INTEGER :
	case SNMPDTYPE_TIME_TICKS :
	case SNMPDTYPE_COUNTER :
	case SNMPDTYPE_GAUGE :
		{
			int *value = ( int * )ptr;

			if (snmpData[id].getfunction != NULL)
			{
				snmpData[id].getfunction( (void *)&snmpData[id].u.intval, &snmpData[id].dataLen );
			}

			*len = sizeof(unsigned int);
			*value = HTONL(snmpData[id].u.intval);
		}
		break;

	default : 
		return INVALID_DATA_TYPE;
	}

	return 1;
}


int setEntry(int id, void *val, int vlen, unsigned char dataType, int index)
{

	int retStatus=OID_NOT_FOUND;
	int j;

	if (snmpData[id].dataType != dataType)
	{
		errorStatus = BAD_VALUE; 
		errorIndex = index;
		return INVALID_DATA_TYPE;
	}

	switch(snmpData[id].dataType)
	{
	case SNMPDTYPE_OCTET_STRING :
	case SNMPDTYPE_OBJ_ID :
	{
			unsigned char *string = val;
			for (j = 0 ; j < vlen ; j++)
			{
				snmpData[id].u.octetstring[j] = string[j];
			}
			snmpData[id].dataLen = vlen;
			if (snmpData[id].getfunction != NULL)					//增加部分
			{
				snmpData[id].getfunction( (void *)&snmpData[id].u.octetstring, &snmpData[id].dataLen );
			}
		}
		retStatus = 1;
		break;

	case SNMPDTYPE_INTEGER :
	case SNMPDTYPE_TIME_TICKS :
	case SNMPDTYPE_COUNTER :
	case SNMPDTYPE_GAUGE :
	{
			snmpData[id].u.intval = getValue( (unsigned char *)val, vlen);
			snmpData[id].dataLen = vlen;

			if (snmpData[id].setfunction != NULL)
			{
				snmpData[id].setfunction(snmpData[id].u.intval);
			}

		}
		retStatus = 1;
		break;

	default : 
		retStatus = INVALID_DATA_TYPE;
		break;

	}

	return retStatus;
}


int parseLength(const unsigned char *msg, int *len)
{
	int i=1;

	if (msg[0] & 0x80)
	{
		int tlen = (msg[0] & 0x7f) - 1;
		*len = msg[i++];

		while (tlen--)
		{
			*len <<= 8;
			*len |= msg[i++];
		}
	}
	else
	{
		*len = msg[0];
	}

	return i;
}

//第2字节为后面数据的长度42-2
int parseTLV(const unsigned char *msg, int index, tlvStructType *tlv)
{
	int Llen = 0;

	tlv->start = index;

	Llen = parseLength((const unsigned char *)&msg[index+1], &tlv->len );

	tlv->vstart = index + Llen + 1;		//有效数据开始位置 2

	switch (msg[index])
	{
	case SNMPDTYPE_SEQUENCE:
	case GET_REQUEST:
	case GET_NEXT_REQUEST:
	case SET_REQUEST:							//set命令 第1字节  0x30
		tlv->nstart = tlv->vstart;
		break;
	default:
		tlv->nstart = tlv->vstart + tlv->len;
		break;
	}

	return 0;
}


void insertRespLen(int reqStart, int respStart, int size)
{
	int indexStart, lenLength;
	unsigned int mask = 0xff;
	int shift = 0;

	if (request_msg.buffer[reqStart+1] & 0x80)
	{
		lenLength = request_msg.buffer[reqStart+1] & 0x7f;
		indexStart = respStart+2;

		while (lenLength--)
		{
			response_msg.buffer[indexStart+lenLength] = 
				(unsigned char)((size & mask) >> shift);
			shift+=8;
			mask <<= shift;
		}
	}
	else
	{
		response_msg.buffer[respStart+1] = (unsigned char)(size & 0xff);
	}
}


int parseVarBind(int reqType, int index)
{
	int seglen = 0, id;
	tlvStructType name, value;
	int size = 0;
	
	extern const int maxData;

	parseTLV(request_msg.buffer, request_msg.index, &name);

	if ( request_msg.buffer[name.start] != SNMPDTYPE_OBJ_ID ) return -1;

	id = findEntry(&request_msg.buffer[name.vstart], name.len);

	if ((reqType == GET_REQUEST) || (reqType == SET_REQUEST))
	{
		seglen = name.nstart - name.start;
		COPY_SEGMENT_TEMP20110929(name);
		size = seglen;
	}
	else if (reqType == GET_NEXT_REQUEST)
	{
		response_msg.buffer[response_msg.index] = request_msg.buffer[name.start];

		if (++id >= maxData)
		{
			id = OID_NOT_FOUND;
			seglen = name.nstart - name.start;
			COPY_SEGMENT_TEMP20110929(name);
			size = seglen;
		}
		else
		{
			request_msg.index += name.nstart - name.start;

			getOID(id, &response_msg.buffer[response_msg.index+2], &response_msg.buffer[response_msg.index+1]);

			seglen = response_msg.buffer[response_msg.index+1]+2;
			response_msg.index += seglen ;
			size = seglen;
		}
	}

	parseTLV(request_msg.buffer, request_msg.index, &value);

	if (id != OID_NOT_FOUND)
	{
		unsigned char dataType;
		int len;

		if ((reqType == GET_REQUEST) || (reqType == GET_NEXT_REQUEST))
		{
			getEntry(id, &dataType, &response_msg.buffer[response_msg.index+2], &len);

			response_msg.buffer[response_msg.index] = dataType;
			response_msg.buffer[response_msg.index+1] = len;
			seglen = (2 + len);
			response_msg.index += seglen;

			request_msg.index += (value.nstart - value.start);

		}
		else if (reqType == SET_REQUEST)
		{
			setEntry(id, &request_msg.buffer[value.vstart], value.len, request_msg.buffer[value.start], index);
			seglen = value.nstart - value.start;
			COPY_SEGMENT_TEMP20110929(value);
		}
	}
	else
	{
		seglen = value.nstart - value.start;
		COPY_SEGMENT_TEMP20110929(value);

		errorIndex = index;
		errorStatus = NO_SUCH_NAME;
	}

	size += seglen;

	return size;
}


int parseSequence(int reqType, int index)
{
	int seglen;
	tlvStructType seq;
	int size = 0, respLoc;

	parseTLV(request_msg.buffer, request_msg.index, &seq);

	if ( request_msg.buffer[seq.start] != SNMPDTYPE_SEQUENCE ) return -1;

	seglen = seq.vstart - seq.start;
	respLoc = response_msg.index;
	COPY_SEGMENT_TEMP20110929(seq);

	size = parseVarBind( reqType, index );
	insertRespLen(seq.start, respLoc, size);
	size += seglen;

	return size;
}


int parseSequenceOf(int reqType)
{
	int seglen;
	tlvStructType seqof;
	int size = 0, respLoc;
	int index = 0;

	parseTLV(request_msg.buffer, request_msg.index, &seqof);

	if ( request_msg.buffer[seqof.start] != SNMPDTYPE_SEQUENCE_OF ) return -1;

	seglen = seqof.vstart - seqof.start;
	respLoc = response_msg.index;
	COPY_SEGMENT_TEMP20110929(seqof);

	while (request_msg.index < request_msg.len)
	{
		size += parseSequence( reqType, index++ );
	}

	insertRespLen(seqof.start, respLoc, size);

	return size;
}


int parseRequest()
{
	int ret, seglen;
	tlvStructType snmpreq, requestid, errStatus, errIndex;
	int size = 0, respLoc, reqType;

	parseTLV(request_msg.buffer, request_msg.index, &snmpreq);

	reqType = request_msg.buffer[snmpreq.start];

	if ( !VALID_REQUEST(reqType) ) return -1;

	seglen = snmpreq.vstart - snmpreq.start;
	respLoc = snmpreq.start;
	size += seglen;
	COPY_SEGMENT_TEMP20110929(snmpreq);

	response_msg.buffer[snmpreq.start] = GET_RESPONSE;

	parseTLV(request_msg.buffer, request_msg.index, &requestid);
	seglen = requestid.nstart - requestid.start;
	size += seglen;
	COPY_SEGMENT_TEMP20110929(requestid);

	parseTLV(request_msg.buffer, request_msg.index, &errStatus);
	seglen = errStatus.nstart - errStatus.start;
	size += seglen;
	COPY_SEGMENT_TEMP20110929(errStatus);

	parseTLV(request_msg.buffer, request_msg.index, &errIndex);
	seglen = errIndex.nstart - errIndex.start;
	size += seglen;
	COPY_SEGMENT_TEMP20110929(errIndex);

	ret = parseSequenceOf(reqType);
	if (ret == -1) return -1;
	else size += ret;

	insertRespLen(snmpreq.start, respLoc, size);

	if (errorStatus)
	{
		response_msg.buffer[errStatus.vstart] = errorStatus;
		response_msg.buffer[errIndex.vstart] = errorIndex + 1;
	}

	return size;
}


int parseCommunity()
{
	int seglen;
	tlvStructType community;
	int size=0;

	parseTLV(request_msg.buffer, request_msg.index, &community);

	if (!((request_msg.buffer[community.start] == SNMPDTYPE_OCTET_STRING) && (community.len == COMMUNITY_SIZE))) 
	{
		return -1;
	}

	if (!memcmp(&request_msg.buffer[community.vstart], (char *)COMMUNITY, COMMUNITY_SIZE))
	{
		seglen = community.nstart - community.start;
		size += seglen;
		COPY_SEGMENT_TEMP20110929(community);

		size += parseRequest();
	}
	else
	{
		return -1;
	}

	return size;
}


int parseVersion()
{
	int size = 0, seglen;
	tlvStructType tlv;

	size = parseTLV(request_msg.buffer, request_msg.index, &tlv);

	if (!((request_msg.buffer[tlv.start] == SNMPDTYPE_INTEGER) && (request_msg.buffer[tlv.vstart] == SNMP_V2C)))
		return -1;

	seglen = tlv.nstart - tlv.start;
	size += seglen;
	COPY_SEGMENT_TEMP20110929(tlv);
	size = parseCommunity();

	if (size == -1) return size;
	else return (size + seglen);
}


int parseSNMPMessage()
{
	int size = 0, seglen, respLoc;
	tlvStructType tlv;

	parseTLV(request_msg.buffer, request_msg.index, &tlv);

	if (request_msg.buffer[tlv.start] != SNMPDTYPE_SEQUENCE_OF) return -1;				//0字节为0x30判断

	seglen = tlv.vstart - tlv.start;		//2
	respLoc = tlv.start;
	COPY_SEGMENT_TEMP20110929(tlv);

	size = parseVersion();

	if (size == -1) return -1;
	else size += seglen;

	insertRespLen(tlv.start, respLoc, size);

	return 0;
}


void dumpCode(char* header, char* tail, unsigned char *buff, int len) 
{ 
	int i;

	printf("%s",header);

	for (i=0; i<len; i++) 
	{ 
		if ( i%16==0 )	printf("0x%04x : ", i); 
		printf("%02x ",buff[i]); 

		if ( i%16-15==0 )
		{
			int j; 
			printf("  "); 
			for (j=i-15; j<=i; j++)
			{
				if ( isprint(buff[j]) )	printf("%c", buff[j]);
				else					printf(".");
			}
			printf("\r\n"); 
		} 
	}

	if ( i%16!=0 ) 
	{ 
		int j; 
		int spaces=(len-i+16-i%16)*3+2; 
		for (j=0; j<spaces; j++) 	printf(" ");
		for (j=i-i%16; j<len; j++) 
		{
			if ( isprint(buff[j]) )	printf("%c", buff[j]);
			else					printf(".");
		}
	} 
	printf("%s",tail);
} 

void ipToByteArray(char *ip, unsigned char *pDes)
{
	unsigned int  i, ip1=0, ip2=0, ip3=0, ip4=0;
	char buff[32];
	unsigned int len = (unsigned int)strlen(ip);
	strcpy(buff, ip);

	for (i=0; i<len; i++)
	{
		if ( buff[i]=='.' )		buff[i] = ' ';
	}

	sscanf(buff, "%u %u %u %u", &ip1, &ip2, &ip3, &ip4);
	pDes[0] = ip1; pDes[1] = ip2; pDes[2] = ip3; pDes[3] = ip4;
}


int makeTrapVariableBindings(dataEntryType *oid_data, void *ptr, unsigned int *len)
{
	unsigned int j;

	((unsigned char*)ptr)[0] = 0x30;
	((unsigned char*)ptr)[1] = 0xff;
	((unsigned char*)ptr)[2] = 0x06;
	((unsigned char*)ptr)[3] = oid_data->oidlen;

	for (j = 0 ; j < oid_data->oidlen ; j++)
	{
		((unsigned char*)ptr)[j+4] = oid_data->oid[j];
	}

	switch(oid_data->dataType)
	{
	case SNMPDTYPE_OCTET_STRING :
	case SNMPDTYPE_OBJ_ID :
		{
			unsigned char *string = &((unsigned char*)ptr)[4+oid_data->oidlen+2];

			if ( oid_data->dataType==SNMPDTYPE_OCTET_STRING )
			{
				oid_data->dataLen = (unsigned char)strlen((char*)&oid_data->u.octetstring);
			}
			for (j = 0 ; j < oid_data->dataLen ; j++)
			{
				string[j] = oid_data->u.octetstring[j];
			}

			((unsigned char*)ptr)[4+oid_data->oidlen] = oid_data->dataType;
			((unsigned char*)ptr)[4+oid_data->oidlen+1] = oid_data->dataLen;
			((unsigned char*)ptr)[1] = 2 + oid_data->oidlen + 2 + oid_data->dataLen;
			*len = 4 + oid_data->oidlen + 2 + oid_data->dataLen;
		}
		break;

	case SNMPDTYPE_INTEGER :
	case SNMPDTYPE_TIME_TICKS :
	case SNMPDTYPE_COUNTER :
	case SNMPDTYPE_GAUGE :
		{
			oid_data->dataLen = 4;

			*(int*)(&((unsigned char*)ptr)[4+oid_data->oidlen+2]) = HTONL(oid_data->u.intval);

			((unsigned char*)ptr)[4+oid_data->oidlen] = oid_data->dataType;
			((unsigned char*)ptr)[4+oid_data->oidlen+1] = oid_data->dataLen;
			((unsigned char*)ptr)[1] = 2 + oid_data->oidlen + 2 + oid_data->dataLen;
			*len = 4 + oid_data->oidlen + 2 + oid_data->dataLen;
		}
		break;

	default : 
		return INVALID_DATA_TYPE;
	}

	return 1;
}

int SnmpXInit()
{
//	initTable();
	return 0;
}

unsigned char packet_trap[1024] = {0,};
int SnmpXTrapSend(char* managerIP, char* agentIP, char* community, dataEntryType enterprise_oid, unsigned int genericTrap, unsigned int specificTrap, unsigned int va_count, ...)
{
	unsigned int i;
	int packet_index = 0;
	int packet_buff1 = 0;
	int packet_buff2 = 0;
	int packet_buff3 = 0;
	unsigned char trap_agentip[4] = {0,};
	
	ipToByteArray(agentIP, trap_agentip);

	packet_trap[packet_index++] = 0x30; // ASN.1 Header

	packet_trap[packet_index] = 0xff; // pdu_length, temp
	packet_buff1 = packet_index++;

	packet_trap[packet_index++] = 0x02; // Version
	packet_trap[packet_index++] = 0x01;
	packet_trap[packet_index++] = 0x00;
	
	packet_trap[packet_index++] = 0x04; // Community
	packet_trap[packet_index++] = (unsigned char)strlen(community);
	memcpy(&(packet_trap[packet_index]), community, strlen(community));

	packet_index = packet_index + (unsigned char)strlen(community);

	packet_trap[packet_index++] = 0xa4; // trap
	packet_trap[packet_index] = 0xff; // length, temp
	packet_buff2 = packet_index++;

	packet_trap[packet_index++] = 0x06; // enterprise_oid
	packet_trap[packet_index++] = enterprise_oid.oidlen;
	for (i=0; i<enterprise_oid.oidlen; i++)
	{
		packet_trap[packet_index++] = enterprise_oid.oid[i];
	}
	
	packet_trap[packet_index++] = 0x40; // agent ip
	packet_trap[packet_index++] = 0x04;
	packet_trap[packet_index++] = trap_agentip[0];
	packet_trap[packet_index++] = trap_agentip[1];
	packet_trap[packet_index++] = trap_agentip[2];
	packet_trap[packet_index++] = trap_agentip[3];

	packet_trap[packet_index++] = 0x02; // Generic Trap
	packet_trap[packet_index++] = 0x01;
	packet_trap[packet_index++] = (unsigned char)genericTrap;

	packet_trap[packet_index++] = 0x02; // Specific Trap
	packet_trap[packet_index++] = 0x01;
	packet_trap[packet_index++] = (unsigned char)specificTrap;

	packet_trap[packet_index++] = 0x43; // Timestamp
	packet_trap[packet_index++] = 0x01;
	packet_trap[packet_index++] = 0x00;

	packet_trap[packet_index++] = 0x30; // Sequence of variable-bindings
	packet_trap[packet_index] = 0xff;
	packet_buff3 = packet_index++;
	
	// variable-bindings
	{
		va_list ap;
		unsigned int length_var_bindings = 0;
		unsigned int length_buff = 0;

		va_start (ap, va_count); 

		for (i=0; i<va_count; i++) 
		{
			dataEntryType* fff = va_arg(ap, dataEntryType*);
			makeTrapVariableBindings(fff, &(packet_trap[packet_index]), &length_buff);
			packet_index = packet_index + length_buff;
			length_var_bindings = length_var_bindings + length_buff;
		}
		packet_trap[packet_buff3] = length_var_bindings;
		va_end (ap);
	}
	packet_trap[packet_buff1] = packet_index - 2;
	packet_trap[packet_buff2] = packet_index - (9 + (unsigned char)strlen(community));
	{
		unsigned char svr_addr[6];    
		ipToByteArray(managerIP, svr_addr);
				
		Write_W5500_SOCK_2Byte(SOCK_SNMP, 	Sn_PORT, 		162);													// snmp 本地监听端口 161
		Write_W5500_SOCK_2Byte(SOCK_SNMP, 	Sn_DPORTR, 	162);													// 目标端口161  被动接收数据，实际目标port为收到的port端口 
		Write_W5500_SOCK_4Byte(SOCK_SNMP, 	Sn_DIPR, 		svr_addr);										// 目标IP		    被动接收数据，实际目标ip为收到的ip地址 				
		Write_SOCK_Data_Buffer(SOCK_SNMP, packet_trap, packet_index);									// 写数据
		Write_W5500_SOCK_2Byte(SOCK_SNMP, 	Sn_PORT, 		161);													// snmp 本地监听端口 161
		return 0;
	}
}

//Process_Socket_Data  接收数据并对数据进行处理
int SnmpXDaemon()
{	
	int len = 0;
  unsigned char  svr_addr[4];
	unsigned short svr_port;
 	//WDEBUG("Start SNMP Daemon(Agent) ");
			
		len = Read_SOCK_Data_Buffer(SOCK_SNMP, (unsigned char *)&request_msg.buffer[0]);	//svr_addr  svr_port
		if (len > 8)
		{
			 memcpy(svr_addr,request_msg.buffer,4);
			 svr_port = (request_msg.buffer[4]<<8)|request_msg.buffer[5];		
			 request_msg.len = len - 8; 	
			 for(len=0;len<request_msg.len;len++)
			 {
				 request_msg.buffer[len] = request_msg.buffer[len+8];		//31  49-8=41 0x29
			 }				
			 request_msg.buffer[len] = '\0';
       dumpCode("\r\n[Request]\r\n", "\r\n", request_msg.buffer, request_msg.len);    	//第1包[17]: 67 92 ; 第1包 36 A7  不同的2个字节，其他完全
       request_msg.index = 0;
       response_msg.index = 0;
       errorStatus = errorIndex = 0;    
       if (parseSNMPMessage() != -1)
       {
					Write_W5500_SOCK_2Byte(SOCK_SNMP, 	Sn_DPORTR, 	svr_port);	// 目标端口161  被动接收数据，实际目标port为收到的port端口 
					Write_W5500_SOCK_4Byte(SOCK_SNMP, 	Sn_DIPR, 		svr_addr);	// 目标IP		    被动接收数据，实际目标ip为收到的ip地址 
					Write_SOCK_Data_Buffer(SOCK_SNMP, response_msg.buffer, response_msg.index);
       }    
       dumpCode("\r\n[Response]\r\n", "\r\n", response_msg.buffer, response_msg.index);
    }  
		return(0);
}

//snmpget -v 1 -cpublic 192.168.1.111 .1.3.6.1.4.1.0.1.0
//snmpset -v 1 -cpublic 192.168.1.111 .1.3.6.1.4.1.0.2.0 i 0
