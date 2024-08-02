#pragma once

#include <citro3d.h>

typedef struct {
	DVLB_s* dvlb;
	shaderProgram_s program;
	int uLocProjection;
	C3D_AttrInfo vertexAttribs;
} Shader;

void Shader_Init(Shader* shader, const u8 shbin[], const u32 shbin_size, bool geometry);
void Shader_Deinit(Shader* shader);

static inline void Shader_Bind(Shader* shader) {
	C3D_BindProgram(&shader->program);
	C3D_SetAttrInfo(&shader->vertexAttribs);
}