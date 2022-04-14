#pragma once

#include "../glm/glm.hpp"

struct CsParticle
{
	glm::vec3 Position = glm::vec3(0.0f);
	glm::vec3 Velocity = glm::vec3(0.0f);
	glm::vec4 Color = glm::vec4(1.0f);

	glm::vec2 Lifetime = glm::vec2(0.0f);
};