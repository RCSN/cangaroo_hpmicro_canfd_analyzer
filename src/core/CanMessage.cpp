/*

  Copyright (c) 2015, 2016 Hubert Denkmair <hubert@denkmair.de>

  This file is part of cangaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/



#include "CanMessage.h"
#include <core/portable_endian.h>
#include <QDebug>
enum {
	id_flag_extended = 0x80000000,
	id_flag_rtr      = 0x40000000,
	id_flag_error    = 0x20000000,
	id_mask_extended = 0x1FFFFFFF,
	id_mask_standard = 0x7FF
};

CanMessage::CanMessage()
{
    _timestamp.tv_sec = 0;
    _timestamp.tv_usec = 0;
    _isBRS = false;
    _isFD = false;
    _dlc = 0;
    _raw_id= 0;
    _interface = 0;
    _isExd = false;
    _isRTR = false;
}

CanMessage::CanMessage(uint32_t can_id)
{
    _timestamp.tv_sec = 0;
    _timestamp.tv_usec = 0;
    _isBRS = 0;
    setId(can_id);
}

CanMessage::CanMessage(const CanMessage &msg)
{
    cloneFrom(msg);
}

void CanMessage::cloneFrom(const CanMessage &msg)
{
    _raw_id = msg._raw_id;
    _dlc = msg._dlc;
    _isFD = msg.isFD();
    _isBRS = msg.isBRS();
    _isExd = msg.isExtended();
    _isRTR = msg.isRTR();
    this->_direction = msg._direction;
    // Copy data
    for(int i=0; i<64; i++)
    {
        _u8[i] = msg._u8[i];
    }

    _interface = msg._interface;
    _timestamp = msg._timestamp;
}


uint32_t CanMessage::getRawId() const {
	return _raw_id;
}

void CanMessage::setRawId(const uint32_t raw_id) {
	_raw_id = raw_id;
}

uint32_t CanMessage::getId() const {
    return _raw_id;
}

void CanMessage::setId(const uint32_t id) {
    _raw_id = id;
}

bool CanMessage::isExtended() const {
    return _isExd;
}

void CanMessage::setExtended(const bool isExtended) {
    _isExd = isExtended;
}

bool CanMessage::isRTR() const {
    return _isRTR;
}

void CanMessage::setRTR(const bool isRTR) {
    _isRTR = isRTR;
}

bool CanMessage::isFD() const {
    return _isFD;
}

void CanMessage::setFD(const bool isFD) {
    _isFD = isFD;
}

bool CanMessage::isBRS() const {
    return _isBRS;
}

void CanMessage::setBRS(const bool isBRS) {
    _isBRS = isBRS;
}

bool CanMessage::isErrorFrame() const {
	return (_raw_id & id_flag_error) != 0;
}

void CanMessage::setErrorFrame(const bool isErrorFrame) {
	if (isErrorFrame) {
		_raw_id |= id_flag_error;
	} else {
		_raw_id &= ~id_flag_error;
    }
}

CanInterfaceId CanMessage::getInterfaceId() const
{
    return _interface;
}

void CanMessage::setInterfaceId(CanInterfaceId interface)
{
    _interface = interface;
}

uint8_t CanMessage::getLength() const {
	return _dlc;
}

void CanMessage::setLength(const uint8_t dlc) {
    // Limit to CANFD max length
    if (dlc<=64) {
		_dlc = dlc;
	} else {
		_dlc = 8;
	}
}

uint8_t CanMessage::getByte(const uint8_t index) const {
	if (index<sizeof(_u8)) {
		return _u8[index];
	} else {
		return 0;
	}
}

void CanMessage::setByte(const uint8_t index, const uint8_t value) {
	if (index<sizeof(_u8)) {
		_u8[index] = value;
    }
}

uint64_t CanMessage::extractRawSignal(uint8_t start_bit, const uint8_t length, const bool isBigEndian) const
{
//    if ((start_bit+length) > (getLength()*8)) {
//        return 0;
//    }

    // FIXME: This only gives access to data bytes 0-8. Need to rework for CANFD.
    uint64_t data = le64toh(_u64[0]);

    data >>= start_bit;

    uint64_t mask =  0xFFFFFFFFFFFFFFFF;
    mask <<= length;
    mask = ~mask;

    data &= mask;

    // If the length is greater than 8, we need to byteswap to preserve endianness
    if (isBigEndian && (length > 8))
    {

        // Swap bytes
        data = __builtin_bswap64(data);

        // Shift out unused bits
        data >>= 64 - length;
    }

    return data;
}

void CanMessage::setDataAt(uint8_t position, uint8_t data)
{
    if(position < 64)
        _u8[position] = data;
    else
        return;
}

void CanMessage::setData(const uint8_t d0) {
	_dlc = 1;
	_u8[0] = d0;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1) {
	_dlc = 2;
	_u8[0] = d0;
	_u8[1] = d1;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1, const uint8_t d2) {
	_dlc = 3;
	_u8[0] = d0;
	_u8[1] = d1;
	_u8[2] = d2;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1, const uint8_t d2,
		const uint8_t d3) {
	_dlc = 4;
	_u8[0] = d0;
	_u8[1] = d1;
	_u8[2] = d2;
	_u8[3] = d3;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1, const uint8_t d2,
		const uint8_t d3, const uint8_t d4) {
	_dlc = 5;
	_u8[0] = d0;
	_u8[1] = d1;
	_u8[2] = d2;
	_u8[3] = d3;
	_u8[4] = d4;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1, const uint8_t d2,
		const uint8_t d3, const uint8_t d4, const uint8_t d5) {
	_dlc = 6;
	_u8[0] = d0;
	_u8[1] = d1;
	_u8[2] = d2;
	_u8[3] = d3;
	_u8[4] = d4;
	_u8[5] = d5;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1, const uint8_t d2,
		const uint8_t d3, const uint8_t d4, const uint8_t d5,
		const uint8_t d6) {
	_dlc = 7;
	_u8[0] = d0;
	_u8[1] = d1;
	_u8[2] = d2;
	_u8[3] = d3;
	_u8[4] = d4;
	_u8[5] = d5;
	_u8[6] = d6;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1, const uint8_t d2,
		const uint8_t d3, const uint8_t d4, const uint8_t d5, const uint8_t d6,
		const uint8_t d7) {
	_dlc = 8;
	_u8[0] = d0;
	_u8[1] = d1;
	_u8[2] = d2;
	_u8[3] = d3;
	_u8[4] = d4;
	_u8[5] = d5;
	_u8[6] = d6;
    _u8[7] = d7;
}

timeval CanMessage::getTimestamp() const
{
    return _timestamp;
}

void CanMessage::setTimestamp(const timeval timestamp)
{
    _timestamp = timestamp;
}

void CanMessage::setTimestamp(const uint64_t seconds, const uint32_t micro_seconds)
{
    _timestamp.tv_sec = seconds;
    _timestamp.tv_usec = micro_seconds;
}

double CanMessage::getFloatTimestamp() const
{
    return (double)_timestamp.tv_sec + ((double)_timestamp.tv_usec/1000000);
}

QDateTime CanMessage::getDateTime() const
{
    return QDateTime::fromMSecsSinceEpoch((qint64)(1000*getFloatTimestamp()));
}

QString CanMessage::getIdString() const
{
    if (isExtended()) {
        return QString().asprintf("0x%08X", getId());
    } else {
        return QString().asprintf("0x%03X", getId());
    }
}

QString CanMessage::getDataHexString() const
{
    if(getLength() == 0)
        return "";

    QString outstr = "";
    for(int i=0; i<getLength(); i++)
    {
        outstr += QString().asprintf("%02X ", getByte(i));
    }

    return outstr;
}

void CanMessage::setDirection(Direction dir)
{
    _direction = dir;
}

CanMessage::Direction CanMessage::direction() const
{
    return _direction;
}


