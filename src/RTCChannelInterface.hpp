/*
 * RTCChannelInterface.hpp
 *
 *  Created on: 6 août 2014
 *      Author: sylvain
 */

#ifndef RTCCHANNELINTERFACE_HPP_
#define RTCCHANNELINTERFACE_HPP_

namespace maya{

typedef void (*ReceiveCallback)(float* buffer, int bufferSize, void * userData);

class RTCChannelInterface{

	public:

		virtual ~RTCChannelInterface() {};

		virtual bool isConnected() = 0;
		virtual void sendData(const char* buffer, int bufferSize) = 0;
		virtual void setNegociationMessage(char * buffer, int bufferSize) = 0;
		virtual void registerReceiveCallback(ReceiveCallback cb, void* userData) = 0;
		virtual void close() = 0;
};

}

#endif /* RTCCHANNELINTERFACE_HPP_ */
