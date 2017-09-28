#ifndef CLIENTEPPB_H
#define CLIENTEPPB_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QPainter>

#include "publisher.h"
#include "subscriber.h"

extern Subscriber* subscriber;
extern Subscriber* map_subscriber;
extern Publisher* publisher;
extern QString username;



extern int PLAYER_AREA_W;
extern int PLAYER_AREA_H;
extern int BOMB_AREA_W;
extern int BOMB_AREA_H;
extern int TILE_AREA;
extern int ITEM_AREA;

#define TILE_FIELD 0
#define TILE_BRICK 1
#define TILE_BLOCK 2

#define BACK_SIDE 0
#define FRONT_SIDE 1
#define RIGHT_SIDE 2
#define LEFT_SIDE 3

#define KEY_UP 0
#define KEY_DOWN 1
#define KEY_RIGHT 2
#define KEY_LEFT 3
#define KEY_BOMB 4

// tipos de explosiones
#define EXP_CENTRAL 0
#define EXP_HORIZONTAL 1
#define EXP_VERTICAL 2
#define EXP_END_UP 3
#define EXP_END_RIGHT 4
#define EXP_END_DOWN 5
#define EXP_END_LEFT 6


#define ITEM_BOMB 0
#define ITEM_RANGE 1
#define ITEM_VELOCITY 2
#define ITEM_DELAY 3

struct Player{
    int id;
    QString name;
    int posx;
    int posy;
    int frame;
    int side;
    bool inmune;
    int bombs;
    int lives;
    int velocity;
};

struct Bomb{
    int pos;
    int frame;
};

struct Brick{
    int pos;
};

struct Block{
    int pos;
};

struct Explosion{
    int pos;
    int type;
    int frame;
};

struct Item{
    int pos;
    int type;
    int frame;
};

struct Scenario{

    int size;
    std::vector<Player> players;
    std::vector<Bomb> bombs;
    std::vector<Block> blocks;
    std::vector<Brick> bricks;
    std::vector<Explosion>  explosions;
    std::vector<Item> items;

};

namespace Ui {
class ClientePPB;
}

class ClientePPB : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientePPB(QWidget *parent = 0);
    ~ClientePPB();

private:
    Ui::ClientePPB *ui;
    void drawPlayerStatus(QImage& img, Player p, int scenario_size);
    void handleNewFrame(QString msg);
    void handleGameOver(QString msg);
    void handleNewChatMessage(QString msg);
    void drawScenario(Scenario scenario);

private slots:
    void messageReceived(QString msg);
    void mapMessageReceived(QString msg);

    void on_send_clicked();

protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
};

#endif // CLIENTEPPB_H
