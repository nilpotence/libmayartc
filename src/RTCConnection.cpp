/*
 * RTCConnection.cpp
 *
 *  Created on: 5 août 2014
 *      Author: sylvain
 */

#include "RTCConnection.hpp"
#include "RTCPeer.hpp"

#include "talk/app/webrtc/jsep.h"
#include "talk/app/webrtc/peerconnectioninterface.h"

#include "RTCChannel.hpp"



namespace maya{

RTCConnection::RTCConnection(RTCPeer *peer, webrtc::PeerConnectionFactoryInterface * peerConnectionFactory, int peerID){
	this->peerID = peerID;
	this->peer = peer;

	webrtc::PeerConnectionInterface::RTCConfiguration config;
	//config.servers = getIceServers();

	this->peerConnection = peerConnectionFactory->CreatePeerConnection(config, getMediaConstraints(), NULL, NULL, this);
}

RTCConnection::~RTCConnection(){
	peerConnection->Close();
	std::cout << "RTCConnection closed !" << std::endl;
}

webrtc::PeerConnectionInterface::IceServers RTCConnection::getIceServers(){
	webrtc::PeerConnectionInterface::IceServers servers;

	webrtc::PeerConnectionInterface::IceServer googleServer;
	googleServer.password = std::string("",0);
	std::string uri = "stun:stun.l.google.com:19302";
	//std::string uri = "stun:stun.services.mozilla.com/";
	googleServer.uri = uri;
	googleServer.username = std::string("", 0);

	servers.push_back(googleServer);

	return servers;
}
SimpleConstraints * RTCConnection::getMediaConstraints(){
	SimpleConstraints * constraints = new SimpleConstraints;

	std::string kDataChannel = "EnableDtlsSrtp";
	std::string vDataChannel = "true";
	constraints->AddMandatory(kDataChannel, vDataChannel);

	std::string kDtlsSrtpKeyAgreement = "DtlsSrtpKeyAgreement";
	std::string vDtlsSrtpKeyAgreement = "true";
	constraints->AddMandatory(kDtlsSrtpKeyAgreement, vDtlsSrtpKeyAgreement);

	constraints->AddMandatory("kEnableSctpDataChannels", "true");

	return constraints;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////PeerConnectionObserver Implementation///////////////////
///////////////////////////////////////////////////////////////////////////////

// Triggered when a remote peer opens a data channel.
void RTCConnection::OnDataChannel(webrtc::DataChannelInterface* data_channel){
//	data_channel->Close(); //Remote opening of data channels not allowed, fuckers !
	std::cout << data_channel->label() << std::endl;
}

// New Ice candidate have been found.
void RTCConnection::OnIceCandidate(const webrtc::IceCandidateInterface* candidate){

	std::cout << "Local ICE Candidate" <<std::endl;

	std::string sdp_mid = candidate->sdp_mid();
	int sdp_mlineindex = candidate->sdp_mline_index();
	std::string sdp;
	if (!candidate->ToString(&sdp)) {
		//Cannot serialize candidate
		return;
	}

	peer->getSignalingChannel()->sendLocalICECandidate(peerID, sdp_mid, sdp_mlineindex, sdp);
}

void RTCConnection::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state){
	if(new_state == webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected){
		peer->deleteConnection(peerID);
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////CreateSessionDescriptionObserver implementation/////////////
/////////////////////////////////////////////////////////////////////////////

void RTCConnection::OnSuccess(webrtc::SessionDescriptionInterface* desc){
	peerConnection->SetLocalDescription(DummySetSessionDescriptionObserver::Create(0), desc);

	std::string sdp;
	if(!desc->ToString(&sdp)){
		return ; //Cannot serialize SDP
	}

	peer->getSignalingChannel()->sendLocalSDP(peerID, desc->type(), sdp);
}

void RTCConnection::OnFailure(const std::string& error){
	std::cerr << "CREATE ANSWER ERROR: " << error << std::endl;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


int RTCConnection::getPeerID(){
	return peerID;
}


rtc::scoped_refptr<webrtc::DataChannelInterface> RTCConnection::createDataChannel(char *name){

	struct webrtc::DataChannelInit *init = new webrtc::DataChannelInit();

	rtc::scoped_refptr<webrtc::DataChannelInterface> ch = peerConnection->CreateDataChannel(std::string(name,strlen(name)), init);

	return ch;
}

void RTCConnection::createOffer(){
	peerConnection->CreateOffer(this, NULL);
	printf("CreateOffer\n");
}

void RTCConnection::setRemoteSessionDescription(webrtc::SessionDescriptionInterface* session_description){

	peerConnection->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(0), session_description);
}

void RTCConnection::addStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream){
	peerConnection->AddStream(stream);
}

void RTCConnection::addICECandidate(webrtc::IceCandidateInterface *candidate){

	if (!peerConnection->AddIceCandidate(candidate)) {
		std::cout << "[SIG] cannot use new candidate" << std::endl;
	}
}

// implements the MessageHandler interface
void RTCConnection::OnMessage(rtc::Message* msg){
	std::cout << "MSG" << std::endl;
}


}
