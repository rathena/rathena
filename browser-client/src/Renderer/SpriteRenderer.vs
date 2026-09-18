#version 300 es
precision highp float;

in vec2 aPosition;
in vec2 aTextureCoord;

out vec2 vTextureCoord;

uniform mat4 uModelViewMat;
uniform mat4 uViewModelMat;
uniform mat4 uProjectionMat;

uniform float uCameraZoom;
uniform float uCameraLatitude;

uniform vec2 uSpriteRendererSize;
uniform vec2 uSpriteRendererOffset;
uniform mat4 uSpriteRendererAngle;
uniform vec3 uSpriteRendererPosition;
uniform float uSpriteRendererDepth;
uniform float uSpriteRendererZindex;
uniform bool  uDisableDepthCorrection;

mat4 Project( mat4 mat, vec3 pos) {

    // xyz = x(-z)y + middle of cell (0.5)
    float x =  pos.x + 0.5;
    float y = -pos.z;
    float z =  pos.y + 0.5;

    // Matrix translation
    mat[3].x += mat[0].x * x + mat[1].x * y + mat[2].x * z;
    mat[3].y += mat[0].y * x + mat[1].y * y + mat[2].y * z;
    mat[3].z += (mat[0].z * x + mat[1].z * y + mat[2].z * z);
    mat[3].w += mat[0].w * x + mat[1].w * y + mat[2].w * z;

    // Spherical billboard
    mat[0].xyz = vec3( 1.0, 0.0, 0.0 );
    mat[1].xyz = vec3( 0.0, 1.0, 0.0 );
    mat[2].xyz = vec3( 0.0, 0.0, 1.0 );

    return mat;
}

vec3 getCameraPosition() {
    return (uViewModelMat * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
}

vec3 getCameraForward() {
    return normalize((uViewModelMat * vec4(0.0, 0.0, -1.0, 0.0)).xyz);
}

void main(void) {
    // Calculate position base on angle and sprite offset/size
    vec4 position = uSpriteRendererAngle * vec4( aPosition.x * uSpriteRendererSize.x, aPosition.y * uSpriteRendererSize.y, 0.0, 1.0 );
    position.x   += uSpriteRendererOffset.x;
    position.y   -= uSpriteRendererOffset.y + 0.5;

    mat4 modelView = Project(uModelViewMat, uSpriteRendererPosition);
    vec4 viewPosition = modelView * position;
    vec4 viewCenter   = modelView * vec4( 0.0, 0.0, 0.0, 1.0 );

    gl_Position = uProjectionMat * viewPosition;

    vec3 cameraPos     = getCameraPosition();
    vec3 cameraForward = getCameraForward();

    if (!uDisableDepthCorrection) {
        // Vertical billboard depth correction (per-vertex), plane anchored at sprite center.
        // Plane normal uses camera forward (flattened Y) for stability.
        // The whole quad takes the vertical plane depth so the part of the sprite below
        // the water surface sorts behind the (later drawn) water pass.
        vec3 planePoint = (uViewModelMat * viewCenter).xyz;
        vec3 planeNormal = normalize(vec3(cameraForward.x, 0.0, cameraForward.z));
        if (length(planeNormal) < 0.000001) {
            planeNormal = cameraForward;
        }

        vec3 worldVertex = (uViewModelMat * viewPosition).xyz;
        vec3 rayDir      = normalize(worldVertex - cameraPos);
        float denom      = max(dot(planeNormal, rayDir), 0.000001);
        float dist       = dot(planePoint - cameraPos, planeNormal) / denom;

        vec4 planeClip       = uProjectionMat * (uModelViewMat * vec4(cameraPos + rayDir * dist, 1.0));
        float correctedZBase = planeClip.z * (gl_Position.w / max(planeClip.w, 0.000001));

        gl_Position.z = correctedZBase;
    }
    gl_Position.z -= (uSpriteRendererZindex * 0.01 + uSpriteRendererDepth) / max(uCameraZoom, 1.0);

    vTextureCoord = aTextureCoord;
}