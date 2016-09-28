#include "Event.hpp"

#include <iostream>
using std::cout;
using std::endl;

// #define NDEBUG
#include <cassert>

namespace Event {

Event singleton;

Event& GetSingleton() {
	return singleton;
}

Event::Event() {}
const id_type id_none = 0xffff;
const id_type id_none_mask = 0xffff;
inline id_type make_listener_id(id_type evt_pos, id_type lst_pos) {
	return (evt_pos << 16) | lst_pos;
}
inline id_type make_event_id(id_type evt_pos) {
	return (evt_pos << 16) | id_none;
}
inline id_type get_listener_pos(id_type id) {
	return id & id_none_mask;
}
inline id_type get_event_pos(id_type id) {
	return id >> 16;
}

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

void Event::StopListening( id_type listener_id ) {
	id_type evt_id = get_event_pos(listener_id);
	id_type lst_id = get_listener_pos(listener_id);
	if(evt_id >= 0 && evt_id < m_events.size()) {
		event& e = m_events[evt_id];
		if(lst_id >= 0 && lst_id < e.listeners.size()) {
			if(e.add_or_remove) {
				std::array<char, 20> dummy;
				e.add_or_remove(false, make_listener_id(evt_id, lst_id), &dummy);
			}
			e.listeners.erase(e.listeners.begin()+lst_id);
		}
	}
}


void Event::_emit(id_type id, const void* args) {
	id_type evt_id = get_event_pos(id);
	if(evt_id < m_events.size()) {
		event& e = m_events[evt_id];
		id_type lst_id = get_listener_pos(id);
		if(lst_id != id_none && lst_id >= 0 && lst_id < e.listeners.size()) {
			e.listeners[lst_id](args);
		} else {
			for(auto& l : e.listeners) {
				l(args);
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
		e.add_or_remove = f;
		m_events.push_back(e);
		return id;
	} else {
		return m_str_to_event[evt_name];
	}
}



id_type Event::_listen(id_type id, any_call f, void* args) {
	id_type evt_id = get_event_pos(id);
	if(evt_id >= 0 && evt_id < m_events.size()) {
		event &e = m_events[evt_id];
		id_type listener_id = make_listener_id(evt_id, e.listeners.size());
		e.listeners.push_back(f);
		if(args && e.add_or_remove) {
			e.add_or_remove(true, listener_id, args);
		}
		return listener_id;
	} else {
		return -1;
	}
}
}
