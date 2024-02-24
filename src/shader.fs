#version 330

in vec2 fragTexCoord;
in vec4 fragColor;


out vec4 finalColor;

void main()
{
    float r = 0.25;
    vec2 p = fragTexCoord - vec2(0.5);
    if(length(p) <= 0.5){
        float s = length(p)-r;
        if(s < 0.0){
            finalColor = fragColor;
        }
        else{
            float t = s / (0.5 - r);
            finalColor = vec4(1,1,1,t);
        }
    }
    else {
        finalColor = vec4(0);
    }
}


