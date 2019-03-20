## Resource Manager
Предположительно, в одной из следующих лабораторных работ может потребоваться сделать менеджер ресурсов. Здесь приведён пример менеджера ресурсов, который монтируется в `/dev/getRndInt`.
Написано при помощи [этого](http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_resmgr%2Fabout.html) мануала.
Проверить работоспособность можно при помощи `cat /dev/getRndInt` или следующего исходного кода:
```c++
#include <iostream>
#include <fstream>

int main() {
    std::ifstream in("/dev/getRndInt");

    for(int i = 0, t; i < 10; i++)
        std::cout << (in >> t, t) <<" ";

	return 0;
}

```
