#include <unordered_map>
#include <string>
#include <functional>
#include <tuple>
#include <vector>
#include <utility>
// #include "fsa.hpp"
// ----- function type information
template< typename t, std::size_t n, typename = void >
struct function_type_information;

template< typename r, typename ... a, std::size_t n >
struct function_type_information< r (*)( a ... ), n > {
	// using type = typename std::tuple_element< n, std::tuple< a ... > >::type; 
	using tuple = std::tuple<a...>;
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

namespace my {
	template <typename F, typename Tuple, std::size_t... Is>
	void tuple_call(F f, Tuple && t, std::index_sequence<Is...> is) {
		f(std::get<Is>( std::forward<Tuple>(t) )...);
	}
	
	template <typename F, typename Tuple>
	void call(F f, Tuple && t) {
		using ttype = typename std::decay<Tuple>::type;
		tuple_call(f, std::forward<Tuple>(t), std::make_index_sequence<std::tuple_size<ttype>::value>{});
	}
}

class Event {
	public:
		typedef unsigned int id_type;
	private:
		
		typedef std::function<void(const void*)> any_call;
		
		void _emit(id_type id, const void* args);
		id_type _register(std::string& evt_name, any_call f);
		id_type _listen(id_type id, any_call f, void* args, id_type& idt);
		
		template<typename F>
		any_call convert_to_any_call(F f) {
			using Tuple = typename function_type_information<F,0>::tuple;
			return [=](const void* v) {
				const Tuple &t = *static_cast<const Tuple*>(v);
				my::call(f, t);
			};
		}
		
		// typedef std::array<int, 64> block;
		// my::fsa<block> allocator;
		
		struct event {
			id_type event_id;
			any_call add_or_remove;
			std::vector<any_call> listeners;
		};
		
		std::unordered_map<std::string, id_type> m_str_to_event;
		std::vector<event> m_events;
		
	public:
		Event();

		// void( bool add_or_remove, id_type listener_id, listener_arguments )
		id_type Register(std::string event_name) {
			return _register(event_name, 0);
		}
		template<typename F>
		id_type Register(std::string event_name, F add_or_remove_func) {
			return _register(event_name, convert_to_any_call(add_or_remove_func));
		}

		void Unregister( std::string registered_event_name );
		void Unregister( id_type event_id );
		void StopListening( id_type listener_id );

		template <typename F, typename ...Args>
		id_type Listen(std::string str, F func, Args... args) {
			auto it = m_str_to_event.find(str);
			if(it != m_str_to_event.end()) {
				return Listen(it->second, func, args...);
			} else {
				return -1;
			}
		}
		
		template <typename F, typename ...Args>
		id_type Listen(id_type event_id, F func, Args... args) {
			std::tuple<bool, id_type, Args...> tuple_args(true, 0, args...);
			id_type id = _listen(event_id, convert_to_any_call(func), &tuple_args, std::get<1>(tuple_args));
			return id;
		}

		template <typename ...Args>
		void Emit( id_type id, Args... args ) {
			std::tuple<Args...> tuple_args(args...);
			_emit(id, &tuple_args);
		}
		
		
		template <typename ...Args>
		static inline void GetData( const void* data, Args&... args ) {
			std::tie(args...) = *static_cast<const std::tuple<Args...>*>(data);
		}
};

