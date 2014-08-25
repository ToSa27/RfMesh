#include "RfMeshThreading.h"

LoggerPtr Thread::logger(Logger::getLogger("ToSa.Mesh"));

Thread::Thread() : started(false) {
	_name = "unknown";
}

Thread::~Thread() {
	LOG4CXX_INFO(logger, "Thread terminated: " + _name);
}

unsigned int Thread::tid() const {
	return _id;
}

std::string Thread::tname() const {
	return _name;
}

void Thread::setName(std::string name) {
	_name = name;
}

void Thread::start(void *arg) {
	int ret;
	if (!started) {
		started = true;
		this->arg = arg;
		if ((ret = pthread_create(&_id, NULL, &Thread::exec, this)) != 0) {
//			LOG4CXX_INFO(logger, "Thread start error: " + strerror(ret));
			throw "Thread::start - Error";
		}
		LOG4CXX_INFO(logger, "Thread started: " + _name);
	}
}

void Thread::join() {
	pthread_join(_id, NULL);
}

void *Thread::exec(void *thr) {
	reinterpret_cast<Thread *> (thr)->run();
}

Lock::Lock() {
	pthread_mutex_init(&mutex, NULL);
}

Lock::~Lock() {
	pthread_mutex_destroy(&mutex);
}

void Lock::lock() {
	pthread_mutex_lock(&mutex);
}

void Lock::unlock() {
	pthread_mutex_unlock(&mutex);
}

Condition::Condition() {
	pthread_cond_init(&cond, NULL);
}

Condition::~Condition() {
	pthread_cond_destroy(&cond);
}

void Condition::wait() {
	pthread_cond_wait(&cond, &mutex);
}

void Condition::notify() {
	pthread_cond_signal(&cond);
}

LoggerPtr MessageQueue::logger(Logger::getLogger("ToSa.Mesh"));

MessageQueue::MessageQueue() {
	_cnd = new Condition();
}

MessageQueue::~MessageQueue() {
	delete _cnd;
}

bool MessageQueue::empty() {
	bool res;
	_cnd->lock();
	res = _queue.empty();
	_cnd->unlock();
	return res;
}

void MessageQueue::enqueue(const Thread& arg, MessagePtr mp) {
	_cnd->lock();
	_queue.push_back(mp);
	LOG4CXX_TRACE(logger, "MessageQueue enqueued by Thread " + arg.tname());
	_cnd->notify();
	_cnd->unlock();
}

MessagePtr MessageQueue::dequeue(const Thread& arg) {
	MessagePtr mp = NULL;
	_cnd->lock();
	while (! mp) {
		if (_queue.empty()) {
			LOG4CXX_TRACE(logger, "MessageQueue waiting Thread " + arg.tname());
			_cnd->wait();
		}
		mp = _queue.front();
		if (mp) {
			_queue.pop_front();
			LOG4CXX_TRACE(logger, "MessageQueue dequeue by  Thread " + arg.tname());
			break;
		}
	}
	_cnd->unlock();
	return mp;
}

RfMeshThreadMessageApp::RfMeshThreadMessageApp(appMessage msg) {
	_msgType = "appMessage";
	_msg = msg;
}

appMessage RfMeshThreadMessageApp::getMessage() {
	return _msg;
}

RfMeshThreadMessageMesh::RfMeshThreadMessageMesh(meshMessage msg) {
	_msgType = "meshMessage";
	_msg = msg;
}

meshMessage RfMeshThreadMessageMesh::getMessage() {
	return _msg;
}

RfMeshThreadMessageSubscribe::RfMeshThreadMessageSubscribe(MessageQueue *mq, uint8_t ptype, uint16_t node) {
	_msgType = "subscribe";
	_mq = mq;
	_ptype = ptype;
	_node = node;
}

MessageQueue * RfMeshThreadMessageSubscribe::getMessageQueue() {
	return _mq;
}

uint8_t RfMeshThreadMessageSubscribe::getPType() {
	return _ptype;
}

uint16_t RfMeshThreadMessageSubscribe::getNode() {
	return _node;
}

RfMeshThreadMessageJson::RfMeshThreadMessageJson(std::string json) {
	_json = json;
}

std::string RfMeshThreadMessageJson::getJson() {
	return _json;
}

void RfMeshThreadBase::run() {
	disp = reinterpret_cast<RfMeshMessageDispatcher *> (arg);
	if (!setup())
		return;
	for (;;) {
		if (!loop())
			break;
	}
}
