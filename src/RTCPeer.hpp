#ifndef __WEBRTCPEER_HPP
#define __WEBRTCPEER_HPP

#include <iostream>
#include <thread>
#include <unordered_map>

#include "webrtc/base/scoped_ref_ptr.h"
#include "talk/app/webrtc/peerconnectioninterface.h"

#include "RTCConnection.hpp"
#include "RTCSignaling.hpp"
#include "RTCCommon.hpp"
#include "RTCChannel.hpp"

#include "RTCPeerInterface.hpp"
 
namespace maya{

class RTCConnection;

	class RTCPeer : public RTCPeerInterface{

	private:
		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peerConnectionFactory;

		std::unordered_map<int, rtc::RefCountedObject<RTCConnection>*> connections;
		std::unordered_map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> > streams;
		RTCSignalingChannel *signalingChannel;
		std::unordered_map<std::string, RTCChannel*> channels;

	public:
		RTCPeer(RTCSignalingChannel *signalingChannel);
		~RTCPeer();

		RTCSignalingChannel * getSignalingChannel();
		void createStreams();
		cricket::VideoCapturer* OpenVideoCaptureDevice();
		void deleteConnection(int peerid);
		RTCConnection * getConnection(int peerid);
		virtual void join();
		bool offerChannel(webrtc::DataChannelInterface *channel);
		virtual RTCChannelInterface* registerChannel(const char* name);


		void disconnect();

		void onStateChanged(RTCSignalingChannelState state);
		void onRemoteSDP(int peerid, std::string type, std::string sdp);
		void onRemoteICECandidate(int peerid, std::string sdp_mid, int sdp_mlineindex, std::string sdp);
		void onMessage(int peerid, const char * msg, int msglength);
		void onConnectionRequest(int peerid, std::vector<std::string> channels);

		std::vector<std::string> getChannelNames();

	};


	void initRTC();
}

#endif
