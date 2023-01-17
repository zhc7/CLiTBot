#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include "CLiTBot.h"
#include "interface.h"

using namespace std;
int num_procs_limit;

extern Game game;

extern Result robot_run(const char *path);

//interface
void warn(char s[]) {
    cout << "\e[93;1m";
    cout << "Warning: ";
    cout << s << endl;
    cout << "\e[0m";
}

void error(char s[]) {
    cout << "\e[91;1m" << "Error: " << s << "\e[0m" << endl;
}

void info(char s[]) {
    cout << "\e[96;1m" << "Info: " << s << "\e[0m" << endl;
}

void info(string s) {
    cout << "\e[96;1m" << "Info: " << s << "\e[0m" << endl;
}

int convert(int direct) {
    switch (direct) {
        case 0:
            return 3;
        case 1:
            return 1;
        case 2:
            return 0;
        case 3:
            return 2;
    }
    return -1;
}

int load(char map_path[]) {
    int direct;
    ifstream fin(map_path);
    if (!fin) {
        char msg[] = "Map file does not exist";
        error(msg);
        cout << "try to enter the proper map name" << endl;
        strcpy(game.map_name, "");
        return 0;
    } else {
        strcpy(game.map_name, map_path);
        fin >> game.map_init.row >> game.map_init.col >> game.map_init.num_lights;
        fin >> num_procs_limit;
        //heights of row
        for (int i = 0; i < game.map_init.row; i++) {
            for (int j = 0; j < game.map_init.col; j++) {
                fin >> game.map_init.cells[i][j].height;
            }
        }
        //
        for (int i = 0; i < game.map_init.num_lights; i++) {
            fin >> game.map_init.lights[i].pos.x >> game.map_init.lights[i].pos.y;
            game.map_init.lights[i].lighten = 0;
        }
        for (int i = 0; i < game.map_init.row; i++) {
            for (int j = 0; j < game.map_init.col; j++) {
                if ((j != game.map_init.lights[i].pos.x) || (i != game.map_init.lights[i].pos.y))
                    game.map_init.cells[i][j].light_id = -1;
            }
        }
        for (int i = 0; i < num_procs_limit; i++) {
            fin >> game.map_init.op_limit[i];
        }
        fin >> game.map_init.robot.pos.x >> game.map_init.robot.pos.y;
        fin >> direct;
        direct = convert(direct);
        game.map_init.robot.dir = (Direction) direct;
        for (int i = 0; i < game.map_init.row; i++) {
            for (int j = 0; j < game.map_init.col; j++) {
                if ((i != game.map_init.robot.pos.x) || (j != game.map_init.robot.pos.y))
                    game.map_init.cells[i][j].robot = false;
                else
                    game.map_init.cells[i][j].robot = true;
            }
        }
        return 1;
    }
}

void mapinfo(Map *map) {
    int i;
    cout << "Map Name:" << ' ' << game.map_name << endl;
    cout << "Autosave:" << ' ' << game.save_path << endl;
    cout << "Step Limit:" << ' ' << game.limit << endl;
    cout << map->row << ' ' << map->col << ' ' << map->num_lights << ' ' << num_procs_limit << endl;
    for (i = 0; i < map->row; i++) {
        for (int j = 0; j < map->col; j++) {
            if (map->cells[i][j].height == 0)
                cout << ' ';
            else {
                //while robot is on this coordinate
                if ((map->robot.pos.x == j) && (map->robot.pos.y == i)) {
                    if (map->cells[i][j].light_id == -1)
                        cout << "\e[91;100;1m"; //red, grey
                    else if (map->cells[i][j].light_id != -1)//there is a light on this place
                    {
                        for (int k = 0; k < map->num_lights; k++) {
                            if ((map->lights[k].pos.x == j) && (map->lights[k].pos.y == i) &&
                                (map->lights[k].lighten))//it is lightened
                            {
                                cout << "\e[91;103;1m";//red, yellow
                                break;
                            } else if ((map->lights[k].pos.x == j) && (map->lights[k].pos.y == i) &&
                                       (!map->lights[k].lighten)) {
                                cout << "\e[91;104;1m";//red, blue
                                break;
                            }
                        }
                    }
                } else {
                    if (map->cells[i][j].light_id == -1)
                        cout << "\e[92;100;1m"; //green, grey
                    else if (map->cells[i][j].light_id != -1)//there is a light on this place
                    {
                        for (int k = 0; k < map->num_lights; k++) {
                            if ((map->lights[k].pos.x == j) && (map->lights[k].pos.y == i) &&
                                (map->lights[k].lighten))//it is lightened
                            {
                                cout << "\e[92;103;1m";//green, yellow
                                break;
                            } else if ((map->lights[k].pos.x == j) && (map->lights[k].pos.y == i) &&
                                       (!map->lights[k].lighten)) {
                                cout << "\e[92;104;1m";//green, blue;
                                break;
                            }
                        }
                    }
                }
                cout << map->cells[i][j].height;
                cout << "\e[0m";
            }
        }
        cout << endl;
    }
    cout << "Robot is facing ";
    switch (map->robot.dir) {
        case LEFT:
            cout << "left." << endl;
            break;
        case RIGHT:
            cout << "right." << endl;
            break;
        case DOWN:
            cout << "down." << endl;
            break;
        case UP:
            cout << "up." << endl;
            break;
    }

    cout << "Proc limits : [";
    for (i = 0; i < num_procs_limit - 1; i++) {
        cout << map->op_limit[i] << ",";
    }
    cout << map->op_limit[num_procs_limit - 1] << ']' << endl;
}


void operation(char op_path[])
{
    int num_procs, * proc_id;
    string* order;
    ofstream op;
    op.open(op_path, ios::out);
    if (!op.is_open())
        cout << "open file error";
    else
    {
        cin >> num_procs;
        while (num_procs > num_procs_limit)
        {
            cout << "process limit exceeded!" << endl;
            cout << "try to enter a value that is not bigger than " << num_procs_limit << endl;
            cin >> num_procs;
        }
        op << num_procs << endl;
        proc_id = new int[num_procs];//enter steps for each procs
        for (int i = 0; i < num_procs; i++)
        {
            cin >> proc_id[i];
            order = new string[proc_id[i]];
            for (int j = 0; j < proc_id[i]; j++)
            {
                cin >> order[j];
            }
            while (proc_id[i] > game.map_init.op_limit[i])
            {
                cout << "steps of this process have exceeded!" << endl;
                cout << "the steps of this process should not be bigger than " << game.map_init.op_limit[i] << endl;
                cout << "please input steps and command again" << endl;
                cin >> proc_id[i];
                delete[] order;
                order = new string[proc_id[i]];
                for (int j = 0; j < proc_id[i]; j++)
                {
                    cin >> order[j];
                }
            }
            if (i != 0)
            {
                for (int j = 0; j < proc_id[i]; j++)
                {

                    string temp = "";
                    for (int k = 0; k < order[j].size(); k++)
                        temp.push_back(toupper(order[j][k]));

                    while (temp != "JMP" && temp != "CALL" && temp != "MOV" && temp != "LIT" && temp != "TL" && temp != "TR")
                    {
                        cout << "the no." << j + 1 << " order is invalid, please try again: " << endl;
                        cout << "correct orders are:(no need to be uppercased) JMP, CALL, MOV, LIT, TL, TR." << endl;
                        temp = "";
                        cin >> order[j];
                        for (int k = 0; k < order[j].size(); k++)
                            temp.push_back(toupper(order[j][k]));
                    }
                }
            }
            op << proc_id[i] << ' ';
            for (int j = 0; j < proc_id[i]; j++)
            {
                op << order[j] << ' ';
            }
            op << endl;
            delete[] order;
        }
        delete[] proc_id;
    }
    op.close();
}//op is ok!!!

int interface() {
    cout << "CLiTBot" << endl;
    //default game.map.name should be null
    int ifload = 0;
    char order[MAX_PATH_LEN], map_path[MAX_PATH_LEN], op_path[MAX_PATH_LEN];
    char autosave_code[20];
    while (true) {
        string s = std::filesystem::current_path().string();
        cout << "\e[92;1m#CLiTBot " << s << "> \e[0m";
        int steps_set;
        cin >> order;
        for (int i = 0; i < strlen(order); i++) {
            order[i] = toupper(order[i]);
        }
        //load
        if (strcmp(order, "LOAD") == 0) {
            cin >> map_path;
            ifload = load(map_path);
            if (ifload) {
                char msg[] = "Map successfully loaded";
                info(msg);
            }
        }
            //autosave
        else if (strcmp(order, "AUTOSAVE") == 0) {
            cin >> autosave_code;
            if (strcmp(autosave_code, "OFF") == 0)
                game.save_path[0] = '!';
            else if (strcmp(autosave_code, "ON") == 0)
                game.save_path[0] = 0;
            else
                strcpy(game.save_path, autosave_code);
        }
            //limit of steps
        else if (strcmp(order, "LIMIT") == 0) {
            cin >> steps_set;
            game.limit = steps_set;
        }
            //output set
        else if (strcmp(order, "STATUS") == 0) {
            if (ifload == 1) {
                mapinfo(&game.map_init);
            }
        } else if (strcmp(order, "OP") == 0) {
            cin >> op_path;
            operation(op_path);
        } else if (strcmp(order, "RUN") == 0) {
            int i;
            cin >> op_path;
            if (!ifload) {
                char msg[] = "You have not loaded any map yet.";
                error(msg);
                continue;
            }
            Result result = robot_run(op_path);
            cout << "Step(s) used:" << ' ' << result.steps << endl;
            for (i = 0; i < game.map_run.row; i++) {
                for (int j = 0; j < game.map_run.col; j++) {
                    if (game.map_run.cells[i][j].height == 0)
                        cout << ' ';
                    else {
                        //while robot is on this coordinate
                        if ((game.map_run.robot.pos.x == j) && (game.map_run.robot.pos.y == i)) {
                            if (game.map_run.cells[i][j].light_id == -1)
                                cout << "\e[91;100;1m"; //red, grey
                            else if (game.map_run.cells[i][j].light_id != -1)//there is a light on this place
                            {
                                for (int k = 0; k < game.map_run.num_lights; k++) {
                                    if ((game.map_run.lights[k].pos.x == j) && (game.map_run.lights[k].pos.y == i) &&
                                        (game.map_run.lights[k].lighten))//it is lightened
                                        cout << "\e[91;103;1m";//red, yellow
                                    else if ((game.map_run.lights[k].pos.x == j) &&
                                             (game.map_run.lights[k].pos.y == i) && (!game.map_run.lights[k].lighten))
                                        cout << "\e[91;104;1m";//red, blue
                                }
                            }
                        } else {
                            if ((game.map_run.robot.pos.x == j) && (game.map_run.robot.pos.y == i)) {
                                if (game.map_run.cells[i][j].light_id == -1)
                                    cout << "\e[92;100;1m"; //red, grey
                                else if (game.map_run.cells[i][j].light_id != -1)//there is a light on this place
                                {
                                    for (int k = 0; k < game.map_run.num_lights; k++) {
                                        if ((game.map_run.lights[k].pos.x == j) &&
                                            (game.map_run.lights[k].pos.y == i) &&
                                            (game.map_run.lights[k].lighten))//it is lightened
                                            cout << "\e[92;103;1m";//red, yellow
                                        else if ((game.map_run.lights[k].pos.x == j) &&
                                                 (game.map_run.lights[k].pos.y == i) &&
                                                 (!game.map_run.lights[k].lighten))
                                            cout << "\e[92;104;1m";//red, blue
                                    }
                                }
                            }
                        }
                        cout << game.map_run.cells[i][j].height;
                        cout << "\e[0m";
                    }
                }
                cout << endl;
            }
            cout << "Robot is facing ";
            switch (game.map_run.robot.dir) {
                case LEFT:
                    cout << "left." << endl;
                    break;
                case RIGHT:
                    cout << "right." << endl;
                    break;
                case DOWN:
                    cout << "down." << endl;
                    break;
                case UP:
                    cout << "up." << endl;
                    break;
            }
            string msg;
            switch (result.result) {
                case LIGHT:
                    msg = "Congratulations! You Won!";
                    break;
                case LIMIT:
                    msg = "Step limitation reached";
                    break;
                case DARK:
                    msg = "Execution completed";
                    break;
            }
            info(msg);
        } else if (strcmp(order, "EXIT") == 0) {
            cout << "Quiting..." << endl;
            break;
        } else if (strcmp(order, "CD") == 0) {
            string path;
            cin >> path;
            std::filesystem::current_path(path);
        } else if (strcmp(order, "LS") == 0) {
            cout << ".\n.." << endl;
            auto path = std::filesystem::current_path();
            for (const auto &entry: std::filesystem::directory_iterator(path)) {
                std::cout << entry.path().filename().string() << std::endl;
            }
        } else {
            char msg[] = "Unknown command";
            error(msg);
        }

    }
    return 0;
}
