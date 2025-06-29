#pragma once

#include "sdk.hpp"

using namespace sdk;

class SDKSpell
{
public:
    float delay{};
    float radius{};
    float speed{ FLT_MAX };
    bool rangeAddRadius{};
    ESpellType type{ ESpellType::linear };
    ECollisionFlags collisionFlags{ ECollisionFlags::none };

public:
    ESpellSlot slot{ ESpellSlot::max };
    float baseRange{};
    float range{};
    EHitchance minHitchance{ EHitchance::low };
    SpellSlot* inst{};
    float lastCastAttempt{ -FLT_MAX };

    Vec3 vecFrom{};

    SDKSpell( ESpellSlot _slot = ESpellSlot::q, float _range = 0.f ) :
        slot( _slot ), range{ _range }, baseRange( _range )
    {
        inst = object_manager::get_local( )->get_spell_slot( _slot );
    };

    void SetSkillshot(
        float _delay,
        float _radius,
        float _speed,
        ESpellType _type = ESpellType::linear,
        bool _isCollision = false,
        ECollisionFlags _collisionFlags = ECollisionFlags::none,
        bool _forceAddRadius = false
    );

    auto IsHexFlash( ) -> bool;
    auto SetFrom( Vec3 f = Vec3{} ) -> void;
    auto IsReady( float time = -1.f ) -> bool;
    u32 Level( );
    float ManaCost( );
    float RealRange( );
    std::string GetName( );
    u32 GetHash( );

    Object* GetTarget( float overrideRange = 0.f, std::string const& configWhitelistBase = "" );
    Object* GetClosestTargetAntiMelee( float overrideRange = 0.f, std::string whitelistbase = "" );
    Object* GetClosestTarget( float overrideRange = 0.f );

    bool CastOnInterruptTarget( float overrideRange = -1.f, bool cast_on_unit = false, int min_danger_level = 2 );
    bool Release( Vec3& pos = Vec3{} );
    bool CastDir( Vec3& pos, Vec3& pos2 );
    bool Cast( Vec3& pos = Vec3{} );
    bool CastOn( Object* tar );
    bool Cast( Object* tar, int _hc );
    bool Cast( Object* tar, EHitchance _hc );
    bool CastAOE( int min_hero_count = 2 );
    bool CastFarm( int countMinions, bool isJungle, int aaCount = 0 );

    PredictionResult GetPrediction( Object* tar, float overrideRange = -1.f, int overrideHC = -1, bool ignoreCollision = false, float overrideDelay = -1.f, float overrideSpeed = -1.f );
    auto IsCollision( Object* tar, Vec3& to, std::optional<ECollisionFlags> flags = std::nullopt ) -> bool;
};