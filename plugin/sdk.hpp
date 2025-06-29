#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <optional>

#include "clipper/clipper.hpp"

namespace sdk {

    constexpr auto m_pi = 3.14159265359f;
    constexpr auto TICK = 1.0 / 30.0;

    using i8 = signed char;
    using i16 = signed short;
    using i32 = signed int;
    using i64 = long long;
    using u8 = unsigned char;
    using u16 = unsigned short;
    using u32 = unsigned int;
    using u64 = unsigned long long;
    using f32 = float;
    using f64 = double;
    using isize = i64;
    using usize = u64;

    constexpr std::uint8_t char_to_lower(std::uint8_t input) {
        if (static_cast<std::uint8_t>(input - 0x41) > 0x19u) {
            return input;
        }

        return input + 0x20;
    }

    template<uint32_t S> consteval uint64_t fnv1a64(const char (&value)[S]) {
        uint64_t hash = 14695981039346656037ui64;
        for (uint32_t i = 0; i < S - 1; i++) {
            hash = hash ^ char_to_lower( value[i] );
            hash = hash * 1099511628211ui64;
        }
        return hash;
    }

    inline constexpr uint64_t fnv1a64_rt( std::string value ) 
    {
        uint64_t hash = 14695981039346656037ui64;
        for ( uint32_t i = 0; i < value.size( ); i++ ) {
            hash = hash ^ char_to_lower( value[ i ] );
            hash = hash * 1099511628211ui64;
        }
        return hash;
    }

    constexpr u32 const fnv1a( const char* str )
    {
        u32 hash = 0x811C9DC5;

        for ( auto i = 0u; str[ i ]; ++i )
        {
            hash = 16777619 * ( hash ^ char_to_lower( str[ i ] ) );
        }

        return hash;
    }

    inline constexpr uint32_t fnv1a32_low_rt( const std::string& str ) {
        uint32_t hash = 0x811C9DC5;
        for ( uint32_t i = 0; i < str.size( ); i++ ) {
            hash = hash ^ char_to_lower( str[ i ] );
            hash = hash * 0x1000193;
        }
        return hash;
    }

#define ct_hash( str ) (std::integral_constant<std::uint32_t, fnv1a(str)>::value)
#define buff_hash( x ) ct_hash( x )
#define fnvhash( x ) ct_hash( x )

#define spell_hash_rt( x ) fnv1a32_low_rt( x )
#define buff_hash_rt( x ) fnv1a32_low_rt( x )
#define fnvhash_rt( x ) fnv1a32_low_rt( x )

    template<uint32_t S> consteval uint32_t character_hash(const char (&value)[S]) {
        uint32_t hash = 0;
        for (uint32_t i = 0; i < S - 1; i++) {
            hash = char_to_lower( value[i] ) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

    #define FNV64(x) fnv1a64(#x)

    struct SmallStr {
        static constexpr uint8_t KEY[] = {
            84,  96,  27,  227, 18,  149, 150, 125, 136, 179, 80,  164, 88,  15,  250, 4,   132, 222, 211, 219, 178, 70,
            250, 28,  95,  129, 50,  59,  53,  138, 244, 21,  70,  77,  241, 232, 144, 237, 6,   169, 189, 68,  209, 37,
            89,  230, 110, 23,  201, 178, 27,  180, 206, 142, 195, 252, 184, 52,  243, 195, 164, 52,  219, 241,
        };
        static constexpr uint8_t CAPACITY = 63;
        static constexpr uint8_t MAX_LENGTH = CAPACITY - 1;

       private:

        uint8_t length = 0;
        char data[CAPACITY] = {0};

       public:

        template<uint8_t S> consteval SmallStr(const char (&str)[S]) {
            length = S - 1;
            if (length > MAX_LENGTH) {
                length = MAX_LENGTH;
            }
            for (uint8_t i = 0; i < length; i++) {
                data[i] = str[i] ^ KEY[i];
            }
        }

        SmallStr( const std::string str ) {
            length = str.size( ) - 1;
            if ( length > MAX_LENGTH ) {
                length = MAX_LENGTH;
            }
            for ( uint8_t i = 0; i < length; i++ ) {
                data[ i ] = str[ i ] ^ KEY[ i ];
            }
        }

        ~SmallStr() = default;
    };

    class Vec2 {
       public:

        Vec2() = default;

        Vec2(const float x, const float y): x(x), y(y) {}

        auto length() const -> float {
            return std::sqrtf(x * x + y * y);
        }

        auto length_squared() const -> float {
            return x * x + y * y;
        }

        auto dist_to(const Vec2& o) const -> float {
            return ((*this) - o).length();
        }

        auto dist_to_squared(const Vec2& other) const -> float {
            return (*this - other).length_squared();
        }

        auto dot(const Vec2& other) const -> float {
            return x * other.x + y * other.y;
        }

        auto extend(const Vec2& to, const float distance) const -> Vec2 {
            const auto from = *this;
            const auto result = from + (to - from).normalize() * distance;
            return result;
        }

        auto normalize() const -> Vec2 {
            const auto l = this->length();
            if (l != 0.0f) {
                const auto inv = 1.0f / l;
                return {x * inv, y * inv};
            }

            return *this;
        }

        auto normalize_in_place() -> Vec2 {
            const auto l = length();
            if (l != 0.f) {
                *this /= length();
            }

            return *this;
        }

        // Operators
        auto operator+(const Vec2& other) const -> Vec2 {
            return {x + other.x, y + other.y};
        }

        auto operator-(const Vec2& other) const -> Vec2 {
            return {x - other.x, y - other.y};
        }

        auto operator*(const Vec2& other) const -> Vec2 {
            return {x * other.x, y * other.y};
        }

        auto operator/(const Vec2& other) const -> Vec2 {
            return {x / other.x, y / other.y};
        }

        auto operator+(const float other) const -> Vec2 {
            return {x + other, y + other};
        }

        auto operator-(const float other) const -> Vec2 {
            return {x - other, y - other};
        }

        auto operator*(const float other) const -> Vec2 {
            return {x * other, y * other};
        }

        auto operator/(const float other) const -> Vec2 {
            return {x / other, y / other};
        }

        auto operator-(const int32_t o) const -> Vec2 {
            return Vec2(x - o, y - o);
        }

        auto operator+(const int32_t o) const -> Vec2 {
            return Vec2(x + o, y + o);
        }

        auto operator*(const int32_t o) const -> Vec2 {
            return Vec2(x * o, y * o);
        }

        auto operator/(const int32_t o) const -> Vec2 {
            return Vec2(x / o, y / o);
        }

        auto operator+=(const Vec2& other) -> Vec2 {
            x += other.x;
            y += other.y;

            return *this;
        }

        auto operator+=(const float other) -> Vec2 {
            x += other;
            y += other;

            return *this;
        }

        auto operator-=(const Vec2& other) -> Vec2 {
            x -= other.x;
            y -= other.y;

            return *this;
        }

        auto operator-=(const float other) -> Vec2 {
            x -= other;
            y -= other;

            return *this;
        }

        auto operator*=(const Vec2& other) -> Vec2 {
            x *= other.x;
            y *= other.y;

            return *this;
        }

        auto operator*=(const float other) -> Vec2 {
            x *= other;
            y *= other;

            return *this;
        }

        auto operator/=(const Vec2& other) -> Vec2 {
            x /= other.x;
            y /= other.y;

            return *this;
        }

        auto operator/=(const float other) -> Vec2 {
            x /= other;
            y /= other;

            return *this;
        }

        friend auto operator==(const Vec2& lhs, const Vec2 rhs) -> bool {
            return lhs.x == rhs.x && lhs.y == rhs.y;
        }

        friend auto operator!=(const Vec2 lhs, const Vec2 rhs) -> bool {
            return lhs != rhs;
        }

        float x{};
        float y{};
    };

    class Vec3 {
       public:

        Vec3() = default;

        Vec3(const float x, const float y, const float z): x(x), y(y), z(z) {}

        [[nodiscard]] auto length() const -> float {
            return std::sqrtf(x * x + y * y + z * z);
        }

        [[nodiscard]] auto is_valid( ) const -> bool {
            return x != 0.f || z != 0.f;
        }

    	[[nodiscard]] auto length_world() const -> float {
            return std::sqrtf(x * x + z * z);
        }

        [[nodiscard]] auto length_squared() const -> float {
            return x * x + y * y + z * z;
        }

        [[nodiscard]] auto length2d() const -> float {
            return std::sqrtf(x * x + y * y);
        }

        auto operator-(const Vec3& o) const -> Vec3 {
            return Vec3(x - o.x, y - o.y, z - o.z);
        }

        auto operator+(const Vec3& o) const -> Vec3 {
            return Vec3(x + o.x, y + o.y, z + o.z);
        }

        auto operator*(const Vec3& o) const -> Vec3 {
            return Vec3(x * o.x, y * o.y, z * o.z);
        }

        auto operator/(const Vec3& o) const -> Vec3 {
            return Vec3(x / o.x, y / o.y, z / o.z);
        }

        auto operator-(const float o) const -> Vec3 {
            return Vec3(x - o, y - o, z - o);
        }

        auto operator+(const float o) const -> Vec3 {
            return Vec3(x + o, y + o, z + o);
        }

        auto operator*(const float o) const -> Vec3 {
            return Vec3(x * o, y * o, z * o);
        }

        auto operator/(const float o) const -> Vec3 {
            return Vec3(x / o, y / o, z / o);
        }

        auto operator-(const int32_t o) const -> Vec3 {
            return Vec3(x - o, y - o, z - o);
        }

        auto operator+(const int32_t o) const -> Vec3 {
            return Vec3(x + o, y + o, z + o);
        }

        auto operator*(const int32_t o) const -> Vec3 {
            return Vec3(x * o, y * o, z * o);
        }

        auto operator/(const int32_t o) const -> Vec3 {
            return Vec3(x / o, y / o, z / o);
        }

        friend auto operator==(const Vec3& lhs, const Vec3& rhs) -> bool {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
        }

        friend auto operator!=(const Vec3& lhs, const Vec3& rhs) -> bool {
            return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z;
        }

        auto operator+=(const Vec3& o) -> Vec3 {
            x += o.x;
            y += o.y;
            z += o.z;
            return *this;
        }

        auto operator-=(const Vec3& o) -> Vec3 {
            x -= o.x;
            y -= o.y;
            z -= o.z;
            return *this;
        }

        auto operator*=(const Vec3& o) -> Vec3 {
            x *= o.x;
            y *= o.y;
            z *= o.z;
            return *this;
        }

        auto operator/=(const Vec3& o) -> Vec3 {
            x /= o.x;
            y /= o.y;
            z /= o.z;
            return *this;
        }

        auto operator+=(const float o) -> Vec3 {
            x += o;
            y += o;
            z += o;
            return *this;
        }

        auto operator-=(const float o) -> Vec3 {
            x -= o;
            y -= o;
            z -= o;
            return *this;
        }

        auto operator*=(const float o) -> Vec3 {
            x *= o;
            y *= o;
            z *= o;
            return *this;
        }

        auto operator/=(const float o) -> Vec3 {
            x /= o;
            y /= o;
            z /= o;
            return *this;
        }
      
        [[nodiscard]] auto dist_to(const Vec3& o) const -> float {
            return ((*this) - o).length_world();
        }

        [[nodiscard]] auto dist_to_3d(const Vec3& o) const -> float {
            return ((*this) - o).length();
        }

        auto dist_to_squared(const Vec3& o) const -> float {
            return (*this - o).length_squared();
        }

        auto dot(const Vec3& o) const -> float {
            return x * o.x + y * o.y + z * o.z;
        }

        auto cross(const Vec3& o) const -> Vec3 {
            return Vec3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
        }

        auto perpendicular() const -> Vec3 {
            return Vec3(-z, 0.f, x);
        }

        auto normalize() const -> Vec3 {
            const auto l = this->length();
            if (l != 0.0f) {
                const auto inv = 1.0f / l;
                return {x * inv, y * inv, z * inv};
            }

            return *this;
        }

        auto normalize_in_place() -> Vec3 {
            const auto l = this->length();
            if (l != 0.f) {
                *this /= l;
                return *this;
            }
            return *this;
        }

        auto dot_product(const Vec3& other) const -> float {
            return this->x * other.x + this->z * other.z;
        }

        static auto close(const float a, const float b, float eps) -> bool {
            if (abs(eps) < FLT_EPSILON) {
                eps = static_cast<float>(1e-9);
            }
            return abs(a - b) <= eps;
        }

        auto polar() const -> float {
            if (this->close(x, 0.f, 0.f)) {
                if (z > 0.f) {
                    return 90.f;
                }
                return z < 0.f ? 270.f : 0.f;
            }

            auto theta = atan(z / x) * 180.f / m_pi;
            if (x < 0.f) {
                theta = theta + 180.f;
            }
            if (theta < 0.f) {
                theta = theta + 360.f;
            }
            return theta;
        }

        auto angle_between(const Vec3& other) const -> float {
            auto theta = this->polar() - other.polar();
            if (theta < 0.f) {
                theta = theta + 360.f;
            }
            if (theta > 180.f) {
                theta = 360.f - theta;
            }
            return theta;
        }

        // with_lua( auto angle_between_lua( sol::object other ) const -> sol::object; )

        auto angle_between_degrees(const Vec3& other) const -> float {
            if (this->close(x, 0.f, 0.f)) {
                if (z > 0.f) {
                    return 90.f;
                }
                return z < 0.f ? 270.f : 0.f;
            }

            const auto rise = other.z - z;
            const auto run = other.x - x;

            auto theta = atan(fabs(rise / run)) * (180.f / m_pi);

            if (run > 0.f) {
                theta = 90.f + (rise > 0.f ? -theta : theta);
            } else {
                theta = 270.f + (rise > 0.f ? theta : -theta);
            }
            return theta;
        }

        auto rotated(const float angle) const -> Vec3 {
            const auto c = cos(angle);
            const auto s = sin(angle);

            return Vec3((x * c - z * s), 0.f, (z * c + x * s));
        }

        auto rotated_raw(float angle) const -> Vec3 {
            angle = angle * (3.14159265358979323846264338327950288419716939937510f / 180.f);

            const auto c = cos(angle);
            const auto s = sin(angle);

            return Vec3((x * c - z * s), 0.f, (z * c + x * s));
        }

        auto extend(const Vec3& to, const float distance) const -> Vec3 {
            const auto from = *this;

            if (from.dist_to(to) < distance) {
                auto result = from + (to - from).normalize() * distance;
                result.y = to.y;

                return result;
            }

            return Vec3{from + (to - from).normalize() * distance};
        }

        float x{};
        float y{};
        float z{};
    };

    struct projection_info {
        bool is_on_segment{ };
        Vec3 line_point{ };
        Vec3 segment_point{ };

        projection_info( bool is_on_segment, const Vec3& segment_point, const Vec3& line_point );
    };

    inline auto project_on( const Vec3& source, const Vec3& segment_start, const Vec3& segment_end ) -> projection_info {
        float rs;
        const auto cx = source.x;
        const auto cy = source.z;
        const auto ax = segment_start.x;
        const auto ay = segment_start.z;
        const auto bx = segment_end.x;
        const auto by = segment_end.z;

        const auto rl =
            static_cast< float >( ( cx - ax ) * ( bx - ax ) + ( cy - ay ) * ( by - ay ) ) / static_cast< float >( pow( bx - ax, 2 ) + pow( by - ay, 2 ) );
        const auto point_line = Vec3( ax + rl * ( bx - ax ), 0, ay + rl * ( by - ay ) );

        if ( rl < 0 ) {
            rs = 0;
        }
        else if ( rl > 1 ) {
            rs = 1;
        }
        else {
            rs = rl;
        }

        const auto is_on_segment = rs == rl;
        const auto point_segment = is_on_segment ? point_line : Vec3( ax + rs * ( bx - ax ), 0, ay + rs * ( by - ay ) );

        return { is_on_segment, point_segment, point_line };
    }

    class Polygon {
       public:

        std::vector<Vec3> points;

        operator const std::vector<Vec3>&() {
            return points;
        }

        auto add(const Vec3& point) -> void {
            points.push_back(point);
        }

        auto is_inside(const Vec3& point) -> bool {
            return !is_outside(point);
        }

        auto is_outside(const Vec3& point) const -> bool {
            const auto p = ClipperLib::IntPoint(static_cast<long long>(point.x), static_cast<long long>(point.z));

            return PointInPolygon(p, to_clipper_path()) != 1;
        }

        auto point_in_polygon(const Vec3& point) -> int {
            const auto p = ClipperLib::IntPoint(static_cast<long long>(point.x), static_cast<long long>(point.z));
            return PointInPolygon(p, to_clipper_path());
        }

        auto to_clipper_path() const -> std::vector<ClipperLib::IntPoint> {
            std::vector<ClipperLib::IntPoint> result;

            for (const auto& point: points) {
                result.emplace_back(static_cast<long long>(point.x), static_cast<long long>(point.z));
            }

            return result;
        }
    };

    class Rectangle
    {
    public:
        Vec3 direction;
        Vec3 perpendicular;
        Vec3 r_end;
        Vec3 r_start;
        float width;

        Rectangle( const Vec3& start, const Vec3& end, const float half_width ) {
            r_start = start;
            r_end = end;
            width = half_width;
            direction = ( end - start ).normalize( );
            perpendicular = direction.perpendicular( );
        }

        auto to_polygon( const int offset = 0, const float override_width = -1 ) const -> Polygon {
            auto result = Polygon( );

            result.add(
                r_start + perpendicular * ( override_width > 0.f ? override_width : width + static_cast< float >( offset ) ) -
                direction * static_cast< float >( offset )
            );
            result.add(
                r_start - perpendicular * ( override_width > 0.f ? override_width : width + static_cast< float >( offset ) ) -
                direction * static_cast< float >( offset )
            );
            result.add(
                r_end - perpendicular * ( override_width > 0.f ? override_width : width + static_cast< float >( offset ) ) +
                direction * static_cast< float >( offset )
            );
            result.add(
                r_end + perpendicular * ( override_width > 0.f ? override_width : width + static_cast< float >( offset ) ) +
                direction * static_cast< float >( offset )
            );

            return result;
        }
    };

    enum class EOrbwalkerMode {
        none = 0,
        combo,
        lasthit,
        laneclear,
        harass,
        flee,
        recalling,
        freeze
    };

    enum class EHeroes : u32
    {
        null,
        aatrox = 266,
        ahri = 103,
        akali = 84,
        alistar = 12,
        amumu = 32,
        anivia = 34,
        annie = 1,
        aphelios = 523,
        ashe = 22,
        aurelion_sol = 136,
        azir = 268,
        bard = 432,
        blitzcrank = 53,
        brand = 63,
        braum = 201,
        caitlyn = 51,
        camille = 164,
        cassiopeia = 69,
        chogath = 31,
        corki = 42,
        darius = 122,
        diana = 131,
        draven = 119,
        dr_mundo = 36,
        ekko = 245,
        elise = 60,
        evelynn = 28,
        ezreal = 81,
        fiddlesticks = 9,
        fiora = 114,
        fizz = 105,
        galio = 3,
        gangplank = 41,
        garen = 86,
        gnar = 150,
        gragas = 79,
        graves = 104,
        gwen = 887,
        hecarim = 120,
        heimerdinger = 74,
        illaoi = 420,
        irelia = 39,
        ivern = 427,
        janna = 40,
        jarvan_iv = 59,
        jax = 24,
        jayce = 126,
        jhin = 202,
        jinx = 222,
        kaisa = 145,
        kalista = 429,
        karma = 43,
        karthus = 30,
        kassadin = 38,
        katarina = 55,
        kayle = 10,
        kayn = 141,
        kennen = 85,
        khazix = 121,
        kindred = 203,
        kled = 240,
        kog_maw = 96,
        leblanc = 7,
        lee_sin = 64,
        leona = 89,
        lillia = 876,
        lissandra = 127,
        lucian = 236,
        lulu = 117,
        lux = 99,
        malphite = 54,
        malzahar = 90,
        maokai = 57,
        master_yi = 11,
        miss_fortune = 21,
        monkey_king = 62,
        mordekaiser = 82,
        morgana = 25,
        nami = 267,
        nasus = 75,
        nautilus = 111,
        neeko = 518,
        nidalee = 76,
        nocturne = 56,
        nunu = 20,
        olaf = 2,
        orianna = 61,
        ornn = 516,
        pantheon = 80,
        poppy = 78,
        pyke = 555,
        qiyana = 246,
        quinn = 133,
        rakan = 497,
        rammus = 33,
        rek_sai = 421,
        rell = 526,
        renekton = 58,
        rengar = 107,
        riven = 92,
        rumble = 68,
        ryze = 13,
        samira = 360,
        sejuani = 113,
        senna = 235,
        seraphine = 147,
        sett = 875,
        shaco = 35,
        shen = 98,
        shyvana = 102,
        singed = 27,
        sion = 14,
        sivir = 15,
        skarner = 72,
        sona = 37,
        soraka = 16,
        swain = 50,
        sylas = 517,
        syndra = 134,
        tahm_kench = 223,
        taliyah = 163,
        talon = 91,
        taric = 44,
        teemo = 17,
        thresh = 412,
        tristana = 18,
        trundle = 48,
        tryndamere = 23,
        twisted_fate = 4,
        twitch = 29,
        udyr = 77,
        urgot = 6,
        varus = 110,
        vayne = 67,
        veigar = 45,
        velkoz = 161,
        vi = 254,
        viego = 234,
        viktor = 112,
        vladimir = 8,
        volibear = 106,
        warwick = 19,
        xayah = 498,
        xerath = 101,
        xin_zhao = 5,
        yasuo = 157,
        yone = 777,
        yorick = 83,
        yuumi = 350,
        zac = 154,
        zed = 238,
        ziggs = 115,
        zilean = 26,
        zoe = 142,
        zyra = 143,
        vex = 711,
        zeri = 221,
        belveth = 9999,
        akshan = 9991,
        nilah = 9990
    };

    enum class EDamageType {
        true_damage,
        physical_damage,
        magic_damage,
        mixed_damage
    };

    enum class EHitchance {
        low = 0,
        medium,
        high,
        very_high,
        immobile,
        invalid
    };

    enum class EInvalidReason {
        unknown = 0,
        out_of_range,
        small_path,
        bad_slow,
        bad_haste,
        invalid_prediction,
        windwall_collision
    };

    enum class EHitchanceReason {
        unknown = 0,
        recent_path,
        similar_path_angle,
        fresh_path_angle,
        abnormal_path_length,
        reaction_time,
        post_spell,
        crowd_control,
        dash,
        idle,
        guaranteed_hit,
        predicted_stun,
        spellcast,
        bad_duration,
        slowed
    };

    struct PredictionResult {
        bool valid{};
        Vec3 position{};
        Vec3 default_position{};
        EHitchance hitchance{};

        EInvalidReason error_reason{};
        EHitchanceReason reason{};
    };

    enum E3dCircleFlags : int32_t {
        filled = 1 << 0,
        outline = 1 << 1
    };

    class Color {
       public:

        Color() = default;

        Color(const int32_t r, const int32_t g, const int32_t b): r(r), g(g), b(b) {}

        Color(const int32_t r, const int32_t g, const int32_t b, const int32_t a): r(r), g(g), b(b), a(a) {}

        Color(const float r, const float g, const float b):
            r(static_cast<int32_t>(r)), g(static_cast<int32_t>(g)), b(static_cast<int32_t>(b)) {}

        Color(const float r, const float g, const float b, const float a):
            r(static_cast<int32_t>(r)), g(static_cast<int32_t>(g)), b(static_cast<int32_t>(b)), a(static_cast<int32_t>(a)) {}

        explicit Color(const std::string& hex_color) {
            if (!hex_color.starts_with("#")) {
                return;
            }
            const auto parsed = std::stoi(hex_color.substr(1), 0, 16);

            r = (parsed >> 16) & 0xFF;
            g = (parsed >> 8) & 0xFF;
            b = parsed & 0xFF;
        }

        auto to_hex() const -> std::string {
            return std::to_string((r << 16) + (g << 8) + b);
        }

        friend auto operator==(const Color& lhs, const Color& rhs) -> bool {
            return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
        }

        friend auto operator!=(const Color& lhs, const Color& rhs) -> bool {
            return !(lhs == rhs);
        }

        static auto white() -> Color {
            return Color(255, 255, 255);
        }

        static auto black() -> Color {
            return Color(0, 0, 0);
        }

        static auto red() -> Color {
            return Color(255, 0, 0);
        }

        static auto green() -> Color {
            return Color(0, 255, 0);
        }

        static auto blue() -> Color {
            return Color(0, 0, 255);
        }

        int32_t r{255};
        int32_t g{255};
        int32_t b{255};
        int32_t a{255};

        auto alpha(int32_t new_value) -> Color {
            a = new_value;
            return *this;
        }

        auto red(int32_t new_value) -> Color {
            r = new_value;
            return *this;
        }

        auto green(int32_t new_value) -> Color {
            g = new_value;
            return *this;
        }

        auto blue(int32_t new_value) -> Color {
            b = new_value;
            return *this;
        }
    };

    using FnLoadFunction = void* (*)(uint64_t hash);
    extern FnLoadFunction load_function;
    using FnEventCallback = void (*)(void*);

#define JOIN(...) __VA_ARGS__
#define IMPORT_FUNCTION(namespace, type, name, args, params)                                   \
    type name(args) {                                                                          \
        static const auto fn = (type(*)(args))(load_function(fnv1a64(#namespace "::" #name))); \
        return fn(params);                                                                     \
    }
#define IMPORT_METHOD(namespace, type, name, args, params)                                            \
    type name(args) {                                                                                 \
        static const auto fn = (type(*)(void*, args))(load_function(fnv1a64(#namespace "::" #name))); \
        return fn(this, params);                                                                      \
    }
#define IMPORT_FIELD(namespace, type, name)                                                     \
    type name() {                                                                               \
        static const auto fn = (type(*)(void*))(load_function(fnv1a64(#namespace "::" #name))); \
        return fn(this);                                                                        \
    }
#define IMPORT_FIELD_VECTOR(namespace, type, name)                                                      \
    type name() {                                                                                       \
        static const auto fn = (void (*)(void*, type*))(load_function(fnv1a64(#namespace "::" #name))); \
        type out;                                                                                       \
        fn(this, &out);                                                                                 \
        return out;                                                                                     \
    }
#define IMPORT_FIELD_STRING(namespace, name)                                                        \
    std::string name() {                                                                            \
        static const auto fn = (void (*)(void*, char*, size_t))(load_function(fnv1a64(#namespace "::" #name))); \
        char buffer[256] = {};                                                                      \
        fn(this, buffer, sizeof(buffer));                                                           \
        return std::string(buffer);                                                                 \
    }

    enum class GameObjectOrder : uint8_t {
        HoldPosition = 1, // hold, no attack
        MoveTo = 2,       // move, attack move
        AttackUnit = 3,   // target attack
        AutoAttackPet,
        AutoAttack,
        MovePet,
        AttackTo,
        Stop = 10 // stop
    };

    struct OnProcessSpellEvent {
        struct RawSpellCastInfo* spell_cast_info;
    };

    struct OnIssueOrderEvent {
        const uint32_t order;
    	Vec3* position;
    	struct Object* target;
        const bool manual;
        bool* prevent;
    };

    struct OnCreateObjectEvent {
        Object* object;
    };

    struct OnDeleteObjectEvent {
        const uintptr_t object_address;
    };

    struct OnStartCastEvent {
        struct SpellSlot* spell;
        const bool manual;
        bool* prevent;
    };

    struct OnBuffEvent {
        Object* object;
        struct BuffInstance* buff;
        const bool gain;
    };

    struct OnSpellImpactEvent {
        RawSpellCastInfo* spell_cast_info;
    };

    struct OnPathChangeEvent {
        Object* object;
    };

    struct OnStopCastEvent {
        struct SpellCastInfo* spell_cast_info;
        const bool stop_animation;
        const bool force_stop;
    };

    struct OnOrbwalkerPreAttackEvent {
    	Object* target;
        bool cancel;
    };

    struct OnOrbwalkerPostAttackEvent {
         Object* target; // can be nullptr
    };

    struct OnOrbwalkerPreMoveEvent {
    	Vec3 position;
        bool cancel;
    };

    enum class GapcloserType {
        skillshot,
        targeted,
        item
    };

    struct OnGapcloseArgs {
        GapcloserType type;
        Object* target;
        Object* sender;

        float start_time;
        float end_time;
        float speed;

        Vec3 start_position;
        Vec3 end_position;

        bool is_unstoppable;
        bool is_cc;

        OnGapcloseArgs( ) :
            type( GapcloserType::skillshot ),
            target( nullptr ),
            sender( nullptr ),
            start_time( 0.f ),
            end_time( 0.f ),
            speed( 0.f ),
            is_unstoppable( false ),
            is_cc( false ) {
        }
    };

    enum class EventType : uint8_t {
        // GAME EVENTS
        OnCreateObject, // Object*
        OnDeleteObject, // uintptr_t object_address
        OnStartCast,    // OnStartCastEvent* arg
        OnProcessSpell, // OnProcessSpellEvent* arg
        OnStopCast,     // OnStopCastEvent* arg
        OnSpellImpact,  // OnSpellImpactEvent* arg
        OnIssueOrder,   // OnIssueOrderEvent* arg
        OnBuff,         // OnBuffEvent* arg
        OnPathChange,   // OnPathChangeEvent* arg

        // FEATURE EVENTS
        OnOrbwalkerPreAttack,  // OnOrbwalkerPreAttackEvent* arg
        OnOrbwalkerPostAttack, // OnOrbwalkerPostAttackEvent* arg
        OnOrbwalkerPreMove,    // OnOrbwalkerPreMoveEvent* arg

        // GENERAL EVENTS
        OnUpdate,
        OnDraw,
        OnUpdateMenu,

        OnOrbwalkerAttackOrder,

        OnUpdatePermashow,
        OnUpdateMenu2, //second_tab
        
        OnGapclose,
        OnDrawGround
    };

    struct Subscription {
        EventType event_type;
        FnEventCallback callback;
    };

    void register_plugin(SmallStr name, const std::vector<Subscription>& subscriptions);
    void disable_core_plugin( );
    float get_time();
    float get_ping();
    Vec3 get_cursor();
    bool move_to(Vec3);
    bool order_attack(uint32_t);

    // w2s
    std::optional<Vec2> world_to_screen(Vec3 world_position);
    Vec2 world_to_minimap(Vec3 world_position);

    // drawings
    namespace render
    {
        void text( Vec2 position, const std::string& text, float text_size, Color color, bool drop_shadow = false );
        void text_3d( Vec3 world_position, const std::string& text, float text_size, Color color, bool drop_shadow = false );
        Vec2 get_text_size( const std::string& text_string, float text_size );
        // 2d
        void box( Vec2 position, Vec2 size, Color color, float rounding, float thickness = 1.f );
        void filled_box( Vec2 position, Vec2 size, Color color, float rounding = -1 );
        void circle( Vec2 position, float radius, Color color, int segments = 32, float thickness = 1.f );
        void filled_circle( Vec2 position, float radius, Color color, int segments = 32 );
        void line( Vec2 start, Vec2 end, Color color, float thickness = 1.f );
        void triangle( Vec2 left, Vec2 right, Vec2 bottom, Color color, float thickness = 1.f );
        void filled_triangle( Vec2 left, Vec2 right, Vec2 bottom, Color color );
        void rectangle_3d( Vec3 start, Vec3 end, float radius, Color color, int flags = 0, float thickness = 1.f ); //E3dCircleFlags = flags
        void polygon_3d( const std::vector<Vec3>& polygon, Color color, int flags = 0, float thickness = 1.f );
        void line_3d( Vec3 start, Vec3 end, Color color, float thickness = 1.f );
        void circle_3d( Vec3 position, Color color, float radius, int flags, int segments, float thickness, float angle, Vec3 direction );
        void texture( Vec2 position, Vec2 size, void* texture_view );
        void circle_minimap( Vec3 position, Color color, float radius, int segments = 32, float thickness = 1.f );
        void circle_3d_glow( const Vec3 center,
            const f32 radius,
            const Color color,
            const f32 thickness = 1.0f,
            const f32 glow = 0.0f,
            const u8 type = 0,
            const f32 glow_opacity = 0.35f,
            const f32 background_opacity = 0.0f );
        void polyline_3d_glow( const std::vector<Vec3>& points,
            const Color color,
            const f32 thickness = 1.0f,
            const f32 glow = 0.0f,
            const f32 glow_opacity = 0.35f );
        void polygon_3d_glow( const std::vector<Vec3>& points,
            const Color color,
            const f32 thickness = 1.0f,
            const f32 glow = 0.0f,
            const f32 glow_opacity = 0.35f,
            const f32 background_opacity = 0.0f );
        void line_3d_glow( Vec3 start, Vec3 end,
            const Color color, const f32 thickness = 1.f, const f32 glow = 0.0f, const f32 glow_strength = 0.35f );
    } // namespace render

    namespace world_math {
        std::vector<Vec3> get_3d_circle_points(Vec3 world_position, float radius, int segments, float draw_angle, Vec3 circle_direction);
    } // namespace world_math

    struct ResourceBarRect {
        float left;
        float top;
        float right;
        float bot;
    };

    enum class ImGuiKey : int
    {
        // Keyboard
        ImGuiKey_None = 0,
        ImGuiKey_Tab = 512,             // == ImGuiKey_NamedKey_BEGIN
        ImGuiKey_LeftArrow,
        ImGuiKey_RightArrow,
        ImGuiKey_UpArrow,
        ImGuiKey_DownArrow,
        ImGuiKey_PageUp,
        ImGuiKey_PageDown,
        ImGuiKey_Home,
        ImGuiKey_End,
        ImGuiKey_Insert,
        ImGuiKey_Delete,
        ImGuiKey_Backspace,
        ImGuiKey_Space,
        ImGuiKey_Enter,
        ImGuiKey_Escape,
        ImGuiKey_LeftCtrl, ImGuiKey_LeftShift, ImGuiKey_LeftAlt, ImGuiKey_LeftSuper,
        ImGuiKey_RightCtrl, ImGuiKey_RightShift, ImGuiKey_RightAlt, ImGuiKey_RightSuper,
        ImGuiKey_Menu,
        ImGuiKey_0, ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, ImGuiKey_4, ImGuiKey_5, ImGuiKey_6, ImGuiKey_7, ImGuiKey_8, ImGuiKey_9,
        ImGuiKey_A, ImGuiKey_B, ImGuiKey_C, ImGuiKey_D, ImGuiKey_E, ImGuiKey_F, ImGuiKey_G, ImGuiKey_H, ImGuiKey_I, ImGuiKey_J,
        ImGuiKey_K, ImGuiKey_L, ImGuiKey_M, ImGuiKey_N, ImGuiKey_O, ImGuiKey_P, ImGuiKey_Q, ImGuiKey_R, ImGuiKey_S, ImGuiKey_T,
        ImGuiKey_U, ImGuiKey_V, ImGuiKey_W, ImGuiKey_X, ImGuiKey_Y, ImGuiKey_Z,
        ImGuiKey_F1, ImGuiKey_F2, ImGuiKey_F3, ImGuiKey_F4, ImGuiKey_F5, ImGuiKey_F6,
        ImGuiKey_F7, ImGuiKey_F8, ImGuiKey_F9, ImGuiKey_F10, ImGuiKey_F11, ImGuiKey_F12,
        ImGuiKey_F13, ImGuiKey_F14, ImGuiKey_F15, ImGuiKey_F16, ImGuiKey_F17, ImGuiKey_F18,
        ImGuiKey_F19, ImGuiKey_F20, ImGuiKey_F21, ImGuiKey_F22, ImGuiKey_F23, ImGuiKey_F24,
        ImGuiKey_Apostrophe,        // '
        ImGuiKey_Comma,             // ,
        ImGuiKey_Minus,             // -
        ImGuiKey_Period,            // .
        ImGuiKey_Slash,             // /
        ImGuiKey_Semicolon,         // ;
        ImGuiKey_Equal,             // =
        ImGuiKey_LeftBracket,       // [
        ImGuiKey_Backslash,         // \ (this text inhibit multiline comment caused by backslash)
        ImGuiKey_RightBracket,      // ]
        ImGuiKey_GraveAccent,       // `
        ImGuiKey_CapsLock,
        ImGuiKey_ScrollLock,
        ImGuiKey_NumLock,
        ImGuiKey_PrintScreen,
        ImGuiKey_Pause,
        ImGuiKey_Keypad0, ImGuiKey_Keypad1, ImGuiKey_Keypad2, ImGuiKey_Keypad3, ImGuiKey_Keypad4,
        ImGuiKey_Keypad5, ImGuiKey_Keypad6, ImGuiKey_Keypad7, ImGuiKey_Keypad8, ImGuiKey_Keypad9,
        ImGuiKey_KeypadDecimal,
        ImGuiKey_KeypadDivide,
        ImGuiKey_KeypadMultiply,
        ImGuiKey_KeypadSubtract,
        ImGuiKey_KeypadAdd,
        ImGuiKey_KeypadEnter,
        ImGuiKey_KeypadEqual,
        ImGuiKey_AppBack,               // Available on some keyboard/mouses. Often referred as "Browser Back"
        ImGuiKey_AppForward,

        // Gamepad (some of those are analog values, 0.0f to 1.0f)                          // NAVIGATION ACTION
        // (download controller mapping PNG/PSD at http://dearimgui.com/controls_sheets)
        ImGuiKey_GamepadStart,          // Menu (Xbox)      + (Switch)   Start/Options (PS)
        ImGuiKey_GamepadBack,           // View (Xbox)      - (Switch)   Share (PS)
        ImGuiKey_GamepadFaceLeft,       // X (Xbox)         Y (Switch)   Square (PS)        // Tap: Toggle Menu. Hold: Windowing mode (Focus/Move/Resize windows)
        ImGuiKey_GamepadFaceRight,      // B (Xbox)         A (Switch)   Circle (PS)        // Cancel / Close / Exit
        ImGuiKey_GamepadFaceUp,         // Y (Xbox)         X (Switch)   Triangle (PS)      // Text Input / On-screen Keyboard
        ImGuiKey_GamepadFaceDown,       // A (Xbox)         B (Switch)   Cross (PS)         // Activate / Open / Toggle / Tweak
        ImGuiKey_GamepadDpadLeft,       // D-pad Left                                       // Move / Tweak / Resize Window (in Windowing mode)
        ImGuiKey_GamepadDpadRight,      // D-pad Right                                      // Move / Tweak / Resize Window (in Windowing mode)
        ImGuiKey_GamepadDpadUp,         // D-pad Up                                         // Move / Tweak / Resize Window (in Windowing mode)
        ImGuiKey_GamepadDpadDown,       // D-pad Down                                       // Move / Tweak / Resize Window (in Windowing mode)
        ImGuiKey_GamepadL1,             // L Bumper (Xbox)  L (Switch)   L1 (PS)            // Tweak Slower / Focus Previous (in Windowing mode)
        ImGuiKey_GamepadR1,             // R Bumper (Xbox)  R (Switch)   R1 (PS)            // Tweak Faster / Focus Next (in Windowing mode)
        ImGuiKey_GamepadL2,             // L Trig. (Xbox)   ZL (Switch)  L2 (PS) [Analog]
        ImGuiKey_GamepadR2,             // R Trig. (Xbox)   ZR (Switch)  R2 (PS) [Analog]
        ImGuiKey_GamepadL3,             // L Stick (Xbox)   L3 (Switch)  L3 (PS)
        ImGuiKey_GamepadR3,             // R Stick (Xbox)   R3 (Switch)  R3 (PS)
        ImGuiKey_GamepadLStickLeft,     // [Analog]                                         // Move Window (in Windowing mode)
        ImGuiKey_GamepadLStickRight,    // [Analog]                                         // Move Window (in Windowing mode)
        ImGuiKey_GamepadLStickUp,       // [Analog]                                         // Move Window (in Windowing mode)
        ImGuiKey_GamepadLStickDown,     // [Analog]                                         // Move Window (in Windowing mode)
        ImGuiKey_GamepadRStickLeft,     // [Analog]
        ImGuiKey_GamepadRStickRight,    // [Analog]
        ImGuiKey_GamepadRStickUp,       // [Analog]
        ImGuiKey_GamepadRStickDown,     // [Analog]

        // Aliases: Mouse Buttons (auto-submitted from AddMouseButtonEvent() calls)
        // - This is mirroring the data also written to io.MouseDown[], io.MouseWheel, in a format allowing them to be accessed via standard key API.
        ImGuiKey_MouseLeft, ImGuiKey_MouseRight, ImGuiKey_MouseMiddle, ImGuiKey_MouseX1, ImGuiKey_MouseX2, ImGuiKey_MouseWheelX, ImGuiKey_MouseWheelY,

        // [Internal] Reserved for mod storage
        ImGuiKey_ReservedForModCtrl, ImGuiKey_ReservedForModShift, ImGuiKey_ReservedForModAlt, ImGuiKey_ReservedForModSuper,
        ImGuiKey_COUNT,

        // Keyboard Modifiers (explicitly submitted by backend via AddKeyEvent() calls)
        // - This is mirroring the data also written to io.KeyCtrl, io.KeyShift, io.KeyAlt, io.KeySuper, in a format allowing
        //   them to be accessed via standard key API, allowing calls such as IsKeyPressed(), IsKeyReleased(), querying duration etc.
        // - Code polling every key (e.g. an interface to detect a key press for input mapping) might want to ignore those
        //   and prefer using the real keys (e.g. ImGuiKey_LeftCtrl, ImGuiKey_RightCtrl instead of ImGuiMod_Ctrl).
        // - In theory the value of keyboard modifiers should be roughly equivalent to a logical or of the equivalent left/right keys.
        //   In practice: it's complicated; mods are often provided from different sources. Keyboard layout, IME, sticky keys and
        //   backends tend to interfere and break that equivalence. The safer decision is to relay that ambiguity down to the end-user...
        ImGuiMod_None = 0,
        ImGuiMod_Ctrl = 1 << 12, // Ctrl
        ImGuiMod_Shift = 1 << 13, // Shift
        ImGuiMod_Alt = 1 << 14, // Option/Menu
        ImGuiMod_Super = 1 << 15, // Cmd/Super/Windows
        ImGuiMod_Shortcut = 1 << 11, // Alias for Ctrl (non-macOS) _or_ Super (macOS).
        ImGuiMod_Mask_ = 0xF800,  // 5-bits

        // [Internal] Prior to 1.87 we required user to fill io.KeysDown[512] using their own native index + the io.KeyMap[] array.
        // We are ditching this method but keeping a legacy path for user code doing e.g. IsKeyPressed(MY_NATIVE_KEY_CODE)
        // If you need to iterate all keys (for e.g. an input mapper) you may use ImGuiKey_NamedKey_BEGIN..ImGuiKey_NamedKey_END.
        ImGuiKey_NamedKey_BEGIN = 512,
        ImGuiKey_NamedKey_END = ImGuiKey_COUNT,
        ImGuiKey_NamedKey_COUNT = ImGuiKey_NamedKey_END - ImGuiKey_NamedKey_BEGIN,
#ifdef IMGUI_DISABLE_OBSOLETE_KEYIO
        ImGuiKey_KeysData_SIZE = ImGuiKey_NamedKey_COUNT,  // Size of KeysData[]: only hold named keys
        ImGuiKey_KeysData_OFFSET = ImGuiKey_NamedKey_BEGIN,  // Accesses to io.KeysData[] must use (key - ImGuiKey_KeysData_OFFSET) index.
#else
        ImGuiKey_KeysData_SIZE = ImGuiKey_COUNT,           // Size of KeysData[]: hold legacy 0..512 keycodes + named keys
        ImGuiKey_KeysData_OFFSET = 0,                        // Accesses to io.KeysData[] must use (key - ImGuiKey_KeysData_OFFSET) index.
#endif

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        ImGuiKey_ModCtrl = ImGuiMod_Ctrl, ImGuiKey_ModShift = ImGuiMod_Shift, ImGuiKey_ModAlt = ImGuiMod_Alt, ImGuiKey_ModSuper = ImGuiMod_Super, // Renamed in 1.89
        ImGuiKey_KeyPadEnter = ImGuiKey_KeypadEnter,    // Renamed in 1.87
#endif
    };

    enum class EWardType {
        unknown = 0,
        normal,
        control,
        blue,
        zombie,
        teemo_shroom,
        shaco_box,
        jhin_trap,
        nidalee_trap,
        maokai_sproutling,
        fiddlesticks_effigy,
        caitlyn_trap
    };

    enum class EMinionType {
        error = 0,
        turret,
        jungle,
        ranged,
        melee,
        siege,
        super,
        plant,
        misc
    };

    enum class EBuffType : unsigned char {
        internal = 0,
        aura,
        combat_enchancer,
        combat_dehancer,
        spell_shield,
        stun,
        invisibility,
        silence,
        taunt,
        berserk,
        polymorph,
        slow,
        snare,
        damage,
        heal,
        haste,
        spell_immunity,
        physical_immunity,
        invulnerability,
        attack_speed_slow,
        near_sight,
        currency,
        fear,
        charm,
        poison,
        suppression,
        blind,
        counter,
        shred,
        flee,
        knockup,
        knockback,
        disarm,
        grounded,
        drowsy,
        asleep,
        obscured,
        clickproof_to_enemies,
        un_killable,
        unknown
    };

    enum class ESpellSlot {
        q = 0,
        w = 1,
        e = 2,
        r = 3,
        d = 4,
        f = 5,
        item1,
        item2,
        item3,
        item4,
        item5,
        item6,
        item7,
        recall = 13,
        max
    };

    enum class ItemID : uint32_t {
        Unknown = 0,
        Boots = 1001,
        FaerieCharm = 1004,
        RejuvenationBead = 1006,
        GiantsBelt = 1011,
        GiantsBeltArena = 221011,
        CloakOfAgility = 1018,
        BlastingWand = 1026,
        BlastingWandArena = 221026,
        SapphireCrystal = 1027,
        RubyCrystal = 1028,
        ClothArmor = 1029,
        ChainVest = 1031,
        ChainVestArena = 221031,
        NullMagicMantle = 1033,
        LongSword = 1036,
        Pickaxe = 1037,
        BFSword = 1038,
        BFSwordArena = 221038,
        Dagger = 1042,
        RecurveBow = 1043,
        RecurveBowArena = 221043,
        AmplifyingTome = 1052,
        VampiricScepter = 1053,
        VampiricScepterArena = 221053,
        DoransShield = 1054,
        DoransBlade = 1055,
        DoransRing = 1056,
        NegatronCloak = 1057,
        NegatronCloakArena = 221057,
        NeedlesslyLargeRod = 1058,
        NeedlesslyLargeRodArena = 221058,
        DarkSeal = 1082,
        Cull = 1083,
        ScorchclawPup = 1101,
        GustwalkerHatchling = 1102,
        MosstomperSeedling = 1103,
        MosstomperSeedling2 = 1105,
        GustwalkerHatchling2 = 1106,
        ScorchclawPup2 = 1107,
        PenetratingBullets = 1500,
        Fortification = 1501,
        ReinforcedArmor = 1502,
        WardensEye = 1503,
        ReinforcedArmor2 = 1506,
        Overcharged = 1507,
        AntiTowerSocks = 1508,
        Gusto = 1509,
        PhreakishGusto = 1510,
        SuperMechArmor = 1511,
        SuperMechPowerField = 1512,
        TurretPlating = 1515,
        StructureBounty = 1516,
        StructureBounty2 = 1517,
        StructureBounty3 = 1518,
        StructureBounty4 = 1519,
        Overerchargedha = 1520,
        Fortification2 = 1521,
        TowerPowerUp = 1522,
        HealthPotion = 2003,
        TotalBiscuitOfEverlastingWill = 2010,
        SteelSigil = 2019,
        TheBrutalizer = 2020,
        Tunneler = 2021,
        GlowingMote = 2022,
        GlowingMoteArena = 222022,
        RefillablePotion = 2031,
        CorruptingPotion = 2033,
        PoroSnax = 2052,
        ControlWard = 2055,
        ShurelyasBattlesong = 2065,
        ShurelyasBattlesongArena = 222065,
        ElixirOfIron = 2138,
        ElixirOfSorcery = 2139,
        ElixirOfWrath = 2140,
        ElixirOfSkill = 2150,
        ElixirOfAvarice = 2151,
        ElixirOfForce = 2152,
        MinionDematerializer = 2403,
        SeekersArmguard = 2420,
        ShatteredArmguard = 2421,
        SlightlyMagicalFootwear = 2422,
        OverlordsBloodmail = 2501,
        UnendingDespair = 2502,
        UnendingDespairArena = 222502,
        BlackfireTorch = 2503,
        BlackfireTorchArena = 222503,
        KaenicRookern = 2504,
        KaenicRookernArena = 222504,
        FatedAshes = 2508,
        Trailblazer = 3002,
        ArchangelsStaff = 3003,
        ArchangelsStaffArena = 223003,
        Manamune = 3004,
        ManamuneArena = 223004,
        BerserkersGreaves = 3006,
        BerserkersGreavesArena = 223006,
        BootsOfSwiftness = 3009,
        BootsOfSwiftnessArena = 223009,
        SymbioticSoles = 3010,
        ChemtechPutrifier = 3011,
        ChemtechPutrifierArena = 223011,
        SynchronizedSouls = 3013,
        SorcerersShoes = 3020,
        SorcerersShoesArena = 223020,
        GlacialBuckler = 3024,
        GuardianAngel = 3026,
        GuardianAngelArena = 223026,
        InfinityEdge = 3031,
        InfinityEdgeArena = 223031,
        YunTalWildarrows = 3032,
        MortalReminder = 3033,
        MortalReminderArena = 223033,
        LastWhisper = 3035,
        LordDominiksRegards = 3036,
        LordDominiksRegardsArena = 223036,
        SeraphsEmbrace = 3040,
        SeraphsEmbraceArena = 223040,
        MejaisSoulstealer = 3041,
        Muramana = 3042,
        MuramanaArena = 223042,
        Phage = 3044,
        PhantomDancer = 3046,
        PhantomDancerArena = 223046,
        PlatedSteelcaps = 3047,
        PlatedSteelcapsArena = 223047,
        ZekesConvergence = 3050,
        ZekesConvergenceArena = 223050,
        HearthboundAxe = 3051,
        SteraksGage = 3053,
        SteraksGageArena = 223053,
        Sheen = 3057,
        SheenArena = 223057,
        SpiritVisage = 3065,
        SpiritVisageArena = 223065,
        WingedMoonplate = 3066,
        Kindlegem = 3067,
        KindlegemArena = 223067,
        SunfireAegis = 3068,
        SunfireAegisArena = 223068,
        TearOfTheGoddess = 3070,
        BlackCleaver = 3071,
        BlackCleaverArena = 223071,
        Bloodthirster = 3072,
        BloodthirsterArena = 223072,
        ExperimentalHexplate = 3073,
        ExperimentalHexplateArena = 223073,
        RavenousHydra = 3074,
        RavenousHydraArena = 223074,
        Thornmail = 3075,
        ThornmailArena = 223075,
        BrambleVest = 3076,
        Tiamat = 3077,
        TrinityForce = 3078,
        TrinityForceArena = 223078,
        WardensMail = 3082,
        WarmogsArmor = 3083,
        Heartsteel = 3084,
        HeartsteelArena = 223084,
        RunaansHurricane = 3085,
        RunaansHurricaneArena = 223085,
        Zeal = 3086,
        StatikkShiv = 3087,
        StatikkShivArena = 223087,
        RabadonsDeathcap = 3089,
        RabadonsDeathcapArena = 223089,
        WitsEnd = 3091,
        WitsEndArena = 223091,
        RapidFirecannon = 3094,
        RapidFirecannonArena = 223094,
        LichBane = 3100,
        LichBaneArena = 223100,
        BansheesVeil = 3102,
        BansheesVeilArena = 223102,
        Redemption = 3107,
        RedemptionArena = 223107,
        FiendishCodex = 3108,
        KnightsVow = 3109,
        KnightsVowArena = 223109,
        FrozenHeart = 3110,
        FrozenHeartArena = 223110,
        MercurysTreads = 3111,
        MercurysTreadsArena = 223111,
        AetherWisp = 3113,
        ForbiddenIdol = 3114,
        NashorsTooth = 3115,
        NashorsToothArena = 223115,
        RylaisCrystalScepter = 3116,
        RylaisCrystalScepterArena = 223116,
        MobilityBoots = 3117,
        Malignance = 3118,
        MalignanceArena = 223118,
        WintersApproach = 3119,
        WintersApproachArena = 223119,
        Fimbulwinter = 3121,
        FimbulwinterArena = 223121,
        ExecutionersCalling = 3123,
        GuinsoosRageblade = 3124,
        GuinsoosRagebladeArena = 223124,
        CaulfieldsWarhammer = 3133,
        SerratedDirk = 3134,
        VoidStaff = 3135,
        VoidStaffArena = 223135,
        Cryptbloom = 3137,
        CryptbloomArena = 223137,
        MercurialScimitar = 3139,
        MercurialScimitarArena = 223139,
        QuicksilverSash = 3140,
        YoumuusGhostblade = 3142,
        YoumuusGhostbladeArena = 223142,
        RanduinsOmen = 3143,
        RanduinsOmenArena = 223143,
        ScoutsSlingshot = 3144,
        HextechAlternator = 3145,
        HauntingGuise = 3147,
        HextechRocketbelt = 3152,
        HextechRocketbeltArena = 223152,
        BladeOfTheRuinedKing = 3153,
        BladeOfTheRuinedKingArena = 223153,
        Hexdrinker = 3155,
        MawOfMalmortius = 3156,
        MawOfMalmortiusArena = 223156,
        ZhonyasHourglass = 3157,
        ZhonyasHourglassArena = 223157,
        IonianBootsOfLucidity = 3158,
        IonianBootsOfLucidityArena = 223158,
        SpearOfShojin = 3161,
        SpearOfShojinArena = 223161,
        Morellonomicon = 3165,
        MorellonomiconArena = 223165,
        Zephyr = 3172,
        UmbralGlaive = 3179,
        Hullbreaker = 3181,
        HullbreakerArena = 223181,
        LocketOfTheIronSolari = 3190,
        LocketOfTheIronSolariArena = 223190,
        SpectresCowl = 3211,
        MikaelsBlessing = 3222,
        MikaelsBlessingArena = 223222,
        Terminus = 3302,
        TerminusArena = 223302,
        ScarecrowEffigy = 3330,
        StealthWard = 3340,
        FarsightAlteration = 3363,
        OracleLens = 3364,
        YourCut = 3400,
        ArdentCenser = 3504,
        ArdentCenserArena = 223504,
        EssenceReaver = 3508,
        EssenceReaverArena = 223508,
        KalistasBlackSpear = 3599,
        KalistasBlackSpear2 = 3600,
        DeadMansPlate = 3742,
        DeadMansPlateArena = 223742,
        TitanicHydra = 3748,
        TitanicHydraArena = 223748,
        CrystallineBracer = 3801,
        LostChapter = 3802,
        CatalystOfAeons = 3803,
        EdgeOfNight = 3814,
        EdgeOfNightArena = 223814,
        WorldAtlas = 3865,
        RunicCompass = 3866,
        BountyOfWorlds = 3867,
        CelestialOpposition = 3869,
        DreamMaker = 3870,
        ZazzaksRealmspike = 3871,
        SolsticeSleigh = 3876,
        Bloodsong = 3877,
        FireAtWill = 3901,
        DeathsDaughter = 3902,
        RaiseMorale = 3903,
        OblivionOrb = 3916,
        ImperialMandate = 4005,
        ImperialMandateArena = 224005,
        ForceOfNature = 4401,
        ForceOfNatureArena = 224401,
        HorizonFocus = 4628,
        HorizonFocusArena = 224628,
        CosmicDrive = 4629,
        CosmicDriveArena = 224629,
        BlightingJewel = 4630,
        VerdantBarrier = 4632,
        Riftmaker = 4633,
        RiftmakerArena = 224633,
        LeechingLeer = 4635,
        NightHarvester = 4636,
        DemonicEmbrace = 4637,
        WatchfulWardstone = 4638,
        StirringWardstone = 4641,
        BandleglassMirror = 4642,
        VigilantWardstone = 4643,
        Shadowflame = 4645,
        ShadowflameArena = 224645,
        Stormsurge = 4646,
        StormsurgeArena = 224646,
        DeathsDance = 6333,
        DeathsDanceArena = 226333,
        ChempunkChainsword = 6609,
        ChempunkChainswordArena = 226609,
        SunderedSky = 6610,
        SunderedSkyArena = 226610,
        StaffOfFlowingWater = 6616,
        StaffOfFlowingWaterArena = 226616,
        MoonstoneRenewer = 6617,
        MoonstoneRenewerArena = 226617,
        EchoesOfHelia = 6620,
        EchoesOfHeliaArena = 226620,
        Dawncore = 6621,
        DawncoreArena = 226621,
        Stridebreaker = 6631,
        StridebreakerArena = 226631,
        LiandrysTorment = 6653,
        LiandrysTormentArena = 226653,
        LudensCompanion = 6655,
        LudensCompanionArena = 226655,
        RodOfAges = 6657,
        RodOfAgesArena = 226657,
        BamisCinder = 6660,
        IcebornGauntlet = 6662,
        IcebornGauntletArena = 226662,
        HollowRadiance = 6664,
        HollowRadianceArena = 226664,
        JakshoTheProtean = 6665,
        JakshoTheProteanArena = 226665,
        Noonquiver = 6670,
        KrakenSlayer = 6672,
        KrakenSlayerArena = 226672,
        ImmortalShieldbow = 6673,
        ImmortalShieldbowArena = 226673,
        NavoriFlickerblade = 6675,
        NavoriFlickerbladeArena = 226675,
        TheCollector = 6676,
        TheCollectorArena = 226676,
        Rectrix = 6690,
        Eclipse = 6692,
        EclipseArena = 226692,
        ProwlersClaw = 6693,
        SeryldasGrudge = 6694,
        SeryldasGrudgeArena = 226694,
        SerpentsFang = 6695,
        SerpentsFangArena = 226695,
        AxiomArc = 6696,
        AxiomArcArena = 226696,
        Hubris = 6697,
        HubrisArena = 226697,
        ProfaneHydra = 6698,
        ProfaneHydraArena = 226698,
        VoltaicCyclosword = 6699,
        VoltaicCycloswordArena = 226699,
        Opportunity = 6701,
        OpportunityArena = 226701,
        GangplankPlaceholder = 7050,
        AnathemasChains = 8001,
        AnathemasChainsArena = 228001,
        AbyssalMask = 8020,
        AbyssalMaskArena = 228020,
    };

    enum class ECombatType {
        Invalid = 0,
        Melee,
        Ranged
    };

    //prediction
    enum class ESpellType {
        none,
        linear,
        circle
    };

    enum ECollisionFlags
    {
        none = 0,
        minions = 1 << 0,
        heroes = 1 << 1,
        spell_wall = 1 << 2,
        terrain = 1 << 3,
        structures = 1 << 4
    };

    /*
     namespace inventory_slot {

    static uint32_t get_item_id(InventorySlot* slot) {
        return slot->get_item_id();
    }

    static void get_item_name(InventorySlot* slot, std::string* out) {
        out->append(slot->get_item_name());
    }

    static int get_max_amount(InventorySlot* slot) {
        return slot->get_max_amount();
    }

    static int get_amount(InventorySlot* slot) {
        return slot->get_amount();
    }

    static float get_price(InventorySlot* slot) {
        return slot->get_price();
    }

    static int get_stacks(InventorySlot* slot) {
        return slot->get_stacks();
    }

    static float get_stats(InventorySlot* slot) {
        return slot->get_stats();
    }

}

     */

    struct InventorySlot {
        IMPORT_FIELD(inventory_slot, uint32_t, get_item_id);
        IMPORT_FIELD_STRING(inventory_slot, get_item_name);
        IMPORT_FIELD(inventory_slot, int, get_max_amount);
        IMPORT_FIELD(inventory_slot, int, get_amount);
        IMPORT_FIELD(inventory_slot, float, get_price);
        IMPORT_FIELD(inventory_slot, int, get_stacks);
        IMPORT_FIELD(inventory_slot, float, get_stats);
    };

    struct SpellData {
        IMPORT_FIELD_STRING(spell_data, spell_name);
        IMPORT_FIELD(spell_data, uint32_t, spell_hash);
        IMPORT_FIELD(spell_data, float, missile_speed);
        IMPORT_FIELD(spell_data, const float*, get_manacost_array);
        IMPORT_FIELD(spell_data, const float*, get_cooldown_array);
    };

    struct SpellInfo {
        IMPORT_FIELD(spell_info, SpellData*, spell_data);
    };

    struct SpellSlot {
        IMPORT_FIELD(spell_slot, bool, is_learned);
        IMPORT_FIELD(spell_slot, bool, is_ready);
        IMPORT_FIELD(spell_slot, float, cooldown);
        IMPORT_FIELD(spell_slot, float, cooldown_expire);
        IMPORT_FIELD(spell_slot, float, manacost);
        IMPORT_FIELD(spell_slot, uint32_t, level);
        IMPORT_FIELD(spell_slot, uint32_t, charges);
        IMPORT_FIELD_STRING(spell_slot, name);

        IMPORT_FIELD(spell_slot, SpellInfo*, get_spell_info);
        IMPORT_FIELD(spell_slot, void*, get_spell_texture);
    };

    struct RawSpellCastInfo {

		IMPORT_FIELD(raw_spell_cast_info, int16_t, caster_index);
        IMPORT_FIELD(raw_spell_cast_info, int16_t, target_index);
        IMPORT_FIELD(raw_spell_cast_info, bool, is_spell);
        IMPORT_FIELD(raw_spell_cast_info, bool, is_special_autoattack);
        IMPORT_FIELD(raw_spell_cast_info, bool, is_autoattack);

        IMPORT_FIELD(raw_spell_cast_info, float, total_cast_time);
        IMPORT_FIELD(raw_spell_cast_info, float, windup_time);
        IMPORT_FIELD(raw_spell_cast_info, float, start_time);
        IMPORT_FIELD(raw_spell_cast_info, float, server_cast_time);
        IMPORT_FIELD(raw_spell_cast_info, float, end_time);

        IMPORT_FIELD(raw_spell_cast_info, uint32_t, slot);
        IMPORT_FIELD(raw_spell_cast_info, uint32_t, missile_network_id);

        IMPORT_FIELD(raw_spell_cast_info, Vec3, start_position);
        IMPORT_FIELD(raw_spell_cast_info, Vec3, end_position);
        IMPORT_FIELD(raw_spell_cast_info, Vec3, direction);
        IMPORT_FIELD(raw_spell_cast_info, SpellInfo*, spell_info);
	    
    };

    struct SpellCastInfo {
        IMPORT_FIELD(spell_cast_info, int16_t, caster_index);
        IMPORT_FIELD(spell_cast_info, int16_t, target_index);
        IMPORT_FIELD(spell_cast_info, bool, is_spell);
        IMPORT_FIELD(spell_cast_info, bool, is_special_autoattack);
        IMPORT_FIELD(spell_cast_info, bool, is_autoattack);

        IMPORT_FIELD(spell_cast_info, float, total_cast_time);
        IMPORT_FIELD(spell_cast_info, float, windup_time);
        IMPORT_FIELD(spell_cast_info, float, start_time);
        IMPORT_FIELD(spell_cast_info, float, server_cast_time);
        IMPORT_FIELD(spell_cast_info, float, end_time);

        IMPORT_FIELD(spell_cast_info, uint32_t, slot);
        IMPORT_FIELD(spell_cast_info, uint32_t, missile_network_id);

        IMPORT_FIELD(spell_cast_info, Vec3, start_position);
        IMPORT_FIELD(spell_cast_info, Vec3, end_position);
        IMPORT_FIELD(spell_cast_info, Vec3, direction);
        IMPORT_FIELD(spell_cast_info, SpellInfo*, spell_info);
    };

    struct BuffInstance {
        IMPORT_FIELD(buff_instance, bool, is_active);
        IMPORT_FIELD(buff_instance, bool, is_permanent);
        IMPORT_FIELD_STRING(buff_instance, _get_name);
        IMPORT_FIELD(buff_instance, uint32_t, get_hash);
        IMPORT_FIELD(buff_instance, uint32_t, stacks);
        IMPORT_FIELD(buff_instance, float, start_time);
        IMPORT_FIELD(buff_instance, float, end_time);
        IMPORT_FIELD(buff_instance, float, duration);
        IMPORT_FIELD(buff_instance, int, type);
    };

    struct BuffManager {
        IMPORT_METHOD(buff_manager, bool, has_buff, uint32_t spell_hash, spell_hash);
        IMPORT_METHOD(buff_manager, BuffInstance*, get_buff, uint32_t spell_hash, spell_hash);
        IMPORT_METHOD(buff_manager, void, get_all, std::vector<BuffInstance*>* out, out);
        // IMPORT_FIELD_VECTOR(buff_manager, std::vector<BuffInstance*>, get_all);
        IMPORT_METHOD(buff_manager, BuffInstance*, has_buff_of_type, int buff_type, buff_type);
        IMPORT_FIELD(buff_manager, bool, has_hard_cc);
        IMPORT_FIELD(buff_manager, bool, is_immobile);
    };

    struct AiManager {
        IMPORT_FIELD( ai_manager, Vec3, path_start);
        IMPORT_FIELD( ai_manager, Vec3, path_end);
        IMPORT_FIELD( ai_manager, Vec3, velocity);
        IMPORT_FIELD( ai_manager, bool, is_dashing);
        IMPORT_FIELD( ai_manager, float, dash_speed);
        IMPORT_FIELD( ai_manager, bool, is_moving);
        IMPORT_FIELD( ai_manager, uint32_t, next_waypoint_index);
        IMPORT_FIELD_VECTOR( ai_manager, std::vector<Vec3>, get_waypoints);
    };

    struct Object {
        IMPORT_FIELD(object, Vec3, position);
        IMPORT_FIELD(object, Vec3, direction);
        IMPORT_FIELD(object, int16_t, index);
        IMPORT_FIELD(object, uint32_t, network_id);
        IMPORT_FIELD(object, float, health);
        IMPORT_FIELD(object, float, max_health);
        IMPORT_FIELD(object, float, mana);
        IMPORT_FIELD(object, float, max_mana);
        IMPORT_FIELD(object, bool, is_hero);
        IMPORT_FIELD(object, bool, is_minion);
        IMPORT_FIELD(object, bool, is_missile);
        IMPORT_FIELD(object, bool, is_turret);
        IMPORT_FIELD(object, bool, is_building); // inhibitor, nexus
        IMPORT_FIELD(object, bool, is_particle);

        IMPORT_FIELD(object, bool, is_dead);
        IMPORT_FIELD(object, bool, is_invisible);
        IMPORT_FIELD(object, bool, is_invulnerable);
        IMPORT_FIELD(object, bool, is_selectable);

        IMPORT_FIELD(object, bool, is_zombie);
        IMPORT_FIELD(object, bool, is_active);
        IMPORT_FIELD(object, uint8_t, team);
        IMPORT_FIELD( object, bool, is_unkillable );
        IMPORT_FIELD( object, bool, valid_ult );
        IMPORT_FIELD( object, bool, ignore_cc );
        IMPORT_FIELD(object, std::optional<ResourceBarRect>, get_hpbar_position);
        IMPORT_FIELD(object, float, total_armor);
        IMPORT_FIELD(object, float, bonus_armor);
        IMPORT_FIELD(object, float, total_magic_resist);
        IMPORT_FIELD(object, float, bonus_magic_resist);
        IMPORT_FIELD(object, float, attack_damage);
        IMPORT_FIELD(object, float, bonus_attack_damage);
        IMPORT_FIELD(object, float, attack_speed);
        IMPORT_FIELD(object, float, bonus_attack_speed);
        IMPORT_FIELD(object, float, attack_range);
        IMPORT_FIELD(object, float, movement_speed);
        IMPORT_FIELD(object, float, ability_haste);
        IMPORT_FIELD(object, float, crit_chance);
        IMPORT_FIELD(object, float, crit_damage_modifier);
        IMPORT_FIELD(object, float, ability_power);
        IMPORT_FIELD(object, float, shield);
        IMPORT_FIELD(object, float, physical_shield);
        IMPORT_FIELD(object, float, magical_shield);
        IMPORT_FIELD(object, float, lethality);
        IMPORT_FIELD(object, float, armor_penetration_percent);
        IMPORT_FIELD(object, float, flat_magic_penetration);
        IMPORT_FIELD(object, float, magic_penetration_percent);
        IMPORT_FIELD(object, float, experience);
        IMPORT_FIELD(object, int, level);
        IMPORT_FIELD(object, float, get_bounding_radius);
        IMPORT_FIELD_STRING(object, object_name);
        IMPORT_FIELD_STRING(object, display_name);
        IMPORT_FIELD(object, Object*, get_owner_object);

        // missile methods
        IMPORT_FIELD(object, Vec3, missile_start_position);
        IMPORT_FIELD(object, Vec3, missile_end_position);
        IMPORT_FIELD(object, float, missile_spawn_time);
        IMPORT_FIELD(object, int16_t, missile_target_index);
        IMPORT_FIELD(object, SpellCastInfo*, missile_sci);
        IMPORT_FIELD(object, SpellInfo*, missile_spell_info);

        IMPORT_FIELD(object, EMinionType, get_minion_type);
        IMPORT_FIELD(object, EWardType, get_ward_type);
        IMPORT_FIELD(object, bool, is_normal_minion);
        IMPORT_FIELD(object, bool, is_misc_minion);
        IMPORT_FIELD(object, bool, is_ward);
        IMPORT_FIELD(object, bool, is_plant);
        IMPORT_FIELD(object, bool, is_lane_minion);
        IMPORT_FIELD(object, bool, is_jungle_monster);
        IMPORT_FIELD(object, int, get_monster_priotity);
        IMPORT_FIELD(object, bool, is_ranged);
        IMPORT_FIELD(object, bool, is_melee);
        IMPORT_FIELD(object, void*, get_square_texture); // this is the champion/minion splash art, drawn with render::texture() function
        IMPORT_FIELD(object, void*, get_circle_texture); // this is the champion/minion splash art, drawn with render::texture() function


        IMPORT_FIELD(object, SpellCastInfo*, get_spell_cast_info);
        IMPORT_FIELD(object, AiManager*, get_ai_manager);
        IMPORT_FIELD(object, BuffManager*, get_buff_manager);
        IMPORT_METHOD(object, SpellSlot*, get_spell_slot, ESpellSlot slot, slot);

        IMPORT_METHOD(object, bool, has_buff, uint32_t hash, hash);
        IMPORT_METHOD(object, bool, has_item, ItemID id, id);
        IMPORT_METHOD(object, InventorySlot*, get_inventory_slot_by_item, ItemID id, id);
        IMPORT_METHOD(object, InventorySlot*, get_inventory_slot_by_index, uint32_t index, index);
    };

    // define functions
    namespace spellbook {
        bool cast_spell_to_position(ESpellSlot slot, Vec3 position);
        bool cast_spell_on_target(ESpellSlot slot, uint32_t network_id);
        bool cast_spell_direction(ESpellSlot slot, Vec3 start, Vec3 end);
        bool cast_spell(ESpellSlot slot);
        bool update_chargeable_spell(ESpellSlot slot, Vec3 position);
    } // namespace spellbook

    namespace indicator_manager {

        void create_range_indicator(ESpellSlot slot, float range, Color color);
        void update_range_indicator(ESpellSlot slot, float range, Color color);
        void remove_range_indicator(ESpellSlot slot);
    } // namespace indicator_manager

    namespace target_selector {
        Object* get_primary_target();
        Object* get_secondary_target();
        bool is_target_forced();
        bool is_bad_target(Object* target);

        void set_primary_target(Object* target);
        void set_secondary_target(Object* target);
        void toggle_core_target_selector(bool enable);
        bool is_core_target_selector_active();
        int get_champion_priority(std::string champion_name);

    } // namespace target_selector

    namespace menu_api {
        struct ConfigKey 
        {
           private:

            uint64_t value = 0;

           public:

            ConfigKey() = default;

            ConfigKey(const uint64_t value): value(value) {}

            template<uint32_t S> consteval ConfigKey(const char (&str)[S]) {
                value = fnv1a64<S>(str);
            }

            inline operator uint64_t() const {
                return value;
            }
        };

        inline const SmallStr hitchance_names[ ] = {
            SmallStr( "Fast" ), SmallStr( "Normal" ), SmallStr( "High" ), SmallStr( "Very High" ), SmallStr( "Immobile" ),
        };

        bool create_sub_menu(SmallStr label, std::function<void()> content, SmallStr description);
        void end_sub_menu();
        void create_separator(SmallStr label = "");
        void create_new_line();
        bool create_combo_box(SmallStr label, int* current_item, const SmallStr items[], int items_count);
        bool create_checkbox(SmallStr label, bool* value);
        bool create_int_slider(SmallStr label, int* value, int min, int max);
        bool create_keybind(SmallStr label, std::uint64_t value_hash); //always use "##" tags for labels, example "Semi-Manual Q##KindredSemiManualQ"
        bool create_color_picker( SmallStr label, SmallStr description, float* col );
        uint64_t* init_config(const uint64_t key, const uint64_t default_value);
        uint64_t config_get(const uint64_t key);
        void set_color( SmallStr str_key, float* v ); //float col[4] = { 1.f, 1.f, 1.f, 1.f }; //0.f - 1.f
        void init_color_flt( SmallStr str_key, float* v ); //float col[4] = { 1.f, 1.f, 1.f, 1.f }; //0.f - 1.f
        void init_color_rgb( SmallStr str_key, Color const& v );
        Color get_color( SmallStr str_key );
        template<typename T> T cfg_get(const uint64_t key) 
        { 
            return static_cast<T>(config_get(key));
        }
        bool* init_bool(const ConfigKey key, const bool default_value);
        int* init_int(const ConfigKey key, const int default_value);
        void set_default_keybind( SmallStr label, int imgui_key, bool is_hold );
        inline void set_default_keybind( SmallStr label, ImGuiKey key, bool is_hold )
        {
            set_default_keybind( label, static_cast<int>( key ), is_hold );
        }
        void init_keybind( const SmallStr label, uint64_t hashed_name );
        void update_permashow( SmallStr label, uint64_t v );
        void enable_second_tab( bool v );
        void set_tab_name( int i, SmallStr name ); // i = 0, i = 1
        void create_whitelist( SmallStr config_base, SmallStr display_name, bool enemies = true, bool allies = false );
        bool is_whitelisted( SmallStr config_base, Object* x );
        void init_whitelist( SmallStr config_base, bool default_enabled = true ); //also inits for allies just in case
    } // namespace menu_api

    // define functions
    namespace utils {
        Vec2 get_screen_cursor_position();
        Vec2 get_screen_size();

        // uses virtual key codes https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
        bool is_key_down(int key);

        // uses virtual key codes https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
        bool is_key_clicked(int key);

        void create_world_circle(Vec3 position, float duration, float radius);
        void create_world_message(Vec3 position, std::string title, std::string text, Color title_color, float duration);
        void console_debug_message(std::string message, std::string tag, Color tag_color);
        float get_autoattack_damage(Object* target, bool include_on_hit);
        float calculate_real_damage(Object* target, float damage, bool is_physical_damage);
        float get_real_health(Object* target, EDamageType damage_type, float delay, bool predict_ally_damage);
        EHeroes get_current_hero();
        EHeroes get_hero( Object* x );
        bool is_wall_in_line(Vec3 start, Vec3 end);
        bool is_position_near_turret(Vec3 position, bool ally_turret, float nearby_threshold);
        bool is_position_under_turret(Vec3 position, bool ally_turret);
        int get_nearby_champions_count(Vec3 position, bool allies, float nearby_threshold);
        Polygon get_line_missile_hitbox_polygon(Vec3 start, Vec3 end, float radius);
        bool is_windwall_in_line( Vec3 start, Vec3 end );
        int is_casting_interruptible_spell( Object* x ); //0 - not casting, 1 - medium danger spell (MasterYi W), 2 - high danger level spell (Katarina R)
        //min_aa_count = total health of all minions inside the hit zone must be >= {x} aa damage
        Vec3 get_best_clear_pos( float delay, float radius, float range, float speed, bool is_jungle, int type, int aa_count, int min_minions_count ); //can be invalid!
        bool is_facing( Object* source, Object* other, float angle_req = 120.f );
        Vec3 get_heroes_multihit_position( float range, float speed, float radius, float delay, bool linear, int min_count = 2 );
    } // namespace utils

    // define functions
    namespace orbwalker {
        EOrbwalkerMode get_mode();
        bool is_logic_modifier_active( ); //FastLaneclear/freeze/fullcombo/etc
        bool can_spellfarm( );
        bool can_attack(int16_t);
        bool is_attackable(Object*);
        bool is_winding_up();
        bool is_casting_spell();
        void on_cast();
        float get_attack_delay();
        float get_attack_cast_delay();
        void ignore_minion(int16_t, float);
        bool is_minion_ignored(int16_t);
        void disable_movement_for(float);   // duration is actually expire time, so do give it arg as game_time + duration
        void disable_autoattack_for(float); // duration is actually expire time, so do give it arg as game_time + duration
        Color get_rainbow_color();
        Color get_pulsing_color();

    } // namespace orbwalker

    struct SSpellData
    {
        u32 spellHash{};
        u32 heroHash{}; //can be not set........
        bool isCC{};
        char menuName[ 128 ]{};;
        bool isTargeted{};
        bool isMissile{};
        bool isSpecial{};
        bool isSkillshot{};
        bool isSkillshotMissile{};
        bool isObject{};

        u64 uniqueID{}; //use this
        EHeroes heroID{ EHeroes::null }; //can be not set........
        //compare both: ( heroID == utils::get_hero( object ) || heroHash == rt_hash( object->object_name( ) ) ) for menu
    };

    struct SEvadeSpell
    {
        int danger_level{}; //empty for targeted
        u64 unique_id{}; //use this
        float time_to_hit{};
        bool is_cc{};
        bool is_missile{};
        bool is_targeted{};
    };

    namespace evade
    {
        bool is_active( );
        bool is_evade_toggled( );
        bool is_position_safe( Vec3, int );
        void set_evade_active( bool is_active );
        std::vector<SSpellData> const get_supported_spells( );
        std::vector<SEvadeSpell>const get_active_missiles( Vec3 pos = Vec3{} ); //is_zero = player->pos
        std::vector<SEvadeSpell>const get_active_targeted( );
        std::vector<SEvadeSpell>const get_active_skillshots( Vec3 pos = Vec3{} );
        std::vector<SEvadeSpell>const get_active_special( bool evade_early = true ); //Warwick E
    } // namespace evade

    namespace prediction {
        float get_time_on_current_path(Object*);
        bool is_minion_in_line(Vec3, Vec3, float, unsigned);
        bool is_minion_in_line_predicted(Vec3, Vec3, float, float, float);
        float get_incoming_damage(Object*, float);
        int get_incoming_attack_count(Object*);
        float predict_minion_health(Object*, float);
        std::optional<Vec3> simple_predict(Object*, float);
        PredictionResult predict_skillshot(Object*, float, float, float, float, Vec3, bool);
    } // namespace prediction

    namespace navgrid {
        bool is_wall(Vec3);
        float get_height(Vec3);
    } // namespace navgrid

    namespace object_manager {
        Object* get_local();
        const std::vector<Object*>& get_enemy_heroes();
        const std::vector<Object*>& get_ally_heroes();
        const std::vector<Object*>& get_enemy_minions();
        const std::vector<Object*>& get_ally_minions();
        const std::vector<Object*>& get_enemy_missiles();
        const std::vector<Object*>& get_ally_missiles();
        const std::vector<Object*>& get_enemy_turrets();
        const std::vector<Object*>& get_ally_turrets();
        const std::vector<Object*>& get_uncategorized_objects();

        Object* get_object_by_index(int16_t);
        Object* get_object_by_network_id(uint32_t);
    }; // namespace object_manager

   namespace console {
        void print( std::string message, Color color = Color::white());
    }; // namespace console

    void init(FnLoadFunction);

#ifdef SDK_IMPLEMENTATION

    // draw imports
    namespace render {

        IMPORT_FUNCTION(
            render,
            void,
            text,
            JOIN(Vec2 position, const std::string& text_string, float text_size, Color color, bool drop_shadow),
            JOIN(position, text_string, text_size, color, drop_shadow)
        );

        IMPORT_FUNCTION(
            render,
            void,
            text_3d,
            JOIN(Vec3 world_position, const std::string& text_string, float text_size, Color color, bool drop_shadow),
            JOIN(world_position, text_string, text_size, color, drop_shadow)
        );

        IMPORT_FUNCTION(render, Vec2, get_text_size, JOIN(const std::string& text_string, float text_size), JOIN(text_string, text_size));
        IMPORT_FUNCTION(
            render,
            void,
            box,
            JOIN(Vec2 position, Vec2 size, Color color, float rounding, float thickness),
            JOIN(position, size, color, rounding, thickness)
        );

        IMPORT_FUNCTION(
            render,
            void,
            filled_box,
            JOIN(Vec2 position, Vec2 size, Color color, float rounding),
            JOIN(position, size, color, rounding)
        );

        IMPORT_FUNCTION(
            render,
            void,
            circle,
            JOIN(Vec2 position, float radius, Color color, int segments, float thickness),
            JOIN(position, radius, color, segments, thickness)
        );

        IMPORT_FUNCTION(
            render,
            void,
            filled_circle,
            JOIN(Vec2 position, float radius, Color color, int segments),
            JOIN(position, radius, color, segments)
        );

        IMPORT_FUNCTION(render, void, line, JOIN(Vec2 start, Vec2 end, Color color, float thickness), JOIN(start, end, color, thickness));
        IMPORT_FUNCTION(
            render,
            void,
            triangle,
            JOIN(Vec2 left, Vec2 right, Vec2 bottom, Color color, float thickness),
            JOIN(left, right, bottom, color, thickness)
        );

        IMPORT_FUNCTION(
            render,
            void,
            filled_triangle,
            JOIN(Vec2 left, Vec2 right, Vec2 bottom, Color color),
            JOIN(left, right, bottom, color)
        );

        IMPORT_FUNCTION(
            render,
            void,
            rectangle_3d,
            JOIN(Vec3 start, Vec3 end, float radius, Color color, int flags, float thickness),
            JOIN(start, end, radius, color, flags, thickness)
        );

        IMPORT_FUNCTION(
            render,
            void,
            polygon_3d,
            JOIN(const std::vector<Vec3>& polygon, Color color, int flags, float thickness),
            JOIN(polygon, color, flags, thickness)
        );

        IMPORT_FUNCTION(
            render,
            void,
            line_3d,
            JOIN(Vec3 start, Vec3 end, Color color, float thickness),
            JOIN(start, end, color, thickness)
        );

        IMPORT_FUNCTION(
            render,
            void,
            circle_3d,
            JOIN(Vec3 position, Color color, float radius, int flags, int segments, float thickness, float angle, Vec3 direction),
            JOIN(position, color, radius, flags, segments, thickness, angle, direction)
        );

        IMPORT_FUNCTION(render, void, texture, JOIN(Vec2 position, Vec2 size, void* texture_view), JOIN(position, size, texture_view));

        IMPORT_FUNCTION(
            render,
            void,
            circle_minimap,
            JOIN(Vec3 position, Color color, float radius, int segments, float thickness),
            JOIN(position, color, radius, segments, thickness)
        );

        // 3d glow functions
        IMPORT_FUNCTION( render, void, circle_3d_glow,
            JOIN( const Vec3 center,
                const f32 radius,
                const Color color,
                const f32 thickness,
                const f32 glow,
                const u8 type,
                const f32 glow_opacity,
                const f32 background_opacity ),
            JOIN( center,
                radius,
                color,
                thickness,
                glow,
                type,
                glow_opacity,
                background_opacity ) );
        IMPORT_FUNCTION( render, void, polyline_3d_glow,
            JOIN( const std::vector<Vec3>& points,
                const Color color,
                const f32 thickness,
                const f32 glow,
                const f32 glow_opacity ),
            JOIN( points,
                color,
                thickness,
                glow,
                glow_opacity ) );
        IMPORT_FUNCTION( render, void, polygon_3d_glow,
            JOIN( const std::vector<Vec3>& points,
                const Color color,
                const f32 thickness,
                const f32 glow,
                const f32 glow_opacity,
                const f32 background_opacity ),
            JOIN( points,
                color,
                thickness,
                glow,
                glow_opacity,
                background_opacity ) );
        IMPORT_FUNCTION( render, void, line_3d_glow,
            JOIN( Vec3 start, Vec3 end,
            const Color color, const f32 thickness, const f32 glow, const f32 glow_strength ),
            JOIN( start, end, color, thickness, glow, glow_strength ) );
    } // namespace render

    // math imports
    namespace world_math {
        std::vector<Vec3> get_3d_circle_points(Vec3 world_position, float radius, int segments, float draw_angle, Vec3 circle_direction) {
            static const auto fn =
                (void (*)(Vec3 world_position, float radius, int segments, float draw_angle, Vec3 circle_direction, std::vector<Vec3>* out))(
                    load_function(fnv1a64("world_math::get_3d_circle_points"))
                );
            std::vector<Vec3> out;
            fn(world_position, radius, segments, draw_angle, circle_direction, &out);
            return out;
        }
    } // namespace world_math

    FnLoadFunction load_function = nullptr;

    IMPORT_FUNCTION(
        globals,
        void,
        register_plugin,
        JOIN(SmallStr name, const std::vector<Subscription>& subscriptions),
        JOIN(name, subscriptions)
    );
    IMPORT_FUNCTION( globals, void, disable_core_plugin, , ); //Disable internal core module if you want to make your own

    IMPORT_FUNCTION(globals, float, get_time, , );
    IMPORT_FUNCTION(globals, float, get_latency, , );
    IMPORT_FUNCTION(globals, Vec3, get_cursor, , );
    IMPORT_FUNCTION(globals, bool, move_to, Vec3 position, position);
    IMPORT_FUNCTION(globals, bool, order_attack, uint32_t network_id, network_id);

    // w2s imports
    IMPORT_FUNCTION(globals, std::optional<Vec2>, world_to_screen, Vec3 world_position, world_position);
    IMPORT_FUNCTION(globals, Vec2, world_to_minimap, Vec3 world_position, world_position);

    namespace spellbook {
        IMPORT_FUNCTION(spellbook, bool, cast_spell_to_position, JOIN(ESpellSlot slot, Vec3 position), JOIN(slot, position));
        IMPORT_FUNCTION(spellbook, bool, cast_spell_on_target, JOIN(ESpellSlot slot, uint32_t network_id), JOIN(slot, network_id));
        IMPORT_FUNCTION(spellbook, bool, cast_spell_direction, JOIN(ESpellSlot slot, Vec3 start, Vec3 end), JOIN(slot, start, end));
        IMPORT_FUNCTION(spellbook, bool, cast_spell, ESpellSlot slot, slot);
        IMPORT_FUNCTION(spellbook, bool, update_chargeable_spell, JOIN(ESpellSlot slot, Vec3 position), JOIN(slot, position));
    } // namespace spellbook

    namespace indicator_manager {
        IMPORT_FUNCTION(
            indicator_manager,
            void,
            create_range_indicator,
            JOIN(ESpellSlot slot, float range, Color color),
            JOIN(slot, range, color)
        );
        IMPORT_FUNCTION(
            indicator_manager,
            void,
            update_range_indicator,
            JOIN(ESpellSlot slot, float range, Color color),
            JOIN(slot, range, color)
        );

        IMPORT_FUNCTION(indicator_manager, void, remove_range_indicator, ESpellSlot slot, slot);
    } // namespace indicator_manager

    namespace target_selector {

        IMPORT_FUNCTION(target_selector, Object*, get_primary_target, , );
        IMPORT_FUNCTION(target_selector, Object*, get_secondary_target, , );
        IMPORT_FUNCTION(target_selector, bool, is_target_forced, , );
        IMPORT_FUNCTION(target_selector, bool, is_bad_target, Object* target, target);
        IMPORT_FUNCTION(target_selector, void, set_primary_target, Object* target, target);
        IMPORT_FUNCTION(target_selector, void, set_secondary_target, Object* target, target);
        IMPORT_FUNCTION(target_selector, void, toggle_core_target_selector, bool enable, enable);
        IMPORT_FUNCTION(target_selector, bool, is_core_target_selector_active, , );
        IMPORT_FUNCTION(target_selector, int, get_champion_priority, std::string champion_name, champion_name);
    } // namespace target_selector

    namespace console {
        IMPORT_FUNCTION(console, void, print, 
            JOIN(std::string message, Color color), 
            JOIN(message, color));
    }

    namespace menu_api {
        IMPORT_FUNCTION(menu_api, bool, create_sub_menu, 
            JOIN(SmallStr label, std::function<void()> content, SmallStr description), 
            JOIN(label, content, description)
        );
        IMPORT_FUNCTION(menu_api, void, create_separator, 
            SmallStr text,
            text );
        IMPORT_FUNCTION(menu_api, void, create_new_line, , );
        IMPORT_FUNCTION( menu_api, void, init_keybind, JOIN( SmallStr label, uint64_t hashed_name ), JOIN( label, hashed_name ) );
        IMPORT_FUNCTION( menu_api, void, set_default_keybind, JOIN( SmallStr label, int imgui_key, bool is_hold ), JOIN( label, imgui_key, is_hold ) )
        IMPORT_FUNCTION(
            menu_api,
            bool,
            create_combo_box,
            JOIN(SmallStr label, int* current_item, const SmallStr items[], int items_count),
            JOIN(label, current_item, items, items_count)
        );
        IMPORT_FUNCTION(menu_api, bool, create_checkbox, JOIN(SmallStr label, bool* value), JOIN(label, value));
        IMPORT_FUNCTION(
            menu_api,
            bool,
            create_int_slider,
            JOIN(SmallStr label, int* value, int min, int max),
            JOIN(label, value, min, max)
        );
        IMPORT_FUNCTION(menu_api, bool, create_keybind, JOIN(SmallStr label, const uint64_t value), JOIN(label, value));
        IMPORT_FUNCTION( menu_api, bool, create_color_picker,
            JOIN( SmallStr label, SmallStr description, float* col ),
            JOIN( label, description, col ) );
        IMPORT_FUNCTION(menu_api, uint64_t*, init_config, JOIN(const uint64_t key, const uint64_t default_value), JOIN(key, default_value));
        IMPORT_FUNCTION(menu_api, uint64_t, config_get, const uint64_t key, key);
        IMPORT_FUNCTION( menu_api, void, set_color, JOIN( SmallStr str_key, float* v ), JOIN( str_key, v ) );
        IMPORT_FUNCTION( menu_api, void, init_color_rgb, JOIN( SmallStr str_key, Color const& v ), JOIN( str_key, v ) );
        IMPORT_FUNCTION( menu_api, void, init_color_flt, JOIN( SmallStr str_key, float* v ), JOIN( str_key, v ) );
        IMPORT_FUNCTION( menu_api, Color, get_color, SmallStr str_key, str_key );
        IMPORT_FUNCTION( menu_api, void, update_permashow, JOIN( SmallStr label, uint64_t v ), JOIN( label, v ) );
        bool* init_bool( const ConfigKey key, const bool default_value ) {
            return reinterpret_cast< bool* >( init_config( uint64_t( key ), uint64_t( default_value ) ) );
        }

        int* init_int( const ConfigKey key, const int default_value ) {
            return reinterpret_cast< int* >( init_config( uint64_t( key ), uint64_t( default_value ) ) );
        };
        IMPORT_FUNCTION( menu_api, void, enable_second_tab, bool v, v );
        IMPORT_FUNCTION( menu_api, void, set_tab_name, JOIN( int i, SmallStr name ), JOIN( i, name ) );
        IMPORT_FUNCTION( menu_api, void, create_whitelist,
            JOIN( SmallStr config_base, SmallStr display_name, bool enemies, bool allies ),
            JOIN( config_base, display_name, enemies, allies ) );
        IMPORT_FUNCTION( menu_api, bool, is_whitelisted, JOIN( SmallStr config_base, Object* x ), JOIN( config_base, x ) );
        IMPORT_FUNCTION( menu_api, void, init_whitelist, JOIN( SmallStr config_base, bool default_enabled ), JOIN( config_base, default_enabled ) );
    } // namespace menu_api

    namespace utils {

        IMPORT_FUNCTION(utils, Vec2, get_screen_cursor_position, , );
        IMPORT_FUNCTION(utils, Vec2, get_screen_size, , );

        IMPORT_FUNCTION(utils, bool, is_key_down, int key, key);
        IMPORT_FUNCTION(utils, bool, is_key_clicked, int key, key);

        IMPORT_FUNCTION(
            utils,
            void,
            create_world_circle,
            JOIN(Vec3 position, float duration, float radius),
            JOIN(position, duration, radius)
        );

        IMPORT_FUNCTION(
            utils,
            void,
            create_world_message,
            JOIN(Vec3 position, std::string title, std::string text, Color title_color, float duration),
            JOIN(position, title, text, title_color, duration)
        );

        IMPORT_FUNCTION(
            utils,
            void,
            console_debug_message,
            JOIN(std::string message, std::string tag, Color tag_color),
            JOIN(message, tag, tag_color)
        );

        IMPORT_FUNCTION(utils, float, get_autoattack_damage, JOIN(Object* target, bool include_on_hit), JOIN(target, include_on_hit));
        IMPORT_FUNCTION(
            utils,
            float,
            calculate_real_damage,
            JOIN(Object* target, float damage, bool is_physical_damage),
            JOIN(target, damage, is_physical_damage)
        );
        IMPORT_FUNCTION(
            utils,
            float,
            get_real_health,
            JOIN(Object* target, EDamageType damage_type, float delay, bool predict_ally_damage),
            JOIN(target, damage_type, delay, predict_ally_damage)
        );
        IMPORT_FUNCTION(utils, EHeroes, get_current_hero, , );
        IMPORT_FUNCTION( utils, EHeroes, get_hero, Object* x, x );
        IMPORT_FUNCTION(utils, bool, is_wall_in_line, JOIN(Vec3 start, Vec3 end), JOIN(start, end));
        IMPORT_FUNCTION(
            utils,
            bool,
            is_position_near_turret,
            JOIN(Vec3 position, bool ally_turret, float nearby_threshold),
            JOIN(position, ally_turret, nearby_threshold)
        );
        IMPORT_FUNCTION(utils, bool, is_position_under_turret, JOIN(Vec3 position, bool ally_turret), JOIN(position, ally_turret));
        IMPORT_FUNCTION(
            utils,
            int,
            get_nearby_champions_count,
            JOIN(Vec3 position, bool allies, float nearby_threshold),
            JOIN(position, allies, nearby_threshold)
        );

        Polygon get_line_missile_hitbox_polygon(Vec3 start, Vec3 end, float radius) {
            static const auto fn = (void (*)(Vec3 start, Vec3 end, float radius, std::vector<Vec3>* points))(
                load_function(fnv1a64("utils::get_line_missile_hitbox_polygon"))
            );
            Polygon out;
            fn(start, end, radius, &out.points);
            return out;
        }

        IMPORT_FUNCTION( utils, bool, is_windwall_in_line, JOIN( Vec3 start, Vec3 end ), JOIN( start, end ) );
        IMPORT_FUNCTION( utils, int, is_casting_interruptible_spell, Object* x, x );
        IMPORT_FUNCTION( utils, Vec3, get_best_clear_pos,
            JOIN( float delay, float radius, float _range, float speed, bool is_jungle, int type, int aa_count, int minions_count ),
            JOIN( delay, radius, _range, speed, is_jungle, type, aa_count, minions_count ) );
        IMPORT_FUNCTION( utils, bool, is_facing,
            JOIN( Object* source, Object* other, float angle_req ),
            JOIN( source, other, angle_req ) );
        IMPORT_FUNCTION( utils, Vec3, get_heroes_multihit_position,
            JOIN( float range, float speed, float radius, float delay, bool linear, int min_count ),
            JOIN( range, speed, radius, delay, linear, min_count ) );
    } // namespace utils

    namespace orbwalker {
        IMPORT_FUNCTION(orbwalker, EOrbwalkerMode, get_mode, , );
        IMPORT_FUNCTION( orbwalker, bool, is_logic_modifier_active, , );
        IMPORT_FUNCTION( orbwalker, bool, can_spellfarm, , );
        IMPORT_FUNCTION(orbwalker, bool, can_attack, int16_t target_index, target_index);
        IMPORT_FUNCTION(orbwalker, bool, is_attackable, Object* target, target);
        IMPORT_FUNCTION(orbwalker, bool, is_winding_up, , );
        IMPORT_FUNCTION(orbwalker, bool, is_casting_spell, , );
        IMPORT_FUNCTION(orbwalker, void, on_cast, , );
        IMPORT_FUNCTION(orbwalker, float, get_attack_delay, , );
        IMPORT_FUNCTION(orbwalker, float, get_attack_cast_delay, , );
        IMPORT_FUNCTION(orbwalker, void, ignore_minion, JOIN(int16_t index, float duration), JOIN(index, duration));
        IMPORT_FUNCTION(orbwalker, bool, is_ignored, int16_t index, index);

        IMPORT_FUNCTION(orbwalker, void, disable_movement_for, float duration, duration); 
        IMPORT_FUNCTION(orbwalker, void, disable_autoattack_for, float duration, duration);
                                                                                           
        IMPORT_FUNCTION(orbwalker, Color, get_rainbow_color, , );
        IMPORT_FUNCTION(orbwalker, Color, get_pulsing_color, , );
    } // namespace orbwalker

    namespace prediction {

        IMPORT_FUNCTION(prediction, float, get_time_on_current_path, JOIN(Object* object), JOIN(object));

        IMPORT_FUNCTION(
            prediction,
            bool,
            is_minion_in_line,
            JOIN(Vec3 start, Vec3 end, float width, unsigned ignored_network_id),
            JOIN(start, end, width, ignored_network_id)
        );

        IMPORT_FUNCTION(
            prediction,
            bool,
            is_minion_in_line_predicted,
            JOIN(Vec3 start, Vec3 end, float width, float windup_time, float missile_speed),
            JOIN(start, end, width, windup_time, missile_speed)
        );
        IMPORT_FUNCTION(prediction, float, get_incoming_damage, JOIN(Object* object, float duration), JOIN(object, duration));
        IMPORT_FUNCTION(prediction, int, get_incoming_attack_count, Object* object, object);
        IMPORT_FUNCTION(prediction, float, predict_minion_health, JOIN(Object* object, float duration), JOIN(object, duration));
        IMPORT_FUNCTION(prediction, std::optional<Vec3>, simple_predict, JOIN(Object* object, float duration), JOIN(object, duration));
        IMPORT_FUNCTION(
            prediction,
            PredictionResult,
            predict_skillshot,
            JOIN(
                Object* target,
                float projectile_range,
                float projectile_speed,
                float projectile_width,
                float windup_delay,
                Vec3 source_position,
                bool edge_range
            ),
            JOIN(target, projectile_range, projectile_speed, projectile_width, windup_delay, source_position, edge_range)
        );
    } // namespace prediction

    namespace evade {
        IMPORT_FUNCTION(evade, bool, is_active, , );
        IMPORT_FUNCTION(evade, bool, is_evade_toggled, , );
        IMPORT_FUNCTION(evade, bool, is_position_safe, JOIN(Vec3 position, int minimum_danger), JOIN(position, minimum_danger));
        IMPORT_FUNCTION( evade, void, set_evade_active, bool b, b );
        IMPORT_FUNCTION( evade, std::vector<SSpellData> const, get_supported_spells, ,  );
        IMPORT_FUNCTION( evade, std::vector<SEvadeSpell> const, get_active_missiles, Vec3 pos, pos );
        IMPORT_FUNCTION( evade, std::vector<SEvadeSpell> const, get_active_targeted, , );
        IMPORT_FUNCTION( evade, std::vector<SEvadeSpell> const, get_active_skillshots, Vec3 pos, pos );
        IMPORT_FUNCTION( evade, std::vector<SEvadeSpell> const, get_active_special, bool evade_early, evade_early );
    } // namespace evade

    namespace navgrid {
        IMPORT_FUNCTION(navgrid, bool, is_wall, Vec3 position, position);
        IMPORT_FUNCTION(navgrid, float, get_height, Vec3 position, position);
    } // namespace navgrid

    namespace object_manager {
        IMPORT_FUNCTION(object_manager, Object*, get_local, , );
        IMPORT_FUNCTION(object_manager, const std::vector<Object*>&, get_enemy_heroes, , );
        IMPORT_FUNCTION(object_manager, const std::vector<Object*>&, get_ally_heroes, , );
        IMPORT_FUNCTION(object_manager, const std::vector<Object*>&, get_enemy_minions, , );
        IMPORT_FUNCTION(object_manager, const std::vector<Object*>&, get_ally_minions, , );
        IMPORT_FUNCTION(object_manager, const std::vector<Object*>&, get_enemy_missiles, , );
        IMPORT_FUNCTION(object_manager, const std::vector<Object*>&, get_ally_missiles, , );
        IMPORT_FUNCTION(object_manager, const std::vector<Object*>&, get_enemy_turrets, , );
        IMPORT_FUNCTION(object_manager, const std::vector<Object*>&, get_ally_turrets, , );
        IMPORT_FUNCTION(object_manager, const std::vector<Object*>&, get_uncategorized_objects, , );
        IMPORT_FUNCTION(object_manager, Object*, get_object_by_index, int16_t index, index);
        IMPORT_FUNCTION(object_manager, Object*, get_object_by_network_id, uint32_t network_id, network_id);
    }; // namespace object_manager

    void init(FnLoadFunction _load_function) {
        load_function = _load_function;
    }
#endif

} // namespace sdk

#define IsCombo sdk::orbwalker::get_mode( ) == sdk::EOrbwalkerMode::combo
#define IsHarass sdk::orbwalker::get_mode( ) == sdk::EOrbwalkerMode::harass
#define IsLaneclear sdk::orbwalker::get_mode( ) == sdk::EOrbwalkerMode::laneclear
#define IsFastClear ( IsLaneclear && sdk::orbwalker::is_logic_modifier_active( ) )
#define IsFlee sdk::orbwalker::get_mode( ) == sdk::EOrbwalkerMode::flee