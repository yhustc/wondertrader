#pragma once
#include <unordered_map>
#include "../Includes/ExecuteDefs.h"
#include "../Share/StdUtils.hpp"

USING_NS_OTP;

class WtTWapExeUnit : public ExecuteUnit
{
public:
	WtTWapExeUnit();
	virtual ~WtTWapExeUnit();

private:
	void	do_calc();
	void	fire_at_once(double qty);

public:
	/*
	*	����ִ������������
	*/
	virtual const char* getFactName() override;

	/*
	*	ִ�е�Ԫ����
	*/
	virtual const char* getName() override;

	/*
	*	��ʼ��ִ�е�Ԫ
	*	ctx		ִ�е�Ԫ���л���
	*	code	�����ĺ�Լ����
	*/
	virtual void init(ExecuteContext* ctx, const char* stdCode, WTSVariant* cfg) override;

	/*
	*	�����ر�
	*	localid	���ص���
	*	code	��Լ����
	*	isBuy	��or��
	*	leftover	ʣ������
	*	price	ί�м۸�
	*	isCanceled	�Ƿ��ѳ���
	*/
	virtual void on_order(uint32_t localid, const char* stdCode, bool isBuy, double leftover, double price, bool isCanceled) override;

	/*
	*	tick���ݻص�
	*	newTick	���µ�tick����
	*/
	virtual void on_tick(WTSTickData* newTick) override;

	/*
	*	�ɽ��ر�
	*	code	��Լ����
	*	isBuy	��or��
	*	vol		�ɽ�����������û��������ͨ��isBuyȷ�����뻹������
	*	price	�ɽ��۸�
	*/
	virtual void on_trade(const char* stdCode, bool isBuy, double vol, double price) override;

	/*
	*	�µ�����ر�
	*/
	virtual void on_entrust(uint32_t localid, const char* stdCode, bool bSuccess, const char* message) override;

	/*
	*	�����µ�Ŀ���λ
	*	code	��Լ����
	*	newVol	�µ�Ŀ���λ
	*/
	virtual void set_position(const char* stdCode, double newVol) override;

	/*
	*	����ͨ�������ص�
	*/
	virtual void on_channel_ready() override;

	/*
	*	����ͨ����ʧ�ص�
	*/
	virtual void on_channel_lost() override;


private:
	WTSTickData* _last_tick;	//��һ������
	double		_target_pos;	//Ŀ���λ
	bool		_channel_ready;


	WTSCommodityInfo*	_comm_info;

	//////////////////////////////////////////////////////////////////////////
	//ִ�в���
	typedef std::unordered_map<uint32_t, uint64_t> Orders;
	Orders			_orders;
	StdRecurMutex	_mtx_ords;
	uint32_t		_cancel_cnt;

	//////////////////////////////////////////////////////////////////////////
	//����
	uint32_t		_total_secs;	//ִ����ʱ�䣬��λs
	uint32_t		_total_times;	//��ִ�д���
	uint32_t		_tail_secs;		//ִ��β��ʱ��
	uint32_t		_ord_sticky;	//�ҵ�ʱ�ޣ���λs
	uint32_t		_price_mode;	//�۸�ģʽ��0-���¼ۣ�1-���żۣ�2-���ּ�
	uint32_t		_price_offset;		//�ҵ��۸�ƫ�ƣ�����ڼ����۸�ƫ�ƣ���+��-

	//////////////////////////////////////////////////////////////////////////
	//��ʱ����
	double			_this_target;	//����Ŀ���λ
	uint32_t		_fire_span;		//�������
	uint32_t		_fired_times;	//��ִ�д���
	uint64_t		_last_fire_time;
};
