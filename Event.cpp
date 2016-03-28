#include "Event.hpp"

// #define NDEBUG
#include <cassert>

//#include <iostream>

Event::Event() : event_id_tracker(1), allocator(512 * 64) {}


void Event::emitEvent( unsigned int registered_event_id, const void* data ) {
	//auto &c = registered_checkers[ registered_event_id ];
	
	// notify listeners
	auto &checker_list = checker_listeners[ registered_event_id ];
	for( auto &i : checker_list ) {
		i.func(data);
	}
	
	// notify attached checkers
	auto &ac_list = attached_checkers[ registered_event_id ];
	for( auto &attached : ac_list ) {
		
		if( attached.type == container_type::type_class ) {
			attached.evt->OnCheckEvent( data );
		} else {
			attached.func( attached.event_id, data );
		}
	}
	
}


void Event::emitEventDirect( unsigned int event_id, const void* data ) {
	direct_event_container &c = listener_contact[ event_id ];
	switch( c.type ) {
		case container_type::type_class:
			c.evt->OnCheckEvent(data);
			break;
		case container_type::type_function:
			c.checker_func(c.checker_id, data);
			break;
		case container_type::type_event_handler:
			c.func(data);
			break;
	}
}


unsigned int Event::RegisterEvent(std::string str) {
	auto h = hash(str);
	auto r = registered_checkers.find( h );
	if( r != registered_checkers.end() ) {
		return 0;
	}
	container c( container_type::type_empty );
	c.event_id = h;
	c.attached_to = 0;
	registered_checkers[ h ] = c;
	return h;
}

unsigned int Event::registerEvent(std::string str, std::string attach, std::function<void(unsigned int, const void*)> func, const void* data) {
	auto h = hash(str);
	auto q = hash(attach);
	
	
	auto r = registered_checkers.find( h );
	if( r != registered_checkers.end() ) {
		return 0;
	}
	
	auto parent = registered_checkers.find( q );
	if( parent == registered_checkers.end() ) {
		return 0;
	}
	
	container c( container_type::type_function );
	c.func = func;
	c.event_id = h;
	c.attached_to = q;
	c.listener_event_id = 0;
	
	if( parent->second.type == container_type::type_class ) {
		c.listener_event_id = event_id_tracker;
		direct_event_container d;
		d.type = container_type::type_function;
		d.checker_func = func;
		d.checker_id = h;
		d.data = data;
		listener_contact[ event_id_tracker ] = d;
		event_id_tracker++;
	}
	
	checker_listeners[ h ] = std::list<checker_container>();
	registered_checkers[ h ] = c;
	
	return h;
}

unsigned int Event::registerEvent(std::string str, std::string attach, EventBase* evt, const void* data) {
	auto h = hash(str);
	auto q = hash(attach);
	
	auto r = registered_checkers.find( h );
	if( r != registered_checkers.end() ) {
		return 0;
	}
	auto parent = registered_checkers.find( q );
	if( parent == registered_checkers.end() ) {
		return 0;
	}
	
	container c( container_type::type_class );
	evt->event_id = h;
	evt->eventsystem = this;
	c.evt = evt;
	c.event_id = h;
	c.attached_to = q;
	c.listener_event_id = 0;
	
	if( parent->second.type == container_type::type_class ) {
		c.listener_event_id = event_id_tracker;
		direct_event_container d;
		d.type = container_type::type_class;
		d.evt = evt;
		d.data = data;
		listener_contact[ event_id_tracker ] = d;
		event_id_tracker++;
	}
	
	checker_listeners[ h ] = std::list<checker_container>();
	registered_checkers[ h ] = c;
	
	return h;
}

void Event::RemoveEventListener( unsigned int event_id ) {
	auto r = listener_to_registered_event_id.find( event_id );
	if( r == listener_to_registered_event_id.end() ) {
		return;
	}
	auto registered_event_id = r->second;
	listener_to_registered_event_id.erase( r );
	
	auto q = registered_checkers.find( registered_event_id );
	assert( q != registered_checkers.end() );
	if( q->second.type == container_type::type_class ) {
		EventBase* cls = q->second.evt ;
		cls->OnRemoveListener( event_id );
	}
	
	checker_container ck( event_id );
	checker_listeners[ registered_event_id ].remove( ck );
	
	// no more listeners, remove from attached checker list
	auto cur = registered_event_id;
	while ( checker_listeners[ cur ].size() == 0 ) {
			
		auto &cur_checker = registered_checkers[ cur ];
		unsigned int &attached_to = cur_checker.attached_to;
		
		if( attached_to != 0 ) {
			auto &lst = attached_checkers[ attached_to ];
			for( auto it = lst.begin(), end = lst.end(); it != end; it++ ) {
				if( it->event_id == cur_checker.event_id ) {
					lst.erase( it );
					break;
				}
			}
			
			auto &parent = registered_checkers[ cur_checker.attached_to ];
			if( parent.type == container_type::type_class ) {
				parent.evt->OnRemoveListener( cur_checker.listener_event_id );
			}
			cur = attached_to;
		} else {
			break;
		}
	}
	
	listener_contact.erase( event_id );
}

bool operator==(const Event::checker_container &a, const Event::checker_container &b) {
	return a.event_id == b.event_id;
}

unsigned int Event::hash( std::string& str ) {
	return (unsigned int)std::hash<std::string>()(str);
}


unsigned int Event::addEventListener(std::string& str, std::function<void(const void*)> func, const void* data) {
	auto h = hash(str);
	auto r = registered_checkers.find( h );
	if( r == registered_checkers.end() ) {
		return 0;
	}
	if( r->second.type == container_type::type_class ) {
		EventBase* cls = r->second.evt;
		cls->OnAddListener( event_id_tracker, data );
	}
	
	direct_event_container d;
	d.type = container_type::type_event_handler;
	d.func = func;
	listener_contact[ event_id_tracker ] = d;
	
	checker_container ck(event_id_tracker);
	ck.func = func;
	checker_listeners[ h ].push_back( ck );
	
	listener_to_registered_event_id[ event_id_tracker ] = h;
	
	
	if( checker_listeners[ h ].size() == 1 && r->second.attached_to != 0) {
		auto cur = h;
		bool done = false;
		while( !done && cur != 0 ) {
			
			auto &cur_checker = registered_checkers[ cur ];
			unsigned int attached_to = cur_checker.attached_to;
			
			if( attached_to != 0 ) {
				bool found = false;
				auto q = attached_checkers.find(attached_to);
				if( q == attached_checkers.end() ) {
					container &c = registered_checkers[ cur ];
					attached_checkers[ attached_to ] = std::list<container>();
					attached_checkers[ attached_to ].push_back( c );
				} else {
					done = true;
					auto &lst = attached_checkers[ attached_to ];
					for( auto &e : lst ) {
						if( e.event_id == cur ) {
							found = true;
							break;
						}
					}
					if( !found ) {
						lst.push_back( registered_checkers[ cur ] );
					}
				}
				if(!found) {
					auto &parent = registered_checkers[ attached_to ];
					if( parent.type == container_type::type_class ) {
						assert( cur_checker.listener_event_id != 0 );
						parent.evt->OnAddListener( cur_checker.listener_event_id, 
								listener_contact[ cur_checker.listener_event_id ].data );
					}
				}
				cur = attached_to;
			} else {
				done = true;
			}
		}
	}
	
	return event_id_tracker++;
}

void Event::UnregisterEvent( std::string registered_event_name ) {
	UnregisterEvent( hash(registered_event_name) );
}
void Event::UnregisterEvent( unsigned int registered_event_id ) {
	// unregister event, and all childs of this event
	auto r = registered_checkers.find( registered_event_id );
	if( r == registered_checkers.end() ) {
		return;
	}
	container &c = r->second;
	
	if( c.attached_to != 0 ) {
		unsigned int attached_to = c.attached_to;
		auto &parent = registered_checkers[ attached_to ];
		if( parent.type == container_type::type_class ) {
			assert( c.listener_event_id != 0 );
			allocator.free( (block64*)( listener_contact[ c.listener_event_id ].data ) );
			listener_contact.erase( c.listener_event_id );
			parent.evt->OnRemoveListener( c.listener_event_id );
		}
	}
	
	auto &lst = attached_checkers[ registered_event_id ];
	for( auto &i : lst ) {
		UnregisterEvent( i.event_id );
	}
	
	attached_checkers.erase( registered_event_id );
	registered_checkers.erase( registered_event_id );
}

