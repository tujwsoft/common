#pragma once
#include <time.h>
#include <atomic>

/**
* @file   : IBaseTask.h
* @brief  : ����Ļ����࣬������������Ľӿ�
* @author : tujw
* @date   : 2018/11/28
*/


class IBaseTask
{
public:
	virtual ~IBaseTask(){}

	/*
	* @brief  : ִ������
	* @return : ����0����ִ�гɹ�����֮����-1
	*/

	virtual int doWork() = 0;

	/*
	* @brief : ������ȡ���ص�����
	*/

	virtual void onCanceled() {}

	/*
	* @brief : ��������ɻص�����
	*/

	virtual void onCompleted(int){}

	/*
	* @brief  : ��ȡ����ִ�г�ʱʱ��
	* @return : ���س�ʱʱ�� 
	*/

	virtual unsigned int getTimeout(void) { return 500; }

	/*
	* @brief  : ����ִ���Ƿ�ʱ ����ִ�г���500����
	* @param  : tmCurTime ��ǰʱ��
��	* @return : ��ʱtrue δ��ʱfalse
	*/

	virtual bool isTimeOut(const clock_t& tmCurTime) 
	{ 
		return static_cast<unsigned int>(tmCurTime - m_tmTime) > getTimeout();
	}

	/*
	* @brief  : ��ȡ����id
	* @return : ���ص�ǰ����ID
	*/

	size_t getId()const { return m_taskId; }

	/*
	* @brief  : ��ǰ�����Ƿ�ȡ��ִ��
	* @return : ȡ������true ��֮false
	*/

	bool isCancelRequired()const { return m_isCancel; }

	/*
	* @brief : ȡ��ִ������
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
	clock_t m_tmTime; //< ����ʱ��
	size_t  m_taskId; //< ����ID
	std::atomic_bool m_isCancel;
	static std::atomic<size_t> nRequsetNum;
};


std::atomic<size_t> IBaseTask::nRequsetNum = 1000; //< ��1000��ʼ
