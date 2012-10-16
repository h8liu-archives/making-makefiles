#include "navy.h"

const uint64_t DEFAULT_SHIFTING = uint64_t(1e6) * 1800;

static uint64_t rollShift(Random * random) {
    if (random == NULL) return DEFAULT_SHIFTING;

    auto sec = random->next() % uint64_t(3600);
    auto microsec = random->next() % uint64_t(1e6);

    return sec * uint64_t(1e6) + microsec;
}

ScaleReader::ScaleReader(const char * timeline, const char * scaleInfo) :
    reader(timeline, sizeof(Access)) {

    Reader infoReader(scaleInfo, sizeof(TraceScale));
    assert(infoReader.total() == 1);
    bool ret = infoReader.read(&scale);
    assert(ret);
    inputTime = 0;
}

void ScaleReader::setScale(uint64_t nshifts, const char * shiftSeeds) {
    // shifts are ordered by sessions of first seen
    assert(shiftGenerators.empty());
    shiftGenerators.push_back(NULL); // the first one is constant shifter
    Reader reader(shiftSeeds, sizeof(uint64_t));
    uint64_t seed;

    for (uint64_t i = 0; i < nshifts; i++) {
        reader.read(&seed);
        shiftGenerators.push_back(new Random(seed));
    }
    reader.close();

    readerTotal = this->reader.total() * ncopy();
    readerCount = 0;
}

void ScaleReader::loadBlockType(const char * blockFile) {
    Reader reader(blockFile, sizeof(uint8_t));
    uint64_t blockId = 0;
    uint8_t nuser;
    uint64_t threshold = scale.nuser / 30;
    if (threshold < 3) threshold = 3;
    while (reader.read(&nuser)) {
        if (nuser >= threshold)
            sharedBlocks.set(blockId);
        blockId++;
    }

    assert(blockId == scale.nblock);
}

void ScaleReader::close() {
    for (auto r : shiftGenerators) {
        if (!r) continue;
        // they should already be auto-closed;
        delete r;
    }
}

void ScaleReader::readInput() {
    // printf("readInput()\n");
    Access * input = new Access; // always allocate a new one
    auto ret = reader.read(input);
    if (!ret) {
        delete input; // nothing is read

        // should always go to the other half from now on
        // reader.close(); // the reader is auto closed here
        close(); // release the shiftGenerators as well
        inputTime = UINT64_MAX; 

        // close all the existing queues
        for (auto queue : userSessions) 
            SessionQueue::close(queue.second);
        userSessions.clear(); // no need to keep them after this
        // the last iterator will deal with deleting

        return;
    }

    assert(input->time >= inputTime); // make sure it is a timeline
    inputTime = input->time; // update input time
    // printf("inputTime = %ld\n", inputTime);

    auto it = userSessions.find(input->user);
    SessionQueue * queue;
    if (it == userSessions.end() 
            || it->second->id() != input->session) {
        // new session
        if (it != userSessions.end())
            SessionQueue::close(it->second); // last queue is finished

        // note that we must add the input first
        // so that the iterators has stuff to iterate
        userSessions[input->user] = queue 
            = new SessionQueue(input->session);
        
        queue->push(input, iterHeap);

        // now we can add all the iterators
        addIterators(queue);
    } else { // easy for existing session
        it->second->push(input, iterHeap);
    }
}

void ScaleReader::scaleUpAccess(Access * a, uint64_t copyId) {
    // time will not be changed
    a->user += copyId * scale.nuser;
    if (isPrivateBlock(a->block)) {
        a->block += copyId * scale.nblock;
    }
    a->session += copyId * (scale.nsession - 1);

    remarkAction(a);
}

void ScaleReader::remarkAction(Access * a) {
    // this won't mark redundant flag 
    uint32_t newAction = 0;

    if (isWrite(a->action)) newAction |= F_WRITE;
    if (!cachedBlocks.get(a->block)) newAction |= F_COLD;
    
    a->action = newAction;
    cachedBlocks.set(a->block);
}

bool ScaleReader::read(Access * ret) {
    // when output is ahead last input, we need to get more input
    // when output is the same as last input, we can continue output
    // when inputTime is UINT64_MAX, there will be no input, it is 
    // impossible to go inside anymore, even when outputTime()
    // is also UINT64_MAX (nothing to output at this time)
    while (inputTime < outputTime())
        readInput();

    if (iterHeap.empty()) return false;

    auto it = iterHeap.pop();
    it->pop(ret, iterHeap);
    scaleUpAccess(ret, it->getCopyId());

    SessionIterator::deleteIfEnded(it);
    
    // ret->time -= DEFAULT_SHIFTING; // centralize the original
    
    readerCount++;

    return true;
}

void ScaleReader::addIterators(SessionQueue * queue) {
    uint64_t maxShifting = 0;
    SessionIterator * lastIterator = NULL;
        
    uint64_t ncopy = shiftGenerators.size();

    for (uint64_t copy = 0; copy < ncopy; copy++) {
        auto shift = rollShift(shiftGenerators[copy]);
        auto it = new SessionIterator(queue, copy, shift);

        if (shift > maxShifting) {
            lastIterator = it;
            maxShifting = shift;
        }
        
        // add the first round
        // we just added an input, so it should not be empty
        assert(!it->empty());
        iterHeap.push(it);
    }
    
    lastIterator->setLast(true); // setup the last iterator
}

SessionQueue::SessionQueue(uint64_t id) {
    sessionId = id;
    baseCount = 0;
    total = UINT64_MAX;
}

void SessionQueue::close(SessionQueue * q) {
    q->total = q->baseCount + q->accesses.size(); 

    SessionIterator * last = NULL;
    for (auto it : q->pendingIterators) {
        if (!it->isLast()) {
            auto ret = SessionIterator::deleteIfEnded(it);
            assert(ret); // must be ended since they was pending
        } else {
            last = it;
        }
    }
    
    if (last) {
        // will delete q as well
        auto ret = SessionIterator::deleteIfEnded(last);
        assert(ret);
    }

}

Access * SessionQueue::get(uint64_t i) const {
    assert(i >= baseCount);
    assert(i < total);
    uint64_t index = i - baseCount;
    if (index < accesses.size())
        return accesses[index];
    return NULL;
}

void SessionQueue::pop(Access * last) {
    assert(accesses.front() == last); // last is just for safty checking
    accesses.pop_front();
    baseCount++;
    delete last;
}

void SessionQueue::push(Access * a, 
        SessionIterHeap & iterHeap) {
    assert(!ended());
    accesses.push_back(a); 

    for (auto it : pendingIterators) {
        assert(!it->empty()); // was empty, but not any more
        iterHeap.push(it);
    }

    pendingIterators.clear();
}

void SessionQueue::addPendingIterator(SessionIterator * it) {
    assert(it->empty());
    pendingIterators.push_back(it);
}

SessionIterator::SessionIterator(SessionQueue * queue, uint64_t copy,
        uint64_t timeOffset) :
    queue(queue), timeOffset(timeOffset), 
    copyId(copy), index(0),
    last(false)
{ 
    assert(queue);
    if (empty()) reportPending();
}

bool SessionIterator::deleteIfEnded(SessionIterator * it) {
    auto ret = it->ended();
    if (ret) {
        // last is responsible to delete the queue
        if (it->last) {
            // printf("last iter freeing session %ld\n", it->queue->id());
            auto deleted = SessionQueue::deleteIfEnded(it->queue);
            assert(deleted);
        }

        /*
        printf("deleted it session=%ld copy=%ld\n",
                it->queue->id(), it->copyId);
        */
        // delete self
        delete it;
    }

    return ret;
}

void SessionIterator::pop(Access * a, SessionIterHeap & iterHeap) {
    assert(!empty());
    auto toPop = peak();
    *a = *toPop; // copy out
    a->time += timeOffset;

    index++;

    // only the last iterator uses pop()
    // pop() returns true means it is the caller to delete
    if (last) queue->pop(toPop);
    if (ended()) return;

    if (empty()) { reportPending(); return; }

    iterHeap.push(this);
}

uint64_t SessionIterator::nextTime() const {
    // when next time is called, must be non-empty
    Access * a = peak(); assert(a); 
    return a->time + timeOffset;
}

bool SessionIterator::operator < (const SessionIterator & other) const {
    if (&other == this) return false;

    // we invert the result, because the priority queue
    // always returns the greatest
    auto thisTime = this->nextTime();
    auto otherTime = other.nextTime();
    if (thisTime < otherTime) return false;
    if (thisTime > otherTime) return true;

    // when the time offset are the same
    // the one with larger copy id needs to pop first
    // this will guarantee that the `last` one (which 
    // will have smaller copy id) will pop the last
    return copyId < other.copyId;
}

void createScaleSeed(const char * fout, uint64_t maxScale) {
    Writer writer(fout, sizeof(uint64_t));
    uint64_t seed;
    // srand(time(NULL));

    for (uint64_t i = 0; i < maxScale; i++) {
        seed = rand();
        writer.write(&seed);
    }

    writer.close();
}
