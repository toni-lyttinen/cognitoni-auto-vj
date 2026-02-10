#version 150

uniform sampler2DRect tex0;
uniform float time;
uniform float subBass, lowMids, mids, highMids, treble;
uniform float pixelSize, rgbShift, impactDelta;
uniform float lowThresh, highThresh; 
uniform vec2 res;
uniform bool invertToggle;

in vec2 texCoordVarying;
out vec4 fragColor;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

void main(){
    vec2 uv = texCoordVarying;

    // --- 2. SUBTLE SHIVER ---
    if (mids > lowThresh) {
        float sliceCount = 12.0; 
        float sliceY = floor(uv.y / (res.y / sliceCount));
        float jitter = (random(vec2(sliceY, floor(time * 20.0))) - 0.5);
        uv.x += jitter * mids * 2.8; 
    }

    // --- 3. DYNAMIC BLOCK SHIFT ---
    if (mids > highThresh) {
        float blockCount = 6.0; 
        // FIXED: Changed vTex to texCoordVarying
        float blockY = floor(texCoordVarying.y / (res.y / blockCount));
        float shiftSeed = random(vec2(blockY, floor(time * 15.0))); 
        if (shiftSeed > 0.7) {
            uv.x += (shiftSeed - 0.5) * mids * 15.0; 
        }
    }

    // --- 4. DYNAMIC GEOMETRIC PATTERN ---
    float shapeMask = 0.0;
    if(mids > lowThresh) { 
        vec2 center = res * 0.5;
        vec2 p = (texCoordVarying - center) / res.y;
        p *= 2.2; 
        
        float rotAmt = (time * 0.4) + (mids * 2.5); 
        mat2 rot = mat2(cos(rotAmt), -sin(rotAmt), sin(rotAmt), cos(rotAmt));
        p = rot * p;
        
        float shapeTime = time * 1.2;
        float freqX = 1.0 + (5.0 * mids); 
        float freqY = 1.0 + (4.0 * mids);
        
        float xShift = sin(p.y * freqX + shapeTime);
        float yShift = cos(p.x * freqY + shapeTime);
        float d = abs(xShift + yShift);
        float mask = smoothstep(1.1, 0.1, length(p));
        
        float intensity = smoothstep(lowThresh, highThresh, mids);
        uv.x += xShift * intensity * 8.0 * mask;
        uv.y += yShift * intensity * 8.0 * mask;
        
        shapeMask = smoothstep(mix(0.08, 0.3, intensity), 0.0, d) * mask;
    }

    // --- 5. SUB-BASS: FLUID WAVE ---
    float waveAmount = subBass * 4.0; 
    uv.x += sin(uv.y * 0.005 + time) * waveAmount;

    // --- 6. RGB SEPARATION ---
    float totalShift = rgbShift + (impactDelta * 80.0) + (subBass * 4.0);
    vec4 color = vec4(
        texture(tex0, uv + vec2(totalShift, 0.0)).r,
        texture(tex0, uv).g,
        texture(tex0, uv + vec2(-totalShift, 0.0)).b,
        1.0
    );

    if(invertToggle) color.rgb = 1.0 - color.rgb;

    // --- 8. FINAL GEOMETRIC OVERLAY ---
    if (mids > lowThresh) {
        float glow = smoothstep(lowThresh, highThresh, mids) * 1.5;
        color.rgb = mix(color.rgb, vec3(1.0), clamp(shapeMask * glow, 0.0, 1.0));
    }

    // --- 9. INTERNAL SCANLINES ---
    // FIXED: Changed vTex to texCoordVarying
    float s = sin(texCoordVarying.y * 2.0) * 0.05;
    color.rgb -= s;

    fragColor = color;
}