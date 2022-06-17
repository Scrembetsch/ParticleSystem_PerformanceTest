#version VERSION

precision mediump float;

DECL_TEX1 // Index

in vec2 vTexCoord;

uniform vec2 uResolution;
uniform uint uStageDistance;
uniform uint uStepDistance;

out vec3 oColor;

struct SortItem
{
  float SortVal;
  vec2 TexCoord;
};

SortItem Self;
SortItem Other;

uint Id;
vec2 Offset;

bool IsSmaller(SortItem item1, SortItem item2)
{
  return item1.SortVal < item2.SortVal;
}

void Swap(inout SortItem item1, inout SortItem item2)
{
  SortItem tmp = item1;
  item1 = item2;
  item2 = tmp;
}

void InitSortItem(inout SortItem item, uint id)
{
  vec2 vid;
  vid.x = float(id % uint(uResolution.x));
  vid.y = float(id / uint(uResolution.x));

  vec2 texCoord = (vid / uResolution) + Offset;
  item.TexCoord = texture(USE_TEX1, texCoord).rg;
  item.SortVal = texture(USE_TEX1, texCoord).b;
}

void main()
{
  Offset = (1.0 / uResolution) / 2.0;
  vec2 uId = (vTexCoord - Offset) * uResolution;

  Id = uint(floor(uId.x + uId.y * uResolution.x));

  uint stageLength = uStageDistance * 2;
  uint stepLength = uStepDistance * 2;

  uint stagePartId = Id / stageLength;
  uint stepPartId = (Id / stepLength) % (stageLength / stepLength);
  bool descending = stagePartId % 2 == 0;

  uint startFirstHalf = stagePartId * stageLength + stepPartId * stepLength;
  uint startSecondHalf = startFirstHalf + uStepDistance;
  bool isCompareValue = Id >= startSecondHalf;

  InitSortItem(Self, Id);
  InitSortItem(Other, (isCompareValue) ? (Id - uStepDistance) : (Id + uStepDistance));

  bool isSmaller = isCompareValue ? IsSmaller(Other, Self) : IsSmaller(Self, Other);

  bool swap = (descending && isSmaller) || (!descending && !isSmaller);

  vec2 vid;
  vid.x = float(Id % uint(uResolution.x));
  vid.y = float(Id / uint(uResolution.x));

  oColor.rg = swap ? Other.TexCoord : Self.TexCoord;
  oColor.b = swap ? Other.SortVal : Self.SortVal;
}