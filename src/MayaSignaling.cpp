/*
 * MayaSignaling.cpp
 *
 *  Created on: 1 août 2014
 *      Author: sylvain
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <iostream>
#include <thread>

#include "webrtc/base/json.h"

#include "MayaSignaling.hpp"
#include "RTCPeerInterface.hpp"

namespace maya{

// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

const char kPeerID[] = "peerid";


class MayaSignaling : public MayaSignalingInterface{
#define SIGNALING_BUFFER_SIZE 4096

	private:
		int signalingSocket;
		bool isConnected;
		std::thread signalingThread;
		bool signalingContinue;
		char signalingBuffer[SIGNALING_BUFFER_SIZE];

		pthread_mutex_t stopMutex;
		pthread_mutex_t startMutex;

		bool tryconnect(){
			int len;
			struct sockaddr_un remote;
			char str[100];

			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 1000;

			if ((signalingSocket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
				perror("socket");
				exit(1);
			}

			setsockopt(signalingSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

			remote.sun_family = AF_UNIX;
			strcpy(remote.sun_path, "/tmp/maya.sock");
			len = strlen(remote.sun_path) + sizeof(remote.sun_family);

			if (connect(signalingSocket, (struct sockaddr *)&remote, len) == -1) {
				return false;
			}


			std::cout << "[SIG] Connected" << std::endl;

			return true;
		}

	public:

		MayaSignaling():
			signalingSocket(-1),
			signalingContinue(false),
			isConnected(false),
			stopMutex(PTHREAD_MUTEX_INITIALIZER),
			startMutex(PTHREAD_MUTEX_INITIALIZER){

		}

		virtual ~MayaSignaling(){

		}

		virtual void start() {

			isConnected = tryconnect();

			signalingContinue = true;

			pthread_mutex_lock(&startMutex);

			signalingThread = std::thread([&](){
				this->run();
			});

			pthread_mutex_lock(&startMutex);
			pthread_mutex_unlock(&startMutex);
		}
		virtual void stop() {
			pthread_mutex_lock(&stopMutex);
			signalingContinue = false;
			pthread_mutex_lock(&stopMutex);
			pthread_mutex_unlock(&stopMutex);
		}

		virtual void join(){
			signalingThread.join();
		}

		void run(){
			int len;
			int buffer_pos = 0;
			char tmpbuff[SIGNALING_BUFFER_SIZE];

			setPeer(RTCPeerInterface::create(this));

			getPeer()->onSignalingThreadStarted();

			pthread_mutex_unlock(&startMutex);

			while(signalingContinue){

				getPeer()->processMessages();

				if(!isConnected){
					isConnected = tryconnect();
					sleep(1);
				}
				else if((len = recv(signalingSocket, signalingBuffer + buffer_pos, SIGNALING_BUFFER_SIZE - buffer_pos, 0)) > 0){

					bool msgComplete = false;
					buffer_pos += len;

					do{
						msgComplete = false;

						int msg_length = *(int*)signalingBuffer;


						if(buffer_pos > msg_length){
							this->processMessage(signalingBuffer+4, msg_length );

							memcpy(tmpbuff, signalingBuffer+4+msg_length, SIGNALING_BUFFER_SIZE - 4 - msg_length);
							memcpy(signalingBuffer, tmpbuff, SIGNALING_BUFFER_SIZE - 4 - msg_length);

							buffer_pos -= (4 + msg_length);

							msgComplete = true;
						}
					}while(msgComplete && buffer_pos > 0);


				}else if(len == 0){
					std::cout << "[SIG] Disconnected" << std::endl;
					isConnected = false;
					close(signalingSocket);
					getPeer()->disconnect();
				}
			}

			close(signalingSocket);
			getPeer()->onSignalingThreadStopped();
			std::cout << "PRE" << std::endl;
			delete getPeer();
			std::cout << "POST" << std::endl;
			pthread_mutex_unlock(&stopMutex);

			std::cout << "[SIG] Quit" << std::endl;

		}

		void processMessage(const char * buffer, int length){
			std::string message = std::string(buffer, length);

			Json::Reader reader;
			Json::Value jmessage;
			if (!reader.parse(message, jmessage)) {
				std::cerr << "[SIG] error : cannot parse JSON message" << std::endl;
				return;
			}


			std::string func;
			GetStringFromJsonObject(jmessage,"func", &func);

			if(func.empty()){
				return ; //no command specified, ignore message
			}

			int peerid = -1;
			//Get Client ID
			GetIntFromJsonObject(jmessage, "peerId", &peerid);
 
			//If client ID not defined, abort
			if(peerid == -1) return ; 

			if(func.compare("ListChannels") == 0){
				processListChannels(peerid, jmessage);
			}else if(func.compare("Connect") == 0){
				processConnect(peerid, jmessage);
			}else if(func.compare("ICECandidate") == 0){
				processICECandidate(peerid, jmessage);
			}else if(func.compare("Answer") == 0){
				processAnswer(peerid, jmessage);
			}
		}

		void processListChannels(int peerid, Json::Value message){
			
			Json::StyledWriter writer;

			//Extract this peer's channel names
			std::vector<std::string> channels = getPeer()->getChannelNames();

			//Serialize to JSON
			message["channels"] = StringVectorToJsonArray(channels);
			
			std::string msg = writer.write(message);

			sendMessage(peerid, msg.c_str(), msg.length());
		}

		void processICECandidate(int peerid, Json::Value message){
			std::string sdp_mid;
			int sdp_mlineindex;
			std::string sdp;

			Json::Value candidate;

			GetValueFromJsonObject(message, "candidate", &candidate);

			GetStringFromJsonObject(candidate, kCandidateSdpMidName, &sdp_mid);
			GetIntFromJsonObject(candidate, kCandidateSdpMlineIndexName, &sdp_mlineindex);
			GetStringFromJsonObject(candidate, kCandidateSdpName, &sdp);


			getPeer()->onRemoteICECandidate(peerid, sdp_mid, sdp_mlineindex, sdp);
		}

		void processConnect(int peerid, Json::Value message){

			std::vector<std::string> channels;
			JsonArrayToStringVector(message["channels"], &channels);

			getPeer()->onConnectionRequest(peerid, channels);
		}

		void processAnswer(int peerid, Json::Value message){

			std::string type;
			std::string sdp;

			GetStringFromJsonObject(message, "type", &type);
			GetStringFromJsonObject(message, "sdp", &sdp);

			getPeer()->onRemoteSDP(peerid, type, sdp);

		}

		virtual void sendLocalSDP(int peerid, std::string type, std::string sdp){
		
			Json::StyledWriter writer;
			Json::Value message;

			message[kSessionDescriptionTypeName] = type;
			message[kSessionDescriptionSdpName] = sdp;
			message["func"] = "RemoteOffer";
			message["peerId"] = peerid;

			std::string msg = writer.write(message);
			sendMessage(-1,msg.c_str(), msg.length());
		}

		virtual void sendLocalICECandidate(int peerid, std::string sdp_mid, int sdp_mlineindex, std::string sdp){
			Json::StyledWriter writer;
			Json::Value message;
			Json::Value candidate;

			message["func"] = "RemoteICECandidate";
			message["peerId"] = peerid;

			candidate[kCandidateSdpMidName] =sdp_mid;
			candidate[kCandidateSdpMlineIndexName] = sdp_mlineindex;
			candidate[kCandidateSdpName] = sdp;

			message["candidate"] = candidate;

			std::string msg = writer.write(message);

			sendMessage(peerid,msg.c_str(), msg.length());
		}

		//Deprecated
		virtual void sendChannelList(int peerid, std::string name){
			Json::StyledWriter writer;
			Json::Value jmessage;

			jmessage["command"] = "neuronInfo";
			jmessage["name"] = name;

			std::string msg = writer.write(jmessage);

			sendMessage(peerid, msg.c_str(), msg.length());
		}

		virtual void sendMessage(int peerid, const char * message, int length) {
			int length_buffer[1];
			length_buffer[0] = length;
			send(signalingSocket,length_buffer, sizeof(int), 0);
			send(signalingSocket,message,length,0);
		}


};

MayaSignalingInterface * MayaSignalingInterface::create(){
	return new MayaSignaling();
}


}

