
#include <glog/logging.h>
#include <iconv.h>
#include "Process.h"
#include "../src/Code.h"
#include "../src/Command.hpp"
#include "../src/Worker.h"

#include "Heartbeat.hpp"
#include "Test.hpp"


namespace bus {
Process::Process():_handler(0), _worker(0) {}
Process::~Process() {}

void Process::deal_done()
{
#define PROC(POT) {\
	_protocol.init((char*)_handler->write_buffer().data(), _handler->write_buffer().space()); \
	_protocol << (Json*)_bus->getAck(); \
	if (_protocol.stat != Protocol::good) {\
		VLOG(200) << "encode err "; \
		Json errack; \
		errack.put("ret", 2); \
		_protocol.init((char*)_handler->write_buffer().data(), _handler->write_buffer().space()); \
		_protocol << errack; \
		}\
	}

#define CASE_PROC(CMD, POT) case CMD: {\
    	PROC(POT);\
    	_protocol << CMD##_ACK;\
    	len = _protocol.Encode();\
    	break;\
	}

	VLOG(200) << "Process::deal_done " << boost::this_thread::get_id();

	int len = -1;

	_protocol.init((char*) _handler->read_buffer().data(),
			_handler->read_buffer().space());
	VLOG(200) << "Cmd: " << _protocol.getCmd();
	switch (_protocol.getCmd())
	{
		CASE_PROC(TEST, Test);

		CASE_PROC(HEART_BEAT, Heartbeat);

		default:
		{
			VLOG(200) << "unknown protocol, CMD = " << _protocol.getCmd();
			break;
		}
    } // end switch

    if (len > 0) {
        VLOG(200) << "Process::deal_done OK ACK len: " << len;
        if (CONF.get<bool>("log.hex_dump"))
        {//默认关闭
            HexDump((char *)_handler->write_buffer().data(), len);
        }
        _worker->ack(*_handler, len);

    }

#undef PROC
#undef CASE_PROC
}

/**
 * @todo 数据库,线程加锁问题
 */
void Process::deal(server_handler_type *handler, Worker* worker)
{
#define PROC(POT) try{\
        _bus = boost::make_shared<bus::Bus##POT>(); \
        _protocol >> (Json*)_bus->getQry(); \
        _bus->peer_addr(handler->socket().remote_endpoint().address().to_string().c_str(), \
                    handler->socket().remote_endpoint().port()); \
        _bus->cmd(_protocol.getCmd()); \
        _bus->process(this); \
    } catch(...) { \
    }
#define CASE_PROC(CMD, POT) case CMD: {\
    	PROC(POT);\
    	break;\
	}

    if (handler == 0 || worker == 0)
    {
        /// @todo 是否需要回错误包?
        return;
    }
    _handler = handler;
    _worker = worker;
    _bus.reset();

    _protocol.init((char*) _handler->read_buffer().data(), _handler->read_buffer().space());
    VLOG(200) << "协议号" << _protocol.getCmd();
    _protocol.Decode();
	switch (_protocol.getCmd())
	{
		CASE_PROC(TEST, Test);
		
		CASE_PROC(HEART_BEAT, Heartbeat);

		default:
		{
			VLOG(200) << "未知的协议";
			break;
		}
    } // end switch
#undef PROC
#undef CASE_PROC
}
}// namespace bus

char *verify_null(char *buf)
{
	static char buf1[2] = "0";
	if (buf == NULL)
		return buf1;
	else
		return buf;
}

std::string UrlEncode(const std::string& szToEncode)
{
	std::string src = szToEncode;
	char hex[] = "0123456789ABCDEF";
	string dst;

	for (size_t i = 0; i < src.size(); ++i)
	{
		unsigned char cc = src[i];
		if (isascii(cc))
		{
			if (cc == ' ')
			{
				dst += "%20";
			}
			else
				dst += cc;
		}
		else
		{
			unsigned char c = static_cast<unsigned char>(src[i]);
			dst += '%';
			dst += hex[c / 16];
			dst += hex[c % 16];
		}
	}
	return dst;
}
