/*
 * RTCConnection.hpp
 *
 *  Created on: 5 août 2014
 *      Author: sylvain
 */

#ifndef RTCCONNECTION_HPP_
#define RTCCONNECTION_HPP_


#include "talk/app/webrtc/peerconnectioninterface.h"

#include "RTCCommon.hpp"

namespace maya{

class RTCPeer;

class RTCConnection : public webrtc::PeerConnectionObserver, webrtc::CreateSessionDescriptionObserver, public rtc::MessageHandler{

	private :
		int peerID;
		rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection;
		RTCPeer *peer;


	public :
		RTCConnection(RTCPeer *peer, webrtc::PeerConnectionFactoryInterface * peerConnectionFactory,  int peerID);
		virtual ~RTCConnection();


		///////////////////////////////////////////////////////////////////////////////
		///////////////////////PeerConnectionObserver Implementation///////////////////
		///////////////////////////////////////////////////////////////////////////////
		virtual void OnError(){	std::cerr << "An Error occured !" << std::endl;}
		// Triggered when the SignalingState changed.
		virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {}
		// Triggered when SignalingState or IceState have changed.
		// TODO(bemasc): Remove once callers transition to OnSignalingChange.
		virtual void OnStateChange(StateType state_changed) {}
		// Triggered when media is received on a new stream from remote peer.
		virtual void OnAddStream(webrtc::MediaStreamInterface* stream) {}
		// Triggered when a remote peer close a stream.
		virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream){}
		// Triggered when renegotiation is needed, for example the ICE has restarted.
		virtual void OnRenegotiationNeeded(){}
		// Called any time the IceConnectionState changes
		virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state);
		// Called any time the IceGatheringState changes
		virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {}
		// TODO(bemasc): Remove this once callers transition to OnIceGatheringChange.
		// All Ice candidates have been found.
		virtual void OnIceComplete() {}


		// Triggered when a remote peer opens a data channel.
		// TODO(perkj): Make pure virtual.
		virtual void OnDataChannel(webrtc::DataChannelInterface* data_channel);

		// New Ice candidate have been found.
		virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);
		/////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////////////////////
		/////////////////CreateSessionDescriptionObserver implementation/////////////
		/////////////////////////////////////////////////////////////////////////////

		virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);

		virtual void OnFailure(const std::string& error);
		/////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////
		
		void OnMessage(rtc::Message * msg);

		void close();

		int getPeerID();

		rtc::scoped_refptr<webrtc::DataChannelInterface> createDataChannel(char *name);
		void createOffer();

		void setRemoteSessionDescription(webrtc::SessionDescriptionInterface* session_description);
		void addStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);
		void addICECandidate(webrtc::IceCandidateInterface *candidate);
		webrtc::PeerConnectionInterface::IceServers getIceServers();
		SimpleConstraints * getMediaConstraints();

};

}

#endif /* RTCCONNECTION_HPP_ */
