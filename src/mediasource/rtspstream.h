

#include "streambase.h"
#include "streaminfo.h"
#include "../requests/rtspprotocol.h"


class TrackEntity;
class RtspStream final : public StreamBase {
private:
    using stream_base_t = StreamBase;
      
public:
    RtspStream(HCSTRR strName);

    ~RtspStream() noexcept;   

public:
    HRET Init() override;

public:  
    HSTR GenerateSDP() const override;

    HN GetTrackRtpPort(TRACK_TYPE tt) const noexcept;

public:
    HSTR GetSupportMethods() const noexcept;

    void SetPort(HN nPort) noexcept { m_port = nPort; }

private:
    HSTR m_strFileName;
    std::vector<RTSP_METHOD> m_suport_methods;
    HN m_port;
    
};