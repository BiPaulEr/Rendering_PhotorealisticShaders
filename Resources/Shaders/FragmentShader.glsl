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

in vec3 fPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fNormal;
in vec2 fTexCoord;

out vec4 colorResponse; // Shader output: the color response attached to this fragment

#define NB_LIGHTSOURCES 4
uniform LightSource lightSourcesArray[NB_LIGHTSOURCES];

uniform float renderingMode;

vec3 computeLightSourceRadiance(LightSource lightSource, vec3 n, vec3 wo)
{

	vec3 wi = normalize(lightSource.position - fPosition);
	float ndwi = clamp(dot(n, wi), 0, 1);
	float ndwo = clamp(dot(n, wo), 0, 1);

	float cosAngle = clamp(dot(normalize(-wi), normalize(lightSource.direction)), 0, 1); //angle between wi and the light direction

	if (cosAngle > cos(lightSource.coneAngle)) {
		vec3 wh = normalize(wi+wo);
		vec3 Li = lightSource.color * lightSource.intensity;
		float d = distance(lightSource.position, fPosition);
		Li = (1/(lightSource.a_c+lightSource.a_l*d+lightSource.a_q*d*d))*Li;

		//distribution GGX
		float alpha = texture(material.roughnessTex, fTexCoord).r; //roughness
		float D = pow(alpha, 2)/(M_PI*pow(1+dot(pow(alpha, 2)-1, pow(clamp(dot(n, wh), 0, 1), 2)), 2));

		//terme de Fresnel
		vec3 F_0 = vec3(texture(material.metallicTex, fTexCoord).r);
		vec3 F = F_0+(1-F_0)*pow((1-max(0, clamp(dot(wi, wh), 0, 1))), 5);

		//terme géométrique GGX
		float k = alpha*sqrt(2/M_PI);
		float G_wi = ndwi/(ndwi*(1-k)+k);
		float G_wo = ndwo/(ndwo*(1-k)+k);
		float G = G_wi*G_wo;

		vec3 fs = (D*F*G)/(4*ndwi*ndwo);

		return max(Li * fs*texture(material.albedoTex, fTexCoord).rgb * ndwi, 0); //équation du rendu
	}
	//intensity = 0 if the point is outside the light cone
	else {
		return vec3 (0.0, 0.0, 0.0);
	}
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
