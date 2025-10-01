#pragma once

namespace mrv
{
    enum class License
    {
        kValid = 0,
        kInvalid = 1,
        kExpired = 2,
    };

    
    std::string get_machine_id();
    
    License validate_license(std::string& expiration_date);
    License license_beat();
}
