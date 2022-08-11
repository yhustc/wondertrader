
#include "ParserFUTU.h"

#include "../Includes/WTSDataDef.hpp"
#include "../Includes/WTSContractInfo.hpp"
#include "../Includes/WTSVariant.hpp"
#include "../Includes/IBaseDataMgr.h"

#include "../Share/TimeUtils.hpp"
#include "../Share/ModuleHelper.hpp"

#include <thread>

using namespace Futu;

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
        FTAPI::Init();
		ParserFUTU* parser = new ParserFUTU();
		return parser;
	}

	EXPORT_FLAG void deleteParser(IParserApi* &parser)
	{
		if (NULL != parser)
		{
			delete parser;
			parser = NULL;
		}
        FTAPI::UnInit();
	}
};


ParserFUTU::ParserFUTU()
	:m_pQotApi(NULL)
	,m_SubSerialNo(0)
	,m_uTradingDate(0)
{
}

ParserFUTU::~ParserFUTU()
{
	release();
}

bool ParserFUTU::init(WTSVariant* config)
{
	m_strHost	= config->getCString("host");
	m_iPort		= config->getInt32("port");
	return true;
}

void ParserFUTU::release()
{
	disconnect();
}

bool ParserFUTU::connect()
{
	m_pQotApi = FTAPI::CreateQotApi();
    m_pQotApi->RegisterQotSpi(this);
    m_pQotApi->RegisterConnSpi(this);

    m_pQotApi->InitConnect(m_strHost.c_str(), m_iPort, false);
	if (m_sink)
		write_log(m_sink, LL_INFO, "[ParserFUTU] Request connect to {}:{}", m_strHost, m_iPort);

	return true;
}

bool ParserFUTU::disconnect()
{
	if (m_pQotApi != nullptr)
    {
        m_pQotApi->UnregisterQotSpi();
        m_pQotApi->UnregisterConnSpi();
        FTAPI::ReleaseQotApi(m_pQotApi);
        m_pQotApi = nullptr;
    }

	return true;
}

void ParserFUTU::OnInitConnect(Futu::FTAPI_Conn* pConn, Futu::i64_t nErrCode, const char* strDesc)
{
    // 这个接口要先订阅
    Qot_Sub::Request req;
    Qot_Sub::C2S *c2s = req.mutable_c2s();
    auto secList = c2s->mutable_securitylist();

	if(!m_fitSHSubs.empty())
	{
		for (const auto& code : m_fitSHSubs) {
			Qot_Common::Security *sec = secList->Add();
			sec->set_code(code.c_str());
			sec->set_market(Qot_Common::QotMarket::QotMarket_CNSH_Security);
		}
	}

	if (!m_fitSZSubs.empty()) {
		for (const auto& code : m_fitSZSubs) {
			Qot_Common::Security *sec = secList->Add();
			sec->set_code(code.c_str());
			sec->set_market(Qot_Common::QotMarket::QotMarket_CNSZ_Security);
		}
	}

    c2s->add_subtypelist(Qot_Common::SubType::SubType_Ticker);
    c2s->set_isregorunregpush(true);
    c2s->set_issuborunsub(true);

    m_SubSerialNo = m_pQotApi->Sub(req);

	if (m_sink)
		write_log(m_sink, LL_INFO, "[ParserFUTU] Request Sub SerialNo: {}", m_SubSerialNo);
}


void ParserFUTU::OnReply_Sub(Futu::u32_t nSerialNo, const Qot_Sub::Response& stRsp)
{
    if(nSerialNo == m_SubSerialNo)
    {
        if (stRsp.rettype() != Common::RetType::RetType_Succeed)
        {
			if (m_sink)
				write_log(m_sink, LL_ERROR, "[ParserFUTU] SerialNo: {} Sub Failed", nSerialNo);
            return;
        }
        else
        {
			if (m_sink)
				write_log(m_sink, LL_INFO, "[ParserFUTU] SerialNo: {} Sub Success", nSerialNo);
            return;
        }
    }
}


void ParserFUTU::OnPush_UpdateTicker(const Qot_UpdateTicker::Response& stRsp)
{
	if(m_pBaseDataMgr == NULL)
	{
		return;
	}

	const string& code = stRsp.s2c().security().code();
	string exchg("HKSE");
	switch (stRsp.s2c().security().market())
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

	int count = stRsp.s2c().tickerlist_size();
    for(int i = 0; i < count; i++) {
		const auto& ticker = stRsp.s2c().tickerlist(i);

		const time_t rawtime = (const time_t)ticker.timestamp();  // 单位秒
		struct tm* dt = localtime(&rawtime);

		uint32_t actDate = (1900 + dt->tm_year) * 10000 + (1 + dt->tm_mon) * 100 + dt->tm_mday;
		uint32_t actTime = (dt->tm_hour * 10000 + dt->tm_min * 100 + dt->tm_sec) * 1000;  // 转换为毫秒
		uint32_t actHour = dt->tm_hour;

		WTSContractInfo* ct = m_pBaseDataMgr->getContract(code.c_str(), exchg.c_str());
		if(ct == NULL)
		{
			if (m_sink)
				write_log(m_sink, LL_ERROR, "[ParserFUTU] Instrument {}.{} not exists...", exchg.c_str(), code.c_str());
			return;
		}
		WTSCommodityInfo* commInfo = ct->getCommInfo();

		WTSTickData* tick = WTSTickData::create(code.c_str());
		tick->setContractInfo(ct);
		WTSTickStruct& quote = tick->getTickStruct();
		strcpy(quote.exchg, commInfo->getExchg());
		
		quote.action_date = actDate;
		quote.action_time = actTime;
		quote.trading_date = m_uTradingDate;

		quote.price = ticker.price();

		quote.total_turnover = 0.0;
		quote.total_volume = 0.0;
		
		quote.turn_over = ticker.turnover();
		quote.volume = quote.turn_over / quote.price;

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
}

void ParserFUTU::subscribe(const CodeSet &vecSymbols)
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

void ParserFUTU::unsubscribe(const CodeSet &vecSymbols)
{
}

bool ParserFUTU::isConnected()
{
	return m_pQotApi != NULL;
}

void ParserFUTU::registerSpi(IParserSpi* listener)
{
	m_sink = listener;

	if(m_sink)
		m_pBaseDataMgr = m_sink->getBaseDataMgr();
}
