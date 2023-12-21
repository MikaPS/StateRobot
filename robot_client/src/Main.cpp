
#include <iostream>
#include <fstream>

#include <chrono>
#include <cstdint>

#include <unistd.h>

#include "Message.h"
#include "socket.h"


using namespace std::chrono;

int main(int argc, char * argv[]) {
		// SOCKET STUFF
	// Create a socket
	// Figure out where to connect to
	// Make the connection
	
	in_port_t port = 8888;
	string host = localhost();
	client_socket client (host, port); // the constructor makes the connection
	// A while forever loop
	while (true) {
		// Sleeps for 5 seconds
		usleep(500000);
		// Calculates the time
		uint64_t time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		// Creates a proto message
		small_world::SM_Event sm;
		// Set the message to the current time and type to 1
		sm.set_event_time(time);
		sm.set_event_type("tick");
		std::string message;
		sm.SerializeToString(&message);
		size_t message_size = message.size();
		// Send to socket a serialized message with the time
		client.send(message.data(), message_size);	
	}
	// Terminate + cleanup
	client.close();
	return EXIT_SUCCESS;
}

