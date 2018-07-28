#include "EventSystem.hpp"

#include <iostream>
using std::cout;
using std::endl;

// #define NDEBUG
#include <cassert>

// namespace EventSystem {

EventSystem singleton;

EventSystem& GetSingleton() {
	return singleton;
}

EventSystem::EventSystem() {}
const id_t id_none = 0xffff;
const id_t id_none_mask = 0xffff;
inline id_t make_listener_id(id_t evt_pos, id_t lst_pos) {
	return (evt_pos << 16) | lst_pos;
}
inline id_t make_event_id(id_t evt_pos) {
	return (evt_pos << 16) | id_none;
}
inline id_t get_listener_pos(id_t id) {
	return id & id_none_mask;
}
inline id_t get_event_pos(id_t id) {
	return id >> 16;
}

void EventSystem::Unregister( const std::string& registered_event_name ) {
	auto it = m_str_to_event.find(registered_event_name);
	if(it != m_str_to_event.end()) {
		Unregister(it->second);
	}
}

void EventSystem::Unregister( id_t event_id ) {
	id_t pos = get_event_pos(event_id);
	if(pos >= 0 && pos < m_events.size()) {
		// m_events.erase(m_events.begin()+pos);
		m_events[pos].listeners.clear();
		m_events[pos].removed=true;
	}
}

void EventSystem::StopListening( id_t listener_id ) {
	id_t evt_id = get_event_pos(listener_id);
	id_t lst_id = get_listener_pos(listener_id);
	if(evt_id >= 0 && evt_id < m_events.size()) {
		event& e = m_events[evt_id];
		if(lst_id >= 0 && lst_id < e.listeners.size()) {
			if(e.add_or_remove) {
				std::array<char, 20> dummy;
				e.add_or_remove(false, make_listener_id(evt_id, lst_id), &dummy);
			}
			//e.listeners.erase(e.listeners.begin()+lst_id);
			e.listeners[lst_id].removed = true;
		}
	}
}


void EventSystem::_emit(id_t id, const void* args) {
	id_t evt_id = get_event_pos(id);
	if(evt_id < m_events.size()) {
		event& e = m_events[evt_id];
		if(e.removed) return;
		id_t lst_id = get_listener_pos(id);
		if(lst_id != id_none && lst_id < e.listeners.size()) {
			listener& l = e.listeners[lst_id];
			if(!l.removed) {
				if(l.oneshot) {
					StopListening(id);
				}
				l.call(args);
			}
		} else {
			for(auto& l : e.listeners) {
				if(!l.removed) {
					if(l.oneshot) {
						l.removed = true;
						lst_id = &l-&e.listeners[0];
						StopListening(make_listener_id(evt_id, lst_id));
					}
					l.call(args);
				}
			}
		}
	}
}

id_t EventSystem::_register(const std::string& evt_name, std::function<void(bool,id_t,const void*)> f) {
	if(m_str_to_event.find(evt_name) == m_str_to_event.end()) {
		id_t pos = m_events.size();
		id_t id = make_event_id(pos);
		m_str_to_event[evt_name] = id;
		event e;
		e.add_or_remove = f;
		m_events.push_back(e);
		return id;
	} else {
		return m_str_to_event[evt_name];
	}
}



id_t EventSystem::_listen(id_t id, any_call f, void* args) {
	id_t evt_id = get_event_pos(id);
	if(evt_id < m_events.size()) {
		event &e = m_events[evt_id];
		id_t listener_id = make_listener_id(evt_id, e.listeners.size());
		e.listeners.emplace_back(f);
		if(args && e.add_or_remove) {
			e.add_or_remove(true, listener_id, args);
		}
		return listener_id;
	} else {
		return -1;
	}
}

id_t EventSystem::_listen_once(id_t id, any_call f, void* args) {
	id = _listen(id,f,args);
	id_t ev = get_event_pos(id);
	id_t lst = get_listener_pos(id);
	m_events[ev].listeners[lst].oneshot=true;
	return id;
}

// }
