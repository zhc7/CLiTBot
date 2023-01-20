#include <iostream>
#include <fstream>
#include <cstring>
#include "CLiTBot.h"
#include "interface.h"

using namespace std;

Game game; // 全局唯一的Game变量
const char *robot_source = "res/robot.bmp";
string map_info_path;

// API Declaration

// part 1 - Graphics
void auto_save();

void save(const char *path);

char path_of_autosave_on[MAX_PATH_LEN];
char path_of_autosave_off[MAX_PATH_LEN];


// part 2 - Execution
// 执行结果枚举类型

Result robot_run(const char *path);

struct Frame;
struct Stack;

OpSeq parse(const char *path);

// API Implementation

// part 1 - Graphics
#pragma pack(1)
struct BMPFileHeader {

    char bfType[2];
    int bfSize;
    short bfReserved1, bfReserved2;
    int bfOffBits;

};

#pragma pack(1)
struct BMPInfoHeader {

    int biSize;
    int biWidth;
    int biHeight;
    short biPlanes;
    short biBitCount;
    int biCompression;
    int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    int biClrUsed;
    int biClrImportant;

};
#pragma pack(1)

struct Pixel {

    unsigned char colorB;
    unsigned char colorG;
    unsigned char colorR;

    Pixel(int B = DEFAULT_B, int G = DEFAULT_G, int R = DEFAULT_R) :
            colorR(R), colorG(G), colorB(B) {}

};

Pixel YELLOW(0x37, 0xff, 0xff);
Pixel BLACK(0x00, 0x00, 0x00);
Pixel WHITE(0xff, 0xff, 0xff);
Pixel GREY(0x80, 0x80, 0x80);
Pixel GREYWHITE(0xef, 0xef, 0xef);
Pixel GREYBLACK(0x60, 0x60, 0x60);
Pixel BLUE(0xff, 0x80, 0x80);
Pixel SKYBLUE(0xff, 0x74, 0x28);
Pixel PURPLEGREY(0xdc, 0xb8, 0xb8);

Pixel pixelChart[IMAGE_WIDTH][IMAGE_HEIGHT];

enum BrickStatus {
    DIM, BRIGHT, NONLIGHT
};

#pragma pack(1)

struct Brick {

    int posx, posy, posz;

    BrickStatus status;

    Brick(int x = 0, int y = 0, int z = 0, BrickStatus s = DIM) :
            posx(x), posy(y), posz(z), status(s) {}


    // if two bricks have the same x and y, then compare their zs;
    // the brick with a lower x + y goes first;

    bool operator<(const Brick &other) {

        if ((posx + posy) < (other.posx + other.posy)) return true;
        if ((posx + posy) > (other.posx + other.posy)) return false;
        if (posx > other.posx) return true;
        if (posx < other.posx) return false;
        if (posz < other.posz) return true;
        return false;

    }

    void draw();

};

Brick bricks[MAX_BRICKS];
int brickCount;

void brickSwap(Brick &a, Brick &b) {
    Brick temp = a;
    a = b;
    b = temp;
}

void brickSort() {
    for (int i = brickCount - 1; i > 0; i--) {
        for (int j = 0; j < i; j++) {
            if (bricks[j + 1] < bricks[j]) brickSwap(bricks[j + 1], bricks[j]);
        }
    }
}

void generateMapFile() {

    char path[MAX_PATH_LEN];
    int tmp, rows, cols, num_lights, num_procs;
    cout << "Save the file at path: ";
    cin >> path;
    ofstream map;
    map.open(path);
    cout << "rows: ";
    cin >> rows;
    map << rows << SPACE;
    cout << "cols: ";
    cin >> cols;
    map << cols << SPACE;
    cout << "num_lights: ";
    cin >> num_lights;
    map << num_lights << SPACE;
    cout << "num_procs: ";
    cin >> num_procs;
    map << num_procs << endl;
    for (int i = 0; i < rows; i++) {
        cout << "heights of row " << i << ": ";
        for (int j = 0; j < cols; j++) {
            cin >> tmp;
            map << tmp << SPACE;
        }
        map << endl;
    }
    int x, y, d;
    for (int i = 0; i < num_lights; i++) {
        cout << "position of light No." << i << ": ";
        cin >> x >> y;
        map << x << SPACE << y << endl;
    }
    for (int i = 0; i < num_procs; i++) {
        cout << "limit of proc No." << i << ": ";
        cin >> tmp;
        map << tmp << SPACE;
    }
    map << endl;
    cout << "info of the robot (x, y, d): ";
    cin >> x >> y >> d;
    map << x << SPACE << y << SPACE << d;

}

void loadMapFile(const char *path) {
    ifstream fin;
    fin.open(path);
    fin >> game.map_init.row;
    fin >> game.map_init.col;
    fin >> game.map_init.num_lights;
    fin >> game.limit;
    for (int i = 0; i < game.map_init.row; i++) {
        for (int j = 0; j < game.map_init.col; j++) {

            fin >> game.map_init.cells[i][j].height;
            game.map_init.cells[i][j].light_id = -1;
            game.map_init.cells[i][j].robot = false;

        }
    }

    for (int i = 0; i < game.map_init.num_lights; i++) {

        int x, y;
        fin >> x;
        fin >> y;
        game.map_init.lights[i].pos.x = x;
        game.map_init.lights[i].pos.y = y;
        game.map_init.lights[i].lighten = false;
        game.map_init.cells[x][y].light_id = i;

    }

    for (int i = 0; i < game.limit; i++) {
        fin >> game.map_init.op_limit[i];
    }

    {

        int x, y, d;
        fin >> x >> y >> d;
        game.map_init.robot.pos.x = x;
        game.map_init.robot.pos.y = y;
        game.map_init.robot.dir = (Direction) d;

    }

    game.map_run = game.map_init;

}


void Brick::draw() {

    int X;
    int Y;

    int centerX =
            BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    int centerY =
            BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;

    X = centerX;
    Y = centerY;

    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y + 1;
    pixelChart[X][Y] = LINE_COLOR;

    for (int i = 0; i < BRICK_EXTENSION_Y + 1; i++) {
        Y--;
        pixelChart[X][Y] = LINE_COLOR;
        X--;
        pixelChart[X][Y] = LINE_COLOR;
        X--;
        pixelChart[X][Y] = LINE_COLOR;
    }

    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y - 1;
    pixelChart[X][Y] = LINE_COLOR;

    for (int i = 0; i < BRICK_EXTENSION_Y - 1; i++) {
        X--;
        pixelChart[X][Y] = LINE_COLOR;
        Y--;
        pixelChart[X][Y] = LINE_COLOR;
        X--;
        pixelChart[X][Y] = LINE_COLOR;
    }

    X--;
    pixelChart[X][Y] = LINE_COLOR;

    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y + 1;
    pixelChart[X][Y] = LINE_COLOR;

    for (int i = 0; i < BRICK_EXTENSION_Y + 1; i++) {
        Y--;
        pixelChart[X][Y] = LINE_COLOR;
        X++;
        pixelChart[X][Y] = LINE_COLOR;
        X++;
        pixelChart[X][Y] = LINE_COLOR;
    }

    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y - 1;
    pixelChart[X][Y] = LINE_COLOR;

    for (int i = 0; i < BRICK_EXTENSION_Y - 1; i++) {
        X++;
        pixelChart[X][Y] = LINE_COLOR;
        Y--;
        pixelChart[X][Y] = LINE_COLOR;
        X++;
        pixelChart[X][Y] = LINE_COLOR;
    }

    X++;
    pixelChart[X][Y] = LINE_COLOR;

    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y -= BRICK_EXTENSION_Y + 1;
    pixelChart[X][Y] = LINE_COLOR;

    for (int i = 0; i < BRICK_EXTENSION_Y + 1; i++) {
        Y++;
        pixelChart[X][Y] = LINE_COLOR;
        X--;
        pixelChart[X][Y] = LINE_COLOR;
        X--;
        pixelChart[X][Y] = LINE_COLOR;
    }

    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y -= BRICK_EXTENSION_Y - 1;
    pixelChart[X][Y] = LINE_COLOR;

    for (int i = 0; i < BRICK_EXTENSION_Y - 1; i++) {
        X--;
        pixelChart[X][Y] = LINE_COLOR;
        Y++;
        pixelChart[X][Y] = LINE_COLOR;
        X--;
        pixelChart[X][Y] = LINE_COLOR;
    }

    X--;
    pixelChart[X][Y] = LINE_COLOR;

    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y -= BRICK_EXTENSION_Y + 1;
    pixelChart[X][Y] = LINE_COLOR;

    for (int i = 0; i < BRICK_EXTENSION_Y + 1; i++) {
        Y++;
        pixelChart[X][Y] = LINE_COLOR;
        X++;
        pixelChart[X][Y] = LINE_COLOR;
        X++;
        pixelChart[X][Y] = LINE_COLOR;
    }

    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y -= BRICK_EXTENSION_Y - 1;
    pixelChart[X][Y] = LINE_COLOR;

    for (int i = 0; i < BRICK_EXTENSION_Y - 1; i++) {
        X++;
        pixelChart[X][Y] = LINE_COLOR;
        Y++;
        pixelChart[X][Y] = LINE_COLOR;
        X++;
        pixelChart[X][Y] = LINE_COLOR;
    }

    X++;
    pixelChart[X][Y] = LINE_COLOR;

    Y = centerY;
    X = centerX - BRICK_EXTENSION_X - 2;

    for (int i = 0; i < BRICK_EXTENSION_Z; i++) {
        pixelChart[X][Y + i] = LINE_COLOR;
        pixelChart[X + 1][Y + i + 1] = LINE_COLOR;
        pixelChart[X + 2][Y + i + 2] = LINE_COLOR;

    }

    Y = centerY;
    X = centerX + BRICK_EXTENSION_X + 2;

    for (int i = 0; i < BRICK_EXTENSION_Z; i++) {
        pixelChart[X][Y + i] = LINE_COLOR;
        pixelChart[X - 1][Y + i + 1] = LINE_COLOR;
        pixelChart[X - 2][Y + i + 2] = LINE_COLOR;

    }

    Y = centerY + BRICK_EXTENSION_Y + 1;
    X = centerX;

    for (int i = 0; i < BRICK_EXTENSION_Z; i++) {
        pixelChart[X][Y + i + 1] = LINE_COLOR;
        pixelChart[X + 1][Y + i] = LINE_COLOR;
        pixelChart[X - 1][Y + i] = LINE_COLOR;
    }

    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y + BRICK_EXTENSION_Z + 1;
    pixelChart[X][Y] = LINE_COLOR;

    for (int i = 0; i < BRICK_EXTENSION_Y + 1; i++) {
        Y--;
        pixelChart[X][Y] = LINE_COLOR;
        X--;
        pixelChart[X][Y] = LINE_COLOR;
        X--;
        pixelChart[X][Y] = LINE_COLOR;
    }

    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y + BRICK_EXTENSION_Z - 1;
    pixelChart[X][Y] = LINE_COLOR;

    for (int i = 0; i < BRICK_EXTENSION_Y; i++) {
        X--;
        pixelChart[X][Y] = LINE_COLOR;
        Y--;
        pixelChart[X][Y] = LINE_COLOR;
        X--;
        pixelChart[X][Y] = LINE_COLOR;
    }

    X--;
    pixelChart[X][Y] = LINE_COLOR;

    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y + BRICK_EXTENSION_Z + 1;
    pixelChart[X][Y] = LINE_COLOR;

    for (int i = 0; i < BRICK_EXTENSION_Y + 1; i++) {
        Y--;
        pixelChart[X][Y] = LINE_COLOR;
        X++;
        pixelChart[X][Y] = LINE_COLOR;
        X++;
        pixelChart[X][Y] = LINE_COLOR;
    }

    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y + BRICK_EXTENSION_Z - 1;
    pixelChart[X][Y] = LINE_COLOR;

    for (int i = 0; i < BRICK_EXTENSION_Y; i++) {
        X++;
        pixelChart[X][Y] = LINE_COLOR;
        Y--;
        pixelChart[X][Y] = LINE_COLOR;
        X++;
        pixelChart[X][Y] = LINE_COLOR;
    }

    X++;
    pixelChart[X][Y] = LINE_COLOR;

    X = centerX;
    Y = centerY + BRICK_EXTENSION_Y + 1;

    for (int i = 2; i <= BRICK_EXTENSION_X - 1; i++) {
        for (int j = 0; j <= BRICK_EXTENSION_Z - 4; j++) {
            pixelChart[X + i][Y - (i - 1) / 2 + j] = SIDE_COLOR_RIGHT;
        }
    }

    X = centerX;
    Y = centerY + BRICK_EXTENSION_Y + 1;

    for (int i = 2; i <= BRICK_EXTENSION_X - 1; i++) {
        for (int j = 0; j <= BRICK_EXTENSION_Z - 4; j++) {
            pixelChart[X - i][Y - (i - 1) / 2 + j] = SIDE_COLOR_LEFT;
        }
    }


    if (status == DIM) {

        for (int i = BRICK_EXTENSION_Y - 2; i >= 0; i--) {
            for (int j = 2 * (BRICK_EXTENSION_Y - 2 - i);
                 j >= 2 * (i - BRICK_EXTENSION_Y + 2); j--) {
                pixelChart[centerX + j][centerY + i] = DARK_COLOR;
            }
        }

        for (int i = -BRICK_EXTENSION_Y + 2; i <= 0; i++) {
            for (int j = 2 * (-BRICK_EXTENSION_Y + 2 - i);
                 j <= 2 * (i + BRICK_EXTENSION_Y - 2); j++) {
                pixelChart[centerX + j][centerY + i] = DARK_COLOR;
            }
        }

    }

    if (status == BRIGHT) {

        for (int i = BRICK_EXTENSION_Y - 2; i >= 0; i--) {
            for (int j = 2 * (BRICK_EXTENSION_Y - 2 - i);
                 j >= 2 * (i - BRICK_EXTENSION_Y + 2); j--) {
                pixelChart[centerX + j][centerY + i] = LIGHT_COLOR;
            }
        }

        for (int i = -BRICK_EXTENSION_Y + 2; i <= 0; i++) {
            for (int j = 2 * (-BRICK_EXTENSION_Y + 2 - i);
                 j <= 2 * (i + BRICK_EXTENSION_Y - 2); j++) {
                pixelChart[centerX + j][centerY + i] = LIGHT_COLOR;
            }
        }

    }

    if (status == NONLIGHT) {

        for (int i = BRICK_EXTENSION_Y - 2; i >= 0; i--) {
            for (int j = 2 * (BRICK_EXTENSION_Y - 2 - i);
                 j >= 2 * (i - BRICK_EXTENSION_Y + 2); j--) {
                pixelChart[centerX + j][centerY + i] = NONLIGHT_COLOR;
            }
        }

        for (int i = -BRICK_EXTENSION_Y + 2; i <= 0; i++) {
            for (int j = 2 * (-BRICK_EXTENSION_Y + 2 - i);
                 j <= 2 * (i + BRICK_EXTENSION_Y - 2); j++) {
                pixelChart[centerX + j][centerY + i] = NONLIGHT_COLOR;
            }
        }

    }

}


void drawRobot() {

    // cout << "drawRobot called" << endl;

    Robot &robot = game.map_run.robot;

    // cout << robot.pos.x << " " << robot.pos.y << endl;


    int off_x = 0, off_y = 0;

    if (robot.dir == LEFT) {
        off_x = 0;
        off_y = 700;
    }
    if (robot.dir == UP) {
        off_x = 350;
        off_y = 700;
    }
    if (robot.dir == DOWN) {
        off_x = 0;
        off_y = 0;
    }
    if (robot.dir == RIGHT) {
        off_x = 350;
        off_y = 0;
    }


    int base_x, base_y;
    int posx = robot.pos.y, posy = robot.pos.x;
    int posz = game.map_run.cells[posx][posy].height - 1;

    ifstream robotbmp;
    robotbmp.open(root_path + robot_source, ios::binary);


    base_x = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    base_y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y -
             posz * BRICK_EXTENSION_Z;

    base_x -= ROBOT_SIZE_X;
    base_y -= ROBOT_SIZE_Y * 1.8;

    Pixel temp;

    for (int i = 0; i < ROBOT_SIZE_X * 2; i++) {
        for (int j = 0; j < ROBOT_SIZE_Y * 2; j++) {

            int rbx = (i / (ROBOT_SIZE_X * 2.0)) * 350 + off_x;
            int rby = (j / (ROBOT_SIZE_Y * 2.0)) * 700 + off_y;
            robotbmp.seekg(0x36 + (rby * 700 + rbx) * 3);
            robotbmp.read((char *) &temp, 3);
            // cout << (int)temp.colorB << " " << (int)temp.colorG << " " << (int)temp.colorR << endl;

            if (temp.colorB != 164 || temp.colorG != 73 || temp.colorR != 163) {
                pixelChart[i + base_x][ROBOT_SIZE_Y * 2 - j + base_y] = temp;
            }

        }
    }

}

void igniteBrick(int x, int y, int z) {
    for (int i = 0; i < brickCount; i++) {
        if (bricks[i].posx == x &&
            bricks[i].posy == y &&
            bricks[i].posz == z) {

            if (bricks[i].status == DIM) {
                bricks[i].status = BRIGHT;

                for (int j = i; j < brickCount; j++) {
                    bricks[j].draw();
                }

            } else if (bricks[i].status == BRIGHT) {
                bricks[i].status = DIM;

                for (int j = i; j < brickCount; j++) {
                    bricks[j].draw();
                }

            }
            break;
        }
    }
}

void save(const char *path) {

    if (path[0] == '%') return;
    brickCount = 0;

    for (int i = 0; i < IMAGE_WIDTH; i++) {
        for (int j = 0; j < IMAGE_HEIGHT; j++) {
            pixelChart[i][j] = Pixel(DEFAULT_B, DEFAULT_G, DEFAULT_R);
        }
    }

    BMPFileHeader bfh = {{'B', 'M'}, 0x36, 0, 0, 0x36};
    BMPInfoHeader bih = {0x28, IMAGE_WIDTH, IMAGE_HEIGHT, 1, 24, 0,
                         IMAGE_WIDTH * IMAGE_HEIGHT * 3, 0, 0, 0, 0};

    // cout << path << endl;
    ofstream mybmp(path, ios::binary);

    mybmp.write((char *) &bfh, sizeof(BMPFileHeader));
    mybmp.write((char *) &bih, sizeof(BMPInfoHeader));

    Brick robot(game.map_run.robot.pos.y, game.map_run.robot.pos.x, 10);

    for (int i = 0; i < game.map_run.row; i++) {

        for (int j = 0; j < game.map_run.col; j++) {

            for (int k = 0; k < game.map_run.cells[i][j].height; k++) {

                BrickStatus s = NONLIGHT;
                {
                    for (int l = 0; l < game.map_run.num_lights; l++) {
                        if (game.map_run.lights[l].pos.x == j && game.map_run.lights[l].pos.y == i) {
                            if (game.map_run.lights[l].lighten) s = BRIGHT;
                            else s = DIM;
                            break;
                        }
                    }
                }
                // cout << i << " " << j << " " << k << " " << s << endl;
                bricks[brickCount++] = Brick(i, j, k, s);

            }

        }

    }

    brickSort();

    bool robotDrawn = false;

    for (int i = 0; i < brickCount; i++) {

        if (robot < bricks[i] && !robotDrawn) {
        //if (true) {

            drawRobot();
            robotDrawn = true;

        }

        bricks[i].draw();

    }
    if (!robotDrawn) {
        drawRobot();
        robotDrawn = true;
    }

    Pixel temp[IMAGE_WIDTH][IMAGE_HEIGHT];
    for (int X = 0; X < IMAGE_WIDTH; X++) {
        for (int Y = 0; Y < IMAGE_HEIGHT; Y++) {
            temp[X][Y] = pixelChart[X][Y];
        }
    }
    for (int X = 1; X < IMAGE_WIDTH - 1; X++) {
        for (int Y = 1; Y < IMAGE_HEIGHT - 1; Y++) {
            pixelChart[X][Y].colorB = (temp[X-1][Y-1].colorB +
                                       temp[X-1][Y].colorB +
                                       temp[X-1][Y+1].colorB +
                                       temp[X][Y-1].colorB +
                                       temp[X][Y].colorB +
                                       temp[X][Y+1].colorB +
                                       temp[X+1][Y-1].colorB +
                                       temp[X+1][Y].colorB +
                                       temp[X+1][Y+1].colorB) / 9;
            pixelChart[X][Y].colorG = (temp[X-1][Y-1].colorG +
                                       temp[X-1][Y].colorG +
                                       temp[X-1][Y+1].colorG +
                                       temp[X][Y-1].colorG +
                                       temp[X][Y].colorG +
                                       temp[X][Y+1].colorG +
                                       temp[X+1][Y-1].colorG +
                                       temp[X+1][Y].colorG +
                                       temp[X+1][Y+1].colorG) / 9;
            pixelChart[X][Y].colorR = (temp[X-1][Y-1].colorR +
                                       temp[X-1][Y].colorR +
                                       temp[X-1][Y+1].colorR +
                                       temp[X][Y-1].colorR +
                                       temp[X][Y].colorR +
                                       temp[X][Y+1].colorR +
                                       temp[X+1][Y-1].colorR +
                                       temp[X+1][Y].colorR +
                                       temp[X+1][Y+1].colorR) / 9;
        }
    }
    for (int Y = IMAGE_HEIGHT - 1; Y >= 0; Y--) {
        for (int X = 0; X < IMAGE_WIDTH; X++) {
            mybmp.write((char *) &pixelChart[X][Y], sizeof(Pixel));
        }
    }
    mybmp.close();
}

// void auto_save() {

//     if (strcmp(path_of_autosave_on,game.save_path)==0||strcmp(path_of_autosave_off,game.save_path)==0)
//     {
//         return;
//     }
//     save(game.save_path);
// }

void auto_save() {
    // cout << "auto_save called" << endl;
    if (strcmp(path_of_autosave_on,game.save_path)==0||strcmp(path_of_autosave_off,game.save_path)==0) return;
    // cout << "auto_save save called" << endl;
    // save(game.save_path);
    string save_path = game.save_path;
    int save_id = game.auto_save_id;
    int ptr = 0;
    while (save_path[++ptr] != '%');
    int len = save_path[ptr + 1] - '0';
    string path;
    for (int i = 0; i < ptr; i++) {
        path += save_path[i];
    }
    char arr[len];
    int arrptr = 0;
    while (save_id) {
        arr[arrptr++] = save_id % 10 + '0';
        save_id /= 10;
    }
    for (int i = 0; i < len - arrptr; i++) {
        path += '0';
    }
    while (arrptr) {
        path += arr[--arrptr];
    }
    ptr += 3;
    while (save_path[ptr]) {
        path += save_path[ptr];
        ptr++;
    }
    path += '\0';
    char p[MAX_PATH_LEN];
    for (int i = 0; i < path.size(); i++) {
        p[i] = path[i];
    }
    save(p);
    // cout << p << endl;
    game.auto_save_id++;
    map_info_path=path;
}


// part 2 - Execution
struct Frame {
    Proc *p;
    int c = 0;
    Frame *next = nullptr;
    Frame *prev = nullptr;

    Frame(Proc *p) : p(p) {};
};

struct Stack {
    Frame *head;
    Frame *current;

    Stack(Proc *main) {
        head = new Frame(main);
        current = head;
    }

    void push(Proc *p) {
        current->next = new Frame(p);
        current->next->prev = current;
        current = current->next;
    }

    Frame *pop() {
        current = current->prev;
        return current;
    }
};

OpSeq parse(const char *path) {
    ifstream fin(path);
    if (!fin) {
        char msg[] = "Operation Sequence file does not exist.";
        error(msg);
        //return OpSeq{{0}, 0};
        return OpSeq{{TL}, 0};
    }
    OpSeq seq;
    int t;
    int n;
    fin >> t;
    seq.count = t;
    char c[5];
    for (int i = 0; i < t; i++) {
        fin >> n;
        seq.procs[i].count = n;
        for (int j = 0; j < n; j++) {
            fin >> c;
            OpType op;
            switch (toupper(c[0])) {
                case 'T':
                    if (toupper(c[1]) == 'L') {
                        op = TL;
                    } else {
                        op = TR;
                    }
                    break;

                case 'M':
                    op = MOV;
                    break;

                case 'J':
                    op = JMP;
                    break;

                case 'L':
                    op = LIT;
                    break;

                default:
                    op = (OpType)(toupper(c[1]) - '0' + CALL);
                    break;
            }
            seq.procs[i].ops[j] = op;
        }
    }
    return seq;
}

Result robot_run(const char *path) {
    game.auto_save_id = 0;

    game.map_run = game.map_init;
    auto_save();
    int light_map[MAX_ROW][MAX_COL] = {0};
    int light_count = game.map_run.num_lights;
    for (int i = 0; i < game.map_run.num_lights; i++) {
        Position pos = game.map_run.lights[i].pos;
        light_map[pos.y][pos.x] = i + 1;
    }
    OpSeq seq = parse(path);
    Proc main = seq.procs[0];
    Stack stack(&main);
    Frame *f = stack.current;
    int step = 0;
    int x, y, pl;
    while (f) {
        for (int i = f->c; i < f->p->count; i++) {
            Robot &r = game.map_run.robot;
            OpType op = f->p->ops[i];
            switch (op) {
                case TL:
                    game.map_run.robot.dir = (Direction)((r.dir + 1) % 4);
                    auto_save();
                    break;

                case TR:
                    game.map_run.robot.dir = (Direction)((r.dir + 3) % 4);
                    auto_save();
                    break;

                case MOV:
                    x = r.pos.x;
                    y = r.pos.y;
                    x += (r.dir - 1) * ((r.dir + 1) % 2);
                    y += -(r.dir - 2) * (r.dir % 2);
                    if (x < 0 || y < 0 || x >= game.map_run.col || y >= game.map_run.row ||
                        game.map_run.cells[y][x].height != game.map_run.cells[r.pos.y][r.pos.x].height||game.map_run.cells[y][x].height==0) {
                        char msg[] = "Robot out of map.";
                        warn(msg);
                        break;
                    }
                    r.pos.x = x;
                    r.pos.y = y;
                    game.map_run.cells[y][x].robot = true;
                    auto_save();
                    break;

                case JMP:
                    x = r.pos.x;
                    y = r.pos.y;
                    x += (r.dir - 1) * ((r.dir + 1) % 2);
                    y += -(r.dir - 2) * (r.dir % 2);
                    if (x < 0 || y < 0 || x >= game.map_run.col || y >= game.map_run.row ||
                                game.map_run.cells[y][x].height - game.map_run.cells[r.pos.y][r.pos.x].height > 1||game.map_run.cells[y][x].height==0) {
                        char msg[] = "Robot out of map.";
                        warn(msg);
                        break;
                    }
                    if(game.map_run.cells[y][x].height - game.map_run.cells[r.pos.y][r.pos.x].height==0)
                    {
                        x = r.pos.x;
                        y = r.pos.y;
                        break;
                    }
                    r.pos.x = x;
                    r.pos.y = y;
                    game.map_run.cells[y][x].robot = true;
                    auto_save();
                    break;

                case LIT:
                    game.map_run.cells[r.pos.y][r.pos.x].light_id = 1;
                    pl = light_map[r.pos.y][r.pos.x];
                    if (pl) {
                        for (int l = 0; l < game.map_run.num_lights; l++) {
                            if (game.map_run.lights[l].pos.y == game.map_run.robot.pos.y &&
                                game.map_run.lights[l].pos.x == game.map_run.robot.pos.x) {
                                    if (!game.map_run.lights[l].lighten) light_count--;
                                    break;
                            }
                        }
                        game.map_run.lights[pl - 1].lighten = true;

                    }
                    auto_save();
                    break;

                default:
                    int n = op - CALL;
                    if (n > seq.count) {
                        char msg[] = "No procedure found.";
                        warn(msg);
                        break;
                    }
                    stack.push(&(seq.procs[n]));
                    f->c = i + 1;
                    f = stack.current;
                    i = f->c = 0;
                    i--; // offset i++ in for
                    break;
            }
            // auto_save();
            step++;
            if (step >= game.limit) {
                return Result{step, LIMIT};
            }
            if (!light_count) {
                return Result{step, LIGHT};
            }
        }
        f = stack.pop();
    }
    return Result{step, DARK};
}


// part 3 - User Interface
// WIP


int main() {
    game.limit = 100;
    memset(path_of_autosave_on, (int)'!', sizeof(char)*MAX_PATH_LEN);
    memset(path_of_autosave_off, 0, sizeof(char)*MAX_PATH_LEN);
    //generateMapFile();
    interface();
    return 0;
}
