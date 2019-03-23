## File System Resource Manager
Предположительно, в одной из следующих лабораторных работ может потребоваться сделать менеджер ресурсов использующий файловую систему. Здесь приведён пример менеджера ресурсов, который монтируется в `/dev/myRand`.
Почти дословная копипаста [этого](http://www.qnx.com/developers/docs/qnxcar2/index.jsp?topic=%2Fcom.qnx.doc.neutrino.getting_started%2Ftopic%2Fs1_resmgr_read_complex.html) мануала.
Проверить работоспособность можно при помощи
```bash
ls /dev/myRand
cat /dev/myRand/int
cat /dev/myRand/double
```
Или такого исходного кода:
```c++
#include <iostream>
#include <fstream>

int main() {
    std::ifstream inInt("/dev/myRand/int"), inDouble("/dev/myRand/double");
    int t;
    double d;

    for(int i = 0; i < 10; i++)
        std::cout << (inInt >> t, t) <<" ";
    std::cout<<std::endl;
    for(int i = 0; i < 10; i++)
        std::cout << (inDouble >> d, d) <<" ";

    return 0;
}

```