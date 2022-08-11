
#pragma once
#include "../Includes/IParserApi.h"
#include "../Share/DLLHelper.hpp"
#include "../API/FUTU6.2/Include_FTAPI.h"
#include "../API/FUTU6.2/Include_3rd_protobuf.h"
#include <map>

NS_WTP_BEGIN
class WTSTickData;
NS_WTP_END

USING_NS_WTP;

class ParserFUTU :	public IParserApi, public Futu::FTSPI_Qot, public Futu::FTSPI_Conn
{
public:
	ParserFUTU();
	virtual ~ParserFUTU();

public:
	enum LoginStatus
	{
		LS_NOTLOGIN,
		LS_LOGINING,
		LS_LOGINED
	};

//IQuoteParser ½Ó¿Ú
public:
	virtual bool init(WTSVariant* config) override;

	virtual void release() override;

	virtual bool connect() override;

	virtual bool disconnect() override;

	virtual bool isConnected() override;

	virtual void subscribe(const CodeSet &vecSymbols) override;
	virtual void unsubscribe(const CodeSet &vecSymbols) override;

	virtual void registerSpi(IParserSpi* listener) override;


protected:
    virtual void OnInitConnect(Futu::FTAPI_Conn* pConn, Futu::i64_t nErrCode, const char* strDesc);
	virtual void OnDisConnect(Futu::FTAPI_Conn* pConn, Futu::i64_t nErrCode){}
    virtual void OnReply_Sub(Futu::u32_t nSerialNo, const Qot_Sub::Response& stRsp);
    virtual void OnPush_UpdateTicker(const Qot_UpdateTicker::Response& stRsp);

protected:
	virtual void OnReply_GetGlobalState(Futu::u32_t nSerialNo, const GetGlobalState::Response &stRsp){}
	virtual void OnReply_RegQotPush(Futu::u32_t nSerialNo, const Qot_RegQotPush::Response &stRsp){}
	virtual void OnReply_GetSubInfo(Futu::u32_t nSerialNo, const Qot_GetSubInfo::Response &stRsp){}
	virtual void OnReply_GetTicker(Futu::u32_t nSerialNo, const Qot_GetTicker::Response &stRsp){}
	virtual void OnReply_GetBasicQot(Futu::u32_t nSerialNo, const Qot_GetBasicQot::Response &stRsp){}
	virtual void OnReply_GetOrderBook(Futu::u32_t nSerialNo, const Qot_GetOrderBook::Response &stRsp){}
	virtual void OnReply_GetKL(Futu::u32_t nSerialNo, const Qot_GetKL::Response &stRsp){}
	virtual void OnReply_GetRT(Futu::u32_t nSerialNo, const Qot_GetRT::Response &stRsp){}
	virtual void OnReply_GetBroker(Futu::u32_t nSerialNo, const Qot_GetBroker::Response &stRsp){}
	virtual void OnReply_RequestRehab(Futu::u32_t nSerialNo, const Qot_RequestRehab::Response &stRsp){}
	virtual void OnReply_RequestHistoryKL(Futu::u32_t nSerialNo, const Qot_RequestHistoryKL::Response &stRsp){}
	virtual void OnReply_RequestHistoryKLQuota(Futu::u32_t nSerialNo, const Qot_RequestHistoryKLQuota::Response &stRsp){}
	virtual void OnReply_GetTradeDate(Futu::u32_t nSerialNo, const Qot_GetTradeDate::Response &stRsp){}
	virtual void OnReply_GetStaticInfo(Futu::u32_t nSerialNo, const Qot_GetStaticInfo::Response &stRsp){}
	virtual void OnReply_GetSecuritySnapshot(Futu::u32_t nSerialNo, const Qot_GetSecuritySnapshot::Response &stRsp){}
	virtual void OnReply_GetPlateSet(Futu::u32_t nSerialNo, const Qot_GetPlateSet::Response &stRsp){}
	virtual void OnReply_GetPlateSecurity(Futu::u32_t nSerialNo, const Qot_GetPlateSecurity::Response &stRsp){}
	virtual void OnReply_GetReference(Futu::u32_t nSerialNo, const Qot_GetReference::Response &stRsp){}
	virtual void OnReply_GetOwnerPlate(Futu::u32_t nSerialNo, const Qot_GetOwnerPlate::Response &stRsp){}
	virtual void OnReply_GetHoldingChangeList(Futu::u32_t nSerialNo, const Qot_GetHoldingChangeList::Response &stRsp){}
	virtual void OnReply_GetOptionChain(Futu::u32_t nSerialNo, const Qot_GetOptionChain::Response &stRsp){}
	virtual void OnReply_GetWarrant(Futu::u32_t nSerialNo, const Qot_GetWarrant::Response &stRsp){}
	virtual void OnReply_GetCapitalFlow(Futu::u32_t nSerialNo, const Qot_GetCapitalFlow::Response &stRsp){}
	virtual void OnReply_GetCapitalDistribution(Futu::u32_t nSerialNo, const Qot_GetCapitalDistribution::Response &stRsp){}
	virtual void OnReply_GetUserSecurity(Futu::u32_t nSerialNo, const Qot_GetUserSecurity::Response &stRsp){}
	virtual void OnReply_ModifyUserSecurity(Futu::u32_t nSerialNo, const Qot_ModifyUserSecurity::Response &stRsp){}
	virtual void OnReply_StockFilter(Futu::u32_t nSerialNo, const Qot_StockFilter::Response &stRsp){}
	virtual void OnReply_GetCodeChange(Futu::u32_t nSerialNo, const Qot_GetCodeChange::Response &stRsp){}
	virtual void OnReply_GetIpoList(Futu::u32_t nSerialNo, const Qot_GetIpoList::Response &stRsp){}
	virtual void OnReply_GetFutureInfo(Futu::u32_t nSerialNo, const Qot_GetFutureInfo::Response &stRsp){}
	virtual void OnReply_RequestTradeDate(Futu::u32_t nSerialNo, const Qot_RequestTradeDate::Response &stRsp){}
	virtual void OnReply_SetPriceReminder(Futu::u32_t nSerialNo, const Qot_SetPriceReminder::Response &stRsp){}
	virtual void OnReply_GetPriceReminder(Futu::u32_t nSerialNo, const Qot_GetPriceReminder::Response &stRsp){}
	virtual void OnReply_GetUserSecurityGroup(Futu::u32_t nSerialNo, const Qot_GetUserSecurityGroup::Response &stRsp){}
	virtual void OnReply_GetMarketState(Futu::u32_t nSerialNo, const Qot_GetMarketState::Response &stRsp){}
	virtual void OnReply_GetOptionExpirationDate(Futu::u32_t nSerialNo, const Qot_GetOptionExpirationDate::Response &stRsp){}

	virtual void OnPush_Notify(const Notify::Response &stRsp){}
	virtual void OnPush_UpdateBasicQot(const Qot_UpdateBasicQot::Response &stRsp){}
	virtual void OnPush_UpdateOrderBook(const Qot_UpdateOrderBook::Response &stRsp){}
	virtual void OnPush_UpdateKL(const Qot_UpdateKL::Response &stRsp){}
	virtual void OnPush_UpdateRT(const Qot_UpdateRT::Response &stRsp){}
	virtual void OnPush_UpdateBroker(const Qot_UpdateBroker::Response &stRsp){}
	virtual void OnPush_UpdatePriceReminder(const Qot_UpdatePriceReminder::Response &stRsp){}


private:
	uint32_t			m_uTradingDate;
	LoginStatus			m_loginState;
	Futu::FTAPI_Qot*	m_pQotApi;
    Futu::u32_t			m_SubSerialNo;

	std::string			m_strHost;
	int32_t				m_iPort;

	CodeSet				m_fitSHSubs;
	CodeSet				m_fitSZSubs;

	IParserSpi*			m_sink;
	IBaseDataMgr*		m_pBaseDataMgr;
};

