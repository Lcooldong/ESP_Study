#include "MyModbus.h"
 

MyModbus::MyModbus(Stream* serial)
{
    _crc.reset(CRC16_MODBUS_POLYNOME,
            CRC16_MODBUS_INITIAL,
            CRC16_MODBUS_XOR_OUT,
            CRC16_MODBUS_REV_IN,
            CRC16_MODBUS_REV_OUT);

    RS485 = serial;
}

MyModbus::MyModbus(Stream* serial, uint8_t slave_id)
{
    _crc.reset(CRC16_MODBUS_POLYNOME,
            CRC16_MODBUS_INITIAL,
            CRC16_MODBUS_XOR_OUT,
            CRC16_MODBUS_REV_IN,
            CRC16_MODBUS_REV_OUT);

    _slave_id = slave_id;     
    RS485 = serial;   
}


MyModbus::~MyModbus()
{

}


void MyModbus::initModbusCRC(CRC16* crc)
{
  crc->reset(CRC16_MODBUS_POLYNOME,
            CRC16_MODBUS_INITIAL,
            CRC16_MODBUS_XOR_OUT,
            CRC16_MODBUS_REV_IN,
            CRC16_MODBUS_REV_OUT);
}

uint8_t* MyModbus::caculateModbusCRC(byte* packet, uint8_t length) 
{
    static uint8_t _crcArray[2] = {0,};
    uint16_t _crcResult = 0;

    for (int i = 0; i < length - 2; i++)
    {
      _crc.add(packet[i]);
      // Serial.printf("INPUT ==> 0x%02X\r\n", _packet[i]);
    }
    
    _crcResult = _crc.calc();

    _crcArray[0] = _crcResult & 0b11111111;
    _crcArray[1] = _crcResult >> 8;

    // Serial.printf( "CRC_L : 0x%02X | CRC_H : 0x%02X => %04X\r\n", _crcArray[0], _crcArray[1] , CRC_RESULT);

    initModbusCRC(&(MyModbus::_crc));

    return (uint8_t*)_crcArray;
}


void MyModbus::printReadPacket()
{
  switch (readCount)
  {
  case 1:
    Serial.printf("[%d]FC              : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 2:
    Serial.printf("[%d]Address_H       : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 3:
    Serial.printf("[%d]Address_L       : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 4:
    Serial.printf("[%d]Coil_Value      : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 5:
    Serial.printf("[%d]Register_Length : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 6:
    Serial.printf("[%d]Checksum_L      : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  case 7:
    Serial.printf("[%d]Checksum_H      : 0x%02X\r\n", readCount, recv[readCount]);
    break;
  }
  Serial.println("------------------------------------------------------");
}

void MyModbus::printPacket(uint16_t _length)
{
  for (int i = 0; i < _length; i++)
  {
    Serial.printf("0x%02X | ", recv[i]);
  }
  Serial.println();
}





void MyModbus::pollModbus()
{
  #ifdef RECEIVE
  // Receive
  if(RS485->available())
  {    
    if(millis() - rs485Time > 1)
    {
      rs485Time = millis();
      int ch = RS485->read();

      uint16_t coil_register_count = 0;
      uint16_t startAddress = 0;

      if(ch != -1)
      {
#ifdef DEBUG_MODBUS
        Serial.printf("READ : 0x%02X\r\n", ch);
#endif
        whileReadingFlag = true;
        receivedFlag = false;
        RS485_TimeOut = 0;
      }

      if(ch == _slave_id && readCount == 0)
      {
        // Serial.printf("[%d]ID         : 0x%X\r\n", readCount, SLAVE_ID);
        recv[0] = _slave_id;
        readCount = 1;
      }
      else if(readCount > 0 )
      {
        recv[readCount] = ch;        
        readCount++;

      }



      if(readCount >= DEFAULT_LENGTH)  // Write Single Coil, Register, Read
      {   
          
          uint8_t fc = recv[1];
          uint8_t* crcReceived;
          startAddress = recv[2] * 256 + recv[3];
          coil_register_count = (recv[4] << 8) + recv[5];
          uint8_t data_count = recv[6];   // Multiple
          uint16_t data_length = data_count + DEFAULT_LENGTH + 1;

          if( (fc > 6) && (readCount == data_length))
          {
            // Serial.printf("[%d]NUM Data : %d [%d]\r\n", readCount, num_data_length, data_length);
            crcReceived = caculateModbusCRC(recv, data_length);
          }
  
          else
          {
            crcReceived = caculateModbusCRC(recv, DEFAULT_LENGTH);
          }

                   
          // Serial.printf("CRC ==> 0x%02X  | 0x%02X\r\n", crcReceived[0], crcReceived[1]);

          if(crcReceived[0] == recv[readCount - 2] && crcReceived[1] == recv[readCount -1])
          {
            getFunctionCode();
            printPacket(readCount);

            switch (fc){

            case READ_COILS:
            {
              readCoils(startAddress, coil_register_count);               
              break;
            }

            case READ_DISCRETE_INPUTS:
            {
              readDiscreteInputs(startAddress, coil_register_count);
              break;
            }


            case READ_HOLDING_REGISTERS:
            {

              readHoldingRegisters(startAddress, coil_register_count);
              break;
            }
            
            case READ_INPUT_REGISTERS:
            {
              readInputRegisters(startAddress, coil_register_count);
              break;
            }

            case WRITE_SINGLE_COIL:
            {
              writeSingleCoil(startAddress);
              break;
            }

            case WRITE_SINGLE_REGISTER:
            {
              writeSingleRegister(startAddress, coil_register_count);
              break;
            }       

            case WRITE_MULTIPLE_COILS:
            {
              writeMultipleCoils(startAddress, coil_register_count, data_count);
              break;
            }
              

            case WRITE_MULTIPLE_REGISTERS:
            {
              writeMultipleRegisters(startAddress, data_count);
              break;
            }

            default:
              break;
            } 

            Serial.printf("Receive Complete \r\n");
            receivedFlag = true;
            readCount = 0;
          }
          whileReadingFlag = false;
      }

      if(receivedFlag)
      {
        startAddress = recv[2] * 256 + recv[3];
        Serial.printf("StartAddress : %d\r\n", startAddress);
      }

      
    }
  }
  else
  {
    if((millis() - RS485_TimeOut > TIME_OUT)  && !whileReadingFlag)
    {
      RS485_TimeOut = millis();
      // Serial.printf("RS485 - TIMEOUT %d\r\n", TIME_OUT);
      readCount = 0;
    }
  }


#endif
}

int MyModbus::getFunctionCode()
{
   switch (recv[1]){

    case READ_COILS:
    {
      Serial.printf("[Read Coils]\r\n");
      return READ_COILS;
      break;
    }

    case READ_DISCRETE_INPUTS:
    {
      Serial.printf("[Read Discrete Inputs]\r\n");
      return READ_DISCRETE_INPUTS;
      break;
    }


    case READ_HOLDING_REGISTERS:
    {
      Serial.printf("[Read Holding Registers]\r\n");

      return READ_HOLDING_REGISTERS;
      break;
    }
    
    case READ_INPUT_REGISTERS:
    {
      Serial.printf("[Read Input Registers]\r\n");
      return READ_INPUT_REGISTERS;
      break;
    }

    case WRITE_SINGLE_COIL:
    {
      Serial.printf("[Write Single Coils]\r\n");
      return WRITE_SINGLE_COIL;  
      break;
    }        

    case WRITE_MULTIPLE_COILS:
    {
      Serial.printf("[Write Multiple Coils]\r\n");
      return WRITE_MULTIPLE_COILS;
      break;
    }
      

    case WRITE_MULTIPLE_REGISTERS:
    {
      Serial.printf("[Write Multiple Registers]\r\n");
      return WRITE_MULTIPLE_REGISTERS;
      break;
    }

    default:
      return -1;
      break;
    } 
}

void MyModbus::readCoils(uint16_t startAddress, uint16_t count)
{
  uint16_t readingCount = (count / DEFAULT_LENGTH) + 1;  // % 가 나머지
  byte readingCoilsByte[readingCount] = {0, };
  uint8_t sendCount = 1;  // 기본 자리 1


  Serial.printf("Read Return Data Count : %d\r\n", readingCount);

  

  for (uint16_t i = 0; i < readingCount; i++)
  {  
    byte _temp = 0;
    for (uint16_t j = 0; j < count; j++)
    {
      _temp += my_Coils[startAddress + (j % DEFAULT_LENGTH) + (i * DEFAULT_LENGTH)] << j;
    }
    readingCoilsByte[i] = _temp;
    
    
    Serial.printf("RETURN COIL BYTE[%d] : 0x%02X\r\n", i, _temp);
  }

  if(count > DEFAULT_LENGTH)
  {
    for (uint16_t i = 1; i < readingCount; i++)
    {
      if((readingCoilsByte[i]) >= ( 0x01 ))
      {
        sendCount++;
      }
    }
  }
  
  
#ifdef DEBUG_MODBUS
  Serial.printf("SendCount : [%d]\r\n", sendCount);
#endif

  byte sendCoils[sendCount + 5] = {0,};
  
  sendCoils[0] = _slave_id;
  sendCoils[1] = recv[1];
  sendCoils[2] = sendCount;
  for (uint16_t i = 0; i < sendCount; i++)
  {
    sendCoils[i + 3] = readingCoilsByte[i];
  }
  
                
  uint8_t* crcReadCoil = caculateModbusCRC(sendCoils, sizeof(sendCoils));

  sendCoils[sizeof(sendCoils) - 2] = crcReadCoil[0];
  sendCoils[sizeof(sendCoils) - 1] = crcReadCoil[1];
  
  
  Serial.println("\r\n===============ReadCoil RX===============");
  for (uint8_t i = 0; i < sizeof(sendCoils); i++)
  {
    Serial.printf("0x%02X ", sendCoils[i]);
  }
  Serial.println("\r\n===========================================");
  

  RS485->write(sendCoils, sizeof(sendCoils));
  RS485->flush(); 
}

void MyModbus::readDiscreteInputs(uint16_t startAddress, uint16_t count)
{

    readCoils(startAddress, count);
}


void MyModbus::writeMultipleCoils(uint16_t startAddress, uint16_t count, uint16_t length)
{
#ifdef DEBUG_MODBUS
  Serial.printf("Write Multiple COils [S %d |C %d |NB %d] \r\n", startAddress, count, length);
#endif
  uint16_t multipleCoils[length] = {0, };
  uint32_t totalCoils = 0;


  Serial.printf("Multiple Coils => ");
  for (uint8_t i = 0; i < length; i++)
  {
    multipleCoils[i] = recv[DEFAULT_LENGTH - 1 + i];
    totalCoils += multipleCoils[i] << (i * DEFAULT_LENGTH);
    Serial.printf("0x%X | ", multipleCoils[i]);
  }
  Serial.println();

#ifdef DEBUG_MODBUS
  Serial.printf("Total : 0x%X\r\n", totalCoils);
#endif
  
  for (int i = 0; i < count; i++)
  {
    uint8_t _bit = (((totalCoils) >> (i)) & 0x01);
    my_Coils[startAddress + i] = _bit;
    
    Serial.printf("[%d] %d, ", startAddress + i, _bit);
  }
  Serial.println();

  byte returnArray[DEFAULT_LENGTH] = {0,};
  for (uint8_t i = 0; i < DEFAULT_LENGTH - 2; i++)
  {
    returnArray[i] = recv[i];
  }

  uint8_t* crcMultipleCoil = caculateModbusCRC(returnArray, sizeof(returnArray));

  returnArray[DEFAULT_LENGTH - 2] = crcMultipleCoil[0];
  returnArray[DEFAULT_LENGTH - 1] = crcMultipleCoil[1];
  
  RS485->write(returnArray, DEFAULT_LENGTH);
  RS485->flush();  
}

void MyModbus::readHoldingRegisters(uint16_t startAddress, uint16_t count)
{
                
  uint8_t holdingBytes = count * 2;
  byte registersByte[holdingBytes + 5] = {0,};

  registersByte[0] = _slave_id;
  registersByte[1] = recv[1];
  registersByte[2] = holdingBytes;

  Serial.printf("READING Bytes : [%d]\r\n", holdingBytes);



  for (uint16_t i = 0; i < count; i++)
  {
    registersByte[i*2 + 3] = holdingRegisters[startAddress + i] >> 8;
    registersByte[i*2 + 4] = holdingRegisters[startAddress + i] & 0xFF;
    Serial.printf("REGISTERS : 0x%02X | 0x%02X\r\n", registersByte[i*2 + 3], registersByte[i*2 + 4]);
  }
  
  

  uint8_t* crcReadCoil = caculateModbusCRC(registersByte, sizeof(registersByte));

  registersByte[sizeof(registersByte) - 2] = crcReadCoil[0];
  registersByte[sizeof(registersByte) - 1] = crcReadCoil[1];

  Serial.println("\r\n===============ReadCoil RX===============");
  for (uint8_t i = 0; i < sizeof(registersByte); i++)
  {
    Serial.printf("0x%02X ", registersByte[i]);
  }
  Serial.println("\r\n===========================================");

  RS485->write(registersByte, sizeof(registersByte));
  RS485->flush(); 
}

void MyModbus::readInputRegisters(uint16_t startAddress, uint16_t count)
{
    readHoldingRegisters(startAddress, count);
}






void MyModbus::writeSingleCoil(uint16_t startAddress)
{
  if(recv[4] != 0)
  {
    my_Coils[startAddress] = 0x01;  
  }
  else
  {
    my_Coils[startAddress] = 0x00;
  }
  
  Serial.printf("WRItE : 0x%X\r\n", my_Coils[startAddress]);
  RS485->write(recv, DEFAULT_LENGTH);
  RS485->flush(); 
}



void MyModbus::writeSingleRegister(uint16_t startAddress, uint16_t register_value)
{
    holdingRegisters[startAddress] = register_value;

    Serial.printf("Changed Registers[%d] : %d\r\n", startAddress, holdingRegisters[startAddress]);

    RS485->write(recv, DEFAULT_LENGTH);
    RS485->flush(); 

}


void MyModbus::writeMultipleRegisters(uint16_t startAddress,  uint8_t data_count)
{
  Serial.printf("Write MULTIPLE Start -> 0x%02X | Byte : [%d] \r\n", startAddress, data_count);

  uint16_t numberOfData = data_count / 2 ;
//   uint16_t receivedData[numberOfData] = {0,};

  Serial.print("Holding => ");
  for (uint16_t i = 0; i < numberOfData; i++)
  {
    holdingRegisters[startAddress + i] = (recv[MULTIPLE_DATA_START + 2 * i] << 8) + recv[MULTIPLE_DATA_START + 2 * i + 1];
    Serial.printf("%d | ", holdingRegisters[startAddress + i]);
  }
  Serial.printf("\r\n========================================\r\n");



  byte returnPacket[DEFAULT_LENGTH] = {0,};
  

  for (uint16_t i = 0; i < DEFAULT_LENGTH - 2; i++)
  {
    returnPacket[i] = recv[i];
  }

  uint8_t* crcreturnMultipleRegisters = caculateModbusCRC(returnPacket, sizeof(returnPacket));

  returnPacket[DEFAULT_LENGTH - 2] = crcreturnMultipleRegisters[0];
  returnPacket[DEFAULT_LENGTH - 1] = crcreturnMultipleRegisters[1];
    
  
  RS485->write(returnPacket, sizeof(returnPacket));
  RS485->flush();

}