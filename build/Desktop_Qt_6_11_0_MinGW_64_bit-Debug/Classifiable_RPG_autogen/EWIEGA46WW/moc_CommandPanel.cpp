/****************************************************************************
** Meta object code from reading C++ file 'CommandPanel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../CommandPanel.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CommandPanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
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
struct qt_meta_tag_ZN13CodeInputEditE_t {};
} // unnamed namespace

template <> constexpr inline auto CodeInputEdit::qt_create_metaobjectdata<qt_meta_tag_ZN13CodeInputEditE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "CodeInputEdit",
        "returnPressed",
        "",
        "parameterSelectionChanged",
        "active",
        "QPoint",
        "globalStart",
        "previewParameter",
        "objectName",
        "clearParameterPreview",
        "commitParameter",
        "cancelParameterSelection"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'returnPressed'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'parameterSelectionChanged'
        QtMocHelpers::SignalData<void(bool, const QPoint &)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 4 }, { 0x80000000 | 5, 6 },
        }}),
        // Slot 'previewParameter'
        QtMocHelpers::SlotData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Slot 'clearParameterPreview'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'commitParameter'
        QtMocHelpers::SlotData<void(const QString &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Slot 'cancelParameterSelection'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CodeInputEdit, qt_meta_tag_ZN13CodeInputEditE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject CodeInputEdit::staticMetaObject = { {
    QMetaObject::SuperData::link<QPlainTextEdit::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13CodeInputEditE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13CodeInputEditE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13CodeInputEditE_t>.metaTypes,
    nullptr
} };

void CodeInputEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CodeInputEdit *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->returnPressed(); break;
        case 1: _t->parameterSelectionChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[2]))); break;
        case 2: _t->previewParameter((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->clearParameterPreview(); break;
        case 4: _t->commitParameter((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->cancelParameterSelection(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CodeInputEdit::*)()>(_a, &CodeInputEdit::returnPressed, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CodeInputEdit::*)(bool , const QPoint & )>(_a, &CodeInputEdit::parameterSelectionChanged, 1))
            return;
    }
}

const QMetaObject *CodeInputEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CodeInputEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13CodeInputEditE_t>.strings))
        return static_cast<void*>(this);
    return QPlainTextEdit::qt_metacast(_clname);
}

int CodeInputEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPlainTextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void CodeInputEdit::returnPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CodeInputEdit::parameterSelectionChanged(bool _t1, const QPoint & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}
namespace {
struct qt_meta_tag_ZN12CommandPanelE_t {};
} // unnamed namespace

template <> constexpr inline auto CommandPanel::qt_create_metaobjectdata<qt_meta_tag_ZN12CommandPanelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "CommandPanel",
        "commandSubmitted",
        "",
        "command",
        "parameterSelectionChanged",
        "active",
        "QPoint",
        "globalStart",
        "previewParameter",
        "objectName",
        "clearParameterPreview",
        "commitParameter",
        "cancelParameterSelection",
        "submit"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'commandSubmitted'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'parameterSelectionChanged'
        QtMocHelpers::SignalData<void(bool, const QPoint &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 5 }, { 0x80000000 | 6, 7 },
        }}),
        // Slot 'previewParameter'
        QtMocHelpers::SlotData<void(const QString &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Slot 'clearParameterPreview'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'commitParameter'
        QtMocHelpers::SlotData<void(const QString &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Slot 'cancelParameterSelection'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'submit'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CommandPanel, qt_meta_tag_ZN12CommandPanelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject CommandPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12CommandPanelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12CommandPanelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12CommandPanelE_t>.metaTypes,
    nullptr
} };

void CommandPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CommandPanel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->commandSubmitted((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->parameterSelectionChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[2]))); break;
        case 2: _t->previewParameter((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->clearParameterPreview(); break;
        case 4: _t->commitParameter((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->cancelParameterSelection(); break;
        case 6: _t->submit(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CommandPanel::*)(const QString & )>(_a, &CommandPanel::commandSubmitted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CommandPanel::*)(bool , const QPoint & )>(_a, &CommandPanel::parameterSelectionChanged, 1))
            return;
    }
}

const QMetaObject *CommandPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CommandPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12CommandPanelE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CommandPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void CommandPanel::commandSubmitted(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void CommandPanel::parameterSelectionChanged(bool _t1, const QPoint & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}
QT_WARNING_POP
