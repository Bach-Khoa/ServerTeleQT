/****************************************************************************
** Meta object code from reading C++ file 'TelemetryServer.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../network/TelemetryServer.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TelemetryServer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN3GDT12ClientWorkerE_t {};
} // unnamed namespace

template <> constexpr inline auto GDT::ClientWorker::qt_create_metaobjectdata<qt_meta_tag_ZN3GDT12ClientWorkerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GDT::ClientWorker",
        "telemetryReceived",
        "",
        "GDT::DataTelemetry",
        "data",
        "data5A42Received",
        "GDT::Data5A42",
        "data5U44Received",
        "GDT::Data5U44",
        "clientDisconnected",
        "address",
        "updateCalib",
        "GDT::Calib5A42",
        "calib",
        "onReadyRead",
        "onDisconnected"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'telemetryReceived'
        QtMocHelpers::SignalData<void(GDT::DataTelemetry)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'data5A42Received'
        QtMocHelpers::SignalData<void(GDT::Data5A42)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 4 },
        }}),
        // Signal 'data5U44Received'
        QtMocHelpers::SignalData<void(GDT::Data5U44)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 4 },
        }}),
        // Signal 'clientDisconnected'
        QtMocHelpers::SignalData<void(QString)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Slot 'updateCalib'
        QtMocHelpers::SlotData<void(GDT::Calib5A42)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Slot 'onReadyRead'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onDisconnected'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ClientWorker, qt_meta_tag_ZN3GDT12ClientWorkerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GDT::ClientWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3GDT12ClientWorkerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3GDT12ClientWorkerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3GDT12ClientWorkerE_t>.metaTypes,
    nullptr
} };

void GDT::ClientWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ClientWorker *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->telemetryReceived((*reinterpret_cast<std::add_pointer_t<GDT::DataTelemetry>>(_a[1]))); break;
        case 1: _t->data5A42Received((*reinterpret_cast<std::add_pointer_t<GDT::Data5A42>>(_a[1]))); break;
        case 2: _t->data5U44Received((*reinterpret_cast<std::add_pointer_t<GDT::Data5U44>>(_a[1]))); break;
        case 3: _t->clientDisconnected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->updateCalib((*reinterpret_cast<std::add_pointer_t<GDT::Calib5A42>>(_a[1]))); break;
        case 5: _t->onReadyRead(); break;
        case 6: _t->onDisconnected(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ClientWorker::*)(GDT::DataTelemetry )>(_a, &ClientWorker::telemetryReceived, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientWorker::*)(GDT::Data5A42 )>(_a, &ClientWorker::data5A42Received, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientWorker::*)(GDT::Data5U44 )>(_a, &ClientWorker::data5U44Received, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientWorker::*)(QString )>(_a, &ClientWorker::clientDisconnected, 3))
            return;
    }
}

const QMetaObject *GDT::ClientWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GDT::ClientWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3GDT12ClientWorkerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GDT::ClientWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void GDT::ClientWorker::telemetryReceived(GDT::DataTelemetry _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void GDT::ClientWorker::data5A42Received(GDT::Data5A42 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void GDT::ClientWorker::data5U44Received(GDT::Data5U44 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void GDT::ClientWorker::clientDisconnected(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
namespace {
struct qt_meta_tag_ZN3GDT15TelemetryServerE_t {};
} // unnamed namespace

template <> constexpr inline auto GDT::TelemetryServer::qt_create_metaobjectdata<qt_meta_tag_ZN3GDT15TelemetryServerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GDT::TelemetryServer",
        "telemetryReceived",
        "",
        "GDT::DataTelemetry",
        "data",
        "data5A42Received",
        "GDT::Data5A42",
        "data5U44Received",
        "GDT::Data5U44",
        "clientConnected",
        "address",
        "clientDisconnected",
        "serverError",
        "message",
        "calibUpdated",
        "GDT::Calib5A42",
        "calib",
        "onNewConnection"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'telemetryReceived'
        QtMocHelpers::SignalData<void(GDT::DataTelemetry)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'data5A42Received'
        QtMocHelpers::SignalData<void(GDT::Data5A42)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 4 },
        }}),
        // Signal 'data5U44Received'
        QtMocHelpers::SignalData<void(GDT::Data5U44)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 4 },
        }}),
        // Signal 'clientConnected'
        QtMocHelpers::SignalData<void(QString)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Signal 'clientDisconnected'
        QtMocHelpers::SignalData<void(QString)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Signal 'serverError'
        QtMocHelpers::SignalData<void(QString)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 },
        }}),
        // Signal 'calibUpdated'
        QtMocHelpers::SignalData<void(GDT::Calib5A42)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 15, 16 },
        }}),
        // Slot 'onNewConnection'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TelemetryServer, qt_meta_tag_ZN3GDT15TelemetryServerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GDT::TelemetryServer::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3GDT15TelemetryServerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3GDT15TelemetryServerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3GDT15TelemetryServerE_t>.metaTypes,
    nullptr
} };

void GDT::TelemetryServer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TelemetryServer *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->telemetryReceived((*reinterpret_cast<std::add_pointer_t<GDT::DataTelemetry>>(_a[1]))); break;
        case 1: _t->data5A42Received((*reinterpret_cast<std::add_pointer_t<GDT::Data5A42>>(_a[1]))); break;
        case 2: _t->data5U44Received((*reinterpret_cast<std::add_pointer_t<GDT::Data5U44>>(_a[1]))); break;
        case 3: _t->clientConnected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->clientDisconnected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->serverError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->calibUpdated((*reinterpret_cast<std::add_pointer_t<GDT::Calib5A42>>(_a[1]))); break;
        case 7: _t->onNewConnection(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TelemetryServer::*)(GDT::DataTelemetry )>(_a, &TelemetryServer::telemetryReceived, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TelemetryServer::*)(GDT::Data5A42 )>(_a, &TelemetryServer::data5A42Received, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TelemetryServer::*)(GDT::Data5U44 )>(_a, &TelemetryServer::data5U44Received, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (TelemetryServer::*)(QString )>(_a, &TelemetryServer::clientConnected, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (TelemetryServer::*)(QString )>(_a, &TelemetryServer::clientDisconnected, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (TelemetryServer::*)(QString )>(_a, &TelemetryServer::serverError, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (TelemetryServer::*)(GDT::Calib5A42 )>(_a, &TelemetryServer::calibUpdated, 6))
            return;
    }
}

const QMetaObject *GDT::TelemetryServer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GDT::TelemetryServer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3GDT15TelemetryServerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GDT::TelemetryServer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void GDT::TelemetryServer::telemetryReceived(GDT::DataTelemetry _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void GDT::TelemetryServer::data5A42Received(GDT::Data5A42 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void GDT::TelemetryServer::data5U44Received(GDT::Data5U44 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void GDT::TelemetryServer::clientConnected(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void GDT::TelemetryServer::clientDisconnected(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void GDT::TelemetryServer::serverError(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void GDT::TelemetryServer::calibUpdated(GDT::Calib5A42 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}
QT_WARNING_POP
