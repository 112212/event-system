#ifndef EVENT_SYSTEM_MACROS_HPP
#define EVENT_SYSTEM_MACROS_HPP

#define _ARG20(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, ...) _20
#define NARG20(...) _ARG20(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define NARG20_HALF(...) _ARG20(__VA_ARGS__, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0)

#define CAT(L, R) CAT_(L, R)
#define CAT_(L, R) L ## R

#define __replace1(_1, ...) _1;
#define __replace2(_1, ...) _1;  __replace1(__VA_ARGS__)
#define __replace3(_1, ...) _1;  __replace2(__VA_ARGS__)
#define __replace4(_1, ...) _1;  __replace3(__VA_ARGS__)
#define __replace5(_1, ...) _1;  __replace4(__VA_ARGS__)
#define __replace6(_1, ...) _1;  __replace5(__VA_ARGS__)
#define __replace7(_1, ...) _1;  __replace6(__VA_ARGS__)
#define __replace8(_1, ...) _1;  __replace7(__VA_ARGS__)
#define __replace9(_1, ...) _1;  __replace8(__VA_ARGS__)
#define __replace10(_1, ...) _1; __replace9(__VA_ARGS__)

#define __replace_2_1(_1,  _2, ...)
#define __replace_2_2(_1,  _2, ...)  _1  _2; //__replace_2_1(__VA_ARGS__)
#define __replace_2_3(_1,  _2, ...)  _1  _2; __replace_2_2(__VA_ARGS__)
#define __replace_2_4(_1,  _2, ...)  _1  _2; __replace_2_3(__VA_ARGS__)
#define __replace_2_5(_1,  _2, ...)  _1  _2; __replace_2_4(__VA_ARGS__)
#define __replace_2_6(_1,  _2, ...)  _1  _2; __replace_2_5(__VA_ARGS__)
#define __replace_2_7(_1,  _2, ...)  _1  _2; __replace_2_6(__VA_ARGS__)
#define __replace_2_8(_1,  _2, ...)  _1  _2; __replace_2_7(__VA_ARGS__)
#define __replace_2_9(_1,  _2, ...)  _1  _2; __replace_2_8(__VA_ARGS__)
#define __replace_2_10(_1, _2, ...)  _1  _2; __replace_2_9(__VA_ARGS__)

#define VAR_NAMES1(_1, _2, ...)
#define VAR_NAMES2(_1, _2, ...)  _2
#define VAR_NAMES3(_1, _2, ...)  _2, VAR_NAMES2(__VA_ARGS__)
#define VAR_NAMES4(_1, _2, ...)  _2, VAR_NAMES3(__VA_ARGS__)
#define VAR_NAMES5(_1, _2, ...)  _2, VAR_NAMES4(__VA_ARGS__)
#define VAR_NAMES6(_1, _2, ...)  _2, VAR_NAMES5(__VA_ARGS__)
#define VAR_NAMES7(_1, _2, ...)  _2, VAR_NAMES6(__VA_ARGS__)
#define VAR_NAMES8(_1, _2, ...)  _2, VAR_NAMES7(__VA_ARGS__)
#define VAR_NAMES9(_1, _2, ...)  _2, VAR_NAMES8(__VA_ARGS__)
#define VAR_NAMES10(_1, _2, ...) _2, VAR_NAMES9(__VA_ARGS__)

#define _rpl(N, ...) CAT(__replace, N)(__VA_ARGS__)
#define replace(...) _rpl(NARG20(__VA_ARGS__), __VA_ARGS__)

#define SPLIT1(_1, ...)
#define SPLIT2(_1, ...) CAT(SPL_, _1)
#define SPLIT3(_1, ...) CAT(SPL_, _1), SPLIT2(__VA_ARGS__)
#define SPLIT4(_1, ...) CAT(SPL_, _1), SPLIT3(__VA_ARGS__)
#define SPLIT5(_1, ...) CAT(SPL_, _1), SPLIT4(__VA_ARGS__)
#define SPLIT6(_1, ...) CAT(SPL_, _1), SPLIT5(__VA_ARGS__)
#define SPLIT7(_1, ...) CAT(SPL_, _1), SPLIT6(__VA_ARGS__)
#define SPLIT8(_1, ...) CAT(SPL_, _1), SPLIT7(__VA_ARGS__)
#define SPLIT9(_1, ...) CAT(SPL_, _1), SPLIT8(__VA_ARGS__)
#define SPLIT10(_1, ...) CAT(SPL_, _1), SPLIT9(__VA_ARGS__)

#define GET_NTH_ARG1(_1, ...) _1
#define GET_NTH_ARG2(_1, ...) GET_NTH_ARG1(__VA_ARGS__)
#define GET_NTH_ARG3(_1, ...) GET_NTH_ARG2(__VA_ARGS__)
#define GET_NTH_ARG4(_1, ...) GET_NTH_ARG3(__VA_ARGS__)
#define GET_NTH_ARG5(_1, ...) GET_NTH_ARG4(__VA_ARGS__)
#define GET_NTH_ARG6(_1, ...) GET_NTH_ARG5(__VA_ARGS__)
#define GET_NTH_ARG7(_1, ...) GET_NTH_ARG6(__VA_ARGS__)
#define GET_NTH_ARG8(_1, ...) GET_NTH_ARG7(__VA_ARGS__)
#define GET_NTH_ARG9(_1, ...) GET_NTH_ARG8(__VA_ARGS__)
#define GET_NTH_ARG10(_1, ...) GET_NTH_ARG9(__VA_ARGS__)

#define GET_NTH_ARG(N,...) CAT(GET_NTH_ARG, N) (__VA_ARGS__)

// standard types
#define SPL_int
#define SPL_uint
#define SPL_char
#define SPL_void
#define SPL_float
#define SPL_unsigned
#define SPL_long

#define __evt2(N, ...) CAT(SPLIT, N) (__VA_ARGS__)
#define _evt2(...) Event::GetData( data, __evt2(NARG20(__VA_ARGS__),__VA_ARGS__) )


// ---------------------------------------------------------------------------------------------

/*
	this versions supports only types written in one word and defined here as standard types
	supports maximum 9 arguments
	example:
		EVENT(function_name, int a, int b, {
			cout << "function_name happened\n";
		})
*/
#define EVENT(func, ...) void func(const void* data) { \
	replace(__VA_ARGS__)  \
	_evt2(__VA_ARGS__);	  \
	GET_NTH_ARG(NARG20(__VA_ARGS__), __VA_ARGS__) \
}

/*
	version that supports all types but they need to be written separated

*/
#define EVENT2(func, ...) void func(const void* data) { \
	CAT(__replace_2_, NARG20_HALF(__VA_ARGS__))(__VA_ARGS__) \
	Event::GetData( data, CAT(VAR_NAMES, NARG20_HALF(__VA_ARGS__))(__VA_ARGS__) ); \
	GET_NTH_ARG(NARG20(__VA_ARGS__), __VA_ARGS__) \
}


#endif
