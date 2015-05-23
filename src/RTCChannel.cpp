/*
 * RTCChannel.cpp
 *
 *  Created on: 6 août 2014
 *      Author: sylvain
 */

#include <iostream>

#include "RTCChannel.hpp"

namespace maya{

RTCChannel::RTCChannel(char * name){
	this->name = name;
	this->channel = NULL;
}

RTCChannel::~RTCChannel(){
	unsetDataChannel();
}

char * RTCChannel::getName(){
	return name;
}

void RTCChannel::unsetDataChannel(){
	if(channel != NULL){
		channel->UnregisterObserver();
		channel->Release();
		channel = NULL;
	}
	std::cout << "CH " << name << " : data channel unregistered" << std::endl;
}

void RTCChannel::setDataChannel(webrtc::DataChannelInterface *channel){

	this->channel = channel;
	channel->AddRef();
	channel->RegisterObserver(this);
	std::cout << "[CH (" << this->name << ")] Connected !"<< std::endl;
}

// The data channel state have changed.
void RTCChannel::OnStateChange(){

}
//  A data buffer was successfully received.
void RTCChannel::OnMessage(const webrtc::DataBuffer& buffer){

	if(recv_cb != NULL){
		float * b = (float*) buffer.data.data();
		this->recv_cb(b, buffer.data.size() * sizeof(char) / sizeof(float), this->recv_cb_data);
	}

}


bool RTCChannel::isConnected(){
	return channel != NULL && channel->state() == webrtc::DataChannelInterface::DataState::kOpen;
}

void RTCChannel::sendData(const char* buffer, int bufferSize){
	if(!this->channel->Send(webrtc::DataBuffer(rtc::Buffer(buffer, bufferSize),true))){
		std::cerr << "cannot send buffer (size:" << bufferSize << ")" << std::endl;
	}
}

void RTCChannel::registerReceiveCallback(ReceiveCallback cb, void * userData){
	this->recv_cb = cb;
	this->recv_cb_data = userData;
}

void RTCChannel::close(){
	this->channel->Close();
}

}
