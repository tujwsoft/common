#pragma once
#include <thread>
#include <atomic>
#include "TaskQueue.h"

typedef struct tagThreadPoolConfig
{
	int nMaxThread, nMinThread; //< 最大线程数量
	double dMaxAddThreadRate, dMinSubThreadRate; //< 最大增线程任务比;最小减线程任务比
}ThreadPoolConfig;

/**
* @file   : ThreadPool.h
* @brief  : 线程池类，主要解决多线程管理问题
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
	* @brief : 释放线程与任务队列
	*/

	void release(void);

	/*
	* @brief  : 新增任务
	* @param  : task 任务对象
	* @param  : priorty 处理优先级别
	* @return : 成功返回true 失败false
	*/

	bool addTask(TaskPointer task, bool priorty = false);

	/*
	* @brief  : 删除任务
	* @param  : nTaskID 任务ID
	* @return : 成功true 失败false
	*/

	bool deleteTask(const size_t nTaskID);

	/*
	* @brief : 删除所有任务
	*/

	void deleteAllTasks(void);

	/*
	* @brief  : 任务是否处理完成
	* @param  : nTaskID任务ID
	* @return : 处理完成true 未处理及处理中false
	*/

	bool isTaskProcessed(const size_t nTaskID);

protected:
	void init(const ThreadPoolConfig& config); //< 初始化线程参数

	double getCurThreadTaskRate(void); //< 获取当前线程任务比

	bool shouldStop(void); //< 查看当前线程是否需要结束

	bool addProcThreads(const int num = 1); //< 添加执行线程

	bool releaseThreadPool(void); //< 释放线程池

	void callbackThreadFunc(void); //< 线程执行函数

private:
	bool m_bValid;
	ThreadPoolConfig m_config; //< 线程池配置属性信息
	std::atomic_int m_nCurThreadCount; //< 当前线程数量
	std::atomic_bool m_bIsThreadRun; //< 线程是否运行
	std::unique_ptr<CTaskQueue<IBaseTask>> m_taskQueuePointer;
};