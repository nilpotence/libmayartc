/*
 * WebRTCServer.cpp
 *
 *  Created on: 24 juil. 2014
 *      Author: sylvain
 */

#include "RTCSignaling.hpp"

namespace maya{

void RTCSignalingChannel::setPeer(RTCSignalingChannelPeer *peer){
	this->peer = peer;
}

RTCSignalingChannelPeer * RTCSignalingChannel::getPeer(){
	return this->peer;
}

void RTCSignalingChannel::onSignalingThreadStarted(){
	this->peer->onSignalingThreadStarted();
}

} //NAMESPACE MAYA

