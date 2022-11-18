#include <iostream>
using namespace std;


// constants
const int MAX_OPS = 32;
const int MAX_PROCS = 8;
const int MAX_ROW = 16;
const int MAX_COL = 16;
const int MAX_LIT = 16;
const int MAX_PATH_LEN = 512;


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
Game game; // 全局唯一的Game变量


// API Declaration

// part 1 - Graphics
// WIP

// part 2 - Execution
// 执行结果枚举类型 
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
Result robot_run(const char* path);

// part 3 - User Interface
void warn(/*WIP*/); //实现命令行警告
// WIP


// API Implementation

// part 1 - Graphics
// WIP

// part 2 - Execution
struct Frame {
    Proc* p;
    int c = 0;
    Frame* next = nullptr;
    Frame* prev = nullptr;
    Frame (Proc* p) : p(p) {};
};

struct Stack {
    Frame* head;
    Frame* current;
    Stack (Proc* main) {
        head = new Frame(main);
        current = head;
    }

    void push (Proc* p) {
        current -> next = new Frame(p);
        current = current -> next;
    }

    Proc* pop() {
        current = current -> prev;
        return current -> p;
    }

    void call() {
        
    }
};

OpSeq& parse(const char* path);

Result robot_run(const char* path) {
    game.auto_save_id = 0;
    game.map_run = game.map_init;
    OpSeq seq = parse(path);
    Proc main = seq.procs[0];
    Stack stack(&main);
    Frame* f = stack.current;
    while (true) {
        for (int i = f->c; i < f->p->count; i++) {
            Robot& r = game.map_run.robot;
            switch (f->p->ops[i]) {
            case TL:
                game.map_run.robot.dir = (Direction)((r.dir + 1) % 4);
                break;
            
            case TR:
                game.map_run.robot.dir = (Direction)((r.dir + 3) % 4);
                break;
            
            case MOV:
                int x = r.pos.x;
                int y = r.pos.y;
                x += (r.dir - 1) * ((r.dir + 1) % 2);
                y += -(r.dir - 2) * (r.dir % 2);
                if (x < 0 or y < 0 or x >= game.map_run.col or y >= game.map_run.row or 
                    game.map_run.cells[y][x].height != game.map_run.cells[r.pos.y][r.pos.x].height) {
                    warn();
                    break;
                }
                r.pos.x = x;
                r.pos.y = y;
                break;
            
            case JMP:
                int x = r.pos.x;
                int y = r.pos.y;
                x += (r.dir - 1) * ((r.dir + 1) % 2);
                y += -(r.dir - 2) * (r.dir % 2);
                if (x < 0 or y < 0 or x >= game.map_run.col or y >= game.map_run.row or 
                    [](int x){return x > 0 ? x:-x;}(game.map_run.cells[y][x].height - game.map_run.cells[r.pos.y][r.pos.x].height) != 1) {
                    warn();
                    break;
                }
                r.pos.x = x;
                r.pos.y = y;
                break;

            case LIT:

            default:
                break;
            }
        }
    }
    for(int i = 0; i < seq.count; i++) {
        Proc p = seq.procs[i];

    }
}

// part 3 - User Interface
// WIP


int main() {
    return 0;
}