#pragma once
#include "BSpline.h"
#include "BSurface.h"
#include "CISpline.h"
#include "Torus.h"
#include <boost/json.hpp>
#include <memory>
#include <unordered_map>

namespace app {
    namespace json = boost::json;

	class SerializationManager {
    public:
        json::value SerializeScene(const std::vector<std::unique_ptr<Object>>& sceneObjects);
        void DeserializeScene(const json::value& jsonDoc, std::vector<std::unique_ptr<Object>>& sceneObjects);

    private:
        enum class GeometryObjectType {
            Torus, Polyline, Spline, BSpline, CISpline, Surface, BSurface
        };

        const static std::unordered_map<std::string, GeometryObjectType> m_gmodGeometryLookup;
        const static std::unordered_map<std::string, GeometryObjectType> m_jsonGeometryLookup;

        bool Contains(int id, std::vector<std::unique_ptr<Object>>& sceneObjects);
        std::vector<Object*> ParseControlPoints(const json::value& controlPoints, const std::unordered_map<int, Object*>& scenePoints);

        json::value SerializePoint(const Point* point);
        Point* DeserializePoint(const json::value& jv);

        json::value SerializeGeometryObject(const Object* object);
        void DeserializeGeometryObject(const json::value& jv, const std::unordered_map<int, Object*>& scenePoints, std::vector<std::unique_ptr<Object>>& sceneObjects);

        json::value SerializeTorus(const Torus* torus);
        Torus* DeserializeTorus(const json::value& jv);

        json::value SerializeChain(const Polyline* polyline);
        Polyline* DeserializeChain(const json::value& jv, const std::unordered_map<int, Object*>& scenePoints);

        json::value SerializeBezierC0(const Spline* curve);
        Spline* DeserializeBezierC0(const json::value& jv, const std::unordered_map<int, Object*>& scenePoints);

        json::value SerializeBezierC2(const BSpline* curve);
        BSpline* DeserializeBezierC2(const json::value& jv, const std::unordered_map<int, Object*>& scenePoints);

        json::value SerializeInterpolatedC2(const CISpline* curve);
        CISpline* DeserializeInterpolatedC2(const json::value& jv, const std::unordered_map<int, Object*>& scenePoints);

        json::value SerializeBezierSurfaceC0(const Surface* surface);
        Surface* DeserializeBezierSurfaceC0(const json::value& jv, const std::unordered_map<int, Object*>& scenePoints);

        json::value SerializeBezierSurfaceC2(const BSurface* surface);
        BSurface* DeserializeBezierSurfaceC2(const json::value& jv, const std::unordered_map<int, Object*>& scenePoints);
	};
}