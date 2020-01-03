#define B_SLOT_TRANSFORM b0
#define B_SLOT_BONES     b1
#define B_SLOT_PARTICLES b2
#define B_SLOT_0 b3
#define B_SLOT_1 b4
#define B_SLOT_2 b5
#define B_SLOT_3 b6
#define B_SLOT_4 b7
#define B_SLOT_5 b8
#define B_SLOT_6 b9

#define MAX_BONES 256

cbuffer SkeletalAnimationData : register(B_SLOT_BONES)
{
	float4x4 BoneTransforms[MAX_BONES];
};