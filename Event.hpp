#include <map>
#include <list>
#include <string>
#include <functional>
#include <tuple>
#include "fsa.hpp"

class EventBase;

class Event {
	public:
		Event();

		/*
			first version of RegisterEvent is used only for base events
			callable from anywhere and they are usually used to attach event checkers to these events
			or add event listeners directly to these base events
		*/
		unsigned int RegisterEvent(std::string);

		/*
			this is sort of second degree of event checker which checks whether new event
			happens and when it happens it notifies every event listener assigned to it
		*/
		template <typename ...Args>
		unsigned int RegisterEvent(std::string event, std::string attach, std::function<void(unsigned int, const void*)> func, Args... args) {
			//std::tuple<Args...> tupl(args...);
			void* tupl = allocator.alloc();
			*static_cast<std::tuple<Args...>*>(tupl) = std::tuple<Args...>(args...);
			return registerEvent(event, attach, func, tupl);
		}

		/*
			this is event checker which is allowed to use extra conditions for event matching
			it checks which of the event listeners (which supplied extra conditional parameters)
			matches conditions and it notifies only them

			it is used with class which must inherit EventBase and must overload these functions:
				void OnAddListener( int listener_id, const void* ) = 0;
				void OnCheckEvent( const void* ) = 0;
				void OnRemoveListener( int listener_id ) = 0;
			class which defines this new event checker must add additonal container which holds
			listener ids and additional conditions received at OnAddListener function overload
		*/
		template <typename ...Args>
		unsigned int RegisterEvent(std::string event, std::string attach, EventBase* evt, Args... args) {
			void* tupl = allocator.alloc();
			*static_cast<std::tuple<Args...>*>(tupl) = std::tuple<Args...>(args...);
			return registerEvent(event, attach, evt, tupl);
		}

		void UnregisterEvent( unsigned int event_id );
		void UnregisterEvent( std::string registered_event_name );

		void RemoveEventListener( unsigned int event_id );

		template <typename ...Args>
		unsigned int AddEventListener(std::string str, std::function<void(const void*)> func, Args... args) {
			std::tuple<Args...> tupl(args...);
			return addEventListener(str,func,&tupl);
		}

		template <typename ...Args>
		void EmitEvent( unsigned int registered_event_id, Args... args ) {
			std::tuple<Args...> tupl(args...);
			emitEvent( registered_event_id, &tupl );
		}

		template <typename ...Args>
		void EmitEventDirect( unsigned int event_id, Args... args ) {
			std::tuple<Args...> tupl(args...);
			emitEventDirect( event_id, &tupl );
		}

		template <typename ...Args>
		static inline void GetData( const void* data, Args&... args ) {
			std::tie(args...) = *static_cast<const std::tuple<Args...>*>(data);
		}

	private:
		unsigned int event_id_tracker;
		struct block64 {
			char dummy[64];
		};
		my::fsa<block64> allocator;
		enum class container_type {
			type_class,
			type_function,
			type_empty,
			type_event_handler
		};
		struct container {
			container() {}
			container( container_type t ) : type(t) {}
			container_type type;
			EventBase* evt;
			std::function<void(unsigned int, const void *)> func;
			unsigned int event_id;
			unsigned int listener_event_id;
			unsigned int attached_to;
		};
		struct checker_container {
			checker_container( unsigned int _event_id ) : event_id(_event_id) {}
			unsigned int event_id;
			std::function<void(const void*)> func;
		};
		struct direct_event_container {
			container_type type;
			const void* data;
			unsigned int checker_id;
			EventBase* evt;
			std::function<void(unsigned int, const void*)> checker_func;
			std::function<void(const void*)> func;
		};
		friend bool operator==(const checker_container &a, const checker_container &b);

		// oh no another map
		std::map< unsigned int, container > registered_checkers;

		// not good
		std::map< unsigned int, std::list<container> > attached_checkers;

		// nooo
		std::map< unsigned int, std::list<checker_container> > checker_listeners;

		std::map< unsigned int, unsigned int > listener_to_registered_event_id;

		std::map< unsigned int, direct_event_container > listener_contact;

		unsigned int hash( std::string& str );
		unsigned int addEventListener(std::string&, std::function<void(const void*)>, const void*);
		void emitEvent( unsigned int registered_event_id, const void* data );
		void emitEventDirect( unsigned int event_id, const void* data );
		unsigned int registerEvent(std::string, std::string, std::function<void(unsigned int, const void*)> func, const void* data);
		unsigned int registerEvent(std::string, std::string, EventBase* evt, const void * data);
};

class EventBase {
	private:
		int event_id;
		friend class Event;
		Event *eventsystem;

	public:
		EventBase() {}
		template <typename ...Args>
		/*
			desc: should call when emitting events
			@param: listener_event_id
				this parameter should be catched with OnAddListener and stored in some container
				check which listener should be called and emit event just to them, not all listeners
			@param: params
				additional parameters sent to event listener
		*/
		void EmitEvent( unsigned int listener_event_id, Args... params ) {
			eventsystem->EmitEventDirect( listener_event_id, params... );
		}

		// must overload
		virtual void OnAddListener( unsigned int listener_id, const void* ) = 0;
		virtual void OnCheckEvent( const void* ) = 0;
		virtual void OnRemoveListener( int listener_id ) = 0;
};
