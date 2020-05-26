#pragma once

#include <vector>
#include <memory>

namespace Bat
{
	template <typename T>
	class State
	{
	public:
		virtual ~State() = default;

		virtual void OnEnter(T& obj) {};
		virtual void OnExit(T& obj) {};
		virtual void Execute(T& obj) = 0;
	};

	template <typename T>
	class NullState : public State<T>
	{
		virtual void Execute(T& obj) {}
	};

	template <typename T>
	class StateMachine
	{
	public:
		StateMachine( T& parent )
			:
			m_Parent( parent )
		{
			m_StateStack.push_back( std::make_unique<NullState<T>>() );
		}

		void PushState(std::unique_ptr<State<T>> state)
		{
			m_StateStack.back()->OnExit( m_Parent );
			m_StateStack.push_back( std::move( state ) );
			m_StateStack.back()->OnEnter( m_Parent );
		}
		void PopState()
		{
			m_StateStack.back()->OnExit( m_Parent );
			m_StateStack.pop_back();
			m_StateStack.back()->OnEnter( m_Parent );
		}
		void ChangeState(std::unique_ptr<State<T>> state)
		{
			m_StateStack.back()->OnExit( m_Parent );
			m_StateStack.back() = std::move( state );
			m_StateStack.back()->OnEnter( m_Parent );
		}

		void Execute()
		{
			if( m_pGlobalState )
			{
				m_pGlobalState->Execute( m_Parent );
			}

			m_StateStack.back()->Execute( m_Parent );
		}

		State<T>* GetCurrentState() { return m_StateStack.back().get(); }
		void SetGlobalState( std::unique_ptr<State<T>> state ) { m_pGlobalState = std::move( state ); }
		State<T>* GetGlobalState() { return m_pGlobalState.get(); }
	private:
		std::vector<std::unique_ptr<State<T>>> m_StateStack;
		std::unique_ptr<State<T>> m_pGlobalState;
		T& m_Parent;
	};
}