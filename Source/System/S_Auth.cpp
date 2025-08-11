//
// Created by floweryclover on 2025-08-11.
//

#include "S_Auth.h"
#include "Database.h"
#include "Environment.h"
#include "GlobalObjectManager.h"
#include "EventManager.h"
#include "G_Auth.h"
#include "StcProxy.h"
#include <pqxx/pqxx>

using namespace Database;
using namespace pqxx;

void S_Auth(const MutableEnvironment& environment)
{
    const auto g_auth = environment.GlobalObjectManager.Get<G_Auth>();

    for (const auto& event_sessionErased : environment.EventManager.Events<Event_SessionErased>())
    {
        g_auth->RemoveContext(event_sessionErased.Context);
    }

    while (const auto job = g_auth->TryPopFinishedBackgroundJob())
    {
        if (std::holds_alternative<LoginJob>(*job))
        {
            auto& loginJob = std::get<LoginJob>(*job);

            if (!loginJob.IsPasswordMatch())
            {
                environment.StcProxy.LoginResponse(loginJob.Context, RatkiniaProtocol::LoginResponse_Result_Failure);
                return;
            }

            const auto contextAddResult = g_auth->TryAddContext(loginJob.Context, loginJob.Id);

            if (contextAddResult == G_Auth::ContextAddResult::Success)
            {
                environment.StcProxy.LoginResponse(loginJob.Context, RatkiniaProtocol::LoginResponse_Result_Success);
            }
            else if (contextAddResult == G_Auth::ContextAddResult::IdAlreadyOnline)
            {
                environment.StcProxy.LoginResponse(loginJob.Context,
                                                   RatkiniaProtocol::LoginResponse_Result_DuplicateAccount);
            }
            else
            {
                environment.StcProxy.LoginResponse(loginJob.Context,
                                                   RatkiniaProtocol::LoginResponse_Result_DuplicateContext);
            }
        }
        else
        {
            auto& registerJob = std::get<RegisterJob>(*job);
            nontransaction sameIdSearchWork{ environment.DbConnection };
            if (const auto sameIdSearchResult = sameIdSearchWork.exec(Prepped_FindUserId, registerJob.Id);
                !sameIdSearchResult.empty())
            {
                environment.StcProxy.RegisterResponse(registerJob.Context, false, "중복된 ID입니다.");
                return;
            }
            work insertAccountWork{ environment.DbConnection };
            const auto insertAccountResult = insertAccountWork.exec(Prepped_InsertAccount,
                                                                    params{
                                                                        registerJob.Id, registerJob.GetHashedPassword()
                                                                    });
            insertAccountWork.commit();

            const bool isSuccessful = insertAccountResult.affected_rows() == 1;
            CRASH_COND(!isSuccessful);
            environment.StcProxy.RegisterResponse(registerJob.Context, true, "");
        }
    }
}
