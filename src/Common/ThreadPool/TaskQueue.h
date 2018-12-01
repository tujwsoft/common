#pragma once
#include <map>
#include <mutex>
#include <memory>
#include <queue>

/**
* @brief  : 任务队列，管理任务
* @author : tujw
* @date   : 2018/11/18
*/

class IBaseTask;
typedef std::shared_ptr<IBaseTask> TaskPointer;

template<typename T>
class CTaskQueue
{
public:
	CTaskQueue(void);
	~CTaskQueue(void);

	
	/*
	* @brief : 入队(尾部)
	* @param : taskPointer 任务对象
	*/

	void pushBack(TaskPointer taskPointer);

	/*
	* @brief : 入队(头部)
	* @param : taskPointer 任务对象
	*/

	void pushFront(TaskPointer taskPointer);

	/*
	* @breif  : 出队(头部)
	* @return : 存在任务则返回任务对象，否则为nullptr
	*/

	TaskPointer popFront(void);

	/*
	* @brief  : 判断队列是否为空
	* @return : 为空返回true 否则false
	*/

	bool isEmpty()const;

	/*
	* @brief : 获取任务数量
	* @reurn : 返回任务数量
	*/

	size_t getSize(void);

	/*
	* @brief : 释放队列
	*/

	void releaseQueue(void);

	/*
	* @brief  : 删除任务
	* @param  : nTaskID 任务ID
	* @return : 成功返回true 否则false
	*/

	bool deleteTask(const size_t nTaskID);

	/*
	* @brief : 删除所有任务
	*/

	void deleteAllTasks(void);

	/*
	* @brief : 任务执行完成回调
	* @param : nTaskID 任务ID
	*/

	void processedCallback(const size_t nTaskID);

	/*
	* @brief  : 任务是否执行完成
	* @return : 执行完成true 否则false
	*/

	bool isProcessed(const size_t nTaskID);

	/*
	* @breif : 等待任务
	* @param : sec 等待时间
	*/

	void waitTask(const std::chrono::milliseconds& sec);

private:
	std::mutex m_mutexPush; //< 入队互斥对象
	std::mutex m_mutexWait; //< 等待互斥对象
	std::mutex m_mutexRun;  //< 出队互斥对象

	std::condition_variable m_condition;
	std::deque<std::shared_ptr<IBaseTask>> m_taskQueue; //< 排队执行任务队列
	std::map<size_t, std::shared_ptr<IBaseTask>> m_mapRunTask; //< 运行中任务
};





template<typename T>
CTaskQueue<T>::CTaskQueue()
{
	m_taskQueue.clear();
}


template<typename T>
CTaskQueue<T>::~CTaskQueue()
{

}


template<typename T>
void CTaskQueue<T>::pushBack(TaskPointer taskPointer)
{
	if (nullptr == taskPointer) { return; }

	std::lock_guard<std::mutex> lock(m_mutexPush);
	m_taskQueue.push_back(taskPointer);
	m_condition.notify_one();
}


template<typename T>
void CTaskQueue<T>::pushFront(TaskPointer taskPointer)
{
	if (nullptr == taskPointer) { return; }

	std::lock_guard<std::mutex> lock(m_mutexPush);
	m_taskQueue.push_front(taskPointer);
	m_condition.notify_one();
}


template<typename T>
TaskPointer CTaskQueue<T>::popFront(void)
{
	std::lock_guard<std::mutex> lock(m_mutexPush);
	if (m_taskQueue.empty()) { return nullptr; }

	std::lock_guard<std::mutex> runLock(m_mutexRun);

	TaskPointer taskPointer = m_taskQueue.front();
	m_taskQueue.pop_front();
	if (nullptr == taskPointer) { return nullptr; }

	m_mapRunTask.insert(std::make_pair(taskPointer->getId(),
		taskPointer));

	return taskPointer;
}


template<typename T>
bool CTaskQueue<T>::isEmpty(void)const
{
	std::lock_guard<std::mutex> lock(m_mutexPush);
	return m_taskQueue.empty();
}


template<typename T>
size_t CTaskQueue<T>::getSize(void)
{
	std::lock_guard<std::mutex> lock(m_mutexPush);
	return m_taskQueue.size();
}


template<typename T>
void CTaskQueue<T>::releaseQueue(void)
{
	deleteAllTasks();
	m_condition.notify_all();
}


template<typename T>
bool CTaskQueue<T>::deleteTask(const size_t nTaskID)
{
	if (-1 == nTaskID) { return false; }

	std::unique_lock<std::mutex> lock(m_mutexPush);
	auto func = [&nTaskID](decltype(*m_taskQueue.begin())& item) {return item->getId() == nTaskID; };
	auto iter = std::remove_if(m_taskQueue.begin(), m_taskQueue.end(), func);
	m_taskQueue.erase(iter, m_taskQueue.end());
	lock.unlock();

	std::lock_guard<std::mutex> mutexRun(m_mutexRun);
	auto index = m_mapRunTask.find(nTaskID);
	if (index != m_mapRunTask.end()) {
		index->second->cancelRequired();
	}

	while (m_mapRunTask.count(nTaskID))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	return true;
}


template<typename T>
void CTaskQueue<T>::deleteAllTasks()
{
	std::unique_lock<std::mutex> lock(m_mutexPush);
	if (!m_taskQueue.empty()) { m_taskQueue.clear(); }
	
	std::unique_lock<std::mutex> lockRun(m_mutexRun);
	for (auto& item : m_mapRunTask)
	{
		item.second->cancelRequired();
	}
}


template<typename T>
void CTaskQueue<T>::processedCallback(const size_t nTaskID)
{
	std::lock_guard<std::mutex> lock(m_mutexRun);
	m_mapRunTask.erase(nTaskID);
}


template<typename T>
bool CTaskQueue<T>::isProcessed(const size_t nTaskID)
{
	{
		std::lock_guard<std::mutex> lock(m_mutexPush);
		auto iter = std::find_if(m_taskQueue.begin(), m_taskQueue.end(),
			[&nTaskID](decltype(*m_taskQueue.begin())& item) {return item->getId() == nTaskID; });
		if (iter != m_taskQueue.end()) { return false; }
	}

	{
		std::lock_guard<std::mutex> lock(m_mutexRun);
		if (m_mapRunTask.count(nTaskID)) { return false; }
	}
	
	return true;
}


template<typename T>
void CTaskQueue<T>::waitTask(const std::chrono::milliseconds& sec)
{
	std::unique_lock<std::mutex> lock(m_mutexWait);
	m_condition.wait_for(lock, sec);
}
