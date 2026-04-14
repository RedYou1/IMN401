in vec3 normale;
in vec2 textureCoords;
in vec3 positionMonde;
in vec3 gouraudColor;

layout(location = 0) out vec4 Color;

uniform bool useGouraud;

uniform sampler2D textureSampler;
uniform vec3 cameraPosition;
uniform float shininess;
uniform bool useBlinnPhong;

struct Light
{
	int type;
	vec3 couleurAmbiante;
    vec3 couleurDiffuse;
    vec3 couleurSpeculaire;
	float puissance;
	vec3 position;
    vec3 direction;
    float attenuation;
    float angleCone;
};
uniform int lightArraySize;
const int MAXLIGHTS = 8;
uniform Light lightArray[MAXLIGHTS];

vec3 calculerLambert(vec3 normaleNormalise, vec3 directionVersLumiere, Light light) {
    //Done: calculer la couleur diffuse
    float diff = max(dot(normaleNormalise, normalize(directionVersLumiere)), 0.0);
    return diff * light.couleurDiffuse * light.puissance;
}

float calculerPhong(vec3 directionVersLumiere, vec3 directionVersCamera, vec3 normale) {
    //Done: calculer le facteur d'intensite Phong
    vec3 reflectDir = reflect(-directionVersLumiere, normale);
    float spec = pow(max(dot(directionVersCamera, reflectDir), 0.0), shininess);
    return spec;
}

float calculerBlinnPhong(vec3 directionVersLumiere, vec3 directionVersCamera, vec3 normale) {
    //Done: calculer le facteur d'intensite Blinn-Phong
    vec3 halfwayDir = normalize(directionVersLumiere + directionVersCamera);
    float spec = pow(max(dot(normale, halfwayDir), 0.0), shininess);
    return spec;
}

float calculerAttenuationQuadratique(Light light, float distanceDeLumiere) {
    //Done calculer attenuation quadratique selon les notes de cours. Utiliser light.attenuation pour les trois coefficients d'attenuation.
    float kc = light.attenuation;
    float kl = light.attenuation;
    float kq = light.attenuation;

    return 1.0 / (kc + kl * distanceDeLumiere + kq * distanceDeLumiere * distanceDeLumiere);
}

float calculerAngleProjecteurIncident(vec3 surfaceVersLumiere, Light light) {
    //Done: calculer l'angle conique selon les notes de cours
    vec3 spotDir = normalize(-light.direction);
    float theta = dot(surfaceVersLumiere, spotDir);
    return acos(theta);
}

void main() {
    // If Gouraud mode is enabled, use the pre-calculated color from vertex shaders
    if (useGouraud) {
        Color = vec4(gouraudColor, 1.0);
        return;
    }
    vec4 couleurTexture = texture(textureSampler, textureCoords);
    const vec3 gamma = vec3(1.0/2.2);
    vec3 normaleNormalise = normalize(normale);

    Color = vec4(0.0, 0.0, 0.0, 1.0);

    for(int i = 0; i < lightArraySize; ++i)
	{
        vec3 couleurAmbiante = lightArray[i].couleurAmbiante * couleurTexture.xyz;
        vec3 surfaceVersLumiere = normalize(lightArray[i].position - positionMonde);
        vec3 surfaceVersCamera = normalize(cameraPosition - positionMonde);    

        float intensiteSpeculaire = 0.0;
        if(useBlinnPhong){
            intensiteSpeculaire = calculerBlinnPhong(surfaceVersLumiere, surfaceVersCamera, normaleNormalise);
        }else{ //Phong
            intensiteSpeculaire = calculerPhong(surfaceVersLumiere, surfaceVersCamera, normaleNormalise);
        }
        vec3 couleurSpeculaire = vec3(intensiteSpeculaire * lightArray[i].couleurSpeculaire);

        if(lightArray[i].type==0) //lumiere directionnelle
        {
            //Done: implementer la lumiere directionnelle en utilisant la fonction calculer Lambert et les bons parametres
            vec3 couleurDiffuse = calculerLambert(normaleNormalise,
                lightArray[i].type == 0 ? -lightArray[i].direction : surfaceVersLumiere,
                lightArray[i]);

            //Nous fournissions la combinaisons des couleurs
            vec3 finalColor = couleurAmbiante + (couleurDiffuse * couleurTexture.xyz) + couleurSpeculaire;
            Color += couleurTexture * vec4(finalColor, 1.0);
        }
        else //lumiere point ou projecteur
        {  
            float distanceDeLumiere = length(lightArray[i].position-positionMonde);
            float attenuationQuadratique = calculerAttenuationQuadratique(lightArray[i], distanceDeLumiere);

            float coneFactor = 1.0;
            if(lightArray[i].type==2) {
                float angleLumiereSurface = calculerAngleProjecteurIncident(surfaceVersLumiere, lightArray[i]);
                coneFactor = (angleLumiereSurface < lightArray[i].angleCone/2.0) ? 1.0 : 0.0;
            }

            //Done: implementer la lumiere directionnelle en utilisant la fonction calculer Lambert et les bons parametres
            vec3 couleurDiffuse = calculerLambert(normaleNormalise, surfaceVersLumiere, lightArray[i]);
            
            //Nous fournissions la combinaisons des couleurs
            vec3 couleurLineaire = couleurAmbiante + coneFactor * attenuationQuadratique*(couleurDiffuse * couleurTexture.xyz + couleurSpeculaire);
            Color.xyz += couleurLineaire;
        }
    }
    Color.xyz = pow(Color.xyz, gamma);
}