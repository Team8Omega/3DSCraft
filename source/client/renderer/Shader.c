#include "client/renderer/Shader.h"

void Shader_Init(Shader* shader, const u8 shbin[], const u32 shbin_size, bool geometry) {
	shader->dvlb = DVLB_ParseFile((u32*)shbin, shbin_size);
	shaderProgramInit(&shader->program);
	shaderProgramSetVsh(&shader->program, &shader->dvlb->DVLE[0]);

	if (geometry)
		shaderProgramSetGsh(&shader->program, &shader->dvlb->DVLE[1], 0);

	shader->uLocProjection = shaderInstanceGetUniformLocation(shader->program.vertexShader, "projection");
}
void Shader_Deinit(Shader* shader) {
	shaderProgramFree(&shader->program);
	DVLB_Free(shader->dvlb);
}