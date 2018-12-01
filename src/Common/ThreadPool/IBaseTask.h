#pragma once
#include <time.h>
#include <atomic>

/**
* @file   : IBaseTask.h
* @brief  : 任务的基础类，定义了最基础的接口
* @author : tujw
* @date   : 2018/11/28
*/


class IBaseTask
{
public:
	virtual ~IBaseTask(){}

	/*
	* @brief  : 执行任务
	* @return : 返回0任务执行成功，反之返回-1
	*/

	virtual int doWork() = 0;

	/*
	* @brief : 任务已取消回调函数
	*/

	virtual void onCanceled() {}

	/*
	* @brief : 任务已完成回调函数
	*/

	virtual void onCompleted(int){}

	/*
	* @brief  : 获取任务执行超时时间
	* @return : 返回超时时间 
	*/

	virtual unsigned int getTimeout(void) { return 500; }

	/*
	* @brief  : 任务执行是否超时 任务执行超出500毫秒
	* @param  : tmCurTime 当前时间
、	* @return : 超时true 未超时false
	*/

	virtual bool isTimeOut(const clock_t& tmCurTime) 
	{ 
		return static_cast<unsigned int>(tmCurTime - m_tmTime) > getTimeout();
	}

	/*
	* @brief  : 获取任务id
	* @return : 返回当前任务ID
	*/

	size_t getId()const { return m_taskId; }

	/*
	* @brief  : 当前任务是否取消执行
	* @return : 取消返回true 反之false
	*/

	bool isCancelRequired()const { return m_isCancel; }

	/*
	* @brief : 取消执行任务
	*/

	void cancelRequired(void) { m_isCancel = true; }

protected:
	IBaseTask()
		: m_tmTime(clock_t())
		, m_isCancel(false)
		, m_taskId(nRequsetNum.fetch_add(1))
	{

	}

private:
	clock_t m_tmTime; //< 创建时间
	size_t  m_taskId; //< 任务ID
	std::atomic_bool m_isCancel;
	static std::atomic<size_t> nRequsetNum;
};


std::atomic<size_t> IBaseTask::nRequsetNum = 1000; //< 从1000开始
