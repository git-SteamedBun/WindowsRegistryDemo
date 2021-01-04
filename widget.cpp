#include "widget.h"
#include "ui_widget.h"

#include <stdio.h>
#include <qt_windows.h>
#include <QDebug>

static HKEY hKey = HKEY_CURRENT_USER; //root
static LPCSTR lpSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static HKEY phkResult;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    initReg();
    QString key, value;
    getAutoRun(&key, &value);
    ui->keyLabel->setText(key);
    ui->valueLabel->setText(value);
    setConnect();
    ui->pathLabel->setText("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
}

Widget::~Widget()
{
    delete ui;
}

void Widget::onAutoRun(bool isAuto)
{
    if(isAuto)
    {
        setAutoRun();
    }
    else
    {
        cancelAutoRun();
    }
}

bool Widget::isAutoRun()
{
    QString key, value;
    getAutoRun(&key, &value);
    if(key.isEmpty() && value.isEmpty())
    {
        qDebug() << key;
        qDebug() << value;
        return false;
    }
    return true;
}

void Widget::setTip(unsigned long error, const QString &tipHead)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, error, 0, (LPWSTR)&lpMsgBuf, 0, nullptr);
    QString strMessage = QString::fromStdWString(static_cast<LPWSTR>(lpMsgBuf));
    qDebug() << tipHead + strMessage;
    ui->tipLabel->setText(tipHead + strMessage);
}

void Widget::setConnect()
{
    connect(ui->writeBtn, &QPushButton::clicked, this, [this](){
        setAutoRun();
    });
    connect(ui->readBtn, &QPushButton::clicked, this, [this](){
        QString key, value;
        getAutoRun(&key, &value);
        ui->keyLabel->setText(key);
        ui->valueLabel->setText(value);
    });
    connect(ui->removeBtn, &QPushButton::clicked, this, [this](){
        if(isAutoRun())
        {
            cancelAutoRun();
        }
    });
}

void Widget::initReg()
{
    DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
//    const wchar_t* subKey = reinterpret_cast<const wchar_t*>(lpSubKey);
    LONG lReg = RegCreateKeyExA(
                hKey,
                lpSubKey,
                0,
                nullptr,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                nullptr,
                &phkResult,
                &dwDisposition);
    setTip(lReg, "initReg");
//    RegCloseKey(hKey);
    closeRegKey();
}

void Widget::openRegKey()
{
//    const wchar_t* subKey = reinterpret_cast<const wchar_t*>(lpSubKey);
    LONG lReg = RegOpenKeyExA(hKey, lpSubKey, 0, KEY_ALL_ACCESS, &phkResult);
//    setTip(lReg, "openReg");
}

void Widget::closeRegKey()
{
    LONG lReg = RegCloseKey(phkResult);
//    setTip(lReg, "closeReg");
}

void Widget::setAutoRun()
{
    openRegKey();

//    QString tempKey = QApplication::applicationName();
//    const wchar_t* wcKey = reinterpret_cast<const wchar_t*>(tempKey.utf16());

    QByteArray keyByte = QApplication::applicationName().toLatin1();
    char* keyChar = keyByte.data();

    QByteArray valueByte = QApplication::applicationFilePath().replace("/", "\\").toLatin1();
    char* valueChar = valueByte.data();

    LONG lReg = RegSetValueExA(
                phkResult,
                keyChar,
                0,
                REG_SZ,
                reinterpret_cast<unsigned char*>(valueChar),
                static_cast<DWORD>(strlen(valueChar)));
    setTip(lReg, "setAuto");
    closeRegKey();
}

bool Widget::getAutoRun(QString *key, QString *value)
{
    openRegKey();

    DWORD dwSize = 1024;
    DWORD dwType = REG_SZ;
    WCHAR buf[1024];
    QString keyTemp = QApplication::applicationName();
    const wchar_t* wcKey = reinterpret_cast<const wchar_t*>(keyTemp.utf16());
//    QString keyTemp = QApplication::applicationName();
//    QByteArray keyByte = keyTemp.toLatin1();
//    char* keyChar = keyByte.data();
    LONG lReg = RegQueryValueEx(phkResult, wcKey, nullptr, &dwType, reinterpret_cast<LPBYTE>(&buf), &dwSize);
    QString tempValue = QString::fromUtf16(reinterpret_cast<const ushort*>(buf), dwSize / 2 - 1);
    setTip(lReg, "getAuto");
    closeRegKey();
    if(lReg != ERROR_SUCCESS)
    {
        return false;
    }
    *key = keyTemp;
    *value = tempValue;
    return true;
}

void Widget::cancelAutoRun()
{
    openRegKey();
//    QString keyTemp = QApplication::applicationName();
//    const wchar_t* wcKey = reinterpret_cast<const wchar_t*>(keyTemp.utf16());
//    const wchar_t* subKey = reinterpret_cast<const wchar_t*>(lpSubKey);

    QByteArray keyByte = QApplication::applicationName().toLatin1();
    char* keyChar = keyByte.data();
    LONG lReg = RegDeleteKeyValueA(hKey, lpSubKey, keyChar);
    setTip(lReg, "cancelAuto");
    closeRegKey();
}

