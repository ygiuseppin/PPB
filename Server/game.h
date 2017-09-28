#ifndef GAME_H
#define GAME_H

#include <QPoint>
#include <iostream>
#include <QImage>
#include <QPainter>
#include <QDateTime>

#include <vector>

using namespace std;

extern int PLAYER_AREA_W;
extern int PLAYER_AREA_H;
extern int BOMB_AREA_W;
extern int BOMB_AREA_H;
extern int TILE_AREA;
extern int ITEM_AREA;

extern int BOMB_FRAMES;
extern int PLAYER_FRAMES;
extern int EXPLODE_FRAMES;
extern int ITEM_FRAMES;
extern int ITEM_DURATION;
extern int INMUNITY_TIME;

extern int MAX_VELOCITY;
extern int MIN_DELAY;
extern int MAX_BOMBS;

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

#define VELOCITY_NORMAL 0
#define VELOCITY_FAST 1
#define VELOCITY_SLOW 2

#define MAP_NORMAL 0
#define MAP_EMPTY 1
#define MAP_BLOCKS 2

// auxiliar
bool rectanglesIntersect(QRect r1,QRect r2);
QRect getPlayerRect(QPoint pos);



class Tile{

public:
    Tile(){}
    int type_;
    int stat_;
};

class Scenario{
public:
    Scenario(){

    }

    Scenario(int dims,int s_type){
        dims_ = dims;
        s_type_ = s_type;
        generate();
    }

    QRect getRect(int x,int y){

        int x1,x2,y1,y2;
        x1 = x*TILE_AREA;
        y1 = y*TILE_AREA;
        x2 = x1+TILE_AREA;
        y2 = y1+TILE_AREA;
        QPoint tl(x1,y1);
        QPoint br(x2,y2);
        QRect tile_rect(tl,br);
        return tile_rect;

    }

    QRect getRect(int i){
        int x_index = i%dims_;
        int y_index = i/dims_;
        return getRect(x_index,y_index);
    }

    Tile getTile(int x,int y){
        int index = dims_*y+x;
        return getTile(index);
    }

    Tile getTile(int i){
        return tiles_[i];
    }

    void setTile(int i,Tile t){
        tiles_[i] = t;
    }

    void setTile(int x,int y,Tile t){
        int index = dims_*y+x;
        setTile(index,t);
    }

    int dims_;
    int s_type_;

private:
    vector<Tile> tiles_;
    void generate(){

        // generar el escenario dependiendo del tipo ..

        for(uint i=0;i<dims_*dims_;i++){
            Tile t;
            int x_index = i%dims_;
            int y_index = i/dims_;

            if(x_index == 0 || x_index == dims_-1 || y_index == 0 || y_index == dims_-1){
                t.type_ = TILE_BLOCK;
            }else{
                if(x_index >3 && x_index < 7)
                    t.type_ = TILE_BRICK;
                else
                    t.type_ = TILE_FIELD;
            }
            tiles_.push_back(t);
        }


        /*
        // sacar los bricks del campo inicial de los jugadores
        scenario_[0].type_ = TILE_FIELD;
        scenario_[1].type_ = TILE_FIELD;

        scenario_[scenario_size-1].type_ = TILE_FIELD;
        scenario_[scenario_size-2].type_ = TILE_FIELD;

        scenario_[scenario_size].type_ = TILE_FIELD;
        scenario_[scenario_size*2-1].type_ = TILE_FIELD;

        scenario_[(scenario_size-2)*scenario_size].type_ = TILE_FIELD;
        scenario_[scenario_size*(scenario_size-1)-1].type_ = TILE_FIELD;

        scenario_[scenario_size*(scenario_size-1)].type_ = TILE_FIELD;
        scenario_[scenario_size*(scenario_size-1)+1].type_ = TILE_FIELD;

        scenario_[scenario_size*scenario_size-1].type_ = TILE_FIELD;
        scenario_[scenario_size*scenario_size-2].type_ = TILE_FIELD;
        */
    }
};

class Item{
public:
    Item(int type, int position, int scenario_size);
    QRect getRect();
    int type_;
    int position_;
    int frame_;
    int duration_;
    int scenario_size_;

};

class ExplosionArea{
public:
    ExplosionArea(int position, int type){
        position_ = position;
        frame_ = 0;
        type_ = type;
    }
    int type_;
    int position_;
    int frame_;
};


class Player{

public:
    Player(int id, QPoint position,int lives,QString name);
    QPoint position_;
    QRect getRect();

    int id_;
    QString name_;
    int side_;
    int frame_;
    bool moving_;
    int velocity_;
    int bomb_range_;
    int bomb_duration_;
    int lives_;

    int bomb_delay_;
    int max_bombs_;
    int inmunity_;
    int curr_delay_;
    int curr_bombs_;

};

vector<Player> sortPlayersByY(vector<Player> players);

class Bomb{

public:
    Bomb(int position, int duration, int range, int scenario_size);
    QRect getRect();
    int duration_;
    int range_;
    int frame_;
    int position_;
    int time_;
    int scenario_size_;
};

class Game : public QObject
{
    Q_OBJECT
public:

    Game(int players_count,
         int scenario_size,
         int scenario_type,
         int lives_count,
         int velocity,
         QString player1="",
         QString player2="",
         QString player3="",
         QString player4="");


    QString getFrameMessage();
    void fillBaseImage();
    bool placeBomb(int player_id,Bomb b);
    void keyPressed(int player,int key);
    void keyReleased(int player,int key);
    void keyPressed(QString player,int key);
    void keyReleased(QString player,int key);
    QPoint getPlayerPos(int id);
    vector<Tile> getScenario();
    Tile getTile(QPoint pos);
    void loop();
    bool canMoveTo(int id,QPoint prev_position,QPoint new_position);
    QImage getScenarioImage();
    vector<ExplosionArea> getBombExplosionAreas(Bomb b);
    void getScenarioItem(int index_x,int index_y);

private:
    QImage img_;
    QImage base_image_;
    vector<Player> players_;
    vector<Bomb> bombs_;

    vector<Item> items_;
    vector<ExplosionArea> explosions_;

    // init config
    int players_count_;
    int scenario_size_;
    int scenario_type_;
    int lives_count_;
    int velocity_;

    Scenario scenario_;

signals:
    void debug(QString msg);

};

#endif // GAME_H
