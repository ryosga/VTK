// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#include "vtkShader.h"
#include "vtkObjectFactory.h"

#include "vtk_glew.h"

VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(vtkShader);

vtkShader::vtkShader()
{
  this->Dirty = true;
  this->Handle = 0;
  this->ShaderType = vtkShader::Unknown;
}

vtkShader::~vtkShader() = default;

void vtkShader::SetType(Type type)
{
  this->ShaderType = type;
  this->Dirty = true;
}

void vtkShader::SetSource(const std::string& source)
{
  this->Source = source;
  this->Dirty = true;
}

bool vtkShader::Compile()
{
  if (this->Source.empty() || this->ShaderType == Unknown || !this->Dirty)
  {
    return false;
  }

  // Ensure we delete the previous shader if necessary.
  if (this->Handle != 0)
  {
    glDeleteShader(static_cast<GLuint>(this->Handle));
    this->Handle = 0;
  }

  GLenum type = GL_VERTEX_SHADER;
  switch (this->ShaderType)
  {
#ifdef GL_GEOMETRY_SHADER
    case vtkShader::Geometry:
      type = GL_GEOMETRY_SHADER;
      break;
#endif
#ifdef GL_COMPUTE_SHADER
    case vtkShader::Compute:
      type = GL_COMPUTE_SHADER;
      break;
#endif
    case vtkShader::Fragment:
      type = GL_FRAGMENT_SHADER;
      break;
    case vtkShader::Vertex:
    case vtkShader::Unknown:
    default:
      type = GL_VERTEX_SHADER;
      break;
  }

  GLuint handle = glCreateShader(type);

  // Handle shader creation failures.
  if (handle == 0)
  {
    this->Error = "Could not create shader object.";
    return false;
  }

  const GLchar* source = static_cast<const GLchar*>(this->Source.c_str());
  glShaderSource(handle, 1, &source, nullptr);
  glCompileShader(handle);
  GLint isCompiled;
  glGetShaderiv(handle, GL_COMPILE_STATUS, &isCompiled);

  // Handle shader compilation failures.
  if (!isCompiled)
  {
    GLint length(0);
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &length);
    if (length > 1)
    {
      char* logMessage = new char[length];
      glGetShaderInfoLog(handle, length, nullptr, logMessage);
      this->Error = logMessage;
      delete[] logMessage;
    }
    glDeleteShader(handle);
    return false;
  }

  // The shader compiled, store its handle and return success.
  this->Handle = static_cast<int>(handle);
  this->Dirty = false;

  return true;
}

void vtkShader::Cleanup()
{
  if (this->ShaderType == Unknown || this->Handle == 0)
  {
    return;
  }

  glDeleteShader(static_cast<GLuint>(this->Handle));
  this->Handle = 0;
  this->Dirty = true;
}

//------------------------------------------------------------------------------
bool vtkShader::IsComputeShaderSupported()
{
#if defined(GL_ES_VERSION_3_0) || defined(GL_ES_VERSION_2_0)
  return false;
#else
  return glewIsSupported("GL_ARB_compute_shader") != 0;
#endif
}

//------------------------------------------------------------------------------
void vtkShader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
VTK_ABI_NAMESPACE_END
