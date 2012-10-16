#include "run.h"

void Mirror(const char * set, const char * from, const char * to) {
    Line outputDir("bigdat/s/%s/%s", set, to);
    mkdirs(outputDir());

    Line input("bigdat/s/%s/%s/timeline", set, from);
    Line temp("bigdat/s/%s/%s/timeline_unordered", set, to);
    Line draft("bigdat/s/%s/%s/draft", set, to);
    Line output("bigdat/s/%s/%s/timeline", set, to);

    printf("[shiftSession] %s -> %s\n", input(), temp());
    shiftSession(input(), temp());

    printf("[hashSortIntoTimeline] %s -> %s\n", temp(), draft());
    hashSortIntoTimeline(temp(), draft());

    printf("[remarkAction2] %s -> %s\n", draft(), output());
    remarkAction2(draft(), output());
}

void MirrorRemarkX1(const char * set) {
    Line timeline("bigdat/s/%s/x1/timeline", set);
    Line output("bigdat/s/%s/x1/timeline_remarked", set);
    
    printf("[remarkAction2] %s -> %s\n", timeline(), output());
    remarkAction2(timeline(), output());

    Line draft("bigdat/s/%s/x1/draft", set);
    Line cmd;
    cmd.format("mv %s %s", timeline(), draft());
    printf("$ %s\n", cmd());
    int ret = system(cmd());
    assert(ret == 0);

    cmd.format("mv %s %s", output(), timeline());
    printf("$ %s\n", cmd());
    ret = system(cmd());
    assert(ret == 0);
}

void CountScale(const char * set) {
    Line input("bigdat/s/%s/orig/timeline", set);
    Line output("bigdat/s/%s/scale", set);

    printf("[countScale] %s -> %s\n", input(), output());
    countScale(input(), output());
}

void Merge(const char * set, const char * from1, uint64_t from1Scale,
        const char * from2, const char * to) {
    Line outdir("bigdat/s/%s/%s", set, to);
    mkdirs(outdir());

    Line trace1("bigdat/s/%s/%s/timeline", set, from1);
    Line trace2("bigdat/s/%s/%s/timeline", set, from2);
    Line scale("bigdat/s/%s/scale", set);
    Line userblock("bigdat/usercount/%s", set); // old data
    Line draft("bigdat/s/%s/%s/timeline.draft", set, to);
    Line output("bigdat/s/%s/%s/timeline", set, to);

    printf("[mergeTrace] %s + %s -> %s\n", trace1(), trace2(), draft());
    mergeTrace(scale(), userblock(), trace1(), 1, trace2(), draft());
    
    printf("[remarkAction2] %s -> %s\n", draft(), output());
    remarkAction2(draft(), output());
}

void MirrorClean(const char * set, const char * scale) {
    Line dir("bigdat/s/%s/%s", set, scale);
    Line cmd("rm %s/draft %s/timeline_unordered", dir(), dir());
    printf("> %s\n", cmd());
    auto ret = system(cmd());
    assert(ret == 0);
}

void MergeClean(const char * set, const char * scale) {
    Line dir("bigdat/s/%s/%s", set, scale);
    Line cmd("rm %s/timeline.draft", dir());
    printf("> %s\n", cmd());
    auto ret = system(cmd());
    assert(ret == 0);
}

void IncScale(const char * set, const char * scale1, 
        uint64_t scale, const char * scale2) {
    Mirror(set, "orig", "_m"); 
    MirrorClean(set, "_m");
    Merge(set, scale1, scale, "_m", scale2);
    MergeClean(set, scale2);

    Line cmd("rm -r bigdat/s/%s/_m", set);
    printf("> %s\n", cmd());
    auto ret = system(cmd());
    assert(ret == 0);
}

