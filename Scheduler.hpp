#include <list>
#include <iostream>
#include "EventSystem.hpp"

class Scheduler {
	private:
		EventSystem* es;
		id_t id;
		struct container {
			id_t listener_id;
			uint32_t fixed_time;
			uint32_t time;
		};
		
		std::list<container> time_list;
		unsigned int time_passed;
		
		void addToList( container &c ) {
			c.time = c.fixed_time;
			c.time += time_passed;
			if( time_passed >= c.time ) {
				// reset time_passed
				for( auto &e : time_list ) {
					e.time -= time_passed;
				}
				time_passed = 0;
			}

			if( !time_list.empty() ) {
				// find place to insert
				bool found = false;
				for( auto it = time_list.begin(); it != time_list.end(); it++ ) {
					if( c.time < it->time ) {
						time_list.insert( it, c );
						found = true;
						break;
					}
				}
				if( !found ) {
					time_list.push_back( c );
				}
			} else {
				time_list.push_back( c );
			}
		}
	public:
	void printList() {
		for(auto &tl : time_list) {
			std::cout << tl.time << " ";
		}
		std::cout << "\n";
	}
	Scheduler(std::string event_name, EventSystem* evt_system) : time_passed(0) {
		es = evt_system;
		
		es->Register(event_name, [&] (bool add, id_t listener_id, int time)
			 { this->OnAddOrRemoveListener(add,listener_id,time); });
			 
		es->Listen("tick", [&](unsigned int time_step) {
			this->OnCheckEvent(time_step);
		});
	}
	void OnAddOrRemoveListener( bool add, id_t listener_id, int time ) {
		if(add) {
			// std::cout << "registered new listener: " << listener_id << " , with param " << time << "\n";
			container c;
			c.listener_id = listener_id;
			c.time = time;
			c.fixed_time = time;
			addToList( c );
		} else {
			for( auto it = time_list.begin(); it != time_list.end(); it++ ) {
			if( it->listener_id == listener_id ) {
				time_list.erase( it );
				break;
				}
			}
			// std::cout << "on remove listener ...\n";
		}
	}
	// void OnCheckEvent( const void* data ) {

		// unsigned int time_step;
		// Event::GetData( data, time_step );
	void OnCheckEvent(unsigned int time_step) {

		id_t send_to_listener_id = 0;
		unsigned int time_p;

		time_passed += time_step;
		if(!time_list.empty()) {
			while( time_passed >= time_list.front().time ) {
				container c = time_list.front();
				time_list.pop_front();
				send_to_listener_id = c.listener_id;
				time_p = time_passed - c.time;
				if( send_to_listener_id != 0 ) {
					es->Emit( send_to_listener_id, time_p );
				}
				addToList( c );
			}
		}

		//cout << "on check event ... sending event to: " << send_to_listener_id << endl;
		
	}
};
