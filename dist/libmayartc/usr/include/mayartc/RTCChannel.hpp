/*
 * RTCChannel.hpp
 *
 *  Created on: 6 août 2014
 *      Author: sylvain
 */

#ifndef RTCCHANNEL_HPP_
#define RTCCHANNEL_HPP_

#include "talk/app/webrtc/datachannelinterface.h"


#include "RTCChannelInterface.hpp"

namespace maya{

class RTCChannel : public RTCChannelInterface, public webrtc::DataChannelObserver{

	private:
		webrtc::DataChannelInterface *channel;
		char *name;
		ReceiveCallback recv_cb;
		void * recv_cb_data;

	public:

		RTCChannel(char * name);
		virtual ~RTCChannel();

		char * getName();

		void unsetDataChannel();

		void setDataChannel(webrtc::DataChannelInterface *channel);

		// The data channel state have changed.
		virtual void OnStateChange();
		//  A data buffer was successfully received.
		virtual void OnMessage(const webrtc::DataBuffer& buffer);

		virtual bool isConnected();
		virtual void sendData(const char* buffer, int bufferSize);
		virtual void registerReceiveCallback(ReceiveCallback cb, void * userData);

		virtual void close();
};

}

#endif /* RTCCHANNEL_HPP_ */
