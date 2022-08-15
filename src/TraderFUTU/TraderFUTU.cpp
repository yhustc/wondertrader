
#include <time.h>
#include "TraderFUTU.h"

#include "../Includes/IBaseDataMgr.h"
#include "../Includes/WTSContractInfo.hpp"
#include "../Includes/WTSSessionInfo.hpp"
#include "../Includes/WTSTradeDef.hpp"
#include "../Includes/WTSError.hpp"
#include "../Includes/WTSVariant.hpp"

#include "../Share/ModuleHelper.hpp"

#include <boost/filesystem.hpp>

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


inline WTSPriceType wrapPriceType(google::protobuf::int32 orderType)
{
	if (Trd_Common::OrderType_Normal == orderType)
		return WPT_LIMITPRICE;
	else 
		return WPT_ANYPRICE;
}

inline WTSOffsetType wrapOffsetType(google::protobuf::int32 side)
{
	if (Trd_Common::TrdSide_Buy == side)
		return WOT_OPEN;
	else
		return WOT_CLOSE;
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
	case Trd_Common::OrderStatus_Submitted:
		return WOS_NotTraded_Queuing;
	case Trd_Common::OrderStatus_Cancelled_Part:
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
	, _thrd_worker(NULL)
{
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
	case Trd_Common::TrdSecMarket_CN_SH:
		exchg = "SSE";
		break;
	case Trd_Common::TrdSecMarket_CN_SZ:
		exchg = "SZSE";
		break;
	case Trd_Common::TrdSecMarket_HK:
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
	pRet->setDirection(WDT_LONG);  // 股票只有做多
	pRet->setPriceType(wrapPriceType(order_info.ordertype()));
	pRet->setOffsetType(wrapOffsetType(order_info.trdside()));
	pRet->setOrderFlag(WOF_NOR);

	pRet->setEntrustID(genEntrustID(order_info.orderid()).c_str());

	std::string usertag = _ini.readString(ENTRUST_SECTION, pRet->getEntrustID());
	if (!usertag.empty())
		pRet->setUserTag(usertag.c_str());

	return pRet;
}

WTSOrderInfo* TraderFUTU::makeOrderInfo(const Trd_Common::Order& order_info)
{
	const std::string& code = order_info.code();
	std::string exchg("HKSE");
	switch (order_info.secmarket())
	{
	case Trd_Common::TrdSecMarket_CN_SH:
		exchg = "SSE";
		break;
	case Trd_Common::TrdSecMarket_CN_SZ:
		exchg = "SZSE";
		break;
	case Trd_Common::TrdSecMarket_HK:
		exchg = "HKSE";
		break;
	default:
		break;
	}

	WTSContractInfo* contract = _bd_mgr->getContract(code.c_str(), exchg.c_str());
	if (contract == NULL)
		return NULL;

	WTSOrderInfo* pRet = WTSOrderInfo::create();
	pRet->setContractInfo(contract);
	pRet->setPrice(order_info.price());
	pRet->setVolume(order_info.qty());
	pRet->setDirection(WDT_LONG);  // 股票只有做多
	pRet->setPriceType(wrapPriceType(order_info.ordertype()));
	pRet->setOffsetType(wrapOffsetType(order_info.trdside()));
	pRet->setOrderFlag(WOF_NOR);

	pRet->setVolTraded(order_info.fillqty());
	pRet->setVolLeft(order_info.qty() - order_info.fillqty());

	pRet->setCode(code.c_str());
	pRet->setExchange(contract->getExchg());

	const time_t create_ts = (const time_t)order_info.updatetimestamp();  // 单位是秒
	struct tm* dt = localtime(&create_ts);
	uint32_t create_date = (1900 + dt->tm_year) * 10000 + (1 + dt->tm_mon) * 100 + dt->tm_mday;
	uint32_t create_time = (dt->tm_hour * 10000 + dt->tm_min * 100 + dt->tm_sec) * 1000;  // 转换为毫秒
	pRet->setOrderDate(create_date);
	pRet->setOrderTime(TimeUtils::makeTime(pRet->getOrderDate(), create_time));

	pRet->setOrderState(wrapOrderState(order_info.orderstatus()));
	if (order_info.orderstatus() == Trd_Common::OrderStatus_SubmitFailed || order_info.orderstatus() == Trd_Common::OrderStatus_Failed)
		pRet->setError(true);

	pRet->setEntrustID(genEntrustID(order_info.orderid()).c_str());
	pRet->setOrderID(fmt::format("{}", order_info.orderid()).c_str());

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
	const std::string& code = trade_info.code();
	std::string exchg("HKSE");
	switch (trade_info.secmarket())
	{
	case Trd_Common::TrdSecMarket_CN_SH:
		exchg = "SSE";
		break;
	case Trd_Common::TrdSecMarket_CN_SZ:
		exchg = "SZSE";
		break;
	case Trd_Common::TrdSecMarket_HK:
		exchg = "HKSE";
		break;
	default:
		break;
	}

	WTSContractInfo* contract = _bd_mgr->getContract(code.c_str(), exchg.c_str());
	if (contract == NULL)
		return NULL;

	WTSTradeInfo *pRet = WTSTradeInfo::create(code.c_str(), exchg.c_str());
	pRet->setVolume(trade_info.qty());
	pRet->setPrice(trade_info.price());
	pRet->setTradeID(fmt::to_string(trade_info.fillid()).c_str());
	pRet->setContractInfo(contract);

	const time_t create_ts = (const time_t)trade_info.updatetimestamp();  // 单位是秒
	struct tm* dt = localtime(&create_ts);
	uint32_t create_date = (1900 + dt->tm_year) * 10000 + (1 + dt->tm_mon) * 100 + dt->tm_mday;
	uint32_t create_time = (dt->tm_hour * 10000 + dt->tm_min * 100 + dt->tm_sec) * 1000;  // 转换为毫秒
	pRet->setTradeDate(create_date);
	pRet->setTradeTime(TimeUtils::makeTime(create_date, create_time));

	pRet->setDirection(WDT_LONG);  // 股票只能做多
	pRet->setOffsetType(wrapOffsetType(trade_info.trdside()));
	pRet->setRefOrder(fmt::to_string(trade_info.orderid()).c_str());
	pRet->setTradeType(WTT_Common);

	double amount = trade_info.qty() * pRet->getPrice();
	pRet->setAmount(amount);

	std::string usertag = _ini.readString(ORDER_SECTION, StrUtil::trim(pRet->getRefOrder()).c_str());
	if (!usertag.empty())
		pRet->setUserTag(usertag.c_str());

	return pRet;
}


bool TraderFUTU::init(WTSVariant *params)
{
	_user = params->getInt32("user");

	if (params->getBoolean("simulate"))
		_env = Trd_Common::TrdEnv_Simulate;
	else
		_env = Trd_Common::TrdEnv_Real;

	if (strcasecmp(params->getCString("market"), "HK") == 0)
		_market = Trd_Common::TrdMarket_HK;
	else
		_market = Trd_Common::TrdMarket_CN;

	_host = params->getCString("host");
	_port = params->getInt32("port");

	return true;
}

void TraderFUTU::release()
{
	if (_api)
	{
		_api->UnregisterTrdSpi();
		_api->UnregisterConnSpi();
		FTAPI::ReleaseTrdApi(_api);
		_api = nullptr;
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

void TraderFUTU::connect()
{
	if (!_api)
	{
		_api = FTAPI::CreateTrdApi();
		_api->RegisterTrdSpi(this);
		_api->RegisterConnSpi(this);

		_api->InitConnect("127.0.0.1", 11111, false);
	}
}


void TraderFUTU::OnInitConnect(FTAPI_Conn* pConn, Futu::i64_t nErrCode, const char* strDesc)
{
	if (_sink)
		_sink->handleEvent(WTE_Connect, 0);  // 这里面会调用login
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
	//这里不再使用sessionid，因为每次登陆会不同，如果使用的话，可能会造成不唯一的情况
	fmtutil::format_to(buffer, "{}#{}#{}", _user, _tradingday, orderRef);

	return buffer;
}

bool TraderFUTU::makeEntrustID(char* buffer, int length)
{
	if (buffer == NULL || length == 0)
		return false;

	try
	{
		uint32_t orderref = _ordref.fetch_add(1) + 1;
		fmtutil::format_to(buffer, "{}#{}#{}", _user, _tradingday, orderref);
		return true;
	}
	catch (...)
	{

	}

	return false;
}

int TraderFUTU::login(const char* user, const char* pass, const char* productInfo)
{
	_user = stoi(user);

	if (_api == NULL)
	{
		return -1;
	}

	_state = TS_LOGINING;

	Trd_SubAccPush::Request req;
	Trd_SubAccPush::C2S *c2s = req.mutable_c2s();
	c2s->add_accidlist(_user);

	_SubAccPushSerialNo = _api->SubAccPush(req);
	write_log(_sink,LL_INFO, "[TraderFUTU] Request SubAccPush SerialNo: {}", _SubAccPushSerialNo);

	return 0;
}


void TraderFUTU::OnReply_SubAccPush(Futu::u32_t nSerialNo, const Trd_SubAccPush::Response &stRsp)
{
	if(nSerialNo == _SubAccPushSerialNo)
	{
		if (stRsp.rettype() != Common::RetType::RetType_Succeed)
		{
			std::string msg = stRsp.retmsg();
			write_log(_sink,LL_ERROR, "[TraderFUTU] Login failed: {}", msg);
			_state = TS_LOGINFAILED;

			if (_thrd_worker == NULL)
			{
				_thrd_worker.reset(new StdThread([this, msg]{
					_sink->onLoginResult(false, msg.c_str(), 0);
				}));
			}
		}
		else
		{
			time_t t = time(NULL);
			auto dt = localtime(&t);

			_tradingday = (1900 + dt->tm_year) * 10000 + (1 + dt->tm_mon) * 100 + dt->tm_mday;  // FUTU的交易日期只能取当前日期

			std::stringstream ss;
			ss << "./futudata/local/";
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

				write_log(_sink,LL_INFO, "[TraderFUTU] [{}] Trading date changed [{} -> {}], local cache cleared...", _user, lastDate, _tradingday);
			}		

			write_log(_sink,LL_INFO, "[TraderFUTU] [{}] Login succeed, trading date: {}...", _user, _tradingday);

			_state = TS_LOGINED;

			if (_thrd_worker == NULL)
			{
				_thrd_worker.reset(new StdThread([this]{
					_sink->onLoginResult(true, 0, _tradingday);
					_state = TS_ALLREADY;
				}));
			}
		}
	}
}

int TraderFUTU::logout()
{
	return 0;
}

int TraderFUTU::orderInsert(WTSEntrust* entrust)
{
	if (_api == NULL || _state != TS_ALLREADY)
	{
		return -1;
	}

	Trd_PlaceOrder::Request req;
	Trd_PlaceOrder::C2S *c2s = req.mutable_c2s();
	Trd_Common::TrdHeader *header = c2s->mutable_header();
	header->set_accid(_user);
	header->set_trdenv(_env);

	if (strcasecmp(entrust->getExchg(), "SSE") == 0) {
		header->set_trdmarket(Trd_Common::TrdMarket_CN);
		c2s->set_secmarket(Trd_Common::TrdSecMarket_CN_SH);
	}
	else if (strcasecmp(entrust->getExchg(), "SZSE") == 0) {
		header->set_trdmarket(Trd_Common::TrdMarket_CN);
		c2s->set_secmarket(Trd_Common::TrdSecMarket_CN_SZ);
	}
	else if (strcasecmp(entrust->getExchg(), "HKSE") == 0) {
		header->set_trdmarket(Trd_Common::TrdMarket_HK);
		c2s->set_secmarket(Trd_Common::TrdSecMarket_HK);
	}
	
	if (entrust->getOffsetType() == WOT_OPEN)
		c2s->set_trdside(Trd_Common::TrdSide_Buy);
	else
		c2s->set_trdside(Trd_Common::TrdSide_Sell);
	
	c2s->set_ordertype(Trd_Common::OrderType_Normal);
	c2s->set_code(entrust->getCode());
	c2s->set_qty(entrust->getVolume());
	c2s->set_price(entrust->getPrice());

	_PlaceOrderSet.insert(_api->PlaceOrder(req));

	return 0;
}

void TraderFUTU::OnReply_PlaceOrder(Futu::u32_t nSerialNo, const Trd_PlaceOrder::Response &stRsp)
{
	if (_PlaceOrderSet.find(nSerialNo) == _PlaceOrderSet.end())  // 不是自己发的单
		return;
	
	if (stRsp.rettype() != Common::RetType::RetType_Succeed)
		write_log(_sink,LL_ERROR, "[TraderFUTU] Order inserting failed: {}", stRsp.retmsg());
}

void TraderFUTU::OnPush_UpdateOrder(const Trd_UpdateOrder::Response &stRsp)
{
	if (stRsp.rettype() != Common::RetType::RetType_Succeed)
	{
		WTSEntrust* entrust = makeEntrust(stRsp.s2c().order());

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

int TraderFUTU::orderAction(WTSEntrustAction* action)  // cancel order
{
	if (_api == NULL || _state != TS_ALLREADY)
	{
		return -1;
	}

	// construct request message
	Trd_ModifyOrder::Request req;
	Trd_ModifyOrder::C2S *c2s = req.mutable_c2s();
	Trd_Common::TrdHeader *header = c2s->mutable_header();
	header->set_accid(_user);
	header->set_trdenv(_env);

	if (strcasecmp(action->getExchg(), "SSE") == 0) {
		header->set_trdmarket(Trd_Common::TrdMarket_CN);
	}
	else if (strcasecmp(action->getExchg(), "SZSE") == 0) {
		header->set_trdmarket(Trd_Common::TrdMarket_CN);
	}
	else if (strcasecmp(action->getExchg(), "HKSE") == 0) {
		header->set_trdmarket(Trd_Common::TrdMarket_HK);
	}
	
	c2s->set_orderid(strtoull(action->getOrderID(), NULL, 10));
	c2s->set_modifyorderop(Trd_Common::ModifyOrderOp_Cancel);

	_ModifyOrderSet.insert(_api->ModifyOrder(req));

	return 0;
}

void TraderFUTU::OnReply_ModifyOrder(Futu::u32_t nSerialNo, const Trd_ModifyOrder::Response &stRsp){
	if (_ModifyOrderSet.find(nSerialNo) == _ModifyOrderSet.end())  // 不是自己发的单
		return;

	if (stRsp.rettype() != Common::RetType_Succeed)
	{
		write_log(_sink,LL_ERROR, "[TraderFUTU] Order cancelling failed: {}", stRsp.retmsg());
		WTSError* error = WTSError::create(WEC_ORDERCANCEL, stRsp.retmsg().c_str());
		_sink->onTraderError(error);
		error->release();
	}
}

uint32_t TraderFUTU::genRequestID()
{
	return _reqid.fetch_add(1) + 1;
}

int TraderFUTU::queryAccount()
{
	Trd_GetFunds::Request req;
	Trd_GetFunds::C2S *c2s = req.mutable_c2s();
	Trd_Common::TrdHeader *header = c2s->mutable_header();
	header->set_accid(_user);
	header->set_trdenv(_env);
	header->set_trdmarket(_market);

	_GetFundsSet.insert(_api->GetFunds(req));

	return 0;
}


void TraderFUTU::OnReply_GetFunds(Futu::u32_t nSerialNo, const Trd_GetFunds::Response &stRsp)
{
	if (_GetFundsSet.find(nSerialNo) == _GetFundsSet.end())
		return;
	
	if (stRsp.rettype() != Common::RetType::RetType_Succeed)
	{
		write_log(_sink,LL_ERROR, "[TraderFUTU] Account querying failed: {}", stRsp.retmsg());
	}
	else
	{
		const Trd_Common::Funds& asset = stRsp.s2c().funds();

		WTSAccountInfo* accountInfo = WTSAccountInfo::create();
		// accountInfo->setPreBalance(asset->orig_banlance);  // 昨日余额, 取不到
		accountInfo->setCloseProfit(0);
		accountInfo->setDynProfit(0);
		accountInfo->setMargin(0);
		accountInfo->setAvailable(asset.cash());
		// accountInfo->setCommission(asset->fund_sell_fee);  // 交易费用
		accountInfo->setFrozenMargin(asset.margincallmargin());  // 系统预扣的资金
		accountInfo->setFrozenCommission(0);
		// if (asset->deposit_withdraw > 0)  // 当天出入金
		// 	accountInfo->setDeposit(asset->deposit_withdraw);
		// else if (asset->deposit_withdraw < 0)
		// 	accountInfo->setWithdraw(0);
		accountInfo->setBalance(asset.totalassets());

		if (_market == Trd_Common::TrdMarket_HK)
			accountInfo->setCurrency("HKD");
		else
			accountInfo->setCurrency("CNY");

		WTSArray * ay = WTSArray::create();
		ay->append(accountInfo, false);
		if (_sink)
			_sink->onRspAccount(ay);

		ay->release();
	}
}


int TraderFUTU::queryPositions()
{
	// construct request message
	Trd_GetPositionList::Request req;
	Trd_GetPositionList::C2S *c2s = req.mutable_c2s();
	Trd_Common::TrdHeader *header = c2s->mutable_header();

	header->set_accid(_user);
	header->set_trdenv(_env);
	header->set_trdmarket(_market);
	
	_GetPositionListSet.insert(_api->GetPositionList(req));

	return 0;
}

void TraderFUTU::OnReply_GetPositionList(Futu::u32_t nSerialNo, const Trd_GetPositionList::Response &stRsp)
{
	if (_GetPositionListSet.find(nSerialNo) == _GetPositionListSet.end())
		return;
	
	if (stRsp.rettype() != Common::RetType::RetType_Succeed)
	{
		write_log(_sink,LL_ERROR, "[TraderFUTU] Position querying failed: {}", stRsp.retmsg());
	}
	else
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
				case Trd_Common::TrdSecMarket_CN_SH:
					exchg = "SSE";
					break;
				case Trd_Common::TrdSecMarket_CN_SZ:
					exchg = "SZSE";
					break;
				case Trd_Common::TrdSecMarket_HK:
					exchg = "HKSE";
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
				pos->setDirection(position.positionside() == Trd_Common::PositionSide_Long ? WDT_LONG : WDT_SHORT);

				pos->setNewPosition((position.td_buyqty() - position.td_sellqty()));
				pos->setPrePosition(position.qty() - pos->getNewPosition());

				pos->setMargin(position.qty() * position.costprice());
				pos->setDynProfit(0);
				pos->setPositionCost(position.qty() * position.costprice());

				pos->setAvgPrice(position.costprice());

				pos->setAvailNewPos(0);
				pos->setAvailPrePos(position.cansellqty());
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

int TraderFUTU::queryOrders()
{
	Trd_GetOrderList::Request req;
	Trd_GetOrderList::C2S *c2s = req.mutable_c2s();
	Trd_Common::TrdHeader *header = c2s->mutable_header();
	header->set_accid(_user);
	header->set_trdenv(_env);
	header->set_trdmarket(_market);

	_GetOrderListSet.insert(_api->GetOrderList(req));

	return 0;
}

void TraderFUTU::OnReply_GetOrderList(Futu::u32_t nSerialNo, const Trd_GetOrderList::Response &stRsp)
{
	if (_GetOrderListSet.find(nSerialNo) == _GetOrderListSet.end())
		return;
	
	if (stRsp.rettype() != Common::RetType::RetType_Succeed)
	{
		write_log(_sink,LL_ERROR, "[TraderFUTU] Orders querying failed: {}", stRsp.retmsg());
	}
	else
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


int TraderFUTU::queryTrades()
{
	Trd_GetOrderFillList::Request req;
	Trd_GetOrderFillList::C2S *c2s = req.mutable_c2s();
	Trd_Common::TrdHeader *header = c2s->mutable_header();
	header->set_accid(_user);
	header->set_trdenv(_env);
	header->set_trdmarket(_market);

	_GetOrderFillListSet.insert(_api->GetOrderFillList(req));
	
	return 0;
}


void TraderFUTU::OnReply_GetOrderFillList(Futu::u32_t nSerialNo, const Trd_GetOrderFillList::Response &stRsp)
{
	if (_GetOrderFillListSet.find(nSerialNo) == _GetOrderFillListSet.end())
		return;
	
	if (stRsp.rettype() != Common::RetType::RetType_Succeed)
	{
		write_log(_sink,LL_ERROR, "[TraderFUTU] Trades querying failed: {}", stRsp.retmsg());
	}
	else
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

