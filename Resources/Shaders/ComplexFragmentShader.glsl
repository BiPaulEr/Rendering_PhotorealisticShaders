#version 450 core // Minimal GL version support expected from the GPU
# define M_PI           3.14159265358979323846f  /* pi */

struct LightSource {
	vec3 position;
	vec3 color;
	float intensity;
	float a_c;
	float a_l;
	float a_q;
	float coneAngle;
	vec3 direction;
};

struct Material {
	sampler2D albedoTex;
	sampler2D roughnessTex;
    sampler2D metallicTex;
	sampler2D ambientTex;
	sampler2D toonTex;
};

uniform Material material;

uniform float zMin;
uniform float zMax;

uniform bool microFacet;		//Blinn-Phong BRDF / micro facet BRDF
uniform bool ggx;					//Cook-Torrance micro facet BRDF / GGX micro facet BRDF
uniform bool schlick;			//Approximation of Schlick for GGX micro facet BRDF

in vec3 fPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fNormal;
in vec2 fTexCoord;

out vec4 colorResponse; // Shader output: the color response attached to this fragment

#define NB_LIGHTSOURCES 4
uniform LightSource lightSourcesArray[NB_LIGHTSOURCES];

uniform float renderingMode;
float alpha =  texture(material.roughnessTex, fTexCoord).r;  // roughness
float F0 = (texture(material.metallicTex, fTexCoord).r+texture(material.metallicTex, fTexCoord).g+texture(material.metallicTex, fTexCoord).b )/3; //Fresnel refraction index, dependent on material
float ks = F0;							//coefficient specular
float kd = M_PI;           //coefficient diffusion
float fd = kd / M_PI; 		//Lambert BRDF (diffusion); 	

float G1Schlick(vec3 w, vec3 n){
	float k = alpha * sqrt(2.0 / M_PI);
	return float(dot(n, w)) / float((dot(n, w) * (1.0 - k) + k));
}

float G1Smith(vec3 w, float alpha2, vec3 n){
	return float(2.0 * dot(n, w)) / float((dot(n, w) + sqrt(alpha2 + (1.0 - alpha2) * pow(dot(n, w), 2.0))));
}

float microFacetFs(vec3 n, vec3 wi, vec3 wo, vec3 wh){
	float nwh2 = pow(dot(n, wh), 2.0);
	float F = F0 + (1.0 - F0) * pow(1.0 - max(0.0, dot(wi, wh)), 5.0);
	if(ggx == false){
		//Cook-Torrance micro facet mode
		float alpha2 = pow(alpha, 2.0);
		float D = exp( (nwh2 - 1.0) / (alpha2 * nwh2) ) / (pow(nwh2, 2.0) * alpha2 * M_PI);

		float shading = 2.0 * (dot(n, wh) * dot(n, wi)) / dot(wo, wh);
		float masking = 2.0 * (dot(n, wh) * dot(n, wo)) / dot(wo, wh);
		float G = min(1.0, min(shading, masking));

		return (D * F * G) / (4.0 * dot(n, wi) * dot(n, wo));
	}
  else{
		//GGX micro facet mode
		float alpha2 = pow(alpha, 2.0);
		float D = alpha2 / (M_PI * pow(1.0 + (alpha2 - 1.0) * nwh2, 2.0));
		float G;
		if(schlick == false) 	G = G1Smith(wi, alpha2, n) * G1Smith(wo, alpha2, n);									//approximation of Smith
		else 								 	G = G1Schlick(wi, n) * G1Schlick(wo, n); 							//approximation of Schlick

		return (D * F * G) / (4.0 * dot(n, wi) * dot(n, wo));
	}
}

vec3 computeLightSourceRadiance(LightSource lightSource, vec3 n, vec3 wo)
{

	vec3 wi = normalize(lightSource.position - fPosition); //wi
	vec3 wh = normalize(wi+wo); //wh
	float d = distance(lightSource.position, fPosition); //d
	float attenuation = 1/ (lightSource.a_c+lightSource.a_l*d+lightSource.a_q*d*d); //attenuation
	vec3 Li = lightSource.color * lightSource.intensity; //Color Light
	float ndwi = clamp(dot(n, wi), 0, 1);
	float ndwo = clamp(dot(n, wo), 0, 1);

	float fs;
	float cosAngle = clamp(dot(normalize(-wi), normalize(lightSource.direction)), 0, 1); //angle between wi and the light direction

	if (cosAngle > cos(lightSource.coneAngle)) 
	{
		if(microFacet == false) 
			{fs = ks * pow(dot(n, wh), texture(material.roughnessTex, fTexCoord).r);}		//Blinn-Phong BRDF (specular)
		else
			{fs = microFacetFs(n, wi, wo, wh);} //Cook-Torrance micro facet BRDF || GGX micro facet BRDF

		float f = fd + fs;
		return Li * f * max(dot(n, wi), 0.0) * attenuation * texture(material.albedoTex, fTexCoord).rgb;
	}
	else
		return vec3(0,0,0);
}

void main() {
	vec3 n = normalize (fNormal); // Linear barycentric interpolation does not preserve unit vectors
	vec3 wo = normalize (-fPosition);
	vec3 radiance = vec3 (0.0, 0.0, 0.0);

	if (renderingMode == 0.f) { //PBR mode
		for(int i = 0; i < NB_LIGHTSOURCES; i++) {
			radiance += computeLightSourceRadiance(lightSourcesArray[i], n, wo);
		}

		//ambient occlusion
		radiance = texture(material.ambientTex, fTexCoord).r * radiance;
	}
	else if (renderingMode == 1.f) { //TOON SHADING
		if (dot(n, wo) < 0.4) { //contour
			radiance = vec3 (0.0, 0.0, 0.0);
		}
		else if (dot(n, wo) > 0.9) { //specular spot
			radiance = vec3 (1.0, 1.0, 1.0);
		}
		else { //other fragment
			radiance = vec3 (0.0, 1.0, 0.0);
		}
	}
	else { //X-TOON SHADING
		float dValue = 1-log(-fPosition.z/zMin)/log(zMax/zMin);

		radiance = texture(material.toonTex, vec2(clamp(dot(n, wo), 0.01, 0.99), clamp(1-dValue, 0.01, 0.99))).rgb;
	}

	colorResponse = vec4 (radiance, 1.0); // Building an RGBA value from an RGB one.
}
