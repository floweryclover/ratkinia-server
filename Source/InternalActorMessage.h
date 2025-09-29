//
// Created by floweryclover on 2025-10-25.
//

#ifndef INTERNALACTORMESSAGE_H
#define INTERNALACTORMESSAGE_H

#include <variant>

struct LoginMessage{};

using InternalActorMessage = std::variant<LoginMessage>;

#endif //INTERNALACTORMESSAGE_H
