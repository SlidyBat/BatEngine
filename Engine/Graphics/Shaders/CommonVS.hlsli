#define MAX_BONES 256

cbuffer SkeletalAnimationData : register(b1)
{
	float4x4 BoneTransforms[MAX_BONES];
};

#define B_SLOT_TRANSFORM b0
#define B_SLOT_0 b2
#define B_SLOT_1 b3
#define B_SLOT_2 b4
#define B_SLOT_3 b5
#define B_SLOT_4 b6
#define B_SLOT_5 b7
#define B_SLOT_6 b8
#define B_SLOT_7 b9