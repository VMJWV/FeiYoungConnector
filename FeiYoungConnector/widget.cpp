#include "widget.h"
#include "ui_widget.h"
#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkReply>
#include <QMessageBox>
#include <QDomDocument>
#include <QRegExp>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    getLoginInfo();
    void (Widget::*p)(const QString &,const QString &,const QString &) = &Widget::loginByPhone;
    void (Widget::*p1)(const QString &,const QString &,const QString &) = &Widget::loginByFormData;

    connect(this,p,this,[&](const QString &AidcAuthAttr1,const QString &AidcAuthAttr2,const QString &LoginURL){
        Q_UNUSED(AidcAuthAttr2)
        auto bytes = QString("AidcAuthAttr1=%1&"
                "AidcAuthAttr3=XtUpGrxT&"
                "AidcAuthAttr4=BqtwW+EENTj35oiDD/fL&"
                "AidcAuthAttr5=XssoGrlVKj7144s=&"
                "AidcAuthAttr6=CcM1DbpMYWn24tKfDerSdmM=&"
                "AidcAuthAttr7=&"
                "AidcAuthAttr8=&"
                "PassWord=5024b78479f0cc19&"
                "UserName=!^Iqnd0%3&"
                "createAuthorFlag=0").arg(AidcAuthAttr1).arg(ui->phoneNumber->text()).toUtf8();
        auth(bytes,LoginURL);
    });
    connect(this,p1,this,[&](const QString &AidcAuthAttr1,const QString &AidcAuthAttr2,const QString &LoginURL){
        Q_UNUSED(AidcAuthAttr2)
        QString res = ui->customContent->text();
        QRegExp rx("AidcAuthAttr1=(.*)&");
        rx.setMinimal(true);
        res.replace(rx,"AidcAuthAttr1=" + AidcAuthAttr1 + "&");
        auth(res.toUtf8(),LoginURL);
    });
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_getInfo_clicked()
{
    auto manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("http://www.msftconnecttest.com/redirect"));
    manager->get(request);
    connect(manager,&QNetworkAccessManager::finished,this,[&](QNetworkReply *reply){
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode != 302){
            QMessageBox::warning(this,"获取信息时出错","redirect failure!");
            reply->deleteLater();
            return;
        }
        auto location = QString(reply->rawHeader("Location"));
        auto params = location.mid(location.indexOf("?") + 1);
        auto list = params.split("&");
        /*"Location" : "http://58.53.199.144:8001?
                            userip=100.64.18.152&
                            wlanacname=&
                            nasip=59.172.216.49&
                            usermac=7c-76-35-e6-02-7b"*/
        int count = 0;
        foreach (auto param, list) {
            auto kvlist = param.split("=");
            if(kvlist.size() == 0){
                QMessageBox::warning(this,"获取信息时出错","信息获取出现问题");
                ui->tips->setText("获取基本信息出错");
                reply->deleteLater();
                return;
            }
            if(kvlist[0] == "usermac"){
                ui->mac->setText(kvlist[1]);
                ++count;
            }
            else if(kvlist[0] == "nasip"){
                ui->nasIp->setText(kvlist[1]);
                ++count;
            }
            else if(kvlist[0] == "userip"){
                ui->ip->setText(kvlist[1]);
                ++count;
            }
        }
        if(count == 3){
            ui->tips->setText("获取成功");
        }
        else{
            ui->tips->setText("获取失败 请检查");
        }
        /*for (auto ele : reply->rawHeaderPairs()) qDebug() << ele.first << ":" << ele.second;*/
        reply->deleteLater();
    });
}

void Widget::on_auth_clicked()
{
    if(ui->ip->text().size() == 0 || ui->nasIp->text().size() == 0 || ui->mac->text().size() == 0){
        QMessageBox::warning(this,"请先获取","请先获取在认证");
        ui->tips->setText("请先获取在认证");
        return;
    }
    if(ui->phoneNumber->text().size() == 0){
        QMessageBox::warning(this,"错误","请填写手机号");
        ui->tips->setText("手机号码不可为空");
        return;
    }
    type = 1;
    getLoginInfo();
}

void Widget::getLoginInfo(){
    auto manager = new QNetworkAccessManager(this);
    auto url = QString("http://58.53.199.144:8001?userip=%1&wlanacname=&nasip=%2&usermac=%3&aidcauthtype=0")
            .arg(ui->ip->text()).arg(ui->nasIp->text()).arg(ui->mac->text());
    QNetworkRequest getLoginUrlRequest(url);
    getLoginUrlRequest.setRawHeader("User-Agent","CDMA+WLAN(Maod)");
    manager->get(getLoginUrlRequest);
    connect(manager,&QNetworkAccessManager::finished,this,[&](QNetworkReply *reply){
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode != 200){
            QMessageBox::warning(this,"获取登录地址时出错","获取登录地址时出错!");
            ui->tips->setText("获取登录地址时出错");
            reply->deleteLater();
            return;
        }
        auto content = QString(reply->readAll());
        QDomDocument document;
        document.setContent(content);
        auto AidcAuthAttr1 = document.elementsByTagName("AidcAuthAttr1").at(0).toElement().text();
        auto AidcAuthAttr2 = document.elementsByTagName("AidcAuthAttr2").at(0).toElement().text();
        auto LoginURL = document.elementsByTagName("LoginURL").at(0).toElement().text();
        if(type == 1){
            emit loginByPhone(AidcAuthAttr1,AidcAuthAttr2,LoginURL);
        }
        else if(type == 2){
            emit loginByFormData(AidcAuthAttr1,AidcAuthAttr2,LoginURL);
        }
        reply->deleteLater();
    });
}


void Widget::auth(const QByteArray &bytes,const QString &LoginURL){
    auto manager = new QNetworkAccessManager(this);
    QNetworkRequest request(LoginURL);
    request.setRawHeader("Content-Type","application/x-www-form-urlencoded");
    request.setRawHeader("Host","58.53.199.144:8001");
    request.setRawHeader("User-Agent","CDMA+WLAN(Maod)");
    //CDMA+WLAN(win64) //PC端
    //"CDMA+WLAN(Maod)" //安卓端
    //"CDMA+WLAN(Mios)" //苹果端
    //request.setRawHeader("User-Agent","CDMA+WLAN(win64)");
    manager->post(request,bytes);
    connect(manager,&QNetworkAccessManager::finished,this,[&](QNetworkReply *reply){
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode != 200){
            QMessageBox::warning(this,"认证出错","认证出错!");
            ui->tips->setText("认证出错");
            reply->deleteLater();
            return;
        }
        auto content = QString(reply->readAll());
        qDebug() << content;
        QDomDocument document;
        document.setContent(content);
        auto message = document.elementsByTagName("ReplyMessage").at(0).toElement().text();
        ui->tips->setText(message);
        reply->deleteLater();
    });
}

void Widget::on_costomAuth_clicked()
{
    if(ui->ip->text().size() == 0 || ui->nasIp->text().size() == 0 || ui->mac->text().size() == 0){
        QMessageBox::warning(this,"请先获取","请先获取在认证");
        ui->tips->setText("请先获取在认证");
        return;
    }
    if(ui->customContent->text().size() == 0){
        QMessageBox::warning(this,"错误","请输入内容");
        ui->tips->setText("认证内容不可以为空");
        return;
    }
    type = 2;
    getLoginInfo();
}
