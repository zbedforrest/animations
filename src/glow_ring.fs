#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output color
out vec4 finalColor;

// Uniforms from the C code
uniform float u_time;
uniform vec2 u_resolution;

void main() {
    // --- Pulse Calculation ---
    // Frequency = 0.1 Hz, so Period = 10 seconds.
    // We use a cosine wave to oscillate between 0.7 and 1.0 over 10 seconds.
    float pulse = 0.7 + 0.3 * (0.5 * (1.0 + cos(u_time * 2.0 * 3.14159 / 10.0)));

    // --- Geometry and Glow ---
    // Normalize coordinates to be -1.0 to 1.0, with 0.0 at the center
    vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution.xy) / u_resolution.y;

    // Define ring properties
    float ringRadius = 0.6;
    float ringCoreThickness = 0.02; // The solid, bright part of the ring
    float glowFalloff = 0.25;       // The distance over which the glow fades

    // Calculate distance from the ring's centerline
    float dist = abs(length(uv) - ringRadius);

    // Calculate intensity: 1.0 in the core, then a smooth falloff.
    // This creates a more natural profile than the previous "band" method.
    float intensity = 1.0 - smoothstep(ringCoreThickness / 2.0, ringCoreThickness / 2.0 + glowFalloff, dist);

    // --- Color ---
    // Define the reddish-orange color
    vec3 color = vec3(1.0, 0.2, 0.0);

    // --- Final Output ---
    // The final color is the ring shape, multiplied by the color, the pulse, and the original vertex color.
    // We use the ring value as the alpha, so it can be blended over other content if needed.
    finalColor = vec4(color * intensity * pulse, intensity);
}
