
#include <iostream>
#include <cstdint>
#include <chrono>
#include <unistd.h>
#include <fstream>

#include "socket.h"
#include "Message.h"

using namespace std;

class Tickable {
	uint64_t last_tick_time = 0;
	public:
		virtual void tick(const small_world::SM_Event & event) {
			last_tick_time = event.event_time();
		}
};

class RobotState {
	map<string, shared_ptr<RobotState>> next_states;
	string state_name;

	public:
		string get_state_name() { return state_name; }
		void set_name(const string & s) { state_name = s;}

		void set_next_state(const string & state_name, shared_ptr<RobotState> state) {
			next_states[state_name] = state;
		}

		shared_ptr<RobotState> get_next_state(const string & transition_name) {
			map<string, shared_ptr<RobotState>>::iterator it = next_states.find(transition_name);
			if (it == next_states.end()) return nullptr;
				return it->second;
		}

		virtual shared_ptr<RobotState> decide_action(uint64_t elapsed) = 0;
};

class StateMachine : public Tickable {
	shared_ptr<RobotState> current_state;
	// The state machine checks how much time passed since previous tick
	uint64_t initial_time = 0; uint64_t current_time = 0; uint64_t prev = 0;

	// use these strings to indicate the state transitions
	// the robot progresses through.  Do not modify these strings
	string robot_finished_waiting = "The robot finished waiting";
	string robot_finished_moving = "The robot finished moving";

	string robot_began_waiting = "The robot began waiting";
	string robot_begin_moving = "The robot begain moving";
	
	public:
		uint64_t get_elapsed() {
			return current_time - initial_time;
		}

		virtual void tick(const small_world::SM_Event & e) {
			Tickable::tick(e);
			// If current state exists
			if (current_state != nullptr) {
				prev = current_time;
				// Resest time at the beginning
				if (initial_time == 0) {
					cout << robot_began_waiting << endl;
					initial_time = e.event_time();
					current_time = e.event_time();
				}
				if (prev == current_time) {
					// Figure out if should switch to the next state
					shared_ptr<RobotState> next = current_state->decide_action(get_elapsed());
					current_time = e.event_time();
					if (next != nullptr) {
						// If the state changed, resest the initial time
						initial_time = e.event_time();
						// Prints messages based on current time
						if (current_state->get_state_name() == "moving state") {
							cout << robot_finished_moving << endl;
							cout << "Robot has moved to state: " << next->get_state_name() << endl;
							cout << robot_began_waiting << endl;
						} else {
							cout << robot_finished_waiting << endl;
							cout << "Robot has moved to state: " << next->get_state_name() << endl;
							cout << robot_begin_moving << endl;
						}
						// Moves to the next state
						set_current_state(next);
					}
				}
			}
		}
		virtual void set_current_state(shared_ptr<RobotState> cs) {
			current_state = cs;
		}
};


class TimedState : public RobotState {
	string verb_name;
	uint64_t time_to_wait = 2000;
	shared_ptr<StateMachine> owner;
	
	// Given string messages
	string robot_waiting = "The robot is waiting";
	string robot_moving = "The robot is moving";

	public:
		void set_time_to_wait (uint64_t t) { time_to_wait = t; }
		void set_verb(const string & v) { verb_name = v;}
		void set_owner(shared_ptr<StateMachine> o) { owner = o; }
		
		virtual shared_ptr<RobotState> decide_action(uint64_t duration) {
			// If it is still doing the same action
			if (duration < time_to_wait) {
				if (verb_name == "waiting") {
					cout << robot_waiting << endl;
				} else {
					cout << robot_moving << endl;
				}
				return nullptr;
			}
			shared_ptr<RobotState> next = get_next_state("done");
			if (next == nullptr) {
				cout << "Can't get a next state to go to" << endl;
				return nullptr;
			}
			// Needs to switch to new state
			return next;
		}
};


int main(int argc, char * argv[]) {
	// Creates the state machine
	shared_ptr<StateMachine> sm = make_shared<StateMachine>();
	shared_ptr<TimedState> s0 = make_shared<TimedState>();
	s0->set_name("wait state");
	s0->set_verb("waiting");
	// s0->set_owner(sm);
	shared_ptr<TimedState> s1 = make_shared<TimedState>();
	s1->set_name("moving state");
	s1->set_verb("moving");
	// s1->set_owner(sm);
	s0->set_next_state("done", s1);
	s1->set_next_state("done", s0);
	sm->set_current_state(s0);
	// Recieve messages + creates a server
		// Create a socket
		// Set the socket options
		// Bind the socket
		// Listen for connections
	in_port_t port = 8888;
	server_socket listener (port);
	accepted_socket client;
		// Aceept connections
		// infinite loop for accept
	while (true) {
		listener.accept (client);
		cout << "client accepted connection: " << to_string (client) << endl;
		// Read and Deserilze messages
		char buf[1024];
		int n = 1;
		while ((n = client.recv(buf, 1024)) > 0) {
			string s (buf, n); 
			small_world::SM_Event e;
			e.ParseFromString(s);
			sm->tick(e);
		} 
		cout << "client disconnected" << endl;
		client.close();
	}
	listener.close();
	
	return EXIT_SUCCESS;
}

