## Что это?
Здесь покоится исходный код двух приложений под ОСРВ QNX. Клиент - lab1.cpp, interface.h, func.cpp и менеджер ресурсов QNX - lab2.cpp, interface.h.
Клиент(модификация [первой лабы](https://github.com/Arin112/projectsSimple/tree/master/QNX/lab1)) умеет шифровать файлы при помощи шифра Вермана, для этого ему нужно уметь получать ПСП (Псевдо Случайную Последовательность), это он делает при помощи сервера, который монтируется в /dev/LCG и откликается на запрос devctl со структурой описанной в interface.h. Более подробно требования описаны преподавателем - ТЗ лежит в файле ТЗ.doc. Выполнение требований описано в последующих главах.
Начнём с самого главного - с тестирования.
## Тестирование
Создадим файлик с названием script в папке /tmp с таким содержимым
```bash
! rm -f in out key file decrypted
! echo "before script:\n"
! ls
create in 20 # create file with 20 symbols
crypt in out key # crypt in file with new key file and put res into out file
decrypt out decrypted key # decrypt out file with key file
! echo "\nafter script:\n"
! ls
! echo "\nin file : $(cat in)\n"
! echo "out file : $(cat out)\n"
! echo "key file : $(cat key)\n"
! echo "decrypded file : $(cat decrypted)\n"
! diff -s in decrypted
! echo now let\'s try big file
create in 50000
crypt in out key
decrypt out decrypted key
! wc -h -c in key out decrypted
! diff -s in decrypted

! rm -f lab1.x
selfCopyTo lab1.x
create inOne 10000000
! wc -h -c inOne
! echo "crypt inOne outOne keyOne\nexit\n" >temp.script
! echo "./lab1.x <temp.script" >temp.sh
! rm -f outOne keyOne
! echo time of crypting 10000000 bytes in one file
! time sh temp.sh
decrypt outOne decryptOne keyOne
! diff -s inOne decryptOne
! rm -f inOne keyOne outOne decryptOne

create in1 10000000
create in2 10000000
create in3 10000000
create in4 10000000
! wc -h -c in?
! rm -f key*
! echo "crypt in1 out1 key1\nexit\n" >temp1.script
! echo "crypt in2 out2 key2\nexit\n" >temp2.script
! echo "crypt in3 out3 key3\nexit\n" >temp3.script
! echo "crypt in4 out4 key4\nexit\n" >temp4.script
! echo "./lab1.x <temp1.script >err1 2>&1 &\n" >temp.sh
! echo "./lab1.x <temp2.script >err2 2>&1 &\n" >>temp.sh
! echo "./lab1.x <temp3.script >err3 2>&1 &\n" >>temp.sh
! echo "./lab1.x <temp4.script >err4 2>&1 \n" >>temp.sh
! echo "wait\n" >>temp.sh
! echo "time of crypting 40000000 bytes in 4 files (10000000 bytes each)"
! time sh temp.sh
! echo dectypring ...
decrypt out1 decrypt1 key1
decrypt out2 decrypt2 key2
decrypt out3 decrypt3 key3
decrypt out4 decrypt4 key4
! diff -s in1 decrypt1
! diff -s in2 decrypt2
! diff -s in3 decrypt3
! diff -s in4 decrypt4
! rm -f in? key? out? decrypt?
! echo done
exit # close program
```
Со времён предыдущей лабы запускать его стало проще! Достаточно выполнить `self <script` в клиенте. Можно, конечно, и просто копировать весь скрипт прямо в консоль самого momentics'a, почти одно и то же.
Скрипт делает следующее - для начала воспроизводит первую лабораторную работу, создвая файл на 20 байт, шифрует и дешифрует его. Это проверяет минимальную работоспособность и самой первой лабы и менеджера ресурсов.
После этого пробуем файл побольше - 50000 байт, для этого потребуется множество вызовов devctl, ведь он передаёт только по 4096 байт. Проверим размеры результирующий файлов при помощи `wc`, а совпадение входного и расшифрованного файлов при помощи `diff -s`. Теперь подготовимся к более сложной задаче - нужно засекать время выполнения команды шифрования. Делать это будем попутно с двойным фляком через левое ухо для единообразия с тестированием будущей многопоточности. Создаём файл temp.script с содержимым
```
crypt inOne outOne keyOne
exit
```
`selfCopyTo lab1.x` - копируем самого себя в lab1.x
`! echo "./lab1.x <temp.script" >temp.sh` - создаём файл temp.sh для запуска lab1.x с перенаправлением стандартного ввода из файла temp.script.
И запускаем с отсчётом времени `! time sh temp.sh` - команта time скажет время выполнения.
После этого расшифровываем файл и убеждаемся, что он совпадает с оригиналом.
Теперь нужно повторить процедуру, но уже запуская 4 копии программы одновременно для тестирования того, как поведёт себя менеджер ресурсов. За последующий скрипт необходимо оправдаться. Первое - я писать циклы в шелле QNX'а больно, так как он поддерживает далеко не все фичи bash'а. Второе - данный скрипт долгое время переписывался в надежде, что именно в нём находится один интересный баг(что оказалось неверно) и переписывать его снова в нормальное состояние у меня нет желания. Коротко - смысл тот же самый, создаётся на этот раз 4 входных файла, для каждого создаётся скрипт с командой его шифрования, после этого создаётся файл с temp.sh с содержимым
```
./lab1.x <temp1.script >err1 2>&1 &
./lab1.x <temp2.script >err2 2>&1 &
./lab1.x <temp3.script >err3 2>&1 &
./lab1.x <temp4.script >err4 2>&1 
wait
```
Который запускает параллельно 4 копии lab1.x со специально подготовленными для них скриптами и ждёт их завершения.
После этого уже своей копией дешифруем файлы и проверяем совпадение оригинальных и расшифрованных файлов.

Такой вывод показывает запуск на моём пылесосе:
```
before script:

clean.sh                               script
err1                                   temp.script
err2                                   temp.sh
err3                                   temp1.script
err4                                   temp2.script
lab1.x                                 temp3.script
lab1Denis1553802313088136              temp4.script
lab1Denis1553811155457150              testScript
lab2Denis1553807001661147              timings1.sh
oneFile.cr

after script:

clean.sh                               oneFile.cr
decrypted                              out
err1                                   script
err2                                   temp.script
err3                                   temp.sh
err4                                   temp1.script
in                                     temp2.script
key                                    temp3.script
lab1.x                                 temp4.script
lab1Denis1553802313088136              testScript
lab1Denis1553811155457150              timings1.sh
lab2Denis1553807001661147

in file : btjizdkdhoyhuyn qcgl

out file : 4шR‹Ѓзn?»#ktіЗд•7

key file : Vg’;nпкѓчВK“¶‡т[

decrypded file : btjizdkdhoyhuyn qcgl

Files in and decrypted are identical
now let's try big file
    bytes file
    50000 in
    50000 key
    50000 out
    50000 decrypted
   200000 total
Files in and decrypted are identical
    bytes file
 10000000 inOne
time of crypting 10000000 bytes in one file
    1.78s real     0.69s user     0.03s system
Files inOne and decryptOne are identical
    bytes file
 10000000 in1
 10000000 in2
 10000000 in3
 10000000 in4
 40000000 total
time of crypting 40000000 bytes in 4 files (10000000 bytes each)
    9.26s real     4.12s user     0.22s system
dectypring ...
Files in1 and decrypt1 are identical
Files in2 and decrypt2 are identical
Files in3 and decrypt3 are identical
Files in4 and decrypt4 are identical
done
```
Можно было бы предположить, что время работы во втором случае должно не намного превышать время работы в первом, но это не так. Это говорит о том, что узкое место заключается не в долгом ожидании ПСП. Глядя на исподный код можно предположить, что узкое место - однопоточное копирование, причём неоднократное. Об это говорит и загрузка процессора, которая почти всегда находится на уровне одного ядра, в отличии от изначальной версии первой лабораторной работы. Тем не менее, тестирование можно считать успешным, менеджер ресурсов в несколько потоков обрабатывает сообщения от нескольких клиентов, верно выдавая необходимую ПСП, при этом и шифрование методом Вермана работает успешно. Избавиться от этого копирования можно, но учитывая требования ТЗ к интерфейсу и вообще использование devctl это просто приведёт к тому, что вся программа будет написана на Си. Не люблю Си. Поэтому это не сделано.

## Задача
1. Реализовать функцию devctl в разрабатываемом АР, предусмотрев вызов на клиентской стороне.(1) Включить в реализацию клиента и сервера общий заголовочный интерфейсный файл.(2) Убедиться, что функция на сервере вызывается при поступлении сообщений от клиента.(3)
	1.1. Функция io_devctl описана в файле lab1.cpp.
	1.2. Файл interface.h.
	1.3. Это происходит за счёт проверок на стороне клиента.
2. Реализовать в серверном коде функцию генерации ПСП с помощью ЛКМ. Функция должна принимать на вход параметры начального состояния (seed): модуль, множитель, приращение и нулевой элемент ПСП, а также размер генерируемой ПСП. Функция должна возвращать полученную ПСП в виде POD динамического массива.
	2.2 Что подразумевается под "POD динамическим массивом", вообще говоря, не ясно. Но функция genLCG это делает. Зачем-то. Хотя нет, с описанным интерфейсом всё ясно. Это нужно только для лишнего копирования. Пусть будет.
3. Предусмотреть ограничения на размер максимально возможной последовательности (с обоснованием этого размера), либо организовать обмен через многосоставный input-output вектор.
	3.1 Избран промежуточный вариант. Потому что нет никакого смысла заморачиваться с многосоставным вектором, учитвая, что обмен ПСП вообще не должен осуществлятся при помощи devctl(см. пункт Контрольные вопросы 3.2). В качестве максимального размера ПСП выбрано число 4096 как максимальная степень двойки не вызывающая "Server fault on msg pass". При внимательном чтении документации может сложиться впечатление, что максимальный размер сообщения составляет 2^14 - 1, но это не так, ведь есть ещё resmgr_attr.msg_max_size(установлено в 8192). Под промежуточным вариантом понимается следующий алгоритм : если требуемый размер ПСП меньше 4096, то достаточно обычного вызова devctl, в противном случае клиент вызывает devctl, после этого копирует полученную ПСП, при этом сервер обновляет значение x0 в переданных параметрах ЛКМ, что позволяет клиенту сразу же вызвать devctl снова, продолжая генерацию ПСП с того места, на котором сервер остановился. Затем клиент снова копирует полученную ПСП и цикл повторяется пока не будет сгенерирована ПСП нужной длинны. Процесс реализован в функции getLCG в файле lab1.cpp. Выбор пал именно на такой алгоритм по следующим причинам.
		3.1.1. devctl не способен передавать большие объёмы данных, хотя механизм обмена сообщениями в QNX в теории может передавать данные размером до 2^31-1.
		3.1.2. Нежеление использовать devctlv, так как в ТЗ дан пример именно devctl.
		3.1.3. Использование двух типов сообщений для установки параметров ЛКМ и чтения ПСП крайне затруднительно в условиях многопоточной работы с несколькими клиентами. Даже просто в условиях работы с несколькими клиентами.
4. Реализовать команду devctl для генерации ПСП с помощью ЛКМ. Команда должна передавать(?) данные (начальные параметры генератора и размер ПСП) и возвращать клиенту ПСП(1). Для обмена данными реализовать структуру, прототип которой должен быть объявлен в интерфейсном файле(2).
	4.0. (?) - по всей видимости принимать, а не передавать. Иначе не ясно, речь идёт в данном моменте про клиент или про сервер.
	4.1. io_devctl в lab2.cpp этим и занимается.
	4.2. Файл interface.h, структура LCG.
5. Протестировать полученный АР посредством вызова с клиентской стороны(1), передачи данных, контроля корректности получения данных сервером и возвращения ПСП клиенту(2), в роли которого выступает программа, разработанная в результате выполнения Лабораторной Работы 1(3).
	5.1. См. главу Тестирование. 
	5.2. И в коде клиента, и в коде сервера есть проверки возвращемых значений требуемых функций, текстовый вывод ошибок и аварийное завершение.
	5.3. Программа, разработанная в результате выполнения Лабораторной Работы 1 была модифицирована под использование devctl, кроме того были добавлены дополнительные команды self и selfCopyTo для более удобного тестирования.
6. Программа-клиент использует ПСП, полученную для от АР, в качестве ключа для шифрования сообщения методом Вернама.
	6.1 Реализовано, см. функцию crypt в lab1.cpp.
7. Все функции в исходном коде должны быть документированы в формате Doxygen, с заполнением набора полей, приведённого в примере.
	7.1 Все функции второй лабораторной работы (lab2.cpp) документированы в соответствии с требованиями.
	7.2 Выхлоп doxygen'а можно посмотреть [тут](https://arin112.github.io/projectsSimple/lab2_8cpp.html)
	
8. Рекомендуется применение техник защитного программирования. Грамотность и красота исходного кода учитывается как один из пунктов при оценке.
	8.1 Как есть.
## Контрольные вопросы
1. Какой системный механизм QNX Neutrino лежит в основе взаимодействия клиент-АР?
    1.1 [Обмен сообщениями](http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_getting_started%2Fs1_msg.html).
2. Что из себя представляет файловый дескриптор в контексте этого взаимодействия?
	2.1 Идентификатор менеджера ресурсов и номер соединения у самого менеджера.
3. Какие недостатки данной реализации вы можете назвать?
	3.1. Интерфейс не соответстует [обещанию](http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_sys_arch%2Fresource.html) "a clean and well-defined interface", если говорить про сам devctl.
	3.2. Если говорить про саму программу, то генерация ПСП при помощи обмена сообщениями с сервером посредством devctl очень плохая идея, сам интерфейс devctl кричит о том, что его не надо так использовать. Размер сообщения devctl очень желательно должен быть фиксирован(если не использовать devctlv), что приводит либо к тому, что пересылаются лишние байты, либо к тому, что нельзя обработать файл даже сравнительно небольшого размера. Обходить эту проблему при помощи множества сообщений devctl затруднительно из-за многопоточности и из-за возможности работы с несколькими клиентами одновременно, поэтому лучше всё-таки завершать всю работу за один вызов devctl. Лучшей альтернативой использованию devctl является использование [менеджера ресурсов с файловой системой](http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_resmgr%2Ffsystems.html), что позволило бы, например, пользователю просто открыть на чтение сразу нужный "файл" по нужному "адресу" например так `open("/dev/LCG/123/21/256/3", O_RDONLY)`, где 123, 21, 256, 3 это соответственно a, c, m, x0 - параметры ЛКМ, а размер последовательности контролируется пользователем за счёт чтения нужного числа байт, что может, в случае необходимости, быть разбито на несколько порций чтения, что потребует синхронизации при помощи RESMGR_OCB_T->offset и map'а хранящего последнее значение ЛКМ для каждого последнего offset'а и мьютекса на добавление & удаление элементов map'а(из-за многопоточности).
4. Нарисуйте диаграмму последовательности функционирования многопоточного АР
  1. Вот как это ![картинка](http://www.qnx.com/developers/docs/6.5.0/topic/com.qnx.doc.neutrino_resmgr/images/dispatch_layer.jpg)
  Но при этом функции из блока blocking function могут быть переопределены или использованы стандартные,
  одновременно несколько функций могут обрабатывать события за счёт неблокирующего вызова Handler function,
  конкретные параметры зависят от атрибутов пула потоков.
5. Какие ограничения имеет ваша программа?
	5.1 Соответствие входных воздействий ТЗ. (в противном случае в ответ на собщение будет возвращено ENOSYS).
	5.2 Не больше 50-ти клиентов, 51-ый будет ждать, пока кто-то из потоков обработки освободится. (хотя ждать не долго, ведь сообщения не большие)
	5.3 Размер запрашиваемой строки должен быть больше нуля и меньше либо равен 4096
	5.4 m в ЛКМ не может быть равен нулю
6. Пройдёт ли ваша программа проверку методом fuzzing testing?
	6.1 Конечно!
7. Расскажите об основных параметров пула потоков.
	7.1 lo_water - сколько потоков минимум всегда должны ожидать прихода пользовательского сообщения.
	7.2 increment - Сколько потоков могут быть созданы единовременно для достижения lo_water.
	7.3 hi_water - Максимальное количество потоков ожидающих прихода пользовательского сообщения.
	7.4 maximum - Сколько всего одновременно может быть потоков.