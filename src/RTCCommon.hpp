/*
 * RTCCommon.hpp
 *
 *  Created on: 5 août 2014
 *      Author: sylvain
 */

#ifndef RTCCOMMON_HPP_
#define RTCCOMMON_HPP_

#include <iostream>

#include "talk/app/webrtc/mediaconstraintsinterface.h"

namespace maya{

/*============================================================*/
/*=======================HELPER CLASSES=======================*/
/*============================================================*/

class SimpleConstraints : public webrtc::MediaConstraintsInterface {
	public:
		SimpleConstraints() {}
		virtual ~SimpleConstraints() {}
		virtual const Constraints& GetMandatory() const {
			return mandatory_;
		}
		virtual const Constraints& GetOptional() const {
			return optional_;
		}
		template <class T> void AddMandatory(const std::string& key, const T& value) {
			mandatory_.push_back(Constraint(key, rtc::ToString<T>(value)));
		}
		template <class T> void AddOptional(const std::string& key, const T& value) {
			optional_.push_back(Constraint(key, rtc::ToString<T>(value)));
		}
	private:
		Constraints mandatory_;
		Constraints optional_;
};

class DummySetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver {
	public:
		static DummySetSessionDescriptionObserver* Create(int verbose) {
			return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>(verbose);
		}
		virtual void OnSuccess() {if(verbose) std::cout <<"SetSessionDescription Succeeded"<<std::endl;}
		virtual void OnFailure(const std::string& error) {if(verbose) std::cout<<"SetSessionDescription Failed"<<std::endl;}

	protected:
		DummySetSessionDescriptionObserver(int verbose) : verbose(verbose){}
		~DummySetSessionDescriptionObserver() {}
		int verbose;
};

class SimpleDataChannelObserver : public webrtc::DataChannelObserver{

	public:
		SimpleDataChannelObserver(webrtc::DataChannelInterface *data_channel){
			 channel = rtc::scoped_refptr<webrtc::DataChannelInterface>(data_channel);
		}

		// The data channel state have changed.
		virtual void OnStateChange(){

			char state[15];

			switch(channel->state()){
				case webrtc::DataChannelInterface::kClosed:
					strcpy(state, "CLOSED");
					break;
				case webrtc::DataChannelInterface::kClosing:
					strcpy(state,"CLOSING");
					break;
				case webrtc::DataChannelInterface::kConnecting:
					strcpy(state,"CONNECTING");
					break;
				case webrtc::DataChannelInterface::kOpen:
					strcpy(state,"CONNECTED");
					break;
				default:
					break;
			}
			std::cout << "[CH (" << channel->label() << ")] state = " << std::string(state, strlen(state))  << std::endl;

			if(channel->state() == webrtc::DataChannelInterface::kOpen){

				channel->Send(webrtc::DataBuffer("Native Hello World !"));
			}
		}
		//  A data buffer was successfully received.
		virtual void OnMessage(const webrtc::DataBuffer& buffer){
			std::cout << "[CH (" << channel->label() << ")] message received : " << std::string(buffer.data.data(), buffer.data.length()) << std::endl;
		}

	private:
		rtc::scoped_refptr<webrtc::DataChannelInterface> channel;

};

}

#endif /* RTCCOMMON_HPP_ */
