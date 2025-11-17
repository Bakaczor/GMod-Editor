static const int numThreads = 16;

cbuffer MillingParams
{
   	// Group 1: 16 bytes
    float3 currPos;
    uint cutterType; // 0 = cylindrical, 1 = spherical

	// Group 2: 16 bytes  
    float3 nextPos;
    uint checkBothDirections; // 0 = false, 1 = true

	// Group 3: 16 bytes
    float tipCentreOffset;
    float cutterRadius;
    float cuttingHeight;
    float maxAngle; // in radians
	
	// Group 4: 16 bytes
    float3 centre;
    uint textureSizeX;

	// Group 5: 16 bytes
    float3 size;
    uint textureSizeY;
};

RWTexture2D<float> heightMap : register(u1);
RWBuffer<uint> errorBuffer : register(u2); // 0 = ok, 1 = using non-cutting part, 2 = bad angle

float3 texelToWorldPos(uint2 texel)
{
    return float3(
        (float(texel.y) * size.y / (textureSizeY - 1)) - size.y * 0.5 + centre.y,
        centre.z,
        (float(texel.x) * size.x / (textureSizeX - 1)) - size.x * 0.5 + centre.x
    );
}

float3 toolPos(float3 P, float3 A, float3 B)
{
    float3 AB = B - A;
    float3 AP = P - A;
    float t = saturate(dot(AP, AB) / dot(AB, AB));
    return A + t * AB;
}

bool isInCylindricalMillingArea(float3 worldPos, float2 middlePosXZ, float cylinderBaseHeight)
{
    return distance(worldPos.xz, middlePosXZ) <= cutterRadius && worldPos.y >= cylinderBaseHeight;
}

bool isWithinAngle()
{
    if (abs(currPos.y - nextPos.y) < 1e-7f)
    {
        return true;
    }
    float3 moveDir = normalize(nextPos - currPos);
    float3 vertical;
    if (nextPos.y > currPos.y)
    {
        vertical = float3(0, 1, 0);
        if (checkBothDirections == 0)
        {
            return true;
        }
    }
    else
    {
        vertical = float3(0, -1, 0);
        if (abs(currPos.x - nextPos.x) < 1e-7f && abs(currPos.z - nextPos.z) < 1e-7f)
        {
            return false;
        }
    }
    
    // cos(90deg - alpha) = sin(alpha)
    const float sinAlpha = dot(moveDir, vertical);
	// alpha <= maxAlpha <=> sin(alpha) <= sin(maxAlpha)
    return sinAlpha <= sin(maxAngle);
}

[numthreads(numThreads, numThreads, 1)]
void main(uint3 id : SV_DispatchThreadID)
{  
    uint2 texel = id.xy;
    if (texel.x >= textureSizeX || texel.y >= textureSizeY)
    {
        return;
    }
    
    if (id.x == 0 && id.y == 0)
    {
        InterlockedMax(errorBuffer[0], 0U);
    }
    GroupMemoryBarrierWithGroupSync();

    float3 worldPos = texelToWorldPos(texel);
    worldPos.y += heightMap[texel];

    float3 tipPos = toolPos(worldPos, currPos, nextPos);
    tipPos.y += tipCentreOffset;   
    
    bool isInMillingArea = false;
    float newHeight = worldPos.y;

    if (cutterType == 0) // cylindrical
    {
        if (isInCylindricalMillingArea(worldPos, tipPos.xz, tipPos.y))
        {
            isInMillingArea = true;     
            if (worldPos.y > tipPos.y + cuttingHeight)
            {
                // cutting with non-cutting part
                InterlockedMax(errorBuffer[0], 1U);
                return;
            }
            newHeight = tipPos.y;
        }
    }
    else // spherical
    {
        float3 cutterCentre = float3(tipPos.x, tipPos.y + cutterRadius, tipPos.z);
        if (isInCylindricalMillingArea(worldPos, cutterCentre.xz, cutterCentre.y) || distance(worldPos, cutterCentre) <= cutterRadius)
        {
            isInMillingArea = true;
            if (worldPos.y > tipPos.y + cuttingHeight)
            {
                // cutting with non-cutting part
                InterlockedMax(errorBuffer[0], 1U);
                return;
            }
            float xDiff = worldPos.x - cutterCentre.x;
            float zDiff = worldPos.z - cutterCentre.z;
            newHeight = cutterCentre.y - sqrt(cutterRadius * cutterRadius - xDiff * xDiff - zDiff * zDiff);
        }
    }
    
    if (!isInMillingArea || newHeight >= worldPos.y)
    {
        // not affected by milling
        return;
    }
    
    if (!isWithinAngle())
    {
        InterlockedMax(errorBuffer[0], 2U);
        return;
    }
            
    heightMap[texel] = newHeight;
}