/*
 * cwrapper.c
 *
 *  Created on: 7 août 2014
 *      Author: sylvain
 */


#include "RTCPeerInterface.hpp"
#include "MayaSignaling.hpp"

#include "cwrapper.h"

using namespace maya;

void create(){
	initRTC();

	RTCPeerInterface * peer = RTCPeerInterface::create(MayaSignalingInterface::create());
	peer->registerChannel("ch_send", 1);
	peer->join();
}
