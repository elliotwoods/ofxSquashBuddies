#include "Sender.h"

namespace ofxSquashBuddies {
	//----------
	template<typename Type>
	Sender<Type>::~Sender() {
		this->close();
	}

	//----------
	template<typename Type>
	void Sender<Type>::init(string address, int port) {
		this->close();
		this->socket = make_shared<ofxAsio::UDP::Socket>(address, port);
		this->sendThread = thread(this->threadedFunction);
	}

	//----------
	template<typename Type>
	void Sender<Type>::close() {
		this->threadRunning = false;
		if (this->sendThread) {
			this->sendThread.join();
		}
	}

	//----------
	template<typename Type>
	void Sender<Type>::send(Type & data) {

	}

	//----------
	template<typename Type>
	void Sender<Type>::threadedFunction() {
		while (this->threadRunning) {
			* wait until send is called
			* pull apart the 

		}
	}
}