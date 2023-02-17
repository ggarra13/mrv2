
#pragma once

namespace mrv
{


	class SignalHandler
	{
	public:
		SignalHandler();
		~SignalHandler();

		
	private:
		void install_signal_handler();
		void restore_signal_handler();
	};
	
}

