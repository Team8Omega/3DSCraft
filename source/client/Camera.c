#include "client/Camera.h"

#include "world/chunk/Chunk.h"

#include "client/gui/DebugUI.h"
#include "client/player/InputData.h"

Camera gCamera;

#define CAM_Z_OFFSET 3.6f

#define CAM_Y_OFFSET 0.3f

void Camera_Init() {
	Mtx_Identity(&gCamera.view);

	gCamera.fov	 = C3D_AngleFromDegrees(60.f);
	gCamera.near = 0.2f, gCamera.far = 8.f * CHUNK_SIZE;

	gCamera.mode = CameraMode_First;

	Mtx_PerspTilt(&gCamera.projection, gCamera.fov, ((400.f) / (240.f)), gCamera.near, gCamera.far, false);
}

void Camera_Update(float iod) {
	float fov = gCamera.fov + C3D_AngleFromDegrees(12.f) * gPlayer.fovAdd;

	float3 playerHead = f3_new(gPlayer.position.x, gPlayer.position.y + PLAYER_EYEHEIGHT + sinf(gPlayer.bobbing) * 0.1f + gPlayer.crouchAdd,
							   gPlayer.position.z);

	Mtx_Identity(&gCamera.view);

	float3 forward = gPlayer.view;
	float3 right   = f3_crs(f3_new(0, 1, 0), forward);
	float3 up	   = f3_crs(forward, right);

	Mtx_RotateX(&gCamera.view, -gPlayer.pitch, true);
	Mtx_RotateY(&gCamera.view, -gPlayer.yaw, true);

	switch (gCamera.mode) {
		case CameraMode_First:
			Mtx_Translate(&gCamera.view, -playerHead.x, -playerHead.y, -playerHead.z, true);
			break;

		case CameraMode_Second: {
			float3 cameraPosition = f3_sub(playerHead, f3_scl(forward, -CAM_Z_OFFSET));
			cameraPosition.y -= CAM_Y_OFFSET;

			Mtx_Translate(&gCamera.view, -cameraPosition.x, -cameraPosition.y, -cameraPosition.z, true);
			Mtx_RotateY(&gCamera.view, M_PI, true);
		} break;

		case CameraMode_Third: {
			float3 cameraPosition = f3_sub(playerHead, f3_scl(forward, CAM_Z_OFFSET));
			cameraPosition.y -= CAM_Y_OFFSET;

			Mtx_Translate(&gCamera.view, -cameraPosition.x, -cameraPosition.y, -cameraPosition.z, true);
		} break;
		default:
			break;
	}

	Mtx_PerspStereoTilt(&gCamera.projection, fov, ((400.f) / (240.f)), gCamera.near, gCamera.far, iod, 1.f, false);

	C3D_Mtx vp;
	Mtx_Multiply(&vp, &gCamera.projection, &gCamera.view);
	Mtx_Copy(&gCamera.vp, &vp);

	// Frustum plane calculations remain the same
	C3D_FVec rowX = vp.r[0];
	C3D_FVec rowY = vp.r[1];
	C3D_FVec rowZ = vp.r[2];
	C3D_FVec rowW = vp.r[3];

	gCamera.frustumPlanes[Frustum_Near]	  = FVec4_Normalize(FVec4_Subtract(rowW, rowZ));
	gCamera.frustumPlanes[Frustum_Right]  = FVec4_Normalize(FVec4_Add(rowW, rowX));
	gCamera.frustumPlanes[Frustum_Left]	  = FVec4_Normalize(FVec4_Subtract(rowW, rowX));
	gCamera.frustumPlanes[Frustum_Top]	  = FVec4_Normalize(FVec4_Add(rowW, rowY));
	gCamera.frustumPlanes[Frustum_Bottom] = FVec4_Normalize(FVec4_Subtract(rowW, rowY));
	gCamera.frustumPlanes[Frustum_Far]	  = FVec4_Normalize(FVec4_Add(rowW, rowZ));

	float ar		  = 400.f / 240.f;
	float tan2halffov = 2.f * tanf(gCamera.fov / 2.f);

	float hNear = tan2halffov * gCamera.near;
	float wNear = hNear * ar;

	float hFar = tan2halffov * gCamera.far;
	float wFar = hFar * ar;

	float3 cNear = f3_add(playerHead, f3_scl(forward, gCamera.near));
	float3 cFar	 = f3_add(playerHead, f3_scl(forward, gCamera.far));

	gCamera.frustumCorners[Frustum_NearBottomLeft]	= f3_sub(f3_sub(cNear, f3_scl(up, hNear * 0.5f)), f3_scl(right, wNear * 0.5f));
	gCamera.frustumCorners[Frustum_NearBottomRight] = f3_add(f3_sub(cNear, f3_scl(up, hNear * 0.5f)), f3_scl(right, wNear * 0.5f));
	gCamera.frustumCorners[Frustum_NearTopLeft]		= f3_sub(f3_add(cNear, f3_scl(up, hNear * 0.5f)), f3_scl(right, wNear * 0.5f));
	gCamera.frustumCorners[Frustum_NearTopRight]	= f3_add(f3_add(cNear, f3_scl(up, hNear * 0.5f)), f3_scl(right, wNear * 0.5f));
	gCamera.frustumCorners[Frustum_FarBottomLeft]	= f3_sub(f3_sub(cFar, f3_scl(up, hFar * 0.5f)), f3_scl(right, wFar * 0.5f));
	gCamera.frustumCorners[Frustum_FarBottomRight]	= f3_add(f3_sub(cFar, f3_scl(up, hFar * 0.5f)), f3_scl(right, wFar * 0.5f));
	gCamera.frustumCorners[Frustum_FarTopLeft]		= f3_sub(f3_add(cFar, f3_scl(up, hFar * 0.5f)), f3_scl(right, wFar * 0.5f));
	gCamera.frustumCorners[Frustum_FarTopRight]		= f3_add(f3_add(cFar, f3_scl(up, hFar * 0.5f)), f3_scl(right, wFar * 0.5f));
}

bool Camera_IsPointVisible(C3D_FVec point) {
	point.w = 1.f;
	for (int i = 0; i < FrustumPlanes_Count; i++)
		if (FVec4_Dot(point, gCamera.frustumPlanes[i]) < 0.f)
			return false;
	return true;
}

bool Camera_IsAABBVisible(C3D_FVec orgin, C3D_FVec size) {
	float3 min = f3_new(orgin.x, orgin.y, orgin.z);
	float3 max = f3_new(orgin.x + size.x, orgin.y + size.y, orgin.z + size.z);
	for (int i = 0; i < 6; i++) {
		int out = 0;
		out += ((FVec4_Dot(gCamera.frustumPlanes[i], FVec4_New(min.x, min.y, min.z, 1.0f)) < 0.0));
		out += ((FVec4_Dot(gCamera.frustumPlanes[i], FVec4_New(max.x, min.y, min.z, 1.0f)) < 0.0));
		out += ((FVec4_Dot(gCamera.frustumPlanes[i], FVec4_New(min.x, max.y, min.z, 1.0f)) < 0.0));
		out += ((FVec4_Dot(gCamera.frustumPlanes[i], FVec4_New(max.x, max.y, min.z, 1.0f)) < 0.0));
		out += ((FVec4_Dot(gCamera.frustumPlanes[i], FVec4_New(min.x, min.y, max.z, 1.0f)) < 0.0));
		out += ((FVec4_Dot(gCamera.frustumPlanes[i], FVec4_New(max.x, min.y, max.z, 1.0f)) < 0.0));
		out += ((FVec4_Dot(gCamera.frustumPlanes[i], FVec4_New(min.x, max.y, max.z, 1.0f)) < 0.0));
		out += ((FVec4_Dot(gCamera.frustumPlanes[i], FVec4_New(max.x, max.y, max.z, 1.0f)) < 0.0));
		if (out == 8)
			return false;
	}

	// http://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
	int out;
	out = 0;
	for (int i = 0; i < 8; i++)
		out += ((gCamera.frustumCorners[i].x > max.x));
	if (out == 8)
		return false;
	out = 0;
	for (int i = 0; i < 8; i++)
		out += ((gCamera.frustumCorners[i].x < min.x));
	if (out == 8)
		return false;
	out = 0;
	for (int i = 0; i < 8; i++)
		out += ((gCamera.frustumCorners[i].y > max.y));
	if (out == 8)
		return false;
	out = 0;
	for (int i = 0; i < 8; i++)
		out += ((gCamera.frustumCorners[i].y < min.y));
	if (out == 8)
		return false;
	out = 0;
	for (int i = 0; i < 8; i++)
		out += ((gCamera.frustumCorners[i].z > max.z));
	if (out == 8)
		return false;
	out = 0;
	for (int i = 0; i < 8; i++)
		out += ((gCamera.frustumCorners[i].z < min.z));
	if (out == 8)
		return false;

	return true;
}
