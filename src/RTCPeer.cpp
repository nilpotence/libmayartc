#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <stdexcept>

#include "webrtc/base/helpers.h"
#include "webrtc/base/json.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/ssladapter.h"
#include "webrtc/base/basictypes.h"
#include "webrtc/base/refcount.h"
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/base/stringencode.h"

#include "talk/media/devices/devicemanager.h"
#include "talk/media/base/videocapturer.h"

#include "talk/app/webrtc/jsep.h"
#include "talk/app/webrtc/mediaconstraintsinterface.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/datachannelinterface.h"
#include "talk/app/webrtc/videosourceinterface.h"

#include "RTCPeer.hpp"

namespace maya{

/*============================================================*/
/*======================CONSTANTS=============================*/
/*============================================================*/

const char kAudioLabel[] = "audio_label";
const char kVideoLabel[] = "video_label";
const char kStreamLabel[] = "stream_label";

/*============================================================*/
/*============================================================*/
/*============================================================*/




//Initialize SSL
void initRTC(){
	rtc::InitializeSSL();
}

void destroyRTC(){
	rtc::CleanupSSL();
}
 



RTCPeer::RTCPeer(RTCSignalingChannel *signalingChannel){
	this->signalingChannel = signalingChannel;

	mutex = PTHREAD_MUTEX_INITIALIZER;
	mutexConnection = PTHREAD_MUTEX_INITIALIZER;
	
	
	
	createPeerConnectionFactory();

}



RTCPeer::~RTCPeer(){
	std::cout << "deleting RTCPeer... " << std::endl;

	peerConnectionFactory = NULL;

	for(auto kv : connections){
		kv.second->Release();
	}

	connections.clear();

	for(auto kv : channels){
		delete kv.second;
	}

	channels.clear();

	std::cout << "[ OK ]" << std::endl;
}

RTCSignalingChannel * RTCPeer::getSignalingChannel(){
	return signalingChannel;
}

void RTCPeer::disconnect(){
	for(auto kv : connections){
		deleteConnection(kv.second->getPeerID());
	}

	for(auto kv : channels){
		kv.second->unsetDataChannel();
	}
}

void RTCPeer::join(){
	signalingChannel->join();
}

void RTCPeer::createPeerConnectionFactory(){

	sig_thread = rtc::ThreadManager::Instance()->WrapCurrentThread();
	worker_thread = new rtc::Thread;
	worker_thread->Start();

	peerConnectionFactory = webrtc::CreatePeerConnectionFactory(worker_thread, sig_thread, NULL, NULL, NULL);
}

void RTCPeer::createStreams(){
	rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
			peerConnectionFactory->CreateAudioTrack(kAudioLabel, peerConnectionFactory->CreateAudioSource(NULL)));

	rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track(
			peerConnectionFactory->CreateVideoTrack(kVideoLabel,peerConnectionFactory->CreateVideoSource(OpenVideoCaptureDevice(), NULL)));

	//  <---- can start local renderer here if needed

	rtc::scoped_refptr<webrtc::MediaStreamInterface> stream(peerConnectionFactory->CreateLocalMediaStream(kStreamLabel));

	stream->AddTrack(audio_track);
	stream->AddTrack(video_track);

	streams["video"] = stream;
}



cricket::VideoCapturer* RTCPeer::OpenVideoCaptureDevice(){
	rtc::scoped_ptr<cricket::DeviceManagerInterface> dev_manager(cricket::DeviceManagerFactory::Create());

	if (!dev_manager->Init()) {
		LOG(LS_ERROR) << "Can't create device manager";
		return NULL;
	}

	std::vector<cricket::Device> devs;

	if (!dev_manager->GetVideoCaptureDevices(&devs)) {
		LOG(LS_ERROR) << "Can't enumerate video devices";
		return NULL;
	}

	std::vector<cricket::Device>::iterator dev_it = devs.begin();
	cricket::VideoCapturer* capturer = NULL;

	for (; dev_it != devs.end(); ++dev_it) {
		std::cout << "Video device found" << std::endl;
		capturer = dev_manager->CreateVideoCapturer(*dev_it);

		/*if (capturer != NULL){
			cricket::VideoFormat f;
			f.Construct(480,270,200000,cricket::FOURCC_ANY);

			capturer->video_adapter()->SetOutputFormat(f);
			capturer->video_adapter()->set_cpu_adaptation(false);
			std::cout << capturer->enable_video_adapter() << std::endl;
			break;
		}*/
	}

	return capturer;
}

RTCChannelInterface* RTCPeer::registerChannel(const char* name){
	RTCChannel *channel = new RTCChannel((char*)name);

	channels[std::string(name, strlen(name))] = channel;

	return channel;
}

bool RTCPeer::offerChannel(webrtc::DataChannelInterface *channel){
	try{
		RTCChannel *ch = channels.at(channel->label());
		if(ch->isConnected()) return false;
		ch->setDataChannel(channel);
		return true;
	}catch(std::out_of_range& ex){
		return false;
	}
}

void RTCPeer::deleteConnection(int peerid){
	rtc::RefCountedObject<RTCConnection> *connection;
	
	pthread_mutex_lock(&mutexConnection);

	try{
		connection = connections.at(peerid);
		connection->Release();
		connections.erase(peerid);

	}catch(std::out_of_range& error){
	}
	pthread_mutex_unlock(&mutexConnection);
}

RTCConnection * RTCPeer::getConnection(int peerid){
	
	rtc::RefCountedObject<RTCConnection> *connection;
	
	pthread_mutex_lock(&mutexConnection);
	try{
		connection = connections.at(peerid);
	}catch(std::out_of_range& error){
		connection = new rtc::RefCountedObject<RTCConnection>(this, peerConnectionFactory, peerid);
		connection->AddRef();
		connections[peerid] = connection;
		//connection->addStream(streams["video"]);
	}
	pthread_mutex_unlock(&mutexConnection);
	return connection;
}

//////////////////////////////////////////////////////////////////////
/////////////SignalingChannel Observer implementation/////////////////
//////////////////////////////////////////////////////////////////////

void RTCPeer::onSignalingThreadStarted(){
}

void RTCPeer::onSignalingThreadStopped(){
}

void RTCPeer::processMessages(){
	sig_thread->ProcessMessages(10);
	worker_thread->ProcessMessages(10);
}

void RTCPeer::onStateChanged(RTCSignalingChannelState state){

}

void RTCPeer::onRemoteSDP(int peerid, std::string type, std::string sdp){

	RTCConnection* connection = getConnection(peerid);

	webrtc::SessionDescriptionInterface* session_description = webrtc::CreateSessionDescription(type, sdp, NULL);
	if (!session_description) {
		std::cerr << "[SIG] error : cannot parse SDP string" << std::endl;
		return;
	}

	connection->setRemoteSessionDescription(session_description);
}

std::vector<std::string> RTCPeer::getChannelNames(){

	std::vector<std::string> ret;

	for(auto kv : this->channels){
		ret.push_back(kv.first);
	}
	
	return ret;
}

void RTCPeer::onConnectionRequest(int peerid, std::vector<std::string> channelnames) {

	std::vector<RTCChannel*> requestedChannels;

	for(int i=0;i<channelnames.size();i++){
		try{
			RTCChannel * ch = this->channels.at(channelnames[i]);
			//TODO : check is the channel is already connected or not
			requestedChannels.push_back(ch);
		}catch(std::out_of_range &error){
			std::cerr << "no channel found for \"" << channelnames[i] <<"\"" << std::endl;	
		}
	}
	
	//If no channel is requested or no valid channel names are provided, abort connection attempt
	if(requestedChannels.size() <= 0){
		printf("connect : no match !\n");
		return ;
	}

	//Create new Connection
	RTCConnection *connection = getConnection(peerid);

	for(int i=0; i<requestedChannels.size(); i++){

		rtc::scoped_refptr<webrtc::DataChannelInterface> wch = connection->createDataChannel(requestedChannels[i]->getName());
		requestedChannels[i]->setDataChannel(wch);
	}

	connection->createOffer();
}

void RTCPeer::onRemoteICECandidate(int peerid, std::string sdp_mid, int sdp_mlineindex, std::string sdp){

	RTCConnection *connection = getConnection(peerid);

	//Create the received candidate
	rtc::scoped_ptr<webrtc::IceCandidateInterface> candidate(webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp));

	if (!candidate.get()) {
		std::cout << "[SIG] cannot parse candidate information" << std::endl;
		return;
	}

	connection->addICECandidate(candidate.get());
}

void RTCPeer::onMessage(int peerid, const char * msg, int msglength){	}



//Factory for RTCPeer
RTCPeerInterface* RTCPeerInterface::create(RTCSignalingChannel *signalingChannel){
	RTCPeer *p = new RTCPeer(signalingChannel);
	return p;
}


} //NAMESPACE MAYA



