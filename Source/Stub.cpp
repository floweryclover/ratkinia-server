// //
// // Created by floweryclover on 2025-05-05.
// //
//
// #include "Stub.h"
// #include "AuthJob.h"
// #include "Environment.h"
// #include "Proxy.h"
// #include "Component/ComponentManager.h"
// #include "EntityManager.h"
// #include "GlobalObject/GlobalObjectManager.h"
//
// #include "C_NameTag.h"
// #include "C_HumanLikeBody.h"
//
// #include "G_Possession.h"
// #include "G_PlayerCharacters.h"
//
// #include <regex>
//
// #include "T_Accounts.h"
// #include "T_PlayerCharacters.h"
//
// using namespace RatkiniaProtocol;
//
// Stub::Stub(MutableEnvironment& environment)
//     : environment_{ environment }
// {
// }
//
// void Stub::OnParseMessageFailed(const uint32_t context, CtsMessageType messageType)
// {
// }
//
// void Stub::OnUnknownMessageType(const uint32_t context, CtsMessageType messageType)
// {
// }
//
// void Stub::OnCreateCharacter(const uint32_t context, const std::string& name)
// {
//     // auto& d_playerCharacters = environment_.DatabaseManager.Get<T_PlayerCharacters>();
//     // const auto playerId = environment_.GlobalObjectManager.Get<G_Auth>().TryGetPlayerIdOfContext(context);
//     // if (!playerId)
//     // {
//     //     environment_.Proxy.CreateCharacterResponse(context, CreateCharacterResponse_CreateCharacterResult_UnknownError);
//     //     return;
//     // }
//     //
//     // const auto [createCharacterResult, characterId] = d_playerCharacters.TryCreatePlayerCharacter(*playerId, name);
//     // if (createCharacterResult == T_PlayerCharacters::CreateCharacterResult::DuplicateName)
//     // {
//     //     environment_.Proxy.CreateCharacterResponse(context, CreateCharacterResponse_CreateCharacterResult_DuplicateName);
//     //     return;
//     // }
//     // if (createCharacterResult == T_PlayerCharacters::CreateCharacterResult::InvalidFormat)
//     // {
//     //     environment_.Proxy.CreateCharacterResponse(context,
//     //                                                CreateCharacterResponse_CreateCharacterResult_InvalidNameLength);
//     //     return;
//     // }
//     //
//     // if (createCharacterResult == T_PlayerCharacters::CreateCharacterResult::UnknownError)
//     // {
//     //     environment_.Proxy.CreateCharacterResponse(context, CreateCharacterResponse_CreateCharacterResult_UnknownError);
//     //     return;
//     // }
//     //
//     // const auto entity = environment_.EntityManager.Create();
//     //
//     // environment_.Proxy.CreateCharacterResponse(context, CreateCharacterResponse_CreateCharacterResult_Success);
//     // environment_.GlobalObjectManager.Get<G_PlayerCharacters>().AddOwnership(*playerId, characterId, entity);
// }
//
// void Stub::OnLoadMyCharacters(const uint32_t context)
// {
//     // auto& d_playerCharacters = environment_.DatabaseManager.Get<T_PlayerCharacters>();
//     // auto& g_auth = environment_.GlobalObjectManager.Get<G_Auth>();
//     // const auto playerId = g_auth.TryGetPlayerIdOfContext(context);
//     // if (!playerId)
//     // {
//     //     environment_.Proxy.Notify(context, Notify_Type_Fatal, "로그인 세션이 유효하지 않습니다.");
//     //     return;
//     // }
//     //
//     // const auto characters = d_playerCharacters.TryGetPlayerCharactersByPlayerId(*playerId);
//     // if (!characters)
//     // {
//     //     return;
//     // }
//     //
//     // environment_.Proxy.SendMyCharacters(
//     //     context,
//     //     *characters,
//     //     [](const T_PlayerCharacters::PlayerCharacterRow* const playerCharacter, SendMyCharacters_Data& data)
//     //     {
//     //         data.set_id(playerCharacter->Id);
//     //         data.set_name(playerCharacter->Name);
//     //     });
// }
//
// void Stub::OnSelectCharacter(const uint32_t context, const uint64_t id)
// {
//     // auto& g_auth = environment_.GlobalObjectManager.Get<G_Auth>();
//     // const auto playerId = g_auth.TryGetPlayerIdOfContext(context);
//     // if (!playerId)
//     // {
//     //     environment_.Proxy.Notify(context, Notify_Type_Fatal, "로그인이 필요합니다.");
//     //     return;
//     // }
//     //
//     // auto& g_playerCharacters = environment_.GlobalObjectManager.Get<G_PlayerCharacters>();
//     // const auto entity = g_playerCharacters.GetEntityOf(*playerId, id);
//     // if (!entity)
//     // {
//     //     environment_.Proxy.Notify(context, Notify_Type_Fatal, "요청한 플레이어와 캐릭터에 해당하는 엔티티를 찾을 수 없습니다.");
//     //     return;
//     // }
//     //
//     // auto& g_possession = environment_.GlobalObjectManager.Get<G_Possession>();
//     // if (const auto possessionResult = g_possession.TryPossess(context, entity);
//     //     possessionResult != G_Possession::PosessionResult::Success)
//     // {
//     //     environment_.Proxy.Notify(context, Notify_Type_Fatal, "이미 접속 중입니다.");
//     //     return;
//     // }
//     //
//     // environment_.Proxy.OpenWorld(context);
//     //
//     // environment_.Proxy.SpawnEntity(
//     //     context,
//     //     environment_.EntityManager,
//     //     [entity](const Entity worldEntity, SpawnEntity_Data& data)
//     //     {
//     //         data.set_type(worldEntity == entity ? SpawnEntity_Type_MyCharacter : SpawnEntity_Type_Normal);
//     //         data.set_entity_id(worldEntity.GetId());
//     //     });
//     //
//     // environment_.Proxy.AttachComponent(
//     //     context,
//     //     environment_.ComponentManager.Components<C_HumanLikeBody>(),
//     //     [](const std::pair<Entity, C_HumanLikeBody>& pair, AttachComponent_Data& data)
//     //     {
//     //         data.set_target_entity(pair.first.GetId());
//     //         data.set_component_runtime_order(C_HumanLikeBody::GetRuntimeOrder());
//     //     });
//     // static int cnt;
//     // ++cnt;
//     //
//     // environment_.Proxy.UpdateComponent(
//     //     context,
//     //     environment_.ComponentManager.Components<C_HumanLikeBody>(),
//     //     [](const std::pair<Entity, C_HumanLikeBody>& pair, UpdateComponent_Data& data)
//     //     {
//     //         data.set_target_entity(pair.first.GetId());
//     //         data.mutable_component_variant()->mutable_human_like_body()->set_static_mesh_name(cnt % 2 ? "Normal" : "Cube");
//     //     });
// }
//
// void Stub::OnDummyRequest(const uint32_t context, const std::string& message)
// {
// }
