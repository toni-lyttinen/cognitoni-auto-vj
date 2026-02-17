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

    // --- SUBTLE SHIVER (Zero-Centered Fix) ---
    float shiverFactor = smoothstep(lowThresh, lowThresh + 0.1, mids);
    float sliceCount = 12.0;
    float sliceY = floor(uv.y / (res.y / sliceCount));
    // Subtracting 0.5 ensures jitter goes left and right equally to prevent drift
    float jitter = (random(vec2(sliceY, floor(time * 20.0))) - 0.5);
    uv.x += jitter * mids * 2.8 * shiverFactor;

    // --- DYNAMIC BLOCK SHIFT (Zero-Centered Fix) ---
    float shiftFactor = smoothstep(highThresh, highThresh + 0.1, mids);
    float blockCount = 6.0;
    float blockY = floor(texCoordVarying.y / (res.y / blockCount));
    float shiftSeed = random(vec2(blockY, floor(time * 15.0)));
    if (shiftSeed > 0.7) {
        // Center the shift range to prevent biased right-hand pulling
        uv.x += (shiftSeed - 0.85) * mids * 15.0 * shiftFactor;
    }

    // --- SUB-BASS: FLUID WAVE ---
    float waveAmount = subBass * 4.0; 
    uv.x += sin(uv.y * 0.005 + time) * waveAmount;

    // --- RGB SEPARATION ---
    float totalShift = rgbShift + (impactDelta * 80.0) + (subBass * 4.0);
    
    vec4 baseColor = vec4(
        texture(tex0, uv + vec2(totalShift, 0.0)).r,
        texture(tex0, uv).g,
        texture(tex0, uv + vec2(-totalShift, 0.0)).b,
        1.0
    );

    if(invertToggle) baseColor.rgb = 1.0 - baseColor.rgb;

    // --- INTERNAL SCANLINES ---
    float s = sin(texCoordVarying.y * 2.0) * 0.05;
    baseColor.rgb -= s;

    // --- DYNAMIC GEOMETRIC MOSH PATTERN (CALCULATED LAST) ---
    // This creates the shape overlay that dictates where the mosh distortion appears
    float shapeMask = 0.0;
    float burst = smoothstep(0.05, 0.4, impactDelta);
    float midActive = smoothstep(lowThresh, lowThresh + 0.1, mids) * (0.5 + burst * 0.5);
    
    vec2 center = res * 0.5;
    vec2 p = (texCoordVarying - center) / res.y;
    
    float warpIntensity = 0.05 + (subBass * 0.15);
    p.x += sin(p.y * 2.2 + time * 0.3) * warpIntensity;
    p.y += cos(p.x * 2.8 + time * 0.3) * warpIntensity;

    float rotAmt = (time * 0.1) + (mids * 2.2) + (impactDelta * 5.0);
    mat2 rot = mat2(cos(rotAmt), -sin(rotAmt), sin(rotAmt), cos(rotAmt));
    p = rot * p;
    
    float moshTime = floor(time * 8.0) / 8.0;
    float mode = mod(time * 0.06, 8.0);

    float d = 0.0;
    float freq = 3.0 + (mids * 7.0);
    float r = length(p);
    float a = atan(p.y, p.x);
    
    // Pattern Selection Logic
    if(mode < 1.0) {
        // STYLE 1: LIQUID SILK - Organic, wavy interference lines
        d = abs(sin(p.y * freq + time) * sin(p.x * freq - time));
    } else if(mode < 2.0) {
        // STYLE 2: THE VORTEX - A spiraling swirl that pulls pixels inward
        d = abs(sin(a * 6.0 + r * 12.0 - time * 2.5));
    } else if(mode < 3.0) {
        // STYLE 3: PLASMA BLOBS - Fluid, cloud-like organic blobs
        d = abs(sin(p.x * freq + time) + sin(p.y * freq + time) + sin((p.x + p.y) * freq));
    } else if(mode < 4.0) {
        // STYLE 4: KALEIDOSCOPE - 12-point rotational starburst symmetry
        d = abs(cos(a * 12.0) * sin(r * 15.0 - time));
    } else if(mode < 5.0) {
        // STYLE 5: THE TUNNEL - Concentric rings pulsing with a "zoom" feel
        d = abs(sin(log(max(r, 0.001)) * 8.0 - time * 3.0));
    } else if(mode < 6.0) {
        // STYLE 6: WAVES OF GRAIN - Sharp, high-frequency digital interference
        d = fract(sin(p.x * freq + p.y * freq) * 10.0 + time);
    } else if(mode < 7.0) {
        // STYLE 7: RINGS OF SATURN - Clean, steady concentric circular ripples
        d = abs(sin(r * 25.0 - time * 4.0));
    } else {
        // STYLE 8: GEOMETRIC FRACTAL - Boxy, folded space with sharp angles
        vec2 q = abs(p) - 0.2;
        d = abs(sin(max(q.x, q.y) * 20.0 + time));
    }

    d += random(p * moshTime) * treble * 0.2;
    float mask = smoothstep(2.2, 0.3, r);
    float intensity = smoothstep(lowThresh, highThresh, mids);
    float lineThickness = mix(0.12, 0.5, intensity * (0.5 + burst * 0.5)) + (subBass * 0.1);
    shapeMask = smoothstep(lineThickness, lineThickness - 0.08, d) * mask * midActive;

	// --- APPLY MOSH OVER PROCESSED IMAGE ---
    // Creates the final distortion based on the generated mask
    float moshDist = 100.0 + (burst * 150.0);
    
    // Calculate an offset if the pattern is actually active here
    vec2 organicOffset = vec2(cos(a + d), sin(a + d)) * moshDist * intensity;
    
    // Use the mask to determine where we sample the "mosh" texture from
    vec2 moshUv = mix(uv, uv + organicOffset, shapeMask);
    
    // Pixelation logic
    float currentBlockSize = mix(1.0, 20.0, intensity * (1.1 - treble));
    if(shapeMask > 0.01) {
        moshUv = floor(moshUv / currentBlockSize) * currentBlockSize;
    }
    
    moshUv = clamp(moshUv, vec2(0.5), res - 0.5);

    // Sample the color for the mosh area
    vec4 moshColor = texture(tex0, moshUv);
    
    // If the global invert is on, we flip the mosh color to keep it distinct
    if(invertToggle) moshColor.rgb = 1.0 - moshColor.rgb;

    // --- FINAL COMPOSITE WITH FORCED INVERSION ---
    // Instead of just mixing with moshColor, we mix with the inverse of the background
    // This guarantees the pattern is always a high-contrast negative of the video
    vec3 finalInvertedMosh = 1.0 - baseColor.rgb;
    
    // Blend the original processed image with its own negative based on the pattern
    fragColor = vec4(mix(baseColor.rgb, finalInvertedMosh, shapeMask), 1.0);
}