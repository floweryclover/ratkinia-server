//
// Created by floweryclover on 2025-08-11.
//

#include "S_Auth.h"

#include "Environment.h"
#include "GlobalObjectManager.h"
#include "EventManager.h"
#include "Event_SessionErased.h"
#include "Proxy.h"

constexpr const char* FailedReason[] =
{
    "",
    "중복된 아이디입니다.",
    "알 수 없는 에러가 발생하였습니다."
};

void S_Auth(const MutableEnvironment& environment)
{
    // auto& g_auth = environment.GlobalObjectManager.Get<G_Auth>();
    //
    // for (const auto& event_sessionErased : environment.EventManager.Events<Event_SessionErased>())
    // {
    //     g_auth.DeauthenticateByContext(event_sessionErased.Context);
    // }
    //
    // while (const auto job = g_auth.TryPopFinishedBackgroundJob())
    // {
    //     if (std::holds_alternative<LoginJob>(*job))
    //     {
    //         auto& loginJob = std::get<LoginJob>(*job);
    //
    //         if (!loginJob.IsPasswordMatch())
    //         {
    //             environment.Proxy.LoginResponse(loginJob.Context, RatkiniaProtocol::LoginResponse_LoginResult_Failure);
    //             return;
    //         }
    //
    //         const auto contextAddResult = g_auth.TryAuthenticate(loginJob.Context, loginJob.Id);
    //
    //         if (contextAddResult == G_Auth::AuthenticationResult::Success)
    //         {
    //             environment.Proxy.LoginResponse(loginJob.Context, RatkiniaProtocol::LoginResponse_LoginResult_Success);
    //         }
    //         else if (contextAddResult == G_Auth::AuthenticationResult::IdAlreadyOnline)
    //         {
    //             environment.Proxy.LoginResponse(loginJob.Context,
    //                                             RatkiniaProtocol::LoginResponse_LoginResult_DuplicateAccount);
    //         }
    //         else
    //         {
    //             environment.Proxy.LoginResponse(loginJob.Context,
    //                                             RatkiniaProtocol::LoginResponse_LoginResult_DuplicateContext);
    //         }
    //     }
    //     else
    //     {
    //         auto& registerJob = std::get<RegisterJob>(*job);
    //         const auto result = environment.DatabaseManager.Get<T_Accounts>().TryCreateAccount(
    //             registerJob.Id,
    //             registerJob.GetHashedPassword());
    //
    //         if (result != T_Accounts::CreateAccountResult::Success)
    //         {
    //             environment.Proxy.RegisterResponse(registerJob.Context, false, FailedReason[static_cast<int>(result)]);
    //             return;
    //         }
    //
    //         environment.Proxy.RegisterResponse(registerJob.Context, true, "");
    //     }
    // }
}
