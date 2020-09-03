#include "Event.hpp"

#include <iostream>
using std::cout;
using std::endl;

// #define NDEBUG
#include <cassert>
#include <array>

namespace EventSystem {

Event singleton;

Event& GetSingleton() {
	return singleton;
}

Event::Event() {}
static const id_type id_none = 0xffff;
static const id_type id_none_mask = 0xffff;
static inline id_type make_listener_id(id_type evt_pos, id_type lst_pos) {
	return (evt_pos << 16) | lst_pos;
}
static inline id_type make_event_id(id_type evt_pos) {
	return (evt_pos << 16) | id_none;
}
static inline id_type get_listener_pos(id_type id) {
	return id & id_none_mask;
}
static inline id_type get_event_pos(id_type id) {
	return id >> 16;
}

/*
void Event::Unregister( const std::string& registered_event_name ) {
	auto it = m_str_to_event.find(registered_event_name);
	if(it != m_str_to_event.end()) {
		Unregister(it->second);
	}
}

void Event::Unregister( id_type event_id ) {
	id_type pos = get_event_pos(event_id);
	if(pos >= 0 && pos < m_events.size()) {
		m_events.erase(m_events.begin()+pos);
	}
}
*/

void Event::StopListening( id_type listener_id ) {
	auto it = m_listener_to_idx.find(listener_id);
	
	if(it != m_listener_to_idx.end()) {
		event& e = *it->second.first;
		
		if(e.add_or_remove) {
			std::array<char, 20> dummy;
			e.add_or_remove(false, listener_id, &dummy);
		}
		
		int idx = it->second.second;
		
		if(e.listeners[idx].listen_once) {
			e.has_listen_once--;
		}
		
		// if not last element in vector
		if(idx != e.listeners.size()-1) {
		
			int id2 = e.listeners.back().id;
			
			// last element fills hole
			e.listeners[idx] = e.listeners.back();
			
			// move handle to point to hole
			m_listener_to_idx[id2].second = idx;
		}
		
		// remove last element from vector
		e.listeners.pop_back();
		
		// erase from map
		m_listener_to_idx.erase(it);
	}
}


void Event::_emit(id_type id, const void* args) {
	
	id_type evt_id = get_event_pos(id);
	if(evt_id < m_events.size()) {
		event& e = m_events[evt_id];
		
		// targeted to single listener or all of them
		id_type lst_id = id;
		
		if(get_listener_pos(lst_id) != id_none) {
			auto it = m_listener_to_idx.find(lst_id);
			if(it == m_listener_to_idx.end()) return;
			// std::cout << "emitting: " << e.has_listen_once << " " << e.listeners.size() << "\n";
			Listener& l = e.listeners[it->second.second];
			l.func(args);
			if(l.listen_once) {
				
				if(e.add_or_remove) {
					std::array<char, 20> dummy;
					e.add_or_remove(false, l.id, &dummy);
				}
				
				// std::cout << "removing listen once: " << e.has_listen_once << " " << e.listeners.size() << "\n";
				int idx = it->second.second;
				if(idx != e.listeners.size()-1) {
					// last element
					int id2 = e.listeners.back().id;
					m_listener_to_idx[id2].second = idx;
					// remove from linear vector
					e.listeners[idx] = e.listeners.back();
				}
				
				// remove from map
				m_listener_to_idx.erase(it);
				
				e.listeners.pop_back();
				e.has_listen_once--;
				// std::cout << "removing listen once2: " << e.has_listen_once << " " << e.listeners.size() << " : " << m_listener_to_idx.size() << "\n";
			}
		} else {
			// std::cout << "emit1 " << e.listeners.size() << "\n";
			for(auto& l : e.listeners) {
				l.func(args);
			}
			if(e.has_listen_once) {
				
				int idx=0;
				// int skipped = 0;
				for(auto& l : e.listeners) {
					if(l.listen_once) {
						if(e.add_or_remove) {
							std::array<char, 20> dummy;
							e.add_or_remove(false, l.id, &dummy);
						}
						m_listener_to_idx.erase(m_listener_to_idx.find(l.id));
						if(idx != e.listeners.size()-1) {
							// last element
							int id2 = e.listeners.back().id;
							m_listener_to_idx[id2].second = idx;
							
							// remove from linear vector
							e.listeners[idx] = e.listeners.back();
						}
					}
					idx++;
				}
				
				// std::cout << "removing listen once-1: " << e.has_listen_once << " " << e.listeners.size() << "\n";
				e.listeners.erase(e.listeners.end()-e.has_listen_once, e.listeners.end());
				e.has_listen_once = 0;
				// std::cout << "removing listen once-2: " << e.has_listen_once << " " << e.listeners.size() << "\n";
			}
		}
	}
}

id_type Event::_register(const std::string& evt_name, std::function<void(bool,id_type,const void*)> f) {
	if(m_str_to_event.find(evt_name) == m_str_to_event.end()) {
		id_type pos = m_events.size();
		id_type id = make_event_id(pos);
		m_str_to_event[evt_name] = id;
		event e;
		e.has_listen_once = 0;
		e.add_or_remove = f;
		m_events.push_back(e);
		return id;
	} else {
		return m_str_to_event[evt_name];
	}
}



id_type Event::_listen(id_type id, bool once, any_call f, void* args) {
	id_type evt_id = get_event_pos(id);
	if(evt_id < m_events.size()) {
		event &e = m_events[evt_id];
		id_type listener_id = make_listener_id(evt_id, ++e.listener_free_id);
		m_listener_to_idx[listener_id] = {&e, e.listeners.size()};
		e.listeners.push_back( {listener_id, once, f} );
		if(once) {
			e.has_listen_once++;
		}
		if(args && e.add_or_remove) {
			e.add_or_remove(true, listener_id, args);
		}
		return listener_id;
	} else {
		return -1;
	}
}
}
