#include <iostream>
#include "Event.hpp"
#include <thread>
#include <chrono>
#include "macros.hpp"

using namespace std;

Event eventsystem;

void my_event_checker( unsigned int checker_id, const void* data ) {
	cout << "my event checker" << endl;
	eventsystem.EmitEvent( checker_id, 2, 67 );
}


// void my_event_click_handler( const void* data ) {
	// int x,y;
	// Event::GetData( data, x, y );
EVENT2(my_event_click_handler, int, x, int, y, {

	static uint counter = 0;
	if( ++counter == 100000 ) {

		cout << "clicked at " << x << ", " << y << " : " << counter << endl;
		counter = 0;
	}
})

class EventWithClass : public EventBase {
	private:
		struct container {
			unsigned int listener_id;
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
	EventWithClass() : time_passed(0) {}
	void OnAddListener( unsigned int listener_id, const void* data ) {
		int time;
		Event::GetData( data, time );
		cout << "registered new listener: " << listener_id << " , with param " << time << endl;
		container c;
		c.listener_id = listener_id;
		c.time = time;
		c.fixed_time = time;
		addToList( c );
	}
	// void OnCheckEvent( const void* data ) {

		// unsigned int time_step;
		// Event::GetData( data, time_step );
	EVENT2(OnCheckEvent, unsigned int, time_step, {

		unsigned int send_to_listener_id = 0;
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
			EmitEvent( send_to_listener_id, time_p );
	})

	void OnRemoveListener( int listener_id ) {
		for( auto it = time_list.begin(); it != time_list.end(); it++ ) {
			if( it->listener_id == listener_id ) {
				time_list.erase( it );
				break;
			}
		}
		cout << "on remove listener ..." << endl;
	}
};

// events
void my_event( const void* data ) {
	int a; char* b;
	Event::GetData( data, a, b );

	cout << "my event called " << a << " , " << b << endl;
}

void my_event2( const void* data ) {
	cout << "my event2 called" << endl;
}

void my_event3( const void* data ) {
	cout << "my event3 called" << endl;
}

void func_class_event_test( const void* data ) {

	int time_diff;
	Event::GetData( data, time_diff );
	cout << "1s passed with error " << time_diff << endl;

}

void my_chain_event_checker( unsigned int checker_id, const void* data ) {
	cout << "chain event checker called" << endl;
}

int main() {

	unsigned int tick_event = eventsystem.RegisterEvent("tick");

	EventWithClass ewc;
	eventsystem.RegisterEvent("timer", "tick", &ewc);


	eventsystem.AddEventListener("timer", func_class_event_test, 1000);


	eventsystem.AddEventListener("timer", []( const void* data ) {
		int error;
		Event::GetData( data, error );
		cout << "500ms passed with error " << error << endl;
	}, 500);


	eventsystem.RegisterEvent("some_event", "timer", my_event_checker, 2000);
	eventsystem.RegisterEvent("some_event1", "some_event", my_event_checker);
	eventsystem.RegisterEvent("chain_to_some_event", "some_event1", my_chain_event_checker);



	eventsystem.AddEventListener("some_event1", my_event3);


	unsigned int evt = eventsystem.AddEventListener("chain_to_some_event", my_event2);
	unsigned int evt2;
	evt2 = eventsystem.AddEventListener("timer", [evt,evt2](const void* data) {
		cout << "\x1b[33munregistered some_event\x1b[0m" << endl;
		eventsystem.RemoveEventListener(evt);
		cout << "trying to unregister: " << evt2 << endl;
		eventsystem.RemoveEventListener(evt2);
		//eventsystem.UnregisterEvent( "some_event" );
	}, 10000);

	// emit base event 1 time
	unsigned int time_step = 1;
	while(true) {
		eventsystem.EmitEvent( tick_event, time_step );
		this_thread::sleep_for(chrono::milliseconds(time_step));
	}

	return 0;
}
