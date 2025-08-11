//
// Created by floweryclover on 2025-08-01.
//

#ifndef AUTHMESSAGE_H
#define AUTHMESSAGE_H

#include "ErrorMacros.h"
#include <string>
#include <bcrypt/bcrypt.h>
#include <array>
#include <variant>

inline static const std::string EmptyHashedPassword{ "$2a$12$YeNGaZyLvcdFapdGyzzBiubEj.LkPp/F0ZX0NydtGAvr6zGX6Cne6" };

class LoginJob final
{
public:
    static constexpr uint32_t InvalidId = std::numeric_limits<uint32_t>::max();

    const uint32_t Context;
    const uint32_t Id;
    const std::string RequestedPassword;
    const std::array<char, 64> SavedPassword;

    explicit LoginJob(const uint32_t context,
                      const uint32_t id,
                      std::string requestedPassword,
                      const std::array<char, 64>& savedPassword)
        : Context{ context },
          Id{ std::move(id) },
          RequestedPassword{ std::move(requestedPassword) },
          SavedPassword{ savedPassword },
          isPasswordMatch_{ false }
    {
    }

    void ExecuteBackgroundJob()
    {
        const int result = bcrypt_checkpw(RequestedPassword.c_str(), SavedPassword.data());
        isPasswordMatch_ = Id != InvalidId && result == 0;
    }

    bool IsPasswordMatch() const
    {
        return isPasswordMatch_;
    }

protected:
    bool isPasswordMatch_;
};

class RegisterJob final
{
public:
    const uint32_t Context;
    const std::string Id;
    const std::string OriginalPassword;

    explicit RegisterJob(const uint32_t context, std::string id, std::string originalPassword)
        : Context{ context },
          Id{ std::move(id) },
          OriginalPassword{ std::move(originalPassword) },
          hashedPassword_{}
    {
    }

    void ExecuteBackgroundJob()
    {
        std::array<char, 64> salt;
        CRASH_COND(bcrypt_gensalt(12, salt.data()));
        CRASH_COND(bcrypt_hashpw(OriginalPassword.c_str(), salt.data(), hashedPassword_.data()));
    }

    const char* GetHashedPassword() const
    {
        return hashedPassword_.data();
    }

private:
    std::array<char, 64> hashedPassword_;
};

using AuthJob = std::variant<LoginJob, RegisterJob>;

#endif //AUTHMESSAGE_H
