/*
 * MayaSignaling.hpp
 *
 *  Created on: 1 août 2014
 *      Author: sylvain
 */

#ifndef MAYASIGNALING_HPP_
#define MAYASIGNALING_HPP_

#include "RTCSignaling.hpp"


#define SIGNALING_BUFFER_SIZE 4096

namespace maya{

class MayaSignalingInterface : public RTCSignalingChannel{
	public:
		static MayaSignalingInterface *create();
};

}


#endif /* MAYASIGNALING_HPP_ */
