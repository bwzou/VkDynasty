#include "runtime/code/math/math.h"
#include "runtime/code/math/quaternion.h"


namespace DynastyEngine
{
    Math::AngleUnit Math::k_AngleUnit;

    Math::Math() { k_AngleUnit = AngleUnit::AU_DEGREE; }

    bool Math::realEqual(float a, float b, float tolerance /* = std::numeric_limits<float>::epsilon() */)
    {
        return std::fabs(b - a) <= tolerance;
    }

    float Math::degreesToRadians(float degrees) { return degrees * Math_fDeg2Rad; }

    float Math::radiansToDegrees(float radians) { return radians * Math_fRad2Deg; }

    float Math::angleUnitsToRadians(float angleunits)
    {
        if (k_AngleUnit == AngleUnit::AU_DEGREE)
            return angleunits * Math_fDeg2Rad;

        return angleunits;
    }

    float Math::radiansToAngleUnits(float radians)
    {
        if (k_AngleUnit == AngleUnit::AU_DEGREE)
            return radians * Math_fRad2Deg;

        return radians;
    }

    float Math::angleUnitsToDegrees(float angleunits)
    {
        if (k_AngleUnit == AngleUnit::AU_RADIAN)
            return angleunits * Math_fRad2Deg;

        return angleunits;
    }

    float Math::degreesToAngleUnits(float degrees)
    {
        if (k_AngleUnit == AngleUnit::AU_RADIAN)
            return degrees * Math_fDeg2Rad;

        return degrees;
    }

    Radian Math::acos(float value)
    {
        if (-1.0 < value)
        {
            if (value < 1.0)
                return Radian(::acos(value));

            return Radian(0.0);
        }

        return Radian(Math_PI);
    }
    //-----------------------------------------------------------------------
    Radian Math::asin(float value)
    {
        if (-1.0 < value)
        {
            if (value < 1.0)
                return Radian(::asin(value));

            return Radian(Math_HALF_PI);
        }

        return Radian(-Math_HALF_PI);
    }

    glm::mat4
    Math::makeViewMatrix(const glm::vec3& position, const Quaternion& orientation, const glm::mat4* reflect_matrix)
    {
        glm::mat4 viewMatrix;

        // View matrix is:
        //
        //  [ Lx  Uy  Dz  Tx  ]
        //  [ Lx  Uy  Dz  Ty  ]
        //  [ Lx  Uy  Dz  Tz  ]
        //  [ 0   0   0   1   ]
        //
        // Where T = -(Transposed(Rot) * Pos)

        // This is most efficiently done using 3x3 Matrices
        glm::mat3 rot;
        orientation.toRotationMatrix(rot);

        // Make the translation relative to new axes
        glm::mat3 rotT  = glm::transpose(rot);
        glm::vec3 trans = -rotT * position;

        // Make final matrix
        viewMatrix = glm::mat4(rotT);
        viewMatrix[0][3] = trans.x;
        viewMatrix[1][3] = trans.y;
        viewMatrix[2][3] = trans.z;
        viewMatrix[3][3] = 1.0f;

        // Deal with reflections
        if (reflect_matrix)
        {
            viewMatrix = viewMatrix * (*reflect_matrix);
        }

        return viewMatrix;
    }

    glm::mat4 Math::makeLookAtMatrix(const glm::vec3& eye_position, const glm::vec3& target_position, const glm::vec3& up_dir)
    {
        glm::vec3 forward(glm::normalize(target_position - eye_position));
        glm::vec3 side(glm::normalize(cross(forward, up_dir)));
        glm::vec3 up(glm::cross(side, forward));

        glm::mat4 view(1.f);

        view[0][0] = side.x;
        view[1][0] = side.y;
        view[2][0] = side.z;
        view[3][0] = -glm::dot(side, eye_position);

        view[0][1] = up.x;
        view[1][1] = up.y;
        view[2][1] = up.z;
        view[3][1] = -glm::dot(up, eye_position);

        view[0][2] = -forward.x;
        view[1][2] = -forward.y;
        view[2][2] = -forward.z;
        view[3][2] = glm::dot(forward, eye_position);

        return view;
    }

    glm::mat4 Math::makePerspectiveMatrix(Radian fovy, float aspect, float znear, float zfar)
    {
        float tan_half_fovy = Math::tan(fovy / 2.f);

        glm::mat4 ret = glm::mat4(0.f);
        ret[0][0]     = 1.f / (aspect * tan_half_fovy);
        ret[1][1]     = 1.f / tan_half_fovy;
        ret[2][2]     = zfar / (znear - zfar);
        ret[3][2]     = -1.f;
        ret[2][3]     = -(zfar * znear) / (zfar - znear);

        return ret;
    }

    glm::mat4
    Math::makeOrthographicProjectionMatrix(float left, float right, float bottom, float top, float znear, float zfar)
    {
        float inv_width    = 1.0f / (right - left);
        float inv_height   = 1.0f / (top - bottom);
        float inv_distance = 1.0f / (zfar - znear);

        float A  = 2 * inv_width;
        float B  = 2 * inv_height;
        float C  = -(right + left) * inv_width;
        float D  = -(top + bottom) * inv_height;
        float q  = -2 * inv_distance;
        float qn = -(zfar + znear) * inv_distance;

        // NB: This creates 'uniform' orthographic projection matrix,
        // which depth range [-1,1], right-handed rules
        //
        // [ A   0   0   C  ]
        // [ 0   B   0   D  ]
        // [ 0   0   q   qn ]
        // [ 0   0   0   1  ]
        //
        // A = 2 * / (right - left)
        // B = 2 * / (top - bottom)
        // C = - (right + left) / (right - left)
        // D = - (top + bottom) / (top - bottom)
        // q = - 2 / (far - near)
        // qn = - (far + near) / (far - near)

        glm::mat4 proj_matrix = glm::mat4(0.f);
        proj_matrix[0][0]     = A;
        proj_matrix[0][3]     = C;
        proj_matrix[1][1]     = B;
        proj_matrix[1][3]     = D;
        proj_matrix[2][2]     = q;
        proj_matrix[2][3]     = qn;
        proj_matrix[3][3]     = 1;

        return proj_matrix;
    }

    glm::mat4
    Math::makeOrthographicProjectionMatrix01(float left, float right, float bottom, float top, float znear, float zfar)
    {
        float inv_width    = 1.0f / (right - left);
        float inv_height   = 1.0f / (top - bottom);
        float inv_distance = 1.0f / (zfar - znear);

        float A  = 2 * inv_width;
        float B  = 2 * inv_height;
        float C  = -(right + left) * inv_width;
        float D  = -(top + bottom) * inv_height;
        float q  = -1 * inv_distance;
        float qn = -znear * inv_distance;

        // NB: This creates 'uniform' orthographic projection matrix,
        // which depth range [-1,1], right-handed rules
        //
        // [ A   0   0   C  ]
        // [ 0   B   0   D  ]
        // [ 0   0   q   qn ]
        // [ 0   0   0   1  ]
        //
        // A = 2 * / (right - left)
        // B = 2 * / (top - bottom)
        // C = - (right + left) / (right - left)
        // D = - (top + bottom) / (top - bottom)
        // q = - 1 / (far - near)
        // qn = - near / (far - near)

        glm::mat4 proj_matrix = glm::mat4(0.f);
        proj_matrix[0][0]     = A;
        proj_matrix[0][3]     = C;
        proj_matrix[1][1]     = B;
        proj_matrix[1][3]     = D;
        proj_matrix[2][2]     = q;
        proj_matrix[2][3]     = qn;
        proj_matrix[3][3]     = 1;

        return proj_matrix;
    }

} // namespace Piccolo
