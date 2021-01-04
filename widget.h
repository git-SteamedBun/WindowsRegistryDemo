#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);

    ~Widget();

    void onAutoRun(bool isAuto);

    bool isAutoRun();

private:
    void setTip(unsigned long error, const QString &tipHead = "");

    void setConnect();

    void initReg();

    void openRegKey();

    void closeRegKey();

    void setAutoRun();

    bool getAutoRun(QString* key, QString* value);

    void cancelAutoRun();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
