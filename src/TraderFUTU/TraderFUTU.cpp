
#include "TraderFUTU.h"

#include "../Includes/IBaseDataMgr.h"
#include "../Includes/WTSContractInfo.hpp"
#include "../Includes/WTSSessionInfo.hpp"
#include "../Includes/WTSTradeDef.hpp"
#include "../Includes/WTSError.hpp"
#include "../Includes/WTSVariant.hpp"

#include "../Share/ModuleHelper.hpp"

#include <boost/filesystem.hpp>
#include <random>

using namespace std;
using namespace Futu;

 //By Wesley @ 2022.01.05
#include "../Share/fmtlib.h"
template<typename... Args>
inline void write_log(ITraderSpi* sink, WTSLogLevel ll, const char* format, const Args&... args)
{
	if (sink == NULL)
		return;

	const char* buffer = fmtutil::format(format, args...);

	sink->handleTraderLog(ll, buffer);
}

extern "C"
{
	EXPORT_FLAG ITraderApi* createTrader()
	{
		TraderFUTU *instance = new TraderFUTU();
		return instance;
	}

	EXPORT_FLAG void deleteTrader(ITraderApi* &trader)
	{
		if (NULL != trader)
		{
			delete trader;
			trader = NULL;
		}
	}
}

const char* ENTRUST_SECTION = "entrusts";
const char* ORDER_SECTION = "orders";


inline WTSDirectionType wrapPosDirection(google::protobuf::int32 position_side)
{
	switch (position_side)
	{
	case Trd_Common::PositionSide_Short: return WDT_SHORT;
	default:
		return WDT_LONG;
	}
}

inline google::protobuf::int32 wrapDirectionType(WTSDirectionType dirType, WTSOffsetType offsetType)
{
	if (WDT_LONG == dirType)
		if (offsetType == WOT_OPEN)
			return Trd_Common::TrdSide_Buy;
		else
			return Trd_Common::TrdSide_Sell;
	else
		if (offsetType == WOT_OPEN)
			return Trd_Common::TrdSide_Sell;
		else
			return Trd_Common::TrdSide_Buy;
}

inline WTSDirectionType wrapDirectionType(google::protobuf::int32 side)
{
	if (Trd_Common::TrdSide_Buy == side)
		return WDT_LONG;
	else
		return WDT_SHORT;
}

inline XTP_POSITION_EFFECT_TYPE wrapOffsetType(WTSOffsetType offType)
{
	if (WOT_OPEN == offType)
		return XTP_POSITION_EFFECT_OPEN;
	else if (WOT_CLOSE == offType)
		return XTP_POSITION_EFFECT_CLOSE;
	else if (WOT_CLOSETODAY == offType)
		return XTP_POSITION_EFFECT_CLOSETODAY;
	else if (WOT_CLOSEYESTERDAY == offType)
		return XTP_POSITION_EFFECT_CLOSE;
	else
		return XTP_POSITION_EFFECT_FORCECLOSE;
}

inline WTSOffsetType wrapOffsetType(XTP_POSITION_EFFECT_TYPE offType)
{
	if (XTP_POSITION_EFFECT_OPEN == offType)
		return WOT_OPEN;
	else if (XTP_POSITION_EFFECT_CLOSE == offType)
		return WOT_CLOSE;
	else if (XTP_POSITION_EFFECT_CLOSETODAY == offType)
		return WOT_CLOSETODAY;
	else if (XTP_POSITION_EFFECT_CLOSE == offType)
		return WOT_CLOSEYESTERDAY;
	else
		return WOT_FORCECLOSE;
}

inline WTSPriceType wrapPriceType(XTP_PRICE_TYPE priceType)
{
	if (XTP_PRICE_LIMIT == priceType)
		return WPT_LIMITPRICE;
	else 
		return WPT_ANYPRICE;
}

inline WTSOrderState wrapOrderState(google::protobuf::int32 orderState)
{
	switch (orderState)
	{
	case Trd_Common::OrderStatus_Unsubmitted:
		return WOS_NotTraded_NotQueuing;
	case Trd_Common::OrderStatus_Filled_All:
		return WOS_AllTraded;
	case Trd_Common::OrderStatus_Filled_Part:
		return WOS_PartTraded_Queuing;
	case Trd_Common::OrderStatus_Cancelled_Part:
		return WOS_Canceled; // WOS_PartTraded_NotQueuing;
	case Trd_Common::OrderStatus_Submitted:
		return WOS_NotTraded_Queuing;
	case Trd_Common:: OrderStatus_Cancelled_All:
		return WOS_Canceled;
	default:
		return WOS_Nottouched;
	}
}


TraderFUTU::TraderFUTU()
	: _api(NULL)
	, _sink(NULL)
	, _ordref(1)
	, _reqid(1)
	, _orders(NULL)
	, _trades(NULL)
	, _positions(NULL)
	, _bd_mgr(NULL)
	, _tradingday(0)
{
	static std::random_device rd;
	static std::uniform_int_distribution<uint64_t> dist(0ULL, 0xFFFFFFFFFFFFFFFFULL);
	_uniq_id = dist(rd) >> 32U;
}


TraderFUTU::~TraderFUTU()
{
}


WTSEntrust* TraderFUTU::makeEntrust(const Trd_Common::Order& order_info)
{
	const std::string& code = order_info.code();
	std::string exchg("HKSE");
	switch (order_info.secmarket())
	{
	case Qot_Common::QotMarket_CNSH_Security:
		exchg = "SSE";
		break;
	case Qot_Common::QotMarket_CNSZ_Security:
		exchg = "SZSE";
		break;
	case Qot_Common::QotMarket_HK_Security:
		exchg = "HKSE";
		break;
	default:
		break;
	}

	WTSContractInfo* ct = _bd_mgr->getContract(code.c_str(), exchg.c_str());
	if (ct == NULL)
		return NULL;

	WTSEntrust* pRet = WTSEntrust::create(
		code.c_str(),
		(uint32_t)order_info.qty(),
		order_info.price(),
		ct->getExchg());
	pRet->setContractInfo(ct);
	pRet->setDirection(wrapDirectionType(order_info->side, order_info->position_effect));
	pRet->setPriceType(wrapPriceType(order_info->price_type));
	pRet->setOffsetType(wrapOffsetType(order_info->position_effect));
	pRet->setOrderFlag(WOF_NOR);

	pRet->setEntrustID(genEntrustID(order_info->order_client_id).c_str());

	std::string usertag = _ini.readString(ENTRUST_SECTION, pRet->getEntrustID());
	if (!usertag.empty())
		pRet->setUserTag(usertag.c_str());

	return pRet;
}

WTSOrderInfo* TraderFUTU::makeOrderInfo(const Trd_Common::Order order_info)
{
	std::string code, exchg;
	if (order_info->market == XTP_MKT_SH_A)
		exchg = "SSE";
	else
		exchg = "SZSE";
	code = order_info->ticker;
	WTSContractInfo* contract = _bd_mgr->getContract(code.c_str(), exchg.c_str());
	if (contract == NULL)
		return NULL;

	WTSOrderInfo* pRet = WTSOrderInfo::create();
	pRet->setContractInfo(contract);
	pRet->setPrice(order_info->price);
	pRet->setVolume((uint32_t)order_info->quantity);
	pRet->setDirection(wrapDirectionType(order_info->side, order_info->position_effect));
	pRet->setPriceType(wrapPriceType(order_info->price_type));
	pRet->setOrderFlag(WOF_NOR);
	pRet->setOffsetType(wrapOffsetType(order_info->position_effect));

	pRet->setVolTraded((uint32_t)order_info->qty_traded);
	pRet->setVolLeft((uint32_t)order_info->qty_left);

	pRet->setCode(code.c_str());
	pRet->setExchange(contract->getExchg());

	pRet->setOrderDate((uint32_t)(order_info->insert_time / 1000000000));
	uint32_t uTime = order_info->insert_time % 1000000000;
	pRet->setOrderTime(TimeUtils::makeTime(pRet->getOrderDate(), uTime));

	pRet->setOrderState(wrapOrderState(order_info->order_status));
	if (order_info->order_status >= XTP_ORDER_STATUS_REJECTED)
		pRet->setError(true);

	pRet->setEntrustID(genEntrustID(order_info->order_client_id).c_str());
	pRet->setOrderID(fmt::format("{}", order_info->order_xtp_id).c_str());

	pRet->setStateMsg("");

	std::string usertag = _ini.readString(ENTRUST_SECTION, pRet->getEntrustID(), "");
	if (usertag.empty())
	{
		pRet->setUserTag(pRet->getEntrustID());
	}
	else
	{
		pRet->setUserTag(usertag.c_str());

		if (strlen(pRet->getOrderID()) > 0)
		{
			_ini.writeString(ORDER_SECTION, StrUtil::trim(pRet->getOrderID()).c_str(), usertag.c_str());
			_ini.save();
		}
	}

	return pRet;
}

WTSTradeInfo* TraderFUTU::makeTradeInfo(const Trd_Common::OrderFill& trade_info)
{
	std::string code, exchg;
	if (trade_info->market == XTP_MKT_SH_A)
		exchg = "SSE";
	else
		exchg = "SZSE";
	code = trade_info->ticker;
	WTSContractInfo* contract = _bd_mgr->getContract(code.c_str(), exchg.c_str());
	if (contract == NULL)
		return NULL;

	WTSTradeInfo *pRet = WTSTradeInfo::create(code.c_str(), exchg.c_str());
	pRet->setVolume((uint32_t)trade_info->quantity);
	pRet->setPrice(trade_info->price);
	pRet->setTradeID(trade_info->exec_id);
	pRet->setContractInfo(contract);

	uint32_t uTime = (uint32_t)(trade_info->trade_time % 1000000000);
	uint32_t uDate = (uint32_t)(trade_info->trade_time / 1000000000);

	pRet->setTradeDate(uDate);
	pRet->setTradeTime(TimeUtils::makeTime(uDate, uTime));

	WTSDirectionType dType = wrapDirectionType(trade_info->side, trade_info->position_effect);

	pRet->setDirection(dType);
	pRet->setOffsetType(wrapOffsetType(trade_info->position_effect));
	pRet->setRefOrder(fmt::format("{}", trade_info->order_xtp_id).c_str());
	pRet->setTradeType(WTT_Common);

	double amount = trade_info->quantity*pRet->getPrice();
	pRet->setAmount(amount);

	std::string usertag = _ini.readString(ORDER_SECTION, StrUtil::trim(pRet->getRefOrder()).c_str());
	if (!usertag.empty())
		pRet->setUserTag(usertag.c_str());

	return pRet;
}

void TraderFUTU::OnPush_UpdateOrder(const Trd_UpdateOrder::Response &stRsp)
{
	if (stRsp.rettype() != 0)
	{
		WTSEntrust* entrust = makeEntrust(stRsp.s2c().order());

		// WTSError* error = WTSError::create(WEC_ORDERCANCEL, error_info->error_msg);
		// _sink->onTraderError(error);
		// error->release();

		WTSError* error = WTSError::create(WEC_ORDERINSERT, stRsp.retmsg().c_str());
		_sink->onRspEntrust(entrust, error);
		error->release();

		entrust->release();
	}
	else
	{
		WTSOrderInfo *orderInfo = makeOrderInfo(stRsp.s2c().order());
		if (orderInfo)
		{
			if (_sink)
				_sink->onPushOrder(orderInfo);

			orderInfo->release();
		}
	}
}


void TraderFUTU::OnPush_UpdateOrderFill(const Trd_UpdateOrderFill::Response &stRsp)
{
	WTSTradeInfo *trdInfo = makeTradeInfo(stRsp.s2c().orderfill());
	if (trdInfo)
	{
		if (_sink)
			_sink->onPushTrade(trdInfo);

		trdInfo->release();
	}
}


void TraderFUTU::OnReply_GetHistoryOrderList(Futu::u32_t nSerialNo, const Trd_GetHistoryOrderList::Response &stRsp)
{
	if (stRsp.rettype() == 0)
	{
		if (NULL == _orders)
			_orders = WTSArray::create();

		int count = stRsp.s2c().orderlist_size();
		for (int i = 0; i < count; i++) {
			const auto& order = stRsp.s2c().orderlist(i);
			WTSOrderInfo* orderInfo = makeOrderInfo(order);
			if (orderInfo)
			{
				_orders->append(orderInfo, false);
			}
		}

		if (_sink)
			_sink->onRspOrders(_orders);

		if (_orders)
			_orders->clear();
	}
}

void TraderFUTU::OnReply_GetHistoryOrderFillList(Futu::u32_t nSerialNo, const Trd_GetHistoryOrderFillList::Response &stRsp)
{
	if (stRsp.rettype() == 0)
	{
		if (NULL == _trades)
			_trades = WTSArray::create();

		int count = stRsp.s2c().orderfilllist_size();
		for (int i = 0; i < count; i++) {
			const auto& orderfill = stRsp.s2c().orderfilllist(i);
			WTSTradeInfo* trdInfo = makeTradeInfo(orderfill);
			if (trdInfo)
			{
				_trades->append(trdInfo, false);
			}
		}

		if (_sink)
			_sink->onRspTrades(_trades);

		if (_trades)
			_trades->clear();
	}
}

void TraderFUTU::OnReply_GetPositionList(Futu::u32_t nSerialNo, const Trd_GetPositionList::Response &stRsp)
{
	if (stRsp.rettype() == 0)
	{
		if (NULL == _positions)
			_positions = PositionMap::create();

		int count = stRsp.s2c().positionlist_size();
		for (int i = 0; i < count; i++) 
		{
			const auto& position = stRsp.s2c().positionlist(i);
			const string& code = position.code();
			string exchg("HKSE");
			switch (position.secmarket())
			{
				case Qot_Common::QotMarket::QotMarket_CNSH_Security:
					exchg = "SSE";
					break;
				case Qot_Common::QotMarket::QotMarket_CNSZ_Security:
					exchg = "SZSE";
					break;
				case Qot_Common::QotMarket::QotMarket_HK_Security:
					exchg = "HKSE";
					break;
				default:
					break;
			}

			WTSContractInfo* contract = _bd_mgr->getContract(code.c_str(), exchg.c_str());
			if (contract)
			{
				WTSCommodityInfo* commInfo = contract->getCommInfo();
				std::string key = fmt::format("{}-{}", code.c_str(), position.positionside());
				WTSPositionItem* pos = (WTSPositionItem*)_positions->get(key);
				if (pos == NULL)
				{
					pos = WTSPositionItem::create(code.c_str(), commInfo->getCurrency(), commInfo->getExchg());
					pos->setContractInfo(contract);
					_positions->add(key, pos, false);
				}
				pos->setDirection(wrapPosDirection(position.positionside()));

				pos->setNewPosition((double)(position->total_qty - position->yesterday_position));
				pos->setPrePosition((double)position->yesterday_position);

				pos->setMargin(position->total_qty*position->avg_price);
				pos->setDynProfit(0);
				pos->setPositionCost(position->total_qty*position->avg_price);

				pos->setAvgPrice(position->avg_price);

				pos->setAvailNewPos(0);
				pos->setAvailPrePos((double)position->sellable_qty);
			}
		}

		WTSArray* ayPos = WTSArray::create();

		if (_positions && _positions->size() > 0)
		{
			for (auto it = _positions->begin(); it != _positions->end(); it++)
			{
				ayPos->append(it->second, true);
			}
		}

		if (_sink)
			_sink->onRspPosition(ayPos);

		if (_positions)
		{
			_positions->release();
			_positions = NULL;
		}

		ayPos->release();
	}
}


void TraderFUTU::OnReply_GetFunds(Futu::u32_t nSerialNo, const Trd_GetFunds::Response &stRsp)
{
	if (stRsp.rettype() == 0)
	{
		WTSAccountInfo* accountInfo = WTSAccountInfo::create();
		accountInfo->setPreBalance(asset->orig_banlance);
		accountInfo->setCloseProfit(0);
		accountInfo->setDynProfit(0);
		accountInfo->setMargin(0);
		accountInfo->setAvailable(asset->buying_power);
		accountInfo->setCommission(asset->fund_sell_fee);
		accountInfo->setFrozenMargin(asset->withholding_amount);
		accountInfo->setFrozenCommission(0);
		if (asset->deposit_withdraw > 0)
			accountInfo->setDeposit(asset->deposit_withdraw);
		else if (asset->deposit_withdraw < 0)
			accountInfo->setWithdraw(0);
		accountInfo->setBalance(asset->total_asset);
		accountInfo->setCurrency("CNY");

		WTSArray * ay = WTSArray::create();
		ay->append(accountInfo, false);
		if (_sink)
			_sink->onRspAccount(ay);

		ay->release();
	}
}

bool TraderFUTU::init(WTSVariant *params)
{
	_user = params->getCString("user");
	_host = params->getCString("host");
	_port = params->getInt32("port");

	return true;
}

void TraderFUTU::release()
{
	if (_api)
	{
		_api->RegisterSpi(NULL);
		_api->Release();
		_api = NULL;
	}

	if (_orders)
		_orders->clear();

	if (_positions)
		_positions->clear();

	if (_trades)
		_trades->clear();
}

void TraderFUTU::registerSpi(ITraderSpi *listener)
{
	_sink = listener;
	if (_sink)
	{
		_bd_mgr = listener->getBaseDataMgr();
	}
}

void TraderFUTU::reconnect()
{
	if (_api)
	{
		_api->RegisterSpi(NULL);
		_api->Release();
		_api = NULL;
	}

	std::stringstream ss;
	ss << _flowdir << "flows/" << _user << "/";
	boost::filesystem::create_directories(ss.str().c_str());
	_api = m_funcCreator(_client, ss.str().c_str(), XTP_LOG_LEVEL_DEBUG);			// 创建UserApi
	if (_api == NULL)
	{
		if (_sink)
			_sink->handleEvent(WTE_Connect, -1);
		write_log(_sink,LL_ERROR, "[TraderrXTP] Module initializing failed");

		StdThreadPtr thrd(new StdThread([this](){
			std::this_thread::sleep_for(std::chrono::seconds(2));
			write_log(_sink,LL_WARN, "[TraderrXTP] {} reconnecting...", _user.c_str());
			reconnect();
		}));
		return;
	}

	_api->SubscribePublicTopic(_quick ? XTP_TERT_QUICK : XTP_TERT_RESUME);
	_api->SetSoftwareVersion("1.0.0"); //设定此软件的开发版本号,用户自定义
	_api->SetSoftwareKey(_acckey.c_str());//设定用户的开发代码,在XTP申请开户时,由xtp人员提供
	_api->SetHeartBeatInterval(15);//设定交易服务器超时时间,单位为秒,此为1.1.16新增接口
	_api->RegisterSpi(this);						// 注册事件

	if (_sink)
		_sink->handleEvent(WTE_Connect, 0);
}

void TraderFUTU::connect()
{
	reconnect();

	if (_thrd_worker == NULL)
	{
		boost::asio::io_service::work work(_asyncio);
		_thrd_worker.reset(new StdThread([this](){
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::seconds(2));
				_asyncio.run_one();
				//m_asyncIO.run();
			}
		}));
	}
}

void TraderFUTU::disconnect()
{
	release();
}

bool TraderFUTU::isConnected()
{
	return (_state == TS_ALLREADY);
}

std::string TraderFUTU::genEntrustID(uint32_t orderRef)
{
	static char buffer[64] = { 0 };
	fmtutil::format_to(buffer, "{}#{}#{}#{}", _user, _tradingday, _uniq_id, orderRef);
	return buffer;
}

bool TraderFUTU::makeEntrustID(char* buffer, int length)
{
	if (buffer == NULL || length == 0)
		return false;

	try
	{
		uint32_t orderref = _ordref.fetch_add(1) + 1;
		fmtutil::format_to(buffer, "{}#{}#{}#{}", _user, _tradingday, _uniq_id, orderref);
		return true;
	}
	catch (...)
	{

	}

	return false;
}

int TraderFUTU::login(const char* user, const char* pass, const char* productInfo)
{
	_user = user;
	_pass = pass;

	if (_api == NULL)
	{
		return -1;
	}

	_state = TS_LOGINING;

	uint64_t iResult = _api->Login(_host.c_str(), _port, user, pass, XTP_PROTOCOL_TCP);
	if (iResult == 0)
	{
		auto error_info = _api->GetApiLastError();
		write_log(_sink,LL_ERROR, "[TraderFUTU] Login failed: {}", error_info->error_msg);
		std::string msg = error_info->error_msg;
		_state = TS_LOGINFAILED;
		_asyncio.post([this, msg]{
			_sink->onLoginResult(false, msg.c_str(), 0);
		});
	}
	else
	{
		_sessionid = iResult;
		_tradingday = strtoul(_api->GetTradingDay(), NULL, 10);

		std::stringstream ss;
		ss << "./xtpdata/local/";
		std::string path = StrUtil::standardisePath(ss.str());
		if (!StdFile::exists(path.c_str()))
			boost::filesystem::create_directories(path.c_str());
		ss << _user << ".dat";

		_ini.load(ss.str().c_str());
		uint32_t lastDate = _ini.readUInt("marker", "date", 0);
		if (lastDate != _tradingday)
		{
			//交易日不同,清理掉原来的数据
			_ini.removeSection(ORDER_SECTION);
			_ini.writeUInt("marker", "date", _tradingday);
			_ini.save();

			write_log(_sink,LL_INFO, "[TraderFUTU] [{}] Trading date changed [{} -> {}], local cache cleared...", _user.c_str(), lastDate, _tradingday);
		}		

		write_log(_sink,LL_INFO, "[TraderFUTU] [{}] Login succeed, trading date: {}...", _user.c_str(), _tradingday);

		_state = TS_LOGINED;
		_asyncio.post([this]{
			_sink->onLoginResult(true, 0, _tradingday);
			_state = TS_ALLREADY;
		});
	}
	return 0;
}

int TraderFUTU::logout()
{
	if (_api == NULL)
		return -1;

	_api->Logout(_sessionid);
	return 0;
}

int TraderFUTU::orderInsert(WTSEntrust* entrust)
{
	if (_api == NULL || _state != TS_ALLREADY)
	{
		return -1;
	}

	XTPOrderInsertInfo req;
	memset(&req, 0, sizeof(req));
	
	req.order_client_id = _ordref;
	strcpy(req.ticker, entrust->getCode());
	req.market = wt_stricmp(entrust->getExchg(), "SSE") == 0 ? XTP_MKT_SH_A : XTP_MKT_SZ_A;
	req.price = entrust->getPrice();
	req.quantity = (int64_t)entrust->getVolume();
	req.price_type = XTP_PRICE_LIMIT;
	req.side = wrapDirectionType(entrust->getDirection(), entrust->getOffsetType());
	req.business_type = XTP_BUSINESS_TYPE_CASH;
	req.position_effect = wrapOffsetType(entrust->getOffsetType());

	if (strlen(entrust->getUserTag()) > 0)
	{
		//m_mapEntrustTag[entrust->getEntrustID()] = entrust->getUserTag();
		_ini.writeString(ENTRUST_SECTION, entrust->getEntrustID(), entrust->getUserTag());
		_ini.save();
	}

	uint64_t iResult = _api->InsertOrder(&req, _sessionid);
	if (iResult == 0)
	{
		auto error_info = _api->GetApiLastError();
		write_log(_sink,LL_ERROR, "[TraderFUTU] Order inserting failed: {}", error_info->error_msg);
	}

	return 0;
}

int TraderFUTU::orderAction(WTSEntrustAction* action)
{
	if (_api == NULL || _state != TS_ALLREADY)
	{
		return -1;
	}

	uint64_t iResult = _api->CancelOrder(strtoull(action->getOrderID(), NULL, 10), _sessionid);
	if (iResult == 0)
	{
		auto error_info = _api->GetApiLastError();
		write_log(_sink,LL_ERROR, "[TraderFUTU] Order cancelling failed: {}", error_info->error_msg);
	}

	return 0;
}

uint32_t TraderFUTU::genRequestID()
{
	return _reqid.fetch_add(1) + 1;
}

int TraderFUTU::queryAccount()
{
	int iResult = _api->QueryAsset(_sessionid, genRequestID());
	if (iResult != 0)
	{
		auto error_info = _api->GetApiLastError();
		write_log(_sink,LL_ERROR, "[TraderFUTU] Account querying failed: {}", error_info->error_msg);
	}

	return 0;
}

int TraderFUTU::queryPositions()
{
	int iResult = _api->QueryPosition("", _sessionid, genRequestID());
	if (iResult != 0)
	{
		auto error_info = _api->GetApiLastError();
		write_log(_sink,LL_ERROR, "[TraderFUTU] Positions querying failed: {}", error_info->error_msg);
	}

	return 0;
}

int TraderFUTU::queryOrders()
{
	XTPQueryOrderReq req;
	memset(&req, 0, sizeof(XTPQueryOrderReq));
	int iResult = _api->QueryOrders(&req, _sessionid, genRequestID());
	if (iResult != 0)
	{
		auto error_info = _api->GetApiLastError();
		write_log(_sink,LL_ERROR, "[TraderFUTU] Orders querying failed: {}", error_info->error_msg);
	}

	return 0;
}

int TraderFUTU::queryTrades()
{
	XTPQueryTraderReq req;
	memset(&req, 0, sizeof(XTPQueryTraderReq));
	int iResult = _api->QueryTrades(&req, _sessionid, genRequestID());
	if (iResult != 0)
	{
		auto error_info = _api->GetApiLastError();
		write_log(_sink,LL_ERROR, "[TraderFUTU] Trades querying failed: {}", error_info->error_msg);
	}

	return 0;
}
