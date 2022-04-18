#pragma once

#include "../glm/glm.hpp"

struct TfParticle
{
    glm::vec4 Position = glm::vec4(0.0);
    glm::vec4 Velocity = glm::vec4(0.0);
    glm::vec4 Color = glm::vec4(1.0);

    glm::vec4 Data = glm::vec4(0.0);
};