/*
 * RTCPeerInterface.hpp
 *
 *  Created on: 6 août 2014
 *      Author: sylvain
 */

#ifndef RTCPEERINTERFACE_HPP_
#define RTCPEERINTERFACE_HPP_

#include "RTCSignaling.hpp"
#include "RTCChannelInterface.hpp"

namespace maya{

	class RTCPeerInterface : public RTCSignalingChannelPeer{

		public:
			virtual ~RTCPeerInterface(){}

			static RTCPeerInterface * create(RTCSignalingChannel *signalingChannel);

			virtual RTCChannelInterface* registerChannel(const char* name) = 0;
			virtual void join() = 0;
	};


	void initRTC();
	void destroyRTC();
}

#endif /* RTCPEERINTERFACE_HPP_ */
