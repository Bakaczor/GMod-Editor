#include "Application.h"
#include "SerializationManager.h"

using namespace app;

const std::unordered_map<std::string, SerializationManager::GeometryObjectType> SerializationManager::m_gmodGeometryLookup = {
    { "Torus", GeometryObjectType::Torus },
    { "Polyline", GeometryObjectType::Polyline },
    { "Spline", GeometryObjectType::Spline },
    { "BSpline", GeometryObjectType::BSpline },
    { "CISpline", GeometryObjectType::CISpline },
    { "Surface", GeometryObjectType::Surface },
    { "BSurface", GeometryObjectType::BSurface }
};

const std::unordered_map<std::string, SerializationManager::GeometryObjectType> SerializationManager::m_jsonGeometryLookup = {
    { "torus", GeometryObjectType::Torus },
    { "chain", GeometryObjectType::Polyline },
    { "bezierC0", GeometryObjectType::Spline },
    { "bezierC2", GeometryObjectType::BSpline },
    { "interpolatedC2", GeometryObjectType::CISpline },
    { "bezierSurfaceC0", GeometryObjectType::Surface },
    { "bezierSurfaceC2", GeometryObjectType::BSurface }
};

json::value SerializationManager::SerializeScene(const std::vector<std::unique_ptr<Object>>& sceneObjects) {
    std::vector<Object*> points;
    std::vector<Object*> geometry;

    for (auto& obj : sceneObjects) {
        if (obj->type() == "Point") {
            points.push_back(obj.get());
        } else {
            if (m_gmodGeometryLookup.contains(obj->type())) {
                geometry.push_back(obj.get());
            }
        }
    }

    json::array pointsArray;
    for (const auto& point : points) {
        const Point* p = dynamic_cast<const Point*>(point);
        pointsArray.push_back(SerializePoint(p));
    }

    json::array geometryArray;
    for (const auto& geom : geometry) {
        geometryArray.push_back(SerializeGeometryObject(geom));
    }

    return {
        { "points", pointsArray },
        { "geometry", geometryArray }
    };
}

void SerializationManager::DeserializeScene(const json::value& jsonDoc, std::vector<std::unique_ptr<Object>>& sceneObjects) {
    std::unordered_map<int, Object*> scenePoints;
    if (jsonDoc.as_object().contains("points")) {
        for (const auto& jv : jsonDoc.at("points").as_array()) {
            if (Contains(jv.at("id").as_int64(), sceneObjects)) { continue; }
            auto ptr = std::unique_ptr<Point>(DeserializePoint(jv));
            scenePoints[ptr->id] = ptr.get();
            sceneObjects.push_back(std::move(ptr));
        }
    }

    if (jsonDoc.as_object().contains("geometry")) {
        for (const auto& jv : jsonDoc.at("geometry").as_array()) {
            DeserializeGeometryObject(jv, scenePoints, sceneObjects);
        }
    }
}

json::value SerializationManager::SerializePoint(const Point* point) {
    auto pos = point->position();
    return {
        { "id", point->id },
        { "name", point->name },
        { "position", {
            { "x", pos.x() },
            { "y", pos.y() },
            { "z", pos.z() }
        }}
    };
}

Point* SerializationManager::DeserializePoint(const json::value& jv) {
    Point* point = new Point(Application::m_pointModel.get());
    point->id = jv.at("id").as_int64();
    point->name = jv.at("name").as_string().c_str();

    auto pos = jv.at("position");
    point->SetTranslation(
        pos.at("x").as_double(),
        pos.at("y").as_double(),
        pos.at("z").as_double()
    );
    return point;
}

json::value SerializationManager::SerializeGeometryObject(const Object* object) {
    GeometryObjectType type = m_gmodGeometryLookup.at(object->type());
    switch (type) {
        case GeometryObjectType::Torus: {
            const Torus* obj = dynamic_cast<const Torus*>(object);
            return SerializeTorus(obj);
        }
        case GeometryObjectType::Polyline: {
            const Polyline* obj = dynamic_cast<const Polyline*>(object);
            return SerializeChain(obj);
        }
        case GeometryObjectType::Spline: {
            const Spline* obj = dynamic_cast<const Spline*>(object);
            return SerializeBezierC0(obj);
        }
        case GeometryObjectType::BSpline: {
            const BSpline* obj = dynamic_cast<const BSpline*>(object);
            return SerializeBezierC2(obj);
        }
        case GeometryObjectType::CISpline: {
            const CISpline* obj = dynamic_cast<const CISpline*>(object);
            return SerializeInterpolatedC2(obj);
        }
        case GeometryObjectType::Surface: {
            const Surface* obj = dynamic_cast<const Surface*>(object);
            return SerializeBezierSurfaceC0(obj);
        }
        case GeometryObjectType::BSurface: {
            const BSurface* obj = dynamic_cast<const BSurface*>(object);
            return SerializeBezierSurfaceC2(obj);
        }
    }
}

void SerializationManager::DeserializeGeometryObject(const json::value& jv,
    const std::unordered_map<int, Object*>& scenePoints, std::vector<std::unique_ptr<Object>>& sceneObjects) {
    const auto& obj = jv.as_object();
    if (Contains(obj.at("id").as_int64(), sceneObjects)) { return; }
    GeometryObjectType type = m_jsonGeometryLookup.at(obj.at("objectType").as_string().c_str());
    switch (type) {
        case GeometryObjectType::Torus: {
            auto ptr = std::unique_ptr<Torus>(DeserializeTorus(jv));
            sceneObjects.push_back(std::move(ptr));
            break;
        }
        case GeometryObjectType::Polyline: {
            auto ptr = std::unique_ptr<Polyline>(DeserializeChain(jv, scenePoints));
            sceneObjects.push_back(std::move(ptr));
            break;
        }
        case GeometryObjectType::Spline: {
            auto ptr = std::unique_ptr<Spline>(DeserializeBezierC0(jv, scenePoints));
            sceneObjects.push_back(std::move(ptr));
            break;
        }
        case GeometryObjectType::BSpline: {
            auto ptr = std::unique_ptr<BSpline>(DeserializeBezierC2(jv, scenePoints));
            sceneObjects.push_back(std::move(ptr));
            break;
        }
        case GeometryObjectType::CISpline: {
            auto ptr = std::unique_ptr<CISpline>(DeserializeInterpolatedC2(jv, scenePoints));
            sceneObjects.push_back(std::move(ptr));
            break;
        }
        case GeometryObjectType::Surface: {
            auto ptr = std::unique_ptr<Surface>(DeserializeBezierSurfaceC0(jv, scenePoints));
            sceneObjects.push_back(std::move(ptr));
            break;
        }
        case GeometryObjectType::BSurface: {
            auto ptr = std::unique_ptr<BSurface>(DeserializeBezierSurfaceC2(jv, scenePoints));
            sceneObjects.push_back(std::move(ptr));
            break;
        }
    }
}

json::value SerializationManager::SerializeTorus(const Torus* torus) {
    auto pos = torus->position();
    auto rot = torus->rotation();
    auto sca = torus->scale();

    return {
        { "objectType", "torus" },
        { "id", torus->id },
        { "name", torus->name },
        { "position", {
            { "x", pos.x() },
            { "y", pos.y() },
            { "z", pos.z() }
        }},
        { "rotation", {
            { "x", rot.x() },
            { "y", rot.y() },
            { "z", rot.z() },
            { "w", rot.w() }
        }},
        { "scale", {
            { "x", sca.x() },
            { "y", sca.y() },
            { "z", sca.z() }
        }},
        { "samples", {
            { "u", torus->Get_uParts() },
            { "v", torus->Get_vParts() }
        }},
        { "smallRadius", torus->Get_r() },
        { "largeRadius", torus->Get_R() }
    };
}

Torus* SerializationManager::DeserializeTorus(const json::value& jv) {
    Torus* torus = new Torus(
        jv.at("largeRadius").as_double(),
        jv.at("smallRadius").as_double(),
        jv.at("samples").at("u").as_int64(),
        jv.at("samples").at("v").as_int64()
    );

    torus->id = jv.at("id").as_int64();
    torus->name = jv.at("name").as_string().c_str();

    auto pos = jv.at("position");
    torus->SetTranslation(
        pos.at("x").as_double(),
        pos.at("y").as_double(),
        pos.at("z").as_double()
    );

    auto rot = jv.at("rotation");
    gmod::quaternion<double> q(
        rot.at("x").as_double(),
        rot.at("y").as_double(),
        rot.at("z").as_double(),
        rot.at("w").as_double()
    );
    auto eulerAngles = gmod::quaternion<double>::to_euler(q);
    torus->SetRotation(
        eulerAngles.x(),
        eulerAngles.y(),
        eulerAngles.z()
    );

    auto scale = jv.at("scale");
    torus->SetScaling(
        scale.at("x").as_double(),
        scale.at("y").as_double(),
        scale.at("z").as_double()
    );

    return torus;
}

json::value SerializationManager::SerializeChain(const Polyline* polyline) {
    json::array points;
    for (const auto& p : polyline->objects) {
        json::value jv = { { "id", p->id } };
        points.emplace_back(jv);
    }

    return {
        { "objectType", "chain" },
        { "id", polyline->id },
        { "name", polyline->name },
        { "controlPoints", points },
    };
}

app::Polyline* SerializationManager::DeserializeChain(const json::value& jv, const std::unordered_map<int, Object*>& scenePoints) {
    const auto& obj = jv.as_object();
    Polyline* polyline = new Polyline(ParseControlPoints(obj.at("controlPoints"), scenePoints));
    polyline->id = obj.at("id").as_int64();
    polyline->name = obj.at("name").as_string().c_str();
    return polyline;
}

json::value SerializationManager::SerializeBezierC0(const Spline* curve) {
    json::array points;
    for (const auto& p : curve->objects) {
        json::value jv = { { "id", p->id } };
        points.emplace_back(jv);
    }

    return {
        { "objectType", "bezierC0" },
        { "id", curve->id },
        { "name", curve->name },
        { "controlPoints", points },
    };
}

Spline* SerializationManager::DeserializeBezierC0(const json::value& jv, const std::unordered_map<int, Object*>& scenePoints) {
    const auto& obj = jv.as_object();
    Spline* curve = new Spline(ParseControlPoints(obj.at("controlPoints"), scenePoints));
    curve->id = obj.at("id").as_int64();
    curve->name = obj.at("name").as_string().c_str();
    return curve;
}

json::value SerializationManager::SerializeBezierC2(const BSpline* curve) {
    json::array points;
    for (const auto& p : curve->objects) {
        json::value jv = { { "id", p->id } };
        points.emplace_back(jv);
    }

    return {
        { "objectType", "bezierC2" },
        { "id", curve->id },
        { "name", curve->name },
        { "controlPoints", points },
    };
}

BSpline* SerializationManager::DeserializeBezierC2(const json::value& jv, const std::unordered_map<int, Object*>& scenePoints) {
    const auto& obj = jv.as_object();
    BSpline* curve = new BSpline(ParseControlPoints(obj.at("controlPoints"), scenePoints));
    curve->id = obj.at("id").as_int64();
    curve->name = obj.at("name").as_string().c_str();
    return curve;
}

json::value SerializationManager::SerializeInterpolatedC2(const CISpline* curve) {
    json::array points;
    for (const auto& p : curve->objects) {
        json::value jv = { { "id", p->id } };
        points.emplace_back(jv);
    }

    return {
        { "objectType", "interpolatedC2" },
        { "id", curve->id },
        { "name", curve->name },
        { "controlPoints", points },
    };
}

CISpline* SerializationManager::DeserializeInterpolatedC2(const json::value& jv, const std::unordered_map<int, Object*>& scenePoints) {
    const auto& obj = jv.as_object();
    CISpline* curve = new CISpline(ParseControlPoints(obj.at("controlPoints"), scenePoints));
    curve->id = obj.at("id").as_int64();
    curve->name = obj.at("name").as_string().c_str();
    return curve;
}

json::value SerializationManager::SerializeBezierSurfaceC0(const Surface* surface) {
    json::array points;
    const auto& controlPoints = surface->GetControlPoints();
    for (const auto& p : controlPoints) {
        json::value jv = { { "id", p->id } };
        points.emplace_back(jv);
    }
    if (surface->GetSurfaceType() == SurfaceType::Cylindric) {
        for (unsigned int i = 0; i < surface->GetBPoints(); ++i) {
            json::value jv = { { "id", controlPoints[i]->id }};
            points.emplace_back(jv);
        }
    }

    return {
        { "objectType", "bezierSurfaceC0" },
        { "id", surface->id },
        { "name", surface->name },
        { "controlPoints", points },
        { "size", {
            { "u", surface->GetBPoints() },
            { "v", surface->GetAPoints() }
        }},
        { "samples", {
            { "u", surface->GetDivisions() },
            { "v", surface->GetDivisions() }
        }}
    };
}

Surface* SerializationManager::DeserializeBezierSurfaceC0(const json::value& jv, const std::unordered_map<int, Object*>& scenePoints) {
    const auto& obj = jv.as_object();
    
    auto size = obj.at("size");
    auto samples = obj.at("samples");

    unsigned int aPoints = size.at("v").as_int64();
    unsigned int bPoints = size.at("u").as_int64();
    unsigned int divisions = (samples.at("u").as_int64() + samples.at("v").as_int64()) / 2;
    divisions = std::min(std::max(Surface::minDivisions, divisions), Surface::maxDivisions);

    std::vector<Object*> objects = ParseControlPoints(obj.at("controlPoints"), scenePoints);

    SurfaceType type = SurfaceType::Flat;
    bool cylindric = true;
    for (unsigned int i = 0; i < bPoints; ++i) {
        unsigned int lastRow = i + aPoints * bPoints;
        if (lastRow >= objects.size() || objects[i]->id != objects[lastRow]->id) {
            cylindric = false;
            break;
        }
    }

    if (cylindric) {
        type = SurfaceType::Cylindric;
    }

    Surface* surface = new Surface(type, aPoints, bPoints, divisions, objects);
    surface->id = obj.at("id").as_int64();
    surface->name = obj.at("name").as_string().c_str();

    return surface;
}

json::value SerializationManager::SerializeBezierSurfaceC2(const BSurface* surface) {
    json::array points;
    const auto& controlPoints = surface->GetControlPoints();
    for (const auto& p : controlPoints) {
        json::value jv = { { "id", p->id } };
        points.emplace_back(jv);
    }
    if (surface->GetSurfaceType() == SurfaceType::Cylindric) {
        for (unsigned int i = 0; i < 3 * surface->GetBPoints(); ++i) {
            json::value jv = { { "id", controlPoints[i]->id } };
            points.emplace_back(jv);
        }
    }

    return {
        { "objectType", "bezierSurfaceC2" },
        { "id", surface->id },
        { "name", surface->name },
        { "controlPoints", points },
        { "size", {
            { "u", surface->GetBPoints() },
            { "v", surface->GetAPoints() }
        }},
        { "samples", {
            { "u", surface->GetDivisions() },
            { "v", surface->GetDivisions() }
        }}
    };
}

BSurface* SerializationManager::DeserializeBezierSurfaceC2(const json::value& jv, const std::unordered_map<int, Object*>& scenePoints) {
    const auto& obj = jv.as_object();

    auto size = obj.at("size");
    auto samples = obj.at("samples");

    unsigned int aPoints = size.at("v").as_int64();
    unsigned int bPoints = size.at("u").as_int64();
    unsigned int divisions = (samples.at("u").as_int64() + samples.at("v").as_int64()) / 2;
    divisions = std::min(std::max(Surface::minDivisions, divisions), Surface::maxDivisions);

    std::vector<Object*> objects = ParseControlPoints(obj.at("controlPoints"), scenePoints);

    SurfaceType type = SurfaceType::Flat;
    bool cylindric = true;
    for (unsigned int i = 0; i < 3 * bPoints; ++i) {
        unsigned int lastRow = i + aPoints * bPoints;
        if (lastRow >= objects.size() || objects[i]->id != objects[lastRow]->id) {
            cylindric = false;
            break;
        }
    }

    if (cylindric) {
        type = SurfaceType::Cylindric;
    }

    BSurface* surface = new BSurface(type, aPoints, bPoints, divisions, objects);
    surface->id = obj.at("id").as_int64();
    surface->name = obj.at("name").as_string().c_str();

    return surface;
}

bool SerializationManager::Contains(int id, std::vector<std::unique_ptr<Object>>& sceneObjects) {
    return std::find_if(sceneObjects.begin(), sceneObjects.end(), [&id](const auto& o) { return o->id == id; }) != sceneObjects.end();
}

std::vector<Object*> SerializationManager::ParseControlPoints(const json::value& controlPoints, const std::unordered_map<int, Object*>& scenePoints) {
    std::vector<Object*> objects;
    for (const auto& cp : controlPoints.as_array()) {
        int id = cp.at("id").as_int64();
        objects.push_back(scenePoints.at(id));
    }
    return objects;
}