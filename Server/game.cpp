#include "game.h"

//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\\
//\/\/\/\/\/\/\/\/\/ GAME \/\/\/\/\/\/\/\/\/\\
//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\\

void Game::fillBaseImage(){

    int width_height = scenario_size_*TILE_AREA;
    base_image_ = QImage(width_height,width_height,QImage::Format_RGB32);
    base_image_.fill(Qt::white);

    // dibujar tiles
    QImage tile_img;
    for(uint i=0;i<scenario_size_*scenario_size_;i++){

        int x_index = i%scenario_size_;
        int y_index = i/scenario_size_;

        if(x_index%2==0 && y_index%2==0)
            tile_img = QImage(":/new/prefix/images/pisos/rayado/1.png");
        if(x_index%2!=0 && y_index%2==0)
            tile_img = QImage(":/new/prefix/images/pisos/rayado/2.png");
        if(x_index%2==0 && y_index%2!=0)
            tile_img = QImage(":/new/prefix/images/pisos/rayado/3.png");
        if(x_index%2!=0 && y_index%2!=0)
            tile_img = QImage(":/new/prefix/images/pisos/rayado/4.png");


        tile_img = tile_img.scaled(QSize(TILE_AREA,TILE_AREA));

        QPoint destPos = scenario_.getRect(i).topLeft();
        QPainter painter(&base_image_);
        painter.drawImage(destPos, tile_img);
        painter.end();

    }
}

Game::Game(int players_count, int scenario_size, int scenario_type, int lives_count,int velocity,QString player1,QString player2, QString player3,QString player4)
{
    velocity_ = velocity;
    players_count_ = players_count;
    lives_count_ = lives_count;
    scenario_type_ = scenario_type;
    scenario_size_ = scenario_size;

    // crear jugadores
    if(players_count>0){
        QPoint player1_position(0+TILE_AREA,1);
        Player p1(0,player1_position,lives_count,player1);
        players_.push_back(p1);
    }

    if(players_count>1){
        QPoint player2_position(scenario_size*TILE_AREA-PLAYER_AREA_W-TILE_AREA-1,1);
        Player p2(1,player2_position,lives_count,player2);
        players_.push_back(p2);
    }

    if(players_count>2){
        QPoint player3_position(0+TILE_AREA,scenario_size*TILE_AREA-PLAYER_AREA_H-TILE_AREA-1);
        Player p3(2,player3_position,lives_count,player3);
        players_.push_back(p3);
    }

    if(players_count>3){
        QPoint player4_position(scenario_size*TILE_AREA-PLAYER_AREA_W-TILE_AREA-1,
                                scenario_size*TILE_AREA-PLAYER_AREA_H-TILE_AREA-1);
        Player p4(3,player4_position,lives_count,player4);
        players_.push_back(p4);
    }

    scenario_ = Scenario(scenario_size_,scenario_type_);

    //crear imagen con todos field y block a los costados, para modificar siempre esa
    fillBaseImage();
}


QRect getPlayerRect(QPoint pos){

    QPoint tl(pos.x()+5,pos.y()+PLAYER_AREA_H/3*2);
    QPoint br(pos.x()+PLAYER_AREA_W-5,pos.y()+PLAYER_AREA_H);
    return QRect(tl,br);

}

bool Game::canMoveTo(int id,QPoint prev_position,QPoint new_position){

    // se escapa del escenario?
    if(new_position.x()<0) return false;
    if(new_position.y()<0) return false;
    if(new_position.x()+PLAYER_AREA_W>scenario_size_*TILE_AREA) return false;
    if(new_position.y()+PLAYER_AREA_H>scenario_size_*TILE_AREA) return false;

    QRect new_r = getPlayerRect(new_position);
    QRect prev_r = getPlayerRect(prev_position);

    // se choca con algun jugador?
    for(uint i=0;i<players_.size();i++){
        if(players_[i].lives_>0 && id != players_[i].id_){
            Player p = players_[i];
            QRect r = p.getRect();
            if(rectanglesIntersect(new_r,r))
                return false;
        }
    }

    // se choca con algun brick/block?
    for(uint i =0;i<scenario_.dims_*scenario_.dims_;i++){
        Tile t = scenario_.getTile(i);
        if(t.type_ == TILE_BLOCK || t.type_ == TILE_BRICK){

            // se choca con ese tile?
            QRect tile_rect = scenario_.getRect(i);
            if(rectanglesIntersect(new_r,tile_rect))
                return false;

        }
    }

    // se choca con una bomba que no es una sobre la que ya estaba parada?
    for(uint i = 0;i<bombs_.size();i++){
        Bomb b = bombs_[i];

        // si no estaba antes parado sobre esa bomba
        if(!rectanglesIntersect(prev_r,b.getRect())){

            // si me superpongo con esa bomba
            if(rectanglesIntersect(b.getRect(),new_r))
                return false;

        }
    }
    return true;
}

bool rectanglesIntersect(QRect r1,QRect r2){
    return r1.intersects(r2);
}

vector<ExplosionArea> Game::getBombExplosionAreas(Bomb b){

    vector<ExplosionArea> res;

    //area central
    ExplosionArea central(b.position_,EXP_CENTRAL);
    res.push_back(central);

    //arriba
    vector<ExplosionArea> up_areas;
    for(uint i=1;i<b.range_+1;i++){
        int x = b.position_%scenario_size_;
        int y = b.position_/scenario_size_;

        if(y-i >=0){

            Tile t = scenario_.getTile(x,y-i);

            if(t.type_ == TILE_BLOCK)
                i = b.range_+1; // for: break

            if(t.type_ == TILE_BRICK){
                ExplosionArea exp_area(b.position_-i*scenario_size_,EXP_VERTICAL);
                up_areas.push_back(exp_area);
                i = b.range_+1; // for: break
            }

            if(t.type_ == TILE_FIELD){
                ExplosionArea exp_area(b.position_-i*scenario_size_,EXP_VERTICAL);
                up_areas.push_back(exp_area);
            }
        }
    }

    // cambiar el tipo al ultimo, por EXP_END_UP
    if(up_areas.size()>0)
        up_areas[up_areas.size()-1].type_ = EXP_END_UP;

    //abajo
    vector<ExplosionArea> down_areas;
    for(uint i=1;i<b.range_+1;i++){
        int x = b.position_%scenario_size_;
        int y = b.position_/scenario_size_;

        if(y+i <=scenario_size_){

            Tile t = scenario_.getTile(x,y+i);

            if(t.type_ == TILE_BLOCK)
                i = b.range_+1; // for: break

            if(t.type_ == TILE_BRICK){
                ExplosionArea exp_area(b.position_+i*scenario_size_,EXP_VERTICAL);
                down_areas.push_back(exp_area);
                i = b.range_+1; // for: break
            }

            if(t.type_ == TILE_FIELD){
                ExplosionArea exp_area(b.position_+i*scenario_size_,EXP_VERTICAL);
                down_areas.push_back(exp_area);
            }
        }
    }

    // cambiar el tipo al ultimo, por EXP_END_UP
    if(down_areas.size()>0)
        down_areas[down_areas.size()-1].type_ = EXP_END_DOWN;


    // derecha
    vector<ExplosionArea> right_areas;
    for(uint i=1;i<b.range_+1;i++){
        int x = b.position_%scenario_size_;
        int y = b.position_/scenario_size_;

        if(x+i <=scenario_size_){

            Tile t = scenario_.getTile(x+i,y);

            if(t.type_ == TILE_BLOCK)
                i = b.range_+1; // for: break

            if(t.type_ == TILE_BRICK){
                ExplosionArea exp_area(b.position_+i,EXP_HORIZONTAL);
                right_areas.push_back(exp_area);
                i = b.range_+1; // for: break
            }

            if(t.type_ == TILE_FIELD){
                ExplosionArea exp_area(b.position_+i,EXP_HORIZONTAL);
                right_areas.push_back(exp_area);
            }
        }
    }

    // cambiar el tipo al ultimo, por EXP_END_UP
    if(right_areas.size()>0)
        right_areas[right_areas.size()-1].type_ = EXP_END_RIGHT;


    //izquierda
    vector<ExplosionArea> left_areas;
    for(uint i=1;i<b.range_+1;i++){
        int x = b.position_%scenario_size_;
        int y = b.position_/scenario_size_;

        if(x-i >=0){

            Tile t = scenario_.getTile(x-i,y);

            if(t.type_ == TILE_BLOCK)
                i = b.range_+1; // for: break

            if(t.type_ == TILE_BRICK){
                ExplosionArea exp_area(b.position_-i,EXP_HORIZONTAL);
                left_areas.push_back(exp_area);
                i = b.range_+1; // for: break
            }

            if(t.type_ == TILE_FIELD){
                ExplosionArea exp_area(b.position_-i,EXP_HORIZONTAL);
                left_areas.push_back(exp_area);
            }
        }
    }

    // cambiar el tipo al ultimo, por EXP_END_UP
    if(left_areas.size()>0)
        left_areas[left_areas.size()-1].type_ = EXP_END_RIGHT;

    for(uint i=0;i<up_areas.size();i++)
        res.push_back(up_areas[i]);
    for(uint i=0;i<down_areas.size();i++)
        res.push_back(down_areas[i]);
    for(uint i=0;i<right_areas.size();i++)
        res.push_back(right_areas[i]);
    for(uint i=0;i<left_areas.size();i++)
        res.push_back(left_areas[i]);

    return res;

}

QString Game::getFrameMessage(){

    QString msg = "frame " + QString::number(scenario_size_);

    vector<int> scenario;
    for(uint i=0;i<scenario_size_*scenario_size_;i++){
        Tile t = scenario_.getTile(i);
        if(t.type_ == TILE_FIELD) scenario.push_back(0);
        if(t.type_ == TILE_BRICK) scenario.push_back(1);
        if(t.type_ == TILE_BLOCK) scenario.push_back(2);
    }
    for(uint i=0;i<bombs_.size();i++){
        Bomb b = bombs_[i];
        scenario[b.position_]=100+b.frame_;
    }
    for(uint i=0;i<explosions_.size();i++){
        ExplosionArea e = explosions_[i];
        scenario[e.position_]=200+e.type_*10+e.frame_;
    }
    for(uint i=0;i<items_.size();i++){
        Item y = items_[i];
        scenario[y.position_]=1000+y.type_*100+y.frame_;
    }

    for(uint i=0;i<scenario.size();i++)
        msg = msg+","+QString::number(scenario[i]);

    msg = msg+" "+ QString::number(players_.size());


    for(uint i=0;i<players_.size();i++){
        Player p = players_[i];
        msg = msg+","+QString::number(p.id_);
        msg = msg+","+p.name_;
        msg = msg+","+QString::number(p.position_.x());
        msg = msg+","+QString::number(p.position_.y());
        msg = msg+","+QString::number(p.frame_);
        msg = msg+","+QString::number(p.side_);
        QString inmune = "0";
        if(p.inmunity_>0)
            inmune="1";
        msg = msg+","+inmune;
        msg = msg+","+QString::number(p.curr_bombs_);
        msg = msg+","+QString::number(p.lives_);
        msg = msg+","+QString::number(p.velocity_);

    }

    return msg;
}

void Game::loop(){

    // mover los jugadores
    for(uint i=0;i<players_.size();i++){

        Player p = players_[i];
        std::cout << p.name_.toStdString() << ": " << p.curr_delay_ << std::endl;
        if(p.curr_delay_ >= p.bomb_delay_){

            if(p.curr_bombs_<p.max_bombs_)
                players_[i].curr_bombs_ = p.curr_bombs_ +1;
            players_[i].curr_delay_ = 0;

        }else{
            players_[i].curr_delay_ = p.curr_delay_+1;
        }

        if(p.moving_){

            // calcular la nueva posicion
            QPoint new_position;
            if(p.side_ == FRONT_SIDE)
                new_position = QPoint(p.position_.x(),p.position_.y()+p.velocity_);
            if(p.side_ == BACK_SIDE)
                new_position = QPoint(p.position_.x(),p.position_.y()-p.velocity_);
            if(p.side_ == RIGHT_SIDE)
                new_position = QPoint(p.position_.x()+p.velocity_,p.position_.y());
            if(p.side_ == LEFT_SIDE)
                new_position = QPoint(p.position_.x()-p.velocity_,p.position_.y());

            // si se puede mover a esa posicion
            if(canMoveTo(p.id_,p.position_,new_position)){

                players_[i].position_ = new_position;

                // si toca un item, eliminar item y realizar cambios
                for(uint h=0;h<items_.size();h++){
                    if(rectanglesIntersect(players_[i].getRect(),items_[h].getRect())){

                        // actualizar stats del jugador
                        int item_type = items_[h].type_;
                        if(item_type == ITEM_BOMB && players_[i].max_bombs_ < MAX_BOMBS)
                            players_[i].max_bombs_ = players_[i].max_bombs_+1;
                        if(item_type == ITEM_DELAY && players_[i].bomb_delay_ > MIN_DELAY)
                            players_[i].bomb_delay_ = players_[i].bomb_delay_-30;
                        if(item_type == ITEM_RANGE)
                            players_[i].bomb_range_ = players_[i].bomb_range_+1;
                        if(item_type == ITEM_VELOCITY && players_[i].velocity_ < MAX_VELOCITY)
                            players_[i].velocity_ = players_[i].velocity_+1;

                        // eliminar item
                        items_.erase(items_.begin()+h);
                    }
                }
            }
        }
    }

    // incrementar frames de jugadores si se estan moviendo
    for(uint i=0;i<players_.size();i++){
        if(players_[i].moving_){
            players_[i].frame_ =  players_[i].frame_+1;
            if(players_[i].frame_ == PLAYER_FRAMES-1)
                players_[i].frame_ = 0;
        }
        if(players_[i].inmunity_>=0)
            players_[i].inmunity_ = players_[i].inmunity_-1;
    }

    // aumentar frame de bombas y explotar si corresponde ..
    for(uint i=0;i<bombs_.size();i++){

        bombs_[i].frame_ = bombs_[i].frame_+1;
        if(bombs_[i].frame_ >= BOMB_FRAMES-1)
            bombs_[i].frame_ = 0;

        bombs_[i].time_ = bombs_[i].time_+1;

        if(bombs_[i].duration_ == bombs_[i].time_){

            // creo las explosiones
            vector<ExplosionArea> exps = getBombExplosionAreas(bombs_[i]);

            //
            for(uint e=0;e<exps.size();e++)
                explosions_.push_back(exps[e]);

            // para cada area de explosion
            for(uint j=0;j<exps.size();j++){

                // si un jugador la toca y no estaba inmune, se disminuye su vida
                for(uint k=0;k<players_.size();k++){
                    if(players_[k].lives_>0 && rectanglesIntersect(players_[k].getRect(),scenario_.getRect(exps[j].position_))){
                        if(players_[k].inmunity_ <=0 && players_[k].lives_>0){

                            // bajarle la vida al jugador y hacerlo inmune por un tiempo
                            players_[k].lives_ = players_[k].lives_-1;
                            players_[k].inmunity_ = INMUNITY_TIME;

                        }
                    }
                }

                // si un item la toca, se destruye
                for(uint t =0;t<items_.size();t++)
                    if(rectanglesIntersect(items_[t].getRect(),scenario_.getRect(exps[j].position_))&& items_[i].duration_!=0)
                        items_[i].duration_ = ITEM_DURATION-2;

                // si un brick la toca, se rompe y dropea un item (quizas)
                Tile destroyed_tile = scenario_.getTile(exps[j].position_);
                if(destroyed_tile.type_ == TILE_BRICK){
                    destroyed_tile.type_ = TILE_FIELD;
                    scenario_.setTile(exps[j].position_,destroyed_tile);

                    //qint64 curr_time = QDateTime::currentMSecsSinceEpoch();
                    int item_type = rand()%(7);
                    if(item_type == ITEM_BOMB){
                        Item bomb_item(ITEM_BOMB,exps[j].position_,scenario_size_);
                        items_.push_back(bomb_item);
                    }
                    if(item_type == ITEM_DELAY){
                        Item delay_item(ITEM_DELAY,exps[j].position_,scenario_size_);
                        items_.push_back(delay_item);
                    }
                    if(item_type == ITEM_RANGE){
                        Item range_item(ITEM_RANGE,exps[j].position_,scenario_size_);
                        items_.push_back(range_item);
                    }
                    if(item_type == ITEM_VELOCITY){
                        Item vel_item(ITEM_VELOCITY,exps[j].position_,scenario_size_);
                        items_.push_back(vel_item);
                    }

                }

                // si otra bomba la toca, esa bomba aumenta su frame a casi explotar
                for(uint k=0;k<bombs_.size();k++)
                    if(bombs_[k].position_ != bombs_[i].position_ &&
                            rectanglesIntersect(scenario_.getRect(exps[j].position_),bombs_[k].getRect()))
                        bombs_[k].time_ = bombs_[k].duration_-2;

            }

            // borro la bomba que explotó
            bombs_.erase(bombs_.begin()+i);


        }
    }

    // aumentar frames de explosiones y eliminar las que terminen
    for(uint i = 0;i<explosions_.size();i++){

        explosions_[i].frame_ = explosions_[i].frame_+1;
        if(explosions_[i].frame_ == EXPLODE_FRAMES){

            explosions_.erase(explosions_.begin()+i);
            i--;
        }
    }

    // aumentar frames de items y eliminar los que superen el timpo maximo de duracion
    for(uint i = 0;i<items_.size();i++){

        items_[i].frame_ = items_[i].frame_+1;
        if(items_[i].frame_ == ITEM_FRAMES-1)
            items_[i].frame_ = 0;

        items_[i].duration_ = items_[i].duration_+1;
        if(items_[i].duration_ >= ITEM_DURATION){
            items_.erase(items_.begin()+i);
            i--;
        }
    }
}

QImage Game::getScenarioImage(){

    QImage img(base_image_);

    // dibujar tiles
    QImage tile_img;
    for(uint i=0;i<scenario_size_*scenario_size_;i++){

        Tile t = scenario_.getTile(i);

        if(t.type_ == TILE_BLOCK || t.type_ == TILE_BRICK){

            if(t.type_ == TILE_BRICK)
                tile_img = QImage(":/images/brick/brick.png");

            if(t.type_ == TILE_BLOCK)
                tile_img = QImage(":/images/block/block.png");

            tile_img = tile_img.scaled(QSize(TILE_AREA,TILE_AREA));

            QPoint destPos = scenario_.getRect(i).topLeft();
            QPainter painter(&img);
            painter.drawImage(destPos, tile_img);
            painter.end();

        }
    }

    // dibujar bombas
    for(uint i=0;i<bombs_.size();i++){
        Bomb b = bombs_[i];
        QImage bomb_img = QImage(":/new/prefix/images/bomba/"+QString::number(b.frame_+1)+".png");
        bomb_img =bomb_img.scaled(BOMB_AREA_W,BOMB_AREA_H);

        QPoint destPos = b.getRect().topLeft();
        QPainter painter(&img);
        painter.drawImage(destPos, bomb_img);
        painter.end();
    }

    // dibujar jugadores
    vector<Player> players_sorted = sortPlayersByY(players_);
    for(uint i=0;i<players_sorted.size();i++){


        Player p = players_sorted[i];

        // dibujar estado
        QImage lives_img = QImage(":/images/live.png");
        lives_img = lives_img.scaled(TILE_AREA/2,TILE_AREA/2);

        QImage bomb_img = QImage(":/images/bomb.png");
        bomb_img = bomb_img.scaled(TILE_AREA/2,TILE_AREA/2);

        QImage roller_img = QImage(":/images/roller.png");
        roller_img = roller_img.scaled(TILE_AREA/2,TILE_AREA/2);

        if(p.id_ == 0){
            QPainter painter(&img);
            QPoint livespos(TILE_AREA/2,0);
            QPoint bombpos(TILE_AREA/2+TILE_AREA,0);
            QPoint rollerspos(TILE_AREA/2+TILE_AREA*2,0);
            painter.drawImage(livespos, lives_img);
            painter.drawImage(bombpos, bomb_img);
            painter.drawImage(rollerspos, roller_img);
            QPen penHLines(QColor("#FFFFFF")/*, 10, Qt::DotLine, Qt::FlatCap, Qt::RoundJoin*/);
            painter.setPen(penHLines);
            QString p1_str = "P1";
            painter.drawText(QRect(0+5,0+5,TILE_AREA/2,TILE_AREA/2),p1_str);
            QString lives_str = QString::number( p.lives_);
            painter.drawText(QRect(TILE_AREA+5,0+5,TILE_AREA/2,TILE_AREA/2),lives_str);
            QString bombs_str = QString::number( p.curr_bombs_ );
            painter.drawText(QRect(TILE_AREA*2+5,0+5,TILE_AREA/2,TILE_AREA/2),bombs_str);
            QString roller_str = QString::number( p.velocity_ );
            painter.drawText(QRect(TILE_AREA*3+5,0+5,TILE_AREA/2,TILE_AREA/2),roller_str);
        }


        if(p.lives_>0){
            QImage player_img;

            if(p.side_ == BACK_SIDE)
                player_img = QImage(":/new/prefix/images/pepe1/atras/"+QString::number(p.frame_+1)+".png");
            if(p.side_ == FRONT_SIDE)
                player_img = QImage(":/new/prefix/images/pepe1/frente/"+QString::number(p.frame_+1)+".png");
            if(p.side_ == RIGHT_SIDE)
                player_img = QImage(":/new/prefix/images/pepe1/der/"+QString::number(p.frame_+1)+".png");
            if(p.side_ == LEFT_SIDE)
                player_img = QImage(":/new/prefix/images/pepe1/izq/"+QString::number(p.frame_+1)+".png");

            player_img = player_img.scaled(PLAYER_AREA_W,PLAYER_AREA_H);

            QPoint destPos = p.position_;

            QPainter painter(&img);
            painter.drawImage(destPos, player_img);
            painter.end();

            // debug: dibujar rectangulo de jugadores
            /*QPainter r_painter(&img);
            r_painter.setBrush(Qt::NoBrush);
            r_painter.setPen(Qt::red);
            r_painter.drawRect(p.getRect().topLeft().x(),p.getRect().topLeft().y(),p.getRect().width(),p.getRect().height());
            r_painter.end();*/
        }
    }

    // dibujar partes de arriba de bricks y blocks
    for(uint i=0;i<scenario_size_*scenario_size_;i++){


        QPoint destPos = scenario_.getRect(i).topLeft();
        destPos.setY(destPos.y()-TILE_AREA+1);

        if(destPos.y()>=0){

            Tile t = scenario_.getTile(i);
            QImage tile_top_img;
            if(t.type_ == TILE_BRICK){
                tile_top_img = QImage(":/images/brick/techo.png");

                tile_top_img = tile_top_img.scaled(QSize(TILE_AREA,TILE_AREA));

                QPainter painter(&img);
                painter.drawImage(destPos, tile_top_img);
                painter.end();
            }
            if(t.type_ == TILE_BLOCK){
                tile_top_img = QImage(":/images/block/techo.png");

                tile_top_img = tile_top_img.scaled(QSize(TILE_AREA,TILE_AREA));

                QPainter painter(&img);
                painter.drawImage(destPos, tile_top_img);
                painter.end();
            }

        }

    }

    // TODO dibujar items

    // TODO dibujar explosiones
    for(uint i=0;i<explosions_.size();i++){
        ExplosionArea ea = explosions_[i];
        QRect exp_rect = scenario_.getRect(ea.position_);
        QImage exp_img;

        if(ea.type_ == EXP_CENTRAL)
            exp_img = QImage(":/new/prefix/images/exp/centro/"+QString::number(explosions_[i].frame_+1)+".png");
        if(ea.type_ == EXP_HORIZONTAL)
            exp_img = QImage(":/new/prefix/images/exp/hor_este/"+QString::number(explosions_[i].frame_+1)+".png");
        if(ea.type_ == EXP_VERTICAL)
            exp_img = QImage(":/new/prefix/images/exp/ver_norte/"+QString::number(explosions_[i].frame_+1)+".png");
        if(ea.type_ == EXP_END_UP)
            exp_img = QImage(":/new/prefix/images/exp/fin_norte/"+QString::number(explosions_[i].frame_+1)+".png");
        if(ea.type_ == EXP_END_RIGHT)
            exp_img = QImage(":/new/prefix/images/exp/fin_este/"+QString::number(explosions_[i].frame_+1)+".png");
        if(ea.type_ == EXP_END_DOWN)
            exp_img = QImage(":/new/prefix/images/exp/fin_sur/"+QString::number(explosions_[i].frame_+1)+".png");
        if(ea.type_ == EXP_END_LEFT)
            exp_img = QImage(":/new/prefix/images/exp/fin_oeste/"+QString::number(explosions_[i].frame_+1)+".png");

        exp_img = exp_img.scaled(TILE_AREA,TILE_AREA);
        QPoint destPos = exp_rect.topLeft();
        QPainter painter(&img);
        painter.drawImage(destPos, exp_img);
        painter.end();

    }

    return img;
}


vector<Player> sortPlayersByY(vector<Player> players){

    vector<Player> in = players;
    vector<Player> res;

    while(res.size()!= players.size()){

        if(in.size()>0){

            int idx = 0;
            int min_y = in[0].position_.y();

            for(uint i=1;i<in.size();i++){
                if(min_y >= in[i].position_.y()){
                    idx = i;
                    min_y = in[i].position_.y();
                }
            }

            // agregar a res ..
            res.push_back(in[idx]);

            // eliminar de in ..
            in.erase(in.begin()+idx);

        }
    }

    return res;
}


void Game::keyPressed(QString player,int key){

    for(uint i = 0;i<players_.size();i++)
        if(player == players_[i].name_ /*&& players_[i].moving_ == false*/){

            Player p = players_[i];

            players_[i].frame_ = 0;

            if(key == KEY_UP){
                players_[i].side_ = BACK_SIDE;
                players_[i].moving_ = true;
            }
            if(key == KEY_DOWN){
                players_[i].side_ = FRONT_SIDE;
                players_[i].moving_ = true;
            }
            if(key == KEY_RIGHT){
                players_[i].side_ = RIGHT_SIDE;
                players_[i].moving_ = true;
            }
            if(key == KEY_LEFT){
                players_[i].side_ = LEFT_SIDE;
                players_[i].moving_ = true;
            }

            if(key == KEY_BOMB){

                if(p.curr_bombs_>0){
                    int x_index = (p.position_.x()+PLAYER_AREA_W/2)/TILE_AREA;
                    int y_index = (p.position_.y()+PLAYER_AREA_H/2+PLAYER_AREA_H/4)/TILE_AREA;
                    int b_index =  y_index*scenario_size_+x_index;
                    Bomb new_bomb(b_index,p.bomb_duration_,p.bomb_range_,scenario_size_);
                    placeBomb(p.id_,new_bomb);
                    players_[i].curr_bombs_ = players_[i].curr_bombs_-1;
                }
            }
        }
}

void Game::keyPressed(int player,int key){

    debug("Press:"+QString::number(key)+", "+"Player: " + QString::number(player));

    for(uint i = 0;i<players_.size();i++)
        if(player == players_[i].id_ /*&& players_[i].moving_ == false*/){

            Player p = players_[i];

            players_[i].frame_ = 0;

            if(key == KEY_UP){
                players_[i].side_ = BACK_SIDE;
                players_[i].moving_ = true;
            }
            if(key == KEY_DOWN){
                players_[i].side_ = FRONT_SIDE;
                players_[i].moving_ = true;
            }
            if(key == KEY_RIGHT){
                players_[i].side_ = RIGHT_SIDE;
                players_[i].moving_ = true;
            }
            if(key == KEY_LEFT){
                players_[i].side_ = LEFT_SIDE;
                players_[i].moving_ = true;
            }

            if(key == KEY_BOMB){

                if(p.curr_bombs_>0){
                    int x_index = (p.position_.x()+PLAYER_AREA_W/2)/TILE_AREA;
                    int y_index = (p.position_.y()+PLAYER_AREA_H/2+PLAYER_AREA_H/4)/TILE_AREA;
                    int b_index =  y_index*scenario_size_+x_index;
                    Bomb new_bomb(b_index,p.bomb_duration_,p.bomb_range_,scenario_size_);
                    if(placeBomb(p.id_,new_bomb))
                        players_[i].curr_bombs_ = players_[i].curr_bombs_-1;
                }
            }
        }
}

bool Game::placeBomb(int player_id,Bomb b){

    bool can_place = true;

    // si se choca con alguna otra bomba, no colocarla
    for(uint i=0;i<bombs_.size();i++){
        Bomb bi = bombs_[i];
        if(rectanglesIntersect(b.getRect(),bi.getRect()))
            can_place = false;
    }

    // si se choca con algun jugador que no sea el que la puso, no colocarla
    for(uint i=0;i<players_.size();i++){

        Player pi = players_[i];
        if(pi.id_!=player_id){
            if(rectanglesIntersect(pi.getRect(),b.getRect()))
                can_place = false;
        }
    }

    if(can_place){
        bombs_.push_back(b);
        return true;
    }else{
        return false;
    }

}

void Game::keyReleased(QString player,int key){
    for(uint i = 0;i<players_.size();i++)
        if(player == players_[i].name_ && players_[i].moving_ == true){

            if(key == KEY_UP && players_[i].side_ == BACK_SIDE){
                players_[i].moving_ = false;
                players_[i].frame_ = false;
            }
            if(key == KEY_DOWN && players_[i].side_ == FRONT_SIDE){
                players_[i].moving_ = false;
                players_[i].frame_ = false;
            }
            if(key == KEY_RIGHT && players_[i].side_ == RIGHT_SIDE){
                players_[i].moving_ = false;
                players_[i].frame_ = false;
            }
            if(key == KEY_LEFT && players_[i].side_ == LEFT_SIDE){
                players_[i].moving_ = false;
                players_[i].frame_ = false;
            }
        }
}

void Game::keyReleased(int player,int key){

    debug("Release:"+QString::number(key)+", "+"Player: " + QString::number(player));

    for(uint i = 0;i<players_.size();i++)
        if(player == players_[i].id_ && players_[i].moving_ == true){

            if(key == KEY_UP && players_[i].side_ == BACK_SIDE){
                players_[i].moving_ = false;
                players_[i].frame_ = false;
            }
            if(key == KEY_DOWN && players_[i].side_ == FRONT_SIDE){
                players_[i].moving_ = false;
                players_[i].frame_ = false;
            }
            if(key == KEY_RIGHT && players_[i].side_ == RIGHT_SIDE){
                players_[i].moving_ = false;
                players_[i].frame_ = false;
            }
            if(key == KEY_LEFT && players_[i].side_ == LEFT_SIDE){
                players_[i].moving_ = false;
                players_[i].frame_ = false;
            }
        }
}

QPoint Game::getPlayerPos(int id){
    for(uint i = 0;i<players_.size();i++)
        if(id == players_[i].id_)
            return players_[i].position_;
    return QPoint(-1,-1);
}


//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\\
//\/\/\/\/\/\/\/\/\ PLAYER /\/\/\/\/\/\/\/\/\\
//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\\


Player::Player(int id, QPoint position, int lives, QString name){
    id_ = id;
    name_ = name;
    position_ = position;
    frame_ = 0;
    moving_ = false;
    velocity_ = 8;
    bomb_range_ = 2;
    bomb_duration_ = 90;
    lives_ = lives;
    side_ = FRONT_SIDE;
    bomb_delay_ = 200;
    curr_delay_ = 200;
    max_bombs_ = 2;
    inmunity_ = 0;
    curr_bombs_ = 0;
}

QRect Player::getRect(){

    QPoint tl(position_.x()+5,position_.y()+PLAYER_AREA_H/3*2);
    QPoint br(position_.x()+PLAYER_AREA_W-5,position_.y()+PLAYER_AREA_H);
    return QRect(tl,br);

}

//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\\
//\/\/\/\/\/\/\/\/\/ BOMB \/\/\/\/\/\/\/\/\/\\
//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\\

Bomb::Bomb(int position, int duration, int range,int scenario_size){
    position_ = position;
    duration_ = duration;
    scenario_size_ = scenario_size;
    range_ = range;
    frame_ = 0;
    time_ = 0;
}

QRect Bomb::getRect(){

    int x_index = position_%scenario_size_;
    int y_index = position_/scenario_size_;

    int x1,x2,y1,y2;
    x1 = x_index*TILE_AREA+1;
    y1 = y_index*TILE_AREA+1;
    x2 = x1+TILE_AREA-1;
    y2 = y1+TILE_AREA-1;

    QPoint tl(x1,y1);
    QPoint br(x2,y2);

    QRect b_rect(tl,br);
    return b_rect;
}

/////////////// ITEM ///////////////////

Item::Item(int type,int position,int scenario_size){
    type_ = type;
    position_ = position;
    frame_ = 0;
    duration_ = 0;
    scenario_size_ = scenario_size;
}

QRect Item::getRect(){
    int x_index = position_%scenario_size_;
    int y_index = position_/scenario_size_;

    int x1,x2,y1,y2;
    x1 = x_index*TILE_AREA+1;
    y1 = y_index*TILE_AREA+1;
    x2 = x1+TILE_AREA-1;
    y2 = y1+TILE_AREA-1;

    QPoint tl(x1,y1);
    QPoint br(x2,y2);

    QRect b_rect(tl,br);
    return b_rect;
}
