#pragma once

#include <srl.hpp>

namespace Log
{
class Logger
{
public:
	template <typename ...Args>
	static void Print(const char* text, Args...args)
	{
		SRL::Debug::Print(0, m_logY, text, args ...);

		m_logY++;

		if (m_logY > 28)
			m_logY = 0;
	}

private:
	static int m_logY;
};
}