#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <cctype>
#include <unistd.h>
#include <vector>
#include <functional>
#include <pthread.h>
#include <sstream>
#include <time.h>

#include "func.hpp"

using namespace std;

/*  Лабораторная работа №1 по QNX
 *  Требования - написать программу, к
 *
 *
 * */

const int N(sysconf(_SC_NPROCESSORS_ONLN));

class Parser {
private:
    string cur;
public:
    bool eat(string str) {
        if (cur == "") {
            getline(cin, cur);
        }
        int index = 0;
        while (index < cur.size() && isspace(cur[index])) {
            index++;
        }
        string s;
        while (index < cur.size() && (!isspace(cur[index]))) {
            s += cur[index];
            index++;
        }
        if (s == str) {
            cur.erase(0, index);
            return true;
        }
        return false;
    }

    string getString() {
        int index = 0;
        while (index < cur.size() && isspace(cur[index])) {
            index++;
        }
        string str;
        while (index < cur.size() && (!isspace(cur[index]))) {
            str += cur[index];
            index++;
        }
        cur.erase(0, index);
        return str;
    }

    string getTail() {
        string str = cur;
        cur = "";
        return str;
    }

    int getInt() {
        return atoi(getString().c_str());
    }
};

template<typename T>
void add(T t){}

template<typename T, typename V, typename ...Args>
void add(T &t, V v, Args ...args){
    t.push_back(v);
    add(t, args...);
}

struct fnct {
    fnct(string cmd_, string desc_, function<void()> f_) :
        cmd(cmd_), desc(desc_), f(f_) {
    }
    string cmd, desc;
    function<void()> f;
};

class parExecutor {
private:
    struct context {
        parExecutor *p;
        int id;
    };

    function<void(int)> f;
    pthread_barrier_t barrier;

    static void* fnc(void* cont) {
        int id = ((context*) cont)->id;
        ((context*) cont)->p->f(id);
        pthread_barrier_wait(&((context*) cont)->p->barrier);
        pthread_exit(0);
    }

public:
    parExecutor() {
        pthread_barrier_init(&barrier, NULL, N + 1);
    }
    void operator()(function<void(int)> f_) {
        f = f_;
        pthread_t *threads = new pthread_t[N];
        int i;
        context *c = new context[N];
        for (i = 0; i < N; i++) {
            c[i].p = this;
            c[i].id = i;
            pthread_create(threads + i, NULL, fnc, (void*) &c[i]);
        }
        pthread_barrier_wait(&barrier);
        delete[] c;
        delete[] threads;
    }
};

void parallel(function<void(int)> f) {
    parExecutor p;
    p(f);
}

std::ifstream::pos_type filesize(string filename) {
    std::ifstream in(filename.c_str(), std::ifstream::ate
            | std::ifstream::binary);
    return in.tellg();
}
string readFile(string file) {
    ifstream t(file.c_str(), ios::binary);
    stringstream buffer;
    buffer << t.rdbuf();
    string str = string(buffer.str());
    t.close();
    return str;
}

void writeFile(string file, string str) {
    ofstream out(file.c_str(), ios::binary | ios::trunc);
    out << str;
    out.close();
}

void crypt(string inS, string outS, string keyS) {
    string key;
    if (filesize(inS) != filesize(keyS)) {
        //cout << "creatin new key..." << endl;
        int size = filesize(inS);
        ofstream out(keyS.c_str(), ios::trunc | ios::binary);
        key.resize(size);
        parallel([&](int id) {
                    for(int i=0;i*N+id<key.size();i++) {
                        key[i*N+id]=char(rand());
                    }
                });
        writeFile(keyS, key);
        //cout << "done" << endl;
    } else {
        key = readFile(keyS);
    }
    //cout << "crypting " << inS << "..." << endl;
    string in = readFile(inS), out;
    out.resize(key.size());
    parallel([&](int id) {
                for(int i=0;i*N+id<key.size();i++) {
                    out[i*N+id]=in[i*N+id]^key[i*N+id];
                }
            });
    writeFile(outS, out);
    //cout << "done" << endl;
}

void decrypt(string inS, string outS, string keyS) {
    string key;
    if (filesize(inS) != filesize(keyS)) {
        cout << "can't decrypt \"" << inS << "\" with \"" << keyS
                << "\" cause of different file size" << endl;
        return;
    } else {
        key = readFile(keyS);
    }
    //cout << "decrypting " << inS << "..." << endl;
    string in = readFile(inS), out;
    out.resize(key.size());
    parallel([&](int id) {
                for(int i=0;i*N+id<key.size();i++) {
                    out[i*N+id]=in[i*N+id]^key[i*N+id];
                }
            });
    writeFile(outS, out);
    //cout << "done" << endl;
}

void create(string outS, int count) {
    //cout << "creating " << outS << ", " << count << endl;
    string out;
    out.resize(count);
    parallel([&](int id) {
                for(int i=0; i*N+id < count;i++) {
                    out[i*N+id] = *(" qweasdzxcrtyfghvbnuiojklmp" + rand()%27);
                }
            });
    writeFile(outS, out);
}

void print(vector<fnct> funcs) {
    cout << "Usage: " << endl;
    for(auto i:funcs) {
        cout<<i.desc<<endl<<endl;
    }
}

int main(int argc, char *argv[]) {
    //srand(time(0));
    Parser p;
    vector<fnct> funcs;
    add(funcs,
            fnct( "crypt",
                    "crypt inFile resFile keyFile - encrypt inFile with keyFile "
                    "if it exist or create it and generate random key. Write result to resFile.",
                    [&]() {auto t1 = p.getString();auto t2 = p.getString();auto t3 = p.getString();
                        crypt(t1, t2, t3);}
            ),
            fnct( "decrypt",
                    "decrypt inFile resFile keyFile - decode inFile with keyFile. "
                    "Write result to resFile",
                    [&]() {auto t1 = p.getString();auto t2 = p.getString();auto t3 = p.getString();
                        decrypt(t1, t2, t3);}
            ),
            fnct( "create",
                    "create filename INT - create filename with INT chars of eng alphabet + spaces.",
                    [&]() {auto t1 = p.getString();auto t2 = p.getInt();
                        create(t1, t2);}
            ),
            fnct( "!",
                    "! COMMAND - execute COMMAND in shell.",
                    [&]() {system(p.getTail().c_str());}
            ),
            fnct( "#",
                    "# - comment to end of line.",
                    [&]() {p.getTail();}
            ),
            fnct( "help",
                    "help - write this help.",
                    [&]() {print(funcs);}
            ),
            fnct( "exit",
                    "exit - exit the programm with 0 return code.",
                    [&]() {exit(0);}
            )
    );
    while (1) {
        bool valid = false;
        for (auto &i:funcs) {
            if(p.eat(i.cmd)) {
                i.f();
                valid=true;
                break;
            }
        }
        if (!valid) {
            cout << "invalid command! \"" << p.getString() << "\"" << endl;
            print(funcs);
            exit(1);
        }
    }
    return 0;
}
