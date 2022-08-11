
#pragma once

#include <stdint.h>
#include <boost/asio/io_service.hpp>

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
		TS_NOTLOGIN,		//未登录
		TS_LOGINING,		//正在登录
		TS_LOGINED,			//已登录
		TS_LOGINFAILED,		//登录失败
		TS_ALLREADY			//全部就绪
	} TraderState;

protected:
    virtual void OnInitConnect(Futu::FTAPI_Conn* pConn, Futu::i64_t nErrCode, const char* strDesc);
	virtual void OnDisConnect(Futu::FTAPI_Conn* pConn, Futu::i64_t nErrCode){}

	/**
	* @brief 订阅接收推送数据的交易账户
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_SubAccPush.proto协议
	*/
	virtual void OnReply_SubAccPush(Futu::u32_t nSerialNo, const Trd_SubAccPush::Response &stRsp);
	/**
	* @brief 推送订单变化
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_UpdateOrder.proto协议
	*/
	virtual void OnPush_UpdateOrder(const Trd_UpdateOrder::Response &stRsp);
	/**
	* @brief 订单成交推送
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_UpdateOrderFill.proto协议
	*/
	virtual void OnPush_UpdateOrderFill(const Trd_UpdateOrderFill::Response &stRsp);
	/**
	* @brief 获取历史订单列表
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_GetHistoryOrderList.proto协议
	*/
	virtual void OnReply_GetHistoryOrderList(Futu::u32_t nSerialNo, const Trd_GetHistoryOrderList::Response &stRsp);
	/**
	* @brief 获取历史成交列表
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_GetHistoryOrderFillList.proto协议
	*/
	virtual void OnReply_GetHistoryOrderFillList(Futu::u32_t nSerialNo, const Trd_GetHistoryOrderFillList::Response &stRsp);
	/**
	* @brief 获取账户资金
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_GetFunds.proto协议
	*/
	virtual void OnReply_GetFunds(Futu::u32_t nSerialNo, const Trd_GetFunds::Response &stRsp);
	/**
	* @brief 获取账户持仓
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_GetPositionList.proto协议
	*/
	virtual void OnReply_GetPositionList(Futu::u32_t nSerialNo, const Trd_GetPositionList::Response &stRsp);

protected:
	/**
	* @brief 获取交易账户列表
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_GetAccList.proto协议
	*/
	virtual void OnReply_GetAccList(Futu::u32_t nSerialNo, const Trd_GetAccList::Response &stRsp){}
	/**
	* @brief 解锁
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_UnlockTrade.proto协议
	*/
	virtual void OnReply_UnlockTrade(Futu::u32_t nSerialNo, const Trd_UnlockTrade::Response &stRsp){}
	/**
	* @brief 订阅接收推送数据的交易账户
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_SubAccPush.proto协议
	*/
	virtual void OnReply_SubAccPush(Futu::u32_t nSerialNo, const Trd_SubAccPush::Response &stRsp){}
	/**
	* @brief 获取最大交易数量
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_GetMaxTrdQtys.proto协议
	*/
	virtual void OnReply_GetMaxTrdQtys(Futu::u32_t nSerialNo, const Trd_GetMaxTrdQtys::Response &stRsp){}
	/**
	* @brief 获取当日订单列表
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_GetOrderList.proto协议
	*/
	virtual void OnReply_GetOrderList(Futu::u32_t nSerialNo, const Trd_GetOrderList::Response &stRsp){}
	/**
	* @brief 下单
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_PlaceOrder.proto协议
	*/
	virtual void OnReply_PlaceOrder(Futu::u32_t nSerialNo, const Trd_PlaceOrder::Response &stRsp){}
	/**
	* @brief 修改订单
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_ModifyOrder.proto协议
	*/
	virtual void OnReply_ModifyOrder(Futu::u32_t nSerialNo, const Trd_ModifyOrder::Response &stRsp){}
	/**
	* @brief 获取当日成交列表
	* @praram nSerialNo 包序列号
	* @param stRsp 回包，具体字段请参考Trd_GetOrderFillList.proto协议
	*/
	virtual void OnReply_GetOrderFillList(Futu::u32_t nSerialNo, const Trd_GetOrderFillList::Response &stRsp){}

public:
	//////////////////////////////////////////////////////////////////////////
	//ITraderApi 接口
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

	virtual int orderAction(WTSEntrustAction* action) override;

	virtual int queryAccount() override;

	virtual int queryPositions() override;

	virtual int queryOrders() override;

	virtual int queryTrades() override;

private:
	void		reconnect();
	inline uint32_t	genRequestID();

	inline WTSOrderInfo*	makeOrderInfo(const Trd_Common::Order& orderField);
	inline WTSEntrust*		makeEntrust(const Trd_Common::Order& entrustField);
	inline WTSTradeInfo*	makeTradeInfo(const Trd_Common::OrderFill& tradeField);

	inline std::string		genEntrustID(uint32_t orderRef);

private:
	Futu::FTAPI_Trd* _api;
    Futu::u32_t 	_SubAccPushSerialNo;
	ITraderSpi*		_sink;

	typedef WTSHashMap<std::string> PositionMap;
	PositionMap*			_positions;
	WTSArray*				_trades;
	WTSArray*				_orders;

	IBaseDataMgr*			_bd_mgr;

	std::string		_user;
	std::string		_host;
	int				_port;

	TraderState		_state;

	uint64_t		_sessionid;
	uint32_t		_tradingday;
	std::atomic<uint32_t>		_reqid;
	std::atomic<uint32_t>		_ordref;		//报单引用

	boost::asio::io_service		_asyncio;
	StdThreadPtr				_thrd_worker;

	IniHelper		_ini;
	uint32_t		_uniq_id;
};

