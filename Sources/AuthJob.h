//
// Created by floweryclover on 2025-08-01.
//

#ifndef AUTHMESSAGE_H
#define AUTHMESSAGE_H

#include "Errors.h"
#include <string>
#include <bcrypt/bcrypt.h>
#include <array>

inline static const std::string EmptyHashedPassword{ "$2a$12$YeNGaZyLvcdFapdGyzzBiubEj.LkPp/F0ZX0NydtGAvr6zGX6Cne6" };

template<typename THandler>
class AuthJob
{
public:
    explicit AuthJob() = default;

    virtual ~AuthJob() = default;

    AuthJob(const AuthJob&) = delete;

    AuthJob& operator=(const AuthJob&) = delete;

    AuthJob(AuthJob&&) = default;

    AuthJob& operator=(AuthJob&&) = default;

    virtual void ExecuteBackgroundJob() = 0;

    virtual void ExecuteFinalForegroundJob(THandler& handler) = 0;
};

template<typename THandler>
class LoginJob final : public AuthJob<THandler>
{
public:
    explicit LoginJob(const uint32_t context,
                      const uint32_t id,
                      std::string requestedPassword,
                      const std::array<char, 64>& savedPassword)
        : context_{ context },
          id_{ std::move(id) },
          requestedPassword_{ std::move(requestedPassword) },
          savedPassword_{ savedPassword },
          isPasswordMatch_{ false }
    {
    }

    ~LoginJob() override = default;

    LoginJob(const LoginJob&) = delete;

    LoginJob& operator=(const LoginJob&) = delete;

    LoginJob(LoginJob&&) = default;

    LoginJob& operator=(LoginJob&&) = default;

    void ExecuteBackgroundJob() override
    {
        const int result = bcrypt_checkpw(requestedPassword_.c_str(), savedPassword_.data());
        isPasswordMatch_ = result == 0;
    }

    void ExecuteFinalForegroundJob(THandler& handler) override
    {
        handler.HandlePostLogin(context_, id_, isPasswordMatch_);
    }

protected:
    uint32_t context_;
    uint32_t id_;
    std::string requestedPassword_;
    std::array<char, 64> savedPassword_;
    bool isPasswordMatch_;
};

template<typename THandler>
class RegisterJob final : public AuthJob<THandler>
{
public:
    explicit RegisterJob(const uint32_t context, std::string id, std::string originalPassword)
        : context_{ context },
          id_{ std::move(id) },
          originalPassword_{ std::move(originalPassword) },
          hashedPassword_{}
    {
    }

    ~RegisterJob() override = default;

    RegisterJob(const RegisterJob&) = delete;

    RegisterJob& operator=(const RegisterJob&) = delete;

    RegisterJob(RegisterJob&&) = default;

    RegisterJob& operator=(RegisterJob&&) = default;

    void ExecuteBackgroundJob() override
    {
        std::array<char, 64> salt;
        CRASH_COND(bcrypt_gensalt(12, salt.data()));
        CRASH_COND(bcrypt_hashpw(originalPassword_.c_str(), salt.data(), hashedPassword_.data()));
    }

    void ExecuteFinalForegroundJob(THandler& handler) override
    {
        handler.HandlePostRegister(context_, id_, hashedPassword_);
    }

private:
    uint32_t context_;
    std::string id_;
    std::string originalPassword_;
    std::array<char, 64> hashedPassword_;
};

#endif //AUTHMESSAGE_H
