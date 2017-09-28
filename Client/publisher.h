#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <QWidget>
#include <QTimer>
#include <QHostAddress>

#include <iostream>

#include <qmqtt/qmqtt.h>

/*

    Modo de uso

    1) instalar QMQTT

        a) descargar repo

        https://github.com/emqtt/qmqtt

        b) compilar
        Dentro del directorio raiz hacer:

        mkdir build
        cd build
        qmake ..
        make install

    2) agregar al proyecto

        En el .pro incluir las lineas:
        INCLUDEPATH += <QMQTT>/qmqtt/build/include
        INCLUDEPATH += <QMQTT>/qmqtt/build/include/qmqtt
        LIBS += <QMQTT>/qmqtt/build/lib/libqmqtt.so

    3) publicar un mensaje

        a) crear publisher y conectar

        quint16 port = 1883;
        Publisher* pub = new Publisher(QHostAddress("127.0.0.1"), port,"test_topic",false);
        pub->connectToHost();

        b) publicar
        QString msg = "Hola!";
        pub->publishOne(msg);


*/

class Publisher : public QMQTT::Client
{
    Q_OBJECT
public:
    Publisher(const QHostAddress& host = QHostAddress::LocalHost,
                       const quint16 port = 1883,
                        QString topic = "test",
                        QObject* parent = NULL,
                        bool encrypt = false)
        : QMQTT::Client(host, port, parent)
        , _number(0){
        topic_ = topic;
        encrypt_ = encrypt;
        connect(this, &Publisher::connected, this, &Publisher::onConnected);
        connect(this, &Publisher::disconnected, this, &Publisher::onDisconnected);
    }
    virtual ~Publisher() {}

    void publishOne(QString msg){
        QString sendmsg = msg;

        if(encrypt_)
            sendmsg = encrypt(msg);

        QMQTT::Message message(_number, topic_,sendmsg.toUtf8());
        publish(message);
        _number++;
    }
    bool encrypt_;
    quint16 _number;
    QString topic_;

    QString encrypt(QString msg){

        // TODO

        return msg;
    }

public slots:
    void onConnected(){
        subscribe(topic_, 0);
    }

    void onDisconnected(){}
};


#endif // PUBLISHER_H
