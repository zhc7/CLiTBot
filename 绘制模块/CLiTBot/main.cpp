#include <iostream>
#include <fstream>
#include <string>
#define outputDir "/Users/Shenium/Documents/output.txt"
#define SPACE " "
#define IMAGE_WIDTH 800
#define IMAGE_HEIGHT 600
#define BASE_POINT_X 399
#define BASE_POINT_Y 199
#define BRICK_EXTENSION_X 60
#define BRICK_EXTENSION_Y 30
#define BRICK_EXTENSION_Z 25
#define ROBOT_SIZE_X 35
#define ROBOT_SIZE_Y 70
#define DEFAULT_B 191
#define DEFAULT_G 191
#define DEFAULT_R 191
#define MAX_OPS 8
#define MAX_PROCS 3
#define MAX_ROW 6
#define MAX_COL 6
#define MAX_LIT 16
#define MAX_PATH_LEN 100
#define MAX_BRICKS 256

#define LINE_COLOR BLACK
#define SIDE_COLOR_RIGHT GREYWHITE
#define SIDE_COLOR_LEFT BLUE
#define LIGHT_COLOR YELLOW
#define DARK_COLOR SKYBLUE
#define NONLIGHT_COLOR PURPLEGREY

using namespace std;


// functions for debugging and testing, ignore them.

void binaryReadIntoFile(char dir[]) {
    
    static char hex[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F'
    };
    
    ifstream fileInput;
    ofstream fileOutput;
    fileInput.open(dir, ios::binary);
    fileOutput.open(outputDir);
    
    unsigned char temp;
    while (!fileInput.eof()) {
        fileInput.read((char *)&temp, 1);
        fileOutput << hex[temp / 16];
        fileOutput << hex[temp % 16];
        fileOutput << ' ';
    }
    
    fileInput.close();
    fileOutput.close();
    
}

void binaryReadIntoCout(char dir[]) {
    
    static char hex[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F'
    };
    
    ifstream fileInput;
    fileInput.open(dir, ios::binary);
    
    unsigned char temp;
    while (!fileInput.eof()) {
        fileInput.read((char *)&temp, 1);
        cout << hex[temp / 16];
        cout << hex[temp % 16];
        cout << ' ';
    }
    
    fileInput.close();
    cout << endl;
    
}

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



struct Position {
    int x, y;
};

enum Direction {
    LEFT, DOWN, RIGHT, UP
};

struct Robot {
    Position pos;
    Direction dir;
};

struct Light {
    Position pos;
    bool lighten;
};

struct Cell {
    int height;
    int light_id;
    bool robot;
};

enum OpType {
    TL, TR, MOV, JMP, LIT, CALL
};

struct Proc {
    OpType ops[MAX_OPS];
};

struct OpSeq {
    Proc procs[MAX_PROCS];
    int count;
};

struct Map {
    Cell cells[MAX_ROW][MAX_COL];
    int row, col;
    Light lights[MAX_LIT];
    int num_lights;
    Robot robot;
    int op_limit[MAX_PROCS];
};

struct Game {
    char map_name[MAX_PATH_LEN];
    Map map_init;
    Map map_run;
    char save_path[MAX_PATH_LEN];
    int auto_save_id;
    int limit;
};

Game game;

char path[MAX_PATH_LEN] = "/Users/Shenium/Documents/LightBot/myBitMap00.bmp";

int steps = 0;


#pragma pack(1)
struct Pixel {
    
    unsigned char colorB;
    unsigned char colorG;
    unsigned char colorR;
    Pixel(int B=DEFAULT_B, int G=DEFAULT_G, int R=DEFAULT_R):
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

enum BrickStatus {DARK, BRIGHT, NONLIGHT};

#pragma pack(1)
struct Brick {
    
    int posx, posy, posz;
    
    BrickStatus status;
    
    Brick(int x=0, int y=0, int z=0, BrickStatus s=DARK):
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
        game.map_init.robot.dir = (Direction)d;
        
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
        Y--; pixelChart[X][Y] = LINE_COLOR;
        X--; pixelChart[X][Y] = LINE_COLOR;
        X--; pixelChart[X][Y] = LINE_COLOR;
    }
    
    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y - 1;
    pixelChart[X][Y] = LINE_COLOR;
    
    for (int i = 0; i < BRICK_EXTENSION_Y - 1; i++) {
        X--; pixelChart[X][Y] = LINE_COLOR;
        Y--; pixelChart[X][Y] = LINE_COLOR;
        X--; pixelChart[X][Y] = LINE_COLOR;
    }
    
    X--; pixelChart[X][Y] = LINE_COLOR;
    
    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y + 1;
    pixelChart[X][Y] = LINE_COLOR;
    
    for (int i = 0; i < BRICK_EXTENSION_Y + 1; i++) {
        Y--; pixelChart[X][Y] = LINE_COLOR;
        X++; pixelChart[X][Y] = LINE_COLOR;
        X++; pixelChart[X][Y] = LINE_COLOR;
    }
    
    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y - 1;
    pixelChart[X][Y] = LINE_COLOR;
    
    for (int i = 0; i < BRICK_EXTENSION_Y - 1; i++) {
        X++; pixelChart[X][Y] = LINE_COLOR;
        Y--; pixelChart[X][Y] = LINE_COLOR;
        X++; pixelChart[X][Y] = LINE_COLOR;
    }
    
    X++; pixelChart[X][Y] = LINE_COLOR;
    
    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y -= BRICK_EXTENSION_Y + 1;
    pixelChart[X][Y] = LINE_COLOR;
    
    for (int i = 0; i < BRICK_EXTENSION_Y + 1; i++) {
        Y++; pixelChart[X][Y] = LINE_COLOR;
        X--; pixelChart[X][Y] = LINE_COLOR;
        X--; pixelChart[X][Y] = LINE_COLOR;
    }
    
    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y -= BRICK_EXTENSION_Y - 1;
    pixelChart[X][Y] = LINE_COLOR;
    
    for (int i = 0; i < BRICK_EXTENSION_Y - 1; i++) {
        X--; pixelChart[X][Y] = LINE_COLOR;
        Y++; pixelChart[X][Y] = LINE_COLOR;
        X--; pixelChart[X][Y] = LINE_COLOR;
    }
    
    X--; pixelChart[X][Y] = LINE_COLOR;
    
    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y -= BRICK_EXTENSION_Y + 1;
    pixelChart[X][Y] = LINE_COLOR;
    
    for (int i = 0; i < BRICK_EXTENSION_Y + 1; i++) {
        Y++; pixelChart[X][Y] = LINE_COLOR;
        X++; pixelChart[X][Y] = LINE_COLOR;
        X++; pixelChart[X][Y] = LINE_COLOR;
    }
    
    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y -= BRICK_EXTENSION_Y - 1;
    pixelChart[X][Y] = LINE_COLOR;
    
    for (int i = 0; i < BRICK_EXTENSION_Y - 1; i++) {
        X++; pixelChart[X][Y] = LINE_COLOR;
        Y++; pixelChart[X][Y] = LINE_COLOR;
        X++; pixelChart[X][Y] = LINE_COLOR;
    }
    
    X++; pixelChart[X][Y] = LINE_COLOR;
    
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
        Y--; pixelChart[X][Y] = LINE_COLOR;
        X--; pixelChart[X][Y] = LINE_COLOR;
        X--; pixelChart[X][Y] = LINE_COLOR;
    }
    
    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y + BRICK_EXTENSION_Z - 1;
    pixelChart[X][Y] = LINE_COLOR;
    
    for (int i = 0; i < BRICK_EXTENSION_Y; i++) {
        X--; pixelChart[X][Y] = LINE_COLOR;
        Y--; pixelChart[X][Y] = LINE_COLOR;
        X--; pixelChart[X][Y] = LINE_COLOR;
    }
    
    X--; pixelChart[X][Y] = LINE_COLOR;
    
    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y + BRICK_EXTENSION_Z + 1;
    pixelChart[X][Y] = LINE_COLOR;
    
    for (int i = 0; i < BRICK_EXTENSION_Y + 1; i++) {
        Y--; pixelChart[X][Y] = LINE_COLOR;
        X++; pixelChart[X][Y] = LINE_COLOR;
        X++; pixelChart[X][Y] = LINE_COLOR;
    }
    
    X = BASE_POINT_X - (posx - posy) * BRICK_EXTENSION_X;
    Y = BASE_POINT_Y + (posx + posy) * BRICK_EXTENSION_Y - posz * BRICK_EXTENSION_Z;
    Y += BRICK_EXTENSION_Y + BRICK_EXTENSION_Z - 1;
    pixelChart[X][Y] = LINE_COLOR;
    
    for (int i = 0; i < BRICK_EXTENSION_Y; i++) {
        X++; pixelChart[X][Y] = LINE_COLOR;
        Y--; pixelChart[X][Y] = LINE_COLOR;
        X++; pixelChart[X][Y] = LINE_COLOR;
    }
    
    X++; pixelChart[X][Y] = LINE_COLOR;
    
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
    
    
    
    if (status == DARK) {
        
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
    
    Robot &robot = game.map_run.robot;
    
    int off_x = 0, off_y = 0;
    
    if (robot.dir == LEFT)  {off_x = 0;   off_y = 700;}
    if (robot.dir == UP)    {off_x = 350; off_y = 700;}
    if (robot.dir == DOWN)  {off_x = 0;   off_y = 0  ;}
    if (robot.dir == RIGHT) {off_x = 350; off_y = 0  ;}
    
    
    int base_x, base_y;
    int posx = robot.pos.y, posy = robot.pos.x;
    int posz = game.map_run.cells[posx][posy].height - 1;
    
    ifstream robotbmp;
    robotbmp.open("/Users/Shenium/Documents/robot.bmp", ios::binary);
    
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
            robotbmp.read((char *)&temp, 3);
            
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
            
            if (bricks[i].status == DARK) {
                bricks[i].status = BRIGHT;
                
                for (int j = i; j < brickCount; j++) {
                    bricks[j].draw();
                }
                
            } else if (bricks[i].status == BRIGHT) {
                bricks[i].status = DARK;
                
                for (int j = i; j < brickCount; j++) {
                    bricks[j].draw();
                }
            
            }
            break;
        }
    }
}

void save(const char *path) {
    
    brickCount = 0;
    
    for (int i = 0; i < IMAGE_WIDTH; i++) {
        for (int j = 0; j < IMAGE_HEIGHT; j++) {
            pixelChart[i][j] = Pixel(DEFAULT_B, DEFAULT_G, DEFAULT_R);
        }
    }
    
    BMPFileHeader bfh = {{'B', 'M'}, 0x36, 0, 0, 0x36};
    BMPInfoHeader bih = {0x28, IMAGE_WIDTH, IMAGE_HEIGHT, 1, 24, 0,
        IMAGE_WIDTH * IMAGE_HEIGHT * 3, 0, 0, 0, 0};
    
    ofstream mybmp(path, ios::binary);
    
    mybmp.write((char *)&bfh, sizeof(BMPFileHeader));
    mybmp.write((char *)&bih, sizeof(BMPInfoHeader));
    
    Brick robot(game.map_run.robot.pos.y, game.map_run.robot.pos.x, 10);
    
    for (int i = 0; i < game.map_run.row; i++) {
        
        for (int j = 0; j < game.map_run.col; j++) {
            
            for (int k = 0; k < game.map_run.cells[i][j].height; k++) {
                
                BrickStatus s;
                
                if (game.map_run.cells[i][j].light_id == -1) {
                    s = NONLIGHT;
                } else if (game.map_run.lights
                           [game.map_run.cells[i][j].light_id].lighten) {
                    s = BRIGHT;
                } else s = DARK;
                
                bricks[brickCount++] = Brick(j, i, k, s);
                
            }
            
        }
        
    }
    
    
    brickSort();
    
    bool robotDrawn = false;
    
    for (int i = 0; i < brickCount; i++) {
        
        if (robot < bricks[i] && !robotDrawn) {
            
            drawRobot();
            robotDrawn = true;
            
        }
        
        bricks[i].draw();
        
    }
    
    for (int Y = IMAGE_HEIGHT - 1; Y >= 0; Y--) {
        
        for (int X = 0; X < IMAGE_WIDTH; X++) {
            
            mybmp.write((char *)&pixelChart[X][Y], sizeof(Pixel));
            
        }
        
    }
    
    mybmp.close();
    
}

void auto_save() {
    if (!game.save_path[0]) return;
    save(game.save_path);
}

bool isValidMove(int px, int py, int x, int y) {
    return game.map_run.cells[x][y].height &&
    game.map_run.cells[x][y].height == game.map_run.cells[px][py].height;
}

bool forward() {
    
    Robot &robot = game.map_run.robot;
    
    int px = robot.pos.x;
    int py = robot.pos.y;
    
    int x = px;
    int y = py;
    
    if (robot.dir == LEFT) px--;
    if (robot.dir == RIGHT) px++;
    if (robot.dir == UP) y = py--;
    if (robot.dir == DOWN) y = py++;
    
    if (isValidMove(px, py, x, y)) {
        
        robot.pos.x = px;
        robot.pos.y = py;
        
        steps++;
        path[43] = steps % 10 + '0';
        path[42] = steps / 10 + '0';
        save(path);
        
        return true;
        
    } else {
        
        return false;
        
    }
    
}

bool turnLeft() {
    
    Robot &robot = game.map_run.robot;
    robot.dir = (Direction)((robot.dir + 1) % 4);
    
    steps++;
    path[43] = steps % 10 + '0';
    path[42] = steps / 10 + '0';
    save(path);
    
    return true;
    
}

bool turnRight() {
    
    Robot &robot = game.map_run.robot;
    robot.dir = (Direction)((robot.dir + 3) % 4);
    
    steps++;
    path[43] = steps % 10 + '0';
    path[42] = steps / 10 + '0';
    save(path);
    
    return true;
    
}

bool isValidJump(int px, int py, int x, int y) {
    
    return game.map_run.cells[x][y].height && (
    game.map_run.cells[x][y].height - game.map_run.cells[px][py].height == 1 ||
    game.map_run.cells[x][y].height - game.map_run.cells[px][py].height == -1);
    
}

bool jump() {
    
    Robot &robot = game.map_run.robot;
    
    int px = robot.pos.x;
    int py = robot.pos.y;
    
    int x = px;
    int y = py;
    
    if (robot.dir == LEFT) px--;
    if (robot.dir == RIGHT) px++;
    if (robot.dir == UP) y = py--;
    if (robot.dir == DOWN) y = py++;
    
    if (isValidJump(px, py, x, y)) {
        
        robot.pos.x = px;
        robot.pos.y = py;
        
        steps++;
        path[43] = steps % 10 + '0';
        path[42] = steps / 10 + '0';
        save(path);
        
        return true;
        
    } else {
        
        return false;
        
    }
    
}

bool isValidLight(int x, int y) {
    
    return game.map_run.cells[x][y].light_id != -1;
    
}

bool lightup() {
    
    Robot &robot = game.map_run.robot;
    
    int x = robot.pos.x, y = robot.pos.y;
    
    if (isValidLight(robot.pos.x, robot.pos.y)) {
        
        game.map_run.lights[game.map_run.cells[x][y].light_id].lighten =
        !game.map_run.lights[game.map_run.cells[x][y].light_id].lighten;
        
        steps++;
        path[43] = steps % 10 + '0';
        path[42] = steps / 10 + '0';
        save(path);
        
        return true;
        
    } else {
        
        return false;
        
    }
    
}




int main() {
    
    loadMapFile("/Users/Shenium/Documents/map.txt");
    save("/Users/Shenium/Documents/LightBot/myBitMap00.bmp");
    
    for (int i = 0; i < 4; i++) {

        lightup();
        jump();
        lightup();
        forward();
        lightup();
        forward();
        lightup();
        jump();
        turnLeft();

    }


    return 0;
    
}
