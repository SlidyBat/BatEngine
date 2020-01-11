#pragma once


inline void HashCombine( size_t& seed ) { }

template <typename T, typename... Rest>
inline void HashCombine( size_t& seed, const T& v, Rest... rest ) {
	std::hash<T> hasher;
	seed ^= hasher( v ) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	HashCombine( seed, rest... );
}

#define MAKE_HASHABLE(type, var, ...) \
	namespace std {\
		template<> struct hash<type> {\
			size_t operator()(const type &var) const {\
				size_t ret = 0;\
				HashCombine(ret, __VA_ARGS__);\
				return ret;\
			}\
		};\
	}

class Hasher
{
public:
	template <typename T>
	void Add( const T& var )
	{
		hash_combine( result, var );
	}

	size_t Result() const { return result; }
private:
	size_t result = 0;
};