#pragma once
#define LINE_COLOR BLACK
#define SIDE_COLOR_RIGHT GREYWHITE
#define SIDE_COLOR_LEFT BLUE
#define LIGHT_COLOR YELLOW
#define DARK_COLOR SKYBLUE
#define NONLIGHT_COLOR PURPLEGREY
#define SPACE " "
#define MAX_OPS  32
#define MAX_PROCS  8
#define MAX_ROW  16
#define MAX_COL  16
#define MAX_LIT  16
#define MAX_PATH_LEN  512
#define DEFAULT_B  191
#define DEFAULT_G  191
#define DEFAULT_R 191
#define IMAGE_WIDTH  800
#define IMAGE_HEIGHT  600
#define BASE_POINT_X  399
#define BASE_POINT_Y  199
#define BRICK_EXTENSION_X  60
#define BRICK_EXTENSION_Y  30
#define BRICK_EXTENSION_Z  25
#define ROBOT_SIZE_X  35
#define ROBOT_SIZE_Y  70
#define MAX_BRICKS 256


// constants



// global definition

// 位置类型，可用来表达机器人或灯所在位置
struct Position {
    int x, y; // x 表示列号，y 表示行号
};

// 方向枚举类型，可用来表达机器人朝向，只有四种取值
enum Direction {
    LEFT,   // 左前方
    DOWN,   // 左后方
    RIGHT,  // 右后方
    UP,     // 右前方
};

// 机器人类型
struct Robot {
    Position pos;  // 机器人位置
    Direction dir; // 机器人朝向
};

// 灯类型
struct Light {
    Position pos; // 灯位置
    bool lighten; // 是否被点亮
};           

// 单元格类型
struct Cell {
    int height;   // 高度
    int light_id; // 灯标识，-1表示该单元格上没有灯
    bool robot;   // true/false分别表示机器人在/不在该单元格上
};

// 指令类型
enum OpType {
    TL,
    TR,
    MOV,
    JMP,
    LIT,
    CALL
}; // TL为左转，TR为右转，MOV为向前行走，JMP为跳跃，LIT为点亮灯；
   // 使用CALL表示调用MAIN，CALL + 1表示调用P1，以此类推。

// 过程类型
struct Proc {
    OpType ops[MAX_OPS];
    // 指令记录，MAX_OPS为合理常数
    int count; // 有效指令数
};

// 指令序列类型
struct OpSeq {
    // 过程记录，MAX_PROCS为合理常数
    // procs[0]为MAIN过程，procs[1]为P1过程，以此类推
    Proc procs[MAX_PROCS];
    int count; // 有效过程数
};             

// 地图状态类型
struct Map { 
    // 单元格组成二维数组，MAX_ROW、MAX_COL为合理常数
    Cell cells[MAX_ROW][MAX_COL];
    int row, col; // 有效行数、有效列数 
    
    // 灯记录，MAX_LIT为合理常数
    Light lights[MAX_LIT];
    int num_lights; // 有效灯数 
    
    // 地图上同时只有一个机器人
    Robot robot;    
    
    // 每个过程的指令数限制
    int op_limit[MAX_PROCS];
}; 

// 游戏状态类型
struct Game {
    char map_name[MAX_PATH_LEN]; // 当前地图的文件路径名
    Map map_init;                // 地图初始状态 
    Map map_run;                 // 指令执行过程中的地图状态 

    // 自动保存的文件路径名，MAX_PATH_LEN为合理常数
    char save_path[MAX_PATH_LEN];
    int auto_save_id; // 自动保存标识
    int limit;        // 执行指令上限（用来避免无限递归）
};

enum ResultType {
    LIGHT, // 结束条件1，点亮了全部灯，干得漂亮 
    LIMIT, // 结束条件2，到达操作数上限 
    DARK // 结束条件3，MAIN过程执行完毕 
};

// 执行结果类型 
struct Result {
    int steps; // 记录总步数 
    ResultType result; // 用enum记录结束原因 
};


