#pragma once

#include <3ds.h>

typedef enum {
    TEXTURE_SQUARE_PNG,
    TEXTURE_CIRCLE_PNG,
} TextureFileName;

// Billion properties
typedef struct {
    float angle;
    float angleVariance;
    int blendFuncDestination;
    int blendFuncSource;
    float duration;
    int emitterType;
    float finishColorAlpha;
    float finishColorBlue;
    float finishColorGreen;
    float finishColorRed;
    float finishColorVarianceAlpha;
    float finishColorVarianceBlue;
    float finishColorVarianceGreen;
    float finishColorVarianceRed;
    float finishParticleSize;
    float finishParticleSizeVariance;
    float gravityx;
    float gravityy;
    float maxParticles;
    float maxRadius;
    float maxRadiusVariance;
    float minRadius;
    float particleLifespan;
    float particleLifespanVariance;
    float radialAccelVariance;
    float radialAcceleration;
    float rotatePerSecond;
    float rotatePerSecondVariance;
    float rotationEnd;
    float rotationEndVariance;
    float rotationStart;
    float rotationStartVariance;
    float sourcePositionVariancex;
    float sourcePositionVariancey;
    float sourcePositionx;
    float sourcePositiony;
    float speed;
    float speedVariance;
    float startColorAlpha;
    float startColorBlue;
    float startColorGreen;
    float startColorRed;
    float startColorVarianceAlpha;
    float startColorVarianceBlue;
    float startColorVarianceGreen;
    float startColorVarianceRed;
    float startParticleSize;
    float startParticleSizeVariance;
    float tangentialAccelVariance;
    float tangentialAcceleration;
    TextureFileName textureFileName;
} ParticleDefinition;

extern const ParticleDefinition drag_effect;
extern const ParticleDefinition ring_effect;
extern const ParticleDefinition portal_effect_01;
extern const ParticleDefinition bump_effect;