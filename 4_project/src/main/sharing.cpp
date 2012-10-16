#include "run.h"

void UserCountPerBlock(const char * set) {
    o("userCountPerBlock", set, userCountPerBlock,
            "bigdat/blocksorted", "bigdat/usercount");
}

void ScatterBurstUsers(const char * set) {
    // plot out the relationship between bursty traffic and 
    // the number of active users
    o("scatterBurstUsers", set, scatterBurstUsers,
            "bigdat/actionrate", "dat/burstuser");
}
void ScatterBurstUsers(const char * set, const char * sub) {
    Line in("bigdat/%s/actionrate", sub);
    Line out("dat/burstuser/%s", sub);
    o("scatterBurstUsers", set, scatterBurstUsers, in(), out());
}
