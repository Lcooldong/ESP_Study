#ifndef __MYMODBUS_H__
#define __MYMODBUS_H_

#include "CRC16.h"
#include "CRC.h"

#define RECEIVE
//#define DEBUG_MODBUS

#define WRITE_COIL 0x05
#define COIL_TRUE 0xFF
#define COIL_FALSE 0x00
#define ADDRESS_LENGTH 100
#define TIME_OUT 500
#define DEFAULT_LENGTH 8
#define MULTIPLE_DATA_START 7



class MyModbus
{
private:

    Stream* RS485;

    CRC16 _crc;
    uint8_t readCount = 0;
    byte recv[ADDRESS_LENGTH] = {0,};
    uint64_t rs485Time = 0;
    uint64_t RS485_TimeOut = 0;

    bool receivedFlag = false;
    bool whileReadingFlag = false;

    typedef enum
    {
    READ_COILS                    = 0x01,
    READ_DISCRETE_INPUTS          = 0x02,
    READ_HOLDING_REGISTERS        = 0X03,
    READ_INPUT_REGISTERS          = 0X04,
    WRITE_SINGLE_COIL             = 0x05,
    WRITE_SINGLE_REGISTER         = 0X06,
    WRITE_MULTIPLE_COILS          = 0x0F,
    WRITE_MULTIPLE_REGISTERS      = 0x10, 
    MASK_WRITE_REGISTER           = 0x16, 
    READ_WRITE_MULTIPLE_REGISTERS = 0x17,
    READ_FIFO_QUEUE               = 0x18

    }FUNCTION_CODE;

public:
    MyModbus(Stream* serial);
    ~MyModbus();

    MyModbus(Stream* serial, uint8_t slave_id);

    void initModbusCRC(CRC16* crc);
    uint8_t* caculateModbusCRC(byte* packet, uint8_t length);
    void printReadPacket();
    void printPacket(uint16_t length);
    void pollModbus();
    int getFunctionCode();
    void readCoils(uint16_t startAddress, uint16_t count);
    void readDiscreteInputs(uint16_t startAddress, uint16_t count);
    void readHoldingRegisters(uint16_t startAddress, uint16_t count);
    void readInputRegisters(uint16_t startAddress, uint16_t count);

    void writeSingleCoil(uint16_t startAddress);
    void writeSingleRegister(uint16_t startAddress, uint16_t register_value);
    void writeMultipleCoils(uint16_t startAddress, uint16_t count, uint16_t length);
    void writeMultipleRegisters(uint16_t startAddress,  uint8_t data_count);


    uint8_t _slave_id = 1;

    uint8_t discreteInputs[ADDRESS_LENGTH] = {0,};
    uint8_t my_Coils[ADDRESS_LENGTH] = {0,};
    int16_t inputRegisters[ADDRESS_LENGTH] = {0,};
    int16_t holdingRegisters[ADDRESS_LENGTH] = {0,};
};




#endif