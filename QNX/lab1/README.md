## Первая лабораторная работа по QNX
Стоит отметить, что все действия выполняются в QNX Momentics IDE 4.7 и на ОСРВ QNX 6.6. IDE была модифицирована - [отсюда](http://community.qnx.com/sf/frs/do/viewRelease/projects.toolchain/frs.binutils.binutils_2_21) и [отсюда](http://community.qnx.com/sf/frs/do/viewRelease/projects.toolchain/frs.gcc.gcc_4_8) (да, надо регаться) были скачаны обновления, позволяющие ~~жрать кактус~~ пользоваться С++ 11 хотябы в урезанной версии за счёт gcc 4.8.3. Так как версия урезанная, в этом и дальнейшех проектах можно будет видеть рукописные/скопированные/допиленные напильником ~~велосипеды~~ реализации стандартных функций из C++ 11, а также костыли, которые обходят ошибки компилятора.

К выполнению были предъявлены следующие требования
* Работоспособность проверяется на ОСРВ QNX.
* Программа может зашифровать файл [шифром Вермана](https://ru.wikipedia.org/wiki/%D0%A8%D0%B8%D1%84%D1%80_%D0%92%D0%B5%D1%80%D0%BD%D0%B0%D0%BC%D0%B0).
* При этом файли ключа либо генерируется случайно либо создаётся программой. 
* Тем же алгоритмом программа должна уметь расшифровать файл.
* Шифрование и дешифровка должны происходить в несколько потоков - столько, сколько на исполняющем компьютере есть процессорных ядер.
* Синхронизация исполняющих потоков должна происходить при помощи [pthread_barrier](https://learnc.info/c/pthreads_barriers.html)

На этом перечисленные на паре требования заканчиваются. Как видно требования сформулированны довольно свободно, поэтому у меня в программе будет присутствовать дополнительный функционал.

Все команды поступают на стандартный поток ввода. Разделитель - перевод строки.
"#" - комментарий до конца строки. Доступные команды:
* crypt file1 file2 file3 - зашифровать file1 при помощи file3, если он существует и совпадает с ним размером, в противном случае сгенерировать file3 как ключ. Результат шифрования положить в file2
* decrypt file1 file2 file3 - расшифровать file1 при помощи file3, если они разного размера - выдать ошибку, результат удачной расшифровки положить в file2
* create file int - создать file с int символов английского алфавита и пробелами
* ! - всё до конца строки после этого символа будет исполнено как команда в терминале
* help - вывести usage (не этот, на английском)
* exit - корректное завершение программы

Так как все команды поступают на стандартный поток ввода, будет не сложно написать скрипт и перенаправить входной поток из файла. Напишем файл с названием script в папке /tmp, куда momentics по умолчанию кладёт исполняемый файл.
```bash
help # show help
! sh clean.sh
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
exit # close program
```
Дополнительно напишем файл clean.sh
```bash
rm -f in out key file decrypted
```
Теперь необходимо скопировать временный файл, который использует momentics в другое имя. Для этого надо прейти в папку /tmp "`cd /tmp`", после этого запустить программу в momontics'е, набрать "`ls`", найти имя временного файла (например helloDenis1242534654534532454 у меня, т.к. проект назывался hello), выполнить "`cp helloDenis1242534654534532454 h`". Теперь можно закрывать программу в momentics'e.
Запустим теперь программу при помощи "`./h < script | less`"
Возможный вывод:
```
Usage: 
crypt inFile resFile keyFile - encrypt inFile with keyFile if it exist or create it and generate random key. Write result to resFile.

decrypt inFile resFile keyFile - decode inFile with keyFile. Write result to resFile

create filename INT - create filename with INT chars of eng alphabet + spaces.

! COMMAND - execute COMMAND in shell.

# - comment to end of line.

help - write this help.

exit - exit the programm with 0 return code.

before script:

clean.sh                               script
h
helloDenis155281445600337

after script:

clean.sh                               key
decrypted                              out
h                                      script
helloDenis155281445600337
in

in file : btjizdkdhoyhuyn qcgl

out file : 4Vг2¶g6“н2У8Ѓлtґ

key file : Vf<{r‡YТYк…GЄVЎg€Ш

decrypded file : btjizdkdhoyhuyn qcgl
```
Что вполне демонстрирует работоспособность программы.
