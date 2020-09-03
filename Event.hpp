#include <unordered_map>
#include <string>
#include <functional>
#include <tuple>
#include <map>
#include <vector>
#include <utility>
// #include "fsa.hpp"

// #ifndef CALL_TUPLE_ARGS
// #define CALL_TUPLE_ARGS
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
// -------------------------------
// #endif

namespace EventSystem {
	
typedef unsigned int id_type;

class Event {
	private:
	
		typedef std::function<void(const void*)> any_call;
		
		void _emit(id_type id, const void* args);
		id_type _register(const std::string& evt_name, std::function<void(bool,id_type,const void*)> f);
		id_type _listen(id_type id, bool once, any_call f, void* args);
		
		template<typename... Args, typename F>
		decltype(auto) convert_to_any_call(F f) {
			using Tuple = typename function_type_information<F,sizeof...(Args)>::tuple;
			return [=](Args... args, const void* v) {
				const Tuple &t = *static_cast<const Tuple*>(v);
				call(f, t, args...);
			};
		}
		
		// typedef std::array<int, 64> block;
		// my::fsa<block> allocator;
		
		struct Listener {
			id_type id;
			bool listen_once;
			any_call func;
		};
			
		struct event {
			id_type event_id;
			std::function<void(bool,id_type,const void*)> add_or_remove;			
			int has_listen_once;
			int listener_free_id;
			std::vector<Listener> listeners;
		};
		
		// for Listen("some_event")
		std::unordered_map<std::string, id_type> m_str_to_event;
		
		// for unlistening
		std::map<id_type, std::pair<event*, int>> m_listener_to_idx;
		
		// for locating event listeners by id_type
		std::vector<event> m_events;
		int m_event_free_id;
		
	public:
		Event();

		// void( bool add_or_remove, id_type listener_id, listener_arguments )
		id_type Register(const std::string& event_name) {
			return _register(event_name, 0);
		}
		template<typename F>
		id_type Register(const std::string& event_name, F add_or_remove_func) {
			return _register(event_name, convert_to_any_call<bool,id_type>(add_or_remove_func));
		}
		
		id_type GetEventId(const std::string& event_name) {
			auto it = m_str_to_event.find(event_name);
			if(it != m_str_to_event.end()) {
				return it->second;
			}
			return -1;
		}

		// void Unregister( const std::string& registered_event_name );
		// void Unregister( id_type event_id );
		void StopListening( id_type listener_id );

		template <typename F, typename ...Args>
		id_type Listen(std::string str, bool once, F func, Args... args) {
			auto it = m_str_to_event.find(str);
			if(it != m_str_to_event.end()) {
				return Listen(it->second, once, func, args...);
			} else {
				id_type id = Register(str);
				return Listen(str, once, func, args...);
			}
		}
		
		template <typename F, typename ...Args>
		id_type Listen(id_type event_id, bool once, F func, Args... args) {
			// TODO: put somewhere on fsa (listener args) (multithreading)
			std::tuple<Args...> tuple_args(args...);
			id_type id = _listen(event_id, once, convert_to_any_call(func), &tuple_args);
			return id;
		}

		template <typename ...Args>
		void Emit( id_type id, Args... args ) {
			// TODO: put somewhere on fsa (multithreading)
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
				id_type id = Register(evt_name);
				_emit(id, &tuple_args);
			}
		}
		
		
		template <typename ...Args>
		static inline void GetData( const void* data, Args&... args ) {
			std::tie(args...) = *static_cast<const std::tuple<Args...>*>(data);
		}
};

	// singleton
	extern Event singleton;
	inline id_type Register(const std::string event_name) {
		return singleton.Register(event_name);
	}
	
	template<typename F>
	inline id_type Register(const std::string event_name, F add_or_remove_func) {
		return singleton.Register(event_name, add_or_remove_func);
	}

	/*
	inline void Unregister( const std::string& registered_event_name ) {
		singleton.Unregister(registered_event_name);
	}
	inline void Unregister( id_type event_id ) {
		singleton.Unregister(event_id);
	}
	*/
	
	inline void StopListening( id_type listener_id ) {
		return singleton.StopListening(listener_id);
	}
	template <typename F, typename ...Args>
	inline id_type Listen(const std::string str, bool once, F func, Args... args) {
		return singleton.Listen(str, once, func, args...);
	}
	
	template <typename F, typename ...Args>
	inline id_type Listen(id_type event_id, bool once, F func, Args... args) {
		return singleton.Listen(event_id, once, func, args...);
	}

	template <typename ...Args>
	void Emit( id_type id, Args... args ) {
		singleton.Emit(id,args...);
	}
	
	template <typename ...Args>
	void Emit( const std::string evt_name, Args... args ) {
		singleton.Emit(evt_name, args...);
	}

	Event& GetSingleton();

}
