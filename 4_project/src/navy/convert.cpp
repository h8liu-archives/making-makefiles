#include "navy.h"

void convertOne2Two(const char * pathIn, const char * pathOut) {
    Reader reader(pathIn, sizeof(Access_1));
    Writer writer(pathOut, sizeof(Access_2));

    Access_1 a1;
    Access_2 a2;

    while (reader.read(&a1)) {
        a2.time = a1.time;
        a2.user = uint32_t(a1.user);
        a2.action = uint32_t(a1.action);
        a2.block = uint64_t(a1.block);
        writer.write(&a2);
    }

    writer.close();
}

void convertTwo2Three(const char * pathIn, const char * pathOut) {
    Reader reader(pathIn, sizeof(Access_2));
    Writer writer(pathOut, sizeof(Access_3));

    Access_2 a2;
    Access_3 a3;
    a3.session = 0;

    while (reader.read(&a2)) {
        a3.time = a2.time;
        a3.user = a2.user;
        a3.action = a2.action;
        a3.block = a2.block;
        writer.write(&a3);
    }

    writer.close();
}

