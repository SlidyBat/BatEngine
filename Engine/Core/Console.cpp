#include "PCH.h"
#include "Console.h"

#include "Util/TokenStream.h"

namespace Bat
{
	Console g_Console;

	Console::Console()
	{
		ClearLog();
		memset(InputBuf, 0, sizeof(InputBuf));
		HistoryPos = -1;
	}

	void Console::ClearLog()
	{
		Items.clear();
		ScrollToBottom = true;
	}
	void Console::Draw( const std::string& title )
	{
		if( !IsVisible() )
		{
			return;
		}

		ImGui::SetNextWindowSize(ImVec2(520,600), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin(title.c_str()))
		{
			ImGui::End();
			return;
		}

		if (ImGui::BeginPopupContextItem())
		{
			if( ImGui::MenuItem( "Close Console" ) )
				SetVisible( false );
			ImGui::EndPopup();
		}

		if (ImGui::SmallButton("Clear")) { ClearLog(); } ImGui::SameLine();
		bool copy_to_clipboard = ImGui::SmallButton("Copy"); ImGui::SameLine();
		if (ImGui::SmallButton("Scroll to bottom")) ScrollToBottom = true;

		ImGui::Separator();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
		static ImGuiTextFilter filter;
		filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
		ImGui::PopStyleVar();
		ImGui::Separator();

		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Clear")) ClearLog();
			ImGui::EndPopup();
		}

		// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
		// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
		// You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
		// To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
		//     ImGuiListClipper clipper(Items.Size);
		//     while (clipper.Step())
		//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		// However, note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
		// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
		// and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
		// If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1)); // Tighten spacing
		if (copy_to_clipboard)
			ImGui::LogToClipboard();
		ImVec4 col_default_text = ImGui::GetStyleColorVec4(ImGuiCol_Text);
		for (size_t i = 0; i < Items.size(); i++)
		{
			const std::string item = Items[i];
			if (!filter.PassFilter(item.c_str()))
				continue;
			ImVec4 col = col_default_text;
			if (item.find("[error]") != std::string::npos) col = ImColor(1.0f,0.4f,0.4f,1.0f);
			else if (item.substr(0, 2) == "# ") col = ImColor(1.0f,0.78f,0.58f,1.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, col);
			ImGui::TextUnformatted(item.c_str());
			ImGui::PopStyleColor();
		}
		if (copy_to_clipboard)
			ImGui::LogFinish();
		if (ScrollToBottom)
			ImGui::SetScrollHereY(1.0f);
		ScrollToBottom = false;
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::Separator();

		// Command-line
		bool reclaim_focus = false;
		if (ImGui::InputText("Input", InputBuf, ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackCompletion|ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
		{
			char* s = InputBuf;
			Bat::Trim(s);
			if (s[0])
				ExecCommand(s);
			s[0] = '\0';
			reclaim_focus = true;
		}

		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();
		if (reclaim_focus)
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

		ImGui::End();
	}

	void Console::AddCommand( const std::string& command, CommandCallback_t callback )
	{
#ifdef _DEBUG
		auto it = Commands.find( command );
		if( it != Commands.end() )
		{
			BAT_WARN( "Command '%s' registered more than once", command );
		}
#endif

		Commands[command] = callback;
	}

	void Console::ExecCommand( const std::string& command_line )
	{
		AddLog("# %s\n", command_line);

		// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
		HistoryPos = -1;
		for (int i = (int)History.size() - 1; i >= 0; i--)
		{
			if (History[i] == command_line)
			{
				History.erase(History.begin() + i);
				break;
			}
		}
		History.push_back(command_line);

		// Process command
		TokenStream ts( command_line );
		std::string command = std::string( ts.NextToken() ); // first token is the command
		auto it = Commands.find( command );
		if( it != Commands.end() )
		{
			std::string_view token;
			CommandArgs_t args;
			args.emplace_back( command ); // args[0] is the command itself
			while( ts >> token )
			{
				args.emplace_back( token );
			}

			it->second( args );
		}
	}

	int Console::TextEditCallback( ImGuiInputTextCallbackData* data )
	{
		//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
		switch (data->EventFlag)
		{
			case ImGuiInputTextFlags_CallbackCompletion:
			{
				// Example of TEXT COMPLETION

				// Locate beginning of current word
				const char* word_end = data->Buf + data->CursorPos;
				const char* word_start = word_end;
				while (word_start > data->Buf)
				{
					const char c = word_start[-1];
					if (c == ' ' || c == '\t' || c == ',' || c == ';')
						break;
					word_start--;
				}

				// Build a list of candidates
				std::vector<std::string> candidates;
				for (auto& command : Commands)
				{
					if (command.first.substr(0, (size_t)(word_end-word_start)) == word_start)
						candidates.push_back(command.first);
				}

				if (candidates.size() == 0)
				{
					AddLog("No match for \"%s\"!\n", word_start);
				}
				else if (candidates.size() == 1)
				{
					// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
					data->DeleteChars((int)(word_start-data->Buf), (int)(word_end-word_start));
					data->InsertChars(data->CursorPos, candidates[0].c_str());
					data->InsertChars(data->CursorPos, " ");
				}
				else
				{
					// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
					int match_len = (int)(word_end - word_start);
					for (;;)
					{
						int c = 0;
						bool all_candidates_matches = true;
						for (size_t i = 0; i < candidates.size() && all_candidates_matches; i++)
							if (i == 0)
								c = toupper(candidates[i][match_len]);
							else if (c == 0 || c != toupper(candidates[i][match_len]))
								all_candidates_matches = false;
						if (!all_candidates_matches)
							break;
						match_len++;
					}

					if (match_len > 0)
					{
						data->DeleteChars((int)(word_start - data->Buf), (int)(word_end-word_start));
						data->InsertChars(data->CursorPos, candidates[0].c_str(), candidates[0].c_str() + match_len);
					}

					// List matches
					AddLog("Possible matches:\n");
					for (size_t i = 0; i < candidates.size(); i++)
						AddLog("- %s\n", candidates[i]);
				}

				break;
			}
			case ImGuiInputTextFlags_CallbackHistory:
			{
				// Example of HISTORY
				const int prev_history_pos = HistoryPos;
				if (data->EventKey == ImGuiKey_UpArrow)
				{
					if (HistoryPos == -1)
						HistoryPos = (int)History.size() - 1;
					else if (HistoryPos > 0)
						HistoryPos--;
				}
				else if (data->EventKey == ImGuiKey_DownArrow)
				{
					if (HistoryPos != -1)
						if (++HistoryPos >= (int)History.size())
							HistoryPos = -1;
				}

				// A better implementation would preserve the data on the current input line along with cursor position.
				if (prev_history_pos != HistoryPos)
				{
					const char* history_str = (HistoryPos >= 0) ? History[HistoryPos].c_str() : "";
					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, history_str);
				}
			}
		}
		return 0;
	}

	int Console::TextEditCallbackStub(ImGuiInputTextCallbackData* data)
	{
		Console* console = (Console*)data->UserData;
		return console->TextEditCallback(data);
	}
}