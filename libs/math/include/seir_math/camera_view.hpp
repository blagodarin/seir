// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_math/mat.hpp>
#include <seir_math/plane.hpp>
#include <seir_math/quad.hpp>
#include <seir_math/ray.hpp>
#include <seir_math/size.hpp>

namespace seir
{
	//
	class CameraView
	{
	public:
		constexpr CameraView() noexcept = default;
		inline CameraView(const Size2D& viewportSize, const Vec3& position, const Euler& orientation, float verticalFov, float nearPlane) noexcept;

		//
		[[nodiscard]] inline Ray3D pixelRay(const Vec2&) const noexcept;

		//
		[[nodiscard]] inline std::optional<Vec3> pixelRayIntersection(const std::optional<Vec2>&, const Plane&) const noexcept;

		// Returns world space to viewport space transformation.
		[[nodiscard]] const Mat4& matrix() const noexcept { return _matrix; }

		//
		[[nodiscard]] inline Quad viewportProjection(const Plane&, const Vec3& origin) const noexcept;

	private:
		Size2D _viewportSize;
		Vec3 _position;
		Mat4 _matrix;
		Mat4 _inverse;
	};
}

seir::CameraView::CameraView(const Size2D& viewportSize, const Vec3& position, const Euler& orientation, float verticalFov, float nearPlane) noexcept
	: _viewportSize{ viewportSize }
	, _position{ position }
	, _matrix{ Mat4::projection3D(viewportSize.width / viewportSize.height, verticalFov, nearPlane) * Mat4::camera(_position, orientation) }
	, _inverse{ inverse(_matrix) }
{
}

seir::Ray3D seir::CameraView::pixelRay(const Vec2& pixel) const noexcept
{
	// Pixel coordinates should be in [0, D) range (where D is width or height).
	// We shift coordinates to the center of the pixel (by adding 0.5),
	// then normalize them from [0, D] to [-1, 1].
	const auto x = (2 * pixel.x + 1) / _viewportSize.width - 1;
	const auto y = (2 * pixel.y + 1) / _viewportSize.height - 1;
	return Ray3D::fromPoints(_position, _inverse * Vec3{ x, y, 1 });
}

std::optional<seir::Vec3> seir::CameraView::pixelRayIntersection(const std::optional<Vec2>& pixel, const Plane& plane) const noexcept
{
	if (pixel.has_value())
		return pixelRay(*pixel).intersection(plane);
	return {};
}

seir::Quad seir::CameraView::viewportProjection(const Plane& plane, const Vec3& origin) const noexcept
{
	const auto planeIntersection = [this, &plane](float x, float y) {
		return Ray3D::fromPoints(_position, _inverse * Vec3{ x, y, 1 }).intersection(plane);
	};

	if (const auto topLeft = planeIntersection(-1, -1)) [[likely]]
		if (const auto topRight = planeIntersection(1, -1)) [[likely]]
			if (const auto bottomLeft = planeIntersection(-1, 1)) [[likely]]
				if (const auto bottomRight = planeIntersection(1, 1)) [[likely]]
				{
					const auto originOnPlane = origin - plane.distanceTo(origin) * plane.normal();
					const auto up = normalize(*topLeft - *bottomLeft + *topRight - *bottomRight);
					const auto right = crossProduct(up, plane.normal());
					const auto inversePlaneMatrix = inverse(
						Mat4{
							right.x, up.x, plane.normal().x, originOnPlane.x,
							right.y, up.y, plane.normal().y, originOnPlane.y,
							right.z, up.z, plane.normal().z, originOnPlane.z,
							0, 0, 0, 1 });
					return {
						Vec2{ inversePlaneMatrix * *topLeft },
						Vec2{ inversePlaneMatrix * *topRight },
						Vec2{ inversePlaneMatrix * *bottomLeft },
						Vec2{ inversePlaneMatrix * *bottomRight },
					};
				}
	return {};
}
