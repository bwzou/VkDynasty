#version 450

layout (location = 0) in vec2 v_texCoord;
layout (location = 1) in vec3 v_normalVector;
layout (location = 2) in vec3 v_worldPos;
layout (location = 3) in vec3 v_cameraDirection;
layout (location = 4) in vec3 v_lightDirection;
layout (location = 5) in vec4 v_shadowFragPos;

layout (location = 0) out vec4 FragColor;

layout (binding = 1, std140) uniform UniformsScene {
    vec3 u_ambientColor;
    vec3 u_cameraPosition;
    vec3 u_pointLightPosition;
    vec3 u_pointLightColor;
};

layout (binding = 2, std140) uniform UniformsMaterial {
    bool u_enableLight;
    bool u_enableIBL;
    bool u_enableShadow;

    float u_pointSize;
    float u_kSpecular;
    vec4 u_baseColor;
};

// layout (binding = 2) uniform sampler2D u_albedoMap;

// layout (binding = 3) uniform sampler2D u_normalMap;

// layout (binding = 4) uniform sampler2D u_emissiveMap;

// layout (binding = 5) uniform sampler2D u_metalRoughnessMap;

layout (binding = 3) uniform sampler2D u_shadowMap;

#define EPS 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586

// 传统的 two-pass 阴影算法 ----------------------------------------
float ShadowCalculation(vec4 fragPos, vec3 normal) {
    vec3 projCoords = fragPos.xyz / fragPos.w;
    float currentDepth = projCoords.z;

    float shadow = 0.0;
    float depth = texture(u_shadowMap, projCoords.xy).r;

    if (currentDepth > depth) {
        shadow = 1.0;
    } else {
        shadow = 0.0;
    }
    return shadow;
}


// PCSS 阴影算法 ---------------------------------------------------
// Shadow map related variables
#define NUM_SAMPLES 20
#define BLOCKER_SEARCH_NUM_SAMPLES NUM_SAMPLES
#define PCF_NUM_SAMPLES NUM_SAMPLES
#define NUM_RINGS 10
#define LIGHT_UV_SIZE 0.2 // 光源宽度
#define NEAR_PLANE 0.01 // 

vec2 poissonDisk[NUM_SAMPLES];
const float depthBiasCoeff = 0.00025;
const float depthBiasMin = 0.00005;

float rand_1to1(float x ) { 
  // -1 -1
  return fract(sin(x)*10000.0);
}

float rand_2to1(vec2 uv ) { 
  // 0 - 1
	float a = 12.9898, b = 78.233, c = 43758.5453;
	float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract(sin(sn) * c);
}

float unpack(vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
    return dot(rgbaDepth, bitShift) < 0.0001 ? 1.0 : dot(rgbaDepth, bitShift);
}

// 泊松圆盘
void poissonDiskSamples( const in vec2 randomSeed ) {

  float ANGLE_STEP = PI2 * float( NUM_RINGS ) / float( NUM_SAMPLES );
  float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES );

  float angle = rand_2to1( randomSeed ) * PI2;
  float radius = INV_NUM_SAMPLES;
  float radiusStep = radius;

  for( int i = 0; i < NUM_SAMPLES; i++ ) {
    poissonDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
    radius += radiusStep;
    angle += ANGLE_STEP;
  }
}

// 均匀采样
void uniformDiskSamples( const in vec2 randomSeed ) {

  float randNum = rand_2to1(randomSeed);
  float sampleX = rand_1to1( randNum ) ;
  float sampleY = rand_1to1( sampleX ) ;

  float angle = sampleX * PI2;
  float radius = sqrt(sampleY);

  for( int i = 0; i < NUM_SAMPLES; i ++ ) {
    poissonDisk[i] = vec2( radius * cos(angle) , radius * sin(angle)  );

    sampleX = rand_1to1( sampleY );
    sampleY = rand_1to1( sampleX );

    angle = sampleX * PI2;
    radius = sqrt(sampleY);
  }
}


float findBlocker(sampler2D shadowMap,  vec2 uv, float zReceiver) {
    float searchWidth = LIGHT_UV_SIZE * (zReceiver - NEAR_PLANE) / zReceiver ;
    float blockSum = 0.0;
    float blockNum = 0.0;
    for (int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; ++i) {
        vec2 sampleCoord = poissonDisk[i] * searchWidth + uv;
        float closetDepth = texture(shadowMap, sampleCoord).r;
        if (zReceiver > closetDepth + 0.001) {
            blockSum += closetDepth;
            blockNum += 1.0;
        }
    }

    return blockSum / blockNum;;
}


float PCF(sampler2D shadowMap, vec4 shadowCoord, float filterSize) {
    int sumBlock = 0;
    for (int i = 0; i < PCF_NUM_SAMPLES; ++i) {
        vec2 sampleCoord = filterSize * poissonDisk[i] + shadowCoord.xy;
        float closetDepth = texture(shadowMap, sampleCoord).r;
        // 计算可见性
        if (shadowCoord.z > closetDepth + 0.001) {
            sumBlock += 1;
        }
    }
    return  float(PCF_NUM_SAMPLES - sumBlock) / float(PCF_NUM_SAMPLES);
}

// https://yangwc.com/2021/04/14/PCSS/
// https://blog.csdn.net/n5/article/details/128955520
float PCSSCalculation(vec4 fragPos, vec3 normal) {
    vec3 projCoords = fragPos.xyz / fragPos.w;
    float currentDepth = projCoords.z;
    currentDepth = currentDepth * 0.5 + 0.5;

    if (currentDepth < 0.0 || currentDepth > 1.0) {
        return 0.0;
    }

    poissonDiskSamples(projCoords.xy);

    // step1: avgblocker depth
    float avgBlockerDepth = findBlocker(u_shadowMap, projCoords.xy, currentDepth);

    // step2: penumbra size
    float penumbraSize = max(currentDepth - avgBlockerDepth, 0.0) * LIGHT_UV_SIZE / avgBlockerDepth;

    // step3: filtering
    return PCF(u_shadowMap, vec4(projCoords.xy, currentDepth, 1.0), penumbraSize);
}


// VSM 阴影算法 ---------------------------------------------------
float VSMCalculation(vec4 fragPos, vec3 normal) {
    vec3 projCoords = fragPos.xyz / fragPos.w;
    float currentDepth = projCoords.z;
    

    float shadow = 0.0;
    float depth = texture(u_shadowMap, projCoords.xy).r;

    if (currentDepth > depth) {
        shadow = 1.0;
    } else {
        shadow = 0.0;
    }
    return shadow;
}

// CSM 阴影算法 --------------------------------------------------



void main() {
    // FragColor = u_baseColor;
    vec4 baseColor = u_baseColor;

    // L = La + Ld + Ls
    //   = ka * Ia + kd * (I / r^2) * max(0, N·L) + ks * (I / r^2) * max(0, N·H)
    // TODO 不考虑衰减
    
    // 计算环境光
    vec3 ambientColor = baseColor.rgb * u_ambientColor;

    // 计算漫反射系数 kd
    vec3 lightDirection = normalize(v_lightDirection);
    vec3 normalVector = normalize(v_normalVector);
    float lambertian = clamp(dot(lightDirection, normalVector), 0.0, 1.0);
    vec3 diffuseColor = baseColor.rgb * u_pointLightColor * lambertian;

    // 计算镜面反射系数 ks
    vec3 halfVector = normalize(v_lightDirection + v_cameraDirection);
    int specularExponent = 5;
    float specular = clamp(dot(halfVector, v_normalVector), 0.0, 1.0);
    vec3 specularColor = u_kSpecular * vec3(pow(specular, specularExponent));

    // float shadow = 1.0 - ShadowCalculation(v_shadowFragPos, normalVector);
    float shadow = PCSSCalculation(v_shadowFragPos, normalVector);
    diffuseColor *= shadow;
    specularColor *= shadow;

    // 计算每一个片元的输出颜色
    FragColor = vec4(ambientColor + diffuseColor + specularColor, baseColor.a);
    // FragColor = vec4(shadow, shadow, shadow, baseColor.a);
}

