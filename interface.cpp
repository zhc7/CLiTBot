#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>
#include <ctime>
#include <map>
#include "CLiTBot.h"
#include "interface.h"
#pragma warning(disable:4996)

using namespace std;
using namespace filesystem;
int num_procs_limit;

extern Game game;

extern Result robot_run(const char *path);

extern void auto_save();

extern void save(const char *path);

extern char path_of_autosave_on[MAX_PATH_LEN];
extern char path_of_autosave_off[MAX_PATH_LEN];

bool if_run;
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
    if_run=false;
    ifstream fin(map_path);
    if (!fin) {
        char msg[] = "Map file does not exist";
        error(msg);
        cout << "try to enter the proper map name" << endl;
        strcpy(game.map_name, "");
        return 0;
    }
    else {
        strcpy(game.map_name, map_path);
        fin >> game.map_init.row >> game.map_init.col >> game.map_init.num_lights;
        fin >> num_procs_limit;
        //heights of row
        for (int i = 0; i < game.map_init.row; i++) {
            for (int j = 0; j < game.map_init.col; j++) {
                game.map_init.cells[i][j].height=0;
                game.map_init.cells[i][j].light_id = 0;//
            }
        }

        for (int i = 0; i < game.map_init.num_lights; i++) {
            game.map_init.lights[i].pos.x = -1;
            game.map_init.lights[i].pos.y = -1;
            game.map_init.lights[i].lighten = 0;

        }

        for (int i = 0; i < game.map_init.row; i++) {
            for (int j = 0; j < game.map_init.col; j++) {
                fin >> game.map_init.cells[i][j].height;
            }
        }

        for (int i = 0; i < game.map_init.num_lights; i++) {
            fin >> game.map_init.lights[i].pos.x >> game.map_init.lights[i].pos.y;
            game.map_init.lights[i].lighten = 0;

        }



        for (int i = 0; i < game.map_init.row; i++) {
            for (int j = 0; j < game.map_init.col; j++) {
                int flag = 0;
                for (int k = 0; k < game.map_init.num_lights; k++)
                {
                    if ((j != game.map_init.lights[k].pos.x) || (i != game.map_init.lights[k].pos.y))
                        flag++;
                    else
                        break;
                }
                if(flag==game.map_init.num_lights)
                    game.map_init.cells[i][j].light_id = -1;
            }
        }

        // for (int i = 0; i < game.map_init.row; i++) {
        //     for (int j = 0; j < game.map_init.col; j++) {
        //         cout << game.map_init.cells[i][j].light_id << ' ';
        //     }
        //     cout << endl;
        // }//

        for (int i = 0; i < num_procs_limit; i++) {
            fin >> game.map_init.op_limit[i];
        }

        // for (int i = 0; i < game.map_init.num_lights; i++) {
        //     cout << game.map_init.lights[i].pos.x << ' ' << game.map_init.lights[i].pos.y << endl;
        // }//

        fin >> game.map_init.robot.pos.x >> game.map_init.robot.pos.y;
        fin >> direct;
        direct = convert(direct);
        game.map_init.robot.dir = (Direction)direct;
        for (int i = 0; i < game.map_init.row; i++) {
            for (int j = 0; j < game.map_init.col; j++) {
                if ((i != game.map_init.robot.pos.x) || (j != game.map_init.robot.pos.y))
                    game.map_init.cells[i][j].robot = false;
                else
                {
                    game.map_init.cells[i][j].robot = true;
                    //cout<<game.map_init.robot.pos.x<<game.map_init.robot.pos.y
                    return 1;
                }

            }
        }
        return 1;
    }
}

void mapinfo(Map *map) {
    int i;
    cout << "Map Name:" << ' ' << game.map_name << endl;
    if(strcmp(path_of_autosave_off, game.save_path)==0)
        cout<<"Autosave: OFF"<<endl;
    else if(strcmp(path_of_autosave_on,game.save_path)==0)
        cout<<"Autosave: ON"<<endl;
    else
        cout << "Autosave:" << ' ' << game.save_path << endl;

    cout << "Step Limit:" << ' ' << game.limit << endl;
   // cout << map->row << ' ' << map->col << ' ' << map->num_lights << ' ' << num_procs_limit << endl;
    for (int i = 0; i < map->row; i++) {//i represents y, j represents x
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
                        int flag=0;
                        for (int k = 0; k < map->num_lights; k++) {
                            if ((map->lights[k].pos.x == j) && (map->lights[k].pos.y == i) &&
                                (map->lights[k].lighten))//it is lightened
                            {
                                flag=1;
                                cout << "\e[91;103;1m";//red, yellow
                                break;
                            } else if ((map->lights[k].pos.x == j) && (map->lights[k].pos.y == i) &&
                                       (!map->lights[k].lighten)) {
                                flag=1;
                                cout << "\e[91;104;1m";//red, blue
                                break;
                            }
                        }
                        if(flag==0)//no lights on this place but lights id!=0
                            cout << "\e[91;100;1m"; //red, grey
                    }
                } else {
                    if (map->cells[i][j].light_id == -1)
                        cout << "\e[92;100;1m"; //green, grey
                    else if (map->cells[i][j].light_id != -1)//there is a light on this place
                    {
                        int flag=0;
                        for (int k = 0; k < map->num_lights; k++) {
                            if ((map->lights[k].pos.x == j) && (map->lights[k].pos.y == i) &&
                                (map->lights[k].lighten))//it is lightened
                            {
                                flag=1;
                                cout << "\e[92;103;1m";//green, yellow
                                break;
                            } else if ((map->lights[k].pos.x == j) && (map->lights[k].pos.y == i) &&
                                       (!map->lights[k].lighten)) {
                                flag=1;
                                cout << "\e[92;104;1m";//green, blue;
                                break;
                            }
                        }
                        if(flag==0)
                        {
                            cout << "\e[92;100;1m";
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

void mapinfo_run(Map* map)
{
    for (int i = 0; i < map->row; i++) {//i represents y, j represents x
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
                        int flag=0;
                        for (int k = 0; k < map->num_lights; k++) {
                            if ((map->lights[k].pos.x == j) && (map->lights[k].pos.y == i) &&
                                (map->lights[k].lighten))//it is lightened
                            {
                                flag=1;
                                cout << "\e[91;103;1m";//red, yellow
                                break;
                            } else if ((map->lights[k].pos.x == j) && (map->lights[k].pos.y == i) &&
                                       (!map->lights[k].lighten)) {
                                flag=1;
                                cout << "\e[91;104;1m";//red, blue
                                break;
                            }
                        }
                        if(flag==0)//no lights on this place but lights id!=0
                            cout << "\e[91;100;1m"; //red, grey
                    }
                } else {
                    if (map->cells[i][j].light_id == -1)
                        cout << "\e[92;100;1m"; //green, grey
                    else if (map->cells[i][j].light_id != -1)//there is a light on this place
                    {
                        int flag=0;
                        for (int k = 0; k < map->num_lights; k++) {
                            if ((map->lights[k].pos.x == j) && (map->lights[k].pos.y == i) &&
                                (map->lights[k].lighten))//it is lightened
                            {
                                flag=1;
                                cout << "\e[92;103;1m";//green, yellow
                                break;
                            } else if ((map->lights[k].pos.x == j) && (map->lights[k].pos.y == i) &&
                                       (!map->lights[k].lighten)) {
                                flag=1;
                                cout << "\e[92;104;1m";//green, blue;
                                break;
                            }
                        }
                        if(flag==0)
                        {
                            cout << "\e[92;100;1m";
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
    cout<<endl;
}



void operation(char op_path[])
{
    int num_procs, * proc_id;
    string* order;
    ofstream op;
    op.open(op_path, ios::out);
    if (!op.is_open())
        cout << "open file error"<<endl;
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

            for (int j = 0; j < proc_id[i]; j++)
            {

                 string temp = "";
                for (int k = 0; k < order[j].size(); k++)
                      temp.push_back(toupper(order[j][k]));

                while (temp != "JMP" && temp != "CALL" && temp != "MOV" && temp != "LIT" && temp != "TL" && temp != "TR"&&strcmp((char*)temp[0],"P")!=0)
                {
                    cout << "the no." << j + 1 << " order is invalid, please try again: " << endl;
                    cout << "correct orders are:(no need to be uppercased) JMP, CALL, MOV, LIT, TL, TR." << endl;
                    temp = "";
                    cin >> order[j];
                    for (int k = 0; k < order[j].size(); k++)
                        temp.push_back(toupper(order[j][k]));
                }
            }

            //identify main
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

void delay(int ms)
{

    clock_t start=0,end=0;
    start=clock();
    while(end-start<ms)
        end=clock();
}
void showrobot()
{
    char olstr[100];
    ifstream ifs("./res/robot.txt");
    if(ifs.good())
    {
        while(!ifs.eof())
        {

            ifs.getline(olstr,100);
            for(auto &e:olstr)
            {
                if(e=='.')
                    e=' ';
            }
            //delay(100000);
            delay(20);
            cout <<"\t\t"<<olstr<<endl;
        }

    }
    else
    {
        cout<<"res or robot.txt is missing! make sure it exists"<<endl;
    }
}
#define pathdepth 100
bool endofpath[pathdepth];

void printtree(int depth)
{
    for (int i = 0; i < depth; i++)
    {

        if(endofpath[i]==false)
            cout << "|    ";
        else
            cout << "     ";
    }

}
void enumdir(path& directory,int depth)
{
    file_status s;
    path p;
#if defined(_WIN32) && !defined(__CYGWIN__)
    wstring str;
#else
    string str;
#endif
    vector<path> curdir;
    vector<path> curfile;
    for (auto it = directory_iterator(directory); it != directory_iterator(); ++it)
    {
        s = it->symlink_status();
        switch (s.type())
        {
        case file_type::regular:
            curfile.push_back(*it);
            break;

        case file_type::directory:
            curdir.push_back(*it);
            break;
        }
    }
    for (int i = 0; i < curfile.size(); i++)
    {

        printtree(depth);
        str = curfile[i].filename();
        cout << "\033[32m";
        wcout << "|-" << str.c_str() << endl;
        cout << "\033[37m";
    }
    for (int i = 0; i < curdir.size(); i++)
    {
        if (i + 1 == curdir.size())
            endofpath[depth] = true;
        printtree(depth);
        str = curdir[i].filename();

        wcout << "|-" << str.c_str() << endl;

        enumdir(curdir[i], depth + 1);
    }



}

map<string,string> CMD={{"LOAD","enter your file path to load it"},
                            {"AUTOSAVE","automatic save to file"},
                            {"LIMIT","enter the maximum steps of each proc"},
                            {"STATUS","get status of the map currently"},
                            {"OP","enter your orders to run the game"},
                            {"RUN","run the game!"},
                            {"EXIT","exit the game"},
                            {"CD","come to directory"},
                            {"LS","list file of current directory!"},
                            {"TREE","display the file tree!"},
                            {"HELP","help system"},
                            {"CLS","clear the screen!"}
                            };

int matchstrnum(string s1,string s2)
{
    int maxmatch=0;
    int match=0;
    for(int i=0;i<s1.size();i++)
    {
        match = 0;
        for(int j=0;j<s2.size();j++)
        {
            if((i+j)<s1.size())
            {
                if (s1[i + j] == s2[j])
                    match++;
                else
                {
                    if(match>maxmatch)
                         maxmatch=match;
                    match=0;
                    continue;
                }
            }

        }
        if(match>maxmatch)
            maxmatch=match;
    }
    return maxmatch;
}
void getpossiblecmd(string str)
{
    string possible;
    int maxmatch=0,match=0;
    for(auto &e:str)
        e=toupper(e);
    for(auto iter=CMD.begin();iter!=CMD.end();++iter)
    {
        match=matchstrnum(iter->first,str);
        if(match>maxmatch)
        {
            maxmatch=match;
            possible=iter->first;
        }
    }
    cout<< "I guess your input is : "<< possible <<endl;

}

void showhelp(string hlpcmd)
{

    string tmpcmd;
    for(auto &e:hlpcmd)
        e=toupper(e);
    if(CMD.count(hlpcmd)>0)
    {
        tmpcmd=CMD[hlpcmd];
        cout<<tmpcmd<<endl;
    }
    else
    {
        for(auto iter=CMD.begin();iter!=CMD.end();++iter)
            cout<<iter->first<<":\t\t"<<iter->second<<endl;
    }


}
int interface() {
    //cout << "CLiTBot" << endl;
    showrobot();
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

            int state=0;
            string command;
            char a;
            while ((a = cin.get()) != '\n')
            {
                if(a!=' ')
                {
                    command = command + a;
                    state=1;
                }
                else
                {
                    if(state!=0)
                    {
                        command = command + a;
                    }
                }
            }
            strcpy(map_path, command.c_str());
            ifload = load(map_path);
            if (ifload) {
                char msg[] = "Map successfully loaded";
                info(msg);
            }
        }
            //autosave
        else if (strcmp(order, "AUTOSAVE") == 0) {
            int state=0;
            string command;
            char a;
            while ((a = cin.get()) != '\n')
            {
                if(a!=' ')
                {
                    command = command + a;
                    state=1;
                }
                else
                {
                    if(state!=0)
                    {
                        command = command + a;
                    }
                }
            }
            strcpy(autosave_code, command.c_str());
            if (strcmp(autosave_code, "OFF") == 0)
            {
                memset(game.save_path,0,sizeof(char)*MAX_PATH_LEN);
                auto_save();
            }

            else if (strcmp(autosave_code, "ON") == 0)
            {
                memset(game.save_path,(int)'!',sizeof(char)*MAX_PATH_LEN);
                auto_save();
            }

            else
            {
                strcpy(game.save_path, autosave_code);
                auto_save();
                cout<<game.save_path<<endl;
            }



        }
            //limit of steps
        else if (strcmp(order, "LIMIT") == 0) {
            cin >> steps_set;
            game.limit = steps_set;
        }
            //output set
        else if (strcmp(order, "STATUS") == 0) {
            if (ifload == 1) {
                if(if_run)
                    mapinfo(&game.map_run);
                else
                    mapinfo(&game.map_init);
            }
        } else if (strcmp(order, "OP") == 0) {
            int state=0;
            string command;
            char a;
            while ((a = cin.get()) != '\n')
            {
                if(a!=' ')
                {
                    command = command + a;
                    state=1;
                }
                else
                {
                    if(state!=0)
                    {
                        command = command + a;
                    }
                }
            }
            strcpy(op_path, command.c_str());
            operation(op_path);
        } else if (strcmp(order, "RUN") == 0) {
            if_run=true;
            int state=0;
            string command;
            char a;
            while ((a = cin.get()) != '\n')
            {
                if(a!=' ')
                {
                    command = command + a;
                    state=1;
                }
                else
                {
                    if(state!=0)
                    {
                        command = command + a;
                    }
                }
            }
            strcpy(op_path, command.c_str());

            if (!ifload) {
                char msg[] = "You have not loaded any map yet.";
                error(msg);
                continue;
            }
            Result result = robot_run(op_path);
            cout<<"RUN "<<op_path<<endl;
            if(result.result==LIGHT)
            {
                cout<<"result: LIGHT"<<endl;
                cout<<"congrats! you win!"<<endl;
            }
            else if(result.result==LIMIT)
            {
                cout<<"result: LIMIT"<<endl;
            }
            else if(result.result==DARK)
                cout<<"result: DARK"<<endl;
            cout << "Step(s) used:" << ' ' << result.steps << endl;
            mapinfo_run(&game.map_run);
        } else if (strcmp(order, "EXIT") == 0) {
            cout << "Quiting..." << endl;
            break;
        } else if (strcmp(order, "CD") == 0) {
            string path;
            char a;
            while ((a = cin.get()) != '\n')
                if (a != ' ')
                    path = path + a;
            if(path.size()!=0)
                std::filesystem::current_path(path);
        } else if (strcmp(order, "LS") == 0) {
            cout << ".\n.." << endl;
            auto path = std::filesystem::current_path();
            for (const auto &entry: std::filesystem::directory_iterator(path)) {
                std::cout << entry.path().filename().string() << std::endl;
            }
        } else if (strcmp(order, "TREE") == 0) {
            for (int i = 0; i < pathdepth; i++)
                endofpath[i] = false;
            path  dir = ".";
            enumdir(dir,0);
        }else if(strcmp(order, "HELP") == 0){
            string command;
            char a;
            while ((a = cin.get()) != '\n')
                if(a!=' ')
                    command = command + a;
            showhelp(command);
        }else if(strcmp(order, "CLS") == 0){
            cout<<"\e[2J\e[0;0H";
        }
        else {
            char msg[] = "Unknown command!";
            error(msg);
            getpossiblecmd(order);
        }

    }
    return 0;
}
