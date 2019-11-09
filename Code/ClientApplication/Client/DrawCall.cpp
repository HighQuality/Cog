#include "ClientApplicationPch.h"
#include "DrawCall.h"
#include "Texture.h"
#include "VertexBuffer.h"

DrawCall::DrawCall() = default;
DrawCall::~DrawCall() = default;

DrawCall::DrawCall(DrawCall&&) = default;
DrawCall::DrawCall(const DrawCall&) = default;
DrawCall& DrawCall::operator=(DrawCall&&) = default;
DrawCall& DrawCall::operator=(const DrawCall&) = default;
