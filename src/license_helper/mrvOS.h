
#include <string>

namespace mrv
{
    namespace os
    {
        //! Execute a command through pipes and capture stdout/sterr.
        //! Returns the exit value of the command.
        int exec_command(const std::string& command, std::string& output,
                         std::string& errors);

    }
}
