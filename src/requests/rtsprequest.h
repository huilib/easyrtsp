

#ifndef __H_RTSP_REQUEST_H__
#define __H_RTSP_REQUEST_H__

#include <huicpp.h>
#include <hsingleton.h>
#include <hiobuffer.h>

#include "rtspprotocol.h"
#include "../rtspconnect.h"

using namespace HUICPP;


class RtspRequest {
public:
    RtspRequest(RtspRequestHeader&& header) noexcept;

    virtual ~RtspRequest();

public:
    virtual void Handle(HIOBuffer& buffer) = 0;

public:
    static RtspRequest* ParseRtspRequest(HCSZ data, HN data_len);

public:
    void SetPayload(HSTR&& str) noexcept { m_strPayload.assign(std::move(str)); }
    void SetConnection(RtspConnect* conn) noexcept{ m_connection = conn; }

    RtspConnect* GetConnection() noexcept { return m_connection; }
    const RtspConnect* GetConnection() const noexcept { return m_connection; }

    const RtspRequestHeader& GetHeader() const noexcept { return m_header; }
    HCSTRR GetPayload() const noexcept { return m_strPayload; }    

private:
    RtspRequestHeader m_header;
    HSTR m_strPayload;

    RtspConnect* m_connection;
};


/*class NotFoundRequest : public RtspRequest{
private:
    using base_class_t = RtspRequest;

public:


};*/


class RequestMng{
public:
    typedef RtspRequest* (*req_maker)(RtspRequestHeader&&);
    using req_map_t = std::map<RTSP_METHOD, req_maker>;

public:
    RequestMng() noexcept = default;
    ~RequestMng() noexcept = default;

public:
    RtspRequest* MakeRequest(RtspRequestHeader&& header);

    void RegisteMaker(RTSP_METHOD method, req_maker);

private:
    req_map_t m_req_map;
};

using req_map_ton = HSingleton<RequestMng>;


#define REGISTE_REQ_API(cmd,reqobj)                                                                   \
	static RtspRequest* class_req_##reqobj (RtspRequestHeader&& header) {                             \
		return new reqobj(std::move(header));                                                         \
	}                                                                                                 \
	class __REQCreate_class_req__##reqobj {                                                           \
	public:                                                                                           \
		__REQCreate_class_req__##reqobj () {                                                          \
			req_map_ton::Instance()->RegisteMaker(cmd,class_req_##reqobj);                           \
		}                                                                                             \
	};                                                                                                \
	static const __REQCreate_class_req__##reqobj __creator_class_req__## reqobj ## _maker



#endif // __H_RTSP_REQUEST_H__

