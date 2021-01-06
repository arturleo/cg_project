#version 430 core
layout(binding = 0, rgba32f) uniform image2D framebufferImage;
layout(std430, binding = 1) buffer ln1
{
    vec4 vs[];
};
layout(std430, binding = 2) buffer ln2
{
    vec2 vts[];
};
layout(std430, binding = 3) buffer ln3
{
    vec4 vns[];
};
layout(std430, binding = 4) buffer ln4
{
    int fs[];
};
layout(std430, binding = 5) buffer ln5
{
    vec4 maxv[];
};
layout(std430, binding = 6) buffer ln6
{
    vec4 minv[];
};
layout(std430, binding = 7) buffer ln7
{
    vec4 ns[];
};

uniform bool useTexture;
uniform sampler2D textureSampler;

uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray01;
uniform vec3 ray10;
uniform vec3 ray11;

uniform float blendFactor;
uniform float time;
uniform int bounceCount;

uniform int vn;
uniform int fn;

#define MAX_SCENE_BOUNDS 100.0

#define EPSILON 0.00001
#define LIGHT_RADIUS 0.2
#define MATERIAL_AMBIENT_CO 0.2
#define LIGHT_BASE_INTENSITY 40.0

const vec3 lightCenterPosition = vec3(6, 8, -5);
const vec4 lightColor = vec4(1.0);

int p=5;
vec4 MaterialSpecularColor = vec4(0.3,0.3,0.3,1.0);

vec3 random3(vec3 f);
vec3 randomDiskPoint(vec3 rand, vec3 n);
vec3 randomHemispherePoint(vec3 rand, vec3 n);
vec3 randomCosineWeightedHemispherePoint(vec3 rand, vec3 n);
vec3 randomSpherePoint(vec3 rand);

struct Hitinfo {
  vec3 point;
  float near;
  int faceid;
};

vec4 getColor(vec2 uv)
{
    if(useTexture)
        // implement interporation outside
        return texture( textureSampler, uv );
    else 
        return vec4(0.5,0.5,1.0,1.0);
}

vec2 checkBoundingBox(vec3 origin, vec3 dir, int faceid) {
  vec3 tMin = (minv[faceid].xyz - origin) / dir;
  vec3 tMax = (maxv[faceid].xyz - origin) / dir;
  vec3 t1 = min(tMin, tMax);
  vec3 t2 = max(tMin, tMax);
  float tNear = max(max(t1.x, t1.y), t1.z);
  float tFar = min(min(t2.x, t2.y), t2.z);
  return vec2(tNear, tFar);
}

bool isSameDir(vec3 dir, int faceid, bool indir)
{
    if(dot(dir,ns[faceid].xyz)<0)
    {
        if (indir)
            return true;
        else
            return false;
    }
    if (indir)
        return false;
    else
        return true;
}

bool PointIn( vec3 P1, vec3 P2, vec3 A, vec3 B )
{
    vec3 CP1 = cross( B - A, P1 - A );
    vec3 CP2 = cross( B - A, P2 - A );
    return dot( CP1, CP2 ) >= 0;
}
bool PointInTriangle(vec3  P,vec3  A,vec3  B, vec3 C )
{
    return PointIn( P, A, B, C ) &&
           PointIn( P, B, C, A ) &&
           PointIn( P, C, A, B );
} 

bool intersect(vec3 origin, vec3 dir, int faceid, bool indir, out Hitinfo info)
{
    vec3 p0=vs[fs[faceid*3]].xyz;
    // we use interporated normal here
    vec3 normal=ns[faceid].xyz;
    if (!indir)
        normal=-normal;
    float t=abs(dot(p0-origin,normal)/dot(dir, ns[faceid].xyz));
    vec3 intersectPoint=origin+dir*t;
    if(PointInTriangle(intersectPoint,vs[fs[faceid*3]].xyz,vs[fs[faceid*3+1]].xyz,vs[fs[faceid*3+2]].xyz))
    {
        info.point=intersectPoint;
        info.near=t;
        info.faceid=faceid;
        return true;
    }
    return false;
}

bool intersectAll(vec3 origin, vec3 dir,  bool indir, out Hitinfo info)
{
    float smallest = MAX_SCENE_BOUNDS;
    bool found=false;
    for (int i = 0; i<fn; i++)
    {
        if(isSameDir(dir, i, indir))
        {
            vec2 lambda = checkBoundingBox(origin, dir, i);
            if (lambda.y >= 0.0 && lambda.x <= lambda.y && lambda.x < smallest) 
            {
              if (intersect( origin,  dir,  i,  indir, info))
              {
                smallest = info.near;
                found = true;
              }
            }
        }
    }
    return found;
}

vec3 calNormal(int faceID, vec3 poz)
{
    int id1=faceID*3,id2=faceID*3+1,id3=faceID*3+2;
    if(abs(vs[fs[id2]].x-vs[fs[id1]].x)<0.01)
    {
        int tmp=id1;
        id1=id3;
        id3=tmp;
    }

    if(abs(vs[fs[id2]].x-vs[fs[id1]].x)<0.01)
    {
        if(abs(vs[fs[id2]].z-vs[fs[id1]].z)<0.01)
        {
            int tmp=id1;
            id1=id3;
            id3=tmp;
        }
        else if(abs(vs[fs[id3]].z-vs[fs[id1]].z)<0.01)
        {
            int tmp=id1;
            id1=id2;
            id2=tmp;
        }
        vec3 ap=vs[fs[id1]].xyz,bp=vs[fs[id2]].xyz,cp=vs[fs[id3]].xyz;
        float alpha1=(poz.z-ap.z)/(bp.z-ap.z),alpha2=(poz.z-ap.z)/(cp.z-ap.z);
        vec3 p1=alpha1*bp+(1-alpha1)*ap, p2=alpha2*cp+(1-alpha2)*ap;
        float beta=(poz.y-p1.y)/(p2.y-p1.y);
    
        vec3 an=vns[fs[id1]].xyz,bn=vns[fs[id2]].xyz,cn=vns[fs[id3]].xyz;
        return beta*(alpha2*cn+(1-alpha2)*an)+(1-beta)*(alpha1*bn+(1-alpha1)*an);
    }
    else
    {
        if(abs(vs[fs[id2]].y-vs[fs[id1]].y)<0.01)
        {
            int tmp=id1;
            id1=id3;
            id3=tmp;
        }
        if(abs(vs[fs[id2]].y-vs[fs[id1]].y)<0.01)
        {
            if(abs(vs[fs[id2]].z-vs[fs[id1]].z)<0.01)
            {
                int tmp=id1;
                id1=id3;
                id3=tmp;
            }           
            else if(abs(vs[fs[id3]].z-vs[fs[id1]].z)<0.01)
            {
                int tmp=id1;
                id1=id2;
                id2=tmp;
            }
            vec3 ap=vs[fs[id1]].xyz,bp=vs[fs[id2]].xyz,cp=vs[fs[id3]].xyz;
            float alpha1=(poz.z-ap.z)/(bp.z-ap.z),alpha2=(poz.z-ap.z)/(cp.z-ap.z);
            vec3 p1=alpha1*bp+(1-alpha1)*ap, p2=alpha2*cp+(1-alpha2)*ap;
            float beta=(poz.x-p1.x)/(p2.x-p1.x);
    
            vec3 an=vns[fs[id1]].xyz,bn=vns[fs[id2]].xyz,cn=vns[fs[id3]].xyz;
            return beta*(alpha2*cn+(1-alpha2)*an)+(1-beta)*(alpha1*bn+(1-alpha1)*an);
        }
        if(abs(vs[fs[id2]].x-vs[fs[id1]].x)<0.01)
        {
            int tmp=id1;
            id1=id3;
            id3=tmp;
        }
        else if(abs(vs[fs[id3]].x-vs[fs[id1]].x)<0.01)
        {
            int tmp=id1;
            id1=id2;
            id2=tmp;
        }

        vec3 ap=vs[fs[id1]].xyz,bp=vs[fs[id2]].xyz,cp=vs[fs[id3]].xyz;
        float alpha1=(poz.x-ap.x)/(bp.x-ap.x),alpha2=(poz.x-ap.x)/(cp.x-ap.x);
        vec3 p1=alpha1*bp+(1-alpha1)*ap, p2=alpha2*cp+(1-alpha2)*ap;
        float beta=(poz.y-p1.y)/(p2.y-p1.y);
    
        vec3 an=vns[fs[id1]].xyz,bn=vns[fs[id2]].xyz,cn=vns[fs[id3]].xyz;
        return beta*(alpha2*cn+(1-alpha2)*an)+(1-beta)*(alpha1*bn+(1-alpha1)*an);
    }
}

vec2 calFace(int faceID, vec3 poz)
{
    int id1=faceID*3,id2=faceID*3+1,id3=faceID*3+2;
    if(abs(vs[fs[id2]].x-vs[fs[id1]].x)<0.01)
    {
        int tmp=id1;
        id1=id3;
        id3=tmp;
    }
    if(abs(vs[fs[id2]].x-vs[fs[id1]].x)<0.01)
    {
        // yz
        if(abs(vs[fs[id2]].z-vs[fs[id1]].z)<0.01)
        {
            int tmp=id1;
            id1=id3;
            id3=tmp;
        }
        else if(abs(vs[fs[id3]].z-vs[fs[id1]].z)<0.01)
        {
            int tmp=id1;
            id1=id2;
            id2=tmp;
        }
        vec3 ap=vs[fs[id1]].xyz,bp=vs[fs[id2]].xyz,cp=vs[fs[id3]].xyz;
        float alpha1=(poz.z-ap.z)/(bp.z-ap.z),alpha2=(poz.z-ap.z)/(cp.z-ap.z);
        vec3 p1=alpha1*bp+(1-alpha1)*ap, p2=alpha2*cp+(1-alpha2)*ap;
        float beta=(poz.y-p1.y)/(p2.y-p1.y);
    
        vec2 at=vts[fs[id1]],bt=vts[fs[id2]],ct=vts[fs[id3]];
        return beta*(alpha2*ct+(1-alpha2)*at)+(1-beta)*(alpha1*bt+(1-alpha1)*at);
    }
    else
    {   
         if(abs(vs[fs[id2]].y-vs[fs[id1]].y)<0.01)
        {
            int tmp=id1;
            id1=id3;
            id3=tmp;
        }
        if(abs(vs[fs[id2]].y-vs[fs[id1]].y)<0.01)
        {
            if(abs(vs[fs[id2]].z-vs[fs[id1]].z)<0.01)
            {
                int tmp=id1;
                id1=id3;
                id3=tmp;
            }
            else if(abs(vs[fs[id3]].z-vs[fs[id1]].z)<0.01)
            {
                int tmp=id1;
                id1=id2;
                id2=tmp;
            }
            // xz
            vec3 ap=vs[fs[id1]].xyz,bp=vs[fs[id2]].xyz,cp=vs[fs[id3]].xyz;
            float alpha1=(poz.z-ap.z)/(bp.z-ap.z),alpha2=(poz.z-ap.z)/(cp.z-ap.z);
            vec3 p1=alpha1*bp+(1-alpha1)*ap, p2=alpha2*cp+(1-alpha2)*ap;
            float beta=(poz.x-p1.x)/(p2.x-p1.x);
    
            vec2 at=vts[fs[id1]],bt=vts[fs[id2]],ct=vts[fs[id3]];
            return beta*(alpha2*ct+(1-alpha2)*at)+(1-beta)*(alpha1*bt+(1-alpha1)*at);
        }

        if(abs(vs[fs[id2]].x-vs[fs[id1]].x)<0.01)
        {
            int tmp=id1;
            id1=id3;
            id3=tmp;
        }
        else if(abs(vs[fs[id3]].x-vs[fs[id1]].x)<0.01)
        {
            int tmp=id1;
            id1=id2;
            id2=tmp;
        }
        // xy
        vec3 ap=vs[fs[id1]].xyz,bp=vs[fs[id2]].xyz,cp=vs[fs[id3]].xyz;
        float alpha1=(poz.x-ap.x)/(bp.x-ap.x),alpha2=(poz.x-ap.x)/(cp.x-ap.x);
        vec3 p1=alpha1*bp+(1-alpha1)*ap, p2=alpha2*cp+(1-alpha2)*ap;
        float beta=(poz.y-p1.y)/(p2.y-p1.y);

        vec2 at=vts[fs[id1]],bt=vts[fs[id2]],ct=vts[fs[id3]];
        return beta*(alpha2*ct+(1-alpha2)*at)+(1-beta)*(alpha1*bt+(1-alpha1)*at);
    }
}

/*
 * We need random values every now and then.
 * So, they will be precomputed for each ray we trace and
 * can be used by any function.
 */
vec3 rand;
vec3 cameraUp;

vec4 trace(vec3 origin, vec3 dir, int maxb) 
{
  Hitinfo i;
  float distS=1;
  int count=0;
  vec4 accumulated = vec4(0.0);
  vec4 attenuation = vec4(1.0);
  for (int bounce = 0; bounce < maxb; bounce++) {
    if (intersectAll(origin, dir, true, i)) {
      int fi=i.faceid;
      vec3 hitPoint = origin + i.near * dir;
      vec3 normal = calNormal(fi, hitPoint);//ns[fi].xyz;
      vec3 lightNormal = normalize(hitPoint - lightCenterPosition);
      vec3 lightPosition = lightCenterPosition + randomDiskPoint(rand, lightNormal) * LIGHT_RADIUS;
      vec4 cColor=getColor(calFace(fi, hitPoint));
      // ambient
      if(bounce != 0)
      {
        distS*=clamp(dot(hitPoint-origin,hitPoint-origin),1,1.0/EPSILON);
      }
      accumulated += attenuation *cColor* MATERIAL_AMBIENT_CO/pow(distS,0.5);
      vec3 shadowRayDir = lightPosition - hitPoint;
      vec3 shadowRayStart = hitPoint + normal * EPSILON;
      Hitinfo shadowRayInfo;
      bool lightObstructed = intersectAll(shadowRayStart, shadowRayDir, true, shadowRayInfo);
      // not obstructed by other objects
      if (shadowRayInfo.near >= 1.0) {
        count+=1;
        float cosineFallOff = clamp(dot(normal, normalize(shadowRayDir)),0,1);
        float oneOverR2 = 1.0 / pow(length(shadowRayDir),2)/distS;
	    vec3 R = reflect(normalize(shadowRayDir),normal);
	    float cosAlpha = clamp( dot( normalize(dir),R ), 0,1 );
        // diffuse
        accumulated += attenuation * cColor * lightColor * LIGHT_BASE_INTENSITY * cosineFallOff * oneOverR2;
        // reflection
        accumulated += attenuation * MaterialSpecularColor * lightColor * LIGHT_BASE_INTENSITY * pow(cosAlpha,p) * oneOverR2;
      }
      origin = shadowRayStart;
      dir = randomCosineWeightedHemispherePoint(rand, normal);
      // cone centered at reflect direction
      dir = normalize(reflect(normalize(-dir),normal)+dir*1.2);
      attenuation *= dot(normal, dir);
    } else {
      break;
    }
  }

  return accumulated;
}
layout (local_size_x = 16, local_size_y = 8) in;

void main(void) {
  ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
  ivec2 size = imageSize(framebufferImage);
  if (pix.x >= size.x || pix.y >= size.y) {
    return;
  }
  vec2 pos = (vec2(pix) + vec2(0.5, 0.5)) / vec2(size.x, size.y);
  vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
  cameraUp = normalize(ray01 - ray00);
  for (int s = 0; s < 1; s++) {
    rand = random3(vec3(pix, time + float(s)));
    vec2 jitter = rand.xy / vec2(size);
    vec2 p = pos + jitter;
    vec3 dir = mix(mix(ray00, ray01, p.y), mix(ray10, ray11, p.y), p.x);
    color += trace(eye, dir, bounceCount);
  }
  color /= 1.0;
  vec4 oldColor = vec4(0.0);
  if (blendFactor > 0.0) {
    oldColor = imageLoad(framebufferImage, pix);
  }
  vec4 finalColor = mix(color, oldColor, blendFactor);
  imageStore(framebufferImage, pix, finalColor);
}
