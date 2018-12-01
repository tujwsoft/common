#include "ThreadPool.h"
#include "IBaseTask.h"


CThreadPool::CThreadPool(ThreadPoolConfig config)
	: m_bValid(false)
	, m_bIsThreadRun(true)
	, m_nCurThreadCount(0)
	, m_taskQueuePointer(new CTaskQueue<IBaseTask>())
{
	init(config);
}


CThreadPool::~CThreadPool()
{

}


void CThreadPool::release(void)
{
	releaseThreadPool();
	m_taskQueuePointer->releaseQueue();

	int i = 0;
	while (m_nCurThreadCount > 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if (i++ == 10) { exit(23); }
	}

	m_nCurThreadCount = 0;
}


bool CThreadPool::addTask(TaskPointer task, bool priorty)
{
	const double& rate = getCurThreadTaskRate();

	if (priorty)
	{
		if (rate > 1000) { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }

		m_taskQueuePointer->pushFront(task);
	}
	else
	{
		if (rate > 100)
		{
			task->onCanceled();
			return false;
		}

		m_taskQueuePointer->pushBack(task);
		if (m_nCurThreadCount < m_config.nMaxThread &&
			rate < m_config.dMaxAddThreadRate)
		{
			return addProcThreads();
		}
	}

	return true;
}


bool CThreadPool::deleteTask(const size_t nTaskID)
{
	return m_taskQueuePointer->deleteTask(nTaskID);
}


void CThreadPool::deleteAllTasks(void)
{
	m_taskQueuePointer->deleteAllTasks();
}


bool CThreadPool::isTaskProcessed(const size_t nTaskID)
{
	return m_taskQueuePointer->isProcessed(nTaskID);
}


void CThreadPool::init(const ThreadPoolConfig& config)
{
	memset(&m_config, 0x00, sizeof(ThreadPoolConfig));

	if (config.dMaxAddThreadRate < config.dMinSubThreadRate)
	{
		return;
	}

	m_config.nMaxThread = config.nMaxThread;
	m_config.nMinThread = config.nMinThread;
	m_config.dMaxAddThreadRate = config.dMaxAddThreadRate;
	m_config.dMinSubThreadRate = config.dMinSubThreadRate;

	m_bValid = (m_config.nMinThread > 0) ? addProcThreads(m_config.nMinThread)
		: false;
}


double CThreadPool::getCurThreadTaskRate(void)
{
	if (m_nCurThreadCount == 0) { return 0.; }

	return (static_cast<double>(m_taskQueuePointer->getSize()) * 1.0) / static_cast<double>(m_nCurThreadCount);
}


bool CThreadPool::shouldStop(void)
{
	auto rate = getCurThreadTaskRate();

	if ((!m_bIsThreadRun || m_nCurThreadCount > m_config.nMinThread)
		&& rate < m_config.dMinSubThreadRate)
	{
		return true;
	}

	return false;
}


bool CThreadPool::addProcThreads(const int num)
{
	try
	{
		for (int i = num;i > 0; --i)
		{
			std::thread(&CThreadPool::callbackThreadFunc, this).detach();
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}


bool CThreadPool::releaseThreadPool(void)
{
	m_bIsThreadRun = false;
	m_config.nMinThread = 0;
	m_config.dMinSubThreadRate = 0.;
	return true;
}


void CThreadPool::callbackThreadFunc(void)
{
	m_nCurThreadCount.fetch_add(1);

	std::chrono::milliseconds millsec(500);
	std::shared_ptr<IBaseTask> ptrTask = nullptr;
	while (m_bIsThreadRun)
	{
		ptrTask = m_taskQueuePointer->popFront();

		if (nullptr == ptrTask)
		{
			if (shouldStop()) { break; }

			m_taskQueuePointer->waitTask(millsec);
			continue;
		}

		if (ptrTask->isCancelRequired())
		{
			ptrTask->onCanceled();
		}
		else
		{
			ptrTask->onCompleted(ptrTask->doWork());
		}

		m_taskQueuePointer->processedCallback(ptrTask->getId());

		if (shouldStop()) { break; }
	}

	m_nCurThreadCount.fetch_sub(1);
}
