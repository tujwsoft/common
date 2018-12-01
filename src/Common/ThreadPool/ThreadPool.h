#pragma once
#include <thread>
#include <atomic>
#include "TaskQueue.h"

typedef struct tagThreadPoolConfig
{
	int nMaxThread, nMinThread; //< ����߳�����
	double dMaxAddThreadRate, dMinSubThreadRate; //< ������߳������;��С���߳������
}ThreadPoolConfig;

/**
* @file   : ThreadPool.h
* @brief  : �̳߳��࣬��Ҫ������̹߳�������
* @author : tujw
* @date   : 2018/11/18
*/


class CThreadPool
{
public:
	explicit CThreadPool(ThreadPoolConfig config);
	~CThreadPool();

	operator bool()const;

	/*
	* @brief : �ͷ��߳����������
	*/

	void release(void);

	/*
	* @brief  : ��������
	* @param  : task �������
	* @param  : priorty �������ȼ���
	* @return : �ɹ�����true ʧ��false
	*/

	bool addTask(TaskPointer task, bool priorty = false);

	/*
	* @brief  : ɾ������
	* @param  : nTaskID ����ID
	* @return : �ɹ�true ʧ��false
	*/

	bool deleteTask(const size_t nTaskID);

	/*
	* @brief : ɾ����������
	*/

	void deleteAllTasks(void);

	/*
	* @brief  : �����Ƿ������
	* @param  : nTaskID����ID
	* @return : �������true δ����������false
	*/

	bool isTaskProcessed(const size_t nTaskID);

protected:
	void init(const ThreadPoolConfig& config); //< ��ʼ���̲߳���

	double getCurThreadTaskRate(void); //< ��ȡ��ǰ�߳������

	bool shouldStop(void); //< �鿴��ǰ�߳��Ƿ���Ҫ����

	bool addProcThreads(const int num = 1); //< ���ִ���߳�

	bool releaseThreadPool(void); //< �ͷ��̳߳�

	void callbackThreadFunc(void); //< �߳�ִ�к���

private:
	bool m_bValid;
	ThreadPoolConfig m_config; //< �̳߳�����������Ϣ
	std::atomic_int m_nCurThreadCount; //< ��ǰ�߳�����
	std::atomic_bool m_bIsThreadRun; //< �߳��Ƿ�����
	std::unique_ptr<CTaskQueue<IBaseTask>> m_taskQueuePointer;
};