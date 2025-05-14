#ifndef CANDLEAPIINTERFACE_H
#define CANDLEAPIINTERFACE_H

#include <driver/CanInterface.h>
#include <core/MeasurementInterface.h>
#include "api/candle.h"
#include "CandleApiTiming.h"
#include <windows.h>
#include <QList>

class CandleApiDriver;

using namespace std;

class CandleApiInterface : public CanInterface
{
    Q_OBJECT
public:
    CandleApiInterface(CandleApiDriver *driver, candle_handle handle);
    virtual ~CandleApiInterface();

    virtual QString getName() const;
    virtual QString getDetailsStr() const;
    virtual QString getDescription() const;
    void setDescription(QString description);

    virtual void applyConfig(const MeasurementInterface &mi);

    virtual unsigned getBitrate();

    virtual uint32_t getCapabilities();
    virtual QList<CanTiming> getAvailableBitrates();

    virtual void open();
    virtual bool isOpen();
    virtual void close();

    virtual void sendMessage(const CanMessage &msg);
    virtual bool readMessage(QList<CanMessage> &msglist, unsigned int timeout_ms);

    virtual bool updateStatistics();
    virtual uint32_t getState();
    virtual int getNumRxFrames();
    virtual int getNumRxErrors();
    virtual int getNumTxFrames();
    virtual int getNumTxErrors();
    virtual int getNumRxOverruns();
    virtual int getNumTxDropped();  
    virtual bool get_enable_terminal_res(void);
    virtual void set_enable_terminal_res(bool enable);

    wstring getPath() const;

    void update(candle_handle dev);

    void set_channel(uint8_t ch);
    uint8_t get_channel(void);

private:

    uint64_t _hostOffsetStart;
    uint32_t _deviceTicksStart;
    bool _isOpen;

    candle_handle _handle;
    MeasurementInterface _settings;
    Backend &_backend;

    uint64_t _numRx;
    uint64_t _numTx;
    uint64_t _numTxErr;

    QList<CandleApiTiming> _timings;
    QList<CandleApiTiming> _fd_data_timings;

    QString _description;

    bool setBitTiming(uint32_t bitrate, uint32_t samplePoint);
    bool setDataBitTiming(uint32_t fdBitrate, uint32_t samplePoint);

    uint8_t _channel;
};

#endif // CANDLEAPIINTERFACE_H
