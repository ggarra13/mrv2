#include "mrvNetwork/mrvMessage.h"
#include "mrvNetwork/mrvSignalingMessage.h"
#include "mrvNetwork/mrvWebRTCConnection.h"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace mrv
{

    class WebRTCManager
    {
    public:
        WebRTCManager();

        void setConfiguration(const rtc::Configuration&);
        
        std::shared_ptr<WebRTCConnection> createPeer(const std::string peerId,
                                                     bool isOfferer);
    
        void handleOffer(const std::string& peerId, const std::string& sdp);
    
        void handleAnswer(const std::string& peerId, const std::string& sdp);
    
        void addRemoteCandidate(const std::string& peerId, const rtc::Candidate& c);

        void publish(const Message&);
        
        std::function<void(const rtc::binary&)>
        onBinaryMessage;
        
        std::function<void(const std::string&)>
        onStringMessage;
        
        std::function<void(const SignalingMessage&)>
        onSignalMessage;

        void erase(const std::string& peerId);
    
    protected:
        void drainPendingCandidates(const std::string& peerId);
    
        rtc::Configuration config;
        std::unordered_map<std::string, std::shared_ptr<WebRTCConnection> > clients;
        std::unordered_map<std::string, std::vector<rtc::Candidate> > pendingCandidates;

        std::mutex mtx;
    };

}
