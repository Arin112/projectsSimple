#define THREAD_POOL_PARAM_T dispatch_context_t
#include <cstdlib>
#include <iostream>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <limits.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <interface.h>

static resmgr_connect_funcs_t connect_funcs;
static resmgr_io_funcs_t io_funcs;
static iofunc_attr_t attr;

/*!
 * \fn inline T min(T a, T b)
 * \brief нахождение минимума
 * \param a, b элементы минимум которых надо найти
 * \return минимум из a и b
 */
template<typename T>
inline T min(T a, T b) { // ни в одном из включенных файлов нет этой простой функции
    return a < b ? a : b;
}

/*!
 * \fn char* genLCG(int a, int c, int m, int x0, int size)
 * \brief генерирует ПСП по заданным параметрам. Тут проверка параметров на корректность не происходит.
 * \param a множитель (0 <=a < m)
 * \param c приращение (0 <= c < m)
 * \param m модуль (0 <= m)
 * \param x0 (0 <= x0 < m)
 * \param size длинна требуемой ПСП
 * \return raw поинтер на ПСП длинны size, с нулём на конце (массив длинны size+1) или 0
 */
char* genLCG(int a, int c, int m, int x0, int size) {
    if (!m || size <= 0) // чтоб не падать
        return 0;
    int i, x = x0;
    char *res = new char[size + 1];
    for (i = 0; i < size; i++) {
        res[i] = x;
        x = (a * x + c) % m;
    }
    res[i] = 0;
    return res;
}

/*!
 * \fn int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb)
 * \brief функция обработки сообщения devctl
 * \param ctp контекст
 * \param msg сообщение
 * \param ocb он же Open Control Block - структура с информацией об открытом файле
 * \return код ошибки или _RESMGR_NOREPLY
 */
int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb) {

    int status;
    if ((status = iofunc_devctl_default(ctp, msg, ocb)) != _RESMGR_DEFAULT) {
        return (status);
    }

    switch (msg->i.dcmd) {

    case DEVCTL_GETLCG: {
        using std::cerr;
        using std::endl;
        LCG *lcgIn = (LCG *) _DEVCTL_DATA(msg->i);
        LCG *lcgOut = (LCG *) _DEVCTL_DATA(msg->o);
        // вообще говоря это указатели на один и тот же участок памяти

        if (lcgIn->m == 0) // не делите на 0, пожалуйста
            return ENOSYS;
        if (lcgIn->size <= 0 || lcgIn->size > LCG_MAX_LENGHT) // и не выходите за рамки дозволенного
            return ENOSYS;

        char *res =
                genLCG(lcgIn->a, lcgIn->c, lcgIn->m, lcgIn->x0, lcgIn->size);

        for (int i = 0; i < lcgIn->size; i++) {
            lcgOut->data[i] = res[i]; // да, давайте копировать руками, никаких стандартных функций
        }
        lcgOut->data[lcgIn->size] = 0;
        delete[] res;

        lcgOut->x0 = (lcgIn->a * lcgOut->data[lcgIn->size - 1] + lcgIn->c)
                % lcgIn->m;
        // супер удобная штука, можно продолжвать с того же места где закончили.
        // была бы ещё удобнее, если бы не интерфейс genLCG, жестко фиксированный в ТЗ

        msg->o.nbytes = sizeof(LCG);
        int rpl = MsgReply(ctp->rcvid, EOK, &msg->o, sizeof(msg->o)
                + sizeof(LCG));
        if (rpl != EOK) {
            cerr << "error on MsgReply : (" << rpl << ")" << strerror(rpl)
                    << endl;
        }
        break;
    }
    default:
        return ENOSYS;
    }
    return _RESMGR_NOREPLY;
}

/*!
 * \fn int main(int argc, char **argv)
 * \brief главная функция АР
 * \param argc
 * \param argv
 * \return 0
*/
int main(int argc, char **argv) {
    /* declare variables we'll be using */
    thread_pool_attr_t pool_attr;
    resmgr_attr_t resmgr_attr;
    dispatch_t *dpp;
    thread_pool_t *tpp;
    //dispatch_context_t *ctp;
    int id;

    /* initialize dispatch interface */
    if ((dpp = dispatch_create()) == NULL) {
        std::cout << "rofl" << std::endl;
        fprintf(stderr, "%s: Unable to allocate dispatch handle.\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* initialize resource manager attributes */
    memset(&resmgr_attr, 0, sizeof resmgr_attr);
    resmgr_attr.nparts_max = 1;
    resmgr_attr.msg_max_size = 8192; // с запасом

    /* initialize functions for handling messages */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS,
            &io_funcs);
    io_funcs.devctl = io_devctl;

    /* initialize attribute structure used by the device */
    iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);

    /* attach our device name */
    id = resmgr_attach(dpp, &resmgr_attr, "/dev/LCG", _FTYPE_ANY, 0,
            &connect_funcs, &io_funcs, &attr);
    if (id == -1) {
        fprintf(stderr, "%s: Unable to attach name.\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* initialize thread pool attributes */
    memset(&pool_attr, 0, sizeof pool_attr);
    pool_attr.handle = dpp;
    pool_attr.context_alloc = dispatch_context_alloc;
    pool_attr.block_func = dispatch_block;
    pool_attr.unblock_func = dispatch_unblock;
    pool_attr.handler_func = dispatch_handler;
    pool_attr.context_free = dispatch_context_free;
    pool_attr.lo_water = 4;
    pool_attr.hi_water = 5;
    pool_attr.increment = 1;
    pool_attr.maximum = 50;

    /* allocate a thread pool handle */
    if ((tpp = thread_pool_create(&pool_attr, POOL_FLAG_EXIT_SELF)) == NULL) {
        fprintf(stderr, "%s: Unable to initialize thread pool.\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* start the threads, will not return */
    thread_pool_start(tpp);
}
