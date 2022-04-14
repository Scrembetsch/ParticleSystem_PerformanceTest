#version VERSION

precision mediump float;

// in vec2 vTexCoord;
out vec4 oColor;

void main()
{           
  // vec4 vTexColor = texture2D(uSampler, vTexCoord);
  // oColor = vTexColor;
  oColor = vec4(1.0, 0.0, 0.0, 1.0);
}