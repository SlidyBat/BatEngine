#pragma once

#include <imgui.h>
#include <vector>
#include <functional>
#include "StringLib.h"
#include <spdlog/fmt/fmt.h>

namespace Bat
{
	using CommandArgs_t = std::vector<std::string_view>;
	using CommandCallback_t = std::function<void( const CommandArgs_t& args )>;

	class Console
	{
	public:
	public:
		Console();

		template <typename... Args>
		void AddLog( const std::string& format, Args&&... args )
		{
			Items.push_back( Bat::Format( format, args ) );
			ScrollToBottom = true;
		}
		void ClearLog();

		void Draw( const std::string& title );
		void AddCommand( const std::string& command, CommandCallback_t callback );
		void ExecCommand( const std::string& command_line );

		bool IsVisible() const { return m_bVisible; }
		void SetVisible( const bool visible ) { m_bVisible = visible; }
	private:
		static int TextEditCallbackStub( ImGuiInputTextCallbackData* data );
		int TextEditCallback( ImGuiInputTextCallbackData* data );
	private:
		char                     InputBuf[256];
		std::vector<std::string> Items;
		bool                     ScrollToBottom;
		std::vector<std::string> History;
		int                      HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.

		std::unordered_map<std::string, CommandCallback_t> Commands;

		bool m_bVisible = false;
	};

	extern Console g_Console;
}