#include "navy.h"

const uint32_t MAX_BLOCK = UINT32_MAX;

void reassignColdActions(const char * pathin, const char * pathout) {
    Reader reader(pathin, sizeof(Access));
    Writer writer(pathout, sizeof(Access));
    Bitmap blocks(MAX_BLOCK);

    Access a;

    while (reader.read(&a)) {
        uint32_t action = a.action;
        if (blocks.get(a.block)) {
            if (action == COLD_READ || action == WARM_READ) {
                a.action = WARM_READ;
            } else if (action == COLD_WRITE || action == WARM_WRITE) {
                a.action = WARM_WRITE;
            }
        } else {
            if (action == COLD_READ || action == WARM_READ) {
                a.action = COLD_READ;
            } else if (action == COLD_WRITE || action == WARM_WRITE) {
                a.action = COLD_WRITE;
            }
            blocks.set(a.block);
        }

        writer.write(&a);
    }
    writer.close();
}
