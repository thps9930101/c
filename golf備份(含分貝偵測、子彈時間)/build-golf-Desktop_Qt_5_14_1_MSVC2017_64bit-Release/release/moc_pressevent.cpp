/****************************************************************************
** Meta object code from reading C++ file 'pressevent.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../golf/tool/pressevent.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pressevent.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PressEvent_t {
    QByteArrayData data[10];
    char stringdata0[116];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PressEvent_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PressEvent_t qt_meta_stringdata_PressEvent = {
    {
QT_MOC_LITERAL(0, 0, 10), // "PressEvent"
QT_MOC_LITERAL(1, 11, 14), // "clickedOutside"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 10), // "keyPressed"
QT_MOC_LITERAL(4, 38, 3), // "key"
QT_MOC_LITERAL(5, 42, 14), // "keyTextPressed"
QT_MOC_LITERAL(6, 57, 4), // "text"
QT_MOC_LITERAL(7, 62, 21), // "keyCombinationPressed"
QT_MOC_LITERAL(8, 84, 21), // "Qt::KeyboardModifiers"
QT_MOC_LITERAL(9, 106, 9) // "modifiers"

    },
    "PressEvent\0clickedOutside\0\0keyPressed\0"
    "key\0keyTextPressed\0text\0keyCombinationPressed\0"
    "Qt::KeyboardModifiers\0modifiers"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PressEvent[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,
       3,    1,   35,    2, 0x06 /* Public */,
       5,    1,   38,    2, 0x06 /* Public */,
       7,    2,   41,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, 0x80000000 | 8, QMetaType::Int,    9,    4,

       0        // eod
};

void PressEvent::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PressEvent *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->clickedOutside(); break;
        case 1: _t->keyPressed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->keyTextPressed((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->keyCombinationPressed((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PressEvent::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PressEvent::clickedOutside)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PressEvent::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PressEvent::keyPressed)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PressEvent::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PressEvent::keyTextPressed)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PressEvent::*)(Qt::KeyboardModifiers , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PressEvent::keyCombinationPressed)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PressEvent::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_PressEvent.data,
    qt_meta_data_PressEvent,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PressEvent::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PressEvent::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PressEvent.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PressEvent::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void PressEvent::clickedOutside()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PressEvent::keyPressed(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PressEvent::keyTextPressed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PressEvent::keyCombinationPressed(Qt::KeyboardModifiers _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
