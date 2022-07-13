#include "bno055.h"

BNO055::BNO055(TwoWire *pWire, uint8_t addr)
{
    lastOperateStatus = eStatusOK;
    _currentPage = 0xff;
    _pWire = pWire;
    _addr = addr;
}

BNO055::eStatus_t BNO055::begin()
{
    // get chip id
    uint8_t temp = getReg(BNO055_CHIP_ID_ADDR, 0);
    if ((lastOperateStatus == eStatusOK) && (temp == 0xA0))
    {
        // switch to config mode (just in case since this is the default)
        setReg(BNO055_OPR_MODE_ADDR, 0, OPERATION_MODE_CONFIG);
        // reset()
        setReg(BNO055_SYS_TRIGGER_ADDR, 0, 0x20);
        delay(100);
        temp = getReg(BNO055_CHIP_ID_ADDR, 0);
        int timeout = 0;
        while ((temp != 0xA0) || (lastOperateStatus != eStatusOK))
        {
            delay(100);
            temp = getReg(BNO055_CHIP_ID_ADDR, 0);
            timeout++;
            if (timeout>=10)
                return eStatusErrDeviceReadyTimeOut;
        };
        delay(10);
        // remove the reset trigger
        setReg(BNO055_SYS_TRIGGER_ADDR, 0, 0x00);
        delay(100);
        setToPage(0);
        // set to normal power mode
        setReg(BNO055_PWR_MODE_ADDR, 0, POWER_MODE_NORMAL);
        delay(20);
        // check the power-on self test reasult
        temp = getReg(BNO055_SELFTEST_RESULT_ADDR, 0);
        if (temp != 0x0f)
            lastOperateStatus = eStatusErrDeviceStatus;
    }
    else
    {
        lastOperateStatus = eStatusErrDeviceNotDetect;
    }
  return lastOperateStatus;
}

BNO055::sAxisAnalog_t BNO055::getMag()
{
    sAxisData_t raw;
    sAxisAnalog_t mag;
    setToPage(0);
    readReg(BNO055_MAG_DATA_X_LSB_ADDR, (uint8_t*) &raw, sizeof(raw));
    // scale magnetometer data: 1 uT = 16 lsb
    mag.x = raw.x * 0.0625;
    mag.y = raw.y * 0.0625;
    mag.z = raw.z * 0.0625;
    return mag;
}

BNO055::sAxisAnalog_t BNO055::getGyro()
{
    sAxisData_t raw;
    sAxisAnalog_t gyro;
    setToPage(0);
    readReg(BNO055_GYRO_DATA_X_LSB_ADDR, (uint8_t*) &raw, sizeof(raw));
    // scale magnetometer data: 1 deg/s = 16 lsb
    gyro.x = raw.x * 0.0625;
    gyro.y = raw.y * 0.0625;
    gyro.z = raw.z * 0.0625;
    return gyro;
}

BNO055::sAxisAnalog_t BNO055::getGravity()
{
    sAxisData_t raw;
    sAxisAnalog_t grav;
    setToPage(0);
    readReg(BNO055_GRAVITY_DATA_X_LSB_ADDR, (uint8_t*) &raw, sizeof(raw));
    // scale gravity data: 1 m/s² = 100 lsb
    grav.x = raw.x * 0.01;
    grav.y = raw.y * 0.01;
    grav.z = raw.z * 0.01;
    return grav;
}

BNO055::sQuaAnalog_t BNO055::getQuaternion()
{
    sQuaData_t raw;
    sQuaAnalog_t qua;
    setToPage(0);
    readReg(BNO055_QUATERNION_DATA_W_LSB_ADDR, (uint8_t*) &raw, sizeof(raw));
    // scale : 1.0 = 2^14 lsb
    qua.w = raw.w * 0.000061035;
    qua.x = raw.x * 0.000061035;
    qua.y = raw.y * 0.000061035;
    qua.z = raw.z * 0.000061035;
    return qua;
}

void BNO055::setToPage(uint8_t pageId)
{
    if(_currentPage != pageId) {
        writeReg(BNO055_PAGE_ID_ADDR, &pageId, sizeof(pageId));
        if(lastOperateStatus == eStatusOK) {
          _currentPage = pageId;
        }
    }
}

uint8_t BNO055::getReg(uint8_t reg, uint8_t pageId)
{
    uint8_t   temp;
    setToPage(pageId);
    readReg(reg, &temp, sizeof(temp));
    return temp;
}

void BNO055::setReg(uint8_t reg, uint8_t pageId, uint8_t val)
{
    setToPage(pageId);
    writeReg(reg, &val, 1);
}

void BNO055::readReg(uint8_t reg, uint8_t *pBuf, uint8_t len)
{   
    lastOperateStatus = eStatusErrDeviceNotDetect;
    _pWire->begin();
    _pWire->beginTransmission(_addr);
    _pWire->write(reg);
    if(_pWire->endTransmission() != 0)
        return;
    _pWire->requestFrom(_addr, len);
    for(uint8_t i = 0; i < len; i ++)
        pBuf[i] = _pWire->read();
    lastOperateStatus = eStatusOK;
}

void BNO055::writeReg(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
    lastOperateStatus = eStatusErrDeviceNotDetect;
    _pWire->begin();
    _pWire->beginTransmission(_addr);
    _pWire->write(reg);
    for(uint8_t i = 0; i < len; i ++)
        _pWire->write(pBuf[i]);
    if(_pWire->endTransmission() != 0)
        return;
    lastOperateStatus = eStatusOK;
}

// ************* methods for non-blocking reads **********

void BNO055::NonBlockingRead_init(uint8_t reg)
{
    lastOperateStatus = eStatusErrDeviceNotDetect;
    _pWire->begin();
    _pWire->beginTransmission(_addr);
    _pWire->write(reg);
    _pWire->runTransmission(true);
}

bool BNO055::NonBlockingRead_finished()
{
    return _pWire->master_finished();
}

void BNO055::NonBlockingRead_request(uint8_t len)
{
    _pWire->runRequest(_addr, len);
}

uint8_t BNO055::NonBlockingRead_available()
{
    return _pWire->master_available();
}

void BNO055::NonBlockingRead_getData(uint8_t *pBuf, uint8_t len)
{
    for(uint8_t i = 0; i < len; i ++)
        pBuf[i] = _pWire->read();
}


