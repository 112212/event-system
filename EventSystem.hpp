#ifndef EVENT_SYSTEM_HPP
#define EVENT_SYSTEM_HPP
#include <unordered_map>
#include <string>
#include <functional>
#include <tuple>
#include <vector>
#include <utility>
// #include "fsa.hpp"



#ifndef CALL_TUPLE_ARGS
#define CALL_TUPLE_ARGS
namespace EventSystem_internal {

// ----- template hell ----------------
template<bool done, int n, typename... Args>
struct tuple_skip_n_args;

template<int n, typename Arg, typename... Args>
struct tuple_skip_n_args<false, n, Arg, Args...> {
	using type = typename tuple_skip_n_args<n == 0 || n == 1, n-1, Args...>::type;
};

template<int n, typename... Args>
struct tuple_skip_n_args<true, n, Args...> {
	using type = std::tuple<Args...>;
};
		
template< typename t, std::size_t n, typename = void >
struct function_type_information;

template< typename r, typename ... a, std::size_t n >
struct function_type_information< r (*)( a ... ), n > {
	// using type = typename std::tuple_element< n, std::tuple< a ... > >::type; 
	// using tuple = std::tuple<a...>;
	using tuple = typename tuple_skip_n_args<n == 0, n, a...>::type;
	using result = r;
};

template< typename r, typename c, typename ... a, std::size_t n >
struct function_type_information< r (c::*)( a ... ), n >
	: function_type_information< r (*)( a ... ), n > {};

template< typename r, typename c, typename ... a, std::size_t n >
struct function_type_information< r (c::*)( a ... ) const, n >
	: function_type_information< r (*)( a ... ), n > {};

template< typename ftor, std::size_t n >
struct function_type_information< ftor, n,
	typename std::conditional< false, decltype( & ftor::operator () ), void >::type >
	: function_type_information< decltype( & ftor::operator () ), n > {};
// --------------------------------------

template <typename F, typename... Args, typename Tuple, std::size_t... Is>
void tuple_call(F f, Tuple && t, std::index_sequence<Is...> is, Args&&... args) {
	f(args..., std::get<Is>( std::forward<Tuple>(t) )...);
}

template <typename F, typename... Args, typename Tuple>
void call(F f, Tuple && t, Args... args) {
	using ttype = typename std::decay<Tuple>::type;
	tuple_call(f, std::forward<Tuple>(t), std::make_index_sequence<std::tuple_size<ttype>::value>{}, std::forward<Args>(args)...);
}

} // end namespace
// -------------------------------
#endif


// namespace EventSystem {


class EventSystem {
	public:
		typedef unsigned int id_t;
	private:
	
		typedef std::function<void(const void*)> any_call;
		
		void _emit(id_t id, const void* args);
		id_t _register(const std::string& evt_name, std::function<void(bool,id_t,const void*)> f);
		id_t _listen(id_t id, any_call f, void* args);
		id_t _listen_once(id_t id, any_call f, void* args);
		template<typename... Args, typename F>
		decltype(auto) convert_to_any_call(F f) {
			using Tuple = typename EventSystem_internal::function_type_information<F,sizeof...(Args)>::tuple;
			return [=](Args... args, const void* v) {
				const Tuple &t = *static_cast<const Tuple*>(v);
				EventSystem_internal::call(f, t, args...);
			};
		}
		
		// typedef std::array<int, 64> block;
		// my::fsa<block> allocator;
		struct listener {
			bool removed;
			bool oneshot;
			listener() { 
				removed=false;
				oneshot=false;
			}
			listener(const any_call& c) : listener() { call = c; }
			any_call call;
		};
		struct event {
			event() : removed(false) {}
			bool removed;
			id_t event_id;
			std::function<void(bool,id_t,const void*)> add_or_remove;
			std::vector<listener> listeners;
		};
		
		std::unordered_map<std::string, id_t> m_str_to_event;
		std::vector<event> m_events;
		
	public:
		
		EventSystem();

		// void( bool add_or_remove, id_t listener_id, listener_arguments )
		id_t Register(const std::string& event_name) {
			return _register(event_name, 0);
		}
		template<typename F>
		id_t Register(const std::string& event_name, F add_or_remove_func) {
			return _register(event_name, convert_to_any_call<bool,id_t>(add_or_remove_func));
		}

		void Unregister( const std::string& registered_event_name );
		void Unregister( id_t event_id );
		void StopListening( id_t listener_id );

		template <typename F, typename ...Args>
		id_t Listen(std::string str, F func, Args... args) {
			auto it = m_str_to_event.find(str);
			if(it != m_str_to_event.end()) {
				return Listen(it->second, func, args...);
			} else {
				return -1;
			}
		}
		
		template <typename F, typename ...Args>
		id_t ListenOnce(std::string str, F func, Args... args) {
			auto it = m_str_to_event.find(str);
			if(it != m_str_to_event.end()) {
				return ListenOnce(it->second, func, args...);
			} else {
				return -1;
			}
		}
		
		template <typename F, typename ...Args>
		id_t Listen(id_t event_id, F func, Args... args) {
			std::tuple<Args...> tuple_args(args...);
			return _listen(event_id, convert_to_any_call(func), &tuple_args);
		}
		
		template <typename F, typename ...Args>
		id_t ListenOnce(id_t event_id, F func, Args... args) {
			std::tuple<Args...> tuple_args(args...);
			return _listen_once(event_id, convert_to_any_call(func), &tuple_args);
		}

		template <typename ...Args>
		void Emit( id_t id, Args... args ) {
			std::tuple<Args...> tuple_args(args...);
			_emit(id, &tuple_args);
		}
		
		template <typename ...Args>
		void Emit( const std::string& evt_name, Args... args ) {
			std::tuple<Args...> tuple_args(args...);
			auto it = m_str_to_event.find(evt_name);
			if(it != m_str_to_event.end()) {
				_emit(it->second, &tuple_args);				
			} else {
				id_t id = Register(evt_name);
				_emit(id, &tuple_args);
			}
		}
		
		
		template <typename ...Args>
		static inline void GetData( const void* data, Args&... args ) {
			std::tie(args...) = *static_cast<const std::tuple<Args...>*>(data);
		}
};

// singleton
extern EventSystem singleton;
inline id_t Register(const std::string event_name) {
	return singleton.Register(event_name);
}

template<typename F>
inline id_t Register(const std::string event_name, F add_or_remove_func) {
	return singleton.Register(event_name, add_or_remove_func);
}

inline void Unregister( const std::string& registered_event_name ) {
	singleton.Unregister(registered_event_name);
}
inline void Unregister( id_t event_id ) {
	singleton.Unregister(event_id);
}
inline void StopListening( id_t listener_id ) {
	return singleton.StopListening(listener_id);
}
template <typename F, typename ...Args>
inline id_t Listen(const std::string str, F func, Args... args) {
	return singleton.Listen(str, func, args...);
}

template <typename F, typename ...Args>
inline id_t Listen(id_t event_id, F func, Args... args) {
	return singleton.Listen(event_id, func, args...);
}

template <typename ...Args>
void Emit( id_t id, Args... args ) {
	singleton.Emit(id,args...);
}

template <typename ...Args>
void Emit( const std::string evt_name, Args... args ) {
	singleton.Emit(evt_name, args...);
}

EventSystem& GetSingleton();
#endif
