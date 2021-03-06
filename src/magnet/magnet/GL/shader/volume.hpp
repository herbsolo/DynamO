/*    dynamo:- Event driven molecular dynamics simulator 
 *    http://www.marcusbannerman.co.uk/dynamo
 *    Copyright (C) 2009  Marcus N Campbell Bannerman <m.bannerman@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 3 as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <magnet/GL/shader/detail/shader.hpp>

//Simple macro to convert a token to a string
#define STRINGIFY(A) #A

namespace magnet {
  namespace GL {
    namespace shader {
      /*! \brief A shader for ray-tracing cubic volumes.
       *
       * This shader will render a volume data set in one pass. For
       * more information on the method please see
       * https://www.marcusbannerman.co.uk/index.php/home/42-articles/97-vol-render-optimizations.html
       */
      class VolumeShader: public detail::Shader
      {
      public:      
	virtual std::string initVertexShaderSource()
	{ return STRINGIFY(void main() { gl_Position = ftransform(); }); }

	virtual std::string initFragmentShaderSource()
	{
	  return STRINGIFY( 
uniform float FocalLength;
uniform vec2 WindowSize;
uniform vec3 RayOrigin;
uniform sampler1D TransferTexture;
uniform sampler2D DepthTexture;
uniform sampler3D DataTexture;
uniform float NearDist;
uniform float FarDist;
uniform float StepSize;
uniform float DiffusiveLighting;
uniform float SpecularLighting;
uniform float DitherRay;

//This function converts a value in a depth buffer back into a object-space distance
float recalcZCoord(float zoverw)
{
  return (2.0 * NearDist * FarDist) 
    / (FarDist + NearDist - (2.0 * zoverw - 1.0) * (FarDist - NearDist));
}

void main()
{
  //Calculate the ray direction using viewport information
  vec3 rayDirection;
  rayDirection.x = 2.0 * gl_FragCoord.x / WindowSize.x - 1.0;
  rayDirection.y = 2.0 * gl_FragCoord.y / WindowSize.y - 1.0;
  rayDirection.y *= WindowSize.y / WindowSize.x;
  rayDirection.z = -FocalLength;
  rayDirection = (vec4(rayDirection, 0.0) * gl_ModelViewMatrix).xyz;
  rayDirection = normalize(rayDirection);

  //Cube ray intersection test
  vec3 invR = 1.0 / rayDirection;
  vec3 boxMin = vec3(-1.0,-1.0,-1.0);
  vec3 boxMax = vec3( 1.0, 1.0, 1.0);
  vec3 tbot = invR * (boxMin - RayOrigin);
  vec3 ttop = invR * (boxMax - RayOrigin);
  
  //Now sort all elements of tbot and ttop to find the two min and max elements
  vec3 tmin = min(ttop, tbot); //Closest planes
  vec2 t = max(tmin.xx, tmin.yz); //Out of the closest planes, find the last to be entered (collision point)
  float tnear = max(t.x, t.y);//...

  //If the viewpoint is penetrating the volume, make sure to only cast the ray
  //from the eye position, not behind it
  if (tnear < 0.0) tnear = 0.0;

  //Now work out when the ray will leave the volume
  vec3 tmax = max(ttop, tbot); //Distant planes
  t = min(tmax.xx, tmax.yz);//Find the first plane to be exited
  float tfar = min(t.x, t.y);//...

  //Check what the screen depth is to make sure we don't sample the
  //volume past any standard GL objects
  float depth = recalcZCoord(texture2D(DepthTexture, gl_FragCoord.xy / WindowSize.xy).r);
  if (tfar > depth) tfar = depth;
  
  //This value is used to ensure that changing the step size does not
  //change the visualization as the alphas are renormalized using it.
  //For more information see the loop below where it is used
  const float baseStepSize = 0.01;

  //We need to calculate the ray's starting position. We add a random
  //fraction of the stepsize to the original starting point to dither
  //the output
  float random = DitherRay * fract(sin(gl_FragCoord.x * 12.9898 + gl_FragCoord.y * 78.233) * 43758.5453); 
  vec3 rayPos = RayOrigin + rayDirection * (tnear + StepSize * random);

  //The color accumulation variable
  vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
  
  //Some lighting information, we perform all the calculations in
  //model space, so we have to undo the modelview matrix
  //multiplication
  vec3 lightPos = (gl_ModelViewMatrixInverse * gl_LightSource[0].position).xyz;
  vec3 Specular = (gl_FrontMaterial.specular * gl_LightSource[0].specular).xyz;

  //We store the last valid normal, incase we hit a homogeneous region
  //and need to reuse it, but at the start we have no normal
  vec3 lastnorm = vec3(0,0,0); 

  for (float length = tfar - tnear; length > 0.0; 
       length -= StepSize, rayPos.xyz += rayDirection * StepSize)
    {
      //Grab the volume sample
      vec4 sample = texture3D(DataTexture, (rayPos + 1.0) * 0.5);

      //Sort out the normal data
      vec3 norm = sample.xyz * 2.0 - 1.0;
      //Test if we've got a bad normal and need to reuse the old one
      if (dot(norm,norm) < 0.5) norm = lastnorm; 
      //Store the current normal
      lastnorm = norm; 

      //Calculate the color of the voxel using the transfer function
      vec4 src = texture1D(TransferTexture, sample.a);
      
      //This corrects the transparency change caused by changing step
      //size. All alphas are defined for a certain base step size
      src.a = 1.0 - pow((1.0 - src.a), StepSize / baseStepSize);

      ////////////Lighting calculations
      vec3 lightDir = normalize(lightPos - rayPos);
      float lightNormDot = dot(normalize(norm), lightDir);
      
      ////////////Normal viewer
      //This allows you to visualize the normals of the system (albeit without direction)
//      src.rgb = vec3(abs(dot(norm, vec3(1.0,0.0,0.0))),
//		     abs(dot(norm, vec3(0.0,1.0,0.0))),
//		     abs(dot(norm, vec3(0.0,0.0,1.0))));

      //Diffuse lighting
      float diffTerm =  max(0.5 * lightNormDot + 0.5, 0.5);
      //Quadratic falloff of the diffusive term
      diffTerm *= diffTerm;

      //We either use diffusive lighting plus an ambient, or if its
      //disabled (DiffusiveLighting = 0), we just use the original
      //color.
      src.rgb *= DiffusiveLighting * (diffTerm + gl_LightModel.ambient) + (1.0 - DiffusiveLighting);
      
      //Specular lighting term
      //This is enabled if (SpecularLighting == 1)
      vec3 ReflectedRay = reflect(lightDir, norm);
      src.rgb += SpecularLighting
	* (lightNormDot > 0) //Test to ensure that specular is only
	//applied to front facing voxels
	* Specular * pow(max(dot(ReflectedRay, rayDirection), 0.0), 
			 gl_FrontMaterial.shininess);
      
      ///////////Front to back blending
      src.rgb *= src.a;
      color = (1.0 - color.a) * src + color;

      //We only accumulate up to 0.95 alpha (the front to back
      //blending never reaches 1). 
      if (color.a >= 0.95)
	{
	  //We have to renormalize the color by the alpha value (see
	  //below)
	  color.rgb /= color.a;
	  //Set the alpha to one to make sure the pixel is not transparent
	  color.a = 1.0; 
	  break;
	}
    }
  /*We must renormalize the color by the alpha value. For example, if
  our ray only hits just one white voxel with a alpha of 0.5, we will have

  src.rgb = vec4(1,1,1,0.5)

  src.rgb *= src.a; 
  //which gives, src.rgb = 0.5 * src.rgb = vec4(0.5,0.5,0.5,0.5)

  color = (1.0 - color.a) * src + color;
  //which gives, color = (1.0 - 0) * vec4(0.5,0.5,0.5,0.5) + vec4(0,0,0,0) = vec4(0.5,0.5,0.5,0.5)

  So the final color of the ray is half way between white and black, but the voxel it hit was white!
  The solution is to divide by the alpha, as this is the "amount of color" added to color.
  */
  color.rgb /= (color.a == 0.0) +  color.a;
  gl_FragColor = color;
});
	}
      };
    }
  }
}

#undef STRINGIFY
