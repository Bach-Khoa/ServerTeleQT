/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../ui/MainWindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN3GDT10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto GDT::MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN3GDT10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GDT::MainWindow",
        "onStartServer",
        "",
        "onStopServer",
        "onResetServer",
        "onStartLog",
        "onStopLog",
        "onSetupServer",
        "onSetup5A42",
        "onReset",
        "onShow5A42Detail",
        "onTelemetryReceived",
        "GDT::DataTelemetry",
        "d",
        "on5A42Received",
        "GDT::Data5A42",
        "on5U44Received",
        "GDT::Data5U44",
        "on5I41Received",
        "GDT::Data5I41Block",
        "on5E15Received",
        "GDT::Data5E15",
        "onClientConnected",
        "addr",
        "onClientDisconnected",
        "onUiTimer"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onStartServer'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onStopServer'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onResetServer'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onStartLog'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onStopLog'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSetupServer'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSetup5A42'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onReset'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onShow5A42Detail'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTelemetryReceived'
        QtMocHelpers::SlotData<void(GDT::DataTelemetry)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Slot 'on5A42Received'
        QtMocHelpers::SlotData<void(GDT::Data5A42)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 15, 13 },
        }}),
        // Slot 'on5U44Received'
        QtMocHelpers::SlotData<void(GDT::Data5U44)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 17, 13 },
        }}),
        // Slot 'on5I41Received'
        QtMocHelpers::SlotData<void(GDT::Data5I41Block)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 19, 13 },
        }}),
        // Slot 'on5E15Received'
        QtMocHelpers::SlotData<void(GDT::Data5E15)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 21, 13 },
        }}),
        // Slot 'onClientConnected'
        QtMocHelpers::SlotData<void(QString)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 23 },
        }}),
        // Slot 'onClientDisconnected'
        QtMocHelpers::SlotData<void(QString)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 23 },
        }}),
        // Slot 'onUiTimer'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN3GDT10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GDT::MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3GDT10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3GDT10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3GDT10MainWindowE_t>.metaTypes,
    nullptr
} };

void GDT::MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onStartServer(); break;
        case 1: _t->onStopServer(); break;
        case 2: _t->onResetServer(); break;
        case 3: _t->onStartLog(); break;
        case 4: _t->onStopLog(); break;
        case 5: _t->onSetupServer(); break;
        case 6: _t->onSetup5A42(); break;
        case 7: _t->onReset(); break;
        case 8: _t->onShow5A42Detail(); break;
        case 9: _t->onTelemetryReceived((*reinterpret_cast<std::add_pointer_t<GDT::DataTelemetry>>(_a[1]))); break;
        case 10: _t->on5A42Received((*reinterpret_cast<std::add_pointer_t<GDT::Data5A42>>(_a[1]))); break;
        case 11: _t->on5U44Received((*reinterpret_cast<std::add_pointer_t<GDT::Data5U44>>(_a[1]))); break;
        case 12: _t->on5I41Received((*reinterpret_cast<std::add_pointer_t<GDT::Data5I41Block>>(_a[1]))); break;
        case 13: _t->on5E15Received((*reinterpret_cast<std::add_pointer_t<GDT::Data5E15>>(_a[1]))); break;
        case 14: _t->onClientConnected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->onClientDisconnected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->onUiTimer(); break;
        default: ;
        }
    }
}

const QMetaObject *GDT::MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GDT::MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3GDT10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int GDT::MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 17;
    }
    return _id;
}
QT_WARNING_POP
