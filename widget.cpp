#include "widget.h"
#include "ui_widget.h"

#include <stdio.h>
#include <qt_windows.h>
#include <QDebug>

static HKEY hKey = HKEY_CURRENT_USER; //root
static QString lpSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static HKEY phkResult;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    initReg();
    showKeyValue();
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

void Widget::showKeyValue()
{
    QString key, value;
    getAutoRun(&key, &value);
    ui->keyLabel->setText(key);
    ui->valueLabel->setText(value);
}

void Widget::setTip(unsigned long error, const QString &tipHead)
{
    QString strMessage = QStringLiteral(" 操作成功 ");
    ui->tipLabel->setText("");
    if(error != ERROR_SUCCESS)
    {
        LPVOID lpMsgBuf;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, error, 0, (LPWSTR)&lpMsgBuf, 0, nullptr);
        QString strMessage = QString::fromStdWString(static_cast<LPWSTR>(lpMsgBuf));
        ui->tipLabel->setText(tipHead + strMessage);
    }

    qDebug() << tipHead + strMessage;

}

void Widget::setConnect()
{
    connect(ui->writeBtn, &QPushButton::clicked, this, [this](){
        setAutoRun();
    });
    connect(ui->readBtn, &QPushButton::clicked, this, [this](){
        showKeyValue();
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
    const wchar_t* subKey = reinterpret_cast<const wchar_t*>(lpSubKey.utf16());
    LONG lReg = RegCreateKeyExW(
                hKey,
                subKey,
                0,
                nullptr,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                nullptr,
                &phkResult,
                &dwDisposition);
    setTip(lReg, "init");
    closeRegKey();
}

void Widget::openRegKey()
{
    const wchar_t* subKey = reinterpret_cast<const wchar_t*>(lpSubKey.utf16());
    RegOpenKeyExW(hKey, subKey, 0, KEY_ALL_ACCESS, &phkResult);
}

void Widget::closeRegKey()
{
    RegCloseKey(phkResult);
}

void Widget::setAutoRun()
{
    openRegKey();
    QByteArray keyByte = QApplication::applicationName().replace("/", "\\").toLatin1();
    char* keyChar = keyByte.data();

    QString tempValue = QApplication::applicationFilePath().replace("/", "\\");
    const wchar_t* wcValue = reinterpret_cast<const wchar_t*>(tempValue.utf16());
    int wlen = static_cast<int>(wcslen(wcValue)) * 2;
    char *pElementText = new char[wlen];
    WideCharToMultiByte(CP_ACP, NULL, wcValue, -1, pElementText, wlen + 2, NULL, NULL);

    LONG lReg = RegSetValueExA(
                phkResult,
                keyChar,
                0,
                REG_SZ,
                (unsigned char*)pElementText,
                wlen);
    setTip(lReg, "set");
    delete [] pElementText;
    closeRegKey();
    showKeyValue();
}

bool Widget::getAutoRun(QString *key, QString *value)
{
    openRegKey();

    DWORD dwType = REG_SZ;
    DWORD cbData = 1024;
    WCHAR lpData[1024];

    QString keyTemp = QApplication::applicationName().replace("/", "\\");
    const wchar_t* wcKey = reinterpret_cast<const wchar_t*>(keyTemp.utf16());

    LONG lReg = RegQueryValueExW(phkResult, wcKey, nullptr, &dwType, (LPBYTE)lpData, &cbData);

    int wcLen = static_cast<int>(wcslen(lpData));
    QString tempValue = QString::fromUtf16(reinterpret_cast<const ushort *>(lpData), wcLen);
    setTip(lReg, "get");
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
    const wchar_t* subKey = reinterpret_cast<const wchar_t*>(lpSubKey.utf16());

    QString keyTemp = QApplication::applicationName().replace("/", "\\");
    const wchar_t* wcKey = reinterpret_cast<const wchar_t*>(keyTemp.utf16());

    LONG lReg = RegDeleteKeyValueW(hKey, subKey, wcKey);
    setTip(lReg, "cancel");
    closeRegKey();
    showKeyValue();
}

