#include "CandleApiDriver.h"
#include "CandleApiInterface.h"

#include <QDebug>

CandleApiInterface::CandleApiInterface(CandleApiDriver *driver, candle_handle handle)
  : CanInterface(driver),
    _hostOffsetStart(0),
    _deviceTicksStart(0),
    _handle(handle),
    _backend(driver->backend()),
    _numRx(0),
    _numTx(0),
    _numTxErr(0)
{
    _channel = 0;
    _isOpen = false;
    _settings.setBitrate(500000);
    _settings.setSamplePoint(875);



    // Timings for 170MHz processors (CANable 2.0)
    // Tseg1: 2..256 Tseg2: 2..128 sjw: 1..128 brp: 1..512
    // Note: as expressed below, Tseg1 does not include 1 count for prop phase
    _timings
        << CandleApiTiming(170000000,   10000, 2000000, 875, 68, 217, 31)
        << CandleApiTiming(170000000,   20000, 4000000, 875, 34, 217, 31)
        << CandleApiTiming(170000000,   50000, 5000000, 875, 17, 173, 25)
        << CandleApiTiming(170000000,   83333, 8000000, 875,  8, 221, 32)
        << CandleApiTiming(170000000,  100000, 0, 875, 10, 147, 21)
        << CandleApiTiming(170000000,  125000, 0, 875, 8,  147, 21)
        << CandleApiTiming(170000000,  250000, 0, 875, 4,  147, 21)
        << CandleApiTiming(170000000,  500000, 0, 875, 2,  147, 21)
        << CandleApiTiming(170000000, 1000000, 0, 875, 1,  147, 21);


    // Timings for 48MHz processors (CANable 0.X)
    _timings
        // sample point: 50.0%
        << CandleApiTiming(48000000,   10000, 2000000, 500, 300, 6, 8)
        << CandleApiTiming(48000000,   20000, 4000000, 500, 150, 6, 8)
        << CandleApiTiming(48000000,   50000, 5000000, 500,  60, 6, 8)
        << CandleApiTiming(48000000,   83333, 8000000, 500,  36, 6, 8)
        << CandleApiTiming(48000000,  100000, 0, 500,  30, 6, 8)
        << CandleApiTiming(48000000,  125000, 0, 500,  24, 6, 8)
        << CandleApiTiming(48000000,  250000, 0, 500,  12, 6, 8)
        << CandleApiTiming(48000000,  500000, 0, 500,   6, 6, 8)
        << CandleApiTiming(48000000,  800000, 0, 500,   3, 8, 9)
        << CandleApiTiming(48000000, 1000000, 0, 500,   3, 6, 8)

        // sample point: 62.5%
        << CandleApiTiming(48000000,   10000, 2000000, 625, 300, 8, 6)
        << CandleApiTiming(48000000,   20000, 4000000, 625, 150, 8, 6)
        << CandleApiTiming(48000000,   50000, 5000000, 625,  60, 8, 6)
        << CandleApiTiming(48000000,   83333, 8000000, 625,  36, 8, 6)
        << CandleApiTiming(48000000,  100000, 0, 625,  30, 8, 6)
        << CandleApiTiming(48000000,  125000, 0, 625,  24, 8, 6)
        << CandleApiTiming(48000000,  250000, 0, 625,  12, 8, 6)
        << CandleApiTiming(48000000,  500000, 0, 625,   6, 8, 6)
        << CandleApiTiming(48000000,  800000, 0, 600,   4, 7, 6)
        << CandleApiTiming(48000000, 1000000, 0, 625,   3, 8, 6)

        // sample point: 75.0%
        << CandleApiTiming(48000000,   10000, 2000000, 750, 300, 10, 4)
        << CandleApiTiming(48000000,   20000, 4000000, 750, 150, 10, 4)
        << CandleApiTiming(48000000,   50000, 5000000, 750,  60, 10, 4)
        << CandleApiTiming(48000000,   83333, 8000000, 750,  36, 10, 4)
        << CandleApiTiming(48000000,  100000, 0, 750,  30, 10, 4)
        << CandleApiTiming(48000000,  125000, 0, 750,  24, 10, 4)
        << CandleApiTiming(48000000,  250000, 0, 750,  12, 10, 4)
        << CandleApiTiming(48000000,  500000, 0, 750,   6, 10, 4)
        << CandleApiTiming(48000000,  800000, 0, 750,   3, 13, 5)
        << CandleApiTiming(48000000, 1000000, 0, 750,   3, 10, 4)

        // sample point: 87.5%
        << CandleApiTiming(48000000,   10000, 2000000, 875, 300, 12, 2)
        << CandleApiTiming(48000000,   20000, 4000000, 875, 150, 12, 2)
        << CandleApiTiming(48000000,   50000, 5000000, 875,  60, 12, 2)
        << CandleApiTiming(48000000,   83333, 8000000, 875,  36, 12, 2)
        << CandleApiTiming(48000000,  100000, 0, 875,  30, 12, 2)
        << CandleApiTiming(48000000,  125000, 0, 875,  24, 12, 2)
        << CandleApiTiming(48000000,  250000, 0, 875,  12, 12, 2)
        << CandleApiTiming(48000000,  500000, 0, 875,   6, 12, 2)
        << CandleApiTiming(48000000,  800000, 0, 867,   4, 11, 2)
        << CandleApiTiming(48000000, 1000000, 0, 875,   3, 12, 2);


    _timings
        // sample point: 50.0%
        << CandleApiTiming(16000000,   10000, 2000000, 520, 64, 11, 12)
        << CandleApiTiming(16000000,   20000, 4000000, 500, 50,  6,  8)
        << CandleApiTiming(16000000,   50000, 5000000, 500, 20,  6,  8)
        << CandleApiTiming(16000000,   83333, 8000000, 500, 12,  6,  8)
        << CandleApiTiming(16000000,  100000, 0, 500, 10,  6,  8)
        << CandleApiTiming(16000000,  125000, 0, 500,  8,  6,  8)
        << CandleApiTiming(16000000,  250000, 0, 500,  4,  6,  8)
        << CandleApiTiming(16000000,  500000, 0, 500,  2,  6,  8)
        << CandleApiTiming(16000000,  800000, 0, 500,  1,  8, 10)
        << CandleApiTiming(16000000, 1000000, 0, 500,  1,  6,  8)

        // sample point: 62.5%
        << CandleApiTiming(16000000,   10000, 2000000, 625, 64, 14,  9)
        << CandleApiTiming(16000000,   20000, 4000000, 625, 50,  8,  6)
        << CandleApiTiming(16000000,   50000, 5000000, 625, 20,  8,  6)
        << CandleApiTiming(16000000,   83333, 8000000, 625, 12,  8,  6)
        << CandleApiTiming(16000000,  100000, 0, 625, 10,  8,  6)
        << CandleApiTiming(16000000,  125000, 0, 625,  8,  8,  6)
        << CandleApiTiming(16000000,  250000, 0, 625,  4,  8,  6)
        << CandleApiTiming(16000000,  500000, 0, 625,  2,  8,  6)
        << CandleApiTiming(16000000,  800000, 0, 625,  1, 11,  7)
        << CandleApiTiming(16000000, 1000000, 0, 625,  1,  8,  6)

        // sample point: 75.0%
        << CandleApiTiming(16000000,   20000, 2000000, 750, 50, 10,  4)
        << CandleApiTiming(16000000,   50000, 4000000, 750, 20, 10,  4)
        << CandleApiTiming(16000000,   83333, 5000000, 750, 12, 10,  4)
        << CandleApiTiming(16000000,  100000, 8000000, 750, 10, 10,  4)
        << CandleApiTiming(16000000,  125000, 0, 750,  8, 10,  4)
        << CandleApiTiming(16000000,  250000, 0, 750,  4, 10,  4)
        << CandleApiTiming(16000000,  500000, 0, 750,  2, 10,  4)
        << CandleApiTiming(16000000,  800000, 0, 750,  1, 13,  5)
        << CandleApiTiming(16000000, 1000000, 0, 750,  1, 10,  4)

        // sample point: 87.5%
        << CandleApiTiming(16000000,   20000, 2000000, 875, 50, 12,  2)
        << CandleApiTiming(16000000,   50000, 4000000, 875, 20, 12,  2)
        << CandleApiTiming(16000000,   83333, 5000000, 875, 12, 12,  2)
        << CandleApiTiming(16000000,  100000, 8000000, 875, 10, 12,  2)
        << CandleApiTiming(16000000,  125000, 0, 875,  8, 12,  2)
        << CandleApiTiming(16000000,  250000, 0, 875,  4, 12,  2)
        << CandleApiTiming(16000000,  500000, 0, 875,  2, 12,  2)
        << CandleApiTiming(16000000,  800000, 0, 900,  2,  7,  1)
        << CandleApiTiming(16000000, 1000000, 0, 875,  1, 12,  2);
}

CandleApiInterface::~CandleApiInterface()
{

}

QString CandleApiInterface::getName() const
{
    return "HPMicro_candle" + QString::number(getId() & 0xFF);
}

QString CandleApiInterface::getDetailsStr() const
{
    return QString::fromStdWString(getPath());
}

QString CandleApiInterface::getDescription() const {
    return  _description ;
}

void CandleApiInterface::setDescription(QString description) {
    _description = description;
}

void CandleApiInterface::applyConfig(const MeasurementInterface &mi)
{
    _settings = mi;
}

unsigned CandleApiInterface::getBitrate()
{
    return _settings.bitrate();
}

uint32_t CandleApiInterface::getCapabilities()
{
    candle_capability_t caps;

    if (candle_channel_get_capabilities(_handle, 0, &caps)) {

        uint32_t retval = 0;

        if (caps.feature & CANDLE_MODE_LISTEN_ONLY) {
            retval |= CanInterface::capability_listen_only;
        }

        if (caps.feature & CANDLE_MODE_ONE_SHOT) {
            retval |= CanInterface::capability_one_shot;
        }

        if (caps.feature & CANDLE_MODE_TRIPLE_SAMPLE) {
            retval |= CanInterface::capability_triple_sampling;
        }

        if (caps.feature & CANDLE_MODE_FD) {
            retval |= CanInterface::capability_canfd;
        }
        return retval;

    } else {
        return 0;
    }
}

QList<CanTiming> CandleApiInterface::getAvailableBitrates()
{
    QList<CanTiming> retval;

    candle_capability_t caps;
    if (candle_channel_get_capabilities(_handle, 0, &caps)) {
        int i = 0;
        foreach (const CandleApiTiming t, _timings) {
            if (t.getBaseClk() == caps.fclk_can) {
                retval << CanTiming(i++, t.getBitrate(), 0, t.getSamplePoint());
            }
        }
    }

    return retval;
}

bool CandleApiInterface::setBitTiming(uint32_t bitrate, uint32_t samplePoint)
{
    candle_capability_t caps;
    if (!candle_channel_get_capabilities(_handle, 0, &caps)) {
        return false;
    }

    foreach (const CandleApiTiming t, _timings) {
        if ( (t.getBaseClk() == caps.fclk_can)
          && (t.getBitrate()==bitrate)
          && (t.getSamplePoint()==samplePoint) )
        {
            candle_bittiming_t timing = t.getTiming();
            return candle_channel_set_timing(_handle, _channel, &timing);
        }
    }

    // no valid timing found
    return false;
}

void CandleApiInterface::open()
{
    if (!candle_dev_open(_handle)) {
        // TODO what?
        _isOpen = false;
        return;
    }
    qDebug() << "open candle......" << _channel;
    if (!setBitTiming(_settings.bitrate(), _settings.samplePoint())) {
        // TODO what?
        _isOpen = false;
        return;
    }

    uint32_t flags = 0;
    if (_settings.isListenOnlyMode()) {
        flags |= CANDLE_MODE_LISTEN_ONLY;
    }
    if (_settings.isOneShotMode()) {
        flags |= CANDLE_MODE_ONE_SHOT;
    }
    if (_settings.isTripleSampling()) {
        flags |= CANDLE_MODE_TRIPLE_SAMPLE;
    }

    _numRx = 0;
    _numTx = 0;
    _numTxErr = 0;

    uint32_t t_dev;
    if (candle_dev_get_timestamp_us(_handle, &t_dev)) {
        _hostOffsetStart =
                _backend.getUsecsAtMeasurementStart() +
                _backend.getUsecsSinceMeasurementStart();
        _deviceTicksStart = t_dev;
    }

    candle_channel_set_interfacenumber_endpoints(_handle, _channel);
    candle_channel_start(_handle, _channel, flags);
    _isOpen = true;
}

bool CandleApiInterface::isOpen()
{
//    qDebug() << "CandleApiInterface: " << _isOpen << _channel;
    return _isOpen;
}

void CandleApiInterface::close()
{
    candle_channel_stop(_handle, _channel);
    candle_dev_close(_handle);
    _isOpen = false;
}

void CandleApiInterface::sendMessage(const CanMessage &msg)
{
    candle_frame_t frame;

    frame.can_id = msg.getId();
    if (msg.isExtended()) {
        frame.can_id |= CANDLE_ID_EXTENDED;
    }
    if (msg.isRTR()) {
        frame.can_id |= CANDLE_ID_RTR;
    }

    frame.can_dlc = msg.getLength();
    for (int i=0; i<8; i++) {
        frame.data[i] = msg.getByte(i);
    }

    if (candle_frame_send(_handle, _channel, &frame)) {
        _numTx++;
    } else {
        _numTxErr++;
    }
}

bool CandleApiInterface::readMessage(QList<CanMessage> &msglist, unsigned int timeout_ms)
{
    candle_frame_t frame;
    CanMessage msg;

    if (candle_frame_read(_handle, &frame, timeout_ms)) {
        if (candle_frame_type(&frame)==CANDLE_FRAMETYPE_RECEIVE) {
            _numRx++;

            msg.setInterfaceId(getId());
            msg.setErrorFrame(false);
            msg.setId(candle_frame_id(&frame));
            msg.setExtended(candle_frame_is_extended_id(&frame));
            msg.setRTR(candle_frame_is_rtr(&frame));

            uint8_t dlc = candle_frame_dlc(&frame);
            uint8_t *data = candle_frame_data(&frame);
            msg.setLength(dlc);
            for (int i=0; i<dlc; i++) {
                msg.setByte(i, data[i]);
            }

            uint32_t dev_ts = candle_frame_timestamp_us(&frame) - _deviceTicksStart;
            uint64_t ts_us = _hostOffsetStart + dev_ts;

            uint64_t us_since_start = _backend.getUsecsSinceMeasurementStart();
            if (us_since_start > 0x180000000) { // device timestamp overflow must have happend at least once
                ts_us += us_since_start & 0xFFFFFFFF00000000;
            }

            msg.setTimestamp(ts_us/1000000, ts_us % 1000000);
            msglist.append(msg);
            return true;
        }

    }

    return false;
}

bool CandleApiInterface::updateStatistics()
{
    return true;
}

uint32_t CandleApiInterface::getState()
{
    return CanInterface::state_ok;
}

int CandleApiInterface::getNumRxFrames()
{
    return _numRx;
}

int CandleApiInterface::getNumRxErrors()
{
    return 0;
}

int CandleApiInterface::getNumTxFrames()
{
    return _numTx;
}

int CandleApiInterface::getNumTxErrors()
{
    return _numTxErr;
}

int CandleApiInterface::getNumRxOverruns()
{
    return 0;
}

int CandleApiInterface::getNumTxDropped()
{
    return 0;
}

bool CandleApiInterface::get_enable_terminal_res()
{
    uint8_t enable;
    QByteArray msg;
    char buf[256];
    int len;
    if (!candle_channel_get_can_resister_enable_state(_handle, _channel, &enable)) {
        return false;
    }
    msg.clear();
    len = sprintf(buf, "hpm_cfg_g_120r_%d_%d", _channel, enable);
    msg.append(buf, len);
    emit _hpm_request_msg(msg);
    return enable;
}

void CandleApiInterface::set_enable_terminal_res(bool enable)
{
    uint8_t _enable = enable;
    if (!candle_channel_set_can_resister_enable_state(_handle, _channel, &_enable)) {
        return ;
    }
}

wstring CandleApiInterface::getPath() const
{
    return wstring(candle_dev_get_path(_handle));
}

void CandleApiInterface::update(candle_handle dev)
{
    candle_dev_free(_handle);
    _handle = dev;
}

void CandleApiInterface::set_channel(uint8_t ch)
{
    _channel = ch;
}

uint8_t CandleApiInterface::get_channel()
{
    return _channel;
}

