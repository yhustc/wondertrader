/*!
 * \file ParserXTP.cpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief 
 */
#include "ParserXTP.h"

#include "../Includes/WTSDataDef.hpp"
#include "../Includes/WTSContractInfo.hpp"
#include "../Includes/WTSVariant.hpp"
#include "../Includes/IBaseDataMgr.h"

#include "../Share/TimeUtils.hpp"
#include "../Share/ModuleHelper.hpp"

#include <boost/filesystem.hpp>
#include <thread>

 //By Wesley @ 2022.01.05
#include "../Share/fmtlib.h"
template<typename... Args>
inline void write_log(IParserSpi* sink, WTSLogLevel ll, const char* format, const Args&... args)
{
	if (sink == NULL)
		return;

	static thread_local char buffer[512] = { 0 };
	memset(buffer, 0, 512);
	fmt::format_to(buffer, format, args...);

	sink->handleParserLog(ll, buffer);
}

extern "C"
{
	EXPORT_FLAG IParserApi* createParser()
	{
		ParserXTP* parser = new ParserXTP();
		return parser;
	}

	EXPORT_FLAG void deleteParser(IParserApi* &parser)
	{
		if (NULL != parser)
		{
			delete parser;
			parser = NULL;
		}
	}
};

inline uint32_t strToTime(const char* strTime)
{
	std::string str;
	const char *pos = strTime;
	while(strlen(pos) > 0)
	{
		if(pos[0] != ':')
		{
			str.append(pos, 1);
		}
		pos++;
	}

	return strtoul(str.c_str(), NULL, 10);
}

inline double checkValid(double val)
{
	if (val == DBL_MAX || val == FLT_MAX)
		return 0;

	return val;
}

ParserXTP::ParserXTP()
	:m_pUserAPI(NULL)
	,m_iRequestID(0)
	,m_uTradingDate(0)
{
}


ParserXTP::~ParserXTP()
{
	m_pUserAPI = NULL;
}

bool ParserXTP::init(WTSVariant* config)
{
	m_strHost	= config->getCString("host");
	m_iPort		= config->getInt32("port");
	m_strUser = config->getCString("user");
	m_strPass = config->getCString("pass");
	m_iProtocol = (XTP_PROTOCOL_TYPE)config->getUInt32("protocol");
	m_subtype = (SUB_DATA_TYPE)config->getUInt32("subtype");
	m_uClientID = config->getUInt32("clientid");
	m_uHBInterval = config->getUInt32("hbinterval");
	m_uBuffSize = config->getUInt32("buffsize");
	m_strFlowDir = config->getCString("flowdir");

	if (m_strFlowDir.empty())
		m_strFlowDir = "XTPMDFlow";

	m_strFlowDir = StrUtil::standardisePath(m_strFlowDir);

	std::string module = config->getCString("xtpmodule");
	if (module.empty())
		module = "xtpquoteapi";

	std::string path = StrUtil::printf("%s/%s/", m_strFlowDir.c_str(), m_strUser.c_str());
	boost::filesystem::create_directories(path.c_str());

	std::string dllpath = getBinDir() + DLLHelper::wrap_module(module.c_str(), "lib");;
	m_hInst = DLLHelper::load_library(dllpath.c_str());
#ifdef _WIN32
#	ifdef _WIN64
	const char* creatorName = "?CreateQuoteApi@QuoteApi@API@XTP@@SAPEAV123@EPEBDW4XTP_LOG_LEVEL@@@Z";
#	else
	const char* creatorName = "?CreateQuoteApi@QuoteApi@API@XTP@@SAPAV123@EPBDW4XTP_LOG_LEVEL@@@Z";
#	endif
#else
	const char* creatorName = "_ZN3XTP3API8QuoteApi14CreateQuoteApiEhPKc13XTP_LOG_LEVEL";
#endif
	m_funcCreator = (XTPCreater)DLLHelper::get_symbol(m_hInst, creatorName);
	m_pUserAPI = m_funcCreator(m_uClientID, path.c_str(), XTP_LOG_LEVEL_DEBUG);
	m_pUserAPI->RegisterSpi(this);

    //靠靠靠靠靠靠靠靠
	m_pUserAPI->SetHeartBeatInterval(m_uHBInterval); //靠1.1.16靠靠
	//靠靠靠靠靠靠靠MB
	m_pUserAPI->SetUDPBufferSize(m_uBuffSize);//靠1.1.16靠靠

	return true;
}

void ParserXTP::release()
{
	disconnect();
}

bool ParserXTP::connect()
{
	DoLogin();

	return true;
}

bool ParserXTP::disconnect()
{
	if(m_pUserAPI)
	{
		m_pUserAPI->RegisterSpi(NULL);
		m_pUserAPI->Release();
		m_pUserAPI = NULL;
	}

	return true;
}

void ParserXTP::OnError(XTPRI *error_info)
{
	IsErrorRspInfo(error_info);
}

/*
void ParserXTP::OnFrontConnected()
{
	if (m_sink)
	{
		write_log(m_sink, LL_INFO, "[ParserXTP]CTP行情服务已连接");
		m_sink->handleEvent(WPE_Connect, 0);
	}

	ReqUserLogin();
}

void ParserXTP::OnRspUserLogin( CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast )
{
	if(bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		m_uTradingDate = strtoul(m_pUserAPI->GetTradingDay(), NULL, 10);
		
		if(m_sink)
		{
			m_sink->handleEvent(WPE_Login, 0);
		}

		//订阅行情数据
		SubscribeMarketData();
	}
}
*/

void ParserXTP::OnDisconnected(int nReason)
{
	if(m_sink)
	{
		write_log(m_sink, LL_ERROR, "[ParserXTP] Market data server disconnected: {}...", nReason);
		m_sink->handleEvent(WPE_Close, 0);
	}
	
	std::this_thread::sleep_for(std::chrono::seconds(5));

	DoLogin();
}

void ParserXTP::OnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	if (!IsErrorRspInfo(error_info))
	{

	}
	else
	{
		if(m_sink)
			write_log(m_sink, LL_ERROR, "[ParserXTP] Tick data subscribe faile, code: {}.{}", ticker->exchange_id == XTP_EXCHANGE_SH ? "SSE" : "SZSE", ticker->ticker);
	}
}

void ParserXTP::OnUnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last)
{

}

void ParserXTP::OnTickByTick(XTPTBT *tbt_data)
{
	if(m_pBaseDataMgr == NULL || tbt_data->type == XTP_TBT_ENTRUST)
	{
		return;
	}

	uint32_t actDate = (uint32_t)(tbt_data->data_time / 1000000000);
	uint32_t actTime = tbt_data->data_time % 1000000000;
	uint32_t actHour = actTime / 10000000;

	std::string code, exchg;
	if (tbt_data->exchange_id == XTP_EXCHANGE_SH)
	{
		exchg = "SSE";
	}
	else
	{
		exchg = "SZSE";
	}
	code = tbt_data->ticker;

	WTSContractInfo* ct = m_pBaseDataMgr->getContract(code.c_str(), exchg.c_str());
	if(ct == NULL)
	{
		if (m_sink)
			write_log(m_sink, LL_ERROR, "[ParserXTP] Instrument {}.{} not exists...", exchg.c_str(), tbt_data->ticker);
		return;
	}
	WTSCommodityInfo* commInfo = ct->getCommInfo();

	WTSTickData* tick = WTSTickData::create(code.c_str());
	tick->setContractInfo(ct);
	WTSTickStruct& quote = tick->getTickStruct();
	strcpy(quote.exchg, commInfo->getExchg());
	
	quote.action_date = actDate;
	quote.action_time = actTime;

	XTPTickByTickTrade& trade = tbt_data->trade;
	
	quote.price = checkValid(trade.price);
	quote.total_volume = 0.0;
	quote.volume = (uint32_t)trade.qty;
	quote.trading_date = m_uTradingDate;
	quote.total_turnover = 0.0;
	quote.turn_over = checkValid(trade.price) * trade.qty;

	// 以下内容都没有, 所以全部填0
	quote.open = 0.0;
	quote.high = 0.0;
	quote.low = 0.0;
	quote.upper_limit = 0.0;
	quote.lower_limit = 0.0;
	quote.pre_close = 0.0;
	for (int i = 0; i < 10; i++)
	{
		quote.ask_prices[i] = 0.0;
		quote.ask_qty[i] = 0;

		quote.bid_prices[i] = 0.0;
		quote.bid_qty[i] = 0;
	}

	if(m_sink)
		m_sink->handleQuote(tick, 1);

	tick->release();
}


void ParserXTP::OnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	if (!IsErrorRspInfo(error_info))
	{

	}
	else
	{
		if(m_sink)
			write_log(m_sink, LL_ERROR, "[ParserXTP] Order Book subscribe faile, code: {}.{}", ticker->exchange_id == XTP_EXCHANGE_SH ? "SSE" : "SZSE", ticker->ticker);
	}
}

void ParserXTP::OnUnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last)
{

}

void ParserXTP::OnOrderBook(XTPOB *order_book)
{
	if(m_pBaseDataMgr == NULL)
	{
		return;
	}

	uint32_t actDate = (uint32_t)(order_book->data_time / 1000000000);
	uint32_t actTime = order_book->data_time % 1000000000;
	uint32_t actHour = actTime / 10000000;

	std::string code, exchg;
	if (order_book->exchange_id == XTP_EXCHANGE_SH)
	{
		exchg = "SSE";
	}
	else
	{
		exchg = "SZSE";
	}
	code = order_book->ticker;

	WTSContractInfo* ct = m_pBaseDataMgr->getContract(code.c_str(), exchg.c_str());
	if(ct == NULL)
	{
		if (m_sink)
			write_log(m_sink, LL_ERROR, "[ParserXTP] Instrument {}.{} not exists...", exchg.c_str(), order_book->ticker);
		return;
	}
	WTSCommodityInfo* commInfo = ct->getCommInfo();

	WTSTickData* tick = WTSTickData::create(code.c_str());
	tick->setContractInfo(ct);
	WTSTickStruct& quote = tick->getTickStruct();
	strcpy(quote.exchg, commInfo->getExchg());
	
	quote.action_date = actDate;
	quote.action_time = actTime;
	
	quote.price = checkValid(order_book->last_price);
	quote.total_volume = (uint32_t)order_book->qty;
	quote.volume = 0.0;
	quote.trading_date = m_uTradingDate;
	quote.total_turnover = order_book->turnover;
	quote.turn_over = 0.0;

	//委卖价格
	for (int i = 0; i < 10; i++)
	{
		quote.ask_prices[i] = checkValid(order_book->ask[i]);
		quote.ask_qty[i] = (uint32_t)order_book->ask_qty[i];

		quote.bid_prices[i] = checkValid(order_book->bid[i]);
		quote.bid_qty[i] = (uint32_t)order_book->bid_qty[i];
	}

	// 以下内容都没有, 所以全部填0
	quote.open = 0.0;
	quote.high = 0.0;
	quote.low = 0.0;
	quote.upper_limit = 0.0;
	quote.lower_limit = 0.0;
	quote.pre_close = 0.0;

	if(m_sink)
		m_sink->handleQuote(tick, 1);

	tick->release();
}

void ParserXTP::OnUnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
{

}

void ParserXTP::OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count)
{	
	if(m_pBaseDataMgr == NULL)
	{
		return;
	}

	uint32_t actDate = (uint32_t)(market_data->data_time / 1000000000);
	uint32_t actTime = market_data->data_time % 1000000000;
	uint32_t actHour = actTime / 10000000;

	std::string code, exchg;
	if (market_data->exchange_id == XTP_EXCHANGE_SH)
	{
		exchg = "SSE";
	}
	else
	{
		exchg = "SZSE";
	}
	code = market_data->ticker;

	WTSContractInfo* ct = m_pBaseDataMgr->getContract(code.c_str(), exchg.c_str());
	if(ct == NULL)
	{
		if (m_sink)
			write_log(m_sink, LL_ERROR, "[ParserXTP] Instrument {}.{} not exists...", exchg.c_str(), market_data->ticker);
		return;
	}
	WTSCommodityInfo* commInfo = ct->getCommInfo();

	WTSTickData* tick = WTSTickData::create(code.c_str());
	tick->setContractInfo(ct);
	WTSTickStruct& quote = tick->getTickStruct();
	strcpy(quote.exchg, commInfo->getExchg());
	
	quote.action_date = actDate;
	quote.action_time = actTime;
	
	quote.price = checkValid(market_data->last_price);
	quote.open = checkValid(market_data->open_price);
	quote.high = checkValid(market_data->high_price);
	quote.low = checkValid(market_data->low_price);
	quote.total_volume = (uint32_t)market_data->qty;
	quote.volume = 0.0;
	quote.trading_date = m_uTradingDate;
	quote.total_turnover = market_data->turnover;
	quote.turn_over = 0.0;

	if (commInfo->getCategoty() == CC_Future)
	{
		quote.settle_price = market_data->settl_price;
		quote.open_interest = (uint32_t)market_data->total_long_positon;

		quote.pre_settle = checkValid(market_data->pre_settl_price);
		quote.pre_interest = (uint32_t)market_data->pre_total_long_positon;
	}

	quote.upper_limit = checkValid(market_data->upper_limit_price);
	quote.lower_limit = checkValid(market_data->lower_limit_price);

	quote.pre_close = checkValid(market_data->pre_close_price);	

	//委卖价格
	for (int i = 0; i < 10; i++)
	{
		quote.ask_prices[i] = checkValid(market_data->ask[i]);
		quote.ask_qty[i] = (uint32_t)market_data->ask_qty[i];

		quote.bid_prices[i] = checkValid(market_data->bid[i]);
		quote.bid_qty[i] = (uint32_t)market_data->bid_qty[i];
	}

	if(m_sink)
		m_sink->handleQuote(tick, 1);

	tick->release();
}

void ParserXTP::OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	if (!IsErrorRspInfo(error_info))
	{

	}
	else
	{
		if(m_sink)
			write_log(m_sink, LL_ERROR, "[ParserXTP] Market data subscribe faile, code: {}.{}", ticker->exchange_id == XTP_EXCHANGE_SH ? "SSE" : "SZSE", ticker->ticker);
	}
}

void ParserXTP::DoLogin()
{
	if(m_pUserAPI == NULL)
	{
		return;
	}

	int iResult = m_pUserAPI->Login(m_strHost.c_str(), m_iPort, m_strUser.c_str(), m_strPass.c_str(), m_iProtocol);
	if(iResult != 0)
	{
		if (m_sink)
		{
			if(iResult == -1)
			{
				m_sink->handleEvent(WPE_Connect, iResult);
			}
			else
			{
				m_sink->handleEvent(WPE_Connect, 0);

				write_log(m_sink, LL_ERROR, "[ParserXTP] Sending login request failed: {}", iResult);
			}
			
		}
	}
	else
	{
		m_uTradingDate = strToTime(m_pUserAPI->GetTradingDay());
		if (m_sink)
		{
			m_sink->handleEvent(WPE_Connect, 0);
			m_sink->handleEvent(WPE_Login, 0);
		}

		DoSubscribeMD();
	}
}

void ParserXTP::DoSubscribeMD()
{
	CodeSet codeFilter = m_fitSHSubs;
	if(!codeFilter.empty())
	{
		char ** subscribe = new char*[codeFilter.size()];
		int nCount = 0;
		CodeSet::iterator it = codeFilter.begin();
		for (; it != codeFilter.end(); it++)
		{
			subscribe[nCount++] = (char*)(*it).c_str();
		}

		if (m_pUserAPI && nCount > 0)
		{
			int iResult = -1;
			if (m_subtype == SUB_TICK_DATA) {
				iResult = m_pUserAPI->SubscribeTickByTick(subscribe, nCount, XTP_EXCHANGE_SH);
			} else if (m_subtype == SUB_ORDER_BOOK) {
				iResult = m_pUserAPI->SubscribeOrderBook(subscribe, nCount, XTP_EXCHANGE_SH);
			} else if (m_subtype == SUB_MARKET_DATA) {
				iResult = m_pUserAPI->SubscribeMarketData(subscribe, nCount, XTP_EXCHANGE_SH);
			}

			if (iResult != 0)
			{
				if (m_sink)
					write_log(m_sink, LL_ERROR, "[ParserXTP] Sending md subscribe request of SSE failed: {}", iResult);
			}
			else
			{
				if (m_sink) {
					if (m_subtype == SUB_TICK_DATA)
						write_log(m_sink, LL_INFO, "[ParserXTP] Tick data of {} instruments of SSE subscribed", nCount);
					if (m_subtype == SUB_ORDER_BOOK)
						write_log(m_sink, LL_INFO, "[ParserXTP] Order book of {} instruments of SSE subscribed", nCount);
					if (m_subtype == SUB_MARKET_DATA)
						write_log(m_sink, LL_INFO, "[ParserXTP] Market data of {} instruments of SSE subscribed", nCount);
				}
			}
		}
		codeFilter.clear();
		delete[] subscribe;
		//int iResult = m_pUserAPI->SubscribeAllMarketData(XTP_EXCHANGE_SH);
	}

	codeFilter = m_fitSZSubs;
	if (!codeFilter.empty())
	{
		char ** subscribe = new char*[codeFilter.size()];
		int nCount = 0;
		CodeSet::iterator it = codeFilter.begin();
		for (; it != codeFilter.end(); it++)
		{
			subscribe[nCount++] = (char*)(*it).c_str();
		}

		if (m_pUserAPI && nCount > 0)
		{
			int iResult = -1;
			if (m_subtype == SUB_TICK_DATA) {
				iResult = m_pUserAPI->SubscribeTickByTick(subscribe, nCount, XTP_EXCHANGE_SZ);
			} else if (m_subtype == SUB_ORDER_BOOK) {
				iResult = m_pUserAPI->SubscribeOrderBook(subscribe, nCount, XTP_EXCHANGE_SZ);
			} else if (m_subtype == SUB_MARKET_DATA) {
				iResult = m_pUserAPI->SubscribeMarketData(subscribe, nCount, XTP_EXCHANGE_SZ);
			}

			if (iResult != 0)
			{
				if (m_sink)
					write_log(m_sink, LL_ERROR, "[ParserXTP] Sending md subscribe request of SZSE failed: {}", iResult);
			}
			else
			{
				if (m_sink) {
					if (m_subtype == SUB_TICK_DATA)
						write_log(m_sink, LL_INFO, "[ParserXTP] Tick data of {} instruments of SZSE subscribed", nCount);
					if (m_subtype == SUB_ORDER_BOOK)
						write_log(m_sink, LL_INFO, "[ParserXTP] Order book of {} instruments of SZSE subscribed", nCount);
					if (m_subtype == SUB_MARKET_DATA)
						write_log(m_sink, LL_INFO, "[ParserXTP] Market data of {} instruments of SZSE subscribed", nCount);
				}
			}
		}
		codeFilter.clear();
		delete[] subscribe;
		//int iResult = m_pUserAPI->SubscribeAllMarketData(XTP_EXCHANGE_SZ);
	}
}

bool ParserXTP::IsErrorRspInfo(XTPRI *error_info)
{
	if (error_info == NULL || error_info->error_id ==0)
		return false;

	return true;
}

void ParserXTP::subscribe(const CodeSet &vecSymbols)
{
	if(m_uTradingDate == 0)
	{
		for(auto& code : vecSymbols)
		{
			if (strncmp(code.c_str(), "SSE.", 4) == 0)
			{
				m_fitSHSubs.insert(code.c_str() + 4);
			}
			else if (strncmp(code.c_str(), "SZSE.", 5) == 0)
			{
				m_fitSZSubs.insert(code.c_str() + 5);
			}
		}
	}
	else
	{
		CodeSet setSH, setSZ;
		for (auto& code : vecSymbols)
		{
			if (strncmp(code.c_str(), "SSE.", 4) == 0)
			{
				m_fitSHSubs.insert(code.c_str() + 4);
				setSH.insert(code.c_str() + 4);
			}
			else if (strncmp(code.c_str(), "SZSE.", 5) == 0)
			{
				m_fitSZSubs.insert(code.c_str() + 5);
				setSZ.insert(code.c_str() + 5);
			}
		}

		if (!setSH.empty())
		{
			char ** subscribe = new char*[setSH.size()];
			int nCount = 0;
			CodeSet::iterator it = setSH.begin();
			for (; it != setSH.end(); it++)
			{
				subscribe[nCount++] = (char*)(*it).c_str();
			}

			if (m_pUserAPI && nCount > 0)
			{
				int iResult = -1;
				if (m_subtype == SUB_TICK_DATA) {
					iResult = m_pUserAPI->SubscribeTickByTick(subscribe, nCount, XTP_EXCHANGE_SH);
				} else if (m_subtype == SUB_ORDER_BOOK) {
					iResult = m_pUserAPI->SubscribeOrderBook(subscribe, nCount, XTP_EXCHANGE_SH);
				} else if (m_subtype == SUB_MARKET_DATA) {
					iResult = m_pUserAPI->SubscribeMarketData(subscribe, nCount, XTP_EXCHANGE_SH);
				}

				if (iResult != 0)
				{
					if (m_sink)
						write_log(m_sink, LL_ERROR, "[ParserXTP] Sending md subscribe request of SSE failed: {}", iResult);
				}
				else
				{
					if (m_sink) {
						if (m_subtype == SUB_TICK_DATA)
							write_log(m_sink, LL_INFO, "[ParserXTP] Tick data of {} instruments of SSE subscribed", nCount);
						if (m_subtype == SUB_ORDER_BOOK)
							write_log(m_sink, LL_INFO, "[ParserXTP] Order book of {} instruments of SSE subscribed", nCount);
						if (m_subtype == SUB_MARKET_DATA)
							write_log(m_sink, LL_INFO, "[ParserXTP] Market data of {} instruments of SSE subscribed", nCount);
					}
				}
			}
			delete[] subscribe;
		}

		if (!setSZ.empty())
		{
			char ** subscribe = new char*[setSZ.size()];
			int nCount = 0;
			CodeSet::iterator it = setSZ.begin();
			for (; it != setSZ.end(); it++)
			{
				subscribe[nCount++] = (char*)(*it).c_str();
			}

			if (m_pUserAPI && nCount > 0)
			{
				int iResult = -1;
				if (m_subtype == SUB_TICK_DATA) {
					iResult = m_pUserAPI->SubscribeTickByTick(subscribe, nCount, XTP_EXCHANGE_SZ);
				} else if (m_subtype == SUB_ORDER_BOOK) {
					iResult = m_pUserAPI->SubscribeOrderBook(subscribe, nCount, XTP_EXCHANGE_SZ);
				} else if (m_subtype == SUB_MARKET_DATA) {
					iResult = m_pUserAPI->SubscribeMarketData(subscribe, nCount, XTP_EXCHANGE_SZ);
				}

				if (iResult != 0)
				{
					if (m_sink)
						write_log(m_sink, LL_ERROR, "[ParserXTP] Sending md subscribe request of SZSE failed: {}", iResult);
				}
				else
				{
					if (m_sink) {
						if (m_subtype == SUB_TICK_DATA)
							write_log(m_sink, LL_INFO, "[ParserXTP] Tick data of {} instruments of SZSE subscribed", nCount);
						if (m_subtype == SUB_ORDER_BOOK)
							write_log(m_sink, LL_INFO, "[ParserXTP] Order book of {} instruments of SZSE subscribed", nCount);
						if (m_subtype == SUB_MARKET_DATA)
							write_log(m_sink, LL_INFO, "[ParserXTP] Market data of {} instruments of SZSE subscribed", nCount);
					}
				}
			}
			delete[] subscribe;
		}
	}
}

void ParserXTP::unsubscribe(const CodeSet &vecSymbols)
{
}

bool ParserXTP::isConnected()
{
	return m_pUserAPI!=NULL;
}

void ParserXTP::registerSpi(IParserSpi* listener)
{
	m_sink = listener;

	if(m_sink)
		m_pBaseDataMgr = m_sink->getBaseDataMgr();
}
