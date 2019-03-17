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

const int N(sysconf(_SC_NPROCESSORS_ONLN)); // запомним сколько всего процессорных ядер

class Parser { // поможет разобраться с входящими команадми
private:
    string cur; // буфер в котором лежат уже прочитанные, но ещё не обработанные команды
public:
    bool eat(string str) { // попытка съесть из буфера str,
        // вернёт true и удалит из буфера str если он там лежит
        // ничего не делает и возвращает false в противном случае
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
        // съедает следующее слово с буфера, если оно есть и возвращает его как результат
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
        // просто возвращает весь текущий буфер и очищает его
        string str = cur;
        cur = "";
        return str;
    }

    int getInt() {
        // возвращает int от следующего слова.
        // если был встречен не int то в никуда съедает слово и это бы пофиксить...
        return atoi(getString().c_str());
    }
};

template<typename T>
void add(T t){} // в gcc 4.8.3 списки инициализации не полноценные, так что напишем свою функцию
// чтобы через запятую добавлять элементы в вектор

template<typename T, typename V, typename ...Args>
void add(T &t, V v, Args ...args){
    t.push_back(v); // производительность понятна,
    // но эта функция не будет вызываться от ста тысяч элементов в любом случае
    add(t, args...);
}

struct fnct { // схороним команду, её описание и присущию ей функцию в эту структурку
    fnct(string cmd_, string desc_, function<void()> f_) :
        cmd(cmd_), desc(desc_), f(f_) {
    }
    string cmd, desc;
    function<void()> f; // захват контекста работает ))
};

class parExecutor { // поможет с параллельным исполнением
    // решение инкапсулировать в класс было принято для большей масштабируемости.
    // если, например, надо будет написать функцию, которая принимает произвольное количество функций
    // и запускает их параллельно, то без класса уже не обойтись (иначе лишние функции будут торчать)
private:
    struct context { // для передачи контекста в параллельно исполняемую функцию
        parExecutor *p;
        int id; // будет использовано функцией для того,
        // чтобы понять какую часть работы необходимо выполнить
    };

    function<void(int)> f; // функция, что будет исполняться параллельно
    pthread_barrier_t barrier; // по заданию лабы надо использовать барьер для синхронизации

    static void* fnc(void* cont) { // вспомогательная функция,
        // которая будет передана в pthread_create, всё из-за того, что pthread может принимать только
        // чистые указатели на функцию, спасибо posix (на самом деле просто либа написана на Си)
        int id = ((context*) cont)->id; // извлечём из контекста id
        ((context*) cont)->p->f(id); // и вызовем основную функцию с этим id
        pthread_barrier_wait(&((context*) cont)->p->barrier); // подождём, пока все наши клоны тоже дойдут
        // до этой точки
        pthread_exit(0); // выйдем так как будто всё хорошо
    }

public:
    parExecutor() {
        pthread_barrier_init(&barrier, NULL, N + 1); // инициализирум барьер не забывая,
        // что основной поток тоже поток и выховет pthread_barrier_wait, так что N + 1
    }
    void operator()(function<void(int)> f_) {
        f = f_;
        pthread_t *threads = new pthread_t[N]; // создадим массив под хранение данных о наших потоках
        int i; // объявим тут, так как указатель на эту переменную будет использоваться до конца функции
        context *c = new context[N]; // и контекст для каждого из них тоже создадим
        for (i = 0; i < N; i++) {
            c[i].p = this;  // запомним в контекст указатель на этот экземпляр
            c[i].id = i;    // запомним номер потока
            pthread_create(threads + i, NULL, fnc, (void*) &c[i]); // и запустим
        }
        pthread_barrier_wait(&barrier); // подождём пока все запущенные потоки отработают
        delete[] c;     // не забудем освободить память
        delete[] threads; //  ведь нормальных умных указателей ещё не завезли в этой версии gcc
    }
};

void parallel(function<void(int)> f) {
    // маленький помощник для того чтобы не создавать parExecutor руками
    parExecutor p;
    p(f); // многие не любят этот оператор. я - не многие.
}

std::ifstream::pos_type filesize(string filename) {
    // для проверки на равенство длинн файла для шифровки
    // и файла ключа
    std::ifstream in(filename.c_str(), std::ifstream::ate
            | std::ifstream::binary);
    return in.tellg();
}
string readFile(string file) { // помогало, которое закинет содержимое файла в строку
    ifstream t(file.c_str(), ios::binary); // не забудем читать в бинарном виде
    stringstream buffer;
    buffer << t.rdbuf();
    string str = string(buffer.str());
    t.close();
    return str;
}

void writeFile(string file, string str) { // помогало, что запишет string в файл
    ofstream out(file.c_str(), ios::binary | ios::trunc); // предварительно удалив содержимое файла(trunc)
    out << str;
    out.close();
}

void crypt(string inS, string outS, string keyS) { // тут будем шифровать
    // inS - имя входного файла
    // outS - имя выходного файла
    // keyS - имя файла с ключом
    string key; // строка для хранения ключа
    if (filesize(inS) != filesize(keyS)) { // не совпал размер файла ключа и файла для шифровки?
        // создадим новый ключ с нуля, перезаписав старый ключ
        //cout << "creatin new key..." << endl;
        int size = filesize(inS); // сколько содержит файл?
        ofstream out(keyS.c_str(), ios::trunc | ios::binary);
        key.resize(size); // столько и будет в ключе
        parallel([&](int id) {
                    for(int i = 0; i*N + id < key.size(); i++) {
                        key[i*N + id] = char(rand());
                        // i*N+id - два разных потока никогда
                        // не обратятся к одному элементу,
                        // все элементы будут охвачены
                    }
                });
        // вообще говоря имеет смысл одновременно с генерацией ключа ещё файл шифровать
        // но я не хочу, поэтому это TODO
        // вечное, вероятно, никогда не будет реализовано (хотя это элементарно)
        writeFile(keyS, key); // запишем ключ
        //cout << "done" << endl;
    } else {
        key = readFile(keyS); // а если размеры совпали, то используем этот ключ
    }
    //cout << "crypting " << inS << "..." << endl;
    string in = readFile(inS), out; // строки для входных и выходных данных
    out.resize(key.size());         // растянем строку до нужного размера
    parallel([&](int id) {
                for(int i=0;i*N+id<key.size();i++) {
                    out[i*N + id]=in[i*N + id]^key[i*N + id];
                    // собственно, шифруем
                }
            });
    writeFile(outS, out); // запишем результат
    //cout << "done" << endl;
}

void decrypt(string inS, string outS, string keyS) { // тут будем расшифровывать
    // процессы шифрования и расшифровки не отличаются принципиально,
    // но у них различная реакция на несоответствие размеров файла keyS и inS
    // inS - имя входного файла
    // outS - имя выходного файла
    // keyS - имя файла с ключом
    string key;
    if (filesize(inS) != filesize(keyS)) { // в данном случае это ошибка
        cout << "can't decrypt \"" << inS << "\" with \"" << keyS
                << "\" cause of different file size" << endl;
        return;
    } else {
        key = readFile(keyS); // размеры совпали? значит всё хорошо, запомним ключ
    }
    //cout << "decrypting " << inS << "..." << endl;
    string in = readFile(inS), out;
    out.resize(key.size()); // можно и в конструкторе, но вроде(!) как resize быстрее
    parallel([&](int id) {
                for(int i=0;i*N+id<key.size();i++) {
                    out[i*N+id]=in[i*N+id]^key[i*N+id];
                    // как видно дешифровка эта таже операция
                    // просто потому что a^c^c ==  a
                }
            });
    writeFile(outS, out); // запишем результат
    //cout << "done" << endl;
}

void create(string outS, int count) {
    // для того чтобы не писать руками файл, который будем шифровать
    // напишем функцию, которая сама создаёт такой файл
    //cout << "creating " << outS << ", " << count << endl;
    string out;
    out.resize(count);
    parallel([&](int id) {
        // почему бы не делать это параллельно, раз уж можно?
                for(int i=0; i*N+id < count;i++) {
                    out[i*N+id] = *(" qweasdzxcrtyfghvbnuiojklmp" + rand()%27);
                    // что вообще означет *("123" + 2) ? очевидно '3' так как
                    // строка в кавычках это просто const char * на первый символ в кавычках,
                    // то есть указатель,
                    // к указателю можно прибавить число и это снова будет указатель на элемент правее.
                    // разыменовав указатель получим символ
                }
            });
    writeFile(outS, out); // запишем результат
}

void print(vector<fnct> funcs) { // должна называться printHelp (TODO)
    cout << "Usage: " << endl;
    for(auto i:funcs) { // маленький кусочек C++11 внезапно работающего с++11
        // когда 11'ый стандарт только вышел, все статьи пихали это как первое отличие от С++03
        // хотя это просто синтаксический сахар, который, по факту, никаких новых возможностей не даёт
        // так ещё и на некоторых компиляторах транслируется как макрос практически, что приводит
        // к вырвиглазному поведению в некоторых ситуациях
        cout<<i.desc<<endl<<endl;
    }
}

int main(int argc, char *argv[]) {
    //srand(time(0)); // оставим это закомменченым, для воспроизводимости
    Parser p;
    vector<fnct> funcs; // тут сохраним все имеющиеся команды
    add(funcs, // этот вызов не требует комментариев
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
    while (1) { // пока не вызовем exit( );
        bool valid = false; // для проверки  - была ли выполнена валидная команда
        for (auto &i:funcs) {
            if(p.eat(i.cmd)) {  // если удалось съесть название команды
                i.f();          // то выполнить команду.
                valid=true;     // запомним, что команда была валидной.
                break;          // break не обязателен сейчас,
                                // но при дальнейшей модификации может потребоваться
            }
        }
        if (!valid) { // если не была выполнена валидная команда
            // значит в буфере лежит что-то непонятное
            cout << "invalid command! \"" << p.getString() << "\"" << endl; // скажем об этом
            print(funcs);   // напомним usage
            exit(1);        // и застрелимся
        }
    }
    return 0;
}
