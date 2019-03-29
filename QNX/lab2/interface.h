#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <devctl.h>
#include <sys/neutrino.h>

#define LCG_MAX_LENGHT 4096

struct LCG{
    int m, a, c, x0, size;
    char data[LCG_MAX_LENGHT];
};

const int DEVCTL_GETLCG = __DIOTF(_DCMD_MISC,0,LCG);

#endif /* INTERFACE_H_ */
