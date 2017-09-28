#include <iostream>

#include <QApplication>
#include <QThread>

#include "clienteppb.h"
#include "logindialog.h"

using namespace std;


int PLAYER_AREA_W = 50;
int PLAYER_AREA_H = 100;
int BOMB_AREA_W = 50;
int BOMB_AREA_H = 50;
int TILE_AREA = 50;
int ITEM_AREA = 25;


Subscriber* subscriber;
Subscriber* map_subscriber;
Publisher* publisher;
QString username = "";

QString style = "background-color: rgb(200, 200, 200);color: rgb(30, 30, 30);selection-background-color: rgb(48, 138, 255);";


// Cliente PPB
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // -- Login --
    LoginDialog login;
    login.setStyleSheet(style);

    login.show();

    while(!login.close_){
        QThread::msleep(10);
        QApplication::processEvents();
    }

    if(login.connected_){

        // -- start --
        ClientePPB w;
        w.setStyleSheet(style);

        w.show();
        return a.exec();

    }else{
        return 0;
    }
}
