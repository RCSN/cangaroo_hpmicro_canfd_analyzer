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

#include "BaseTraceViewModel.h"

#include <QDateTime>
#include <QColor>

#include <core/Backend.h>
#include <core/CanTrace.h>
#include <core/CanMessage.h>
#include <core/CanDbMessage.h>

BaseTraceViewModel::BaseTraceViewModel(Backend &backend)
{
    _backend = &backend;
}

int BaseTraceViewModel::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return column_count;
}

QVariant BaseTraceViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {

        if (orientation == Qt::Horizontal) {
            switch (section) {
                case column_timestamp:
                    return QString("Timestamp");
                case column_channel:
                    return QString("Port");
                case column_direction:
                    return QString("Rx/Tx");
                case column_canid:
                    return QString("CAN ID");
                case column_sender:
                    return QString("DB_Sender");
                case column_name:
                    return QString("DB_Name");
                case column_dlc:
                    return QString("DLC");
                case column_frame_type:
                    return QString("Frame Type");
                case column_frame_format:
                    return QString("Frame Format");
                case column_can_type:
                    return QString("CAN Type");
                case column_data:
                    return QString("Data");
                case column_comment:
                    return QString("DB_Comment");
            }
        }

    } else if (role == Qt::TextAlignmentRole) {  // 新增对齐处理
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case column_timestamp:
                case column_canid:
                case column_sender:
                case column_channel:
                case column_direction:
                case column_frame_type:
                case column_frame_format:
                case column_can_type:
                case column_comment:
                case column_dlc:
                    return Qt::AlignCenter;
                default:
                    return Qt::AlignLeft;
            }
        }
    }
    return QVariant();
}


QVariant BaseTraceViewModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
        case Qt::DisplayRole:
            return data_DisplayRole(index, role);
        case Qt::TextAlignmentRole:
            return data_TextAlignmentRole(index, role);
        case Qt::TextColorRole:
            return data_TextColorRole(index, role);
        default:
            return QVariant();
    }
}

Backend *BaseTraceViewModel::backend() const
{
    return _backend;
}

CanTrace *BaseTraceViewModel::trace() const
{
    return _backend->getTrace();
}

timestamp_mode_t BaseTraceViewModel::timestampMode() const
{
    return _timestampMode;
}

void BaseTraceViewModel::setTimestampMode(timestamp_mode_t timestampMode)
{
    _timestampMode = timestampMode;
}

QVariant BaseTraceViewModel::formatTimestamp(timestamp_mode_t mode, const CanMessage &currentMsg, const CanMessage &lastMsg) const
{

    if (mode==timestamp_mode_delta) {

        double t_current = currentMsg.getFloatTimestamp();
        double t_last = lastMsg.getFloatTimestamp();
        if (t_last==0) {
            return QVariant();
        } else {
            return QString().asprintf("%.04lf", t_current-t_last);
        }

    } else if (mode==timestamp_mode_absolute) {

        return currentMsg.getDateTime().toString("hh:mm:ss.zzz");

    } else if (mode==timestamp_mode_relative) {

        double t_current = currentMsg.getFloatTimestamp();
        return QString().asprintf("%.04lf", t_current - backend()->getTimestampAtMeasurementStart());

    }

    return QVariant();
}

//Colin formatAggregate
QVariant BaseTraceViewModel::formatAggregate(aggregated_mode_t mode, const CanMessage &currentMsg, const CanMessage &lastMsg) const
{

    QString current_port = backend()->getInterfaceName(currentMsg.getInterfaceId());
    uint32_t current_id = currentMsg.getId();

    if (mode == aggregated_mode_id) { 
//        if(current_id == 0x123)
        return current_id;
    } else if (mode == aggregated_mode_port) {
       // if(current_port == "COM15")
                return current_port;
       // else    return QVariant();
    }

    return QVariant();
}

QVariant BaseTraceViewModel::data_DisplayRole(const QModelIndex &index, int role) const
{
    (void) index;
    (void) role;
    return QVariant();
}

QVariant BaseTraceViewModel::data_DisplayRole_Message(const QModelIndex &index, int role, const CanMessage &currentMsg, const CanMessage &lastMsg) const
{
    (void) role;
    CanDbMessage *dbmsg = backend()->findDbMessage(currentMsg);

    switch (index.column()) {

        case column_timestamp:
            return formatTimestamp(_timestampMode, currentMsg, lastMsg);

        case column_channel:
//            return backend()->getInterfaceName(currentMsg.getInterfaceId()); //Colin 发送口名字
//            return backend()->getPortName(currentMsg.getInterfaceId());//Colin 接收口名字
            return formatAggregate(aggregated_mode_port, currentMsg, lastMsg);

        case column_direction:
            return (currentMsg.direction() == CanMessage::Tx) ? "Tx" : "Rx";

        case column_canid:
            return currentMsg.getIdString();

        case column_name:
            return (dbmsg) ? dbmsg->getName() : "NULL";

        case column_sender:
            return (dbmsg) ? dbmsg->getSender()->name() : "NULL";
        
        case column_can_type:
        if (currentMsg.isFD() == true) {
            if (currentMsg.isBRS() == true) {
                return "CANFD_BRS";
            } else {
                return "CANFD";
            }
        } else {
            return "CAN";
        }

        case column_frame_type:
            return (currentMsg.isExtended()) ? "Extended" : "Standard";

        case column_frame_format:
            return (currentMsg.isRTR()) ? "Remote" : "Data";

        case column_dlc:
            return currentMsg.getLength();

        case column_data:
            return currentMsg.getDataHexString();

        case column_comment:
            return (dbmsg) ? dbmsg->getComment() : "NULL";

        default:
            return QVariant();

    }
}

QVariant BaseTraceViewModel::data_DisplayRole_Signal(const QModelIndex &index, int role, const CanMessage &msg) const
{
    (void) role;
    uint64_t raw_data;
    QString value_name;
    QString unit;

    CanDbMessage *dbmsg = backend()->findDbMessage(msg);
    if (!dbmsg) { return QVariant(); }

    CanDbSignal *dbsignal = dbmsg->getSignal(index.row());
    if (!dbsignal) { return QVariant(); }

    switch (index.column()) {

        case column_name:
            return dbsignal->name();

        case column_data:

            if (dbsignal->isPresentInMessage(msg)) {
                raw_data = dbsignal->extractRawDataFromMessage(msg);
            } else {
                if (!trace()->getMuxedSignalFromCache(dbsignal, &raw_data)) {
                    return QVariant();
                }
            }

            value_name = dbsignal->getValueName(raw_data);
            if (value_name.isEmpty()) {
                unit = dbsignal->getUnit();
                if (unit.isEmpty()) {
                    return dbsignal->convertRawValueToPhysical(raw_data);
                } else {
                    return QString("%1 %2").arg(dbsignal->convertRawValueToPhysical(raw_data)).arg(unit);
                }
            } else {
                return QString("%1 - %2").arg(raw_data).arg(value_name);
            }

        case column_comment:
            return dbsignal->comment().replace('\n', ' ');

        default:
            return QVariant();

    }
}

QVariant BaseTraceViewModel::data_TextAlignmentRole(const QModelIndex &index, int role) const
{
    (void) role;
    switch (index.column()) {
        case column_timestamp: return Qt::AlignHCenter + Qt::AlignVCenter;
        case column_channel: return Qt::AlignHCenter + Qt::AlignVCenter;
        case column_direction: return Qt::AlignHCenter + Qt::AlignVCenter;
        case column_canid: return Qt::AlignHCenter + Qt::AlignVCenter;
        case column_sender: return Qt::AlignHCenter + Qt::AlignVCenter;
        case column_name: return Qt::AlignHCenter + Qt::AlignVCenter;
        case column_dlc: return Qt::AlignHCenter + Qt::AlignVCenter;
        case column_data: return Qt::AlignLeft + Qt::AlignVCenter;
        case column_comment: return Qt::AlignLeft + Qt::AlignVCenter;
        default: return QVariant();
    }
}

QVariant BaseTraceViewModel::data_TextColorRole(const QModelIndex &index, int role) const
{
    (void) index;
    (void) role;
    return QVariant();
}

QVariant BaseTraceViewModel::data_TextColorRole_Signal(const QModelIndex &index, int role, const CanMessage &msg) const
{
    (void) role;

    CanDbMessage *dbmsg = backend()->findDbMessage(msg);
    if (!dbmsg) { return QVariant(); }

    CanDbSignal *dbsignal = dbmsg->getSignal(index.row());
    if (!dbsignal) { return QVariant(); }

    if (dbsignal->isPresentInMessage(msg)) {
        return QVariant(); // default text color
    } else {
        return QVariant::fromValue(QColor(200,200,200));
    }
}
