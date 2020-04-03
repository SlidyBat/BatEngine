#pragma once

#include <string>
#include "Reflect.h"
#include "StringLib.h"
#include "Entity.h"

class TypeToString
{
public:
	static std::string Indent( int level )
	{
		std::string indent;
		for( int i = 0; i < level; i++ )
		{
			indent += "  ";
		}
		return indent;
	}
	static std::string Dump( const Bat::TypeElement& el, int level = 0, bool last = true )
	{
		std::string s;
		s += Indent( level );
		s += Bat::Format( "%s %s (size=%i, off=%i)", el.desc.name, el.name, el.desc.size, el.offset );
		if( el.desc.num_members )
		{
			s += " {\n";
			for( size_t i = 0; i < el.desc.num_members; i++ )
			{
				s += Dump( el.desc.members[i], level + 1, i == el.desc.num_members - 1 );
			}
			s += Indent( level );
			s += "}";
		}
		s += last ? "\n" : ",\n";
		return s;
	}
	static std::string Dump( const Bat::TypeDescriptor& desc, int level = 0, bool last = true )
	{
		std::string s;
		s += Indent( level );
		s += Bat::Format( "%s (size=%i)", desc.name, desc.size );
		if( desc.num_members )
		{
			s += " {\n";
			for( size_t i = 0; i < desc.num_members; i++ )
			{
				s += Dump( desc.members[i], level + 1, i == desc.num_members - 1 );
			}
			s += Indent( level );
			s += "}";
		}
		s += last ? "\n" : ",\n";
		return s;
	}

	static std::string DumpComponents( Bat::Entity e )
	{
		std::vector<Bat::ComponentId> components = Bat::world.GetComponentsList( e );

		std::string s;
		for( Bat::ComponentId component : components )
		{
			Bat::TypeDescriptor desc = Bat::GetComponentTypeDescriptor( component );
			s += Dump( desc );
		}

		return s;
	}
};