#pragma once

#include "../glm/glm.hpp"

struct CsParticle
{
	glm::vec4 Position = glm::vec4(0.0f);
	glm::vec4 Velocity = glm::vec4(0.0f);
	glm::vec4 Color = glm::vec4(1.0f);
	glm::vec4 Lifetime = glm::vec4(0.0f);
};