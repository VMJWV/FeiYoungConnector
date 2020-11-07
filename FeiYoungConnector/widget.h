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

private slots:
    void on_auth_clicked();

    void on_getInfo_clicked();


    void on_costomAuth_clicked();

private:
    Ui::Widget *ui;
    void getLoginInfo();
    int type = 0;
    void auth(const QByteArray &bytes,const QString &LoginURL);

signals:
    void loginByPhone(const QString &AidcAuthAttr1,const QString &AidcAuthAttr2,const QString &LoginURL);
    void loginByFormData(const QString &AidcAuthAttr1,const QString &AidcAuthAttr2,const QString &LoginURL);
};
#endif // WIDGET_H
