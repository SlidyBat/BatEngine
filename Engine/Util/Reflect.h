#pragma once

#include <memory>
#include <vector>
#include <string>

// Declares the necessary functions for the class to be reflected
// NOTE: Must have public visibility
// Example usage:
//  class MyClass
//  {
//    public:
//      BAT_REFLECT();
//      int x, y, z;
//  }
#define BAT_REFLECT() \
	static constexpr const char* GetClassName(); \
	static Bat::TypeDescriptor Reflect()

// Does the actual work of reflecting a class
// NOTE: Must be used in a .cpp file
// Example usage:
//  BAT_REFLECT_BEGIN( MyClass );
//    BAT_REFLECT_MEMBER( x );
//    BAT_REFLECT_MEMBER( y );
//    BAT_REFLECT_MEMBER( z );
//  BAT_REFLECT_END();
#define BAT_REFLECT_BEGIN( classname ) \
	constexpr const char* classname::GetClassName() { return #classname; } \
	template <> Bat::TypeDescriptor Bat::GetTypeDescriptorImpl<classname>() { return classname::Reflect(); } \
	Bat::TypeDescriptor classname::Reflect() { \
		using ThisClass = classname; \
		Bat::TypeDescriptor desc; \
		desc.name = #classname; \
		desc.size = sizeof( classname ); \
		std::vector<Bat::TypeElement> elements

#define BAT_REFLECT_MEMBER( membername ) \
		elements.emplace_back( Bat::TypeElement{ #membername, offsetof( ThisClass, membername ), Bat::GetTypeDescriptor<decltype( ThisClass::membername )>() } )

#define BAT_REFLECT_END() \
		desc.num_members = elements.size(); \
		desc.members = std::make_unique<Bat::TypeElement[]>( desc.num_members ); \
		for( size_t i = 0; i < elements.size(); i++ ) { desc.members[i] = std::move( elements[i] ); } \
		return desc; \
	}

// Same as BAT_REFLECT_BEGIN, but doesn't depend on the given class having a `Reflect()` member function (and therefore no need for BAT_REFLECT() to be used)
// Disadvantage is that private members can't be reflected
// Use this for external classes that you can't/won't change the definition of
#define BAT_REFLECT_EXTERNAL_BEGIN( classname ) \
	template <> Bat::TypeDescriptor Bat::GetTypeDescriptorImpl<classname>() {  \
		using ThisClass = classname; \
		Bat::TypeDescriptor desc; \
		desc.name = #classname; \
		desc.size = sizeof( classname ); \
		std::vector<Bat::TypeElement> elements

namespace Bat
{
	struct TypeElement;

	struct TypeDescriptor
	{
		const char* name;
		size_t size;
		std::unique_ptr<TypeElement[]> members;
		size_t num_members;
	};

	struct TypeElement
	{
		const char* name;
		size_t offset;
		TypeDescriptor desc;
	};

	template <typename T>
	TypeDescriptor GetTypeDescriptorImpl();

	template <typename T>
	class TypeResolver
	{
	public:
		static TypeDescriptor GetTypeDescriptor()
		{
			return GetTypeDescriptorImpl<T>();
		}
	};

	template <typename T>
	TypeDescriptor GetTypeDescriptor()
	{
		return TypeResolver<T>::GetTypeDescriptor();
	}

	template <typename T>
	class TypeResolver<T*>
	{
	public:
		static TypeDescriptor GetTypeDescriptor()
		{
			TypeDescriptor desc;
			desc.name = "raw_ptr";
			desc.size = sizeof( T* );
			desc.num_members = 1;
			desc.members = std::make_unique<TypeElement[]>( 1 );

			auto& val = desc.members[0];
			val.name = "val";
			val.offset = 0;
			val.desc = Bat::GetTypeDescriptor<T>();

			return desc;
		}
	};

	template <typename T>
	class TypeResolver<std::unique_ptr<T>>
	{
	public:
		static TypeDescriptor GetTypeDescriptor()
		{
			TypeDescriptor desc;
			desc.name = "std::unique_ptr";
			desc.size = sizeof( std::unique_ptr<T> );
			desc.num_members = 1;
			desc.members = std::make_unique<TypeElement[]>( 1 );

			auto& val = desc.members[0];
			val.name = "val";
			val.offset = 0;
			val.desc = Bat::GetTypeDescriptor<T>();

			return desc;
		}
	};

	template <typename T>
	class TypeResolver<std::vector<T>>
	{
	public:
		static TypeDescriptor GetTypeDescriptor()
		{
			TypeDescriptor desc;
			desc.name = "std::vector";
			desc.size = sizeof( std::vector<T> );
			desc.num_members = 1;
			desc.members = std::make_unique<TypeElement[]>( 1 );

			auto& val = desc.members[0];
			val.name = "type";
			val.offset = -1;
			val.desc = Bat::GetTypeDescriptor<T>();

			return desc;
		}
	};
}