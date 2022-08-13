
#pragma once

#include <stdint.h>
#include <boost/asio/io_service.hpp>
#include <unordered_set>

#include "../API/FUTU6.2/Include_FTAPI.h"
#include "../API/FUTU6.2/Include_3rd_protobuf.h"

#include "../Includes/ITraderApi.h"
#include "../Includes/WTSCollection.hpp"

#include "../Share/IniHelper.hpp"
#include "../Share/StdUtils.hpp"
#include "../Share/DLLHelper.hpp"

USING_NS_WTP;

class TraderFUTU : public ITraderApi, public Futu::FTSPI_Trd, public Futu::FTSPI_Conn
{
public:
	TraderFUTU();
	virtual ~TraderFUTU();

	typedef enum
	{
		TS_NOTLOGIN,		//δ��¼
		TS_LOGINING,		//���ڵ�¼
		TS_LOGINED,			//�ѵ�¼
		TS_LOGINFAILED,		//��¼ʧ��
		TS_ALLREADY			//ȫ������
	} TraderState;

protected:
    virtual void OnInitConnect(Futu::FTAPI_Conn* pConn, Futu::i64_t nErrCode, const char* strDesc);
	virtual void OnDisConnect(Futu::FTAPI_Conn* pConn, Futu::i64_t nErrCode){}

	/**
	* @brief ���Ľ����������ݵĽ����˻�
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_SubAccPush.protoЭ��
	*/
	virtual void OnReply_SubAccPush(Futu::u32_t nSerialNo, const Trd_SubAccPush::Response &stRsp);
	/**
	* @brief �µ�
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_PlaceOrder.protoЭ��
	*/
	virtual void OnReply_PlaceOrder(Futu::u32_t nSerialNo, const Trd_PlaceOrder::Response &stRsp);
	/**
	* @brief �޸Ķ���
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_ModifyOrder.protoЭ��
	*/
	virtual void OnReply_ModifyOrder(Futu::u32_t nSerialNo, const Trd_ModifyOrder::Response &stRsp);
	/**
	* @brief ���Ͷ����仯
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_UpdateOrder.protoЭ��
	*/
	virtual void OnPush_UpdateOrder(const Trd_UpdateOrder::Response &stRsp);
	/**
	* @brief �����ɽ�����
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_UpdateOrderFill.protoЭ��
	*/
	virtual void OnPush_UpdateOrderFill(const Trd_UpdateOrderFill::Response &stRsp);
		/**
	* @brief ��ȡ���ն����б�
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_GetOrderList.protoЭ��
	*/
	virtual void OnReply_GetOrderList(Futu::u32_t nSerialNo, const Trd_GetOrderList::Response &stRsp);
	/**
	* @brief ��ȡ���ճɽ��б�
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_GetOrderFillList.protoЭ��
	*/
	virtual void OnReply_GetOrderFillList(Futu::u32_t nSerialNo, const Trd_GetOrderFillList::Response &stRsp);
	/**
	* @brief ��ȡ�˻��ʽ�
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_GetFunds.protoЭ��
	*/
	virtual void OnReply_GetFunds(Futu::u32_t nSerialNo, const Trd_GetFunds::Response &stRsp);
	/**
	* @brief ��ȡ�˻��ֲ�
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_GetPositionList.protoЭ��
	*/
	virtual void OnReply_GetPositionList(Futu::u32_t nSerialNo, const Trd_GetPositionList::Response &stRsp);

protected:
	/**
	* @brief ��ȡ�����˻��б�
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_GetAccList.protoЭ��
	*/
	virtual void OnReply_GetAccList(Futu::u32_t nSerialNo, const Trd_GetAccList::Response &stRsp){}
	/**
	* @brief ����
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_UnlockTrade.protoЭ��
	*/
	virtual void OnReply_UnlockTrade(Futu::u32_t nSerialNo, const Trd_UnlockTrade::Response &stRsp){}
	/**
	* @brief ��ȡ���������
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_GetMaxTrdQtys.protoЭ��
	*/
	virtual void OnReply_GetMaxTrdQtys(Futu::u32_t nSerialNo, const Trd_GetMaxTrdQtys::Response &stRsp){}
	/**
	* @brief ��ȡ��ʷ�����б�
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_GetHistoryOrderList.protoЭ��
	*/
	virtual void OnReply_GetHistoryOrderList(Futu::u32_t nSerialNo, const Trd_GetHistoryOrderList::Response &stRsp){}
	/**
	* @brief ��ȡ��ʷ�ɽ��б�
	* @praram nSerialNo �����к�
	* @param stRsp �ذ��������ֶ���ο�Trd_GetHistoryOrderFillList.protoЭ��
	*/
	virtual void OnReply_GetHistoryOrderFillList(Futu::u32_t nSerialNo, const Trd_GetHistoryOrderFillList::Response &stRsp){}
	virtual void OnReply_GetMarginRatio(Futu::u32_t nSerialNo, const Trd_GetMarginRatio::Response &stRsp){}

public:
	//////////////////////////////////////////////////////////////////////////
	//ITraderApi �ӿ�
	virtual bool init(WTSVariant *params) override;

	virtual void release() override;

	virtual void registerSpi(ITraderSpi *listener) override;

	virtual void connect() override;

	virtual void disconnect() override;

	virtual bool isConnected() override;

	virtual bool makeEntrustID(char* buffer, int length) override;

	virtual int login(const char* user, const char* pass, const char* productInfo) override;

	virtual int logout() override;

	virtual int orderInsert(WTSEntrust* eutrust) override;
	// cancel order
	virtual int orderAction(WTSEntrustAction* action) override;

	virtual int queryAccount() override;

	virtual int queryPositions() override;

	virtual int queryOrders() override;

	virtual int queryTrades() override;

private:
	inline uint32_t	genRequestID();

	inline WTSOrderInfo*	makeOrderInfo(const Trd_Common::Order& orderField);
	inline WTSEntrust*		makeEntrust(const Trd_Common::Order& entrustField);
	inline WTSTradeInfo*	makeTradeInfo(const Trd_Common::OrderFill& tradeField);

	inline std::string		genEntrustID(uint32_t orderRef);

private:
	Futu::FTAPI_Trd* _api;

	typedef std::unordered_set<Futu::u32_t> SNSet;
    Futu::u32_t		_SubAccPushSerialNo;
	SNSet			_PlaceOrderSet;
	SNSet			_ModifyOrderSet;
	SNSet			_GetPositionListSet;
	SNSet			_GetFundsSet;
	SNSet			_GetOrderListSet;
	SNSet			_GetOrderFillListSet;
	ITraderSpi*		_sink;

	typedef WTSHashMap<std::string> PositionMap;
	PositionMap*			_positions;
	WTSArray*				_trades;
	WTSArray*				_orders;

	IBaseDataMgr*			_bd_mgr;

	int				_user;
	int				_env;
	int				_market;
	std::string		_host;
	int				_port;

	TraderState		_state;

	uint32_t		_tradingday;
	std::atomic<uint32_t>		_reqid;
	std::atomic<uint32_t>		_ordref;		//��������

	StdThreadPtr				_thrd_worker;

	IniHelper		_ini;
};

