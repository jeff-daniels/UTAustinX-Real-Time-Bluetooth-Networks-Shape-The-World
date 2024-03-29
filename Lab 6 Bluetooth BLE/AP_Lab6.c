// AP_Lab6.c
// Runs on either MSP432 or TM4C123
// see GPIO.c file for hardware connections 

// Daniel Valvano and Jonathan Valvano
// November 20, 2016
// CC2650 booster or CC2650 LaunchPad, CC2650 needs to be running SimpleNP 2.2 (POWERSAVE)

#include <stdint.h>
#include "../inc/UART0.h"
#include "../inc/UART1.h"
#include "../inc/AP.h"
#include "AP_Lab6.h"
//**debug macros**APDEBUG defined in AP.h********
#ifdef APDEBUG
#define OutString(STRING) UART0_OutString(STRING)
#define OutUHex(NUM) UART0_OutUHex(NUM)
#define OutUHex2(NUM) UART0_OutUHex2(NUM)
#define OutChar(N) UART0_OutChar(N)
#else
#define OutString(STRING)
#define OutUHex(NUM)
#define OutUHex2(NUM)
#define OutChar(N)
#endif

//****links into AP.c**************
extern const uint32_t RECVSIZE;
extern uint8_t RecvBuf[];
typedef struct characteristics{
  uint16_t theHandle;          // each object has an ID
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data, stored little endian
  void (*callBackRead)(void);  // action if SNP Characteristic Read Indication
  void (*callBackWrite)(void); // action if SNP Characteristic Write Indication
}characteristic_t;
extern const uint32_t MAXCHARACTERISTICS;
extern uint32_t CharacteristicCount;
extern characteristic_t CharacteristicList[];
typedef struct NotifyCharacteristics{
  uint16_t uuid;               // user defined 
  uint16_t theHandle;          // each object has an ID (used to notify)
  uint16_t CCCDhandle;         // generated/assigned by SNP
  uint16_t CCCDvalue;          // sent by phone to this object
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data array, stored little endian
  void (*callBackCCCD)(void);  // action if SNP CCCD Updated Indication
}NotifyCharacteristic_t;
extern const uint32_t NOTIFYMAXCHARACTERISTICS;
extern uint32_t NotifyCharacteristicCount;
extern NotifyCharacteristic_t NotifyCharacteristicList[];


//**************Lab 6 routines*******************
// **********SetFCS**************
// helper function, add check byte to message
// assumes every byte in the message has been set except the FCS
// used the length field, assumes less than 256 bytes
// FCS = 8-bit EOR(all bytes except SOF and the FCS itself)
// Inputs: pointer to message
//         stores the FCS into message itself
// Outputs: none
void SetFCS(uint8_t *msg){
//****You implement this function as part of Lab 6*****
	uint32_t size = AP_GetSize(msg);
	uint8_t data;
	uint8_t fcs = 0;
  msg++;														// SOF
  data=*msg; fcs=fcs^data; msg++;   // LSB length
  data=*msg; fcs=fcs^data; msg++;   // MSB length
  data=*msg; fcs=fcs^data; msg++;   // CMD0
  data=*msg; fcs=fcs^data; msg++;   // CMD1
  for(int i=0;i<size;i++){
    data=*msg; fcs=fcs^data; msg++; // payload
  }
  *msg = fcs;                      // FCS stored at last entry in message

  
}
//*************BuildGetStatusMsg**************
// Create a Get Status message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will Get Status
void BuildGetStatusMsg(uint8_t *msg){
// hint: see NPI_GetStatus in AP.c
//****You implement this function as part of Lab 6*****
	uint8_t *pt;
	pt = msg;
	*pt = SOF;	pt++;	// SOF
	*pt = 0x00;	pt++;	// LSB length = 0						
	*pt = 0x00;	pt++;	// MSB length
	*pt = 0x55;	pt++;	// SNP Get Status	CMD0
	*pt = 0x06;	pt++;	// 								CMD1
	SetFCS(msg);					// FCS
}
//*************Lab6_GetStatus**************
// Get status of connection, used in Lab 6
// Input:  none
// Output: status 0xAABBCCDD
// AA is GAPRole Status
// BB is Advertising Status
// CC is ATT Status
// DD is ATT method in progress
uint32_t Lab6_GetStatus(void){volatile int r; uint8_t sendMsg[8];
  OutString("\n\rGet Status");
  BuildGetStatusMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return (RecvBuf[4]<<24)+(RecvBuf[5]<<16)+(RecvBuf[6]<<8)+(RecvBuf[7]);
}

//*************BuildGetVersionMsg**************
// Create a Get Version message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will Get Status
void BuildGetVersionMsg(uint8_t *msg){
// hint: see NPI_GetVersion in AP.c
//****You implement this function as part of Lab 6*****
	uint8_t *pt;
	pt = msg;
	*pt = SOF;	pt++;	// SOF
	*pt = 0x00;	pt++;	// LSB length = 0						
	*pt = 0x00;	pt++;	// MSB length
	*pt = 0x35;	pt++;	// SNP Get Status	CMD0
	*pt = 0x03;	pt++;	// 								CMD1
	SetFCS(msg);					// FCS
  
  
}
//*************Lab6_GetVersion**************
// Get version of the SNP application running on the CC2650, used in Lab 6
// Input:  none
// Output: version
uint32_t Lab6_GetVersion(void){volatile int r;uint8_t sendMsg[8];
  OutString("\n\rGet Version");
  BuildGetVersionMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE); 
  return (RecvBuf[5]<<8)+(RecvBuf[6]);
}

//*************BuildAddServiceMsg**************
// Create an Add service message, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        pointer to empty buffer of at least 9 bytes
// Output none
// build the necessary NPI message that will add a service
void BuildAddServiceMsg(uint16_t uuid, uint8_t *msg){
//****You implement this function as part of Lab 6*****
  uint8_t *pt;
	pt = msg;
	*pt = SOF;	pt++;					// SOF
	*pt = 0x03;	pt++;					// LSB length = 3						
	*pt = 0x00;	pt++;					// MSB length
	*pt = 0x35;	pt++;					// SNP Add Service	CMD0
	*pt = 0x81;	pt++;					// 									CMD1
	*pt = 0x01;	pt++;					// Primary Service
	*pt = uuid & 0xFF; pt++;	// UUID parameter
	*pt = uuid >> 8; pt++;		// UUID parameter
	SetFCS(msg);							// FCS
  
}
//*************Lab6_AddService**************
// Add a service, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
// Output APOK if successful,
//        APFAIL if SNP failure
int Lab6_AddService(uint16_t uuid){ int r; uint8_t sendMsg[12];
  OutString("\n\rAdd service");
  BuildAddServiceMsg(uuid,sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);  
  return r;
}
//*************AP_BuildRegisterServiceMsg**************
// Create a register service message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will register a service
void BuildRegisterServiceMsg(uint8_t *msg){
//****You implement this function as part of Lab 6*****
  uint8_t *pt;
	pt = msg;
	*pt = SOF;	pt++;	// SOF
	*pt = 0x00;	pt++;	// LSB length = 0						
	*pt = 0x00;	pt++;	// MSB length
	*pt = 0x35;	pt++;	// SNP Get Status	CMD0
	*pt = 0x84;	pt++;	// 								CMD1
	SetFCS(msg);					// FCS
  
}
//*************Lab6_RegisterService**************
// Register a service, used in Lab 6
// Inputs none
// Output APOK if successful,
//        APFAIL if SNP failure
int Lab6_RegisterService(void){ int r; uint8_t sendMsg[8];
  OutString("\n\rRegister service");
  BuildRegisterServiceMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return r;
}

//*************BuildAddCharValueMsg**************
// Create a Add Characteristic Value Declaration message, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        permission is GATT Permission, 0=none,1=read,2=write, 3=Read+write 
//        properties is GATT Properties, 2=read,8=write,0x0A=read+write, 0x10=notify
//        pointer to empty buffer of at least 14 bytes
// Output none
// build the necessary NPI message that will add a characteristic value
void BuildAddCharValueMsg(uint16_t uuid,  
  uint8_t permission, uint8_t properties, uint8_t *msg){
// set RFU to 0 and
// set the maximum length of the attribute value=512
// for a hint see NPI_AddCharValue in AP.c
// for a hint see first half of AP_AddCharacteristic and first half of AP_AddNotifyCharacteristic
//****You implement this function as part of Lab 6*****

	uint8_t *pt;
	pt = msg;
	*pt = SOF;	pt++;				// SOF
	*pt = 0x08;	pt++;				// LSB length=8						
	*pt = 0x00;	pt++;				// MSB length
	*pt = 0x35;	pt++;				// SNP Add Characteristic Value	CMD0
	*pt = 0x82;	pt++;				// 															CMD1
	*pt = permission; pt++;	// 0=none,1=read,2=write, 3=Read+write, GATT Permission
	*pt = properties; pt++; // 2=read,8=write,0x0A=read+write,0x10=notify, GATT Properties
	*pt = 0x00; pt++;				// Placeholder
	*pt = 0x00;	pt++;				// RFU=0
	*pt = 0x00; pt++;				// Maximum length of the attribute value=512
	*pt = 0x02; pt++;				// Maximum length of the attribute value=512
	*pt = uuid&0xFF; pt++;	// UUID parameter
	*pt = uuid>>8; pt++;		// UUID parameter
	SetFCS(msg);						// FCS
	
}

//*************BuildAddCharDescriptorMsg**************
// Create a Add Characteristic Descriptor Declaration message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 20 bytes
//        pointer to empty buffer of at least 32 bytes
// Output none
// build the necessary NPI message that will add a Descriptor Declaration
void BuildAddCharDescriptorMsg(char name[], uint8_t *msg){
// set length and maxlength to the string length
// set the permissions on the string to read
// for a hint see NPI_AddCharDescriptor in AP.c
// for a hint see second half of AP_AddCharacteristic
//****You implement this function as part of Lab 6*****
  int i, string_length;

	// determine string_length of name
	i=0;
	while((i<20)&&(name[i])){
    msg[11+i] = name[i]; i++;
  }
	string_length = i+1;

	uint8_t *pt;
	pt = msg;
	*pt = SOF;	pt++;							// SOF
	*pt = 6+string_length;	pt++;	// LSB length determined at run time, 6+string_length					
	*pt = 0x00;	pt++;							// MSB length
	*pt = 0x35;	pt++;							// SNP Add Characteristic Descriptor	CMD0
	*pt = 0x83;	pt++;							// 																		CMD1
	*pt = 0x80; pt++;							// User Description String
	*pt = 0x01; pt++; 						// GATT Read Permission, 0=none,1=read,2=write, 3=Read+write
	*pt = string_length;	pt++;		// string length
	*pt = 0;	pt++;
	*pt = string_length;	pt++;		// string length
  *pt = 0;	pt++;
	msg[11+i] = 0;								// Because
	SetFCS(msg);									// FCS
  
}

//*************Lab6_AddCharacteristic**************
// Add a read, write, or read/write characteristic, used in Lab 6
//        for notify properties, call AP_AddNotifyCharacteristic 
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        thesize is the number of bytes in the user data 1,2,4, or 8 
//        pt is a pointer to the user data, stored little endian
//        permission is GATT Permission, 0=none,1=read,2=write, 3=Read+write 
//        properties is GATT Properties, 2=read,8=write,0x0A=read+write
//        name is a null-terminated string, maximum length of name is 20 bytes
//        (*ReadFunc) called before it responses with data from internal structure
//        (*WriteFunc) called after it accepts data into internal structure
// Output APOK if successful,
//        APFAIL if name is empty, more than 8 characteristics, or if SNP failure
int Lab6_AddCharacteristic(uint16_t uuid, uint16_t thesize, void *pt, uint8_t permission,
  uint8_t properties, char name[], void(*ReadFunc)(void), void(*WriteFunc)(void)){
  int r; uint16_t handle; 
  uint8_t sendMsg[32];  
  if(thesize>8) return APFAIL;
  if(name[0]==0) return APFAIL;       // empty name
  if(CharacteristicCount>=MAXCHARACTERISTICS) return APFAIL; // error
  BuildAddCharValueMsg(uuid,permission,properties,sendMsg);
  OutString("\n\rAdd CharValue");
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  handle = (RecvBuf[7]<<8)+RecvBuf[6]; // handle for this characteristic
  OutString("\n\rAdd CharDescriptor");
  BuildAddCharDescriptorMsg(name,sendMsg);
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  CharacteristicList[CharacteristicCount].theHandle = handle;
  CharacteristicList[CharacteristicCount].size = thesize;
  CharacteristicList[CharacteristicCount].pt = (uint8_t *) pt;
  CharacteristicList[CharacteristicCount].callBackRead = ReadFunc;
  CharacteristicList[CharacteristicCount].callBackWrite = WriteFunc;
  CharacteristicCount++;
  return APOK; // OK
} 
  

//*************BuildAddNotifyCharDescriptorMsg**************
// Create a Add Notify Characteristic Descriptor Declaration message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 20 bytes
//        pointer to empty buffer of at least bytes
// Output none
// build the necessary NPI message that will add a Descriptor Declaration
void BuildAddNotifyCharDescriptorMsg(char name[], uint8_t *msg){
// set length and maxlength to the string length
// set the permissions on the string to read
// set User Description String
// set CCCD parameters read+write
// for a hint see NPI_AddCharDescriptor4 in VerySimpleApplicationProcessor.c
// for a hint see second half of AP_AddNotifyCharacteristic
//****You implement this function as part of Lab 6*****
  int i, string_length;

	// determine string_length of name
	i=0;
	while((i<19)&&(name[i])){
    msg[12+i] = name[i]; i++;
  }
	string_length = i+1;

	uint8_t *pt;
	pt = msg;
	*pt = SOF;	pt++;							// SOF
	*pt = 7+string_length;	pt++;	// LSB length determined at run time, 7+string_length					
	*pt = 0x00;	pt++;							// MSB length
	*pt = 0x35;	pt++;							// SNP Add Characteristic Descriptor	CMD0
	*pt = 0x83;	pt++;							// 																		CMD1
	*pt = 0x84; pt++;							// User Description String
	*pt = 0x03; pt++;							// CCD parameters read+write
	*pt = 0x01; pt++; 						// GATT Read Permission, 0=none,1=read,2=write, 3=Read+write
	*pt = string_length;	pt++;		// string length
	*pt = 0;	pt++;
	*pt = string_length;	pt++;		// string length
  *pt = 0;	pt++;
	msg[12+i] = 0;								// Because
	SetFCS(msg);									// FCS
  
}
  
//*************Lab6_AddNotifyCharacteristic**************
// Add a notify characteristic, used in Lab 6
//        for read, write, or read/write characteristic, call AP_AddCharacteristic 
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        thesize is the number of bytes in the user data 1,2,4, or 8 
//        pt is a pointer to the user data, stored little endian
//        name is a null-terminated string, maximum length of name is 20 bytes
//        (*CCCDfunc) called after it accepts , changing CCCDvalue
// Output APOK if successful,
//        APFAIL if name is empty, more than 4 notify characteristics, or if SNP failure
int Lab6_AddNotifyCharacteristic(uint16_t uuid, uint16_t thesize, void *pt,   
  char name[], void(*CCCDfunc)(void)){
  int r; uint16_t handle; 
  uint8_t sendMsg[36];  
  if(thesize>8) return APFAIL;
  if(NotifyCharacteristicCount>=NOTIFYMAXCHARACTERISTICS) return APFAIL; // error
  BuildAddCharValueMsg(uuid,0,0x10,sendMsg);
  OutString("\n\rAdd Notify CharValue");
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  handle = (RecvBuf[7]<<8)+RecvBuf[6]; // handle for this characteristic
  OutString("\n\rAdd CharDescriptor");
  BuildAddNotifyCharDescriptorMsg(name,sendMsg);
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  NotifyCharacteristicList[NotifyCharacteristicCount].uuid = uuid;
  NotifyCharacteristicList[NotifyCharacteristicCount].theHandle = handle;
  NotifyCharacteristicList[NotifyCharacteristicCount].CCCDhandle = (RecvBuf[8]<<8)+RecvBuf[7]; // handle for this CCCD
  NotifyCharacteristicList[NotifyCharacteristicCount].CCCDvalue = 0; // notify initially off
  NotifyCharacteristicList[NotifyCharacteristicCount].size = thesize;
  NotifyCharacteristicList[NotifyCharacteristicCount].pt = (uint8_t *) pt;
  NotifyCharacteristicList[NotifyCharacteristicCount].callBackCCCD = CCCDfunc;
  NotifyCharacteristicCount++;
  return APOK; // OK
}

//*************BuildSetDeviceNameMsg**************
// Create a Set GATT Parameter message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 24 bytes
//        pointer to empty buffer of at least 36 bytes
// Output none
// build the necessary NPI message to set Device name
void BuildSetDeviceNameMsg(char name[], uint8_t *msg){
// for a hint see NPI_GATTSetDeviceNameMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_GATTSetDeviceName in AP.c
//****You implement this function as part of Lab 6*****
  
  int i, string_length;

	// determine string_length of name
	i=0;
	while((i<20)&&(name[i])){
    msg[8+i] = name[i]; i++;
  }
	string_length = i+1;

	uint8_t *pt;
	pt = msg;
	*pt = SOF;	pt++;							// SOF
	*pt = 2+string_length;	pt++;	// LSB length determined at run time, 2+string_length					
	*pt = 0x00;	pt++;							// MSB length
	*pt = 0x35;	pt++;							// SNP Add Characteristic Descriptor	CMD0
	*pt = 0x8C;	pt++;							// 																		CMD1
	*pt = 0x01; pt++;							// Generic Access Service
	*pt	= 0x00; pt++;							// Device Name
	*pt = 0x00; pt++;							// Device Name
	SetFCS(msg);									// FCS
}
//*************BuildSetAdvertisementData1Msg**************
// Create a Set Advertisement Data message, used in Lab 6
// Inputs pointer to empty buffer of at least 16 bytes
// Output none
// build the necessary NPI message for Non-connectable Advertisement Data
void BuildSetAdvertisementData1Msg(uint8_t *msg){
// for a hint see NPI_SetAdvertisementMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_SetAdvertisement1 in AP.c
// Non-connectable Advertisement Data
// GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR  
// Texas Instruments Company ID 0x000D
// TI_ST_DEVICE_ID = 3
// TI_ST_KEY_DATA_ID
// Key state=0
//****You implement this function as part of Lab 6*****
  
	uint8_t *pt;
	pt = msg;
	*pt = SOF;	pt++;							// SOF
	*pt = 11;	pt++;								// LSB length=11				
	*pt = 0x00;	pt++;							// MSB length
	*pt = 0x55;	pt++;							// SNP Set Advertisement Data	CMD0
	*pt = 0x43;	pt++;							// 														CMD1
	*pt = 0x01; pt++;							// Not connected Advertisement Data
	*pt = 0x02; pt++;							// GAP_ADTYPE_FLAGS 
	*pt = 0x01;	pt++;							// DISCOVERABLE 
	*pt = 0x06; pt++;							// | no BREDR  
	*pt = 0x06; pt++;							// length
	*pt = 0xFF; pt++;							// manufacturer specific
	*pt = 0x0D; pt++;							// Texas Instruments Company ID
	*pt = 0x00;	pt++;							// Texas Instruments Company ID
	*pt = 0x03; pt++;							// TI_ST_DEVICE_ID
	*pt = 0x00; pt++;							// TI_ST_KEY_DATA_ID
	*pt = 0x00; pt++;							// Key State
	SetFCS(msg);									// FCS
  
}

//*************BuildSetAdvertisementDataMsg**************
// Create a Set Advertisement Data message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 24 bytes
//        pointer to empty buffer of at least 36 bytes
// Output none
// build the necessary NPI message for Scan Response Data
void BuildSetAdvertisementDataMsg(char name[], uint8_t *msg){
// for a hint see NPI_SetAdvertisementDataMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_SetAdvertisementData in AP.c
//****You implement this function as part of Lab 6*****

  int i, string_length;
 	uint8_t *pt;

	pt = msg;
	*pt = SOF;	pt++;							// SOF
	*pt = 0x00;	pt++;							// LSB length determined at run time					
	*pt = 0x00;	pt++;							// MSB length
	*pt = 0x55;	pt++;							// SNP Set Advertisement Data	CMD0
	*pt = 0x43;	pt++;							// 														CMD1
	*pt = 0x00; pt++;							// Scan Response Data
	*pt = 0x00; pt++; 						// length=TBD
	*pt = 0x09; pt++;							// type=LOCAL_NAME_COMPLETE
	
	// insert name here
	i=0;
	while((i<24)&&(name[i])){
		*pt = name[i];
		pt++;
		i++;
	}
	string_length  = i+1;
	
	// connection interval range
	*pt = 0x05; pt++;							// length of this data
	*pt = 0x12; pt++;							// GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE
	*pt = 0x50; pt++;							// DEFAULT_DESIRED_MIN_CONN_INTERVAL
	*pt = 0x00; pt++;
	*pt = 0x20; pt++;							// DEFAULT_DESIRED_MAX_CONN_INTERVAL
	*pt = 0x03; pt++;

	// Tx power level
	*pt = 0x02; pt++;							// length of this data
	*pt = 0x0A; pt++;							// GAP_ADTYPE_POWER_LEVEL
	*pt = 0x00; pt++;							// 0dBm
	*pt = 0x00; pt++;							// Because

	msg[1] = string_length+11;		// LSB length=string_length+11
	msg[6] = string_length;				// length
	SetFCS(msg);									// FCS 
}
//*************BuildStartAdvertisementMsg**************
// Create a Start Advertisement Data message, used in Lab 6
// Inputs advertising interval
//        pointer to empty buffer of at least 20 bytes
// Output none
// build the necessary NPI message to start advertisement
void BuildStartAdvertisementMsg(uint16_t interval, uint8_t *msg){
// for a hint see NPI_StartAdvertisementMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_StartAdvertisement in AP.c
//****You implement this function as part of Lab 6*****		
	uint8_t *pt;

	pt = msg;
  *pt = SOF; pt++; 								// SOF
	*pt = 14; pt++; 								// LSB length=14
	*pt = 0x00; pt++; 							// MSB
  *pt = 0x55; pt++;								// SNP Start Advertisement	CMD0
	*pt = 0x42; pt++; 							// 													CMD1
  *pt = 0x00; pt++; 							// Connectable Undirected Advertisements
  *pt = 0x00; pt++; 							// Advertise infinitely.
	*pt = 0x00; pt++;
	*pt = (interval & 0xFF); pt++; // Advertising Interval (interval * 0.625 ms=62.5ms)
	*pt = (interval >> 8); pt++;
  *pt = 0x00; pt++;     // Filter Policy RFU
  *pt = 0x00; pt++;     // Initiator Address Type RFU
	*pt = 0x00; pt++;			// RFU
	*pt = 0x01; pt++;
	*pt = 0x00; pt++;
	*pt = 0x00; pt++;
	*pt = 0x00; pt++;
	*pt = 0xC5; pt++;
  *pt = 0x02; pt++;     // Advertising will restart with connectable advertising when a connection is terminated
  SetFCS(msg);					// FCS 
}

//*************Lab6_StartAdvertisement**************
// Start advertisement, used in Lab 6
// Input:  none
// Output: APOK if successful,
//         APFAIL if notification not configured, or if SNP failure
int Lab6_StartAdvertisement(void){volatile int r; uint8_t sendMsg[40];
  OutString("\n\rSet Device name");
  BuildSetDeviceNameMsg("Shape the World",sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement1");
  BuildSetAdvertisementData1Msg(sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement Data");
  BuildSetAdvertisementDataMsg("Shape the World",sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rStartAdvertisement");
  BuildStartAdvertisementMsg(100,sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return r;
}

