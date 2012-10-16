#include "navy.h"

void retime(const char * accessIn, const char * shapeIn, 
        const char * out) {
    Reader reader1(accessIn, sizeof(Access));
    Reader reader2(shapeIn, sizeof(Access));
    panicIf(reader1.total() != reader2.total() , "count different");
    Writer writer(out, sizeof(Access));

    ProgLine progLine;

    Access a1, a2;
    while (reader1.read(&a1) && reader2.read(&a2)) {
        a1.time = a2.time;
        writer.write(&a1);
        readerProg(reader1, progLine);
    }

    writer.close();
}
