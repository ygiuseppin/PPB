
#include <QApplication>
#include <QThread>
#include <QHostAddress>

#include "mainwindow.h"

// areas
int PLAYER_AREA_W = 50;
int PLAYER_AREA_H = 100;
int BOMB_AREA_W = 50;
int BOMB_AREA_H = 50;
int TILE_AREA = 50;
int ITEM_AREA = 25;

// frames / cantidad de imagenes
int ITEM_FRAMES = 9;
int BOMB_FRAMES = 89;
int PLAYER_FRAMES = 16;
int EXPLODE_FRAMES = 5;

int ITEM_DURATION = 400;
int INMUNITY_TIME = 20;

// tope de valores
int MAX_VELOCITY = 12;
int MIN_DELAY = 100;
int MAX_BOMBS = 10;

Subscriber* subscriber;
Publisher* publisher;
Publisher* map_publisher;

GameConfig game_config;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qsrand(QTime::currentTime().msec());

    quint16 port = 1883;
    subscriber = new Subscriber(QHostAddress::LocalHost,port,"ppb");
    publisher = new Publisher(QHostAddress::LocalHost,port,"ppb");
    map_publisher = new Publisher(QHostAddress::LocalHost,port,"ppb/map");

    subscriber->connectToHost();
    publisher->connectToHost();
    map_publisher->connectToHost();

    game_config.player1 = "";
    game_config.player2 = "";
    game_config.player3 = "";
    game_config.player4 = "";
    game_config.lives = 3;
    game_config.velocity = VELOCITY_NORMAL;
    game_config.map_type = MAP_NORMAL;
    game_config.map_size = 10;

    MainWindow w;
    //w.show();

    return a.exec();

}
