/*
** EPITECH PROJECT, 2023
** guitare_heror_game_jam
** File description:
** play
*/

#include <iostream>
#include <algorithm>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <time.h>

using namespace sf;
using namespace std;

const int WIDTH = 10;
const int HEIGHT = 25;
const int FRAMERATE = 75;

#define Z_TETROMINO 0
#define L_TETROMINO 1
#define O_TETROMINO 2
#define S_TETROMINO 3
#define I_TETROMINO 4
#define J_TETROMINO 5
#define T_TETROMINO 6

int board[HEIGHT][WIDTH] = {0};
int board_bis[HEIGHT][WIDTH] = {0};

vector <int> generateNewBag() {

    vector <int> template_bag;

    for(int i = 0; i < 7; i++) {

        template_bag.push_back(i);
    }

    vector <int> new_bag;

    while(template_bag.size() > 0) {

        int index = rand() % template_bag.size();
        int choose_piece = template_bag.at(index);
        template_bag.erase(template_bag.begin() + index);
        new_bag.push_back(choose_piece);
    }

    return new_bag;
}

class Piece {

    public :

    int x;
    int y;
    int rotation = 1;

    void Rotate(Piece center, int direction) {

        int x = this->y - center.y;
        int y = this->x - center.x;
        this->x = center.x + (x * -direction);
        this->y = center.y + (y * direction);

        this->rotation += direction;

        if(this->rotation > 4) this->rotation = 1;
        if(this->rotation < 1) this->rotation = 4;
    }
};

Piece piece[4], cache[4], ghost[4];
Piece piece_bis[4], cache_bis[4], ghost_bis[4];

int pieces[7][4] = {
    {3, 4, 5, 6},
    {2, 4, 6, 7},
    {2, 3, 4, 5},
    {2, 4, 5, 7},
    {1, 3, 5, 7},
    {3, 5, 6, 7},
    {2, 4, 5, 6}
};

class PieceLock {

    public :

    int x;
    int y;
    double animation = 0;
};
vector <vector <PieceLock>> pieces_lock;
vector <vector <PieceLock>> pieces_lock_bis;

class Particle {

    public : Particle(int x, int y, int speed, int direction) {

        this->x = x;
        this->y = y;
        this->direction = direction;
        this->speed = speed;
    }

    double x;
    double y;
    int direction;
    double speed;
    double alpha = 255;
    RectangleShape shape;

    void update(double time_per_frame) {

        double angle = this->direction * (M_PI / 180);
        this->x += this->speed * cos(angle) * time_per_frame;
        this->y += this->speed * sin(angle) * time_per_frame;
        this->alpha -= 400 * time_per_frame;
        this->speed += 100 * ((-this->speed) / 15) * time_per_frame;
    }

    void draw(RenderWindow *window) {

        this->shape.setSize(Vector2f(2, 2));
        this->shape.setFillColor(Color(255, 255, 255, max((double)0, (double)this->alpha)));
        this->shape.setPosition(this->x, this->y);
        window->draw(this->shape);
    }
};
vector <Particle> particles;
vector <Particle> particles_bis;

int sign(int num) {

    if(num < 0) return -1;
    else if(num > 0) return 1;
    else return 0;
}

void createParticle(vector <Particle> *particles) {

    for(int i = 0; i < 4; i++) {

        Particle particle((piece[i].x * 30) + 150 + 15 + (rand() % 60 - 30), (piece[i].y * 30) - 60 - 30, rand() % 250 + 150, 270 + (rand() % 60 - 30));
        particles->push_back(particle);
    }
}

bool isCollidedGhost() {

    for(int i = 0; i < 4; i++) {

        if(ghost[i].y >= HEIGHT) return false;
        else if(board[ghost[i].y][ghost[i].x]) return false;

        if(ghost_bis[i].y >= HEIGHT) return false;
        else if(board_bis[ghost_bis[i].y][ghost_bis[i].x]) return false;
    }
    return 1;
}

bool isCollided() {

    for(int i = 0; i < 4; i++) {

        if(piece[i].x < 0 || piece[i].x >= WIDTH || piece[i].y >= HEIGHT) return false;
        else if(board[piece[i].y][piece[i].x]) return false;

        if(piece_bis[i].x < 0 || piece_bis[i].x >= WIDTH || piece_bis[i].y >= HEIGHT) return false;
        else if(board_bis[piece_bis[i].y][piece_bis[i].x]) return false;
    }
    return 1;
}

bool isDead() {

    for(int i = 0; i < 4; i++) if(board[piece[i].y][piece[i].x]) return true;
    for(int i = 0; i < 4; i++) if(board_bis[piece_bis[i].y][piece_bis[i].x]) return true;
    return false;
}

int main() {

    srand(time(0));

    RenderWindow window(VideoMode(1300, 800), "TETRIS!", Style::Close);

    //Sound
    Sound sfx_music; SoundBuffer buffer_music;
    buffer_music.loadFromFile("Music/music.ogg");
    sfx_music.setBuffer(buffer_music);

    //Pieces
    Texture t;
    t.loadFromFile("Sprites/jstris1.png");
    int texture_size = 30;
    Sprite s(t);
    s.setTextureRect(IntRect(0, 0, texture_size, texture_size));

    //Ghost
    Texture t_ghost;
    t_ghost.loadFromFile("Sprites/ghost.png");
    Sprite s_ghost(t_ghost);
    s_ghost.setTextureRect(IntRect(0, 0, texture_size, texture_size));

    //Board
    Texture board_t;
    board_t.loadFromFile("Sprites/board.png");
    Sprite board_s(board_t);

    Texture board_bis_t;
    board_bis_t.loadFromFile("Sprites/board.png");
    Sprite board_bis_s(board_t);

    int hold;
    int move_x;
    int rotate;
    int color;
    int color_bis;
    int harddrop;
    int holded;
    int move_left;
    int move_right;
    int start;

    Clock clock;
    Clock game_clock;
    Clock game_elapsed_time;

    double timer;
    double delay;
    vector <int> seven_bag;
    vector <int> seven_bag_next;

    restart :

    //Handling control
    double das = 8;
    double lock_delay = 0.5;
    double fastdrop_delay = 0;
    int lock_count = 14;
    int is_touch_ground = 0;

    int all_line_count = 0;
    int all_piece_count = 0;

    int lock_count_value = lock_count;
    double das_value = das;
    double lock_delay_value = lock_delay;

    for(int i = 0; i < HEIGHT; i++)
        for(int j = 0; j < WIDTH; j++)
            board[i][j] = 0;

    for(int i = 0; i < HEIGHT; i++)
        for(int j = 0; j < WIDTH; j++)
            board_bis[i][j] = 0;

    start = 3;
    timer = 0;
    delay = 0.5;
    move_x = 0;
    color = 0;
    harddrop = 0;
    rotate = 0;
    hold = -1;
    move_left = 0;
    move_right = 0;
    holded = 0;
    seven_bag = generateNewBag();
    seven_bag_next = generateNewBag();

    double piece_indicator_shape_alpha = 0;
    double piece_indicator_shape_alpha_bis = 0;
    double board_wobble = 0;

    int line_clear_combo = 0;
    int btb_combo = 0;

    //Choose First Piece
    int choose_piece = seven_bag.at(0);
    int choose_piece_bis = seven_bag.at(0);
    seven_bag.erase(seven_bag.begin());

    color = choose_piece;
    color_bis = choose_piece_bis;
    for(int i = 0; i < 4; i++) {

        piece[i].x = pieces[choose_piece][i] % 2 + 4;
        piece[i].y = pieces[choose_piece][i] / 2 + 3;
        piece[i].rotation = 0;

        if(color == J_TETROMINO) piece[i].x --;

        piece_bis[i].x = pieces[choose_piece_bis][i] % 2 + 4;
        piece_bis[i].y = pieces[choose_piece_bis][i] / 2 + 3;
        piece_bis[i].rotation = 0;

        if(color_bis == J_TETROMINO) piece_bis[i].x --;
    }
    for(int i = 0; i < 4; i++) {

        Piece center = piece[1];
        if(color == I_TETROMINO) piece[i].Rotate(center, 1);
        if(color == T_TETROMINO) piece[i].Rotate(center, -1);
        if(color == S_TETROMINO) piece[i].Rotate(center, -1);
        if(color == Z_TETROMINO) piece[i].Rotate(center, -1);
        if(color == L_TETROMINO) piece[i].Rotate(center, -1);
        if(color == J_TETROMINO) piece[i].Rotate(center, 1);

        Piece center_bis = piece_bis[1];
        if(color_bis == I_TETROMINO) piece[i].Rotate(center_bis, 1);
        if(color_bis == T_TETROMINO) piece[i].Rotate(center_bis, -1);
        if(color_bis == S_TETROMINO) piece[i].Rotate(center_bis, -1);
        if(color_bis == Z_TETROMINO) piece[i].Rotate(center_bis, -1);
        if(color_bis == L_TETROMINO) piece[i].Rotate(center_bis, -1);
        if(color_bis == J_TETROMINO) piece[i].Rotate(center_bis, 1);
    }

    int is_rotate_cw = 0;
    int is_rotate_ccw = 0;
    int is_harddrop = 0;
    int is_right = 0;
    int is_left = 0;
    int is_hold = 0;
    int is_sidehit = 0;
    int is_tspin = 0;
    int is_restart = 0;

    int key_rotate_cw = 0;
    int key_rotate_ccw = 0;
    int key_harddrop = 0;
    int key_right = 0;
    int key_left = 0;
    int key_right_hold = 0;
    int key_left_hold = 0;
    bool is_on_focus = true;
    int last_key = 0;
    int key_restart = 0;

    sfx_music.play();

    while(window.isOpen()) {

        float time = clock.getElapsedTime().asSeconds();

        for(int i = 0; i < 4; i++) {
            cache[i] = piece[i];
            piece[i].y ++;
            cache_bis[i] = piece_bis[i];
            piece_bis[i].y ++;
        }
        if(!isCollided()) {
            lock_delay_value -= time;

            if(lock_delay_value <= 0) {

                for(int i = 0; i < 4; i++) piece[i] = cache[i];
                for(int i = 0; i < 4; i++) piece_bis[i] = cache_bis[i];
                goto makeNewPiece;
            }
            if(is_touch_ground == 0) {

                is_touch_ground = 1;
            }
        }
        else {

            is_touch_ground = 0;
        }
        for(int i = 0; i < 4; i++) piece[i] = cache[i];
        for(int i = 0; i < 4; i++) piece_bis[i] = cache_bis[i];

        timer += time;
        clock.restart();

        Event e;
        while(window.pollEvent(e)) {

            if(e.type == Event::GainedFocus) is_on_focus = true;
            if(e.type == Event::LostFocus) is_on_focus = false;

            if(is_on_focus && e.type == Event::KeyReleased) {

                if(e.key.code == Keyboard::X || e.key.code == Keyboard::Up) is_rotate_cw = 0;
                if(e.key.code == Keyboard::Z) is_rotate_ccw = 0;
                if(e.key.code == Keyboard::Space) is_harddrop = 0;
                if(e.key.code == Keyboard::C) is_hold = 0;
                if(e.key.code == Keyboard::Left) is_left = 0;
                if(e.key.code == Keyboard::Right) is_right = 0;
                if(e.key.code == Keyboard::R) is_restart = 0;
            }

            if(e.type == Event::Closed) {

                window.close();
            }
        }

        if(is_on_focus) {

            //Key Fixed
            if(Keyboard::isKeyPressed(Keyboard::C) && is_hold == 0) {

                is_hold = -1;
                holded = 1;
            }
            if((Keyboard::isKeyPressed(Keyboard::X) || Keyboard::isKeyPressed(Keyboard::Up)) && is_rotate_cw == 0) {

                is_rotate_cw = -1;
                rotate = 1;
            }
            if(Keyboard::isKeyPressed(Keyboard::Z) && is_rotate_ccw == 0) {

                is_rotate_ccw = -1;
                rotate = -1;
            }
            if(Keyboard::isKeyPressed(Keyboard::Space) && is_harddrop == 0) {

                is_harddrop = -1;
                harddrop = 1;
            }
            if(Keyboard::isKeyPressed(Keyboard::Left) && is_left == 0) {

                is_left = -1;
                move_left = 1;
            }
            if(Keyboard::isKeyPressed(Keyboard::Right) && is_right == 0) {

                is_right = -1;
                move_right = 1;
            }
            if(Keyboard::isKeyPressed(Keyboard::R) && is_restart == 0) {

                is_restart = -1;
                goto restart;
            }

            //Dead
            if(isDead()) {

                goto restart;
            }

            if(Keyboard::isKeyPressed(Keyboard::Down)) delay = fastdrop_delay;

            //========================= Move =========================
            key_left = Keyboard::isKeyPressed(Keyboard::Left);
            key_right = Keyboard::isKeyPressed(Keyboard::Right);
            if(!key_left && !key_right) das_value = das;

            if(move_right) {

                for(int i = 0; i < 4; i++) {

                    cache[i] = piece[i];
                    piece[i].x += 1;
                    cache_bis[i] = piece_bis[i];
                    piece_bis[i].x += 1;
                }

                if(!isCollided()) for(int i = 0; i < 4; i++) piece[i] = cache[i];
                if(!isCollided()) for(int i = 0; i < 4; i++) piece_bis[i] = cache_bis[i];
                else {
                    if(lock_count_value > 0) {
                        lock_count_value--;
                        lock_delay_value = lock_delay;
                    }
                }
                das_value = das;
            }
            if(move_left) {
                for(int i = 0; i < 4; i++) {
                    cache[i] = piece[i];
                    piece[i].x -= 1;
                    cache_bis[i] = piece_bis[i];
                    piece_bis[i].x -= 1;
                }
                if(!isCollided()) for(int i = 0; i < 4; i++) piece[i] = cache[i];
                if(!isCollided()) for(int i = 0; i < 4; i++) piece_bis[i] = cache_bis[i];
                else {
                    if(lock_count_value > 0) {
                        lock_count_value--;
                        lock_delay_value = lock_delay;
                    }
                }
                das_value = das;
            }

            if(das_value <= 0) {
                for(int i = 0; i < 4; i++) {
                    cache[i] = piece[i];
                    piece[i].x += sign(key_right - key_left);
                    cache_bis[i] = piece_bis[i];
                    piece_bis[i].x += sign(key_right - key_left);
                }
                if(!isCollided()) {
                    if(is_sidehit == 0) {
                        is_sidehit = 1;
                    }
                    for(int i = 0; i < 4; i++) piece[i] = cache[i];
                    for(int i = 0; i < 4; i++) piece_bis[i] = cache_bis[i];
                }
                else {
                    is_sidehit = 0;
                    if(lock_count_value > 0) {
                        lock_count_value--;
                        lock_delay_value = lock_delay;
                    }
                }
            }
            das_value -= clock.getElapsedTime().asSeconds() * 75;
            //Hold
            if(holded) {
                if(hold == -1) {
                    hold = color;
                    color = -1;
                    while(isCollided()) {
                        for(int i = 0; i < 4; i++) piece[i].y ++;
                        for(int i = 0; i < 4; i++) piece_bis[i].y ++;
                    }
                    for(int i = 0; i < 4; i++) piece[i].y --;
                    for(int i = 0; i < 4; i++) piece_bis[i].y --;
                    lock_delay_value = -1;
                    timer = 0;
                    goto makeNewPiece;
                }
                else {
                    int temp;
                    temp = color;
                    color = hold;
                    hold = temp;
                    timer = 0;
                    lock_count_value = lock_count;
                    lock_delay_value = lock_delay;
                    choose_piece = color;
                    for(int i = 0; i < 4; i++) {
                        piece[i].x = pieces[choose_piece][i] % 2 + 4;
                        piece[i].y = pieces[choose_piece][i] / 2 + 2;
                        piece[i].rotation = 0;

                        if(color == J_TETROMINO) piece[i].x --;
                        if(color == I_TETROMINO) piece[i].y ++;

                        piece_bis[i].x = pieces[choose_piece_bis][i] % 2 + 4;
                        piece_bis[i].y = pieces[choose_piece_bis][i] / 2 + 2;
                        piece_bis[i].rotation = 0;

                        if(color == J_TETROMINO) piece_bis[i].x --;
                        if(color == I_TETROMINO) piece_bis[i].y ++;
                    }
                    for(int i = 0; i < 4; i++) {

                        Piece center = piece[1];
                        if(color == I_TETROMINO) piece[i].Rotate(center, 1);
                        if(color == T_TETROMINO) piece[i].Rotate(center, -1);
                        if(color == S_TETROMINO) piece[i].Rotate(center, -1);
                        if(color == Z_TETROMINO) piece[i].Rotate(center, -1);
                        if(color == L_TETROMINO) piece[i].Rotate(center, -1);
                        if(color == J_TETROMINO) piece[i].Rotate(center, 1);
                        piece[i].rotation = 1;

                        Piece center_bis = piece_bis[1];
                        if(color_bis == I_TETROMINO) piece_bis[i].Rotate(center_bis, 1);
                        if(color_bis == T_TETROMINO) piece_bis[i].Rotate(center_bis, -1);
                        if(color_bis == S_TETROMINO) piece_bis[i].Rotate(center_bis, -1);
                        if(color_bis == Z_TETROMINO) piece_bis[i].Rotate(center_bis, -1);
                        if(color_bis == L_TETROMINO) piece_bis[i].Rotate(center_bis, -1);
                        if(color_bis == J_TETROMINO) piece_bis[i].Rotate(center_bis, 1);
                        piece_bis[i].rotation = 1;
                    }
                }
            }

            //HardDrop
            if(harddrop && start <= 0) {
                while(isCollided()) {
                    for(int i = 0; i < 4; i++) piece[i].y ++;
                    for(int i = 0; i < 4; i++) piece_bis[i].y ++;
                    createParticle(&particles);
                }
                for(int i = 0; i < 4; i++) piece[i].y --;
                for(int i = 0; i < 4; i++) piece_bis[i].y --;
                lock_delay_value = -1;
                goto makeNewPiece;
            }

            //========================= Rotate =========================
            if(rotate != 0) {

                int before_rotation, after_rotation;
                int before_rotation_bis, after_rotation_bis;
                for(int i = 0; i < 4; i++) {

                    cache[i] = piece[i];
                    before_rotation = piece[i].rotation;
                    cache_bis[i] = piece_bis[i];
                    before_rotation_bis = piece_bis[i].rotation;
                    if(color == I_TETROMINO) {

                        if(piece[i].rotation == 1) {

                            if(rotate == 1) {
                                if(i == 0) { piece[i].x-=1, piece[i].y-=1; };
                                if(i == 2) { piece[i].x+=1, piece[i].y+=1; };
                                if(i == 3) { piece[i].x+=2, piece[i].y+=2; };
                            }
                            else {
                                if(i == 0) { piece[i].x-=2, piece[i].y+=2; };
                                if(i == 1) { piece[i].x-=1, piece[i].y+=1; };
                                if(i == 3) { piece[i].x+=1, piece[i].y-=1; };
                            }
                        }
                        if(piece[i].rotation == 2) {

                            if(rotate == 1) {
                                if(i == 0) { piece[i].x-=2, piece[i].y+=2; };
                                if(i == 1) { piece[i].x-=1, piece[i].y+=1; };
                                if(i == 3) { piece[i].x+=1, piece[i].y-=1; };
                            }
                            else {
                                if(i == 0) { piece[i].x+=1, piece[i].y+=1; };
                                if(i == 2) { piece[i].x-=1, piece[i].y-=1; };
                                if(i == 3) { piece[i].x-=2, piece[i].y-=2; };
                            }
                        }
                        if(piece[i].rotation == 3) {

                            if(rotate == 1) {
                                if(i == 0) { piece[i].x+=1, piece[i].y+=1; };
                                if(i == 2) { piece[i].x-=1, piece[i].y-=1; };
                                if(i == 3) { piece[i].x-=2, piece[i].y-=2; };
                            }
                            else {
                                if(i == 0) { piece[i].x+=2, piece[i].y-=2; };
                                if(i == 1) { piece[i].x+=1, piece[i].y-=1; };
                                if(i == 3) { piece[i].x-=1, piece[i].y+=1; };
                            }
                        }
                        if(piece[i].rotation == 4) {

                            if(rotate == 1) {
                                if(i == 0) { piece[i].x+=2, piece[i].y-=2; };
                                if(i == 1) { piece[i].x+=1, piece[i].y-=1; };
                                if(i == 3) { piece[i].x-=1, piece[i].y+=1; };
                            }
                            else {
                                if(i == 0) { piece[i].x-=1, piece[i].y-=1; };
                                if(i == 2) { piece[i].x+=1, piece[i].y+=1; };
                                if(i == 3) { piece[i].x+=2, piece[i].y+=2; };
                            }
                        }
                        piece[i].rotation += rotate;
                    }
                    if(color_bis == I_TETROMINO) {

                        if(piece_bis[i].rotation == 1) {

                            if(rotate == 1) {
                                if(i == 0) { piece_bis[i].x-=1, piece_bis[i].y-=1; };
                                if(i == 2) { piece_bis[i].x+=1, piece_bis[i].y+=1; };
                                if(i == 3) { piece_bis[i].x+=2, piece_bis[i].y+=2; };
                            }
                            else {
                                if(i == 0) { piece_bis[i].x-=2, piece_bis[i].y+=2; };
                                if(i == 1) { piece_bis[i].x-=1, piece_bis[i].y+=1; };
                                if(i == 3) { piece_bis[i].x+=1, piece_bis[i].y-=1; };
                            }
                        }
                        if(piece_bis[i].rotation == 2) {

                            if(rotate == 1) {
                                if(i == 0) { piece_bis[i].x-=2, piece_bis[i].y+=2; };
                                if(i == 1) { piece_bis[i].x-=1, piece_bis[i].y+=1; };
                                if(i == 3) { piece_bis[i].x+=1, piece_bis[i].y-=1; };
                            }
                            else {
                                if(i == 0) { piece_bis[i].x+=1, piece_bis[i].y+=1; };
                                if(i == 2) { piece_bis[i].x-=1, piece_bis[i].y-=1; };
                                if(i == 3) { piece_bis[i].x-=2, piece_bis[i].y-=2; };
                            }
                        }
                        if(piece_bis[i].rotation == 3) {

                            if(rotate == 1) {
                                if(i == 0) { piece_bis[i].x+=1, piece_bis[i].y+=1; };
                                if(i == 2) { piece_bis[i].x-=1, piece_bis[i].y-=1; };
                                if(i == 3) { piece_bis[i].x-=2, piece_bis[i].y-=2; };
                            }
                            else {
                                if(i == 0) { piece_bis[i].x+=2, piece_bis[i].y-=2; };
                                if(i == 1) { piece_bis[i].x+=1, piece_bis[i].y-=1; };
                                if(i == 3) { piece_bis[i].x-=1, piece_bis[i].y+=1; };
                            }
                        }
                        if(piece_bis[i].rotation == 4) {

                            if(rotate == 1) {
                                if(i == 0) { piece_bis[i].x+=2, piece_bis[i].y-=2; };
                                if(i == 1) { piece_bis[i].x+=1, piece_bis[i].y-=1; };
                                if(i == 3) { piece_bis[i].x-=1, piece_bis[i].y+=1; };
                            }
                            else {
                                if(i == 0) { piece_bis[i].x-=1, piece_bis[i].y-=1; };
                                if(i == 2) { piece_bis[i].x+=1, piece_bis[i].y+=1; };
                                if(i == 3) { piece_bis[i].x+=2, piece_bis[i].y+=2; };
                            }
                        }
                        piece_bis[i].rotation += rotate;
                    }
                    else if(color != O_TETROMINO) piece[i].Rotate(piece[1], rotate);
                    else if(color_bis != O_TETROMINO) piece_bis[i].Rotate(piece_bis[1], rotate);

                    if(piece[i].rotation > 4) piece[i].rotation = 1;
                    if(piece[i].rotation < 1) piece[i].rotation = 4;

                    if(piece_bis[i].rotation > 4) piece_bis[i].rotation = 1;
                    if(piece_bis[i].rotation < 1) piece_bis[i].rotation = 4;

                    after_rotation = piece[i].rotation;
                    after_rotation_bis = piece_bis[i].rotation;
                }

                Piece rotation_piece_bis[4];
                if(color_bis != I_TETROMINO) {

                    if((before_rotation_bis == 1 && after_rotation_bis == 2) || (before_rotation_bis == 3 && after_rotation_bis == 2)) { //1 >> 2 & 3 >> 2
                        //TEST 2
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                rotation_piece_bis[i].x = piece_bis[i].x;
                                rotation_piece_bis[i].y = piece_bis[i].y;
                                piece_bis[i].x += -1;
                                piece_bis[i].y += 0;
                            }
                        }
                        //TEST 3
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece_bis[i].x = rotation_piece_bis[i].x;
                                piece_bis[i].y = rotation_piece_bis[i].y;
                                piece_bis[i].x += -1;
                                piece_bis[i].y += -1;
                            }
                        }
                        //TEST 4
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece_bis[i].x = rotation_piece_bis[i].x;
                                piece_bis[i].y = rotation_piece_bis[i].y;
                                piece_bis[i].x += 0;
                                piece_bis[i].y += 2;
                            }
                        }
                        //TEST 5
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece_bis[i].x = rotation_piece_bis[i].x;
                                piece_bis[i].y = rotation_piece_bis[i].y;
                                piece_bis[i].x += -1;
                                piece_bis[i].y += 2;
                            }
                        }
                    }
                    if((before_rotation_bis == 2 && after_rotation_bis == 1) || (before_rotation_bis == 2 && after_rotation_bis == 3)) { //2 >> 1 && 2 >> 3
                        //TEST 2
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                rotation_piece_bis[i].x = piece_bis[i].x;
                                rotation_piece_bis[i].y = piece_bis[i].y;
                                piece_bis[i].x += 1;
                                piece_bis[i].y += 0;
                            }
                        }
                        //TEST 3
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece_bis[i].x = rotation_piece_bis[i].x;
                                piece_bis[i].y = rotation_piece_bis[i].y;
                                piece_bis[i].x += 1;
                                piece_bis[i].y += 1;
                            }
                        }
                        //TEST 4
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece_bis[i].x = rotation_piece_bis[i].x;
                                piece_bis[i].y = rotation_piece_bis[i].y;
                                piece_bis[i].x += 0;
                                piece_bis[i].y += -2;
                            }
                        }
                        //TEST 5
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece_bis[i].x = rotation_piece_bis[i].x;
                                piece_bis[i].y = rotation_piece_bis[i].y;
                                piece_bis[i].x += 1;
                                piece_bis[i].y += -2;
                            }
                        }
                    }
                    if((before_rotation_bis == 3 && after_rotation_bis == 4) || (before_rotation_bis == 1 && after_rotation_bis == 4)) { //3 >> 4 & 1 >> 4
                        //TEST 2
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                rotation_piece_bis[i].x = piece_bis[i].x;
                                rotation_piece_bis[i].y = piece_bis[i].y;
                                piece_bis[i].x += 1;
                                piece_bis[i].y += 0;
                            }
                        }
                        //TEST 3
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece_bis[i].x = rotation_piece_bis[i].x;
                                piece_bis[i].y = rotation_piece_bis[i].y;
                                piece_bis[i].x += 1;
                                piece_bis[i].y += -1;
                            }
                        }
                        //TEST 4
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece_bis[i].x = rotation_piece_bis[i].x;
                                piece_bis[i].y = rotation_piece_bis[i].y;
                                piece_bis[i].x += 0;
                                piece_bis[i].y += 2;
                            }
                        }
                        //TEST 5
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece_bis[i].x = rotation_piece_bis[i].x;
                                piece_bis[i].y = rotation_piece_bis[i].y;
                                piece_bis[i].x += 1;
                                piece_bis[i].y += 2;
                            }
                        }
                    }
                    if((before_rotation_bis == 4 && after_rotation_bis == 3) || (before_rotation_bis == 4 && after_rotation_bis == 1)) { //4 >> 3 && 4 >> 1
                        //TEST 2
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                rotation_piece_bis[i].x = piece_bis[i].x;
                                rotation_piece_bis[i].y = piece_bis[i].y;
                                piece_bis[i].x += -1;
                                piece_bis[i].y += 0;
                            }
                        }
                        //TEST 3
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece_bis[i].x = rotation_piece_bis[i].x;
                                piece_bis[i].y = rotation_piece_bis[i].y;
                                piece_bis[i].x += -1;
                                piece_bis[i].y += 1;
                            }
                        }
                        //TEST 4
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece_bis[i].x = rotation_piece_bis[i].x;
                                piece_bis[i].y = rotation_piece_bis[i].y;
                                piece_bis[i].x += 0;
                                piece_bis[i].y += -2;
                            }
                        }
                        //TEST 5
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece_bis[i].x = rotation_piece_bis[i].x;
                                piece_bis[i].y = rotation_piece_bis[i].y;
                                piece_bis[i].x += -1;
                                piece_bis[i].y += -2;
                            }
                        }
                    }
                }

                Piece rotation_piece[4];
                if(color != I_TETROMINO) {

                    if((before_rotation == 1 && after_rotation == 2) || (before_rotation == 3 && after_rotation == 2)) { //1 >> 2 & 3 >> 2
                        //TEST 2
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                rotation_piece[i].x = piece[i].x;
                                rotation_piece[i].y = piece[i].y;
                                piece[i].x += -1;
                                piece[i].y += 0;
                            }
                        }
                        //TEST 3
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece[i].x = rotation_piece[i].x;
                                piece[i].y = rotation_piece[i].y;
                                piece[i].x += -1;
                                piece[i].y += -1;
                            }
                        }
                        //TEST 4
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece[i].x = rotation_piece[i].x;
                                piece[i].y = rotation_piece[i].y;
                                piece[i].x += 0;
                                piece[i].y += 2;
                            }
                        }
                        //TEST 5
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece[i].x = rotation_piece[i].x;
                                piece[i].y = rotation_piece[i].y;
                                piece[i].x += -1;
                                piece[i].y += 2;
                            }
                        }
                    }
                    if((before_rotation == 2 && after_rotation == 1) || (before_rotation == 2 && after_rotation == 3)) { //2 >> 1 && 2 >> 3
                        //TEST 2
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                rotation_piece[i].x = piece[i].x;
                                rotation_piece[i].y = piece[i].y;
                                piece[i].x += 1;
                                piece[i].y += 0;
                            }
                        }
                        //TEST 3
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece[i].x = rotation_piece[i].x;
                                piece[i].y = rotation_piece[i].y;
                                piece[i].x += 1;
                                piece[i].y += 1;
                            }
                        }
                        //TEST 4
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece[i].x = rotation_piece[i].x;
                                piece[i].y = rotation_piece[i].y;
                                piece[i].x += 0;
                                piece[i].y += -2;
                            }
                        }
                        //TEST 5
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece[i].x = rotation_piece[i].x;
                                piece[i].y = rotation_piece[i].y;
                                piece[i].x += 1;
                                piece[i].y += -2;
                            }
                        }
                    }
                    if((before_rotation == 3 && after_rotation == 4) || (before_rotation == 1 && after_rotation == 4)) { //3 >> 4 & 1 >> 4
                        //TEST 2
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                rotation_piece[i].x = piece[i].x;
                                rotation_piece[i].y = piece[i].y;
                                piece[i].x += 1;
                                piece[i].y += 0;
                            }
                        }
                        //TEST 3
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece[i].x = rotation_piece[i].x;
                                piece[i].y = rotation_piece[i].y;
                                piece[i].x += 1;
                                piece[i].y += -1;
                            }
                        }
                        //TEST 4
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece[i].x = rotation_piece[i].x;
                                piece[i].y = rotation_piece[i].y;
                                piece[i].x += 0;
                                piece[i].y += 2;
                            }
                        }
                        //TEST 5
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece[i].x = rotation_piece[i].x;
                                piece[i].y = rotation_piece[i].y;
                                piece[i].x += 1;
                                piece[i].y += 2;
                            }
                        }
                    }
                    if((before_rotation == 4 && after_rotation == 3) || (before_rotation == 4 && after_rotation == 1)) { //4 >> 3 && 4 >> 1
                        //TEST 2
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                rotation_piece[i].x = piece[i].x;
                                rotation_piece[i].y = piece[i].y;
                                piece[i].x += -1;
                                piece[i].y += 0;
                            }
                        }
                        //TEST 3
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece[i].x = rotation_piece[i].x;
                                piece[i].y = rotation_piece[i].y;
                                piece[i].x += -1;
                                piece[i].y += 1;
                            }
                        }
                        //TEST 4
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece[i].x = rotation_piece[i].x;
                                piece[i].y = rotation_piece[i].y;
                                piece[i].x += 0;
                                piece[i].y += -2;
                            }
                        }
                        //TEST 5
                        if(!isCollided()) {
                            for(int i = 0; i < 4; i++) {

                                piece[i].x = rotation_piece[i].x;
                                piece[i].y = rotation_piece[i].y;
                                piece[i].x += -1;
                                piece[i].y += -2;
                            }
                        }
                    }
                }

                if(!isCollided()) for(int i = 0; i < 4; i++) piece[i] = cache[i];
                if(!isCollided()) for(int i = 0; i < 4; i++) piece_bis[i] = cache_bis[i];
                else {

                    if(lock_count_value > 0) {

                        lock_count_value--;
                        lock_delay_value = lock_delay;
                    }
                    if(color == T_TETROMINO) {

                        int corner_count = 0;
                        if(board[piece[1].y + 1][piece[1].x + 1] != 0 || piece[1].x + 1 >= WIDTH || piece[1].y + 1 >= HEIGHT) corner_count ++;
                        if(board[piece[1].y + 1][piece[1].x - 1] != 0 || piece[1].x - 1 < 0 || piece[1].y + 1 >= HEIGHT) corner_count ++;
                        if(board[piece[1].y - 1][piece[1].x + 1] != 0 || piece[1].x + 1 >= WIDTH || piece[1].y - 1 < 0) corner_count ++;
                        if(board[piece[1].y - 1][piece[1].x - 1] != 0 || piece[1].x - 1 < 0 || piece[1].y + 1 < 0) corner_count ++;

                        if(corner_count >= 3) {
                            is_tspin = 1;
                        }
                    }
                    if(color_bis == T_TETROMINO) {

                        int corner_count_bis = 0;
                        if(board_bis[piece_bis[1].y + 1][piece_bis[1].x + 1] != 0 || piece_bis[1].x + 1 >= WIDTH || piece_bis[1].y + 1 >= HEIGHT) corner_count_bis ++;
                        if(board_bis[piece_bis[1].y + 1][piece_bis[1].x - 1] != 0 || piece_bis[1].x - 1 < 0 || piece_bis[1].y + 1 >= HEIGHT) corner_count_bis ++;
                        if(board_bis[piece_bis[1].y - 1][piece_bis[1].x + 1] != 0 || piece_bis[1].x + 1 >= WIDTH || piece_bis[1].y - 1 < 0) corner_count_bis ++;
                        if(board_bis[piece_bis[1].y - 1][piece_bis[1].x - 1] != 0 || piece_bis[1].x - 1 < 0 || piece_bis[1].y + 1 < 0) corner_count_bis ++;

                        if(corner_count_bis >= 3) {
                            is_tspin = 1;
                        }
                    }
                }
            }

            //Game Update
            if(timer > delay) {

                makeNewPiece :

                for(int i = 0; i < 4; i++) {
                    cache[i] = piece[i];
                    piece[i].y ++;
                    cache_bis[i] = piece_bis[i];
                    piece_bis[i].y ++;
                }

                if(!isCollided() && lock_delay_value < 0) {

                    if(!holded) {

                        createParticle(&particles);
                        createParticle(&particles);
                        createParticle(&particles);

                        vector <PieceLock> piece_lock;
                        vector <PieceLock> piece_lock_bis;
                        for(int i = 0; i < 4; i++) {

                            PieceLock piece_lock_one;
                            piece_lock_one.x = piece[i].x;
                            piece_lock_one.y = piece[i].y;

                            piece_lock.push_back(piece_lock_one);

                            PieceLock piece_lock_one_bis;
                            piece_lock_one_bis.x = piece_bis[i].x;
                            piece_lock_one_bis.y = piece_bis[i].y;

                            piece_lock_bis.push_back(piece_lock_one_bis);
                        }
                        pieces_lock.push_back(piece_lock);
                        pieces_lock_bis.push_back(piece_lock_bis);

                        board_wobble = 7;
                        all_piece_count ++;
                    }

                    if(!harddrop && !holded) {
                    }

                    for(int i = 0; i < 4; i++) board[cache[i].y][cache[i].x] = color + 1;
                    for(int i = 0; i < 4; i++) board_bis[cache_bis[i].y][cache_bis[i].x] = color_bis + 1;
                    int choose_piece = seven_bag.at(0);
                    seven_bag.erase(seven_bag.begin());
                    if(seven_bag.size() == 0) {
                        seven_bag = seven_bag_next;
                        seven_bag_next = generateNewBag();
                    }

                    color = choose_piece;
                    for(int i = 0; i < 4; i++) {

                        piece[i].x = pieces[choose_piece][i] % 2 + 4;
                        piece[i].y = pieces[choose_piece][i] / 2 + 2;
                        piece[i].rotation = 0;

                        if(color == J_TETROMINO) piece[i].x --;
                        if(color == I_TETROMINO) piece[i].y ++;

                        piece_bis[i].x = pieces[choose_piece_bis][i] % 2 + 4;
                        piece_bis[i].y = pieces[choose_piece_bis][i] / 2 + 2;
                        piece_bis[i].rotation = 0;

                        if(color_bis == J_TETROMINO) piece_bis[i].x --;
                        if(color_bis == I_TETROMINO) piece_bis[i].y ++;
                    }
                    for(int i = 0; i < 4; i++) {

                        Piece center = piece[1];
                        if(color == I_TETROMINO) piece[i].Rotate(center, 1);
                        if(color == T_TETROMINO) piece[i].Rotate(center, -1);
                        if(color == S_TETROMINO) piece[i].Rotate(center, -1);
                        if(color == Z_TETROMINO) piece[i].Rotate(center, -1);
                        if(color == L_TETROMINO) piece[i].Rotate(center, -1);
                        if(color == J_TETROMINO) piece[i].Rotate(center, 1);
                        piece[i].rotation = 1;

                        Piece center_bis = piece_bis[1];
                        if(color_bis == I_TETROMINO) piece_bis[i].Rotate(center_bis, 1);
                        if(color_bis == T_TETROMINO) piece_bis[i].Rotate(center_bis, -1);
                        if(color_bis == S_TETROMINO) piece_bis[i].Rotate(center_bis, -1);
                        if(color_bis == Z_TETROMINO) piece_bis[i].Rotate(center_bis, -1);
                        if(color_bis == L_TETROMINO) piece_bis[i].Rotate(center_bis, -1);
                        if(color_bis == J_TETROMINO) piece_bis[i].Rotate(center_bis, 1);
                        piece_bis[i].rotation = 1;
                    }

                    lock_delay_value = lock_delay;
                    lock_count_value = lock_count;
                    is_touch_ground = 0;

                    //=====================Clear Lines========================
                    int line_clear_count = 0;
                    int checkLine = HEIGHT - 1;
                    for(int i = HEIGHT - 1; i > 0; i--) {

                        int count = 0;
                        for(int j = 0; j < WIDTH; j++) {

                            if(board[i][j]) count++;
                            board[checkLine][j] = board[i][j];
                            board_bis[checkLine][j] = board_bis[i][j];
                        }
                        if(count < WIDTH) checkLine--;
                        else line_clear_count++;
                    }
                    if(line_clear_count != 0) {

                        line_clear_combo ++;
                        if(line_clear_count == 4 || is_tspin) {
                            btb_combo ++;

                            int combo = line_clear_combo - 1;
                        }
                        else {

                            if(btb_combo != 0) {
                               btb_combo = 0;
                            }
                            int combo = line_clear_combo - 1;
                        }

                        all_line_count += line_clear_count;
                    }
                    else if(line_clear_combo != 0) {
                        line_clear_combo = 0;
                    }

                    int perfect_clear = 1;
                    for(int i = 0; i < HEIGHT; i++) {
                        for(int j = 0; j < WIDTH; j++) {
                            if(board[i][j] != 0) perfect_clear = 0;
                            if(board_bis[i][j] != 0) perfect_clear = 0;
                        }
                    }
                    is_tspin = 0;
                }
                else if(!isCollided()) {

                    for(int i = 0; i < 4; i++) piece[i] = cache[i];
                    for(int i = 0; i < 4; i++) piece_bis[i] = cache_bis[i];
                }
                else {

                    lock_count_value = lock_count;

                    if(Keyboard::isKeyPressed(Keyboard::Down)) {

                        createParticle(&particles);
                        createParticle(&particles);
                    }
                }

                timer = 0;
            }
            board_wobble += 100 * ((-board_wobble) / 15) * time;

            //Update Ghost
            for(int i = 0; i < 4; i++) {

                ghost[i].x = piece[i].x;
                ghost[i].y = piece[i].y;
                ghost_bis[i].x = piece_bis[i].x;
                ghost_bis[i].y = piece_bis[i].y;
            }
            for(int i = 0; i < 4; i++) {

                while(isCollidedGhost()) {

                    for(int i = 0; i < 4; i++) ghost[i].y ++;
                    for(int i = 0; i < 4; i++) ghost_bis[i].y ++;
                }

                for(int i = 0; i < 4; i++) ghost[i].y --;
                for(int i = 0; i < 4; i++) ghost_bis[i].y --;
            }

            move_x = 0;
            rotate = 0;
            delay = 0.5;
            harddrop = 0;
            holded = 0;
            move_left = 0;
            move_right = 0;

            window.clear(Color::Black);

            //Draw Backboard
            RectangleShape backboard_shape;
            backboard_shape.setSize(Vector2f(320, 650));
            backboard_shape.setFillColor(Color::White);
            backboard_shape.setPosition(140, 20 + board_wobble);
            window.draw(backboard_shape);

            RectangleShape backboard_bis_shape;
            backboard_bis_shape.setSize(Vector2f(320, 650));
            backboard_bis_shape.setFillColor(Color::White);
            backboard_bis_shape.setPosition(790, 20 + board_wobble);
            window.draw(backboard_bis_shape);

            //Draw Grid
            board_s.setPosition(150, -30 + board_wobble);
            window.draw(board_s);

            board_bis_s.setPosition(800, -30 + board_wobble);
            window.draw(board_bis_s);

            //Draw Das Bar
            float das_progress = max((double)0, (double)(lock_delay_value / lock_delay));

            RectangleShape das_bar_shape;
            das_bar_shape.setSize(Vector2f(das_progress * 320, 8));
            das_bar_shape.setFillColor(Color::White);
            das_bar_shape.setPosition(140, 700 + board_wobble);
            window.draw(das_bar_shape);

            RectangleShape das_bar_bis_shape;
            das_bar_bis_shape.setSize(Vector2f(das_progress * 320, 8));
            das_bar_bis_shape.setFillColor(Color::White);
            das_bar_bis_shape.setPosition(790, 700 + board_wobble);
            window.draw(das_bar_bis_shape);

            //Draw Lock Count
            CircleShape lock_count_circle;
            lock_count_circle.setRadius(6);
            lock_count_circle.setFillColor(Color::White);
            for(int i = 0; i < lock_count_value; i++) {

                lock_count_circle.setPosition(141 + (i * 23.5), 680 + board_wobble);
                window.draw(lock_count_circle);
            }

            CircleShape lock_count_circle_bis;
            lock_count_circle_bis.setRadius(6);
            lock_count_circle_bis.setFillColor(Color::White);
            for(int i = 0; i < lock_count_value; i++) {

                lock_count_circle_bis.setPosition(791 + (i * 23.5), 680 + board_wobble);
                window.draw(lock_count_circle_bis);
            }

            //Draw Hold
            s.setColor(Color(255, 255, 255, 255));
            if(hold != -1) {

                Piece hold_piece[4];
                int hold_piece_choose;
                hold_piece_choose = hold;

                for(int j = 0; j < 4; j++) {

                    hold_piece[j].x = pieces[hold_piece_choose][j] % 2 + 4;
                    hold_piece[j].y = pieces[hold_piece_choose][j] / 2 + 3;

                    if(hold == J_TETROMINO) hold_piece[j].x --;
                }
                for(int j = 0; j < 4; j++) {

                    Piece center = hold_piece[1];
                    if(hold == I_TETROMINO) hold_piece[j].Rotate(center, 1);
                    if(hold == T_TETROMINO) hold_piece[j].Rotate(center, -1);
                    if(hold == S_TETROMINO) hold_piece[j].Rotate(center, -1);
                    if(hold == Z_TETROMINO) hold_piece[j].Rotate(center, -1);
                    if(hold == L_TETROMINO) hold_piece[j].Rotate(center, -1);
                    if(hold == J_TETROMINO) hold_piece[j].Rotate(center, 1);
                }

                for(int j = 0; j < 4; j++) {

                    s.setTextureRect(IntRect(hold*texture_size, 0, texture_size, texture_size));
                    int x_offset = 0;
                    if(hold == I_TETROMINO || hold == O_TETROMINO) x_offset = 15;
                    s.setPosition(hold_piece[j].x * texture_size - 65 - x_offset, hold_piece[j].y * texture_size - 10);
                    window.draw(s);
                }
            }

            s.setColor(Color(255, 255, 255, 255));
            if(hold != -1) {
                Piece hold_piece_bis[4];
                int hold_piece_choose_bis;
                hold_piece_choose_bis = hold;
                for(int j = 0; j < 4; j++) {

                    hold_piece_bis[j].x = pieces[hold_piece_choose_bis][j] % 2 + 4;
                    hold_piece_bis[j].y = pieces[hold_piece_choose_bis][j] / 2 + 3;

                    if(hold == J_TETROMINO) hold_piece_bis[j].x --;
                }
                for(int j = 0; j < 4; j++) {

                    Piece center_bis = hold_piece_bis[1];
                    if(hold == I_TETROMINO) hold_piece_bis[j].Rotate(center_bis, 1);
                    if(hold == T_TETROMINO) hold_piece_bis[j].Rotate(center_bis, -1);
                    if(hold == S_TETROMINO) hold_piece_bis[j].Rotate(center_bis, -1);
                    if(hold == Z_TETROMINO) hold_piece_bis[j].Rotate(center_bis, -1);
                    if(hold == L_TETROMINO) hold_piece_bis[j].Rotate(center_bis, -1);
                    if(hold == J_TETROMINO) hold_piece_bis[j].Rotate(center_bis, 1);
                }

                for(int j = 0; j < 4; j++) {

                    s.setTextureRect(IntRect(hold*texture_size, 0, texture_size, texture_size));
                    int x_offset = 0;
                    if(hold == I_TETROMINO || hold == O_TETROMINO) x_offset = 15;
                    s.setPosition(hold_piece_bis[j].x * texture_size - 65 - x_offset, hold_piece_bis[j].y * texture_size - 10);
                    window.draw(s);
                }
            }

            //Draw Next Pieces
            s.setColor(Color(255, 255, 255, 255));
            for(int i = 0; i < seven_bag.size() + seven_bag_next.size(); i++) {

                if(i < 5) {

                    Piece next_piece[4];
                    int next_piece_choose;
                    if(i < seven_bag.size()) next_piece_choose = seven_bag.at(i);
                    else next_piece_choose = seven_bag_next.at(i - seven_bag.size());

                    int next_color = next_piece_choose;
                    for(int j = 0; j < 4; j++) {

                        next_piece[j].x = pieces[next_piece_choose][j] % 2 + 4;
                        next_piece[j].y = pieces[next_piece_choose][j] / 2 + 3;

                        if(next_color == J_TETROMINO) next_piece[j].x --;
                    }
                    for(int j = 0; j < 4; j++) {

                        Piece center = next_piece[1];
                        if(next_color == I_TETROMINO) next_piece[j].Rotate(center, 1);
                        if(next_color == T_TETROMINO) next_piece[j].Rotate(center, -1);
                        if(next_color == S_TETROMINO) next_piece[j].Rotate(center, -1);
                        if(next_color == Z_TETROMINO) next_piece[j].Rotate(center, -1);
                        if(next_color == L_TETROMINO) next_piece[j].Rotate(center, -1);
                        if(next_color == J_TETROMINO) next_piece[j].Rotate(center, 1);
                    }

                    for(int j = 0; j < 4; j++) {

                        s.setTextureRect(IntRect(next_color*texture_size, 0, texture_size, texture_size));
                        int x_offset = 0;
                        if(next_color == I_TETROMINO || next_color == O_TETROMINO) x_offset = 15;
                        s.setPosition(next_piece[j].x * texture_size + 395 - x_offset, next_piece[j].y * texture_size - 10 + (90 * i));
                        window.draw(s);
                    }
                }
            }

            s.setColor(Color(255, 255, 255, 255));
            for(int i = 0; i < seven_bag.size() + seven_bag_next.size(); i++) {

                if(i < 5) {

                    Piece next_piece_bis[4];
                    int next_piece_choose_bis;
                    if(i < seven_bag.size()) next_piece_choose_bis = seven_bag.at(i);
                    else next_piece_choose_bis = seven_bag_next.at(i - seven_bag.size());

                    int next_color_bis = next_piece_choose_bis;
                    for(int j = 0; j < 4; j++) {

                        next_piece_bis[j].x = pieces[next_piece_choose_bis][j] % 2 + 4;
                        next_piece_bis[j].y = pieces[next_piece_choose_bis][j] / 2 + 3;

                        if(next_color_bis == J_TETROMINO) next_piece_bis[j].x --;
                    }
                    for(int j = 0; j < 4; j++) {

                        Piece center_bis = next_piece_bis[1];
                        if(next_color_bis == I_TETROMINO) next_piece_bis[j].Rotate(center_bis, 1);
                        if(next_color_bis == T_TETROMINO) next_piece_bis[j].Rotate(center_bis, -1);
                        if(next_color_bis == S_TETROMINO) next_piece_bis[j].Rotate(center_bis, -1);
                        if(next_color_bis == Z_TETROMINO) next_piece_bis[j].Rotate(center_bis, -1);
                        if(next_color_bis == L_TETROMINO) next_piece_bis[j].Rotate(center_bis, -1);
                        if(next_color_bis == J_TETROMINO) next_piece_bis[j].Rotate(center_bis, 1);
                    }

                    for(int j = 0; j < 4; j++) {

                        s.setTextureRect(IntRect(next_color_bis*texture_size, 0, texture_size, texture_size));
                        int x_offset = 0;
                        if(next_color_bis == I_TETROMINO || next_color_bis == O_TETROMINO) x_offset = 15;
                        s.setPosition(next_piece_bis[j].x * texture_size + 395 - x_offset, next_piece_bis[j].y * texture_size - 10 + (90 * i));
                        window.draw(s);
                    }
                }
            }

            //Draw Placed Pieces
            for(int i = 0; i < HEIGHT; i++) {

                for(int j = 0; j < WIDTH; j++) {

                    s.setColor(Color(255, 255, 255, 255));
                    if(board[i][j] == 0) continue;
                    s.setTextureRect(IntRect((board[i][j] - 1)*texture_size, 0, texture_size, texture_size));
                    s.setPosition(j * texture_size + 150, i * texture_size - 90 + board_wobble);
                    window.draw(s);
                }
            }

            for(int i = 0; i < HEIGHT; i++) {
                for(int j = 0; j < WIDTH; j++) {
                    s.setColor(Color(255, 255, 255, 255));
                    if(board_bis[i][j] == 0) continue;
                    s.setTextureRect(IntRect((board_bis[i][j] - 1)*texture_size, 0, texture_size, texture_size));
                    s.setPosition(j * texture_size + 800, i * texture_size - 90 + board_wobble);
                    window.draw(s);
                }
            }

            //Draw Piece Lock
            for(int i = 0; i < pieces_lock_bis.size(); i++) {

                for(int j = 0; j < 4; j++) {
                    pieces_lock_bis.at(i).at(j).animation += time * FRAMERATE * 4;
                }

                for(int j = 0; j < 4; j++) {

                    if(pieces_lock_bis.at(i).at(j).animation >= 67) {

                        pieces_lock_bis.erase(pieces_lock_bis.begin() + i);
                        break;
                    }
                }
            }

            for(int i = 0; i < pieces_lock.size(); i++) {

                for(int j = 0; j < 4; j++) {
                    pieces_lock.at(i).at(j).animation += time * FRAMERATE * 4;
                }

                for(int j = 0; j < 4; j++) {

                    if(pieces_lock.at(i).at(j).animation >= 67) {

                        pieces_lock.erase(pieces_lock.begin() + i);
                        break;
                    }
                }
            }

            //Draw Ghost
            s_ghost.setTextureRect(IntRect(color*texture_size, 0, texture_size, texture_size));
            for(int i = 0; i < 4; i++) {

                s_ghost.setPosition(piece[i].x * texture_size + 150, ghost[i].y * texture_size - 90 + board_wobble);
                window.draw(s_ghost);
            }

            s_ghost.setTextureRect(IntRect(color*texture_size, 0, texture_size, texture_size));
            for(int i = 0; i < 4; i++) {

                s_ghost.setPosition(piece_bis[i].x * texture_size + 800, ghost_bis[i].y * texture_size - 90 + board_wobble);
                window.draw(s_ghost);
            }

            //Draw Pieces
            piece_indicator_shape_alpha = (sin(game_elapsed_time.getElapsedTime().asSeconds() * 10) + 1) * 30;
            RectangleShape piece_indicator_shape;
            piece_indicator_shape.setSize(Vector2f(30, 30));
            piece_indicator_shape.setFillColor(Color(255, 255, 255, piece_indicator_shape_alpha));

            piece_indicator_shape_alpha_bis = (sin(game_elapsed_time.getElapsedTime().asSeconds() * 10) + 1) * 30;
            RectangleShape piece_indicator_shape_bis;
            piece_indicator_shape_bis.setSize(Vector2f(30, 30));
            piece_indicator_shape_bis.setFillColor(Color(255, 255, 255, piece_indicator_shape_alpha_bis));

            float piece_alpha = max((double)0, (double)(lock_delay_value / lock_delay));
            for(int i = 0; i < 4; i++) {

                s.setTextureRect(IntRect(7*texture_size, 0, texture_size, texture_size));
                s.setColor(Color(255, 255, 255, 255));
                s.setPosition(piece[i].x * texture_size + 150, piece[i].y * texture_size - 90 + board_wobble);
                window.draw(s);

                s.setTextureRect(IntRect(color*texture_size, 0, texture_size, texture_size));
                s.setColor(Color(255, 255, 255, piece_alpha * 255));
                s.setPosition(piece[i].x * texture_size + 150, piece[i].y * texture_size - 90 + board_wobble);
                window.draw(s);

                piece_indicator_shape.setPosition(piece[i].x * texture_size + 150, piece[i].y * texture_size - 90 + board_wobble);
                window.draw(piece_indicator_shape);
            }

            for(int i = 0; i < 4; i++) {

                s.setTextureRect(IntRect(7*texture_size, 0, texture_size, texture_size));
                s.setColor(Color(255, 255, 255, 255));
                s.setPosition(piece_bis[i].x * texture_size + 800, piece_bis[i].y * texture_size - 90 + board_wobble);
                window.draw(s);

                s.setTextureRect(IntRect(color*texture_size, 0, texture_size, texture_size));
                s.setColor(Color(255, 255, 255, piece_alpha * 255));
                s.setPosition(piece_bis[i].x * texture_size + 800, piece_bis[i].y * texture_size - 90 + board_wobble);
                window.draw(s);

                piece_indicator_shape.setPosition(piece_bis[i].x * texture_size + 800, piece_bis[i].y * texture_size - 90 + board_wobble);
                window.draw(piece_indicator_shape_bis);
            }

            //Draw Particle
            for(int i = 0; i < particles.size(); i++) {

                particles.at(i).update(time);
                particles.at(i).draw(&window);
                if(particles.at(i).alpha <= 0) particles.erase(particles.begin() + i);
            }

            for(int i = 0; i < particles_bis.size(); i++) {

                particles_bis.at(i).update(time);
                particles_bis.at(i).draw(&window);
                if(particles_bis.at(i).alpha <= 0) particles_bis.erase(particles_bis.begin() + i);
            }

            //Draw Backboard
            backboard_shape.setSize(Vector2f(320, 20));
            backboard_shape.setFillColor(Color::Black);
            backboard_shape.setPosition(140, 0 + board_wobble);
            window.draw(backboard_shape);

            backboard_shape.setSize(Vector2f(320, 10));
            backboard_shape.setFillColor(Color::White);
            backboard_shape.setPosition(140, 10 + board_wobble);
            window.draw(backboard_shape);

            window.display();

        }
        start --;
        game_clock.restart();
    }

    return 0;
}