#include <iostream>
#include <fstream>
#include <cstring>
#include "CLiTBot.h" 
#include "interface.h"
using namespace std;
int num_procs_limit;

extern Game game;
extern Result robot_run(const char* path);

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
}

int load(char map_path[])
{
    int direct;
    ifstream fin(map_path);
    if (!fin)
    {
        char msg[] = "Map file does not exist";
        error(msg);
        cout << "try to enter the proper map name"<<endl;
        strcpy(game.map_name, "");
        return 0;
    }
    else
    {
        strcpy(game.map_name, map_path);
        fin >> game.map_init.row >> game.map_init.col >> game.map_init.num_lights;
        fin >> num_procs_limit;
        //heights of row
        for (int i = 0; i < game.map_init.row; i++)
        {
            for (int j = 0; j < game.map_init.col; j++)
            {
                fin >> game.map_init.cells[i][j].height;
            }
        }
        //
        for (int i = 0; i < game.map_init.num_lights; i++)
        {
            fin >> game.map_init.lights[i].pos.x >> game.map_init.lights[i].pos.y;
            game.map_init.lights[i].lighten = 0;
        }
        for (int i = 0; i < game.map_init.row; i++)
        {
            for (int j = 0; j < game.map_init.col; j++)
            {
                if ((i != game.map_init.lights[i].pos.x) || (j != game.map_init.lights[i].pos.y))
                    game.map_init.cells[i][j].light_id = -1;
            }
        }
        for (int i = 0; i < num_procs_limit; i++)
        {
            fin >> game.map_init.op_limit[i];
        }
        fin >> game.map_init.robot.pos.x >> game.map_init.robot.pos.y;
        fin >> direct;
        direct = convert(direct);
        game.map_init.robot.dir=(Direction)direct;
        for (int i = 0; i < game.map_init.row; i++)
        {
            for (int j = 0; j < game.map_init.col; j++)
            {
                if ((i != game.map_init.robot.pos.x) || (j != game.map_init.robot.pos.y))
                    game.map_init.cells[i][j].robot = false;
                else
                    game.map_init.cells[i][j].robot = true;
            }
        }
        return 1;
    }
}

void mapinfo(Map *map)
{
    int i;
    cout << "Map Name:" << ' ' << game.map_name << endl;
    cout << "Autosave:" << ' ' << game.save_path << endl;
    cout << "Step Limit:" << ' ' << game.limit << endl;
    cout <<map->row << ' ' << map->col << ' ' << map->num_lights << ' ' << num_procs_limit << endl;
    for (i = 0; i < map->row; i++)
    {
        for (int j = 0; j < map->col; j++)
        {
            if (map->cells[i][j].height == 0)
                cout << ' ';
            else
            {
                //while robot is on this cordinate
                if ((map->robot.pos.x == i) && (map->robot.pos.y == j))
                {
                    if (map->cells[i][j].light_id == -1)
                        cout << "\e[91;100;1m"; //red, grey
                    else if (map->cells[i][j].light_id != -1)//there is a light on this place
                    {
                        for (int k = 0; k < map->num_lights; k++)
                        {
                            if ((map->lights[k].pos.x == i) && (map->lights[k].pos.y == j) && (map->lights[k].lighten))//it is lighten
                                cout << "\e[91;103;1m";//red, yellow
                            else if ((map->lights[k].pos.x == i) && (map->lights[k].pos.y == j) && (!map->lights[k].lighten))
                                cout << "\e[91;104;1m";//red, blue
                        }
                    }
                }
                else
                {
                    if (map->cells[i][j].light_id == -1)
                        cout << "\e[92;100;1m"; //green, grey
                    else if (map->cells[i][j].light_id != -1)//there is a light on this place
                    {
                        for (int k = 0; k < map->num_lights; k++)
                        {
                            if ((map->lights[k].pos.x == i) && (map->lights[k].pos.y == j) && (map->lights[k].lighten))//it is lighten
                                cout << "\e[92;103;1m";//green, yellow
                            else if ((map->lights[k].pos.x == i) && (map->lights[k].pos.y == j) && (!map->lights[k].lighten))
                                cout << "\e[92;104;1m";//green, blue;
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
    switch (map->robot.dir)
    {
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

    cout << '[';
    for (i = 0; i < num_procs_limit-1; i++)
    {
        cout << map->op_limit[i] << ",";
    }
    cout<<map->op_limit[num_procs_limit - 1] << ']' << endl;
}

void operation(char op_path[])
{
    int num_procs,*proc_id;
    char order[10];
    ofstream op;
    op.open(op_path,ios::out);
    if (!op.is_open())
        cout << "open file error";
    else
    {
        cin >> num_procs;
        while (num_procs > num_procs_limit)
        {
            cout << "process limit exceeded!" << endl;
            cout << "try to enter a value that is not bigger than "<< num_procs_limit << endl;
            cin >> num_procs;
        }
        op << num_procs<<endl;
        proc_id = new int[num_procs];//enter steps for each procs
        for (int i = 0; i < num_procs; i++)
        {
            cin >> proc_id[i];
            for (int j = 0; j < proc_id[i]; j++)
            {
                cin >> order;
            }
            while (proc_id[i] > game.map_init.op_limit[i])
            {
                cout << "steps of this process have exceeded!" << endl;
                cout << "the steps of this process should not be bigger than " << game.map_init.op_limit[i] << endl;
                cout << "please input steps and command again" << endl;
                cin >> proc_id[i];
                for (int j = 0; j < proc_id[i]; j++)
                {
                    cin >> order;
                }
            }
            op << proc_id[i]<<' ';
            for (int j = 0; j < proc_id[i]; j++)
            {
                op << order << ' ';
            }
            op << endl;
        }
        delete[] proc_id;
    }
    op.close();
}

int interface()
{
    cout << "CLiTBot" << endl;
    //default game.map.name should be null
    int ifload = 0;
    char order[MAX_PATH_LEN], map_path[MAX_PATH_LEN],op_path[MAX_PATH_LEN];
    char autosave_code[20];
    for (int steps = 1; steps <= game.limit; steps++)
    {
        cout << "\e[92;1m#CLiTBot > \e[0m";
        int steps_set;
        cin >> order;
        for (int i = 0; i < strlen(order); i++)
        {
            order[i]=toupper(order[i]);
        }
        //load
        if (strcmp(order, "LOAD") == 0)
        {
            cin >> map_path;
            ifload = load(map_path);
            if (ifload) {
                char msg[] = "Map successfully loaded";
                info(msg);
            }
        }
        //autosave
        else if (strcmp(order, "AUTOSAVE") == 0)
        {
            cin >> autosave_code;
            if (strcmp(autosave_code, "OFF") == 0)
                game.save_path[0] = '!';
            else if (strcmp(autosave_code, "ON") == 0)
                game.save_path[0] = 0;
            else
                strcpy(game.save_path, autosave_code);
        }
        //limit of steps
        else if (strcmp(order, "LIMIT") == 0)
        {
            cin >> steps_set;
            game.limit = steps_set;
        }
        //output set
        else if (strcmp(order, "STATUS")==0)
        {
            if (ifload == 1)
            {
                mapinfo(&game.map_init);
            }
        }
        else if (strcmp(order, "OP") == 0)
        {
            cin >> op_path;
            operation(op_path);
        }
        else if (strcmp(order, "RUN") == 0)
        {
            int i;
            cin >> op_path;
            Result result=robot_run(op_path);
            cout << "Step(s) used:" << ' ' << result.steps << endl;
            for (i = 0; i <game.map_run.row; i++)
            {
                for (int j = 0; j < game.map_run.col; j++)
                {
                    if (game.map_run.cells[i][j].height == 0)
                        cout << ' ';
                    else
                    {
                        //while robot is on this cordinate
                        if ((game.map_run.robot.pos.x == i) && (game.map_run.robot.pos.y == j))
                        {
                            if (game.map_run.cells[i][j].light_id == -1)
                                cout << "\e[91;100;1m"; //red, grey
                            else if (game.map_run.cells[i][j].light_id != -1)//there is a light on this place
                            {
                                for (int k = 0; k < game.map_run.num_lights; k++)
                                {
                                    if ((game.map_run.lights[k].pos.x == i) && (game.map_run.lights[k].pos.y == j) && (game.map_run.lights[k].lighten))//it is lighten
                                        cout << "\e[91;103;1m";//red, yellow
                                    else if ((game.map_run.lights[k].pos.x == i) && (game.map_run.lights[k].pos.y == j) && (!game.map_run.lights[k].lighten))
                                        cout << "\e[91;104;1m";//red, blue
                                }
                            }
                        }
                        else
                        {
                            if ((game.map_run.robot.pos.x == i) && (game.map_run.robot.pos.y == j))
                            {
                                if (game.map_run.cells[i][j].light_id == -1)
                                    cout << "\e[92;100;1m"; //red, grey
                                else if (game.map_run.cells[i][j].light_id != -1)//there is a light on this place
                                {
                                    for (int k = 0; k < game.map_run.num_lights; k++)
                                    {
                                        if ((game.map_run.lights[k].pos.x == i) && (game.map_run.lights[k].pos.y == j) && (game.map_run.lights[k].lighten))//it is lighten
                                            cout << "\e[92;103;1m";//red, yellow
                                        else if ((game.map_run.lights[k].pos.x == i) && (game.map_run.lights[k].pos.y == j) && (!game.map_run.lights[k].lighten))
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
            switch (game.map_run.robot.dir)
            {
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

        }
        else if (strcmp(order, "EXIT") == 0)
        {
            cout << "Quiting..." << endl;
            break;
        }
        else
        {
            char msg[] = "Unknown command";
            error(msg);
        }

    }
    return 0;
}
