#pragma once
#include <map>
#include <mutex>
#include <memory>
#include <queue>

/**
* @brief  : ������У���������
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
	* @brief : ���(β��)
	* @param : taskPointer �������
	*/

	void pushBack(TaskPointer taskPointer);

	/*
	* @brief : ���(ͷ��)
	* @param : taskPointer �������
	*/

	void pushFront(TaskPointer taskPointer);

	/*
	* @breif  : ����(ͷ��)
	* @return : ���������򷵻�������󣬷���Ϊnullptr
	*/

	TaskPointer popFront(void);

	/*
	* @brief  : �ж϶����Ƿ�Ϊ��
	* @return : Ϊ�շ���true ����false
	*/

	bool isEmpty()const;

	/*
	* @brief : ��ȡ��������
	* @reurn : ������������
	*/

	size_t getSize(void);

	/*
	* @brief : �ͷŶ���
	*/

	void releaseQueue(void);

	/*
	* @brief  : ɾ������
	* @param  : nTaskID ����ID
	* @return : �ɹ�����true ����false
	*/

	bool deleteTask(const size_t nTaskID);

	/*
	* @brief : ɾ����������
	*/

	void deleteAllTasks(void);

	/*
	* @brief : ����ִ����ɻص�
	* @param : nTaskID ����ID
	*/

	void processedCallback(const size_t nTaskID);

	/*
	* @brief  : �����Ƿ�ִ�����
	* @return : ִ�����true ����false
	*/

	bool isProcessed(const size_t nTaskID);

	/*
	* @breif : �ȴ�����
	* @param : sec �ȴ�ʱ��
	*/

	void waitTask(const std::chrono::milliseconds& sec);

private:
	std::mutex m_mutexPush; //< ��ӻ������
	std::mutex m_mutexWait; //< �ȴ��������
	std::mutex m_mutexRun;  //< ���ӻ������

	std::condition_variable m_condition;
	std::deque<std::shared_ptr<IBaseTask>> m_taskQueue; //< �Ŷ�ִ���������
	std::map<size_t, std::shared_ptr<IBaseTask>> m_mapRunTask; //< ����������
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
