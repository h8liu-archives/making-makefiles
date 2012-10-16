#include "simu.h"

const double Server::DEFAULT_WAN_BANDWIDTH = .1l;
const double Server::DEFAULT_LAN_BANDWIDTH = 1.l;
const double Server::DEFAULT_DISK_BANDWIDTH = 2.l;

const char * stepNames[] = {
    "RECVFROM_LAN",
    "SENDTO_LAN",
    "RECVFROM_WAN",
    "SENDTO_WAN",
    "WRITE_DISK",
    "READ_DISK",
    "ACCESS_DISK",
    "ACCESS_WAN",
    "WRITEBACK_DELAY",
    "N_MOD_STEP",
    "RESPOND",
    "END",
    "N_STEP",
};

Server::Server() :
    writeBackBuffer(BLOCK_MAX)
{
    // bind the queues
    lanReceiver.setQueue(&lanRecvQueue);
    lanSender.setQueue(&lanSendQueue);
    wanReceiver.setQueue(&wanRecvQueue);
    wanSender.setQueue(&wanSendQueue);
    diskProcesser.setQueue(&diskQueue);

    // bind in all the modules
    Module * allModules[] = {
        &wanSender, &wanReceiver,
        &lanSender, &lanReceiver,
        &diskProcesser,
        &diskDelay,
        &wanDelay,
        &writeBackDelay,
    };
    for (auto m : allModules) {
        modules.push_back(m);
        m->setPool(&jobPool);
    }
    
    // setup step mapping to modules
    execMap.resize(N_MOD_STEP, NULL);
    struct ModMap {
        uint8_t step;
        Module * mod;
    } mappings[] = {
        { RECVFROM_LAN, &lanReceiver },
        { SENDTO_LAN, &lanSender },
        { RECVFROM_WAN, &wanReceiver },
        { SENDTO_WAN, &wanSender },
        { WRITE_DISK, &diskProcesser },
        { READ_DISK, &diskProcesser },
        { ACCESS_DISK, &diskDelay },
        { ACCESS_WAN, &wanDelay },
        { WRITEBACK_DELAY, &writeBackDelay },
    };
    for (auto m : mappings) execMap[m.step] = m.mod;

    useDefaultSettings();

    currentPeriod = 0;
    periodCount = 0;
    periodInterval = DEFAULT_PERIOD;

    // stats
    nAbsorbedWrites = 0;
}

void Server::useDefaultSettings() {
    setWanBandwidth(DEFAULT_WAN_BANDWIDTH);
    setLanBandwidth(DEFAULT_LAN_BANDWIDTH);
    setDiskBandwidth(DEFAULT_DISK_BANDWIDTH);

    setDiskDelay(DEFAULT_DISK_DELAY);
    setWanDelay(DEFAULT_WAN_DELAY);
    setWriteBackDelay(DEFAULT_WRITEBACK_DELAY);
}

// steps
static uint8_t coldReadSteps[] = {
    ACCESS_WAN, // req
    RECVFROM_WAN,
    RECVFROM_LAN, // using the same nic
    SENDTO_LAN,
    RESPOND,
    WRITE_DISK,
    ACCESS_DISK, // ack
    END,
};

static uint8_t warmReadSteps[] = {
    ACCESS_DISK,
    READ_DISK,
    SENDTO_LAN,
    RESPOND,
    END,
};

static uint8_t writeSteps[] = {
    RECVFROM_LAN,
    WRITE_DISK,
    ACCESS_DISK,
    RESPOND,
    WRITEBACK_DELAY, // may end here
    ACCESS_DISK,
    READ_DISK,
    SENDTO_LAN, // using the same nic
    SENDTO_WAN,
    ACCESS_WAN, // ack
    END,
};

void Server::step(Job * job) {
    while (1) {
        uint8_t s = *(job->currentStep++);
        /*
        printf("t=%ld (u=%d b=%ld s=%ld) %s\n",
                job->t, job->access.user,
                job->access.block, job->access.session,
                stepNames[s]); */

        assert(s != N_MOD_STEP);
        assert(s != N_STEP);
        
        if (s == WRITEBACK_DELAY) {
            // printf("write back delay start: %ld\n", job->t);
            bool absorbed = writeBackBuffer.get(job->access.block);
            if (absorbed) {
                // printf("absorbed\n");
                nAbsorbedWrites++;
                s = END; // overwrite it to end
            } else {
                writeBackBuffer.set(job->access.block);
            }
        } 

        if (s == END) {
            uint64_t completeTime = job->t - job->access.time;
            if (isRead(job->access.action)) {
                statReadComplete->write(&completeTime);
            } else {
                statWriteComplete->write(&completeTime);
            }

            jobPool.free(job);
            return;
        } 
        
        if (s == RESPOND) {
            uint64_t responseTime = job->t - job->access.time;
            if (isRead(job->access.action)) {
                statReadResponse->write(&responseTime);
            } else {
                statWriteResponse->write(&responseTime);
            }
            continue;
        } 
        
        if (s < N_MOD_STEP) {
            Module * mod = execMap[s];

            auto q = mod->queue();
            if (q) {
                if (mod->hungry())
                    mod->push(job); // shortcut to module
                else
                    q->push(job); // wait on the queue first
            } else {
                // it does not have a queue, so just push in
                mod->push(job);
            }

            break;
        }
    }
}

void Server::simulateTo(uint64_t targetTime) {
    while (1) {
        Module * mod = NULL;
        uint64_t tmin = targetTime;
        for (auto m : modules) {
            uint64_t t = m->nextTime();
            if (t < tmin) {
                mod = m;
                tmin = t;
            }
        }
        
        if (!mod) return; // no active module up to targetTime

        period(tmin);

        Job * job = mod->pop();
        assert(job->t == tmin);
        
        // check if there are jobs queueing
        auto q = mod->queue();
        if (q) {
            assert(mod->hungry()); // just finished a job, must be hungry

            if (Job * nextJob = q->pull()) {
                if (job->currentStep[-1] == SENDTO_WAN 
                        && isWrite(job->access.action)) {
                    // a successful write back here, clear the buffer
                    writeBackBuffer.clear(job->access.block);
                }

                nextJob->t = tmin; // waiting is done, update time
                mod->push(nextJob); // keep the mod busy
                assert(!mod->hungry()); // so not hungry anymore
            }
        }

        if (job) step(job);
    }
}

void Server::periodFlush() {
    // activity 
    if (nAbsorbedWrites > 0) {
        fprintf(fAbsorbedWrites, "%ld %ld\n", 
                periodCount, nAbsorbedWrites);
        nAbsorbedWrites = 0;
    }

    // TODO: access latency CDF
    // TODO: user satisfaction
}

void Server::period(uint64_t curTime) {
    uint64_t p = curTime / periodInterval;
    if (periodCount == 0) { // bootstrap
        periodCount = 1;
        currentPeriod = p;
    }

    while (currentPeriod < p) {
        periodFlush();
        periodCount++;
        currentPeriod++;
    }
}

void Server::setStatDir(const char * dir) {
    // caller is responsible for making these dirs
    auto d = statDir.format("%s", dir);

    Line p;
    FILE * fout;

    /*
    static struct ModFile {
        Module * mod;
        const char * fname;
    } modFiles[] = {
        { &lanUplink, "lanup" },
        { &lanDownlink, "landown" },
        { &wanUplink, "wanup" },
        { &wanDownlink, "wandown" },
        { &diskLink, "disk" },
    };

    for (auto mf : modFiles) {
        p.format("%s/%s", d, mf.fname);
        fout = fopen(p(), "w");
        assert(fout);
        mf.mod->setReportFile(fout);
        statFiles.push_back(fout);
    }
    */

    static struct ServerFile {
        FILE ** f;
        const char * fname;
    } serverFiles[] = {
        { &fAbsorbedWrites, "absorbed_writes"},
    };

    for (auto sf : serverFiles) {
        p.format("%s/%s", d, sf.fname);
        fout = fopen(p(), "w");
        assert(fout);
        *sf.f = fout;
        statFiles.push_back(fout);
    }

    // latency writers 
    static struct ServerWriters {
        Writer ** w;
        const char * fname;
        size_t elementSize;
    } serverWriters[] = {
        { &statReadComplete, "read_complete", sizeof(uint64_t)},
        { &statWriteComplete, "write_complete", sizeof(uint64_t)},
        { &statReadResponse, "read_response", sizeof(uint64_t)},
        { &statWriteResponse, "write_response", sizeof(uint64_t)},
    };

    for (auto sw : serverWriters) {
        p.format("%s/%s", d, sw.fname);
        auto writer = new Writer(p(), sw.elementSize);
        *sw.w = writer;
        statWriters.push_back(writer);
    }
}

void Server::closeStatFiles() {
    for (auto f : statFiles) {
        fclose(f);
    }

    for (auto f : statWriters) {
        f->close();
        delete f;
    }
}

void Server::finishUp() {
    simulateTo(END_OF_TIME); 
    periodFlush();
    closeStatFiles();
}

bool Server::feed(TimelineReader & reader) {
    Job * job = jobPool.alloc();
    auto ret = reader.read(&job->access);

    if (!ret) {
        finishUp();
    } else {
        Access & a = job->access;
        if (a.time < END_OF_TIME) {
            simulateTo(a.time);
            
            job->t = a.time;
            if (isRead(a.action)) {
                if (isWarm(a.action)) { // cache hit
                    job->currentStep = warmReadSteps;
                } else {
                    job->currentStep = coldReadSteps;
                }
            } else {
                job->currentStep = writeSteps;
            }

            step(job);
        }
    }
    return ret;
}

void Server::run(const char * timeline) {
    TimelineFileReader reader(timeline);
    run(reader);
}

void Server::run(ScaleReader & r) {
    TimelineScaleReader reader(&r);
    run(reader);
}

void Server::run(TimelineReader & reader) {
    ProgLine prog;
    while (feed(reader)) 
        prog.count(reader.count(), reader.total());
}
