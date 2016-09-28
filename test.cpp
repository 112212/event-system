#include <iostream>
#include "Event.hpp"
#include <thread>
#include <chrono>
#include <list>

using namespace std;

Event es;

// void my_event_checker( Event::id_type checker_id, const void* data ) {
	// cout << "my event checker" << endl;
	// es.EmitEvent( checker_id, 2, 67 );
// }


void my_event_click_handler( int x, int y) {

	static uint32_t counter = 0;
	if( ++counter == 100000 ) {
		cout << "clicked at " << x << ", " << y << " : " << counter << endl;
		counter = 0;
	}
}

class EventWithClass {
	private:
		Event* es;
		Event::id_type id;
		struct container {
			Event::id_type listener_id;
			int fixed_time;
			int time;
		};
		std::list<container> time_list;
		unsigned int time_passed;
		void addToList( container &c ) {
			c.time = c.fixed_time;
			c.time += time_passed;
			if( time_passed > c.time ) {
				// reset time_passed
				for( auto &e : time_list ) {
					e.time -= time_passed;
				}
				time_passed = 0;
			}

			if( time_list.size() > 0 ) {
				// find place to insert
				bool found = false;
				for( auto it = time_list.begin(); it != time_list.end(); it++ ) {
					if( it->time > c.time ) {
						time_list.insert( it, c );
						found = true;
						break;
					}
				}
				if( !found )
					time_list.push_back( c );
			} else {
				time_list.push_front( c );
			}
		}
	public:
	EventWithClass(std::string event_name, Event* evt_system) : time_passed(0) {
		es = evt_system;
		es->Register(event_name, 
			[&] (bool add, Event::id_type listener_id, int time)
			 { this->OnAddOrRemoveListener(add,listener_id,time); });
		es->Listen("tick", [&](unsigned int time_step) {
			this->OnCheckEvent(time_step);
		});
	}
	void OnAddOrRemoveListener( bool add, Event::id_type listener_id, int time ) {
		if(add) {
			cout << "registered new listener: " << listener_id << " , with param " << time << endl;
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
			cout << "on remove listener ..." << endl;
		}
	}
	// void OnCheckEvent( const void* data ) {

		// unsigned int time_step;
		// Event::GetData( data, time_step );
	void OnCheckEvent(unsigned int time_step) {

		Event::id_type send_to_listener_id = 0;
		unsigned int time_p;

		time_passed += time_step;
		if( time_passed >= time_list.front().time ) {
			container c = time_list.front();
			time_list.pop_front();
			send_to_listener_id = c.listener_id;
			time_p = time_passed - c.time;
			addToList( c );
		}

		//cout << "on check event ... sending event to: " << send_to_listener_id << endl;
		if( send_to_listener_id != 0 )
			es->Emit( send_to_listener_id, time_p );
	}
};

// events
void my_event( int a, char* b ) {
	cout << "my event called " << a << " , " << b << endl;
}

void my_event2( const void* data ) {
	cout << "my event2 called" << endl;
}

void my_event3( const void* data ) {
	cout << "my event3 called" << endl;
}

void func_class_event_test( int time_diff ) {
	cout << "1s passed with error " << time_diff << endl;
}

// void my_chain_event_checker( unsigned int checker_id, const void* data ) {
	// cout << "chain event checker called" << endl;
// }

int main() {

	Event::id_type tick_event = es.Register("tick");

	cout << "tick_event: " << tick_event << endl;
	EventWithClass ewc("timer", &es);
	// es.RegisterEvent("timer", "tick", &ewc);
	
	
	

	es.Listen("timer", func_class_event_test, 1000);


	es.Listen("timer", []( int error ) {
		cout << "500ms passed with error " << error << endl;
	}, 500);

	// es.Listen("tick", []( int ts ) {
		// cout << "TICK: " << ts << endl;
	// });
	/*
	es.RegisterEvent("some_event", "timer", my_event_checker, 2000);
	es.RegisterEvent("some_event1", "some_event", my_event_checker);
	es.RegisterEvent("chain_to_some_event", "some_event1", my_chain_event_checker);



	
	es.Listen("some_event1", my_event3);

	unsigned int evt = es.AddEventListener("chain_to_some_event", my_event2);
	unsigned int evt2;
	*/
	es.Listen("timer", []() {
		// cout << "\x1b[33munregistered some_event\x1b[0m" << endl;
		// es.RemoveEventListener(evt);
		// cout << "trying to unregister: " << evt2 << endl;
		// es.RemoveEventListener(evt2);
		//es.UnregisterEvent( "some_event" );
		cout << "LOL\n";
	}, 10000);
	

	// emit base event 1 time
	unsigned int time_step = 1;
	while(true) {
		es.Emit( tick_event, time_step );
		this_thread::sleep_for(chrono::milliseconds(time_step));
	}

	return 0;
}
